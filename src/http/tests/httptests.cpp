/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 

  https://github.com/creytiv/re
  https://en.wikipedia.org/wiki/UDP_hole_punching 
  cricket::Candidate represents an address discovered by a cricket::Port. A candidate can be local (i.e discovered by a local port) or remote. Remote candidates are transported using signaling, i.e outside of webrtc. There are 4 types of candidates: local, stun, prflx or relay (standard)
  https://github.com/clibs/net/blob/master/src/net.c
  https://github.com/geminiwen/libuv-mbedtls/blob/master/src/uv_tls.c
  
// good
  https://github.com/deleisha/libuv-tls/blob/master/test_tls_client.c
  
  https://github.com/geminiwen/libuv-mbedtls/blob/master/README.md
 
  
  
 */

#include "http/HTTPResponder.h"
#include "http/HttpServer.h"
//#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/certificate.h"

using namespace base;
using namespace base::net;
//using namespace base::test;


#if HTTPSSL
extern ConfCert config;

class testwebscoket: public net::HttpsServer 
{
public:
    
     testwebscoket( std::string ip, int port, ServerConnectionFactory *factory = nullptr,  bool multithreaded =false) : net::HttpsServer(  ip, port,  factory, multithreaded)
     {
         
     }
#else
class testwebscoket: public net::HttpServer 
{
public:
    
     testwebscoket( std::string ip, int port, ServerConnectionFactory *factory = nullptr,  bool multithreaded =false) : net::HttpServer(  ip, port,  factory, multithreaded)
     {
         
     }     
     
#endif
     
    void on_wsread(Listener* connection, const char* msg, size_t len) {
      
        //connection->send("arvind", 6 );
        SInfo << "msg " << std::string(msg,len);
        WebSocketConnection *con = (WebSocketConnection*)connection;
        
        //con->send( msg, len );
        
        sendAll( msg, len );
        
    }
    
    void sendAll(const char* msg, size_t len) {
      
        
        SInfo << "No of Connectons " << this->GetNumConnections();
        
        for (auto* connection :  this->GetConnections())
        {
            
#if HTTPSSL
                    
             WebSocketConnection *con = ((HttpsConnection*)connection)->getWebSocketCon();
#else
             WebSocketConnection *con = ((HttpConnection*)connection)->getWebSocketCon();
#endif
             if(con)
             con->send(msg ,len );
//             else
//             {
//                WebSocketConnection *con = ((HttpsConnection*)connection)->getWebSocketCon();
//                if(con)
//                con->send(msg ,len );
//             }
        }
         
    }
    
    
  
    
};

int main(int argc, char** argv) {

   ConsoleChannel *ch =  new ConsoleChannel("debug", Level::Trace);
            
   Logger::instance().add(ch);
    //test::init();
   
   #if HTTPSSL
    config.certificatePemFile = "/var/tmp/certificate.crt";
    config.keyPemFile = "/var/tmp/private.key" ;
   
    
   SInfo << "https://localhost:8000";
   
   #else
    SInfo << "http://localhost:8000";
   #endif
    
   StreamingResponderFactory *stream =   new StreamingResponderFactory();
            
   Application app;
   testwebscoket  *socket = new testwebscoket("0.0.0.0", 8000, stream , false  );
    //socket.start();

   app.waitForShutdown([&](void*)
   {
     
        SInfo << "Main shutdwon1";
        socket->Close();
        socket->shutdown();
        delete socket;

        SInfo << "Main shutdwon";

        delete stream;

        SInfo << "Main shutdwon2";

        app.stop();
        //app.uvDestroy();
        delete ch;

    }
    
    );


/*
 
for numbe of file descriptor  
lsof -p `pidof runHttp` 

*/ 
    

/*Leak test  without multithreaded server
pmap -x 18321
    Total kB          322044    6112    1084
  
    Total kB          322044    6112    1084   RSS /nerver goes above 6112
 
 
 
 ==19630== LEAK SUMMARY:
==19630==    definitely lost: 0 bytes in 0 blocks
==19630==    indirectly lost: 0 bytes in 0 blocks
==19630==      possibly lost: 1,152 bytes in 4 blocks
==19630==    still reachable: 5,138 bytes in 31 blocks
==19630==         suppressed: 0 bytes in 0 blocks

  
 */  
    
/*
valgrind --leak-check=full   --show-leak-kinds=all  --track-origins=yes   ./runHttp      
valgrind --leak-check=full   --show-leak-kinds=all  --track-origins=yes  --verbose 

  total kB          469284    6244    1216
EAK SUMMARY:
==25134==    definitely lost: 0 bytes in 0 blocks
==25134==    indirectly lost: 0 bytes in 0 blocks
==25134==      possibly lost: 1,728 bytes in 6 blocks
==25134==    still reachable: 11,326 bytes in 49 blocks
==25134==         suppressed: 0 bytes in 0 blocks

*/



    return 0;

    //  test::runAll();

    // return test::finalize();
}
