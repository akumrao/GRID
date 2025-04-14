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
#include <Reader.h>
#include <Writer.h>

#define PASSWORD "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC"  /* our ice-pwd value */

static void on_udp_data(std::string rip, uint16_t rport, std::string lip, uint16_t lport, uint8_t* data, uint32_t nbytes, void* user);             /* gets called when we recieve data on our 'candidate' */

rtc::ConnectionUDP* udp_ptr = NULL;
bool stun_done = false;

int main() 

{

  rtc::ConnectionUDP sock;
  

  printf("\n\ntest_ice\n\n");

  /* read SDP file. */
 
  stun::Reader stun;

/*
    if (offer.find(sdp::SDP_AUDIO, &audio)) {
      audio->remove(sdp::SDP_ATTR_CANDIDATE);
      audio->remove(sdp::SDP_ATTR_ICE_UFRAG);
      audio->remove(sdp::SDP_ATTR_ICE_PWD);
      audio->remove(sdp::SDP_ATTR_ICE_OPTIONS);
      audio->remove(sdp::SDP_ATTR_FINGERPRINT);
      audio->remove(sdp::SDP_ATTR_SETUP);
      audio->add(new sdp::Attribute("ice-ufrag", ice_ufrag, sdp::SDP_ATTR_ICE_UFRAG));
      audio->add(new sdp::Attribute("ice-pwd", ice_pwd, sdp::SDP_ATTR_ICE_PWD));
      audio->add(new sdp::Attribute("candidate", "4252876256 1 udp 2122260223 192.168.0.194 59976 typ host"));
      audio->add(new sdp::AttributeFingerprint("sha-256", fingerprint));
      audio->add(new sdp::AttributeSetup(sdp::SDP_PASSIVE));
    }
  }

 

 */
  
  
  #define STUN_SERVER_IP "74.125.250.129"
  #define STUN_SERVER_PORT 19302

  
  
    /* write */
  stun::Message response(stun::STUN_BINDING_REQUEST);
  response.setTransactionID(0x8c7cc0e7, 0x8e993bf, 0xce7ed169);
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
  

   
   
    if (!sock.bind("192.168.0.19", 59976)) {
    exit(1);
  
}
   
   

  sock.on_data = on_udp_data;
  sock.user = (void*)&stun;

  sock.sendTo(STUN_SERVER_IP, STUN_SERVER_PORT, &writer.buffer[0], writer.buffer.size());
  
  
  
  /* start receiving data */
  while (true) {
    sock.update();
  }
  return 0;
}

static void on_udp_data(std::string rip, uint16_t rport, 
                        std::string lip, uint16_t lport, 
                        uint8_t* data, uint32_t nbytes, void* user) 
{
  stun::Message msg;
  stun::Reader* stun = static_cast<stun::Reader*>(user);
  int r = stun->process(data, nbytes, &msg);

  if (r == 0) {
    /* valid stun message */
    msg.computeMessageIntegrity(PASSWORD);
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

static void on_dtls_data(uint8_t* data, uint32_t nbytes, void* user) {
  /* handle dtls data, e.g. send back to sock. */
}
