#include <Types.h>

namespace stun {

    /* --------------------------------------------------------------------- */

    std::string attribute_type_to_string(uint32_t t) {
        switch (t) {
            case STUN_ATTR_TYPE_NONE:
            {
                return "STUN_ATTR_TYPE_NONE";
            }
            case STUN_ATTR_MAPPED_ADDR:
            {
                return "STUN_ATTR_MAPPED_ADDR";
            }
            case STUN_ATTR_CHANGE_REQ:
            {
                return "STUN_ATTR_CHANGE_REQ";
            }
            case STUN_ATTR_USERNAME:
            {
                return "STUN_ATTR_USERNAME";
            }
            case STUN_ATTR_MESSAGE_INTEGRITY:
            {
                return "STUN_ATTR_MESSAGE_INTEGRITY";
            }
            case STUN_ATTR_ERR_CODE:
            {
                return "STUN_ATTR_ERR_CODE";
            }
            case STUN_ATTR_UNKNOWN_ATTRIBUTES:
            {
                return "STUN_ATTR_UNKNOWN_ATTRIBUTES";
            }
            case STUN_ATTR_CHANNEL_NUMBER:
            {
                return "STUN_ATTR_CHANNEL_NUMBER";
            }
            case STUN_ATTR_LIFETIME:
            {
                return "STUN_ATTR_LIFETIME";
            }
            case STUN_ATTR_XOR_PEER_ADDR:
            {
                return "STUN_ATTR_XOR_PEER_ADDR";
            }
            case STUN_ATTR_DATA:
            {
                return "STUN_ATTR_DATA";
            }
            case STUN_ATTR_REALM:
            {
                return "STUN_ATTR_REALM";
            }
            case STUN_ATTR_NONCE:
            {
                return "STUN_ATTR_NONCE";
            }
            case STUN_ATTR_XOR_RELAY_ADDRESS:
            {
                return "STUN_ATTR_XOR_RELAY_ADDRESS";
            }
            case STUN_ATTR_REQ_ADDRESS_FAMILY:
            {
                return "STUN_ATTR_REQ_ADDRESS_FAMILY";
            }
            case STUN_ATTR_EVEN_PORT:
            {
                return "STUN_ATTR_EVEN_PORT";
            }
            case STUN_ATTR_REQUESTED_TRANSPORT:
            {
                return "STUN_ATTR_REQUESTED_TRANSPORT";
            }
            case STUN_ATTR_DONT_FRAGMENT:
            {
                return "STUN_ATTR_DONT_FRAGMENT";
            }
            case STUN_ATTR_XOR_MAPPED_ADDRESS:
            {
                return "STUN_ATTR_XOR_MAPPED_ADDRESS";
            }
            case STUN_ATTR_RESERVATION_TOKEN:
            {
                return "STUN_ATTR_RESERVATION_TOKEN";
            }
            case STUN_ATTR_PRIORITY:
            {
                return "STUN_ATTR_PRIORITY";
            }
            case STUN_ATTR_USE_CANDIDATE:
            {
                return "STUN_ATTR_USE_CANDIDATE";
            }
            case STUN_ATTR_PADDING:
            {
                return "STUN_ATTR_PADDING";
            }
            case STUN_ATTR_RESPONSE_PORT:
            {
                return "STUN_ATTR_RESPONSE_PORT";
            }
            case STUN_ATTR_PASSWORD_ALGORITHMS:
            {
                return "STUN_ATTR_PASSWORD_ALGORITHMS";
            }
            case STUN_ATTR_ALTERNATE_DOMAIN:
            {
                return "STUN_ATTR_ALTERNATE_DOMAIN";
            }
            case STUN_ATTR_SOFTWARE:
            {
                return "STUN_ATTR_SOFTWARE";
            }
            case STUN_ATTR_ALTERNATE_SERVER:
            {
                return "STUN_ATTR_ALTERNATE_SERVER";
            }
            case STUN_ATTR_FINGERPRINT:
            {
                return "STUN_ATTR_FINGERPRINT";
            }
            case STUN_ATTR_ICE_CONTROLLED:
            {
                return "STUN_ATTR_ICE_CONTROLLED";
            }
            case STUN_ATTR_ICE_CONTROLLING:
            {
                return "STUN_ATTR_ICE_CONTROLLING";
            }
            case STUN_ATTR_RESPONSE_ORIGIN:
            {
                return "STUN_ATTR_RESPONSE_ORIGIN";
            }
            case STUN_ATTR_OTHER_ADDRESS:
            {
                return "STUN_ATTR_OTHER_ADDRESS";
            }
            case STUN_ATTR_USERHASH:
            {
                return "STUN_ATTR_USERHASH";
            }
            case STUN_ATTR_PASSWORD_ALGORITHM:
            {
                return "STUN_ATTR_PASSWORD_ALGORITHM";
            }
            case STUN_ATTR_MESSAGE_INTEGRITY_SHA256:
            {
                return "STUN_ATTR_MESSAGE_INTEGRITY_SHA256";
            }
            default:
            {
                return "unknown";
            }
        }
    }

    std::string message_type_to_string(uint32_t t) {
        switch (t) {
                // Binding
            case STUN_BINDING_REQUEST:
            {
                return "STUN_BINDING_REQUEST";
            }
            case STUN_BINDING_RESPONSE:
            {
                return "STUN_BINDING_RESPONSE";
            }
            case STUN_BINDING_ERROR_RESPONSE:
            {
                return "STUN_BINDING_ERROR_RESPONSE";
            }
            case STUN_BINDING_INDICATION:
            {
                return "STUN_BINDING_INDICATION";
            }

                // Allocate
            case 0x0003:
            {
                return "STUN_ALLOCATE_REQUEST";
            }
            case 0x0103:
            {
                return "STUN_ALLOCATE_RESPONSE";
            }
            case 0x0113:
            {
                return "STUN_ALLOCATE_ERROR_RESPONSE";
            }

                // Refresh
            case 0x0004:
            {
                return "STUN_REFRESH_REQUEST";
            }
            case 0x0104:
            {
                return "STUN_REFRESH_RESPONSE";
            }
            case 0x0114:
            {
                return "STUN_REFRESH_ERROR_RESPONSE";
            }

                // Send
            case 0x0006:
            {
                return "STUN_SEND_INDICATION";
            }

                // Data
            case 0x0007:
            {
                return "STUN_DATA_INDICATION";
            }

                // Create Permission
            case 0x0008:
            {
                return "STUN_CREATE_PERMISSION_REQUEST";
            }
            case 0x0108:
            {
                return "STUN_CREATE_PERMISSION_RESPONSE";
            }
            case 0x0118:
            {
                return "STUN_CREATE_PERMISSION_ERROR_RESPONSE";
            }

                // Channel Bind
            case 0x0009:
            {
                return "STUN_CHANNEL_BIND_REQUEST";
            }
            case 0x0109:
            {
                return "STUN_CHANNEL_BIND_RESPONSE";
            }
            case 0x0119:
            {
                return "STUN_CHANNEL_BIND_ERROR_RESPONSE";
            }

            default:
            {
                return "unknown";
            }
        }
    }

    /* ----------------------------------------------------------- */

} /* namespace stun */