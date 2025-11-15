#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pty.h"
#include "server.h"
#include "utils.h"
//#include <unistd.h>

#include "http/websocket.h"

#include "base/application.h"

using namespace base;



// initial message list
static char initial_cmds[] = {SET_WINDOW_TITLE, SET_PREFERENCES};
#define  LWS_PRE 6



int write ( base::net::Listener* conn, unsigned char * buf, size_t len, bool binary)
{
    
    base::net::WebSocketConnection *con = (base::net::WebSocketConnection*)conn;
    con->send((const char*) buf, len, binary );

}

static int send_initial_message(base::net::Listener* con, int index) {
  unsigned char message[LWS_PRE + 1 + 4096];
  unsigned char *p = &message[LWS_PRE];
  char buffer[128];
  int n = 0;

  char cmd = initial_cmds[index];
  switch (cmd) {
    case SET_WINDOW_TITLE:
      gethostname(buffer, sizeof(buffer) - 1);
      n = sprintf((char *)p, "%c%s (%s)", cmd, server->command, buffer);
      break;
    case SET_PREFERENCES:
      n = sprintf((char *)p, "%c%s", cmd, server->prefs_json);
      break;
    default:
      break;
  }

  return write(con, p, (size_t)n, LWS_WRITE_BINARY);
}

static void output(base::net::Listener* con, pty_buf_t *buf) {
  if (buf == NULL) return;
  char *message = (char *)xmalloc(LWS_PRE + 1 + buf->len);
  char *ptr = message + LWS_PRE;

  *ptr = OUTPUT;
  memcpy(ptr + 1, buf->base, buf->len);
  size_t n = buf->len + 1;

  if (write(con, (unsigned char *)ptr, n, LWS_WRITE_BINARY) < n) 
  {
//    printf("write OUTPUT to WS\n");
  }
  // arvind

  free(message);
}


int close_reason(base::net::Listener* conn, uint16_t statusCode  )
{
        
    base::net::WebSocketConnection *con = (base::net::WebSocketConnection*)conn;
    std::string reason= "close";
    con->shutdown( statusCode, reason);
}

int callback_on_writable(	base::net::Listener* conn)
{
    
     struct pss_tty *pss;
        
    base::net::WebSocketConnection *con = (base::net::WebSocketConnection*)conn;
    if(!con->user)
    {
        SError << "not possible state";
        exit (0);
    }else
    {
        pss = (pss_tty*)con->user;
    }
     
    if (!pss->initialized) {
        if (pss->initial_cmd_index == sizeof (initial_cmds)) {
            pss->initialized = true;
            pty_resume(pss->process);
            return 0;
        }
        if (send_initial_message(con, pss->initial_cmd_index) < 0) {
            printf("failed to send initial message, index: %d\n", pss->initial_cmd_index);
            close_reason(con, LWS_CLOSE_STATUS_UNEXPECTED_CONDITION);
            return -1;
        }
        pss->initial_cmd_index++;
        callback_on_writable(con);
        return 0;
    }

    if (pss->lws_close_status > LWS_CLOSE_STATUS_NOSTATUS) {
        close_reason(con, pss->lws_close_status);
        return 1;
    }

    if (pss->pty_buf != NULL) {
        output(con, pss->pty_buf);
        pty_buf_free(pss->pty_buf);
        pss->pty_buf = NULL;
        pty_resume(pss->process);
    }
}





 json parse_window_size(const char *buf, size_t len, uint16_t *cols, uint16_t *rows) {
//  json_tokener *tok = json_tokener_new();
//  json_object *obj = json_tokener_parse_ex(tok, buf, len);
//  struct json_object *o = NULL;
//
//  if (json_object_object_get_ex(obj, "columns", &o)) *cols = (uint16_t)json_object_get_int(o);
//  if (json_object_object_get_ex(obj, "rows", &o)) *rows = (uint16_t)json_object_get_int(o);
//
//  json_tokener_free(tok);
     
     
       json obj = json::parse( std::string(buf, len));
       
       if (obj.find("columns") != obj.end()) 
      *cols = obj["columns"].get<uint16_t>();
     
       if (obj.find("rows") != obj.end()) 
       *rows = obj["rows"].get<uint16_t>();
     
     
  return obj;
}


static pty_ctx_t *pty_ctx_init(struct pss_tty *pss) {
  pty_ctx_t *ctx = ( pty_ctx_t *) xmalloc(sizeof(pty_ctx_t));
  ctx->pss = pss;
  ctx->ws_closed = false;
  return ctx;
}

static void pty_ctx_free(pty_ctx_t *ctx) { free(ctx); }

static void process_read_cb(pty_process *process, pty_buf_t *buf, bool eof) {
  pty_ctx_t *ctx = (pty_ctx_t *)process->ctx;
  if (ctx->ws_closed) {
    pty_buf_free(buf);
    return;
  }

  if (eof && !process_running(process))
    ctx->pss->lws_close_status = process->exit_code == 0 ? 1000 : 1006;
  else
    ctx->pss->pty_buf = buf;
   
  callback_on_writable(ctx->pss->con);
}

static void process_exit_cb(pty_process *process) {
  pty_ctx_t *ctx = (pty_ctx_t *)process->ctx;
  if (ctx->ws_closed) {
    printf("process killed with signal %d, pid: %d\n", process->exit_signal, process->pid);
    goto done;
  }

  printf("process exited with code %d, pid: %d\n", process->exit_code, process->pid);
  ctx->pss->process = NULL;
  ctx->pss->lws_close_status = process->exit_code == 0 ? 1000 : 1006;
  callback_on_writable(ctx->pss->con);  //arvind

done:
  pty_ctx_free(ctx);
}

static char **build_args(struct pss_tty *pss) {
  int i, n = 0;
  char **argv = (char **) xmalloc((server->argc + pss->argc + 1) * sizeof(char *));

  for (i = 0; i < server->argc; i++) {
    argv[n++] = server->argv[i];
  }

  for (i = 0; i < pss->argc; i++) {
    argv[n++] = pss->args[i];
  }

  argv[n] = NULL;

  return argv;
}

static char **build_env(struct pss_tty *pss) {
  int i = 0, n = 2;
  char **envp = (char **)xmalloc(n * sizeof(char *));

  // TERM
  envp[i] = (char*)xmalloc(36);
  snprintf(envp[i], 36, "TERM=%s", server->terminal_type);
  i++;

  // TTYD_USER
  if (strlen(pss->user) > 0) {
    envp = (char**)xrealloc(envp, (++n) * sizeof(char *));
    envp[i] = (char*)xmalloc(40);
    snprintf(envp[i], 40, "TTYD_USER=%s", pss->user);
    i++;
  }

  envp[i] = NULL;

  return envp;
}

extern bool spawn_process(struct pss_tty *pss, uint16_t columns, uint16_t rows) 
{
  pty_process *process = process_init((void *)pty_ctx_init(pss), server->loop, build_args(pss), build_env(pss));
  if (server->cwd != NULL) process->cwd = strdup(server->cwd);
  if (columns > 0) process->columns = columns;
  if (rows > 0) process->rows = rows;
  if (pty_spawn(process, process_read_cb, process_exit_cb) != 0) {
    printf("pty_spawn: %d (%s)\n", errno, strerror(errno));
    process_free(process);
    return false;
  }
  printf("started process, pid: %d\n", process->pid);
  pss->process = process;
  callback_on_writable(pss->con); // arvind

  return true;
}


