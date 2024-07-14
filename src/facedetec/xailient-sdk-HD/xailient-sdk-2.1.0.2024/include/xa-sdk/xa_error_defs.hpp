/**
* Copyright © 2024 Xailient Inc. All rights reserved
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Xailient Inc.
* No part of this file may be copied, modified, sold, and distributed in any
* form or by any means without prior explicit permission in writing from
* Xailient Inc.
*
*/
#ifndef XAILIENT_SDK_XA_ERROR_DEFS_HPP_
#define XAILIENT_SDK_XA_ERROR_DEFS_HPP_

namespace xailient {
namespace sdk {

/**
 * @brief Error definition
 *
 */
enum class ErrorCode {
    // no error happened
    XA_ERR_NONE = 0,
    // some of the arguments are invalid
    XA_ERR_INVALID_ARGS,
    // operation not permitted, when something was not properly initialized for example
    XA_ERR_INVALID_OP,
    // there was an internal error
    XA_ERR_INTERNAL,
    // we have reached a limit on the amount of items that can be registered
    XA_ERR_LIMIT,
    // not implemented error, this error is returned when certain features are not implemented
    // on the target platform
    XA_ERR_NOT_IMPLEMENTED,
    // not found based on arguments
    XA_ERR_NOT_FOUND,
    // FR disabled
    XA_ERR_FR_NOT_ENABLED,
    // Out of memory error
    XA_ERR_OUT_OF_MEMORY,
    // Malformed JSON
    XA_ERR_MALFORMED_JSON,
    // A JSON argument is missing a required field
    XA_ERR_MISSING_REQUIRED_FIELD
};

}
}


#endif // XAILIENT_SDK_XA_ERROR_DEFS_HPP_
