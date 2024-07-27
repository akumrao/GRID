/**
* Copyright © 2024 Xailient Inc. All rights reserved
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Xailient Inc.
* No part of this file may be copied, modified, sold, and distributed in any
* form or by any means without prior explicit permission in writing from
* Xailient Inc.
*
*/
#ifndef XAILIENT_SDK_XA_LOGGING_DEFS_HPP_
#define XAILIENT_SDK_XA_LOGGING_DEFS_HPP_

namespace xailient {
namespace sdk {


/**
 * @brief The different type of logs that the sdk can emit (check xa_fi_log_cb_t)
 *
 */
enum class LogType {
  XA_LOG_TYPE_ALWAYS = 0,
  XA_LOG_TYPE_ERROR = 1,
  XA_LOG_TYPE_WARN = 2,
  XA_LOG_TYPE_INFO = 3,
  XA_LOG_TYPE_DEBUG = 4,
  XA_LOG_TYPE_TRACE = 5
};

/**
 * @brief Defines the callback signature for receiving log messages
 *
 */
typedef void(*LogCallbackType)(LogType, const char*);

/**
 * @brief Convert to string the log type
 *
 * @param logType
 * @return constexpr const char*
 */
static constexpr const char*
toString(LogType logType)
{
    switch(logType) {
    case LogType::XA_LOG_TYPE_ALWAYS: return "ALWAYS";
    case LogType::XA_LOG_TYPE_ERROR: return "ERROR";
    case LogType::XA_LOG_TYPE_WARN: return "WARN";
    case LogType::XA_LOG_TYPE_INFO: return "INFO";
    case LogType::XA_LOG_TYPE_DEBUG: return "DEBUG";
    case LogType::XA_LOG_TYPE_TRACE: return "TRACE";
    }
    return "??";
}

}
}

#endif // XAILIENT_SDK_XA_LOGGING_DEFS_HPP_
