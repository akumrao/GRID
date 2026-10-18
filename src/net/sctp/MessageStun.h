/*

  MessageStun
  --------

  Represents a stun::MessageStun. When you're using MessageStun-Integrity and Fingerprint
  attributes, make sure to call computeMessageIntegrity(), before you call computeFingerprint().
  The fingerprint (crc32) is computer over the buffer including the message-integrity value, whne 
  you don't compute this first, the crc will be incorrect.

 */
#ifndef STUN_MESSAGE_H
#define STUN_MESSAGE_H


#include <Types.h>

#include "candidate.hpp"


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
    
 #define MAX_USERHASH_INPUT_LEN (STUN_MAX_USERNAME_LEN + STUN_MAX_REALM_LEN + 1)
    
//struct stun_value_password_algorithm {
//	uint16_t algorithm;
//	uint16_t parameters_length;
//	uint8_t parameters[];
//};


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

#define STUN_MAX_PEER_ADDRESSES 8

void  stun_process_credentials(const stun_credentials_t *credentials, stun_credentials_t *dst);
    

  class MessageStun {
  public:
    MessageStun( stun_class_t msg_class , stun_method_t msg_method );
    MessageStun(uint16_t type = STUN_MSG_TYPE_NONE);
    ~MessageStun();
    void copyTransactionID(MessageStun* from);                     /* Copy the transaction ID from the given messsage. */
    void setRandomTransactionID(); 
    void setTransactionID( uint8_t *transaction_id);           /* Set the transaction ID from the given values. */
    std::string printTransactionID();                                 /* Print the transaction ID for debugging. */
    

    // Added setter/initialization functions for direct variable filling
    void setXorMappedAddress(const char * ip, uint16_t p);
    void setIceControlled(uint64_t tie_breaker);
    void setIceControlling(uint64_t tie_breaker);
    void setPriority(uint32_t value);
    void setUseCandidate(bool enable = true);

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
    
    unsigned int error_code{0};
    
    uint64_t ice_controlling{0};
    uint64_t ice_controlled{0};
    
    uint32_t priority{0};  // TBD

    bool use_candidate{false};  // TBD

    bool has_integrity{false};
    bool has_fingerprint{false};

    // Integrity and fingerprint values stored directly
    union
    {
        uint8_t sha1[20];
        uint8_t sha256[32];
    } message_integrity;
    size_t message_integrity_sz{20};
    uint32_t fingerprint_crc{0};
    
    addr_record_t mapped;
    
#if TURN
    
    // Turn
	// TURN
	addr_record_t peers[STUN_MAX_PEER_ADDRESSES];
	size_t peers_size{0};
	addr_record_t relayed;
	addr_record_t alternate_server;
	const char *data{nullptr};
	size_t data_size{0};
	uint32_t lifetime{0};
	uint16_t channel_number{0};
	bool lifetime_set{false};
	bool even_port{false};
	bool next_port{false};
	bool dont_fragment{false};
	bool requested_transport{false};
	uint64_t reservation_token{0};
#endif
            
  };


#define HASH_MD5_SIZE 16
#define MAX_HMAC_KEY_LEN STUN_MAX_USERNAME_LEN
  
   size_t generate_hmac_key( MessageStun *msg, const char *password, void *key);


} /* namespace stun */

#endif
