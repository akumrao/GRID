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
#include "base/time.h"
#include "base/platform.h"
#include <string> 
#include <iostream>

// Include your local timer header file
 #include "base/Timer.h" 

using std::endl;
using namespace base;
using namespace net;

class testUdpServer1 : public UdpServer::Listener {
public:
    testUdpServer1(std::string IP, int port) : IP(IP), port(port) {}

    void start() {
        udpServer = new UdpServer(this, IP, port);
        udpServer->bind();
    }

    void send(std::string txt, std::string ip, int port) {
        if (udpServer) {
            udpServer->send((char*) txt.c_str(), txt.length(), ip, port);
        }
    }

    void shutdown() {
        if (udpServer) {
            delete udpServer;
            udpServer = nullptr;
        }
    }

    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, struct sockaddr* remoteAddr) {
        int family;
        std::string peerIp;
        uint16_t peerPort;
        IP::GetAddressInfo(remoteAddr, family, peerIp, peerPort);
        std::cout << " OnUdpSocketPacketReceived " << peerIp << ":" << peerPort;

        addr_record_t remotesrc;
        IP::CopyAddress(remoteAddr, remotesrc);

#ifdef __linux__
        IP::addr_unmap_inet6_v4mapped((struct sockaddr *) &remotesrc.addr, &remotesrc.len);
#endif
        std::cout << "testUdpServer1::OnRecv " << data << " ip " << peerIp << ":" << peerPort << std::endl << std::flush;
        udpServer->send(data, len, (struct sockaddr *) &remotesrc.addr);
    }

    UdpServer *udpServer{nullptr};
    std::string IP;
    int port;
};

class testUdpServer2 : public UdpServer::Listener {
public:
    testUdpServer2(std::string IP, int port) : IP(IP), port(port) {}

    void start() {
        udpServer = new UdpServer(this, IP, port);
        udpServer->bind();
    }

    void send(std::string txt, std::string ip, int port) {
        if (udpServer) {
            udpServer->send((char*) txt.c_str(), txt.length(), ip, port);
        }
    }

    void shutdown() {
        if (udpServer) {
            delete udpServer;
            udpServer = nullptr;
        }
    }

    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, struct sockaddr* remoteAddr) {
        int family;
        std::string peerIp;
        uint16_t peerPort;
        IP::GetAddressInfo(remoteAddr, family, peerIp, peerPort);
        std::cout << " OnUdpSocketPacketReceived " << peerIp << ":" << peerPort;

        addr_record_t remotesrc;
        IP::CopyAddress(remoteAddr, remotesrc);

#ifdef __linux__
        IP::addr_unmap_inet6_v4mapped((struct sockaddr *) &remotesrc.addr, &remotesrc.len);
#endif
        std::cout << "testUdpServer2::OnRecv " << data << " ip " << peerIp << ":" << peerPort << std::endl << std::flush;
        udpServer->send(data, len, (struct sockaddr *) &remotesrc.addr);
    }

    UdpServer *udpServer{nullptr};
    std::string IP;
    int port;
};

// Implement the Timer Listener to process recurring events
class TestManager : public base::Timer::Listener {
public:
    TestManager() : 
        socket1("::", 6001), 
        socket2("::", 6002), 
        testTimer(this) // Pass this as the Listener
    {
        socket1.start();
        socket2.start();
    }

    void startTesting() {
        // Start a recurring timer: fires initially in 1000ms, repeats every 2000ms
        testTimer.Start(1000, 2000); 
    }

    // Pure virtual method called by the base::Timer instance
    void OnTimer(base::Timer* timer) override {
        std::cout << "\n--- Timer Event Fired (Tick " << iteration << ") ---" << std::endl;

        // Perform the disconnect and reconnect simulation at a specific interval
        if (iteration == 3) {
            std::cout << "[Test] Stopping and restarting socket1 to check reconnect functionality..." << std::endl;
            socket1.shutdown();
            
            // Simulating a temporary drop, then restarting the binding
            base::sleep(100); 
            socket1.start();
            std::cout << "[Test] socket1 restarted successfully." << std::endl;
        }

        // Incrementing packet payloads
        std::string text1 = "Connection1 " + std::to_string(inc1++);
        std::string text2 = "Connection2 " + std::to_string(inc2++);

        // Transmit data across paths
        std::cout << "[Send] socket1 -> socket2: " << text1 << std::endl;
        socket1.send(text1, "127.0.0.1", 6002);

        std::cout << "[Send] socket2 -> socket1: " << text2 << std::endl;
        socket2.send(text2, "127.0.0.1", 6001);

        iteration++;
    }

    void stopTesting() {
        testTimer.Stop(); // Clean stop
        socket1.shutdown();
        socket2.shutdown();
    }

private:
    testUdpServer1 socket1;
    testUdpServer2 socket2;
    base::Timer testTimer;
    
    int inc1{1};
    int inc2{1};
    int iteration{1};
};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
    Application app;

    // Encapsulate testing lifecycle within the manager
    TestManager manager;
    manager.startTesting();

    std::cout << "waitForShutdown active. Press Ctrl+C to stop." << std::endl << std::flush;
    app.waitForShutdown([&](void*) {
        manager.stopTesting();
    });

    return 0;
}
