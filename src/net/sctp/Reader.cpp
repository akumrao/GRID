#include <string.h>
#include <Reader.h>
#include <UtilStun.h>
//#include <openssl/engine.h>
//#include <openssl/hmac.h>
//#include <openssl/evp.h>

#include <base/logger.h>


using namespace base;

namespace stun {

    /* --------------------------------------------------------------------- */

    static bool stun_validate_cookie(uint32_t cookie);
    static int stun_read_value_mapped_address(const void *data, size_t size, addr_record_t *mapped, const uint8_t *mask);

    /* --------------------------------------------------------------------- */

    Reader::Reader() {
    }

    /* @todo Reader::process - we can optimize this part by not copying but setting pointers to the  members of the MessageStun. */

#define STUN_SECURITY_PASSWORD_ALGORITHMS_BIT 0x01
#define STUN_SECURITY_USERNAME_ANONYMITY_BIT 0x02

    int Reader::process(uint8_t* data, uint32_t size, MessageStun* msg) {
        if (!data) {
            SError << "stun::Reader - error: received invalid data in Reader::process()";
            return -1;
        }
        if (!msg) {
            SError << "stun::Reader - error: invalid stun::MessageStun passed into Reader::process()";
            return -1;
        }

//       /* handle non-stun data (e.g. DTLS) */
//        if ((data[0] & 0xC0) != 0x00) {
//            return -1;
//        }

        if (size < sizeof (struct stun_header)) {
            SError << "STUN message too short, size=" << size;
            return -1;
        }





        const struct stun_header *header = (const struct stun_header *) data;
        const size_t length = ntohs(header->length);
        if (size < sizeof (struct stun_header) +length) {
            SError << "Invalid STUN message length, length=" << length << ", available=" << (size - sizeof (struct stun_header));
            return -1;
        }

        uint16_t type = ntohs(header->type);
        msg->type = type;
        msg->msg_class = (stun_class_t) (type & STUN_CLASS_MASK);
        msg->msg_method = (stun_method_t) (type & ~STUN_CLASS_MASK);
        msg->length = length;
        msg->cookie = ntohl(header->magic);
        memcpy(msg->transaction_id, header->transaction_id, STUN_TRANSACTION_ID_SIZE);

        STrace << "Reading STUN message, class=0x" << std::hex << (unsigned int) msg->msg_class
                << ", method=0x" << (unsigned int) msg->msg_method << std::dec;

        uint32_t security_bits = 0;
        uint8_t *begin = data;
        uint8_t *attr_begin = begin + sizeof (struct stun_header);
        uint8_t *end = attr_begin + length;
        const uint8_t *pos = attr_begin;

        while (pos < end) {
            // Calculate offset for safe attribute parsing
            //dx = pos - begin;
            int ret = stun_read_attr(pos, end - pos, msg, begin, attr_begin, &security_bits);
            if (ret <= 0) {
                STrace << "Reading STUN attribute failed";
                return -1;
            }
            pos += ret;
        }

        STrace << "Finished reading STUN attributes";

        stun_credentials_t *credentials = &msg->credentials;

        if (msg->msg_class == STUN_CLASS_RESP_ERROR &&
                (msg->error_code == 401 || msg->error_code == 438) &&
                security_bits & STUN_SECURITY_PASSWORD_ALGORITHMS_BIT &&
                credentials->password_algorithms_value_size == 0) {
            SInfo << "STUN Security Feature \"Password algorithms\" bit is set in " << msg->error_code
                    << " error response but the corresponding attribute is missing";
            msg->error_code = STUN_ERROR_INTERNAL_VALIDATION_FAILED;
        }

        if (!STUN_IS_RESPONSE(msg->msg_class)) {
            if (credentials->password_algorithms_value_size == 0 &&
                    credentials->password_algorithm == STUN_PASSWORD_ALGORITHM_UNSET) {
                credentials->password_algorithm = STUN_PASSWORD_ALGORITHM_MD5;
            } else if (credentials->password_algorithm == STUN_PASSWORD_ALGORITHM_UNSET) {
                SInfo << "No suitable password algorithm in STUN request";
                msg->error_code = STUN_ERROR_INTERNAL_VALIDATION_FAILED;
            } else if (credentials->password_algorithms_value_size == 0) {
                SInfo << "Missing password algorithms list in STUN request";
                msg->error_code = STUN_ERROR_INTERNAL_VALIDATION_FAILED;
            }
        }

        if (security_bits & STUN_SECURITY_USERNAME_ANONYMITY_BIT) {
            STrace << "Remote agent supports user anonymity";
            credentials->enable_userhash = true;
        }

        return (int) (sizeof (struct stun_header) +length);
    }

    int Reader::stun_read_attr(const void *data, size_t size, MessageStun *msg, uint8_t *begin, uint8_t *attr_begin, uint32_t *security_bits) {
        // RFC 8489: When present, the FINGERPRINT attribute MUST be the last attribute in the
        // message and thus will appear after MESSAGE-INTEGRITY and MESSAGE-INTEGRITY-SHA256.[cite: 4]
        if (msg->has_fingerprint) {
            STrace << "Invalid STUN attribute after fingerprint";
            return -1;
        }

        if (size < sizeof (struct stun_attr)) {
            STrace << "STUN attribute too short";
            return -1;
        }

        const struct stun_attr *attr = (const struct stun_attr *) data;
        size_t length = ntohs(attr->length);
        AttributeType type = (AttributeType) ntohs(attr->type);
        //STrace << "Reading attribute 0x" << std::hex << (unsigned int) type << std::dec << ", length=" << length;
        if (size < sizeof (struct stun_attr) +length) {
            STrace << "STUN attribute length invalid, length=" << length << ", available=" << (size - sizeof (struct stun_attr));
            return -1;
        }

        // RFC 8489: Note that agents MUST ignore all attributes that follow MESSAGE-INTEGRITY, with
        // the exception of the MESSAGE-INTEGRITY-SHA256 and FINGERPRINT attributes.[cite: 4]
        if (msg->has_integrity && type != STUN_ATTR_MESSAGE_INTEGRITY &&
                type != STUN_ATTR_MESSAGE_INTEGRITY_SHA256 && type != STUN_ATTR_FINGERPRINT) {
            STrace << "Ignoring STUN attribute 0x" << std::hex << (unsigned int) type << std::dec << " after message integrity";
            while (length & 0x03)
                ++length; // attributes are aligned on 4 bytes
            return (int) (sizeof (struct stun_attr) +length);
        }

        switch (type) {
            case STUN_ATTR_MAPPED_ADDRESS:
            {
                STrace << "Reading mapped address";
                uint8_t zero_mask[16] = {0};
                if (stun_read_value_mapped_address(attr->value, length, &msg->mapped, zero_mask) < 0)
                    return -1;
                break;
            }
            case STUN_ATTR_XOR_MAPPED_ADDRESS:
            {
                STrace << "Reading XOR mapped address";
                uint8_t mask[16];
                generate_mask(msg->transaction_id, mask);
                if (stun_read_value_mapped_address(attr->value, length, &msg->mapped, mask) < 0)
                    return -1;
                break;
            }

            case STUN_ATTR_ERROR_CODE:
            {
                STrace << "Reading error code";
                if (length < sizeof (struct stun_value_error_code)) {
                    STrace << "STUN error code value too short, length=" << length;
                    return -1;
                }
                const struct stun_value_error_code *error =
                        (const struct stun_value_error_code *) attr->value;
                msg->error_code = (error->code_class & 0x07) * 100 + error->code_number;

                if (msg->error_code == 401 || msg->error_code == 438) { // Unauthenticated or Stale Nonce
                    STrace << "Got STUN error code " << msg->error_code;
                } else {
                    size_t reason_length = length - sizeof (struct stun_value_error_code);
                    if (reason_length >= STUN_MAX_ERROR_REASON_LEN)
                        reason_length = STUN_MAX_ERROR_REASON_LEN - 1;

                    char err_buffer[STUN_MAX_ERROR_REASON_LEN];
                    memcpy(err_buffer, (const char *) error->reason, reason_length);
                    err_buffer[reason_length] = '\0';

                    SInfo << "Got STUN error code " << msg->error_code << ", reason \"" << err_buffer << "\"";
                }
                break;
            }
            case STUN_ATTR_UNKNOWN_ATTRIBUTES:
            {
                STrace << "Reading STUN unknown attributes";
                const uint16_t *attributes = (const uint16_t *) attr->value;
                for (int i = 0; i < (int) ntohs(attr->length) / 2; ++i) {
                    AttributeType unk_type = (AttributeType) ntohs(attributes[i]);
                    SInfo << "Got unknown attribute response for attribute 0x" << std::hex << (unsigned int) unk_type;
                }
                break;
            }
            case STUN_ATTR_USERNAME:
            {
                STrace << "Reading username";
                if (length + 1 > STUN_MAX_USERNAME_LEN) {
                    SWarn << "STUN username attribute value too long, length=" << length;
                    return -1;
                }
                memcpy(msg->credentials.username, (const char *) attr->value, length);
                msg->credentials.username[length] = '\0';
                STrace << "Got username: " << msg->credentials.username;
                break;
            }
            case STUN_ATTR_MESSAGE_INTEGRITY:
            {
                STrace << "Reading message integrity";
                if (length != HMAC_SHA1_SIZE) {
                    STrace << "STUN message integrity length invalid, length=" << length;
                    return -1;
                }
                msg->has_integrity = true;
                break;
            }
            case STUN_ATTR_MESSAGE_INTEGRITY_SHA256:
            {
                STrace << "Reading message integrity SHA256";
                if (length != HMAC_SHA256_SIZE) {
                    STrace << "STUN message integrity SHA256 length invalid, length=" << length;
                    return -1;
                }
                msg->has_integrity = true;
                break;
            }
            case STUN_ATTR_FINGERPRINT:
            {
                STrace << "Reading fingerprint";
                if (length != 4) {
                    STrace << "STUN fingerprint length invalid, length=" << length;
                    return -1;
                }
                size_t tmp_length = (uint8_t *) data - attr_begin + STUN_ATTR_SIZE + 4;
                size_t prev_length = stun_update_header_length(begin, tmp_length);
                uint32_t expected = my_crc32(begin, (uint8_t *) data - begin) ^ STUN_FINGERPRINT_XOR;
                stun_update_header_length(begin, prev_length);

                uint32_t fingerprint = ntohl(*((uint32_t *) attr->value));
                if (fingerprint != expected) {
                    SError << "STUN fingerprint check failed, expected=" << std::hex << expected << ", actual=" << fingerprint;
                    return -1;
                }
                STrace << "STUN fingerprint check succeeded";
                msg->has_fingerprint = true;
                msg->fingerprint_crc = fingerprint;
                break;
            }
            case STUN_ATTR_REALM:
            {
                STrace << "Reading realm";
                if (length + 1 > STUN_MAX_REALM_LEN) {
                    SWarn << "STUN realm attribute value too long, length=" << length;
                    return -1;
                }
                memcpy(msg->credentials.realm, (const char *) attr->value, length);
                msg->credentials.realm[length] = '\0';
                STrace << "Got realm: " << msg->credentials.realm;
                break;
            }
            case STUN_ATTR_NONCE:
            {
                STrace << "Reading nonce";
                if (length + 1 > STUN_MAX_NONCE_LEN) {
                    SWarn << "STUN nonce attribute value too long, length=" << length;
                    return -1;
                }
                memcpy(msg->credentials.nonce, (const char *) attr->value, length);
                msg->credentials.nonce[length] = '\0';
                STrace << "Got nonce: " << msg->credentials.nonce;

                if (STUN_IS_RESPONSE(msg->msg_class) &&
                        strlen(msg->credentials.nonce) > STUN_NONCE_COOKIE_LEN + 4 &&
                        strncmp(msg->credentials.nonce, STUN_NONCE_COOKIE, STUN_NONCE_COOKIE_LEN) == 0) {
                    char encoded_security_bits[5];
                    memcpy(encoded_security_bits, msg->credentials.nonce + STUN_NONCE_COOKIE_LEN, 4);
                    encoded_security_bits[4] = '\0';

                    uint8_t bytes[4];
                    bytes[0] = 0;
                    int len = BASE64_DECODE(encoded_security_bits, bytes + 1, 3);
                    if (len == 3) {
                        *security_bits = ntohl(*((uint32_t *) bytes));
                        STrace << "Nonce has cookie, Security Feature bits are 0x" << std::hex << *security_bits;
                    } else {
                        SWarn << "Nonce has cookie, but the encoded Security Feature bits field is invalid";
                        *security_bits = 0;
                    }
                } else if (msg->msg_class == STUN_CLASS_RESP_ERROR) {
                    STrace << "Remote agent does not support RFC 8489";
                }
                break;
            }
            case STUN_ATTR_PASSWORD_ALGORITHM:
            {
                STrace << "Reading password algorithm";
                if (length < sizeof (struct stun_value_password_algorithm)) {
                    SWarn << "STUN password algorithm value too short, length=" << length;
                    return -1;
                }
                if (!STUN_IS_RESPONSE(msg->msg_class)) {
                    const struct stun_value_password_algorithm *pwa =
                            (const struct stun_value_password_algorithm *) attr->value;
                    stun_password_algorithm_t algorithm = (stun_password_algorithm_t) ntohs(pwa->algorithm);
                    if (algorithm == STUN_PASSWORD_ALGORITHM_MD5 ||
                            algorithm == STUN_PASSWORD_ALGORITHM_SHA256)
                        msg->credentials.password_algorithm = algorithm;
                    else
                        SWarn << "Unknown password algorithm";
                } else {
                    SWarn << "Found password algorithm in response, ignoring";
                }
                break;
            }
            case STUN_ATTR_PASSWORD_ALGORITHMS:
            {
                STrace << "Reading password algorithms list";
                if (length < sizeof (struct stun_value_password_algorithm)) {
                    SWarn << "STUN password algorithms list too short, length=" << length;
                    return -1;
                }
                if (length > STUN_MAX_PASSWORD_ALGORITHMS_VALUE_SIZE) {
                    SWarn << "STUN password algorithms list too long, length=" << length;
                    return -1;
                }

                memcpy(msg->credentials.password_algorithms_value, attr->value, length);
                msg->credentials.password_algorithms_value_size = length;
                break;
            }
            case STUN_ATTR_USERHASH:
            {
                STrace << "Reading user hash";
                if (length != USERHASH_SIZE) {
                    SWarn << "STUN user hash value too long, length=" << length;
                    return -1;
                }
                memcpy(msg->credentials.userhash, attr->value, USERHASH_SIZE);
                msg->credentials.enable_userhash = true;
                break;
            }
            case STUN_ATTR_SOFTWARE:
            {
                STrace << "Reading software";
                if (length + 1 > STUN_MAX_SOFTWARE_LEN) {
                    SWarn << "STUN software attribute value too long, length=" << length;
                    return -1;
                }
                char sw_buffer[STUN_MAX_SOFTWARE_LEN];
                memcpy(sw_buffer, (const char *) attr->value, length);
                sw_buffer[length] = '\0';
                STrace << "Remote agent is \"" << sw_buffer << "\"";
                break;
            }
            case STUN_ATTR_PRIORITY:
            {
                STrace << "Reading priority";
                if (length != 4) {
                    STrace << "STUN priority length invalid, length=" << length;
                    return -1;
                }
                msg->priority = ntohl(*((uint32_t *) attr->value));
                STrace << "Got priority: " << msg->priority;
                break;
            }
            case STUN_ATTR_USE_CANDIDATE:
            {
                STrace << "Found use candidate flag";
                msg->use_candidate = true;
                break;
            }
            case STUN_ATTR_ICE_CONTROLLING:
            {
                STrace << "Found ICE controlling attribute";
                if (length != 8) {
                    STrace << "STUN ICE controlling attribute length invalid, length=" << length;
                    return -1;
                }
                uint32_t *value32 = (uint32_t *) attr->value;
                msg->ice_controlling = ((uint64_t) ntohl(value32[0]) << 32) | ntohl(value32[1]);
                break;
            }
            case STUN_ATTR_ICE_CONTROLLED:
            {
                STrace << "Found ICE controlled attribute";
                if (length != 8) {
                    STrace << "STUN ICE controlled attribute length invalid, length=" << length;
                    return -1;
                }
                uint32_t *value32 = (uint32_t *) attr->value;
                msg->ice_controlled = ((uint64_t) ntohl(value32[0]) << 32) | ntohl(value32[1]);
                break;
            }
            #if TURN

            case STUN_ATTR_ALTERNATE_SERVER:
            {
                STrace << "Reading alternate server";
                uint8_t zero_mask[16] = {0};
                if (stun_read_value_mapped_address(attr->value, length, &msg->alternate_server, zero_mask) < 0)
                    return -1;
                break;
            }
            case STUN_ATTR_CHANNEL_NUMBER:
            {
                STrace << "Reading channel number attribute";
                if (length < sizeof (struct stun_value_channel_number)) {
                    STrace << "STUN channel number attribute value too short, length=" << length;
                    return -1;
                }
                const struct stun_value_channel_number *channel_number =
                        (const struct stun_value_channel_number *) attr->value;
                msg->channel_number = ntohs(channel_number->channel_number);
                break;
            }
            case STUN_ATTR_LIFETIME:
            {
                STrace << "Reading lifetime attribute";
                if (length != 4) {
                    STrace << "STUN lifetime attribute length invalid, length=" << length;
                    return -1;
                }
                msg->lifetime = ntohl(*((uint32_t *) attr->value));
                msg->lifetime_set = true;
                break;
            }
            case STUN_ATTR_XOR_PEER_ADDRESS:
            {
                STrace << "Reading XOR peer address";
                if (msg->peers_size < STUN_MAX_PEER_ADDRESSES) {
                    uint8_t mask[16];
                    generate_mask(msg->transaction_id, mask);
                    addr_record_t *peer = msg->peers + msg->peers_size;
                    if (stun_read_value_mapped_address(attr->value, length, peer, mask) < 0)
                        return -1;
                    if (peer->len)
                        ++msg->peers_size;
                } else {
                    SWarn << "Too many STUN XOR-PEER-ADDRESS attributes, ignoring";
                }
                break;
            }
            case STUN_ATTR_XOR_RELAYED_ADDRESS:
            {
                STrace << "Reading XOR relayed address";
                uint8_t mask[16];
                generate_mask(msg->transaction_id, mask);
                if (stun_read_value_mapped_address(attr->value, length, &msg->relayed, mask) < 0)
                    return -1;
                break;
            }
            case STUN_ATTR_DATA:
            {
                STrace << "Found data";
                msg->data = (const char *) attr->value;
                msg->data_size = length;
                break;
            }
            case STUN_ATTR_EVEN_PORT:
            {
                STrace << "Found even port attribute";
                if (length < 1) {
                    STrace << "STUN even port attribute length invalid, length=" << length;
                    return -1;
                }
                msg->even_port = true;
                msg->next_port = ((struct stun_value_even_port *) attr->value)->r & 0x80;
                break;
            }
            case STUN_ATTR_REQUESTED_TRANSPORT:
            {
                STrace << "Found requested transport attribute";
                if (length < sizeof (struct stun_value_requested_transport)) {
                    STrace << "STUN requested transport attribute length invalid, length=" << length;
                    return -1;
                }
                const struct stun_value_requested_transport *requested_transport =
                        (const struct stun_value_requested_transport *) attr->value;
                if (requested_transport->protocol != 17) { // UDP
                    SWarn << "Unexpected requested transport protocol: " << (int) requested_transport->protocol;
                    return -1;
                }
                msg->requested_transport = true;
                break;
            }
            case STUN_ATTR_DONT_FRAGMENT:
            {
                STrace << "Found don't fragment attribute";
                msg->dont_fragment = true;
                break;
            }
            case STUN_ATTR_RESERVATION_TOKEN:
            {
                STrace << "Found reservation token";
                if (length != 8) {
                    STrace << "STUN reservation token length invalid, length=" << length;
                    return -1;
                }
                uint32_t *value32 = (uint32_t *) attr->value;
                msg->reservation_token = ((uint64_t) ntohl(value32[0]) << 32) | ntohl(value32[1]);
                break;
            }
#endif
            
            default:
            {
                if (STUN_IS_OPTIONAL_ATTR(type))
                    STrace << "Ignoring unknown optional STUN attribute type 0x" << std::hex << (unsigned int) type;
                else
                    SWarn << "Unknown STUN attribute type 0x" << std::hex << (unsigned int) type << ", ignoring";
                break;
            }
        }
        return (int) (sizeof (struct stun_attr) +align32(length));
    }



    //    unsigned char* Reader::getArray(uint16_t len) {
    //
    //        if (bytesLeft() < len) {
    //            LError("Error: trying to read a getArray from the buffer, but the buffer is not big enough.");
    //            return nullptr;
    //        }
    //        unsigned char* ret = ptr();
    //        dx += len;
    //        return ret;
    //    }

    static int stun_read_value_mapped_address(const void *data, size_t size, addr_record_t *mapped, const uint8_t *mask) {
        size_t len = sizeof (struct stun_value_mapped_address);
        if (size < len) {
            STrace << "STUN mapped address value too short, size=" << size;
            return -1;
        }
        const struct stun_value_mapped_address *value = (const struct stun_value_mapped_address *) data;
        stun_address_family_t family = (stun_address_family_t) value->family;
        switch (family) {
            case STUN_ADDRESS_FAMILY_IPV4:
            {
                len += 4;
                if (size < len) {
                    STrace << "IPv4 mapped address value too short, size=" << size;
                    return -1;
                }
                LTrace("Reading IPv4 address");
                mapped->len = sizeof (struct sockaddr_in);
                struct sockaddr_in *sin = (struct sockaddr_in *) &mapped->addr;
                sin->sin_family = AF_INET;
                sin->sin_port = value->port ^ *((uint16_t *) mask);
                uint8_t *bytes = (uint8_t *) & sin->sin_addr;
                for (int i = 0; i < 4; ++i)
                    bytes[i] = value->address[i] ^ mask[i];
                break;
            }
            case STUN_ADDRESS_FAMILY_IPV6:
            {
                len += 16;
                if (size < len) {
                    STrace << "IPv6 mapped address value too short, size=" << size;
                    return -1;
                }
                LTrace("Reading IPv6 address");
                mapped->len = sizeof (struct sockaddr_in6);
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &mapped->addr;
                sin6->sin6_family = AF_INET6;
                sin6->sin6_port = value->port ^ *((uint16_t *) mask);
                uint8_t *bytes = (uint8_t *) & sin6->sin6_addr;
                for (int i = 0; i < 16; ++i)
                    bytes[i] = value->address[i] ^ mask[i];
                break;
            }
            default:
            {
                STrace << "Unknown STUN address family 0x%X" << (unsigned int) family;
                len = size;
                break;
            }
        }
        return (int) len;
    }

    //    /* See http://tools.ietf.org/html/rfc5389#section-15.2 */
    //    bool Reader::readXorMappedAddress(MessageStun* msg, int len) {
    //
    //        if (bytesLeft() < 8) {
    //            LError("Error: cannot read a XorMappedAddress as the buffer is too small in stun::Reader");
    //            return false;
    //        }
    //
    //        uint8_t mask[16];
    //        *((uint32_t *) mask) = htonl(STUN_MAGIC);
    //        memcpy(mask + 4, msg->transaction_id, 12);
    //
    //        if (stun_read_value_mapped_address(getArray(len), len, &msg->mapped, mask) < 0)
    //            return false;
    //
    //        return true;
    //    }
    //
    //    uint8_t* Reader::ptr() {
    //        return &buffer[dx];
    //    }
    //




#define MAX_HMAC_KEY_LEN STUN_MAX_USERNAME_LEN

    // bool Reader::computeMessageIntegrity(MessageStun* msg, std::string password) {

    bool Reader::stun_check_integrity(void *buf, size_t size, const MessageStun *msg, const char *password) {

        if (!msg->has_integrity) {
            LError("Error: cannot compute the message integrity in stun::MessageStun because the message doesn't contain a MessageIntegrity attribute.");
            return false;
        }



        const struct stun_header *header = (const struct stun_header *) buf;
        const size_t length = ntohs(header->length);
        if (size < sizeof (struct stun_header) +length)
            return false;

        uint8_t key[MAX_HMAC_KEY_LEN];
        size_t key_len = generate_hmac_key(msg, password, key);

        bool success = false;
        uint8_t *begin = buf;
        const uint8_t *attr_begin = begin + sizeof (struct stun_header);
        const uint8_t *end = attr_begin + length;
        const uint8_t *pos = attr_begin;
        while (pos < end) {
            const struct stun_attr *attr = (const struct stun_attr *) pos;
            size_t attr_length = ntohs(attr->length);
            if (size < sizeof (struct stun_attr) +attr_length)
                return false;

            AttributeType type = (AttributeType) ntohs(attr->type);
            switch (type) {
                case STUN_ATTR_MESSAGE_INTEGRITY:
                {
                    if (attr_length != HMAC_SHA1_SIZE)
                        return false;

                    size_t tmp_length = pos - attr_begin + STUN_ATTR_SIZE + HMAC_SHA1_SIZE;
                    size_t prev_length = stun_update_header_length(begin, tmp_length);
                    uint8_t hmac[HMAC_SHA1_SIZE];
                    hmac_sha1(begin, pos - begin, key, key_len, hmac);
                    stun_update_header_length(begin, prev_length);

                    const uint8_t *expected_hmac = attr->value;
                    if (const_time_memcmp(hmac, expected_hmac, HMAC_SHA1_SIZE) != 0) {
                        STrace << "STUN message integrity SHA1 check failed";
                        return false;
                    }

                    success = true;
                    break;
                }
                case STUN_ATTR_MESSAGE_INTEGRITY_SHA256:
                {
                    if (attr_length != HMAC_SHA256_SIZE)
                        return false;

                    size_t tmp_length = pos - attr_begin + STUN_ATTR_SIZE + HMAC_SHA256_SIZE;
                    size_t prev_length = stun_update_header_length(begin, tmp_length);
                    uint8_t hmac[HMAC_SHA256_SIZE];
                    hmac_sha256(begin, pos - begin, key, key_len, hmac);
                    stun_update_header_length(begin, prev_length);

                    const uint8_t *expected_hmac = attr->value;
                    if (const_time_memcmp(hmac, expected_hmac, HMAC_SHA256_SIZE) != 0) {
                        STrace << "STUN message integrity SHA256 check failed";
                        return false;
                    }

                    success = true;
                    break;
                }
                default:
                    // Ignore
                    break;
            }

            pos += sizeof (struct stun_attr) +align32(attr_length);
        }

        if (!success)
            return false;

        return true;
    }


    /* --------------------------------------------------------------------- */

    static bool stun_validate_cookie(uint32_t cookie) {
        //PrintDebug("%02X:%02X:%02X:%02X\n", ptr[3], ptr[2], ptr[1], ptr[0]);
        uint8_t* ptr = (uint8_t*) & cookie;
        return (ptr[3] == 0x21
                && ptr[2] == 0x12
                && ptr[1] == 0xA4
                && ptr[0] == 0x42);

        return true;
    }

} /* namespace stun */
