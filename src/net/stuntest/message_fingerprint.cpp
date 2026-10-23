#include <stdio.h>
#include <vector>
#include <UtilStun.h>
#include <Reader.h>
#include <Writer.h>

#include "base/logger.h"

#define USE_WEBRTC 1
#if USE_WEBRTC
#  define PASSWD "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC"
#else 
#  define PASSWD "VOkJxbRl1RmTxUk/WvJxBt"
#endif

static void on_stun_message(stun::MessageStun* msg, void* user);
static void create_stun_message1();
static void create_stun_message2(); 
static void create_stun_message3(); 

using namespace base;

int main() {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    printf("\n\ntest_stun_message_integrity\n\n");

    create_stun_message1();
    create_stun_message2();
    create_stun_message3();
  
    return 0;
}

static void on_stun_message(stun::MessageStun* msg, void* user) {
    printf("Successfully parsed a stun message.\n");
}

static void create_stun_message1() {
    printf("create_stun_message1.\n");

    stun::MessageStun response(stun::STUN_BINDING_REQUEST);
    uint8_t tx_id[STUN_TRANSACTION_ID_SIZE] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
    response.setTransactionID(tx_id);
    
    // Fingerprint will be automatically added by the writer if desired, or we can test parsing.
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

    stun::MessageStun msg;
    stun::Reader reader;
    int  ret = reader.process(&writer.buffer[0], writer.buffer.size(), &msg);
    if( ret > 0)
    {
      SInfo << "test passed ";
    }
}

static void create_stun_message2() {
    printf("creating new message2.\n");

    stun::MessageStun response(stun::STUN_BINDING_RESPONSE);
    uint8_t tx_id[STUN_TRANSACTION_ID_SIZE] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
    response.setTransactionID(tx_id);
    response.setXorMappedAddress("192.168.0.19", 55164);
    snprintf(response.credentials.username, sizeof(response.credentials.username), "%s", "avind");
    response.setUseCandidate(true);
    response.setPriority(50);
    response.error_code = 300;
    response.setIceControlled(1);
    response.setIceControlling(2);

    stun::Writer writer;
    writer.writeMessage(&response, "75C96DDDFC38D194FEDF75986CF962A2D56F3B65F1F7");

    stun::MessageStun msg;
    stun::Reader reader;
    int retRead = reader.process(&writer.buffer[0], writer.buffer.size(), &msg);

    if(retRead > 0 )
    {
      bool ret = reader.stun_check_integrity(writer.buffer.data(), writer.buffer.size(), &msg, "75C96DDDFC38D194FEDF75986CF962A2D56F3B65F1F7");  
      if(!ret) {
          SWarn << "STUN integrity check failed, password";
      }
    }
}

static void create_stun_message3() {
    printf("creating new message3.\n");

    stun::MessageStun response(stun::STUN_BINDING_RESPONSE);
    uint8_t tx_id[STUN_TRANSACTION_ID_SIZE] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
    response.setTransactionID(tx_id);
    response.setXorMappedAddress("fe80::3cb4:1014:d913:87b9", 55164);

   stun::Writer writer;
   writer.writeMessage(&response, "75C96DDDFC38D194FEDF75986CF962A2D56F3B65F1F7");

    stun::MessageStun msg;
    stun::Reader reader;
   int ret =  reader.process(&writer.buffer[0], writer.buffer.size(), &msg);
    if( ret > 0)
    {
      SInfo << "test passed ";
    }
}