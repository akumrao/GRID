#ifndef STUN_UTILS_H
#define STUN_UTILS_H

#include <stdint.h>
#include <string>
#include <vector>
#include <Types.h>


//#define PRINTDEBUG 1

#if PRINTDEBUG
#define PrintDebug printf
#else

#define PrintDebug 

#endif 


namespace stun {
  
    struct stun_header {
	uint16_t type;
	uint16_t length;
	uint32_t magic;
	uint8_t transaction_id[12];
};


struct stun_attr {
	uint16_t type;
	uint16_t length;
	uint8_t value[];
};

#define STUN_MAGIC 0x2112A442

  /* 
     Compute the hmac-sha1 over message.

     uint8_t* message:  the data over which we compute the hmac sha
     uint32_t nbytes:   the number of bytse in message
     std::string key:   key to use for hmac 
     uint8_t* output:   we write the sha1 into this buffer.
   */
  bool compute_hmac_sha(uint8_t* message, uint32_t nbytes, std::string key, int sz, uint8_t* output);

  size_t stun_update_header_length(void *buf, size_t length);
  
  //bool compute_message_verify(unsigned char *buf, size_t size, std::string key, int keylen,  uint8_t *integSha );
  


  /* 
     Compute the fingerprint value for the stun message.
     This will not change the given buffer.

     std::vector<uint8_t>& buffer:    the buffer that contains a valid stun message.
     uint32_t& result:                will be set to the calculated crc value.
  */
  bool compute_fingerprint(std::vector<uint8_t>& buffer, uint32_t& result);


  
   int random_bytes(void *buf, size_t size);
   
   uint32_t rand32(void);
   
   void random_str64(char *buf, size_t size);
   
   void hash_md5(const void *message, size_t size, void *digest);
   void hash_sha256(const void *message, size_t size, void *digest);
   
   void sha256(const std::string& str , std::string& key) ;
   
   
   int const_time_memcmp(unsigned char *a, unsigned char *b, size_t len);
   
   
   
    size_t align32(size_t len);

#define HMAC_SHA1_SIZE 20
#define HMAC_SHA256_SIZE 32
   
#define STUN_MAX_ERROR_REASON_LEN 763 + 1
#define STUN_ATTR_SIZE sizeof(struct stun_attr)
#define STUN_FINGERPRINT_XOR 0x5354554E // "STUN"
#define STUN_MAX_SOFTWARE_LEN 763 + 1
 #define STUN_IS_OPTIONAL_ATTR(attr_type) (attr_type & 0x8000)

    void hmac_sha1(const void *message, size_t size, const void *key, size_t key_size, void *digest);
    void hmac_sha256(const void *message, size_t size, const void *key, size_t key_size, void *digest);
    
    
    void generate_mask(const uint8_t *transaction_id, uint8_t *mask) ;
    
    uint32_t my_crc32(const void *data, size_t size);
    int BASE64_DECODE(const char *str, void *out, size_t out_size);
   
   //bool IsStun(const char* data, size_t len);
   
} /* namespace stun */

#endif
