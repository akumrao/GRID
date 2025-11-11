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



//uv_signal_t sigint_watcher;
//
//void on_signal(uv_signal_t *handle, int signum) {
//    fprintf(stderr, "Caught signal %d, stopping loop...\n", signum);
//    uv_stop(handle->loop);
//    uv_close((uv_handle_t*)handle, NULL); // Close the signal watcher
//}


void set_terminal_title(const char *title) {
    // Escape sequence to set the window title: ESC]0;TitleBEL
    // \033 is the escape character, ]0; means set title and icon name, \007 is the bell character
    printf("\033]0;%s\007", title);
    fflush(stdout); // Flush stdout to ensure the title is set immediately
}


int main(int argc, char** argv) {

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace) );

    //Logger::instance().add(new RotatingFileChannel("tty", "/tmp/tty.txt", Level::Trace, "log", 100));
    
      
      
    {
       
        
        Application app;
        
      
      //  set_terminal_title("My C Program Title");
                
       // uv_signal_init(app.uvGetLoop(), &sigint_watcher);
       // uv_signal_start(&sigint_watcher, on_signal, SIGINT);
    
    
        TTY rtc([m_client]( char* buf , int nread) 
        {
            
            char tmp[30] = {'0'};
            memcpy(&tmp[1], buf, nread);
 
            if( !nread && !buf &&  m_client)
            {
                m_client->Close();
                delete m_client;
                m_client = nullptr;
            }
            else
                m_client->send(tmp, nread+1, false);
            
         });

      //bool ssl = false;
        std::ostringstream url;
        bool ssl = false;
        std::string host = SERVER_HOST;
        int port = SERVER_PORT;

        url << "/";


        if(!ssl)
        {
            m_client = new HttpClient("ws", host, port, url.str());
        }
        else
        {
             m_client = new HttpsClient("wss", host, port, url.str());
        }

        // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
        m_client->fnComplete = [&](const Response & response) {
            std::string reason = response.getReason();
            StatusCode statuscode = response.getStatus();
            std::string body = m_client->readStream() ? m_client->readStream()->str():"";
            STrace << "SocketIO handshake response:" << "Reason: " << reason << " Response: " << body;
        };
        
        m_client->fnConnect = [&](HttpBase * con ) {
            
            STrace << "client->fnConnect "  ;
          //  m_con_state = con_opened;
           // m_reconn_timer.Stop();
            
            char tmp[3] = "{}";
            
            con->send(tmp, 2);
         };
        

        m_client->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
            STrace << "client->fnPayload " << std::string(data,sz) ;

            if(sz > 0)
            {
                const char command = data[0];
                
                
                switch (command) {
                case OUTPUT:
                {
                    printf("%.*s",sz, &data[1] );
                    fflush(stdout);
                    break;
                }
                case SET_WINDOW_TITLE:
                {   
                   // printf("%.*s",sz, &data[1] );
                    set_terminal_title( &data[1] );
                    fflush(stdout);
                   
                    break;
                }
                case SET_PREFERENCES:
                {
                     
                    break;
                }
                default:
                {
                    printf("ignored unknown message type: %c\n", command);
                    break;
                }
                
            };
            
        }
           
        };
        
        m_client->fnClose = [&](HttpBase * con, std::string str) {
            STrace << "client->fnClose " << str ;
           // close(0,"exit");
             //on_close();
            
            rtc.stop();
            m_client->Close();
            delete m_client;
            m_client = nullptr; 
             
            
//            m_con_state = con_closed;
        };
        
        //  conn->_request.setKeepAlive(false);
        m_client->setReadStream(new std::stringstream);
        m_client->send();
        LTrace("sendHandshakeRequest over")




        app.run();
        
        set_terminal_title("");
       
          
    }


    return 0;

}
