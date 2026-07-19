#include <stdbool.h>
#include <uv.h>
#include "ttydpty.h"
#include  "net/netInterface.h"

#include "http/websocket.h"

#include <json/json.hpp>
#include <uv.h>
using json = nlohmann::json;




// client message
#define INPUT '0'
#define RESIZE_TERMINAL '1'
#define PAUSE '2'
#define RESUME '3'
#define JSON_DATA '{'

// server message
#define OUTPUT '0'
#define SET_WINDOW_TITLE '1'
#define SET_PREFERENCES '2'


#define CLOSE_STATUS_NOSTATUS      0
#define CLOSE_STATUS_UNEXPECTED_CONDITION   1011
#define CLOSE_STATUS_POLICY_VIOLATION     1008


 #define WRITE_BINARY  1

// url paths
//struct endpoints {
//  char *ws;
//  char *index;
//  char *token;
//  char *parent;
//};


struct pss_http {
  char path[128];
  char *buffer;
  char *ptr;
  size_t len;
};

class TTYServer;

struct pss_tty {
  bool initialized;
  int initial_cmd_index;
  bool authenticated;
  char user[30];
  char address[50];
  char path[128];
  char **args;
  int argc;

  base::net::WebSocketConnection *con{nullptr};
  
  TTYServer *thisTTYServer{nullptr};
  
  char *buffer{nullptr};
  size_t len{0};

  pty_process *process{nullptr};
  pty_buf_t *pty_buf{nullptr};

  int close_status;
};

typedef struct {
  struct pss_tty *pss;
  bool ws_closed;
} pty_ctx_t;

struct TTYStServer {
  int client_count;        // client count
  char prefs_json[256]; // client preferences
  char *credential;        // encoded basic auth credential
  char *auth_header;       // header name used for auth proxy
  char *index;             // custom index.html
  char *command;           // full command line
  char **argv;             // command with arguments
  int argc;                // command + arguments count
  char *cwd;               // working directory
  int sig_code;            // close signal
  char sig_name[20];       // human readable signal string
  bool url_arg;            // allow client to send cli arguments in URL
  bool writable;           // whether clients to write to the TTY
  bool check_origin;       // whether allow websocket connection from different origin
  int max_clients;         // maximum clients to support
  bool once;               // whether accept only one client and exit on disconnection
  bool exit_no_conn;       // whether exit on all clients disconnection
  char socket_path[255];   // UNIX domain socket path
  char terminal_type[30];  // terminal type to report

  uv_loop_t *loop;         // the libuv event loop
};


class TTYServer

{

public:
  
volatile bool force_exit;
struct lws_context *context;
//struct server *server;
void print_help();
void server_new(int argc, char **argv, int start) ;
void server_free();
int server_init( uv_loop_t *loop, int argc, char **argv );
void print_config() ;
//extern struct endpoints endpoints;


json parse_window_size(const char *buf, size_t len, uint16_t *cols, uint16_t *rows);

int lws_close_reason( base::net::Listener* conn, uint16_t statusCode ) ;

bool spawn_process(struct pss_tty *pss, uint16_t columns, uint16_t rows);







void server_wsread(void *user,  const char* msg, size_t len) ;
void server_wsclose(void **user);
struct pss_tty* server_wsconnect(void **user);








 int send_initial_message(struct pss_tty *pss, int index) ;
 char ** build_env(struct pss_tty *pss) ;
 char ** build_args(struct pss_tty *pss);

//void close_reason(base::net::Listener* conn, uint16_t statusCode  );
//
//
//int callback_on_writable(	base::net::Listener* conn);




TTYStServer *server{nullptr};

};