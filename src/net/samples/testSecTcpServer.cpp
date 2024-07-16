/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
// #include "base/test.h"
#include "base/time.h"
#include "net/SslConnection.h"

///ssl
//#include "net/sslmanager.h"

using std::endl;
using namespace base;
using namespace net;
// using namespace base::test;


class testSslCon :  public SslConnection{
public:

    testSslCon(bool server): SslConnection(true )
    {
    }


    void on_close( ) {

        std::cout << "TCP server closing, LocalIP" << this->GetLocalIp() << " PeerIP" << this->GetPeerIp() << std::endl << std::flush;

    }

    
    void on_read(const char* data, size_t len) {
        std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        SslConnection::send((const char*) send.c_str(), 5);

    }

};


static constexpr size_t MaxTcpConnectionsPerServer{ 4000};
  
class SecTcpServer : public TcpServerBase
{
public:


public:
    SecTcpServer(Listener* listener, std::string ip, int port, bool multiThreaded=false, bool ssl=false ): TcpServerBase(BindTcp(ip, port), 256, multiThreaded), listener(listener),ssl(ssl){

        }
    ~SecTcpServer() {

            if (uvHandle)
                delete uvHandle;
            //UnbindTcp(this->localIp, this->localPort); // please do not do it here . Drive your own class from TServerBase.
        }
    /* Pure virtual methods inherited from ::TcpServer. */
public:
    void UserOnTcpConnectionAlloc(TcpConnectionBase** connection) {


             *connection = new testSslCon( true);
         
        }
    
    bool UserOnNewTcpConnection(TcpConnectionBase* connection) 
    {
    
            if (GetNumConnections() >= MaxTcpConnectionsPerServer)
            {
                LError("cannot handle more than %zu connections", MaxTcpConnectionsPerServer);

                return false;
            }

            return true;
    }
    
    void UserOnTcpConnectionClosed(TcpConnectionBase* connection)
    {
        
    }

private:
    // Passed by argument.
    Listener* listener{ nullptr};
    uv_tcp_t* uvHandle{ nullptr};
    bool ssl;
};






int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

 
     //  net::SSLManager::initNoVerifyServer();

        Application app;
        SecTcpServer *tcpServer = new SecTcpServer(nullptr, "0.0.0.0", 5001, false, true);
   

        app.waitForShutdown([&](void*) {

           // socket.shutdown();

        });



    return 0;
}
