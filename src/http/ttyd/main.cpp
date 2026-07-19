#include "src/server.h"

// #include <errno.h>
// //#include <getopt.h>
// //#include <json.h>
 #include <signal.h>
// #include <stdbool.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include "src/ttydutils.h"

#include "base/filesystem.h"
#include "net/certificate.h"

#include "http/HTTPResponder.h"
//#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

using namespace base;
using namespace base::net;
//using namespace base::test;


// #ifndef TTYD_VERSION
// #define TTYD_VERSION "unknown"
// #endif

using namespace base;
using namespace base::net;
//using namespace base::test;


#ifndef TTYD_VERSION
#define TTYD_VERSION "unknown"
#endif


volatile bool force_exit = false;



TTYServer  ttyServer;

static void signal_cb(uv_signal_t *handle, int signum) {
    char sig_name[20];

    switch (handle->signum) {
        case SIGINT:
        case SIGTERM:
            get_sig_name(handle->signum, sig_name, sizeof (sig_name));
            printf("received signal: %s (%d), exiting...\n", sig_name, handle->signum);
            break;
        default:
            signal(SIGABRT, SIG_DFL);
            abort();
    }

    if (force_exit) exit(EXIT_FAILURE);
    force_exit = true;

    //lws_cancel_service(context); arvind
    uv_signal_stop(handle);
    uv_stop(handle->loop);

    printf("send ^C to force exit.\n");
}



 #if HTTPSSL
           class testwebscoket : public net::HttpsServer {
        public:
    testwebscoket(std::string ip, int port, ServerConnectionFactory *factory = nullptr, bool multithreaded = false) : net::HttpsServer(ip, port, factory, multithreaded) {}
       
 #else


class testwebscoket : public net::HttpServer {
public:
   testwebscoket(std::string ip, int port, ServerConnectionFactory *factory = nullptr, bool multithreaded = false) : net::HttpServer(ip, port, factory, multithreaded) {}
  

#endif


    void on_wsread(Listener* connection, const char* msg, size_t len) {
         
      WebSocketConnection *con = (WebSocketConnection*) connection;
      ttyServer.server_wsread(con->user, msg, len);
    }

    void on_wsclose(Listener* connection) {

        
        WebSocketConnection *con = (WebSocketConnection*) connection;
         

        ttyServer.server_wsclose(&con->user);
        
    }

    void on_wsconnect(Listener* connection) {

       
        
      WebSocketConnection *con = (WebSocketConnection*) connection;
        
      struct pss_tty *pss = ttyServer.server_wsconnect(&con->user );
      if(pss)
      {
        pss->con =con;
      }

    }



    //    

};

int main(int argc, char **argv) {

    if (argc == 1 ||  argc >  2 ) {
        ttyServer.print_help();
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


    
    Application app;
      



 if( ttyServer.server_init(  app.uvGetLoop(),  argc, argv))
 {
    return -1;
 }

  
  

    
#define sig_count 2
    int sig_nums[] = {SIGINT, SIGTERM};
    uv_signal_t signals[sig_count];
    for (int i = 0; i < sig_count; i++) {
        uv_signal_init(app.uvGetLoop(), &signals[i]);
        uv_signal_start(&signals[i], signal_cb, sig_nums[i]);
    }



   
    StreamingResponderFactory *stream = new StreamingResponderFactory();
    testwebscoket *socket = new testwebscoket("0.0.0.0", 8000, stream, false);




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
    ttyServer.server_free();

    return 0;
}



