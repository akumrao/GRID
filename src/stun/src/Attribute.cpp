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
  {
  }

  

  XorMappedAddress::XorMappedAddress(const char * ip , uint16_t port, stun_address_family_t fam)
    :Attribute(STUN_ATTR_XOR_MAPPED_ADDRESS)
  {

    char ipstr[INET6_ADDRSTRLEN];
    socklen_t addrlen;

    if(fam == STUN_ADDRESS_FAMILY_IPV4) 
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&mapped.addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port  = htons(port);
        inet_pton(AF_INET, ip, &(addr4->sin_addr));
        mapped.len = sizeof(struct sockaddr_in);
    }
    
    else if(fam == STUN_ADDRESS_FAMILY_IPV6) 
     {
    // Example IPv6 address
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&mapped.addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port  = htons(port);
        inet_pton(AF_INET6, ip, &(addr6->sin6_addr));
        mapped.len = sizeof(struct sockaddr_in6);
        
    }
  
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

static const char *stun_get_error_reason(unsigned int code) {
	switch (code) {
	case 0:
		return "";
	case 300:
		return "Try Alternate";
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthenticated";
	case 403:
		return "Forbidden";
	case 420:
		return "Unknown Attribute";
	case 437:
		return "Allocation Mismatch";
	case 438:
		return "Stale Nonce";
	case 440:
		return "Address Family not Supported";
	case 441:
		return "Wrong credentials";
	case 442:
		return "Unsupported Transport Protocol";
	case 443:
		return "Peer Address Family Mismatch";
	case 486:
		return "Allocation Quota Reached";
	case 500:
		return "Server Error";
	case 508:
		return "Insufficient Capacity";
	default:
		return "Error";
	}
}
  
   /*-----------------------------------------------------------------------*/
  
    ErrorIce::ErrorIce(unsigned int error_code):Attribute(STUN_ATTR_ERR_CODE)
    {
        error.code_class = (error_code / 100) & 0x07;
        error.code_number =  error_code % 100;
    }
 
    const char * ErrorIce::errorNumber(unsigned int error_code)
    {
       return  stun_get_error_reason(error_code);
    }

} /* namespace stun */
