#include <stdio.h>
#include <Utils.h>
#include <Reader.h>
#include <Writer.h>

#define PASSWD "VOkJxbRl1RmTxUk/WvJxBt"
static void on_stun_message(stun::Message* msg, void* user);

int main() {


    {
        printf("\n\ntest_stun_message_fingerprint\n\n");

        const unsigned char req[] =
                "\x00\x01\x00\x58"
                "\x21\x12\xa4\x42"
                "\xb7\xe7\xa7\x01\xbc\x34\xd6\x86\xfa\x87\xdf\xae"
                "\x80\x22\x00\x10"
                "STUN test client"
                "\x00\x24\x00\x04"
                "\x6e\x00\x01\xff"
                "\x80\x29\x00\x08"
                "\x93\x2f\xf9\xb1\x51\x26\x3b\x36"
                "\x00\x06\x00\x09"
                "\x65\x76\x74\x6a\x3a\x68\x36\x76\x59\x20\x20\x20"
                "\x00\x08\x00\x14"
                "\x9a\xea\xa7\x0c\xbf\xd8\xcb\x56\x78\x1e\xf2\xb5"
                "\xb2\xd3\xf2\x49\xc1\xb5\x71\xa2"
                "\x80\x28\x00\x04"
                "\xe5\x7a\x3b\xcf";

        int nl = 0;
        printf("\nINPUT");
        printf("\n-----------\n");
        for (int i = 0; i < sizeof (req) - 1; ++i, ++nl) {

            if (nl == 4) {
                printf("\n");
                nl = 0;
            }
            printf("%02X ", req[i]);
        }
        printf("\n-----------\n\n");


        /* @todo - stun::Reader does not call on_message anymore! */
        stun::Message msg;
        stun::Reader reader;
        // reader.on_message = on_stun_message;
         int r = reader.process((uint8_t*) req, sizeof (req) - 1, &msg);
    }
    
    
    printf("\n\ntest_stun_message_fingerprint\n\n");
      
    {
        uint8_t message1[] = {
	    0x00, 0x01, 0x00, 0x58, // Request type and message length
	    0x21, 0x12, 0xa4, 0x42, // Magic cookie
	    0xb7, 0xe7, 0xa7, 0x01, // Transaction ID
	    0xbc, 0x34, 0xd6, 0x86, //
	    0xfa, 0x87, 0xdf, 0xae, //
	    0x80, 0x22, 0x00, 0x10, // SOFTWARE attribute header
	    0x53, 0x54, 0x55, 0x4e, //
	    0x20, 0x74, 0x65, 0x73, //
	    0x74, 0x20, 0x63, 0x6c, //
	    0x69, 0x65, 0x6e, 0x74, //
	    0x00, 0x24, 0x00, 0x04, // PRIORITY attribute header
	    0x6e, 0x00, 0x01, 0xff, //
	    0x80, 0x29, 0x00, 0x08, // ICE-CONTROLLED attribute header
	    0x93, 0x2f, 0xf9, 0xb1, //
	    0x51, 0x26, 0x3b, 0x36, //
	    0x00, 0x06, 0x00, 0x09, // USERNAME attribute header
	    0x65, 0x76, 0x74, 0x6a, //
	    0x3a, 0x68, 0x36, 0x76, //
	    0x59, 0x20, 0x20, 0x20, //
	    0x00, 0x08, 0x00, 0x14, // MESSAGE-INTEGRITY attribute header
	    0x9a, 0xea, 0xa7, 0x0c, //
	    0xbf, 0xd8, 0xcb, 0x56, //
	    0x78, 0x1e, 0xf2, 0xb5, //
	    0xb2, 0xd3, 0xf2, 0x49, //
	    0xc1, 0xb5, 0x71, 0xa2, //
	    0x80, 0x28, 0x00, 0x04, // FINGERPRINT attribute header
	    0xe5, 0x7a, 0x3b, 0xcf, //
	};
        
        
        int nl = 0;
        printf("\nINPUT");
        printf("\n-----------\n");
        for (int i = 0; i < sizeof (message1) ; ++i, ++nl) {

            if (nl == 4) {
                printf("\n");
                nl = 0;
            }
            printf("%02X ", message1[i]);
        }
        printf("\n-----------\n\n");

        
        stun::Message msg;
        stun::Reader reader;
        // reader.on_message = on_stun_message;
        int r = reader.process((uint8_t*) message1, sizeof (message1) , &msg);
        
        
        
        /*
         
	if (msg.priority != 0x6e0001ff)
		return -1;
	if (msg.ice_controlled != 0x932ff9b151263b36LL)
		return -1;
        */

        if( r == 0)
        {
            bool ret = reader.computeMessageIntegrity(&msg, "VOkJxbRl1RmTxUk/WvJxBt");
            if(ret)
            {
                printf("\n check_integrity passed \n");
            }
            else
                printf("\n check_integrity falled \n");
        }
        
    }
    
    
    
    
    printf("\n\ntest_stun_message_fingerprint\n\n");
      
    {
        uint8_t message1[] = {
	    0x00, 0x01, 0x00, 0x90, // Request type and message length
	    0x21, 0x12, 0xa4, 0x42, // Magic cookie
	    0x78, 0xad, 0x34, 0x33, // Transaction ID
	    0xc6, 0xad, 0x72, 0xc0, //
	    0x29, 0xda, 0x41, 0x2e, //
	    0x00, 0x1e, 0x00, 0x20, // USERHASH attribute header
	    0x4a, 0x3c, 0xf3, 0x8f, // Userhash value (32 bytes)
	    0xef, 0x69, 0x92, 0xbd, //
	    0xa9, 0x52, 0xc6, 0x78, //
	    0x04, 0x17, 0xda, 0x0f, //
	    0x24, 0x81, 0x94, 0x15, //
	    0x56, 0x9e, 0x60, 0xb2, //
	    0x05, 0xc4, 0x6e, 0x41, //
	    0x40, 0x7f, 0x17, 0x04, //
	    0x00, 0x15, 0x00, 0x29, // NONCE attribute header
	    0x6f, 0x62, 0x4d, 0x61, // Nonce value and padding (3 bytes)
	    0x74, 0x4a, 0x6f, 0x73, //
	    0x32, 0x41, 0x41, 0x41, //
	    0x43, 0x66, 0x2f, 0x2f, //
	    0x34, 0x39, 0x39, 0x6b, //
	    0x39, 0x35, 0x34, 0x64, //
	    0x36, 0x4f, 0x4c, 0x33, //
	    0x34, 0x6f, 0x4c, 0x39, //
	    0x46, 0x53, 0x54, 0x76, //
	    0x79, 0x36, 0x34, 0x73, //
	    0x41, 0x00, 0x00, 0x00, //
	    0x00, 0x14, 0x00, 0x0b, // REALM attribute header
	    0x65, 0x78, 0x61, 0x6d, // Realm value (11 bytes) and padding (1 byte)
	    0x70, 0x6c, 0x65, 0x2e, //
	    0x6f, 0x72, 0x67, 0x00, //
	    0x00, 0x1d, 0x00, 0x04, // PASSWORD-ALGORITHM attribute header
	    0x00, 0x02, 0x00, 0x00, // PASSWORD-ALGORITHM value (4 bytes)
	    0x00, 0x1c, 0x00, 0x20, // MESSAGE-INTEGRITY-SHA256 attribute header
	    0xb5, 0xc7, 0xbf, 0x00, // HMAC-SHA256 value
	    0x5b, 0x6c, 0x52, 0xa2, //
	    0x1c, 0x51, 0xc5, 0xe8, //
	    0x92, 0xf8, 0x19, 0x24, //
	    0x13, 0x62, 0x96, 0xcb, //
	    0x92, 0x7c, 0x43, 0x14, //
	    0x93, 0x09, 0x27, 0x8c, //
	    0xc6, 0x51, 0x8e, 0x65, //
	};
        
        
        int nl = 0;
        printf("\nINPUT");
        printf("\n-----------\n");
        for (int i = 0; i < sizeof (message1) ; ++i, ++nl) {

            if (nl == 4) {
                printf("\n");
                nl = 0;
            }
            printf("%02X ", message1[i]);
        }
        printf("\n-----------\n\n");

        
        
          /* @todo - stun::Reader does not call on_message anymore! */
        stun::Message msg;
        stun::Reader reader;
        // reader.on_message = on_stun_message;
        int r = reader.process((uint8_t*) message1, sizeof (message1) , &msg);
        
        if( r == 0)
        {
            bool ret = reader.computeMessageIntegrity(&msg, "TheMatrIX");
            if(ret)
            {
                printf("\n check_integrity passed \n");
            }
            else
                printf("\n check_integrity falled \n");
        }
    }
    

 
 return 0;
}

static void on_stun_message(stun::Message* msg, void* user) {
  printf("Received a stun message\n");
  
  /* 
     We write the message and calculate the message integrity + fingerprint.
     Note, because the padded bytes of the source contain 0x20 values and we
     use 0x00, our message integrity value and fingerprint value are different. 
  */
  stun::Writer writer;
  writer.writeMessage(msg, PASSWD);

 int nl = 0;
 printf("\nOUTPUT");
 printf("\n-----------\n");
 for (int i = 0; i < writer.buffer.size(); ++i, ++nl) {

   if (nl == 4) {
     printf("\n");
     nl=0;
   }
   printf("%02X ", writer.buffer[i]);
 }

 printf("\n-----------\n\n");
  
}
