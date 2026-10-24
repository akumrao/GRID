//#include <zlib.h>
#include <stdio.h>
#include <MessageStun.h>
#include <UtilStun.h>
#include <cstring>
#include <base/logger.h>


using namespace base;

/*
            STUN Attributes

            After the STUN header are zero or more attributes.  Each attribute
            MUST be TLV encoded, with a 16-bit type, 16-bit length, and value.
            Each STUN attribute MUST end on a 32-bit boundary.  As mentioned
            above, all fields in an attribute are transmitted most significant
            bit first.

                0                   1                   2                   3
                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
               |         Type                  |            Length             |
               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
               |                         Value (variable)                ....
               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

namespace stun {

    void stun_compute_userhash(const char *username, const char *realm, uint8_t *out) {
        char input[MAX_USERHASH_INPUT_LEN];
        int input_len = snprintf(input, MAX_USERHASH_INPUT_LEN, "%s:%s", username, realm);
        if (input_len < 0)
            return;

        if (input_len >= MAX_USERHASH_INPUT_LEN)
            input_len = MAX_USERHASH_INPUT_LEN - 1;

        hash_sha256(input, input_len, out);
    }

    void stun_process_credentials(const stun_credentials_t *credentials, stun_credentials_t *dst) {
        char username[STUN_MAX_USERNAME_LEN];
        strcpy(username, dst->username);
        *dst = *credentials;
        strcpy(dst->username, username);

        if (credentials->enable_userhash)
            stun_compute_userhash(username, credentials->realm, dst->userhash);
    }

    /* --------------------------------------------------------------------- */

    MessageStun::MessageStun(uint16_t type)
    : type(type)
    , length(0)
    , cookie(0x2112a442) {
        memset(&credentials, '\0', sizeof (stun_credentials_t));
        memset(&message_integrity, 0, sizeof (message_integrity));

        msg_class = (stun_class_t) (type & STUN_CLASS_MASK);
        msg_method = (stun_method_t) (type & ~STUN_CLASS_MASK);
    }

    MessageStun::MessageStun(stun_class_t msg_class, stun_method_t msg_method)
    : type((uint16_t) msg_class | (uint16_t) msg_method), msg_class(msg_class), msg_method(msg_method)
    , length(0)
    , cookie(0x2112a442) {
        memset(&credentials, '\0', sizeof (stun_credentials_t));
        memset(&message_integrity, 0, sizeof (message_integrity));
    }

    MessageStun::~MessageStun() {
    }

    void MessageStun::copyTransactionID(MessageStun* from) {

        if (!from) {
            printf("Error: tryign to copy a transaction ID, but the message is invalid in stun::MessageStun.\n");
            return;
        }

        //    transaction[0] = from->transaction[0];
        //    transaction[1] = from->transaction[1];
        //    transaction[2] = from->transaction[2];
    }

    void MessageStun::setRandomTransactionID() {
        //    transaction[0] = a;
        //    transaction[1] = b;
        //    transaction[2] = c;
        random_bytes(transaction_id, STUN_TRANSACTION_ID_SIZE);

#if PRINTDEBUG

        printf("MessageStun::Set msg  transactionids: ");
        for (int k = 0; k < STUN_TRANSACTION_ID_SIZE; ++k) {
            printf("%02X ", transaction_id[k]);
        }
        printf("\n");
#endif

    }

    std::string MessageStun::printTransactionID() {
        char id_str[STUN_TRANSACTION_ID_SIZE * 3 + 1] = {0};
        int offset = 0;
        for (int k = 0; k < STUN_TRANSACTION_ID_SIZE; ++k) {
            offset += snprintf(id_str + offset, sizeof (id_str) - offset, "%02X ", transaction_id[k]);
        }
        SDebug << "MessageStun transaction ID: " << id_str;
        return id_str;
    }

    void MessageStun::setTransactionID(uint8_t *transactionid) {
        memcpy(transaction_id, transactionid, STUN_TRANSACTION_ID_SIZE);
    }

    void MessageStun::setXorMappedAddress(const char * ip, uint16_t p) {
        IP::StringToAddress(ip, p, mapped);
    }

    void MessageStun::setIceControlled(uint64_t tie_breaker) {
        ice_controlled = tie_breaker;
    }

    void MessageStun::setIceControlling(uint64_t tie_breaker) {
        ice_controlling = tie_breaker;
    }

    void MessageStun::setPriority(uint32_t value) {
        priority = value;
    }

    void MessageStun::setUseCandidate(bool enable) {
        use_candidate = enable;
    }

#define HASH_MD5_SIZE 16
#define MAX_HMAC_KEY_LEN STUN_MAX_USERNAME_LEN

    size_t generate_hmac_key(MessageStun *msg, const char *password, void *key) {
        if (*msg->credentials.realm != '\0') {
            // long-term credentials
            if (*msg->credentials.username == '\0')
                SWarn << "Generating HMAC key for long-term credentials with empty STUN username";

            char input[MAX_HMAC_INPUT_LEN];
            int input_len = snprintf(input, MAX_HMAC_INPUT_LEN, "%s:%s:%s", msg->credentials.username,
                    msg->credentials.realm, password ? password : "");
            if (input_len < 0)
                return 0;

            if (input_len >= MAX_HMAC_INPUT_LEN)
                input_len = MAX_HMAC_INPUT_LEN - 1;

            switch (msg->credentials.password_algorithm) {
                case STUN_PASSWORD_ALGORITHM_SHA256:
                    hash_sha256(input, input_len, key);
                    return HASH_SHA256_SIZE;
                default:
                    hash_md5(input, input_len, key);
                    return HASH_MD5_SIZE;
            }
        } else {
            // short-term credentials
            int key_len = snprintf((char *) key, MAX_HMAC_KEY_LEN, "%s", password ? password : "");
            if (key_len < 0)
                return 0;

            if (key_len >= MAX_HMAC_KEY_LEN)
                key_len = MAX_HMAC_KEY_LEN - 1;

            return key_len;
        }
    }




} /* namespace stun */
