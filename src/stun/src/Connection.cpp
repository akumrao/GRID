#include <Connection.h>
#include <string.h>

namespace rtc
{
static void on_udp_data(  const char* data, uint32_t nbytes) 
{
  stun::Message msg;
  stun::Reader stun;
  int r = stun.process((uint8_t*)data, nbytes, &msg);

  if (r == 0) {
    /* valid stun message */
   // msg.computeMessageIntegrity(PASSWORD);
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

    on_udp_data(data ,len );

    std::cout  <<  " ip " << peerIp << ":" << peerPort   << std::endl << std::flush;

}    
    
    
void tesTcpServer::on_read(Listener* connection, const char* data, size_t len)
{
    std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
    //std::string send = "12345";
   // connection->send((const char*) send.c_str(), 5);
    
    on_udp_data(data ,len );

}





void tesTcpClient::sendit( uint8_t* data, size_t len )
{
    tcpClient->send( (char*) data, len );
}

void tesTcpClient::on_read(Listener* connection, const char* data, size_t len) {
    std::cout  << "len: " << len << std::endl << std::flush;

    on_udp_data(data ,len );
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
