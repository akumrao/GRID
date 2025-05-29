#ifndef STUN_ATTRIBUTE_H
#define STUN_ATTRIBUTE_H

#include <stdint.h>
#include <string>
#include <vector>
#include <Types.h>
#include <arpa/inet.h>
namespace stun {

  /* --------------------------------------------------------------------- */

  class Attribute {
  public:
    Attribute(uint16_t type = STUN_ATTR_TYPE_NONE);
    
  public:
    uint16_t type;
    uint16_t length;    /* The number of bytes of the attribute data. This is the size w/o the padded bytes that are added when the the attribute is not padded to 32 bits. */
    uint16_t nbytes;    /* The number of bytes the attribute takes inside the buffer. Because the STUN message has to be padded on 32 bits the length may be different from the nbytes. Also, this nbytes includes the bytes of the type field and length field. */
    uint32_t offset;    /* Byte offset where the header of the attribute starts in the Message::buffer. */
  };

  /* --------------------------------------------------------------------- */

  class StringValue {
  public:
    StringValue(){}
    StringValue(std::string v) { std::copy(v.begin(), v.end(), std::back_inserter(buffer)); } 
    std::vector<uint8_t> buffer;
  };

  /* --------------------------------------------------------------------- */

  class Username : public Attribute {
  public:
    Username():Attribute(STUN_ATTR_USERNAME){ }
    Username(std::string name):value(name),Attribute(STUN_ATTR_USERNAME) { }
    StringValue value;
  };

  /* --------------------------------------------------------------------- */

  class Software : public Attribute {
  public:
    Software():Attribute(STUN_ATTR_SOFTWARE) {}
    Software(std::string name):value(name),Attribute(STUN_ATTR_SOFTWARE) {}
    StringValue value;
  };

  /* --------------------------------------------------------------------- */

  class XorMappedAddress : public Attribute {
  public:
    XorMappedAddress();
    XorMappedAddress(const char * ip, uint16_t p, stun_address_family_t family = STUN_ADDRESS_FAMILY_IPV4);
      
    void addressToString( char *buf,  uint16_t &port);
  
   // stun_address_family_t family;
    //uint16_t port;
    //char address[47];  /* IP address in string notation: 192.168.0.1 */
    //struct sockaddr_in sin;
    //struct sockaddr_in6 sin6;
    
    addr_record_t mapped;

  };

  /* --------------------------------------------------------------------- */

  class Fingerprint : public Attribute {
  public:
    Fingerprint();
    uint32_t crc;
  };

  /* --------------------------------------------------------------------- */

  class IceControlled : public Attribute {
  public:
    IceControlled();
    uint64_t tie_breaker;
  };

  /* --------------------------------------------------------------------- */

  class IceControlling : public Attribute {
  public:
    IceControlling();
    uint64_t tie_breaker;
  };

  
 /* --------------------------------------------------------------------- */
  
  class UseCandidate : public Attribute {
    public:
       UseCandidate():Attribute(STUN_ATTR_USE_CANDIDATE)
       {
       }
   };
    
  
  
  /*-----------------------------------------------------------------------*/
  
   class ErrorIce : public Attribute {
    public:
        ErrorIce(unsigned int );
        const char * errorNumber(unsigned int error_code);      
        stun_value_error_code error;
     
    };
  
 
  /* --------------------------------------------------------------------- */

  class Priority : public Attribute {
  public:
    Priority();
    uint32_t value;
  };

  /* --------------------------------------------------------------------- */

  class MessageIntegrity : public Attribute {
  public:
    MessageIntegrity(int size);
    union
    {
        uint8_t sha1[20];
        uint8_t sha256[32];
    }sha;
     
    int sz;
  };

} /* namespace stun */

#endif
