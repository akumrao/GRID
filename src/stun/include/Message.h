/*

  Message
  --------

  Represents a stun::Message. When you're using Message-Integrity and Fingerprint
  attributes, make sure to call computeMessageIntegrity(), before you call computeFingerprint().
  The fingerprint (crc32) is computer over the buffer including the message-integrity value, whne 
  you don't compute this first, the crc will be incorrect.

 */
#ifndef STUN_MESSAGE_H
#define STUN_MESSAGE_H

#include <Attribute.h>
#include <Types.h>

#include "candidate.h"


using namespace rtc;


#define STUN_TRANSACTION_ID_SIZE 12


#define HASH_SHA256_SIZE 32
// USERHASH is a SHA256 digest
#define USERHASH_SIZE HASH_SHA256_SIZE

namespace stun {

    
// sequence of less than 513 bytes [...]
#define STUN_MAX_USERNAME_LEN 513 + 1

// The REALM attribute [...] MUST be a UTF-8 [RFC3629] encoded sequence of less than 128 characters
// (which can be as long as 763 bytes)
#define STUN_MAX_REALM_LEN 763 + 1

// The NONCE attribute may be present in requests and responses. It [...] MUST be less than 128
// characters (which can be as long as 763 bytes)
#define STUN_MAX_NONCE_LEN 763 + 1

#define STUN_MAX_PASSWORD_ALGORITHMS_VALUE_SIZE 256
    
    // Nonce cookie prefix as specified in https://www.rfc-editor.org/rfc/rfc8489.html#section-9.2
#define STUN_NONCE_COOKIE "obMatJos2"
#define STUN_NONCE_COOKIE_LEN 9

    
    
#define HMAC_SHA1_SIZE 20
#define HMAC_SHA256_SIZE 32
    
    
// sequence of less than 513 bytes [...]
#define STUN_MAX_USERNAME_LEN 513 + 1
#define STUN_MAX_PASSWORD_LEN STUN_MAX_USERNAME_LEN    
#define MAX_HMAC_INPUT_LEN (STUN_MAX_USERNAME_LEN + STUN_MAX_REALM_LEN + STUN_MAX_PASSWORD_LEN + 2)
    
struct stun_value_password_algorithm {
	uint16_t algorithm;
	uint16_t parameters_length;
	uint8_t parameters[];
};


typedef enum stun_password_algorithm {
	STUN_PASSWORD_ALGORITHM_UNSET = 0x0000,
	STUN_PASSWORD_ALGORITHM_MD5 = 0x0001,
	STUN_PASSWORD_ALGORITHM_SHA256 = 0x0002,
} stun_password_algorithm_t;
    
    
typedef struct stun_credentials {
	char username[STUN_MAX_USERNAME_LEN];
	char realm[STUN_MAX_REALM_LEN];
	char nonce[STUN_MAX_NONCE_LEN];
	uint8_t userhash[USERHASH_SIZE];
	bool enable_userhash;
	stun_password_algorithm_t password_algorithm;
	uint8_t password_algorithms_value[STUN_MAX_PASSWORD_ALGORITHMS_VALUE_SIZE];
	size_t password_algorithms_value_size;
} stun_credentials_t;


  class Message {
  public:
    Message( stun_class_t msg_class , stun_method_t msg_method );
    Message(uint16_t type = STUN_MSG_TYPE_NONE);
    ~Message();
    void addAttribute(Attribute* attr);                        /* Add an attribute to the message who takes ownership (will delete all attributes in the d'tor. */
    void copyTransactionID(Message* from);                     /* Copy the transaction ID from the given messsage. */
    void setTransactionID(); 
    void setTransactionID( uint8_t *transaction_id);           /* Set the transaction ID from the given values. */
    bool hasAttribute(AttributeType atype);                    /* Check if the given attribute is found in one of the attributes */
    bool find(MessageIntegrity** result);                      /* Find a message integrity attribute. */
    bool find(XorMappedAddress** result);                      /* Find a xor-mapped-address attribute.*/
    bool find(Fingerprint** result);                           /* Find a fingerprint attrbiute. */
    bool find(Priority** result);

    template<class T> bool find(uint16_t atype, T** result) {
      *result = NULL;
      for (size_t i = 0; i < attributes.size(); ++i) {
        if (attributes[i]->type == atype) {
          *result = static_cast<T*>(attributes[i]);
          return true;
        }
      }
      return false;
    }

  public:
   // uint16_t type;
      
    stun_class_t msg_class;
    stun_method_t msg_method;
    uint16_t type;    
    uint16_t length;
    uint32_t cookie;
    //uint32_t transaction[3];
    stun_credentials_t credentials;
    uint8_t transaction_id[STUN_TRANSACTION_ID_SIZE];
    std::vector<Attribute*> attributes;
    
    unsigned int error_code{0};
    
    uint64_t ice_controlling{0};
    uint64_t ice_controlled{0};
    
    const addr_record_t *mapped{nullptr};
            
  };

} /* namespace stun */

#endif
