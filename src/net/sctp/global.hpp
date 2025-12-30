

#ifndef RTC_GLOBAL_H
#define RTC_GLOBAL_H

#include "common.h"

#include <chrono>
#include <future>
#include <iostream>

namespace rtc {

enum class LogLevel { // Don't change, it must match plog severity
	None = 0,
	Fatal = 1,
	Error = 2,
	Warning = 3,
	Info = 4,
	Debug = 5,
	Verbose = 6
};

typedef std::function<void(LogLevel level, string message)> LogCallback;
//
RTC_CPP_EXPORT void InitLogger(LogLevel level, LogCallback callback = nullptr);
//
RTC_CPP_EXPORT void Preload();
RTC_CPP_EXPORT std::shared_future<void> Cleanup();

//struct SctpSettings {
//	// For the following settings, not set means optimized default
//	optional<size_t> recvBufferSize;                // in bytes
//	optional<size_t> sendBufferSize;                // in bytes
//	optional<size_t> maxChunksOnQueue;              // in chunks
//	optional<size_t> initialCongestionWindow;       // in MTUs
//	optional<size_t> maxBurst;                      // in MTUs
//	optional<unsigned int> congestionControlModule; // 0: RFC2581, 1: HSTCP, 2: H-TCP, 3: RTCC
//	optional<std::chrono::milliseconds> delayedSackTime;
//	optional<std::chrono::milliseconds> minRetransmitTimeout;
//	optional<std::chrono::milliseconds> maxRetransmitTimeout;
//	optional<std::chrono::milliseconds> initialRetransmitTimeout;
//	optional<unsigned int> maxRetransmitAttempts;
//	optional<std::chrono::milliseconds> heartbeatInterval;
//};


    struct SctpSettings {
	// For the following settings, not set means optimized default
	size_t recvBufferSize{1024 * 1024};                // in bytes
	size_t sendBufferSize{1024 * 1024};                // in bytes
	size_t maxChunksOnQueue{10 * 1024};              // in chunks
	size_t initialCongestionWindow{10};       // in MTUs
	size_t maxBurst{10};                      // in MTUs
	unsigned int congestionControlModule{0}; // 0: RFC2581, 1: HSTCP, 2: H-TCP, 3: RTCC
	std::chrono::milliseconds delayedSackTime{20};
	std::chrono::milliseconds minRetransmitTimeout{200};
	std::chrono::milliseconds maxRetransmitTimeout{10000};
	std::chrono::milliseconds initialRetransmitTimeout{1000};
	unsigned int maxRetransmitAttempts{5};
	std::chrono::milliseconds heartbeatInterval{10000};
};

RTC_CPP_EXPORT void SetSctpSettings(SctpSettings s);

RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, LogLevel level);

} // namespace rtc

#endif
