#include <Connection.h>
#include <TransportTuple.h>
#include <UtilStun.h>
#include <Agent.h>
#include <string.h>


using namespace stun;
using namespace base::net;

namespace rtc
{



testUdpServer::testUdpServer(std::string IP, int port,   Agent *agent ) :IP(IP), port(port), agent(agent) {
    
//    int x = 0;

}

int testUdpServer::agent_direct_send( uint8_t* data, uint32_t nbytes, addr_record_t &record )
{
    return udpServer->send( (char*) data, nbytes , (const struct sockaddr*)&record.addr);
}
      




void testUdpServer::OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) {

    int family;

    std::string peerIp;
    uint16_t peerPort;

    IP::GetAddressInfo(
                remoteAddr, family, peerIp, peerPort);

  //  on_udp_data(data ,len );

    SInfo  <<  " OnUdpSocketPacketReceived ip " << peerIp << ":" << peerPort ;
    
    addr_record_t remotesrc;
    
    
    IP::CopyAddress(remoteAddr, remotesrc );
    
    
   #if 0
   std::string ret{"addr_record_is_equal "};
   {
        
        char ip[40];
        uint16_t port;
        IP::AddressToString(remotesrc, ip, 40, port);
        ret +=  ip + std::string(":") + std::to_string(port);
    
        ret +=" < > ";


    }    
    
    #endif
    
    
    IP::addr_unmap_inet6_v4mapped((struct sockaddr *)&remotesrc.addr , &remotesrc.len);
    
    
    
    #if 0
    {
    
        char ip[40];
        uint16_t port;
        IP::AddressToString(remotesrc, ip, 40, port);
        ret +=  ip + std::string(":") + std::to_string(port);
        
        SInfo << ret;
    }
    #endif    
    
    TransportTuple tuple(socket, (struct sockaddr *) &remotesrc.addr);
    
    
    
    agent->onStunMessage((unsigned char *)data, len,  &remotesrc, nullptr );
// Increase receive transmission.
//    Transport::DataReceived(len);

//    // Check if it's STUN.
//    if (stun::IsStun(data, len))
//    {
//           // OnStunDataReceived(tuple, data, len);
//           agent->onStunMessage((unsigned char *)data, len,  &remotesrc, nullptr );
//    }
//    else
//    {
//        agent->onStunMessage((unsigned char *)data, len,  &remotesrc, nullptr );
//    }
    // Check if it's RTCP.
//    else if (RTC::RTCP::Packet::IsRtcp(data, len))
//    {
//            OnRtcpDataReceived(tuple, data, len);
//    }
//    // Check if it's RTP.
//    else if (RTC::RtpPacket::IsRtp(data, len))
//    {
//            OnRtpDataReceived(tuple, data, len);
//    }
    // Check if it's DTLS.
//    else if (RTC::DtlsTransport::IsDtls(data, len))
//    {
//            OnDtlsDataReceived(tuple, data, len);
//    }
//    else
//    {
//            MS_WARN_DEV("ignoring received packet of unknown type");
//    }
//
//    
//    
//    
//    
//    
//    
//    
//    agent->onStunMessage((unsigned char *)data, len,  &remotesrc, nullptr );
//    
    
    return ;
    
//    stun::Message msg;
//    stun::Reader stun;
//    int r = stun.process((uint8_t*)data, len, &msg);
//
//    if (r == 0) 
//    {
//      /* valid stun message */
//     // msg.computeMessageIntegrity(PASSWORD);
//
//        
//        //Agent agent( locadesp);
//        
//        
//        XorMappedAddress* result;
//                
//        msg.find( &result );
//        
//        Candidate candidate;
//        candidate.mType = Candidate::Type::ServerReflexive;
//        
//        std::memcpy(&candidate.resolved.addr , remoteAddr, sizeof(struct sockaddr));
//        candidate.resolved.len = sizeof(struct sockaddr);
//                    
//
//        agent->ice_create_local_reflexive_candidate( &candidate );
//       
//        // printf("final family: %u, address:%s, port: %d\n", result->family,  result->address, result->port);
//        //SInfo << "family " << result->family << " address " <<  result->address << " port " << result->port   ;
//        
//        
//       // socket = new testUdpServer("0.0.0.0", ++inc , localDes );
//       // socket->start();
//    
//        
//    }
//    else if (r == 1) {
//      /* other data, e.g. DTLS ClientHello or SRTP data */   
//     // if (dtls_parser_ptr) {
//        //dtls_parser_ptr->process(data, nbytes);
//     // }
//    }
//    else {
//      printf("Error: unhandled stun::Reader::process() result.\n");
//      exit(1);
//    }
//  
//  
//    shutdown();

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


 

}//end namespace

    


