#ifndef STUN_TYPES_H
#define STUN_TYPES_H

#include <stdint.h>
#include <string>
#include <sys/socket.h>

typedef enum stun_address_family {
  STUN_ADDRESS_FAMILY_IPV4 = 0x01,
  STUN_ADDRESS_FAMILY_IPV6 = 0x02,
} stun_address_family_t;

namespace stun {

  enum MessageType {
    STUN_MSG_TYPE_NONE           = 0x0000,
    STUN_BINDING_REQUEST         = 0x0001,
    STUN_BINDING_RESPONSE        = 0x0101,
    STUN_BINDING_ERROR_RESPONSE  = 0x0111,
    STUN_BINDING_INDICATION      = 0x0011,
            
  };

    
struct stun_value_error_code {
	uint16_t reserved;
	uint8_t code_class; // lower 3 bits only, higher bits are reserved
	uint8_t code_number;
	//uint8_t reason[];
};


typedef struct addr_record {
	struct sockaddr_storage addr;
	socklen_t len;
} addr_record_t;

struct stun_value_mapped_address {
	uint8_t padding;
	uint8_t family;
	uint16_t port;
	uint8_t address[];
};

#define STUN_ERROR_INTERNAL_VALIDATION_FAILED 599

   
#define STUN_CLASS_MASK 0x0110

    enum stun_class_t {
           STUN_CLASS_REQUEST = 0x0000,
           STUN_CLASS_INDICATION = 0x0010,
           STUN_CLASS_RESP_SUCCESS = 0x0100,
           STUN_CLASS_RESP_ERROR = 0x0110
   } ;

    enum stun_method_t {
            STUN_METHOD_BINDING = 0x0001,

            // Methods for TURN
            // See https://www.rfc-editor.org/rfc/rfc8656.html#section-17
            STUN_METHOD_ALLOCATE = 0x003,
            STUN_METHOD_REFRESH = 0x004,
            STUN_METHOD_SEND = 0x006,
            STUN_METHOD_DATA = 0x007,
            STUN_METHOD_CREATE_PERMISSION = 0x008,
            STUN_METHOD_CHANNEL_BIND = 0x009
    } ;

  enum AttributeType {
    STUN_ATTR_TYPE_NONE            = 0x0000,
    STUN_ATTR_MAPPED_ADDR          = 0x0001,
    STUN_ATTR_CHANGE_REQ           = 0x0003,
    STUN_ATTR_USERNAME             = 0x0006,
    STUN_ATTR_MESSAGE_INTEGRITY    = 0x0008,                 /* See: http://tools.ietf.org/html/rfc5389#section-15.4 + http://tools.ietf.org/html/rfc4013, SHA1, 20 bytes*/
    STUN_ATTR_ERR_CODE             = 0x0009,
    STUN_ATTR_UNKNOWN_ATTRIBUTES   = 0x000a,
    STUN_ATTR_CHANNEL_NUMBER       = 0x000c,
    STUN_ATTR_LIFETIME             = 0x000d,
    STUN_ATTR_XOR_PEER_ADDR        = 0x0012,
    STUN_ATTR_DATA                 = 0x0013,
    STUN_ATTR_REALM                = 0x0014,
    STUN_ATTR_NONCE                = 0x0015,
    STUN_ATTR_XOR_RELAY_ADDRESS    = 0x0016,
    STUN_ATTR_REQ_ADDRESS_FAMILY   = 0x0017,
    STUN_ATTR_EVEN_PORT            = 0x0018,
    STUN_ATTR_REQUESTED_TRANSPORT  = 0x0019,
    STUN_ATTR_DONT_FRAGMENT        = 0x001a,
    STUN_ATTR_XOR_MAPPED_ADDRESS   = 0x0020,
    STUN_ATTR_RESERVATION_TOKEN    = 0x0022,
    STUN_ATTR_PRIORITY             = 0x0024,                /* See: http://tools.ietf.org/html/rfc5245#section-7.1.2.1 */
    STUN_ATTR_USE_CANDIDATE        = 0x0025,                /* See: http://tools.ietf.org/html/rfc5245#section-7.1.2.1 */
    STUN_ATTR_PADDING              = 0x0026,
    STUN_ATTR_RESPONSE_PORT        = 0x0027,
    STUN_ATTR_SOFTWARE             = 0x8022,
    STUN_ATTR_ALTERNATE_SERVER     = 0x8023,
    STUN_ATTR_FINGERPRINT          = 0x8028,                 /* See: http://tools.ietf.org/html/rfc5389#section-8 + http://tools.ietf.org/html/rfc5389#section-15.5 */
    STUN_ATTR_ICE_CONTROLLED       = 0x8029,                 /* See: http://tools.ietf.org/html/rfc5245#section-19.1 */
    STUN_ATTR_ICE_CONTROLLING      = 0x802a,                 /* See: http://tools.ietf.org/html/rfc5245#section-19.1 */
    STUN_ATTR_RESPONSE_ORIGIN      = 0x802b,
    STUN_ATTR_OTHER_ADDRESS        = 0x802c,
    STUN_ATTR_USERHASH             = 0x001E,
    STUN_ATTR_PASSWORD_ALGORITHM   = 0x001D,
    STUN_ATTR_MESSAGE_INTEGRITY_SHA256 = 0x001C,
  };

  /* --------------------------------------------------------------------- */

  std::string attribute_type_to_string(uint32_t t);
  std::string message_type_to_string(uint32_t t);

  /* --------------------------------------------------------------------- */

} /* namespace stun */

#endif
