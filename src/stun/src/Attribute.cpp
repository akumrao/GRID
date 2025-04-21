#include <string.h>
#include <Attribute.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  Attribute::Attribute(uint16_t type)
    :type(type)
    ,length(0)
    ,nbytes(0)
    ,offset(0)
  {
  }

  /* --------------------------------------------------------------------- */

  XorMappedAddress::XorMappedAddress()
    :Attribute(STUN_ATTR_XOR_MAPPED_ADDRESS)
    ,family(0)
    ,port(0)
  {
  }

  XorMappedAddress::XorMappedAddress(std::string addr, uint16_t port, uint8_t fam)
    :Attribute(STUN_ATTR_XOR_MAPPED_ADDRESS)
    ,address(addr)
    ,port(port)
    ,family(fam)
  {
  }

  /* --------------------------------------------------------------------- */

  Fingerprint::Fingerprint()
    :Attribute(STUN_ATTR_FINGERPRINT)
    ,crc(0)
  {
    
  }
  
  /* --------------------------------------------------------------------- */

  Priority::Priority()
    :Attribute(STUN_ATTR_PRIORITY) 
    ,value(0)
  {
  }

  /* --------------------------------------------------------------------- */

  IceControlled::IceControlled()
    :Attribute(STUN_ATTR_ICE_CONTROLLED)
    ,tie_breaker(0)
  {
  }

  /* --------------------------------------------------------------------- */

  IceControlling::IceControlling()
    :Attribute(STUN_ATTR_ICE_CONTROLLING)
    ,tie_breaker(0)
  {
  }

  /* --------------------------------------------------------------------- */

  MessageIntegrity::MessageIntegrity(int sz) 
    :sz(sz), Attribute(STUN_ATTR_MESSAGE_INTEGRITY)
  {
    if(sz == 20)
    memset(sha.sha1, 0x00, 20);
    else
    memset(sha.sha256, 0x00, 32);
    
  }

  /* --------------------------------------------------------------------- */



} /* namespace stun */
