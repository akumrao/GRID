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
#include "base/platform.h"

using std::endl;
using namespace base;
using namespace net;
//using namespace base::test;


class testUdpServer1 : public UdpServer::Listener {
public:

    testUdpServer1(std::string IP, int port):IP(IP), port(port) {
    }

    void start() {
        udpServer = new UdpServer( this, IP, port);
        udpServer->bind();
    }

    void send( std::string txt, std::string ip, int port )
    {
         udpServer->send( (char*) txt.c_str(), txt.length() , ip , port);
    }

    void shutdown() {

        delete udpServer;
        udpServer = nullptr;

    }


    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) {

        int family;
        
        std::string peerIp;
        uint16_t peerPort;

        IP::GetAddressInfo(
                    remoteAddr, family, peerIp, peerPort);
            
        std::cout << "testUdpServer1::OnRecv " << data << " ip " << peerIp << ":" << peerPort  << std::endl << std::flush;
        udpServer->send(data, len, remoteAddr);
    }

    UdpServer *udpServer;

    std::string IP;
    int port;

};


class testUdpServer2 : public UdpServer::Listener {
public:

    testUdpServer2(std::string IP, int port):IP(IP), port(port) {
    }

    void start() {
        udpServer = new UdpServer( this, IP, port);
        udpServer->bind();
    }

    void send( std::string txt, std::string ip, int port )
    {
         udpServer->send( (char*) txt.c_str(), txt.length() , ip , port);
    }




    void shutdown() {

        delete udpServer;
        udpServer = nullptr;

    }


    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) {

        int family;
        
        std::string peerIp;
        uint16_t peerPort;

        IP::GetAddressInfo(
                    remoteAddr, family, peerIp, peerPort);
            
          std::cout << "testUdpServer2::OnRecv " << data << " ip " << peerIp << ":"       << peerPort << std::endl        << std::flush;

        udpServer->send(data, len, remoteAddr);
        
    }

    UdpServer *udpServer;

    std::string IP;
    int port;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

 
        Application app;

        testUdpServer1 socket1("0.0.0.0", 6001);
        //testUdpServer1 socket1("::", 6001);
        socket1.start();

        testUdpServer2 socket2("0.0.0.0", 6002);
        //testUdpServer2 socket1("::", 6002);
        socket2.start();

        base::sleep(660);
        
        socket1.send("arvind1", "127.0.0.1" , 6002);
        socket2.send("arvind2", "127.0.0.1" , 6001);


        std::cout << "waitForShutdown" << std::endl << std::flush;

        app.waitForShutdown([&](void*) {

            socket1.shutdown();
          socket2.shutdown();

        });



    return 0;
}
