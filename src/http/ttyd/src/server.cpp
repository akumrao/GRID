#include "server.h"

#include <errno.h>
#include <getopt.h>
//#include <json.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"



#include "http/HTTPResponder.h"
//#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

using namespace base;
using namespace base::net;
//using namespace base::test;


#ifndef TTYD_VERSION
#define TTYD_VERSION "unknown"
#endif



volatile bool force_exit = false;
struct lws_context *context;
struct server *server{nullptr};
//struct endpoints endpoints = {"/ws", "/", "/token", ""};



#if LWS_LIBRARY_VERSION_NUMBER >= 4000000
static const uint32_t backoff_ms[] = {1000, 2000, 3000, 4000, 5000};
static lws_retry_bo_t retry = {
    .retry_ms_table = backoff_ms,
    .retry_ms_table_count = LWS_ARRAY_SIZE(backoff_ms),
    .conceal_count = LWS_ARRAY_SIZE(backoff_ms),
    .secs_since_valid_ping = 5,
    .secs_since_valid_hangup = 10,
    .jitter_percent = 0,
};
#endif

// command line options
static const struct option options[] = {
  
#if LWS_LIBRARY_VERSION_NUMBER >= 4000000
    {"ping-interval", required_argument, NULL, 'P'},
#endif
    {"srv-buf-size", required_argument, NULL, 'f'},
    {"ipv6", no_argument, NULL, '6'},
    {"ssl", no_argument, NULL, 'S'},
    {"ssl-cert", required_argument, NULL, 'C'},
    {"ssl-key", required_argument, NULL, 'K'},
    {"ssl-ca", required_argument, NULL, 'A'},
    {"url-arg", no_argument, NULL, 'a'},
    {"writable", no_argument, NULL, 'W'},
    {"terminal-type", required_argument, NULL, 'T'},
    {"client-option", required_argument, NULL, 't'},
    {"check-origin", no_argument, NULL, 'O'},
    {"max-clients", required_argument, NULL, 'm'},
    {"once", no_argument, NULL, 'o'},
    {"exit-no-conn", no_argument, NULL, 'q'},
    {"browser", no_argument, NULL, 'B'},
    {"debug", required_argument, NULL, 'd'},
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, 0, 0}};
static const char *opt_string = "p:i:U:c:H:u:g:s:w:I:b:P:f:6aSC:K:A:Wt:T:Om:oqBd:vh";

static void print_help() {
    // clang-format off
    fprintf(stderr, "ttyd is a tool for sharing terminal over the web\n\n"
            "USAGE:  ./ttyd -W bash  then browse    http://localhost:8000/ \n"
            "    ttyd [options] <command> [<arguments...>]\n\n"
            "VERSION:\n"
            "    %s\n\n"
            "OPTIONS:\n"
            "    -p, --port              Port to listen (default: 7681, use `0` for random port)\n"
            "    -i, --interface         Network interface to bind (eg: eth0), or UNIX domain socket path (eg: /var/run/ttyd.sock)\n"
            "    -U, --socket-owner      User owner of the UNIX domain socket file, when enabled (eg: user:group)\n"
            "    -c, --credential        Credential for basic authentication (format: username:password)\n"
            "    -H, --auth-header       HTTP Header name for auth proxy, this will configure ttyd to let a HTTP reverse proxy handle authentication\n"
            "    -u, --uid               User id to run with\n"
            "    -g, --gid               Group id to run with\n"
            "    -s, --signal            Signal to send to the command when exit it (default: 1, SIGHUP)\n"
            "    -w, --cwd               Working directory to be set for the child program\n"
            "    -a, --url-arg           Allow client to send command line arguments in URL (eg: http://localhost:7681?arg=foo&arg=bar)\n"
            "    -W, --writable          Allow clients to write to the TTY (readonly by default)\n"
            "    -t, --client-option     Send option to client (format: key=value), repeat to add more options\n"
            "    -T, --terminal-type     Terminal type to report, default: xterm-256color\n"
            "    -O, --check-origin      Do not allow websocket connection from different origin\n"
            "    -m, --max-clients       Maximum clients to support (default: 0, no limit)\n"
            "    -o, --once              Accept only one client and exit on disconnection\n"
            "    -q, --exit-no-conn      Exit on all clients disconnection\n"
            "    -B, --browser           Open terminal with the default system browser\n"
            "    -I, --index             Custom index.html path\n"
            "    -b, --base-path         Expected base path for requests coming from a reverse proxy (eg: /mounted/here, max length: 128)\n"
#if LWS_LIBRARY_VERSION_NUMBER >= 4000000
            "    -P, --ping-interval     Websocket ping interval(sec) (default: 5)\n"
#endif
            "    -f, --srv-buf-size      Maximum chunk of file (in bytes) that can be sent at once, a larger value may improve throughput (default: 4096)\n"
#ifdef LWS_WITH_IPV6
            "    -6, --ipv6              Enable IPv6 support\n"
#endif
#if defined(LWS_OPENSSL_SUPPORT) || defined(LWS_WITH_TLS)
            "    -S, --ssl               Enable SSL\n"
            "    -C, --ssl-cert          SSL certificate file path\n"
            "    -K, --ssl-key           SSL key file path\n"
            "    -A, --ssl-ca            SSL CA file path for client certificate verification\n"
#endif
            "    -d, --debug             Set log level (default: 7)\n"
            "    -v, --version           Print the version and exit\n"
            "    -h, --help              Print this text and exit\n\n"
            "Visit https://github.com/tsl0922/ttyd to get more information and report bugs.\n",
            TTYD_VERSION
            );
    // clang-format on
}

static void print_config() {
    printf("tty configuration:\n");
    if (server->credential != NULL) printf("  credential: %s\n", server->credential);
    printf("  start command: %s\n", server->command);
    printf("  close signal: %s (%d)\n", server->sig_name, server->sig_code);
    printf("  terminal type: %s\n", server->terminal_type);
//    if (endpoints.parent[0]) {
//        printf("endpoints:\n");
//        printf("  base-path: %s\n", endpoints.parent);
//        printf("  index    : %s\n", endpoints.index);
//        printf("  token    : %s\n", endpoints.token);
//        printf("  websocket: %s\n", endpoints.ws);
//    }
    if (server->auth_header != NULL) printf("  auth header: %s\n", server->auth_header);
    if (server->check_origin) printf("  check origin: true\n");
    if (server->url_arg) printf("  allow url arg: true\n");
    if (server->max_clients > 0) printf("  max clients: %d\n", server->max_clients);
    if (server->once) printf("  once: true\n");
    if (server->exit_no_conn) printf("  exit_no_conn: true\n");
    if (server->index != NULL) printf("  custom index.html: %s\n", server->index);
    if (server->cwd != NULL) printf("  working directory: %s\n", server->cwd);
    if (!server->writable) printf("The --writable option is not set, will start in readonly mode\n");
}

static struct server *server_new(int argc, char **argv, int start) {
    struct server *ts;
    size_t cmd_len = 0;

    ts = (struct server*) xmalloc(sizeof (struct server));

    memset(ts, 0, sizeof (struct server));
    ts->client_count = 0;
    ts->sig_code = SIGHUP;
    sprintf(ts->terminal_type, "%s", "xterm-256color");
    get_sig_name(ts->sig_code, ts->sig_name, sizeof (ts->sig_name));
    if (start == argc) return ts;

    int cmd_argc = argc - start;
    char **cmd_argv = &argv[start];
    ts->argv = (char**) xmalloc(sizeof (char *) * (cmd_argc + 1));
    for (int i = 0; i < cmd_argc; i++) {
        ts->argv[i] = strdup(cmd_argv[i]);
        cmd_len += strlen(ts->argv[i]);
        if (i != cmd_argc - 1) {
            cmd_len++; // for space
        }
    }
    ts->argv[cmd_argc] = NULL;
    ts->argc = cmd_argc;

    ts->command = (char*) xmalloc(cmd_len + 1);
    char *ptr = ts->command;
    for (int i = 0; i < cmd_argc; i++) {
        size_t len = strlen(ts->argv[i]);
        ptr = (char*) memcpy(ptr, ts->argv[i], len + 1) + len;
        if (i != cmd_argc - 1) {
            *ptr++ = ' ';
        }
    }
    *ptr = '\0'; // null terminator

    //ts->loop = (uv_loop_t*) xmalloc(sizeof *ts->loop);
    //uv_loop_init(ts->loop);

    return ts;
}

static void server_free(struct server *ts) {
    
    SInfo << "server_free";
            
    if (ts == NULL) return;
    if (ts->credential != NULL) free(ts->credential);
    if (ts->auth_header != NULL) free(ts->auth_header);
    if (ts->index != NULL) free(ts->index);
    if (ts->cwd != NULL) free(ts->cwd);
    free(ts->command);
//    free(ts->prefs_json);

    char **p = ts->argv;
    for (; *p; p++) free(*p);
    free(ts->argv);

    if (strlen(ts->socket_path) > 0) {
        struct stat st;
        if (!stat(ts->socket_path, &st)) {
            unlink(ts->socket_path);
        }
    }

    uv_loop_close(ts->loop);

    free(ts->loop);
    free(ts);
}

static void signal_cb(uv_signal_t *watcher, int signum) {
    char sig_name[20];

    switch (watcher->signum) {
        case SIGINT:
        case SIGTERM:
            get_sig_name(watcher->signum, sig_name, sizeof (sig_name));
            printf("received signal: %s (%d), exiting...\n", sig_name, watcher->signum);
            break;
        default:
            signal(SIGABRT, SIG_DFL);
            abort();
    }

    if (force_exit) exit(EXIT_FAILURE);
    force_exit = true;

    //lws_cancel_service(context); arvind

    uv_stop(server->loop);

    printf("send ^C to force exit.\n");
}

static int parse_int(char *name, char *str) {
    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 0);
    if (errno != 0 || endptr == str) {
        fprintf(stderr, "ttyd: invalid value for %s: %s\n", name, str);
        exit(EXIT_FAILURE);
    }
    return (int) val;
}

static int calc_command_start(int argc, char **argv) {
    // make a copy of argc and argv
    int argc_copy = argc;
    char **argv_copy = (char **) xmalloc(sizeof (char *) * argc);
    for (int i = 0; i < argc; i++) {
        argv_copy[i] = strdup(argv[i]);
    }

    // do not print error message for invalid option
    opterr = 0;
    while (getopt_long(argc_copy, argv_copy, opt_string, options, NULL) != -1)
        ;

    int start = argc;
    if (optind < argc) {
        char *command = argv_copy[optind];
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], command) == 0) {
                start = i;
                break;
            }
        }
    }

    // free argv copy
    for (int i = 0; i < argc; i++) {
        free(argv_copy[i]);
    }
    free(argv_copy);

    // reset for next use
    opterr = 1;
    optind = 0;

    return start;
}

class testwebscoket : public net::HttpServer {
public:

    testwebscoket(std::string ip, int port, ServerConnectionFactory *factory = nullptr, bool multithreaded = false) : net::HttpServer(ip, port, factory, multithreaded) {

    }

    void on_wsread(Listener* connection, const char* msg, size_t len) {

        //connection->send("arvind", 6 );
        SInfo << "msg " << std::string(msg, len);
        WebSocketConnection *con = (WebSocketConnection*) connection;

        //con->send( msg, len );

        // sendAll( msg, len );
        
        struct pss_tty *pss;
        
        if(!con->user)
        {
            SError << "not possible state";
            exit (0);
        }else
        {
            pss = (pss_tty*)con->user;
        }
        
        SInfo << "on_wsread" << pss;

        if (pss->buffer == NULL) {
            pss->buffer = (char *) xmalloc(len);
            pss->len = len;
            memcpy(pss->buffer, msg, len);
        } else {
            pss->buffer = (char *) xrealloc(pss->buffer, pss->len + len);
            memcpy(pss->buffer + pss->len, msg, len);
            pss->len += len;
        }

        const char command = pss->buffer[0];

        // check auth
        if (server->credential != NULL && !pss->authenticated && command != JSON_DATA) {
            printf("WS client not authenticated\n");
            return;
        }

        // check if there are more fragmented messages
        //      if (lws_remaining_packet_payload(wsi) > 0 || !lws_is_final_fragment(wsi)) {
        //        return ;
        //      }

        switch (command) {
            case INPUT:
            {
                if (!server->writable) break;
                int err = pty_write(pss->process, pty_buf_init(pss->buffer + 1, pss->len - 1));
                if (err) {
                    printf("uv_write: %s (%s)\n", uv_err_name(err), uv_strerror(err));
                    return;
                }
                break;
            }
            case RESIZE_TERMINAL:
            {
                if (pss->process == NULL) break;
                     parse_window_size(pss->buffer + 1, pss->len - 1, &pss->process->columns, &pss->process->rows);
                pty_resize(pss->process);
                break;
            }
            case PAUSE:
            {
                pty_pause(pss->process);
                break;
            }
            case RESUME:
            {
                pty_resume(pss->process);
                break;
            }
            case JSON_DATA:
            {
                if (pss->process != NULL) break;
                uint16_t columns = 0;
                uint16_t rows = 0;
                json obj = parse_window_size(pss->buffer, pss->len, &columns, &rows);
//                if (server->credential != NULL) {
//                    struct json_object *o = NULL;
//                    if (json_object_object_get_ex(obj, "AuthToken", &o)) {
//                        const char *token = json_object_get_string(o);
//                        if (token != NULL && !strcmp(token, server->credential))
//                            pss->authenticated = true;
//                        else
//                            printf("WS authentication failed with token: %s\n", token);
//                    }
//                    if (!pss->authenticated) {
//                        json_object_put(obj);
//                        lws_close_reason(connection, LWS_CLOSE_STATUS_POLICY_VIOLATION);
//                        return;
//                    }
//                }
               // json_object_put(obj);
                if (!spawn_process(pss, columns, rows)) return;
                break;
            }
            default:
            {
                printf("ignored unknown message type: %c\n", command);
                break;
            }
        }

        if (pss->buffer != NULL) {
            free(pss->buffer);
            pss->buffer = NULL;
        }



    }

    void on_wsclose(Listener* connection) {

        
        WebSocketConnection *con = (WebSocketConnection*) connection;
         

        struct pss_tty *pss;
        
        
        if(!con->user)
        {
            SError << "not possible state";
            exit (0);
        }else
        {
            pss = (pss_tty*)con->user;
        }
        
        SInfo << "on_wsclose" << pss;
        
        
        if (pss->con == NULL) return;

        server->client_count--;
        printf("WS closed from %s, clients: %d\n", pss->address, server->client_count);
        if (pss->buffer != NULL) free(pss->buffer);
        if (pss->pty_buf != NULL) pty_buf_free(pss->pty_buf);
        for (int i = 0; i < pss->argc; i++) {
            free(pss->args[i]);
        }

        if (pss->process != NULL) {
            ((pty_ctx_t *) pss->process->ctx)->ws_closed = true;
            if (process_running(pss->process)) {
                pty_pause(pss->process);
                printf("killing process, pid: %d\n", pss->process->pid);
                pty_kill(pss->process, server->sig_code);
            }
        }

        if ((server->once || server->exit_no_conn) && server->client_count == 0) {
            printf("exiting due to the --once/--exit-no-conn option.\n");
            force_exit = true;
            // lws_cancel_service(context);
            exit(0);
        }
        
        free(pss);
        con->user = nullptr;
        
    }

    void on_wsconnect(Listener* connection) {

       
        
        WebSocketConnection *con = (WebSocketConnection*) connection;
       
        struct pss_tty *pss;
        
        if(!con->user)
        {
            pss = (struct pss_tty *) malloc(sizeof (struct pss_tty));
            
            memset(pss, 0, sizeof(struct pss_tty));
            con->user = pss;
            
        }else
        {
            pss = (pss_tty*)con->user;
	    SError << "not possible state";
            exit (0);	
        }
        
        
        SInfo << "on_wsconnect" << pss;
         

        pss->initialized = false;
        pss->authenticated = false;
        pss->con = connection;
        pss->lws_close_status = LWS_CLOSE_STATUS_NOSTATUS;

        if (server->url_arg) {
            //        while (lws_hdr_copy_fragment(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_URI_ARGS, n++) > 0) {
            //        if (strncmp(buf, "arg=", 4) == 0) {
            //          pss->args = (char**)xrealloc(pss->args, (pss->argc + 1) * sizeof(char *));
            //          pss->args[pss->argc] = strdup(&buf[4]);
            //          pss->argc++;
            //        }
            //        }
        }

        server->client_count++;

    }



    //    

};

int main(int argc, char **argv) {
    if (argc == 1) {
        print_help();
        return 0;
    }
#ifdef _WIN32
    if (!conpty_init()) {
        fprintf(stderr, "ERROR: ConPTY init failed! Make sure you are on Windows 10 1809 or later.");
        return 1;
    }
#endif


    ConsoleChannel *ch = new ConsoleChannel("debug", Level::Info);
    Logger::instance().add(ch);



    int start = calc_command_start(argc, argv);
    server = server_new(argc, argv, start);

    Application app;

    server->loop = app.uvGetLoop();


    StreamingResponderFactory *stream = new StreamingResponderFactory();


    testwebscoket *socket = new testwebscoket("0.0.0.0", 8000, stream, false);



    //  struct lws_context_creation_info info;
    //  memset(&info, 0, sizeof(info));
    //  info.port = 7681;
    //  info.iface = NULL;
    //  info.protocols = protocols;
    //  info.gid = -1;
    //  info.uid = -1;
    //  info.max_http_header_pool = 16;
    //  info.options = LWS_SERVER_OPTION_LIBUV | LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_DISABLE_IPV6;
    //#ifndef LWS_WITHOUT_EXTENSIONS
    //  info.extensions = extensions;
    //#endif
    //  info.max_http_header_data = 65535;

    //arvind

    //int debug_level =   LLL_NOTICE ;//LLL_ERR | LLL_WARN | LLL_NOTICE;
    char iface[128] = "";
    char socket_owner[128] = "";
    bool browser = false;
    bool ssl = false;
    char cert_path[1024] = "";
    char key_path[1024] = "";
    char ca_path[1024] = "";

    json client_prefs = json::object();
    
       

#ifdef _WIN32
    json_object_object_add(client_prefs, "isWindows", json_object_new_boolean(true));
#endif

    // parse command line options
    int c;
    while ((c = getopt_long(start, argv, opt_string, options, NULL)) != -1) {
        switch (c) {
            case 'h':
                print_help();
                return 0;
            case 'v':
                printf("ttyd version %s\n", TTYD_VERSION);
                return 0;
            case 'd':
                // debug_level = parse_int("debug", optarg);
                break;
            case 'a':
                server->url_arg = true;
                break;
            case 'W':
                server->writable = true;
                break;
            case 'O':
                server->check_origin = true;
                break;
            case 'm':
                server->max_clients = parse_int("max-clients", optarg);
                break;
            case 'o':
                server->once = true;
                break;
            case 'q':
                server->exit_no_conn = true;
                break;
            case 'B':
                browser = true;
                break;
            case 'p':
                //        info.port = parse_int("port", optarg);
                //        if (info.port < 0) {
                //          fprintf(stderr, "ttyd: invalid port: %s\n", optarg);
                //          return -1;
                //        }
                //arvind
                break;
            case 'i':
                strncpy(iface, optarg, sizeof (iface) - 1);
                iface[sizeof (iface) - 1] = '\0';
                break;
            case 'U':
                strncpy(socket_owner, optarg, sizeof (socket_owner) - 1);
                socket_owner[sizeof (socket_owner) - 1] = '\0';
                break;
                //      case 'c':
                //        if (strchr(optarg, ':') == NULL) {
                //          fprintf(stderr, "ttyd: invalid credential, format: username:password\n");
                //          return -1;
                //        }
                //        char b64_text[256];
                //        lws_b64_encode_string(optarg, strlen(optarg), b64_text, sizeof(b64_text));
                //        server->credential = strdup(b64_text);
                //        break;
            case 'H':
                server->auth_header = strdup(optarg);
                break;
            case 'u':
                // info.uid = parse_int("uid", optarg);
                break;
                //      case 'g':
                //        info.gid = parse_int("gid", optarg);
                //        break;
            case 's':
            {
                int sig = get_sig(optarg);
                if (sig > 0) {
                    server->sig_code = sig;
                    get_sig_name(sig, server->sig_name, sizeof (server->sig_name));
                } else {
                    fprintf(stderr, "ttyd: invalid signal: %s\n", optarg);
                    return -1;
                }
            }
            break;
            case 'w':
                server->cwd = strdup(optarg);
                break;
            case 'I':
                if (!strncmp(optarg, "~/", 2)) {
                    const char *home = getenv("HOME");
                    server->index = (char*) malloc(strlen(home) + strlen(optarg) - 1);
                    sprintf(server->index, "%s%s", home, optarg + 1);
                } else {
                    server->index = strdup(optarg);
                }
                struct stat st;
                if (stat(server->index, &st) == -1) {
                    fprintf(stderr, "Can not stat index.html: %s, error: %s\n", server->index, strerror(errno));
                    return -1;
                }
                if (S_ISDIR(st.st_mode)) {
                    fprintf(stderr, "Invalid index.html path: %s, is it a dir?\n", server->index);
                    return -1;
                }
                break;
            case 'b':
            {
//                char path[128];
//                strncpy(path, optarg, 128);
//                size_t len = strlen(path);
//                while (len && path[len - 1] == '/') path[--len] = 0; // trim trailing /
//                if (!len) break;
//#define sc(f)                                  \
//  strncpy(path + len, endpoints.f, 128 - len); \
//  endpoints.f = strdup(path);
//                sc(ws) sc(index) sc(token) sc(parent)
//#undef sc
            }
            break;
#if LWS_LIBRARY_VERSION_NUMBER >= 4000000
            case 'P':
            {
                int interval = parse_int("ping-interval", optarg);
                if (interval < 0) {
                    fprintf(stderr, "ttyd: invalid ping interval: %s\n", optarg);
                    return -1;
                }
                retry.secs_since_valid_ping = interval;
                retry.secs_since_valid_hangup = interval + 7;
            }
            break;
#endif
                //      case 'f': {
                //        int serv_buf_size = parse_int("srv-buf-size", optarg);
                //        if (serv_buf_size < 0) {
                //          fprintf(stderr, "ttyd: invalid srv-buf-size: %s\n", optarg);
                //          return -1;
                //        }
                //        info.pt_serv_buf_size = serv_buf_size;
                //      } break;
            case '6':
                //        info.options &= ~(LWS_SERVER_OPTION_DISABLE_IPV6);
                break;
#if defined(LWS_OPENSSL_SUPPORT) || defined(LWS_WITH_TLS)
            case 'S':
                ssl = true;
                break;
            case 'C':
                strncpy(cert_path, optarg, sizeof (cert_path) - 1);
                cert_path[sizeof (cert_path) - 1] = '\0';
                break;
            case 'K':
                strncpy(key_path, optarg, sizeof (key_path) - 1);
                key_path[sizeof (key_path) - 1] = '\0';
                break;
            case 'A':
                strncpy(ca_path, optarg, sizeof (ca_path) - 1);
                ca_path[sizeof (ca_path) - 1] = '\0';
                break;
#endif
            case 'T':
                strncpy(server->terminal_type, optarg, sizeof (server->terminal_type) - 1);
                server->terminal_type[sizeof (server->terminal_type) - 1] = '\0';
                break;
            case '?':
                break;
//            case 't':
//                optind--;
//                for (; optind < start && *argv[optind] != '-'; optind++) {
//                    char *option = optarg;
//                    char *key = strsep(&option, "=");
//                    if (key == NULL) {
//                        fprintf(stderr, "ttyd: invalid client option: %s, format: key=value\n", optarg);
//                        return -1;
//                    }
//                    char *value = strsep(&option, "=");
//                    if (value == NULL) {
//                        fprintf(stderr, "ttyd: invalid client option: %s, format: key=value\n", optarg);
//                        return -1;
//                    }
//                    json obj = json::parse(value);
//                    json_object_object_add(client_prefs, key, obj != NULL ? obj : json_object_new_string(value));
//                }
//                break;
            default:
                print_help();
                return -1;
        }
    }
    
    std::string tmp = client_prefs.dump();
            
    server->prefs_json = tmp;
   // json_object_put(client_prefs);

    if (server->command == NULL || strlen(server->command) == 0) {
        fprintf(stderr, "ttyd: missing start command\n");
        return -1;
    }

    //  lws_set_log_level(debug_level, NULL);

    char server_hdr[128] = "";
    sprintf(server_hdr, "ttyd/%s /%s)", TTYD_VERSION, "3.2.3");
    //  info.server_string = server_hdr; //arvind




    printf("ttyd %s)\n", TTYD_VERSION );
    print_config();

    // lws custom header requires lower case name, and terminating :
    if (server->auth_header != NULL) {
        size_t auth_header_len = strlen(server->auth_header);
        server->auth_header = (char*) xrealloc(server->auth_header, auth_header_len + 2);
        strcat(server->auth_header + auth_header_len, ":");
        lowercase(server->auth_header);
    }

    void *foreign_loops[1];
    foreign_loops[0] = server->loop;
    //  info.foreign_loops = foreign_loops;
    // info.options |= LWS_SERVER_OPTION_EXPLICIT_VHOSTS;




#define sig_count 2
    int sig_nums[] = {SIGINT, SIGTERM};
    uv_signal_t signals[sig_count];
    for (int i = 0; i < sig_count; i++) {
        uv_signal_init(server->loop, &signals[i]);
        uv_signal_start(&signals[i], signal_cb, sig_nums[i]);
    }





    //   pss->initialized = false;
    //   pss->authenticated = false;
    //      
    //  uint16_t columns =0;
    //  uint16_t rows =0;
    //  
    //   if (!spawn_process(pss, columns, rows)) return 1;
    //  
    //  
    //    pss->process->columns = 300;
    //    pss->process->rows = 75;
    //    
    //    pty_resize(pss->process);
    //    
    //    
    //     if (process_running(pss->process)) {
    //          pty_pause(pss->process);
    //          printf("killing process, pid: %d\n", pss->process->pid);
    //          pty_kill(pss->process, server->sig_code);
    //        }
    //    
    //     if (!spawn_process(pss, columns, rows)) return 1;
    //  
    //  
    //    pss->process->columns = 300;
    //    pss->process->rows = 75;
    //    
    //    pty_resize(pss->process);
    //    
    //    pss->initialized = true;
    //    pty_resume(pss->process);


    //    char in[]="ls\r\t";
    //    int len = strlen(in)+1;
    //    
    //    if (pss->buffer == NULL) {
    //    pss->buffer = xmalloc(len);
    //    pss->len = len;
    //    memcpy(pss->buffer, in, len);
    //    } else {
    //    pss->buffer = xrealloc(pss->buffer, pss->len + len);
    //    memcpy(pss->buffer + pss->len, in, len);
    //    pss->len += len;
    //    }
    //    int err = pty_write(pss->process, pty_buf_init(pss->buffer , pss->len));




    //uv_run(server->loop, UV_RUN_DEFAULT);

   app.waitForShutdown([&](void*)
   {
     
        SInfo << "Main shutdwon1";
        socket->Close();
        socket->shutdown();
        delete socket;

        SInfo << "Main shutdwon";

        delete stream;

        SInfo << "Main shutdwon2";
        
        for (int i = 0; i < sig_count; i++) {
            uv_signal_stop(&signals[i]);
        }
        

        app.stop();
        //app.uvDestroy();
        delete ch;

    }
    
    );

    //  lws_service(context, 0);// arvind

   
#undef sig_count

    //lws_context_destroy(context); // arvind

    // cleanup
    server_free(server);

    return 0;
}



