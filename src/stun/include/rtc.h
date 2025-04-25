
#ifndef RTC_C_API
#define RTC_C_API

#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef RTC_STATIC
#define RTC_C_EXPORT
#else // dynamic library
#ifdef _WIN32
#ifdef RTC_EXPORTS
#define RTC_C_EXPORT __declspec(dllexport) // building the library
#else
#define RTC_C_EXPORT __declspec(dllimport) // using the library
#endif
#else // not WIN32
#define RTC_C_EXPORT
#endif
#endif

#ifndef RTC_ENABLE_WEBSOCKET
#define RTC_ENABLE_WEBSOCKET 1
#endif

#ifndef RTC_ENABLE_MEDIA
#define RTC_ENABLE_MEDIA 1
#endif

#define RTC_DEFAULT_MTU 1280 // IPv6 minimum guaranteed MTU

#if RTC_ENABLE_MEDIA
#define RTC_DEFAULT_MAX_FRAGMENT_SIZE ((uint16_t)(RTC_DEFAULT_MTU - 12 - 8 - 40)) // SRTP/UDP/IPv6
#define RTC_DEFAULT_MAX_STORED_PACKET_COUNT 512
// Deprecated, do not use
#define RTC_DEFAULT_MAXIMUM_FRAGMENT_SIZE RTC_DEFAULT_MAX_FRAGMENT_SIZE
#define RTC_DEFAULT_MAXIMUM_PACKET_COUNT_FOR_NACK_CACHE RTC_DEFAULT_MAX_STORED_PACKET_COUNT
#endif

#ifdef _WIN32
#ifdef CAPI_STDCALL
#define RTC_API __stdcall
#else
#define RTC_API
#endif
#else // not WIN32
#define RTC_API
#endif

#if defined(__GNUC__) || defined(__clang__)
#define RTC_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define RTC_DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED
#endif

// libdatachannel C API

typedef enum {
	RTC_NEW = 0,
	RTC_CONNECTING = 1,
	RTC_CONNECTED = 2,
	RTC_DISCONNECTED = 3,
	RTC_FAILED = 4,
	RTC_CLOSED = 5
} rtcState;

typedef enum {
	RTC_ICE_NEW = 0,
	RTC_ICE_CHECKING = 1,
	RTC_ICE_CONNECTED = 2,
	RTC_ICE_COMPLETED = 3,
	RTC_ICE_FAILED = 4,
	RTC_ICE_DISCONNECTED = 5,
	RTC_ICE_CLOSED = 6
} rtcIceState;

typedef enum {
	RTC_GATHERING_NEW = 0,
	RTC_GATHERING_INPROGRESS = 1,
	RTC_GATHERING_COMPLETE = 2
} rtcGatheringState;

typedef enum {
	RTC_SIGNALING_STABLE = 0,
	RTC_SIGNALING_HAVE_LOCAL_OFFER = 1,
	RTC_SIGNALING_HAVE_REMOTE_OFFER = 2,
	RTC_SIGNALING_HAVE_LOCAL_PRANSWER = 3,
	RTC_SIGNALING_HAVE_REMOTE_PRANSWER = 4,
} rtcSignalingState;

typedef enum { // Don't change, it must match plog severity
	RTC_LOG_NONE = 0,
	RTC_LOG_FATAL = 1,
	RTC_LOG_ERROR = 2,
	RTC_LOG_WARNING = 3,
	RTC_LOG_INFO = 4,
	RTC_LOG_DEBUG = 5,
	RTC_LOG_VERBOSE = 6
} rtcLogLevel;

typedef enum {
	RTC_CERTIFICATE_DEFAULT = 0, // ECDSA
	RTC_CERTIFICATE_ECDSA = 1,
	RTC_CERTIFICATE_RSA = 2,
} rtcCertificateType;

typedef enum {
	RTC_DIRECTION_UNKNOWN = 0,
	RTC_DIRECTION_SENDONLY = 1,
	RTC_DIRECTION_RECVONLY = 2,
	RTC_DIRECTION_SENDRECV = 3,
	RTC_DIRECTION_INACTIVE = 4
} rtcDirection;


typedef enum { RTC_TRANSPORT_POLICY_ALL = 0, RTC_TRANSPORT_POLICY_RELAY = 1 } rtcTransportPolicy;

}

#endif
