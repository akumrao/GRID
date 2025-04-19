

#ifndef RTC_IMPL_INTERNALS_H
#define RTC_IMPL_INTERNALS_H

#include "common.hpp"
#include "base/logger.h"
#include <cstdint>
#include <cstring>
#include <iostream>
 #include <cctype>

using namespace std;
using namespace base;

// Disable warnings before including plog
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif



#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace rtc {

const size_t MAX_NUMERICNODE_LEN = 48; // Max IPv6 string representation length
const size_t MAX_NUMERICSERV_LEN = 6;  // Max port string representation length

const uint16_t DEFAULT_SCTP_PORT = 5000; // SCTP port to use by default

const uint16_t MAX_SCTP_STREAMS_COUNT = 1024; // Max number of negotiated SCTP streams
                                              // RFC 8831 recommends 65535 but usrsctp needs a lot
                                              // of memory, Chromium historically limits to 1024.

const size_t DEFAULT_LOCAL_MAX_MESSAGE_SIZE = 256 * 1024; // Default local max message size
const size_t DEFAULT_REMOTE_MAX_MESSAGE_SIZE = 65536;     // Remote max message size if not in SDP

const size_t DEFAULT_WS_MAX_MESSAGE_SIZE = 256 * 1024;   // Default max message size for WebSockets

const size_t RECV_QUEUE_LIMIT = 1024; // Max per-channel queue size (messages)

const int MIN_THREADPOOL_SIZE = 4; // Minimum number of threads in the global thread pool (>= 2)

const size_t DEFAULT_MTU = RTC_DEFAULT_MTU; // defined in rtc.h

} // namespace rtc



// #define STrace LogStream(Level::Trace, _fileName(__FILE__), __LINE__)
// #define SDebug LogStream(Level::Debug, _fileName(__FILE__), __LINE__)
// #define SInfo  LogStream(Level::Info, _fileName(__FILE__), __LINE__)
// #define SWarn  LogStream(Level::Warn, _fileName(__FILE__), __LINE__)
// #define SError LogStream(Level::Error, _fileName(__FILE__), __LINE__)
// #define SRTrace LogStream(Level::Remote, _fileName(__FILE__), __LINE__)


#define PLOG_VERBOSE STrace
#define PLOG_WARNING SWarn
#define PLOG_ERROR SError
#define PLOG_INFO SInfo
#define PLOG_DEBUG SDebug
#define LOG_WARNING SWarn
#define LOG_VERBOSE STrace
#define PLOG_FATAL SError
#define PLOG(severity) SError


namespace plog
{
    enum Severity
    {
        none = 0,
        fatal = 1,
        error = 2,
        warning = 3,
        info = 4,
        debug = 5,
        verbose = 6
    };

#ifdef _MSC_VER
#   pragma warning(suppress: 26812) //  Prefer 'enum class' over 'enum'
#endif
    inline const char* severityToString(Severity severity)
    {
        switch (severity)
        {
        case fatal:
            return "FATAL";
        case error:
            return "ERROR";
        case warning:
            return "WARN";
        case info:
            return "INFO";
        case debug:
            return "DEBUG";
        case verbose:
            return "VERB";
        default:
            return "NONE";
        }
    }

    inline Severity severityFromString(const char* str)
    {
        switch (std::toupper(str[0]))
        {
        case 'F':
            return fatal;
        case 'E':
            return error;
        case 'W':
            return warning;
        case 'I':
            return info;
        case 'D':
            return debug;
        case 'V':
            return verbose;
        default:
            return none;
        }
    }



 // Severity get()
 // {

 // }



};

#endif