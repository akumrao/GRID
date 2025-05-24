#include <Connection.h>

#include <string.h>


using namespace stun;


namespace rtc
{



testUdpServer::testUdpServer(std::string IP, int port,   Agent &agent ) :IP(IP), port(port), agent(agent) {
    
    int x = 0;

}

void testUdpServer::send( uint8_t* data, uint32_t nbytes, std::string ip, int port )
{
     udpServer->send( (char*) data, nbytes , ip , port);
}
      
void testUdpServer::OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) {

    int family;

    std::string peerIp;
    uint16_t peerPort;

    IP::GetAddressInfo(
                remoteAddr, family, peerIp, peerPort);

  //  on_udp_data(data ,len );

    std::cout  <<  " ip " << peerIp << ":" << peerPort   << std::endl << std::flush;
    
    
    stun::Message msg;
    stun::Reader stun;
    int r = stun.process((uint8_t*)data, len, &msg);

    if (r == 0) 
    {
      /* valid stun message */
     // msg.computeMessageIntegrity(PASSWORD);

        
        //Agent agent( locadesp);
        
        
        XorMappedAddress* result;
                
        msg.find( &result );
        
        Candidate candidate;
        candidate.mType = Candidate::Type::ServerReflexive;
        
        std::memcpy(&candidate.resolved.addr , remoteAddr, sizeof(struct sockaddr));
        candidate.resolved.len = sizeof(struct sockaddr);
                    

        agent.ice_create_local_reflexive_candidate( result->address,  result->port, result->family, &candidate );
       
        // printf("final family: %u, address:%s, port: %d\n", result->family,  result->address, result->port);
        SInfo << "family " << result->family << " address " <<  result->address << " port " << result->port   ;
        
        
       // socket = new testUdpServer("0.0.0.0", ++inc , localDes );
       // socket->start();
    
        
    }
    else if (r == 1) {
      /* other data, e.g. DTLS ClientHello or SRTP data */   
     // if (dtls_parser_ptr) {
        //dtls_parser_ptr->process(data, nbytes);
     // }
    }
    else {
      printf("Error: unhandled stun::Reader::process() result.\n");
      exit(1);
    }
  
  
    shutdown();

}    
    
    
void tesTcpServer::on_read(Listener* connection, const char* data, size_t len)
{
    std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
    //std::string send = "12345";
   // connection->send((const char*) send.c_str(), 5);
    
   // on_udp_data(data ,len );

}





void tesTcpClient::sendit( uint8_t* data, size_t len )
{
    tcpClient->send( (char*) data, len );
}

void tesTcpClient::on_read(Listener* connection, const char* data, size_t len) {
    std::cout  << "len: " << len << std::endl << std::flush;

//    on_udp_data(data ,len );
}

void Transport::resolveStunServer( )
{
    
    for( IceServer &icesv:  mConfig.iceServers  )
    {
        SInfo << "resolve " <<  icesv.hostname << ":" << icesv.port;
        resolve(icesv.hostname, icesv.port, Application::uvGetLoop(), &icesv);
        //break;
    }
   
}
 
 
void Transport::cbDnsResolve(addrinfo* res, std::string ip, int port,  void* ptr)
{
    
    IceServer *icesv = (IceServer *)ptr;
    icesv->ip = ip;

     SInfo <<  "IceServer" <<  ip << ":" << port  ;
}


}
