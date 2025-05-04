#ifndef CONNEC_TRANSPORT_H
#define CONNEC_TRANSPORT_H


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
#include "net/UdpSocket.h"
//#include "base/test.h"
#include "base/time.h"
#include "net/dns.h"


#include "net/TcpServer.h"

#include "base/time.h"
#include "net/netInterface.h"
#include "configuration.h"


#include <Reader.h>
#include <Writer.h>

using std::endl;
using namespace base;
using namespace net;
///using namespace rtc;
//using namespace base::test;

namespace rtc {
    

class testUdpServer: public UdpServer::Listener {
public:

    testUdpServer(std::string IP, int port):IP(IP), port(port) {
    }

    void start() {
        udpServer = new UdpServer( this, IP, port);
        udpServer->bind();
    }


    void shutdown() {

        delete udpServer;
        udpServer = nullptr;

    }
    

    void send( uint8_t* data, uint32_t nbytes, std::string ip, int port );
      
    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr); 

    UdpServer *udpServer;

    std::string IP;
    int port;

};


class tesTcpServer : public Listener {
public:

    tesTcpServer() {
    }

    void start(std::string ip, int port) {
        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, ip, port);

    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpServer;
        tcpServer = nullptr;

    }

    void on_close(Listener* connection) {

        std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void on_read(Listener* connection, const char* data, size_t len); 
    
    TcpServer *tcpServer;

};







class tesTcpClient {
public:

    tesTcpClient() {}

    void start(std::string ip, int port) {

        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpClient = new TcpConnectionBase();

        tcpClient->Connect(ip, port);


    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpClient;
        tcpClient = nullptr;

    }

    void on_close(Listener* connection) {

        std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void sendit( uint8_t* data, size_t len );
    
    void on_read(Listener* connection, const char* data, size_t len) ;
    

   // TcpConnection *tcpClient; // do not use this this is for RTP
    TcpConnectionBase *tcpClient;

};




 class Transport: public GetAddrInfoReq
 {
 public:
        Transport( Configuration &Config): mConfig(Config)
        {
            
        }
        void resolveStunServer();
        void cbDnsResolve(addrinfo* res, std::string ip, int port,  void* ptr) override;
        
        Configuration &mConfig;
     
 };
 
 
}
 
 #endif
