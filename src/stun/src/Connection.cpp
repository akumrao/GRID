#include <Connection.h>
#include <Agent.h>
#include <string.h>


using namespace stun;
using namespace base::net;

namespace rtc
{



testUdpServer::testUdpServer(std::string IP, int port,   Agent *agent ) :IP(IP), port(port), agent(agent) {
    
    int x = 0;

}

int testUdpServer::agent_direct_send( uint8_t* data, uint32_t nbytes, addr_record_t &record )
{
    return udpServer->send( (char*) data, nbytes , (const struct sockaddr*)&record.addr);
}
      


bool addr_unmap_inet6_v4mapped(struct sockaddr *sa, socklen_t *len) {
	if (sa->sa_family != AF_INET6)
		return false;

	const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
	if (!IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr))
		return false;

	struct sockaddr_in6 copy = *sin6;
	sin6 = &copy;

	struct sockaddr_in *sin = (struct sockaddr_in *)sa;
	memset(sin, 0, sizeof(*sin));
	sin->sin_family = AF_INET;
	sin->sin_port = sin6->sin6_port;
	memcpy(&sin->sin_addr, ((const uint8_t *)&sin6->sin6_addr) + 12, 4);
	*len = sizeof(*sin);
	return true;
}


void testUdpServer::OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) {

    int family;

    std::string peerIp;
    uint16_t peerPort;

    IP::GetAddressInfo(
                remoteAddr, family, peerIp, peerPort);

  //  on_udp_data(data ,len );

    STrace  <<  " OnUdpSocketPacketReceived ip " << peerIp << ":" << peerPort ;
    
    addr_record_t src;
    
    
    IP::CopyAddress(remoteAddr, src );
    
    addr_unmap_inet6_v4mapped((struct sockaddr *)&src.addr , &src.len);
    
    agent->onStunMessage((unsigned char *)data, len,  &src, nullptr );
    
    
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

void Transport::resolveStunServer( )
{
    
    for( IceServer &icesv:  mConfig.iceServers  )
    {
        SInfo << "resolve " <<  icesv.hostname << ":" << icesv.port;
        resolve(icesv.hostname, icesv.port, Application::uvGetLoop(), &icesv);
        //break;
    }
   
}

void Transport::cbDnsResolve(addrinfo* start)
{
    
    //IceServer *icesv = (IceServer *)ptr;
   // icesv->ip = ip;

    // SInfo <<  "IceServer" <<  ip << ":" << port  ;
    
                    char addr[40] = {'\0'};
                int port =0; 

                struct addrinfo*  res = start;
                
                for (;res != NULL; res = res->ai_next) 
                { 
                    
                    if (res->ai_family == AF_INET) {
                        // ipv4
                        //char c[17] = { '\0' };
                        
                        sockaddr_in* tmp  =   (sockaddr_in*) res->ai_addr;
                        port= htons(tmp->sin_port);
                        uv_ip4_name(tmp, addr, 16);
                        
        
                        
                    } else if (res->ai_family == AF_INET6) {
                        // ipv6
                        //char c[40] = { '\0' };
                        sockaddr_in6* tmp  =   (sockaddr_in6*) res->ai_addr;
                        port= htons(tmp->sin6_port);
                        uv_ip6_name(tmp, addr, 39);
                    }
                    LTrace("address ",  addr);
                    // uv_tcp_connect(connect_req, socket, (const struct sockaddr*) res->ai_addr, on_connect);

                }
}

void Transport::cbNameResolve(  const char* hostname, const char* service,  void* ptr)
{
     SInfo <<  "resoved " <<  hostname << ":" << service  ;
}




 
void Transport::resolveNames( Candidate *cand )
{
   // SInfo << "resolveName " <<  icesv.hostname << ":" << icesv.port << " addd " << cand;
    
   resolveName(cand->resolved.addr,   Application::uvGetLoop(),  cand) ;
}
 

}//end namespace

    


