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
#include "net/netInterface.h"
#include "net/SslConnection.h"
// #include "base/test.h"
#include "base/time.h"

//#include "net/sslmanager.h"

using std::endl;
using namespace base;
using namespace net;
// using namespace base::test;


class tesTcpClient : public SslConnection {
public:

    tesTcpClient(): SslConnection()
    {
        Connect("127.0.0.1", 5001);
    }



    void on_close() {

        std::cout << " Close Con LocalIP" << this->GetLocalIp() << " PeerIP" << this->GetPeerIp() << std::endl << std::flush;

    }

    void on_read( const char* data, size_t len) {
        
      std::cout << " on_read " << this->GetLocalIp() << " PeerIP " << this->GetPeerIp() << std::endl << std::flush;

        std::cout << "data: " << data << " len: " << len << std::endl << std::flush;
        std::string send = "12345";
       // connection->send((const char*) send.c_str(), 5);

    }
    
    void on_connect() {
        
       SslConnection::on_connect();
        
       std::cout << " on_read " << this->GetLocalIp() << " PeerIP " << this->GetPeerIp() << std::endl << std::flush;

     
       std::string send = "12345";
       SslConnection::send((const char*) send.c_str(), 5);
       std::cout << "TCP Client send data: " << send << "len: " << strlen((const char*) send.c_str()) << std::endl << std::flush;

    }
    
  

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

      // net::SSLManager::initNoVerifyClient();
       
        Application app;

        tesTcpClient socket;

        app.run();

        




    return 0;
}
