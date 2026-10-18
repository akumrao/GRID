#include <UtilStun.h>
#include <Writer.h>
//#include <arpa/inet.h> /* for inet_aton, not supported on windows, on win, use http://msdn.microsoft.com/en-us/library/cc805844%28VS.85%29.aspx */
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstring>

#include <base/logger.h>



using namespace base;

//#define TURN 1

namespace stun {

#ifndef htonll
#define htonll(x)                                                                                  \
        ((uint64_t)(((uint64_t)htonl((uint32_t)(x))) << 32) | (uint64_t)htonl((uint32_t)((x) >> 32)))
#endif
#ifndef ntohll
#define ntohll(x) htonll(x)
#endif

    int stun_write_value_mapped_address(void *buf, size_t size, const struct sockaddr *addr,
            socklen_t addrlen, const uint8_t *mask) {
        if (size < sizeof (struct stun_value_mapped_address))
            return -1;

        struct stun_value_mapped_address *value = (stun_value_mapped_address*) buf;
        value->padding = 0;
        switch (addr->sa_family) {
            case AF_INET:
            {
                value->family = STUN_ADDRESS_FAMILY_IPV4;
                if (size < sizeof (struct stun_value_mapped_address) + 4)
                    return -1;
                if (addrlen < (socklen_t)sizeof (struct sockaddr_in))
                    return -1;
                const struct sockaddr_in *sin = (const struct sockaddr_in *) addr;
                value->port = sin->sin_port ^ *((uint16_t *) mask);
                const uint8_t *bytes = (const uint8_t *) &sin->sin_addr;
                for (int i = 0; i < 4; ++i)
                    value->address[i] = bytes[i] ^ mask[i];
                return sizeof (struct stun_value_mapped_address) + 4;
            }
            case AF_INET6:
            {
                value->family = STUN_ADDRESS_FAMILY_IPV6;
                if (size < sizeof (struct stun_value_mapped_address) + 16)
                    return -1;
                if (addrlen < (socklen_t)sizeof (struct sockaddr_in6))
                    return -1;
                //PrintDebug("Writing IPv6 address");
                const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *) addr;
                value->port = sin6->sin6_port ^ *((uint16_t *) mask);
                const uint8_t *bytes = (const uint8_t *) &sin6->sin6_addr;
                for (int i = 0; i < 16; ++i)
                    value->address[i] = bytes[i] ^ mask[i];
                return sizeof (struct stun_value_mapped_address) + 16;
            }
            default:
            {
                SError << "Unknown address family %u", (unsigned int) addr->sa_family;
                return -1;
            }
        }
    }



    /*
      This function will write the message and apply the given `messageIntegrityPassword`
      to create the hmac-sha1 of the message. The whole message will be written into 
      the internal 'buffer' member. 
    
      This function also checks if there is an STUN_ATTR_FINGERPRINT attribute; when 
      found it will calcualte the CRC32 and put it into Writer::buffer.
     */

#define HMAC_SHA1_SIZE 20
#define HMAC_SHA256_SIZE 32
#define BUFFER_SIZE 4096
    int Writer::writeMessage(MessageStun* msg, const char * messageIntegrityPassword) {
        PrintDebug("stun::Writer - verbose: creating stun message with integrity password: `%s`\n", messageIntegrityPassword);
        

//        char buffer[BUFFER_SIZE];
//        int size = stun_write(buffer, BUFFER_SIZE, msg, NULL); // no password
//        if (size <= 0) {
//            SError << "STUN message write failed";
//            return -1;
//        }
//        
        buffer.resize(BUFFER_SIZE);
        int size = stun_write(buffer.data(), buffer.size(), msg, messageIntegrityPassword);
        if (size <= 0) {
            SError << "STUN message write failed";
            return -1;
        }
        buffer.resize(size); // Adjust vector size to the actual written message length
        

        /* first write all the attributes. */
//        writeMessage(msg);
//
//        /* Generate the dynamic HMAC key using the updated signature */
//        std::string hmac_key;
//        size_t keylen;
//        if (!messageIntegrityPassword.empty()) {
//            keylen = stun::generate_hmac_key(msg, messageIntegrityPassword, hmac_key);
//
//            char hex_buffer[2 * keylen + 1];
//
//            // 2. Loop through the key and format each byte
//            for (size_t i = 0; i < keylen; ++i) {
//                // %02x -> Lowercase hex, padded out to 2 characters (e.g., '0a')
//                // Multiply index by 2 to write into the correct offset of the string
//                sprintf(&hex_buffer[i * 2], "%02x", (unsigned char) hmac_key[i]);
//            }
//            SInfo << " hmac_key " << std::string(hex_buffer);
//        }
//
//        /* Check and compute SHA-1 message integrity if present */
//        MessageIntegrity* mit = NULL;
//        uint8_t sha1[20] = {0};
//        if (msg->find(STUN_ATTR_MESSAGE_INTEGRITY, &mit)) {
//            if (compute_message_integrity(buffer.data(), buffer.size(), hmac_key, keylen, sha1)) {
//                for (size_t i = 0; i < 20; ++i) {
//                    buffer[mit->offset + 4 + i] = sha1[i];
//                }
//            } else {
//                SError << "stun::Writer - warning: couldn't write the message integrity value in stun::Writer";
//                return -1;
//            }
//        }
//
//        /* Check and compute SHA-256 message integrity if present */
//        MessageIntegrity* mit_sha256 = NULL;
//        uint8_t sha256_digest[32] = {0};
//        if (msg->find(STUN_ATTR_MESSAGE_INTEGRITY_SHA256, &mit_sha256)) {
//            if (compute_message_integrity(buffer.data(), buffer.size(), hmac_key, keylen, sha256_digest)) {
//                for (size_t i = 0; i < 32; ++i) {
//                    buffer[mit_sha256->offset + 4 + i] = sha256_digest[i];
//                }
//            } else {
//                SError << "stun::Writer - warning: couldn't write the message integrity SHA-256 value in stun::Writer";
//                return -1;
//            }
//        }
//
//        /* when there is a fingerprint element we calc the crc too. */
//        Fingerprint* fp = NULL;
//        uint32_t crc = 0;
//        if (msg->find(STUN_ATTR_FINGERPRINT, &fp)) {
//            if (compute_fingerprint(buffer, crc)) {
//                PrintDebug("stun::Writer - verbose: Fingerprint: %x\n", crc);
//                rewriteU32(fp->offset + 4, crc);
//            } else {
//                SError << "stun::Writer - warning: couldn't write the message fingerprint in stun::Writer";
//                return -1;
//            }
//        }

        return 0;
    }




    int Writer::stun_write_header(void *buf, size_t size, stun_class_t class1, stun_method_t method, const uint8_t *transaction_id) {
        if (size < sizeof (struct stun_header))
            return -1;

        uint16_t type = (uint16_t) class1 | (uint16_t) method;

        struct stun_header *header = (struct stun_header *) buf;
        header->type = htons(type);
        header->length = htons(0);
        header->magic = htonl(STUN_MAGIC);
        memcpy(header->transaction_id, transaction_id, STUN_TRANSACTION_ID_SIZE);

        return sizeof (struct stun_header);
    }

    int stun_write_attr(void *buf, size_t size, uint16_t type, const void *value, size_t length) {
        STrace << "Writing STUN attribute type 0x" << std::hex << (unsigned int) type << std::dec << ", length=" << length;

        if (size < sizeof (struct stun_attr) +length)
            return -1;

        struct stun_attr *attr = (struct stun_attr *) buf;
        attr->type = htons(type);
        attr->length = htons((uint16_t) length);

        if (length > 0) {
            memcpy(attr->value, value, length);

            // Pad to align on 4 bytes
            while (length & 0x03)
                attr->value[length++] = 0;
        }

        return (int) (sizeof (struct stun_attr) +length);
    }

    const char *stun_get_error_reason(unsigned int code) {
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

    static size_t generate_password_algorithms_attr(uint8_t *attr) {
        // attr size must be at least STUN_PASSWORD_ALGORITHMS_ATTR_MAX_SIZE
        struct stun_value_password_algorithm *pwa = (struct stun_value_password_algorithm *) attr;
        pwa->algorithm = htons(STUN_PASSWORD_ALGORITHM_SHA256);
        pwa->parameters_length = 0;
        ++pwa;
        pwa->algorithm = htons(STUN_PASSWORD_ALGORITHM_MD5);
        pwa->parameters_length = 0;
        ++pwa;
        return (uint8_t *) pwa - attr;
    }

    
    int Writer::stun_write(void *buf, size_t size, const MessageStun *msg, const char *password) {
        uint8_t *begin = (uint8_t *) buf;
        uint8_t *pos = begin;
        uint8_t *end = begin + size;

        STrace << "Writing STUN message, class=0x" << std::hex << (unsigned int) msg->msg_class
                << ", method=0x" << (unsigned int) msg->msg_method << std::dec;

        size_t len =
                stun_write_header(pos, end - pos, msg->msg_class, msg->msg_method, msg->transaction_id);
        if (len <= 0) {
            SError << "Not enough space in buffer for STUN message, size=" << size;
            return -1;
        }
        pos += len;
        uint8_t *attr_begin = pos;

        if (msg->error_code) {
            const char *reason = stun_get_error_reason(msg->error_code);
            char buffer[sizeof (struct stun_value_error_code) +STUN_MAX_ERROR_REASON_LEN + 1];
            struct stun_value_error_code *error = (struct stun_value_error_code *) buffer;
            memset(error, 0, sizeof (*error));
            error->code_class = (msg->error_code / 100) & 0x07;
            error->code_number = msg->error_code % 100;
            strcpy((char *) error->reason, reason);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_ERROR_CODE, error,
                    sizeof (struct stun_value_error_code) +strlen(reason));
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->mapped.len) {
            STrace << "Writing XOR mapped address";
            uint8_t value[32];
            uint8_t mask[16];
            generate_mask(msg->transaction_id, mask);
            int value_len = stun_write_value_mapped_address(
                    value, 32, (const struct sockaddr *) &msg->mapped.addr, msg->mapped.len, mask);
            if (value_len > 0) {
                len = stun_write_attr(pos, end - pos, STUN_ATTR_XOR_MAPPED_ADDRESS, value, value_len);
                if (len <= 0) {
                    SError << "Not enough space in buffer for STUN message, size=" << size;
                    return -1;
                }
                pos += len;
            }
        }
        if (msg->priority) {
            uint32_t priority = htonl(msg->priority);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_PRIORITY, &priority, 4);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->use_candidate) {
            len = stun_write_attr(pos, end - pos, STUN_ATTR_USE_CANDIDATE, NULL, 0);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->ice_controlling) {
            uint64_t ice_controlling = htonll(msg->ice_controlling);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_ICE_CONTROLLING, &ice_controlling, 8);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->ice_controlled) {
            uint64_t ice_controlled = htonll(msg->ice_controlled);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_ICE_CONTROLLED, &ice_controlled, 8);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        

        #if TURN
        if (msg->channel_number) {
            struct stun_value_channel_number channel_number;
            memset(&channel_number, 0, sizeof (channel_number));
            channel_number.channel_number = htons(msg->channel_number);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_CHANNEL_NUMBER, &channel_number,
                    sizeof (channel_number));
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->lifetime_set || msg->lifetime) {
            uint32_t lifetime = htonl(msg->lifetime);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_LIFETIME, &lifetime, 4);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        for (size_t i = 0; i < msg->peers_size; ++i) {
            const addr_record_t *peer = msg->peers + i;
            if (peer->len) {
                STrace << "Writing XOR peer address";
                uint8_t value[32];
                uint8_t mask[16];
                generate_mask(msg->transaction_id, mask);
                int value_len = stun_write_value_mapped_address(
                        value, 32, (const struct sockaddr *) &peer->addr, peer->len, mask);
                if (value_len > 0) {
                    len = stun_write_attr(pos, end - pos, STUN_ATTR_XOR_PEER_ADDRESS, value, value_len);
                    if (len <= 0) {
                        SError << "Not enough space in buffer for STUN message, size=" << size;
                        return -1;
                    }
                    pos += len;
                }
            }
        }
        if (msg->relayed.len) {
            STrace << "Writing XOR relay address";
            uint8_t value[32];
            uint8_t mask[16];
            generate_mask(msg->transaction_id, mask);
            int value_len = stun_write_value_mapped_address(
                    value, 32, (const struct sockaddr *) &msg->relayed.addr, msg->relayed.len, mask);
            if (value_len > 0) {
                len = stun_write_attr(pos, end - pos, STUN_ATTR_XOR_RELAYED_ADDRESS, value, value_len);
                if (len <= 0) {
                    SError << "Not enough space in buffer for STUN message, size=" << size;
                    return -1;
                }
                pos += len;
            }
        }
        if (msg->data) {
            len = stun_write_attr(pos, end - pos, STUN_ATTR_DATA, (const uint8_t *) msg->data,
                    msg->data_size);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->even_port) {
            struct stun_value_even_port even_port;
            memset(&even_port, 0, sizeof (even_port));
            if (msg->next_port)
                even_port.r |= 0x80;
            len = stun_write_attr(pos, end - pos, STUN_ATTR_CHANNEL_NUMBER, &even_port,
                    sizeof (even_port));
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->requested_transport) {
            struct stun_value_requested_transport requested_transport;
            memset(&requested_transport, 0, sizeof (requested_transport));
            requested_transport.protocol = 17;
            len = stun_write_attr(pos, end - pos, STUN_ATTR_REQUESTED_TRANSPORT, &requested_transport,
                    sizeof (requested_transport));
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->dont_fragment) {
            len = stun_write_attr(pos, end - pos, STUN_ATTR_DONT_FRAGMENT, NULL, 0);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
        if (msg->reservation_token) {
            uint64_t reservation_token = htonll(msg->reservation_token);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_RESERVATION_TOKEN, &reservation_token, 8);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;
        }
#endif // end TUNRN

        const char *software = "libjuice";
        len = stun_write_attr(pos, end - pos, STUN_ATTR_SOFTWARE, software, strlen(software));
        if (len <= 0) {
            SError << "Not enough space in buffer for STUN message, size=" << size;
            return -1;
        }
        pos += len;

        if (msg->msg_class == STUN_CLASS_REQUEST) {
            if (msg->credentials.enable_userhash) {
                len = stun_write_attr(pos, end - pos, STUN_ATTR_USERHASH, msg->credentials.userhash,
                        USERHASH_SIZE);
                if (len <= 0) {
                    SError << "Not enough space in buffer for STUN message, size=" << size;
                    return -1;
                }
                pos += len;

            } else if (*msg->credentials.username != '\0') {
                len = stun_write_attr(pos, end - pos, STUN_ATTR_USERNAME, msg->credentials.username,
                        strlen(msg->credentials.username));
                if (len <= 0) {
                    SError << "Not enough space in buffer for STUN message, size=" << size;
                    return -1;
                }
                pos += len;
            }
        }
        if (msg->msg_class == STUN_CLASS_REQUEST ||
                (msg->msg_class == STUN_CLASS_RESP_ERROR &&
                (msg->error_code == 401 || msg->error_code == 438) // Unauthenticated or Stale Nonce
                )) {
            if (*msg->credentials.realm != '\0') {
                len = stun_write_attr(pos, end - pos, STUN_ATTR_REALM, msg->credentials.realm,
                        strlen(msg->credentials.realm));
                if (len <= 0) {
                    SError << "Not enough space in buffer for STUN message, size=" << size;
                    return -1;
                }
                pos += len;
            }
            if (*msg->credentials.nonce != '\0') {
                len = stun_write_attr(pos, end - pos, STUN_ATTR_NONCE, msg->credentials.nonce,
                        strlen(msg->credentials.nonce));
                if (len <= 0) {
                    SError << "Not enough space in buffer for STUN message, size=" << size;
                    return -1;
                }
                pos += len;

                if (msg->credentials.password_algorithm > 0) {
                    len = stun_write_attr(pos, end - pos, STUN_ATTR_PASSWORD_ALGORITHMS,
                            msg->credentials.password_algorithms_value,
                            msg->credentials.password_algorithms_value_size);
                    if (len <= 0) {
                        SError << "Not enough space in buffer for STUN message, size=" << size;
                        return -1;
                    }
                    pos += len;

                } else if (msg->msg_class != STUN_CLASS_REQUEST) {
                    uint8_t pwa_value[STUN_MAX_PASSWORD_ALGORITHMS_VALUE_SIZE];
                    size_t pwa_size = generate_password_algorithms_attr(pwa_value);
                    len = stun_write_attr(pos, end - pos, STUN_ATTR_PASSWORD_ALGORITHMS, pwa_value,
                            pwa_size);
                    if (len <= 0) {
                        SError << "Not enough space in buffer for STUN message, size=" << size;
                        return -1;
                    }
                    pos += len;
                }

                if (msg->msg_class == STUN_CLASS_REQUEST &&
                        msg->credentials.password_algorithm != STUN_PASSWORD_ALGORITHM_UNSET) {
                    struct stun_value_password_algorithm pwa;
                    pwa.algorithm = htons(msg->credentials.password_algorithm);
                    len = stun_write_attr(pos, end - pos, STUN_ATTR_PASSWORD_ALGORITHM, &pwa,
                            sizeof (pwa));
                    if (len <= 0) {
                        SError << "Not enough space in buffer for STUN message, size=" << size;
                        return -1;
                    }
                    pos += len;
                }
            }
        }
        if (msg->msg_class != STUN_CLASS_INDICATION && password) {
            uint8_t key[MAX_HMAC_KEY_LEN];
            size_t key_len = generate_hmac_key(msg, password, key);

            size_t tmp_length = pos - attr_begin + STUN_ATTR_SIZE + HMAC_SHA1_SIZE;
            stun_update_header_length(begin, tmp_length);

            uint8_t hmac[HMAC_SHA1_SIZE];
            hmac_sha1(begin, pos - begin, key, key_len, hmac);
            len = stun_write_attr(pos, end - pos, STUN_ATTR_MESSAGE_INTEGRITY, hmac, HMAC_SHA1_SIZE);
            if (len <= 0) {
                SError << "Not enough space in buffer for STUN message, size=" << size;
                return -1;
            }
            pos += len;

            // According to RFC 8489, the agent must include both MESSAGE-INTEGRITY and
            // MESSAGE-INTEGRITY-SHA256. However, this makes legacy agents and servers fail with error
            // 420 Unknown Attribute. Therefore, unless the password algorithm SHA-256 is enabled, only
            // MESSAGE-INTEGRITY is included in the message for compatibility.
            if (msg->credentials.password_algorithm != STUN_PASSWORD_ALGORITHM_UNSET) {
                // If the response contains a PASSWORD-ALGORITHMS attribute, all the
                // subsequent requests MUST be authenticated using MESSAGE-INTEGRITY-
                // SHA256 only.
                size_t tmp_length = pos - attr_begin + STUN_ATTR_SIZE + HMAC_SHA256_SIZE;
                stun_update_header_length(begin, tmp_length);

                uint8_t hmac[HMAC_SHA256_SIZE];
                hmac_sha256(begin, pos - begin, key, key_len, hmac);
                len = stun_write_attr(pos, end - pos, STUN_ATTR_MESSAGE_INTEGRITY_SHA256, hmac,
                        HMAC_SHA256_SIZE);
                if (len <= 0) {
                    SError << "Not enough space in buffer for STUN message, size=" << size;
                    return -1;
                }
                pos += len;
            }
        }

        size_t length = pos - attr_begin + STUN_ATTR_SIZE + 4;
        if (length & 0x03) {
            SError << "Written STUN message length is not multiple of 4, length=" << length;
            return -1;
        }
        stun_update_header_length(begin, length);

        uint32_t fingerprint = htonl(my_crc32(buf, pos - begin) ^ STUN_FINGERPRINT_XOR);
        len = stun_write_attr(pos, end - pos, STUN_ATTR_FINGERPRINT, &fingerprint, 4);
        if (len <= 0) {
            SError << "Not enough space in buffer for STUN message, size=" << size;
            return -1;
        }
        pos += len;

        return (int) (pos - begin);
    }

} /* namespace stun */
