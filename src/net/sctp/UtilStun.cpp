#include <stdio.h>

//#include <zlib.h>  /* for crc */
//#include <netinet/in.h>

#include <Types.h>
#include <UtilStun.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <uv.h>
#include <base/logger.h>

#if USE_MBEDTLS
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"
#else
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#endif

using namespace base;

namespace stun {




//#include <cstdint>
//#include <iomanip>
//#include <iostream>
//#include <string>
//#include <vector>
//
//    class ZlibCRC32 {
//    private:
//        uint32_t table[256];
//
//        void InitializeTable() {
//            const uint32_t polynomial = 0xEDB88320;
//            for (uint32_t i = 0; i < 256; i++) {
//                uint32_t remainder = i;
//                for (uint32_t j = 0; j < 8; j++) {
//                    if (remainder & 1) {
//                        remainder = (remainder >> 1) ^ polynomial;
//                    } else {
//                        remainder >>= 1;
//                    }
//                }
//                table[i] = remainder;
//            }
//        }
//
//    public:
//
//        ZlibCRC32() {
//            InitializeTable();
//        }
//
//        // Direct replacement for zlib's crc32() function
//
//        uint32_t Calculate(uint32_t crc, const uint8_t *buf, size_t len) const {
//            if (buf == nullptr) {
//                return 0; // Zlib returns 0 (the initial state) if buffer is null
//            }
//
//            // Zlib internal logic handles the inversion implicitly via the running crc
//            // variable
//            crc ^= 0xFFFFFFFF;
//            while (len--) {
//                crc = (crc >> 8) ^ table[(crc ^ *buf++) & 0xFF];
//            }
//            return crc ^ 0xFFFFFFFF;
//        }
//    };
//    ZlibCRC32 zlib;

    // int main() {
    //     ZlibCRC32 zlib;
    //
    //     // 1. Get initial CRC state (mimics zlib calling crc32(0, NULL, 0))
    //     uint32_t crc = zlib.Calculate(0, nullptr, 0);
    //
    //     // 2. Feed data in chunks (Streaming support)
    //     std::string chunk1 = "The quick brown fox ";
    //     crc = zlib.Calculate(crc, reinterpret_cast<const
    //     uint8_t*>(chunk1.c_str()), chunk1.length());
    //
    //     std::string chunk2 = "jumps over the lazy dog";
    //     crc = zlib.Calculate(crc, reinterpret_cast<const
    //     uint8_t*>(chunk2.c_str()), chunk2.length());
    //
    //     // Print final result
    //     std::cout << "Zlib CRC-32: 0x"
    //               << std::uppercase << std::hex << std::setw(8) <<
    //               std::setfill('0')
    //               << crc << " (Expected: 0x414FA339)\n";
    //
    //     return 0;
    // }


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
        if ((size_t) ret < size) {
            printf("getrandom returned too few bytes, size=%zu, returned=%zu", size, (size_t) ret);
            return -1;
        }
        return 0;
    }

#elif defined(_WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 // Windows 7
#endif

#include <windows.h>
#include <bcrypt.h>

    int random_bytes(void *buf, size_t size) {
        // Requires Windows 7 or later
        NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR) buf, (ULONG) size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        return !status ? 0 : -1;
    }

#else
    // static int random_bytes(void *buf, size_t size) {
    // 	(void)buf;
    // 	(void)size;
    // 	return -1;
}
#endif

static unsigned int generate_seed() {
#ifdef _WIN32
    return (unsigned int) GetTickCount();
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
        return (unsigned int) (ts.tv_sec ^ ts.tv_nsec);
    else
        return (unsigned int) time(NULL);
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

uint32_t rand32(void) {
    uint32_t r = 0;
    random_bytes(&r, sizeof (r));
    return r;
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

#if USE_MBEDTLS


    // const char* key_data = "my_secret_key";
    // const char* message_data = "Hello, world!";

    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = sz == 20 ? MBEDTLS_MD_SHA1 : MBEDTLS_MD_SHA256;
    //   unsigned char hmac_result[MBEDTLS_MD_MAX_SIZE];
    int ret;

    // 1. Initialize the MD context
    mbedtls_md_init(&ctx);

    // 2. Setup the HMAC algorithm (SHA256)
    ret = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    if (ret != 0) {
        SError << "mbedtls_md_setup failed with error " << ret;
        return false;
    }

    // 3. Start the HMAC process with the key
    ret = mbedtls_md_hmac_starts(&ctx, (const unsigned char*) key.c_str(), key.length());
    if (ret != 0) {
        SError << "mbedtls_md_hmac_starts failed with error " << ret;
        goto exit;
    }

    // 4. Update the HMAC with the message data
    ret = mbedtls_md_hmac_update(&ctx, (const unsigned char*) message, nbytes);
    if (ret != 0) {
        SError << "mbedtls_md_hmac_update failed with error " << ret;
        goto exit;
    }

    // 5. Finish the HMAC and get the result
    ret = mbedtls_md_hmac_finish(&ctx, output);
    if (ret != 0) {
        SError << "mbedtls_md_hmac_finish failed with error " << ret;
        goto exit;
    }

    //    printf("HMAC-SHA256 result: ");
    //    for (int i = 0; i < mbedtls_md_get_size(mbedtls_md_info_from_type(md_type)); i++) {
    //        printf("%02x", hmac_result[i]);
    //    }
    //    printf("\n");

exit:
    // 6. Free the MD context
    mbedtls_md_free(&ctx);

    return true;


#else


    HMAC_CTX *ctx = HMAC_CTX_new();

    if (!HMAC_Init_ex(ctx, (const unsigned char*) key.c_str(), key.size(), sz == 20 ? EVP_sha1() : EVP_sha256(), NULL)) {
        printf("Error: cannot init the HMAC context in compute_hmac_sha1().\n");
    }

    HMAC_Update(ctx, (const unsigned char*) message, nbytes);
    HMAC_Final(ctx, output, &len);

    HMAC_CTX_free(ctx);

#endif





#if PRINTDEBUG
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

#if PRINTDEBUG
    printf("stun::compute_hmac_sha1 - verbose: computed hash: ");
    for (unsigned int i = 0; i < len; ++i) {
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

 size_t align32(size_t len) {
    while (len & 0x03)
        ++len;
    return len;
}

size_t stun_update_header_length(void *buf, size_t length) {
    struct stun_header *header = (struct stun_header *) buf;
    size_t previous = ntohs(header->length);
    header->length = htons((uint16_t) length);
    return previous;
}

//bool compute_message_verify(unsigned char *buf, size_t size, std::string key, int keylen, uint8_t *integSha) {
//
//    std::vector<uint8_t> output(keylen);
//    if (!compute_message_integrity(buf, size, key, keylen, output.data())) {
//        return false;
//    }
//
//    if (const_time_memcmp(integSha, output.data(), keylen) != 0) {
//        SError << "STUN message integrity SHA1 check failed";
//        return false;
//    }
//
//    return true;
//}

//bool compute_message_integrity(unsigned char *buf, size_t size, std::string key, int keylen, uint8_t* output) {
//
//
//
//
//    const struct stun_header *header = (const struct stun_header *) buf;
//    const size_t length = ntohs(header->length);
//    if (size < sizeof (struct stun_header) +length)
//        return false;
//
//
//    bool success = false;
//    uint8_t *begin = (uint8_t *) buf;
//    const uint8_t *attr_begin = begin + sizeof (struct stun_header);
//    const uint8_t *end = attr_begin + length;
//    const uint8_t *pos = attr_begin;
//    while (pos < end) {
//        const struct stun_attr *attr = (const struct stun_attr *) pos;
//        size_t attr_length = ntohs(attr->length);
//        if (size < sizeof (struct stun_attr) +attr_length)
//            return false;
//
//        uint16_t type = (uint16_t) ntohs(attr->type);
//        switch (type) {
//            case STUN_ATTR_MESSAGE_INTEGRITY:
//            {
//                if (attr_length != 20)
//                    return false;
//
//                size_t tmp_length = pos - attr_begin + sizeof (struct stun_attr) + 20;
//                size_t prev_length = stun_update_header_length(begin, tmp_length);
//                //uint8_t hmac[20];
//
//
//                stun::compute_hmac_sha(begin, pos - begin, key, keylen, output);
//
//
//                //hmac_sha1(begin, pos - begin, key, key_len, hmac);
//                stun_update_header_length(begin, prev_length);
//
//                //const uint8_t *expected_hmac = attr->value;
//                //			if (const_time_memcmp(hmac, output, 20) != 0) {
//                //				printf("STUN message integrity SHA1 check failed \n");
//                //				return false;
//                //			}
//
//                success = true;
//                break;
//            }
//            case STUN_ATTR_MESSAGE_INTEGRITY_SHA256:
//            {
//                if (attr_length != 32)
//                    return false;
//
//                size_t tmp_length = pos - attr_begin + sizeof (struct stun_attr) + 32;
//                size_t prev_length = stun_update_header_length(begin, tmp_length);
//                //uint8_t hmac[32];
//                //hmac_sha256(begin, pos - begin, key, key_len, hmac);
//                stun::compute_hmac_sha(begin, pos - begin, key, keylen, output);
//                stun_update_header_length(begin, prev_length);
//
//
//                //			if (const_time_memcmp(hmac, output, 32) != 0) {
//                //				printf("STUN message integrity SHA256 check failed \n");
//                //				return false;
//                //			}
//
//                success = true;
//                break;
//            }
//            default:
//                // Ignore
//                break;
//        }
//
//        pos += sizeof (struct stun_attr) +align32(attr_length);
//    }
//
//    if (!success)
//        return false;
//
//    ///printf( "STUN message integrity check succeeded \n");
//    return true;
//}







void hash_md5(const void *message, size_t size, void *digest) {
    if (!message || !digest) {
        return;
    }

    unsigned char *hash = static_cast<unsigned char *> (digest);

#if USE_MBEDTLS
    // mbedTLS MD5 calculation
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_MD5);
    if (md_info) {
        mbedtls_md(md_info, static_cast<const unsigned char *> (message), size, hash);
    } else {
        printf("Error: hash_md5\n");
        exit(0);
    }
#else
    // OpenSSL MD5 calculation
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, message, size);
    MD5_Final(hash, &md5);
#endif
}

void hash_sha256(const void *message, size_t size, void *digest) {
    if (!message || !digest) {
        return;
    }

    unsigned char *hash = static_cast<unsigned char *> (digest);

#if USE_MBEDTLS
    mbedtls_sha256_context ctx;
    // 1. Initialize the SHA-256 context
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 indicates SHA-256 (not SHA-224)
    // 2. Update the context with the input data
    mbedtls_sha256_update(&ctx, static_cast<const unsigned char*> (message), size);
    // 3. Finish the hash calculation and write to digest
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
#else
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, message, size);
    SHA256_Final(hash, &sha256);
#endif
}

void sha256(const std::string& str, std::string& key) {
    unsigned char hash[32];

    // Delegate hash generation to hash_sha256
    hash_sha256(str.data(), str.size(), hash);

    key = std::string(reinterpret_cast<char*> (hash), 32);
}

void generate_mask(const uint8_t *transaction_id, uint8_t *mask) {
    // mask is 16 bytes
    const uint32_t magic = htonl(STUN_MAGIC);
    memcpy(mask, (const uint8_t*) &magic, 4);
    memcpy(mask + 4, transaction_id, 12);
}

int BASE64_DECODE(const char *str, void *out, size_t out_size) {
	const uint8_t *in = (const uint8_t *)str;
	uint8_t *w = (uint8_t *)out;
	while (*in && *in != '=') {
		uint8_t tab[4] = {0, 0, 0, 0};
		size_t size = 0;
		while (*in && size < 4) {
			uint8_t c = *in++;
			if (isspace(c))
				continue;
			if (c == '=')
				break;

			if ('A' <= c && c <= 'Z')
				tab[size++] = c - 'A';
			else if ('a' <= c && c <= 'z')
				tab[size++] = c + 26 - 'a';
			else if ('0' <= c && c <= '9')
				tab[size++] = c + 52 - '0';
			else if (c == '+' || c == '-')
				tab[size++] = 62;
			else if (c == '/' || c == '_')
				tab[size++] = 63;
			else
				return -1; // Invalid character
		}

		if (size > 0) {
			if (out_size < size - 1)
				return -1;

			out_size -= size - 1;

			*w++ = (tab[0] << 2) | (tab[1] >> 4);
			if (size > 1) {
				*w++ = (tab[1] << 4) | (tab[2] >> 2);
				if (size > 2)
					*w++ = (tab[2] << 6) | tab[3];
			}
		}
	}

	return (int)(w - (uint8_t *)out);
}




void hmac_sha1(const void *message, size_t size, const void *key, size_t key_size, void *digest) {
    if (!message || !key || !digest) {
        return;
    }

#if USE_MBEDTLS
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    
    if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 1) != 0) {
        mbedtls_md_free(&ctx);
        return;
    }
    
    if (mbedtls_md_hmac_starts(&ctx, static_cast<const unsigned char*>(key), key_size) != 0 ||
        mbedtls_md_hmac_update(&ctx, static_cast<const unsigned char*>(message), size) != 0 ||
        mbedtls_md_hmac_finish(&ctx, static_cast<unsigned char*>(digest)) != 0) {
        // Handle error if needed
    }
    
    mbedtls_md_free(&ctx);
#else
    unsigned int len = 0;
    HMAC_CTX *ctx = HMAC_CTX_new();
    if (!ctx) return;

    if (HMAC_Init_ex(ctx, key, key_size, EVP_sha1(), NULL)) {
        HMAC_Update(ctx, static_cast<const unsigned char*>(message), size);
        HMAC_Final(ctx, static_cast<unsigned char*>(digest), &len);
    }
    
    HMAC_CTX_free(ctx);
#endif
}

void hmac_sha256(const void *message, size_t size, const void *key, size_t key_size, void *digest) {
    if (!message || !key || !digest) {
        return;
    }

#if USE_MBEDTLS
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    
    if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1) != 0) {
        mbedtls_md_free(&ctx);
        return;
    }
    
    if (mbedtls_md_hmac_starts(&ctx, static_cast<const unsigned char*>(key), key_size) != 0 ||
        mbedtls_md_hmac_update(&ctx, static_cast<const unsigned char*>(message), size) != 0 ||
        mbedtls_md_hmac_finish(&ctx, static_cast<unsigned char*>(digest)) != 0) {
        // Handle error if needed
    }
    
    mbedtls_md_free(&ctx);
#else
    unsigned int len = 0;
    HMAC_CTX *ctx = HMAC_CTX_new();
    if (!ctx) return;

    if (HMAC_Init_ex(ctx, key, key_size, EVP_sha256(), NULL)) {
        HMAC_Update(ctx, static_cast<const unsigned char*>(message), size);
        HMAC_Final(ctx, static_cast<unsigned char*>(digest), &len);
    }
    
    HMAC_CTX_free(ctx);
#endif
}



#define CRC32_REVERSED_POLY 0xEDB88320
#define CRC32_INIT 0xFFFFFFFF
#define CRC32_XOR 0xFFFFFFFF



static uint32_t crc32_byte(uint32_t crc) {
	for (int i = 0; i < 8; ++i)
		if (crc & 1)
			crc = (crc >> 1) ^ CRC32_REVERSED_POLY;
		else
			crc = (crc >> 1);
	return crc;
}

static uint32_t crc32_table(const uint8_t *p, size_t size, uint32_t *table) {
	uint32_t crc = CRC32_INIT;
	while (size--)
		crc = table[(uint8_t)(crc & 0xFF) ^ *p++] ^ (crc >> 8);
	return crc ^ CRC32_XOR;
}

 uint32_t my_crc32(const void *data, size_t size) {
	static uint32_t table[256] = {0};
	if (table[255] == 0)
		for (uint32_t i = 0; i < 256; ++i)
			table[i] = crc32_byte(i);

	return crc32_table(data, size, table);
}



} /* namespace stun */
