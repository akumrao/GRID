#include <stdio.h>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <zlib.h>  /* for crc */
#include <netinet/in.h>

#include <Types.h>
#include <Utils.h>

#include <openssl/sha.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <uv.h>
#include <base/logger.h>

using namespace base;

namespace stun {

// getrandom() is not available in Android NDK API < 28 and needs glibc >= 2.25
#if defined(__linux__) && !defined(__ANDROID__) && (!defined(__GLIBC__) || __GLIBC__ > 2 || __GLIBC_MINOR__ >= 25)

#include <errno.h>
#include <sys/random.h>

 int random_bytes(void *buf, size_t size) {
	ssize_t ret = getrandom(buf, size, 0);
	if (ret < 0) {
		printf("getrandom failed, errno=%d", errno);
		return -1;
	}
	if ((size_t)ret < size) {
		printf("getrandom returned too few bytes, size=%zu, returned=%zu", size, (size_t)ret);
		return -1;
	}
	return 0;
}

#elif defined(_WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 // Windows 7
#endif

#include <windows.h>
//
#include <bcrypt.h>

static int random_bytes(void *buf, size_t size) {
	// Requires Windows 7 or later
	NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
	return !status ? 0 : -1;
}

#else
static int random_bytes(void *buf, size_t size) {
	(void)buf;
	(void)size;
	return -1;
}
#endif

static unsigned int generate_seed() {
#ifdef _WIN32
	return (unsigned int)GetTickCount();
#else
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
		return (unsigned int)(ts.tv_sec ^ ts.tv_nsec);
	else
		return (unsigned int)time(NULL);
#endif
}



void random_str64(char *buf, size_t size) {
  static const char chars64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t i = 0;
  for (i = 0; i + 1 < size; ++i) {
    uint8_t byte = 0;
    random_bytes(&byte, 1);
    buf[i] = chars64[byte & 0x3F];
  }
  buf[i] = '\0';
}

bool compute_hmac_sha(uint8_t* message, uint32_t nbytes, std::string key, int sz, uint8_t* output) {

    if (!message) { 
      printf("Error: can't compute hmac_sha1 as the input message is empty in compute_hmac_sha1().\n");
      return false;
    }

    if (nbytes == 0) {
      printf("Error: can't compute hmac_sha1 as the input length is invalid in compute_hmac_sha1().\n");
      return false;
    }

    if (key.size() == 0) {
      printf("Error: can't compute the hmac_sha1 as the key size is 0 in compute_hmac_sha1().\n");
      return false;
    }

    if (!output) {
      printf("Error: can't compute the hmac_sha as the output buffer is NULL in compute_hmac_sha1().\n");
      return false;
    }

    unsigned int len;
    // HMAC_CTX ctx;
    // HMAC_CTX_init(&ctx);

     HMAC_CTX *ctx = HMAC_CTX_new();

    if (!HMAC_Init_ex(ctx, (const unsigned char*)key.c_str(), key.size(), sz == 20 ? EVP_sha1():EVP_sha256(), NULL)) {
      printf("Error: cannot init the HMAC context in compute_hmac_sha1().\n");
    }

    HMAC_Update(ctx, (const unsigned char*)message, nbytes);
    HMAC_Final(ctx, output, &len);

    HMAC_CTX_free(ctx);

#if 1
    printf("stun::compute_hmac_sha1 - verbose: computing hash over %u bytes, using key `%s`:\n", nbytes, key.c_str());
    printf("-----------------------------------\n\t0: ");
    int nl = 0, lines = 0;
    for (int i = 0; i < nbytes; ++i, ++nl) {
      if (nl == 4) {
        printf("\n\t");
        nl = 0;
        lines++;
        printf("%d: ", lines);
      }
      printf("%02X ", message[i]);
    }
    printf("\n-----------------------------------\n");
#endif

#if 1
    printf("stun::compute_hmac_sha1 - verbose: computed hash: ");
    for(unsigned int i = 0; i < len; ++i) {
      printf("%02X ", output[i]);
    }
    printf("\n");
#endif

    return true;
    
  }

  int const_time_memcmp(unsigned char *a, unsigned char *b, size_t len) {
	const unsigned char *ca = a;
	const unsigned char *cb = b;
	unsigned char x = 0;
	for (size_t i = 0; i < len; i++)
		x |= ca[i] ^ cb[i];

	return x;
}

  static size_t align32(size_t len) {
	while (len & 0x03)
		++len;
	return len;
}

  
  size_t stun_update_header_length(void *buf, size_t length) {
	struct stun_header *header = (struct stun_header *)buf;
	size_t previous = ntohs(header->length);
	header->length = htons((uint16_t)length);
	return previous;
}
  
  bool compute_message_integrity(std::vector<uint8_t>& buffer, std::string key, int sz,  uint8_t* output) {

//    uint16_t dx = sz;
//    uint16_t offset = 0;
//    uint16_t len = 0;
//    uint16_t type = 0;
//    uint8_t curr_size[2];
//
//    int temp = buffer.size();
//    
//    if (!buffer.size()) {
//      printf("Error: cannot compute message integrity; buffer empty.\n");
//      return false;
//    }
//
//    if (!key.size()) {
//      printf("Error: cannot compute message inegrity, key empty.\n");
//      return false;
//    }
//
//    curr_size[0] = buffer[2];
//    curr_size[1] = buffer[3];
//    
//    while (dx < buffer.size()) {
//
//      type |= buffer[dx + 1] & 0x00FF;
//      type |= (buffer[dx + 0] << 8) & 0xFF00;
//      dx += 2;
//
//      len |= (buffer[dx + 1] & 0x00FF);
//      len |= (buffer[dx + 0] << 8) & 0xFF00;
//      dx += 2;
//
//      offset = dx;
//      dx += len;
//
//      /* skip padding. */
//      while ( (dx & 0x03) != 0 && dx < buffer.size()) {
//        dx++;
//      }
//
//      if (type == STUN_ATTR_MESSAGE_INTEGRITY ) {
//        break;
//      }
//
//      type = 0;
//      len = 0;
//    }
//
//    /* rewrite Message-Length header field */
//    buffer[2] = (offset >> 8) & 0xFF;
//    buffer[3] = offset & 0xFF;
//
//    /*
//      and compute the sha1 
//      we subtract the last 4 bytes, which are the attribute-type and
//      attribute-length of the Message-Integrity field which are not
//      used. 
//    */
//    if (!stun::compute_hmac_sha1(&buffer[0], offset - 4, key, sz, output)) {
//      buffer[2] = curr_size[0];
//      buffer[3] = curr_size[1];
//      return false;
//    }
//
//    /* rewrite message-length. */
//    buffer[2] = curr_size[0];
//    buffer[3] = curr_size[1];
      
      
      

	const struct stun_header *header = (const struct stun_header *)buffer.data();
	const size_t length = ntohs(header->length);
	if (buffer.size() < sizeof(struct stun_header) + length)
		return false;


	bool success = false;
	uint8_t *begin = buffer.data();
	const uint8_t *attr_begin = begin + sizeof(struct stun_header);
	const uint8_t *end = attr_begin + length;
	const uint8_t *pos = attr_begin;
	while (pos < end) {
		const struct stun_attr *attr = (const struct stun_attr *)pos;
		size_t attr_length = ntohs(attr->length);
		if (buffer.size() < sizeof(struct stun_attr) + attr_length)
			return false;

		uint16_t type = (uint16_t)ntohs(attr->type);
		switch (type) {
		case STUN_ATTR_MESSAGE_INTEGRITY: {
			if (attr_length != 20)
				return false;

			size_t tmp_length = pos - attr_begin + sizeof(struct stun_attr) + 20;
			size_t prev_length = stun_update_header_length(begin, tmp_length);
			uint8_t hmac[20];
                        
                        stun::compute_hmac_sha(begin, pos - begin, key, sz, hmac);
                        
			//hmac_sha1(begin, pos - begin, key, key_len, hmac);
			stun_update_header_length(begin, prev_length);

			//const uint8_t *expected_hmac = attr->value;
			if (const_time_memcmp(hmac, output, 20) != 0) {
				printf("STUN message integrity SHA1 check failed \n");
				return false;
			}

			success = true;
			break;
		}
		case STUN_ATTR_MESSAGE_INTEGRITY_SHA256: {
			if (attr_length != 32)
				return false;

			size_t tmp_length = pos - attr_begin + sizeof(struct stun_attr) + 32;
			size_t prev_length = stun_update_header_length(begin, tmp_length);
			uint8_t hmac[32];
			//hmac_sha256(begin, pos - begin, key, key_len, hmac);
                        stun::compute_hmac_sha(begin, pos - begin, key, sz, hmac);
			stun_update_header_length(begin, prev_length);

			
			if (const_time_memcmp(hmac, output, 32) != 0) {
				printf("STUN message integrity SHA256 check failed \n");
				return false;
			}

			success = true;
			break;
		}
		default:
			// Ignore
			break;
		}

		pos += sizeof(struct stun_attr) + align32(attr_length);
	}

	if (!success)
		return false;

	printf( "STUN message integrity check succeeded \n");
	return true;
}  
      
      
      


#define STUN_FINGERPRINT_XOR 0x5354554e

  bool compute_fingerprint(std::vector<uint8_t>& buffer, uint32_t& result) {

    uint32_t dx = 20;
    uint16_t offset = 0;
    uint16_t len = 0;  /* messsage-length */
    uint16_t type = 0;
    uint8_t curr_size[2];

    if (!buffer.size()) {
      printf("Error: cannot compute fingerprint because the buffer is empty.\n");
    }

    /* copy current message-length */
    curr_size[0] = buffer[2];
    curr_size[1] = buffer[3];

    /* compute the size that should be used as Message-Length when computing the CRC32 */
    while (dx < buffer.size()) {
      
      type |= buffer[dx + 1] & 0x00FF;
      type |= (buffer[dx + 0] << 8) & 0xFF00;
      dx += 2;

      len |= buffer[dx + 1] & 0x00FF;
      len |= (buffer[dx + 0] << 8) & 0xFF00;
      dx += 2;

      offset = dx;
      dx += len;

      /* skip padding. */
      while ( (dx & 0x03) != 0 && dx < buffer.size()) {
        dx++;
      }

      if (type == STUN_ATTR_FINGERPRINT) {
        break;
      }

      type = 0;
      len = 0;
    }

    /* rewrite message-length */
    offset -= 16;
    buffer[2] = (offset >> 8) & 0xFF;
    buffer[3] = offset & 0xFF;

    result = crc32(0L, &buffer[0], offset + 12) ^ STUN_FINGERPRINT_XOR;
    
    /* and reset the size */
    buffer[2] = curr_size[0];
    buffer[3] = curr_size[1];


    return true;
  }
  
  
  
void sha256(const std::string& str , std::string& key) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
//    std::stringstream ss;
//    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
//        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
//    }
  
    key= std::string(reinterpret_cast<char*>(hash), SHA256_DIGEST_LENGTH);
    
    printf("key = ");
    
    for (int k = 0; k < SHA256_DIGEST_LENGTH; ++k) {
         printf("%02X ", hash[k]);
    }
    printf("\n");
    return  ;
}


  void addressToString( char *buf,  uint16_t &port,  addr_record_t &mapped)
  {

    if(mapped.addr.ss_family == AF_INET6)
    {
        uv_ip6_name((sockaddr_in6* )&mapped.addr, buf, 39);
        port = ntohs( ((sockaddr_in6 *)&mapped.addr)->sin6_port);

    }
    else if(mapped.addr.ss_family  == AF_INET )
    {
         uv_ip4_name((sockaddr_in*)&mapped.addr, buf, 16);
         port =  ntohs( ((sockaddr_in *)&mapped.addr)->sin_port); 
    }
                        
    STrace << "stun::Reader - verbose: address: "<<  buf  << " port: " << port;
}


} /* namespace stun */
