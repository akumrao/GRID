/*
  test_ice
  ---------
  
  This is used during development of the library; it contains ugly snippets of 
  code to try/experiment with the different parts of libwebrtc. This is not 
  meant to be used by end-users of the library. This is also not a unit test.

 */
#include <stdio.h>
#include <stdlib.h>
#include <Connection.h>
#include <Agent.h>
#include "configuration.h"
#include "peerconnection.h"

//#include <udpClient.h>

using namespace stun;
using namespace rtc;

static void on_udp_data(std::string rip, uint16_t rport, std::string lip, uint16_t lport, uint8_t* data, uint32_t nbytes, void* user); /* gets called when we recieve data on our 'candidate' */

//rtc::ConnectionUDP* udp_ptr = NULL;
bool stun_done = false;

int main()
{

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    Configuration config1;
    config1.iceTransportPolicy = TransportPolicy::All; // force relay
    // STUN server example
    config1.iceServers.emplace_back("stun:stun.l.google.com:19302");

    Configuration config2;
    // TURN server example (use your own server in production)
    config1.iceServers.emplace_back("turn:openrelayproject:openrelayproject@openrelay.metered.ca:80");

    PeerConnection pc1(config1);


    pc1.onStateChange([](PeerConnection::State state) {
        std::cout << "State 1: " << state << endl; });

    pc1.onIceStateChange(
            [](PeerConnection::IceState state) {
                std::cout << "ICE state 1: " << state << endl; });

    pc1.onGatheringStateChange([&pc1 ](PeerConnection::GatheringState state) {
        std::cout << "Gathering state 1: " << state << endl;
        if (state == PeerConnection::GatheringState::Complete) {
            auto sdp = pc1.localDescription();
                    std::cout << "Description 1: " << sdp << endl;
                    // pc2.setRemoteDescription(string(sdp));
        }
    });

    pc1.onSignalingStateChange([](PeerConnection::SignalingState state) {
        std::cout << "Signaling state 1: " << state << endl;
    });


    pc1.setLocalDescription();

    // STUN server example (use your own server in production)



    printf("\n\ntest_ice\n\n");


   

    /* read SDP file. */



#define STUN_SERVER_IP "74.125.250.129"
#define STUN_SERVER_PORT 19302



    /* write */
    stun::Message response(stun::STUN_BINDING_REQUEST);
    response.setTransactionID();
    response.addAttribute(new stun::Software("libjuice"));
    response.addAttribute(new stun::Fingerprint());


    stun::Writer writer;
    writer.writeMessage(&response, "");

    printf("---------------\n");
    for (size_t i = 0; i < writer.buffer.size(); ++i) {
        if (i == 0 || i % 4 == 0) {
            printf("\n");
        }
        printf("%02X ", writer.buffer[i]);
    }
    printf("\n---------------\n");




    //    if (!sock.bind("192.168.0.19", 59976)) {
    //    exit(1);

    //}



    //  sock.on_data = on_udp_data;
    //  sock.user = (void*)&stun;

    // sock.sendTo(STUN_SERVER_IP, STUN_SERVER_PORT, &writer.buffer[0], writer.buffer.size());



    /* start receiving data */
    //  while (true) {
    //    sock.update();
    //  }




    Application app;
    
    
    Transport transport(config1);
    
    transport.resolveStunServer( );
    
//    
//    Agent agent;
//
//    agent.getInterfaces();
//
//    //return 0;
//    
//
//    testUdpServer socket("0.0.0.0", 6000);
//    socket.start();
//
//    socket.send(&writer.buffer[0], writer.buffer.size(), STUN_SERVER_IP, STUN_SERVER_PORT);
//
//

    //    tesTcpServer tsvsocket;
    //    tsvsocket.start("0.0.0.0", 6000);
    //    
    //    
    //    tesTcpClient socket;
    //    socket.start(STUN_SERVER_IP , STUN_SERVER_PORT);
    //    
    //    socket.sendit( &writer.buffer[0], writer.buffer.size());    

    app.waitForShutdown([&](void*) {


    //app.run();

//    socket.shutdown();

     });


    return 0;
}




