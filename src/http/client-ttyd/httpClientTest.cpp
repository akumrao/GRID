/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "net/netInterface.h"
#include "httpClientTest.h"
#include "http/client.h"

#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"

#include "http/url.h"
//#include "http/util.h"
#include "base/filesystem.h"
#include "http/HttpClient.h"
#include "http/HttpsClient.h"
//#include "crypto/hash.h"
#include "base/platform.h"

#include "http/form.h"

#include "base/tty.h"

using namespace base;
using namespace base::net;


#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 8000 //443

#define OUTPUT '0'
#define SET_WINDOW_TITLE '1'
#define SET_PREFERENCES '2'


ClientConnecton *m_client = nullptr;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        // Process the received character(s)
        //printf("Received: %s\n", buf->base);
        
        
        char tmp[30]={'0'};
        
      //sprintf(tmp,"%c%s", '0', buf->base );

        memcpy(&tmp[1],buf->base, nread );
        m_client->send(tmp, nread+1, false);
        
        // If you want to stop reading after one character:
        // uv_read_stop(stream);
        // uv_close((uv_handle_t*) stream, NULL);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
        }
        uv_close((uv_handle_t*) stream, NULL);
    }
    // Free the buffer if it was allocated by the allocator callback
    if (buf->base) {
        free(buf->base);
    }
}


uv_signal_t sigint_watcher;

void on_signal(uv_signal_t *handle, int signum) {
    fprintf(stderr, "Caught signal %d, stopping loop...\n", signum);
    uv_stop(handle->loop);
    uv_close((uv_handle_t*)handle, NULL); // Close the signal watcher
}


int main(int argc, char** argv) {

    //Logger::instance().add(new RemoteChannel("Remote", Level::Remote, "127.0.0.1", 6000));

      Logger::instance().add(new RotatingFileChannel("tty", "/tmp/tty.txt", Level::Trace, "log", 100));
    
      
      
 {
       
        
        Application app;
        
      
        uv_signal_init(app.uvGetLoop(), &sigint_watcher);
        uv_signal_start(&sigint_watcher, on_signal, SIGINT);
    
    
        TTY rtc([m_client]( char* buf , int nread) {
            
        char tmp[30] = {'0'};
        memcpy(&tmp[1], buf, nread);
        
        //sprintf(tmp,"%c%s", '0', buf );
        
        //rtcthread->ProcessMessages(3);
       // LTrace(" rtcthread->ProcessMessages")
       // base::sleep(1000);
               
           
//        m_client->send(tmp, nread+1, false);
            
               int x = 1;
         });

//      //bool ssl = false;
//        std::ostringstream url;
//        bool ssl = false;
//        std::string host = SERVER_HOST;
//        int port = SERVER_PORT;
//
//        url << "/";
//
//
//        if(!ssl)
//        {
//            m_client = new HttpClient("ws", host, port, url.str());
//        }
//        else
//        {
//             m_client = new HttpsClient("wss", host, port, url.str());
//        }
//
//        // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
//        m_client->fnComplete = [&](const Response & response) {
//            std::string reason = response.getReason();
//            StatusCode statuscode = response.getStatus();
//            std::string body = m_client->readStream() ? m_client->readStream()->str():"";
//            STrace << "SocketIO handshake response:" << "Reason: " << reason << " Response: " << body;
//        };
//        
//        m_client->fnConnect = [&](HttpBase * con ) {
//            
//            STrace << "client->fnConnect "  ;
//          //  m_con_state = con_opened;
//           // m_reconn_timer.Stop();
//            
//            char tmp[3] = "{}";
//            
//            con->send(tmp, 2);
//         };
//        
//
//        m_client->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
//            STrace << "client->fnPayload " << std::string(data,sz) ;
//
//            if(sz > 0)
//            {
//                const char command = data[0];
//                
//                
//                switch (command) {
//                case OUTPUT:
//                {
//                    printf("%.*s",sz, &data[1] );
//                    fflush(stdout);
//                    break;
//                }
//                case SET_WINDOW_TITLE:
//                {
//                   
//                    break;
//                }
//                case SET_PREFERENCES:
//                {
//              
//                    break;
//                }
//                default:
//                {
//                    printf("ignored unknown message type: %c\n", command);
//                    break;
//                }
//                
//            };
//            
//        }
//           
//        };
//        
//        m_client->fnClose = [&](HttpBase * con, std::string str) {
//            STrace << "client->fnClose " << str ;
//           // close(0,"exit");
//            //on_close();
////            delete m_client;
////            m_client = nullptr;
////            m_con_state = con_closed;
//        };
//        
//        //  conn->_request.setKeepAlive(false);
//        m_client->setReadStream(new std::stringstream);
//        m_client->send();
//        LTrace("sendHandshakeRequest over")
//






















        app.run();
        
       
          
    }


    return 0;

}
