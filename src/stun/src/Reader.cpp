#include <string.h>
#include <Reader.h>
#include <Utils.h>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

#include <base/logger.h>


using namespace base;

namespace stun {

  /* --------------------------------------------------------------------- */

  static bool stun_validate_cookie(uint32_t cookie);

  /* --------------------------------------------------------------------- */

  Reader::Reader() 
    :dx(0)
  {
  }

  /* @todo Reader::process - we can optimize this part by not copying but setting pointers to the  members of the Message. */
  /* @todo Reader::process - we need to implement the rules as described here: http://tools.ietf.org/html/rfc5389#section-7.3 */
  int Reader::process(uint8_t* data, uint32_t nbytes, Message* msg) {

    
    if (!data) {
        
        
      LError("stun::Reader - error: received invalid data in Reader::process()");
      return -1;
    }
    if (!msg) {
      LError("stun::Reader - error: invalid stun::Message passed into Reader::process()");
      return -1;
    }

    /* handle non-stun data (e.g. DTLS) */
    if ( (data[0] & 0xC0) != 0x00) {
      return 1;
    }
    

    /* resetting the buffer - @todo - at the bottom of this function we erase all read bytes which isn't 100% necessary as we process a full packet a time */
    dx = 0;
    buffer.clear();
    
    std::copy(data, data + nbytes, std::back_inserter(buffer));
    
    /* A stun message must at least contain 20 bytes. */
    if (buffer.size() < 20) {
      return 1;
    }

    STrace << "stun::Reader - verbose: data to process: " << nbytes << " bytes, " <<  buffer.size();
    
    /* create the base message */
    msg->type = readU16();
  
    msg->msg_class = (stun_class_t)( msg->type & STUN_CLASS_MASK);
    msg->msg_method = (stun_method_t)( msg->type & ~STUN_CLASS_MASK);
    
    msg->length = readU16();
    msg->cookie = readU32();
    memcpy(msg->transaction_id,  getArray(STUN_TRANSACTION_ID_SIZE), STUN_TRANSACTION_ID_SIZE);
//  msg->transaction[0] = readU32();
//  msg->transaction[1] = readU32();
//  msg->transaction[2] = readU32();

    //printf("stun::Reade msg len %d, transactionids %x, %x, %x \n ", msg->length,  msg->transaction[0],  msg->transaction[1],  msg->transaction[2] );

    printf("stun::Reade msg  transactionids: ");
       for (int k = 0; k < STUN_TRANSACTION_ID_SIZE; ++k) {
         printf("%02X ", msg->transaction_id[k]);
    }
    printf("\n");
          
    /* validate */
    if (!stun_validate_cookie(msg->cookie)) {
      SWarn << "stun::Reader - warning: invalid STUN cookie, number of bytes: "  << nbytes;
      printf("stun::Reader - warning: invalid cookie data: %02X %02X %02X %02X\n", data[0], data[1], data[2], data[3]);
      buffer.clear();
      dx = 0;
      return 1;
    }

    /* parse the rest of the message */
    int c = 0;
    uint16_t attr_type;
    uint16_t attr_length;
    uint32_t attr_offset;
    uint32_t prev_dx;
    
    if(bytesLeft() != msg->length)
    printf("stun::Read Error msg len %d, bytesLeft %d \n ", msg->length,bytesLeft());
    
    while (bytesLeft() >= 4) {
      
      Attribute* attr = NULL;
      prev_dx = dx;
      attr_offset = dx; 
      attr_type = readU16();
      attr_length = readU16();

      printf("stun::Reader - received message type: %s, Type: %s, Length: %d, bytes left: %u, current index: %ld\n", 
             message_type_to_string(msg->type).c_str(),
             attribute_type_to_string(attr_type).c_str(), 
             attr_length, 
             bytesLeft(), 
             dx);

      switch (attr_type) {


        /* no parsing needed for these */
        case STUN_ATTR_USE_CANDIDATE: {
          attr = new UseCandidate();
          break;
        }

        case STUN_ATTR_USERNAME: { 
          Username* username = new Username();
          username->value = readString(attr_length);
         
          memcpy(msg->credentials.username,  username->value.buffer.data(), attr_length);
          msg->credentials.username[attr_length] = '\0';
          
                  
          attr = (Attribute*) username;
          STrace << "stun::Reader - verbose: username: " <<  username->value.buffer.data();
          break;
        }

        case STUN_ATTR_SOFTWARE: { 
          Software* software = new Software();
          software->value = readString(attr_length);
          attr = (Attribute*) software;
          STrace<< " stun::Reader - verbose: software %s " <<  software->value.buffer.data();
          break;
        }

        case STUN_ATTR_XOR_MAPPED_ADDRESS: {
          XorMappedAddress* address = readXorMappedAddress( msg, attr_length);
          
          
          if (address) {
              
            msg->mapped = &address->mapped;
            
            char buf[40];  uint16_t port;
            addressToString(buf, port, address->mapped);
            
            attr = (Attribute*) address;
           
          }
          break;
        }

        case STUN_ATTR_PRIORITY: {
          /* priority: http://tools.ietf.org/html/rfc5245#section-4.1.2.1 */
          Priority* prio = new Priority();
          prio->value = readU32();
          attr = (Attribute*) prio;
          STrace << "stun::Reader - verbose: priority: " <<  prio->value;
          break;
        } 

        case STUN_ATTR_MESSAGE_INTEGRITY: {     
          MessageIntegrity* integ = new MessageIntegrity(20);
          memcpy(integ->sha.sha1, &buffer[dx], 20);
          printf("stun::Reader - received Message-Integrity: ");
          for (int k = 0; k < 20; ++k) {
            printf("%02X ", integ->sha.sha1[k]);
          }
          printf("\n");
          attr = (Attribute*) integ;
          skip(20);
          break;
        }

        case STUN_ATTR_FINGERPRINT: {
          /* CRC32-bit, see http://tools.ietf.org/html/rfc5389#section-15.5 */
          Fingerprint* fp = new Fingerprint();
          fp->crc = readU32();
          printf("stun::Reader - verbose: Fingerprint: %x\n",fp->crc);
          attr = (Attribute*) fp;
          break;
        }

        case STUN_ATTR_ICE_CONTROLLED: {
          IceControlled* ic = new IceControlled();
          ic->tie_breaker = readU64();
          STrace << "stun::Reader - verbose: STUN_ATTR_ICE_CONTROLLED:  " <<  ic->tie_breaker;
          attr = (Attribute*) ic;
          msg->ice_controlled = ic->tie_breaker;
          break;
        }

        case STUN_ATTR_ICE_CONTROLLING: {
          IceControlling* ic = new IceControlling();
          ic->tie_breaker = readU64();
          STrace <<  "stun::Reader - verbose: STUN_ATTR_ICE_CONTROLLING:" <<   ic->tie_breaker;
          attr = (Attribute*) ic;
          msg->ice_controlling = ic->tie_breaker;
          break;
        }

         case STUN_ATTR_ERR_CODE: {

            if (attr_length < sizeof (struct stun_value_error_code)) {
                SDebug << "STUN error code value too short, length= " << attr_length;
                return -1;
            }
		
            stun_value_error_code *error = (stun_value_error_code *)getArray(attr_length);
            msg->error_code = (error->code_class & 0x07) * 100 + error->code_number;

            if (msg->error_code == 401 || msg->error_code == 438) { // Unauthenticated or Stale Nonce
                SDebug << "Got STUN error code " << msg->error_code;
            }
            
            ErrorIce* err = new ErrorIce(msg->error_code);
             
            attr = (Attribute*) err;
             
            SInfo <<  " Error code "  <<  msg->error_code  <<  " reason " << err->errorNumber(msg->error_code); 
          break;
        }
        
        case STUN_ATTR_USERHASH: {
            printf("STUN_ATTR_USERHASH\n");
            if (attr_length != USERHASH_SIZE) {
        			SWarn << "STUN user hash value too long, length= " << attr_length;
        			return -1;
        		}
            memcpy(msg->credentials.userhash, getArray(USERHASH_SIZE), USERHASH_SIZE);
            msg->credentials.enable_userhash = true;
                
            break;
        }
        case STUN_ATTR_REALM: {
      		printf("Reading realm\n");
      		if (attr_length + 1 > STUN_MAX_REALM_LEN) {
      			SWarn << "STUN realm attribute value too long, length= " <<  attr_length;
      			return -1;
		}
		memcpy(msg->credentials.realm, getArray(attr_length), attr_length);
		msg->credentials.realm[attr_length] = '\0';
		printf("Got realm: %s \n", msg->credentials.realm);
                
                skip(1);// padding
		break;
	}
	case STUN_ATTR_NONCE: {
		printf("Reading nonce \n");
		if (attr_length + 1 > STUN_MAX_NONCE_LEN) {
			SWarn << "STUN nonce attribute value too long, length= " <<  attr_length;
			return -1;
		}
		memcpy(msg->credentials.nonce, getArray(attr_length), attr_length);
		msg->credentials.nonce[attr_length] = '\0';
		printf("Got nonce: %s \n", msg->credentials.nonce);

		// If the nonce of a response starts with the nonce cookie, decode the Security Feature bits
		// See https://www.rfc-editor.org/rfc/rfc8489.html#section-9.2
		if ((msg->type == STUN_BINDING_RESPONSE) &&
		    strlen(msg->credentials.nonce) > STUN_NONCE_COOKIE_LEN + 4 &&
		    strncmp(msg->credentials.nonce, STUN_NONCE_COOKIE, STUN_NONCE_COOKIE_LEN) == 0) {
			char encoded_security_bits[5];
			memcpy(encoded_security_bits, msg->credentials.nonce + STUN_NONCE_COOKIE_LEN, 4);
			encoded_security_bits[4] = '\0';

			uint8_t bytes[4];
			bytes[0] = 0;
//			int len = BASE64_DECODE(encoded_security_bits, bytes + 1, 3);
//			if (len == 3) {
//				*security_bits = ntohl(*((uint32_t *)bytes));
//				JLOG_VERBOSE("Nonce has cookie, Security Feature bits are 0x%lX",
//				             (unsigned long)*security_bits);
//			} else {
//				JLOG_WARN("Nonce has cookie, but the encoded Security Feature bits field \"%s\" is "
//				          "invalid",
//				          encoded_security_bits);
//				security_bits = 0;
//			}
		}
//                else if (msg->msg_class == STUN_CLASS_RESP_ERROR) {
//			printf("Remote agent does not support RFC 8489");
//		}
                 skip(3);// padding
		break;
	}
	case STUN_ATTR_PASSWORD_ALGORITHM: {
		printf("Reading password algorithm\n");
		if (attr_length < sizeof(struct stun_value_password_algorithm)) {
			printf("STUN password algorithm value too short, length=%zu \n", attr_length);
			return -1;
		}
		if ((msg->type != STUN_BINDING_RESPONSE)) {
			stun_value_password_algorithm pwa ;
                        memcpy( &pwa, getArray(attr_length),attr_length );
			stun_password_algorithm_t algorithm = (stun_password_algorithm_t)ntohs(pwa.algorithm);
			if (algorithm == STUN_PASSWORD_ALGORITHM_MD5 ||
			    algorithm == STUN_PASSWORD_ALGORITHM_SHA256)
				msg->credentials.password_algorithm = algorithm;
			else
				printf("Unknown password algorithm 0x%hX \n", algorithm);
		} else {
			printf("Found password algorithm in response, ignoring \n");
		}
		break;
	}
        
        case STUN_ATTR_MESSAGE_INTEGRITY_SHA256: {
                if (attr_length != HMAC_SHA256_SIZE)
                       return -1;

                MessageIntegrity* integ = new MessageIntegrity(32);
                memcpy(integ->sha.sha256, &buffer[dx], 32);
                printf("stun::Reader - received Message-Integrity: ");
                for (int k = 0; k < 32; ++k) {
                  printf("%02X ", integ->sha.sha256[k]);
                }
                printf("\n");
                attr = (Attribute*) integ;
                skip(32);

                break;
        }
        

        default: {
          printf("stun::Reader - error: unhandled STUN attribute %s of length: %u, this will result in incorrect message integrity\n", attribute_type_to_string(attr_type).c_str(), attr_length);
          break;
        }
      }

      /* Padding: http://tools.ietf.org/html/rfc5389#section-15, must be 32bit aligned */
      while ( (dx & 0x03) != 0 && bytesLeft() > 0) {
        dx++;
      }

      /* when we parsed an attribute, we set the members and append it to the message */
      if (attr) {
        attr->length = attr_length;
        attr->type = attr_type;
        attr->offset = attr_offset;
        attr->nbytes = dx - prev_dx;
        msg->addAttribute(attr);
      }

      /* reset vars used while parsing */
      attr = NULL;
      attr_length = 0;
      prev_dx = dx;
    }

    /* and erase any read data. */
    /* @todo - we could use buffer.clear() here ... */
   // buffer.erase(buffer.begin(), buffer.begin() + dx);
    
   // dx = 0;

    return 0;
  }

  uint32_t Reader::bytesLeft() {
    if (buffer.size() == 0) { 
      return 0;
    }

    return buffer.size() - dx;
  }

  uint8_t Reader::readU8() {
    dx++;
    return buffer[dx - 1];
  }

  uint16_t Reader::readU16() {

    if (bytesLeft() < 2) { 
      printf("Error: trying to readU16() in STUN, but the buffer is not big enough.\n");
      return 0;
    }

    uint16_t result;
    uint8_t* dst = (uint8_t*)&result;

    dst[0] = buffer[dx + 1];
    dst[1] = buffer[dx + 0];

    dx += 2;

    return result;
  }

  uint32_t Reader::readU32() {

    if (bytesLeft() < 4) {
      printf("Error: trying to readU32() in STUN, but the buffer is not big enough.\n");
      return 0;
    }

    uint32_t result;
    uint8_t* dst = (uint8_t*)&result;

    dst[0] = buffer[dx + 3];
    dst[1] = buffer[dx + 2];
    dst[2] = buffer[dx + 1];
    dst[3] = buffer[dx + 0];

    dx += 4;

    return result;
  }

  uint64_t Reader::readU64() {

    if (bytesLeft() < 8) {
      printf("Error: trying to readU64() in STUN, but the buffer is not big enough.\n");
      return 0;
    }

    uint64_t result;
    uint8_t* dst = (uint8_t*)&result;

    dst[0] = buffer[dx + 7];
    dst[1] = buffer[dx + 6];
    dst[2] = buffer[dx + 5];
    dst[3] = buffer[dx + 4];
    dst[4] = buffer[dx + 3];
    dst[5] = buffer[dx + 2];
    dst[6] = buffer[dx + 1];
    dst[7] = buffer[dx + 0];

    dx += 8;

    return result;
  }


  
  void Reader::skip(uint32_t nbytes) {
    if (dx + nbytes > buffer.size()) {
      SError << "Error: trying to skip " <<  nbytes <<  " bytes, but we only have %u left in STUN " << bytesLeft();
      return;
    }
    dx += nbytes;
  }


  StringValue Reader::readString(uint16_t len) {
    StringValue v;

    if (bytesLeft() < len) {
      LError("Error: trying to read a StringValue from the buffer, but the buffer is not big enough");
      return v;
    }

    std::copy(ptr(), ptr() + len, std::back_inserter(v.buffer));
    dx += len;
    return v;
  }

   unsigned char* Reader::getArray(uint16_t len) {
       
    if (bytesLeft() < len) {
      LError("Error: trying to read a getArray from the buffer, but the buffer is not big enough.");
      return nullptr;
    }
    unsigned char* ret =  ptr();
    dx += len;
    return ret;
  }

   
   
   
   
   
   
    static int stun_read_value_mapped_address(const void *data, size_t size,  addr_record_t *mapped,  const uint8_t *mask) {
	size_t len = sizeof(struct stun_value_mapped_address);
	if (size < len) {
		STrace << "STUN mapped address value too short, size=" <<  size;
		return -1;
	}
	const struct stun_value_mapped_address *value = (const struct stun_value_mapped_address *) data;
	stun_address_family_t family = (stun_address_family_t)value->family;
	switch (family) {
	case STUN_ADDRESS_FAMILY_IPV4: {
		len += 4;
		if (size < len) {
			STrace << "IPv4 mapped address value too short, size=" <<  size;
			return -1;
		}
		LTrace("Reading IPv4 address");
		mapped->len = sizeof(struct sockaddr_in);
		struct sockaddr_in *sin = (struct sockaddr_in *)&mapped->addr;
		sin->sin_family = AF_INET;
		sin->sin_port = value->port ^ *((uint16_t *)mask);
		uint8_t *bytes = (uint8_t *)&sin->sin_addr;
		for (int i = 0; i < 4; ++i)
			bytes[i] = value->address[i] ^ mask[i];
		break;
	}
	case STUN_ADDRESS_FAMILY_IPV6: {
		len += 16;
		if (size < len) {
			STrace << "IPv6 mapped address value too short, size=" <<  size;
			return -1;
		}
		LTrace("Reading IPv6 address");
		mapped->len = sizeof(struct sockaddr_in6);
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&mapped->addr;
		sin6->sin6_family = AF_INET6;
		sin6->sin6_port = value->port ^ *((uint16_t *)mask);
		uint8_t *bytes = (uint8_t *)&sin6->sin6_addr;
		for (int i = 0; i < 16; ++i)
			bytes[i] = value->address[i] ^ mask[i];
		break;
	}
	default: {
		STrace << "Unknown STUN address family 0x%X" <<  (unsigned int)family;
		len = size;
		break;
	}
	}
	return (int)len;
    }
   
  /* See http://tools.ietf.org/html/rfc5389#section-15.2 */
  XorMappedAddress* Reader::readXorMappedAddress( Message* msg , int len) {

    if (bytesLeft() < 8) {
      LError("Error: cannot read a XorMappedAddress as the buffer is too small in stun::Reader");
      return NULL;
    }
    
    XorMappedAddress* addr = new XorMappedAddress();
  
    //uint8_t cookie[] = { 0x42, 0xA4, 0x12, 0x21 }; 
      uint8_t mask[16];
   // const uint32_t magic = htonl(STUN_MAGIC);
    //memcpy(mask, (const uint8_t*)&magic, 4);
    *((uint32_t *)mask) = htonl(STUN_MAGIC);
    memcpy(mask + 4, msg->transaction_id, 12);
    
    
    if (stun_read_value_mapped_address(getArray(len), len, ( addr_record_t *) &addr->mapped, mask) < 0)
    return nullptr;
    
    //uint8_t* port_ptr = (uint8_t*) &addr->port;
   

    /* skip the first byte */
//    skip(1); 
//
//    /* read family: 0x01 = IP4, 0x02 = IP6 */
//    addr->family = readU8();
//    if (addr->family != STUN_IP4 && addr->family != STUN_IP6) {
//      LError("Error: invalid family for the xor mapped address in stun::Reader");
//      delete addr;
//      return NULL;
//    }
//
//    /* read the port. */
//    addr->port = *((uint16_t *)getArray(2)) ^ *((uint16_t *)mask);
//   // port_ptr[0] = port_ptr[0] ^ cookie[2];
//   // port_ptr[1] = port_ptr[1] ^ cookie[3];
//
//    /* read the address part. */
//    if (addr->family == STUN_IP4) {
//        
//        //unsigned char bytes[4];
//        //unsigned char ip_addr[16];
//        //memcpy(bytes, getArray(4), 4 );
//        
//        unsigned char *tmp =  getArray(4);
//
//     // uint32_t ip = 0;
//      //uint8_t* bytes = (uint8_t*) &ip;
//      
//     // ip = readU32();
//
////      ip_ptr[0] = ip_ptr[0] ^ cookie[0];
////      ip_ptr[1] = ip_ptr[1] ^ cookie[1];
////      ip_ptr[2] = ip_ptr[2] ^ cookie[2];
////      ip_ptr[3] = ip_ptr[3] ^ cookie[3];
//      
//     // struct sockaddr_in sa;
//      
//
//      
//        addr->sin.sin_family = AF_INET;
//	addr->sin.sin_port =  addr->port;
//        uint8_t *bytes = (uint8_t *)&addr->sin.sin_addr;
//        
//                
//        for (int i = 0; i < 4; ++i)
//            bytes[i] = tmp[i] ^ mask[i];
//
//        inet_ntop(AF_INET, &addr->sin.sin_addr,(char *) addr->address, INET_ADDRSTRLEN);
//        addr->port = htons(addr->sin.sin_port);
//
//        //sprintf((char*)ip_addr, "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
//       // std::copy(ip_addr, ip_addr + 16, std::back_inserter(addr->address));
//
//    }
//    else if (addr->family == STUN_IP6) {
//      
//        unsigned char *tmp =  getArray(16);   
//        addr->sin6.sin6_family = AF_INET6;
//        addr->sin6.sin6_port =  addr->port;
//        uint8_t *bytes = (uint8_t *)&addr->sin6.sin6_addr;
//
//        for (int i = 0; i < 16; ++i)
//           bytes[i] = tmp[i] ^ mask[i];
//
//        inet_ntop(AF_INET6, &addr->sin6.sin6_addr,(char *) addr->address, INET6_ADDRSTRLEN);
//        
//        addr->port = htons(addr->sin6.sin6_port);
//         
//      //   struct sockaddr_in6 sa6;
//	//char astring[INET6_ADDRSTRLEN];
//
//	// IPv6 string to sockaddr_in6.
//	//inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr));
//
//	// sockaddr_in6 to IPv6 string.
//	//inet_ntop(AF_INET6, &(sa6.sin6_addr), astring, INET6_ADDRSTRLEN);
//
//    }
//    else {
//      printf("Warning: we shouldn't arrive here in stun::Reader.\n");
//      delete addr;
//      return NULL;
//    }

    return addr;
  }

  uint8_t* Reader::ptr() {
    return &buffer[dx];
  }
  
  #define HASH_MD5_SIZE 16

size_t generate_hmac_key(Message* msg, std::string &password, std::string &key)
{
    if (*msg->credentials.realm != '\0') {
        // long-term credentials
        if( *msg->credentials.username == '\0')
           printf("Generating HMAC key for long-term credentials with empty STUN username \n");

        char input[MAX_HMAC_INPUT_LEN];
        int input_len = snprintf(input, MAX_HMAC_INPUT_LEN, "%s:%s:%s", msg->credentials.username,
                msg->credentials.realm, password.size() ? password.data() : "");
        if (input_len < 0)
            return 0;

        if (input_len >= MAX_HMAC_INPUT_LEN)
            input_len = MAX_HMAC_INPUT_LEN - 1;

        switch (msg->credentials.password_algorithm) {
            case STUN_PASSWORD_ALGORITHM_SHA256:
               // hash_sha256(input, input_len, key);
                sha256(input, key);
                return 32;
            default:
              //  hash_md5(input, input_len, key);
                return HASH_MD5_SIZE;
        }
    } else {
        // short-term credentials
//        int key_len = snprintf((char *) key, MAX_HMAC_KEY_LEN, "%s", password ? password : "");
//        if (key_len < 0)
//            return 0;
//
//        if (key_len >= MAX_HMAC_KEY_LEN)
//            key_len = MAX_HMAC_KEY_LEN - 1;
        key = password;
        return 20;
    }
}
  
bool Reader::computeMessageIntegrity(Message* msg, std::string password) {

    MessageIntegrity* integ = NULL;
    
    std::string key;
    int keylen = generate_hmac_key( msg,password, key );
            
    if (!key.size()) { 
      LError("Error: cannot compute message integrity in stun::Message because the key is empty");
      return false;
    }

    if (!msg->attributes.size() || !msg->find(&integ)) {
      LError("Error: cannot compute the message integrity in stun::Message because the message doesn't contain a MessageIntegrity attribute.");
      return false;
    }

    uint8_t output[keylen]; 
            

    if  (!compute_message_integrity(buffer, key, keylen, output ))
    {
        return false;
    }
    
   
    if (const_time_memcmp( (keylen == 20 ? integ->sha.sha1: integ->sha.sha256) , output, keylen) != 0) {
            SError << "STUN message integrity SHA1 check failed";
            return false;
    }

    
    return true;
  }


  bool Reader::computeFingerprint(Message* msg) {

    Fingerprint* finger = NULL;
    if (!msg->attributes.size() || !msg->find(&finger)) {
      LError("Error: cannot compute fingerprint because there is not fingerprint attribute");
      return false;
    }
     uint32_t crc = 0;
     compute_fingerprint(buffer, crc);
     
     return crc == finger->crc;
  }

  /* --------------------------------------------------------------------- */

  static bool stun_validate_cookie(uint32_t cookie) {
    //printf("%02X:%02X:%02X:%02X\n", ptr[3], ptr[2], ptr[1], ptr[0]);
    uint8_t* ptr = (uint8_t*) &cookie;
    return (ptr[3] == 0x21 
            && ptr[2] == 0x12 
            && ptr[1] == 0xA4 
            && ptr[0] == 0x42);

    return true;
  }

} /* namespace stun */
