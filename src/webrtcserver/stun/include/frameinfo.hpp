

#ifndef RTC_FRAMEINFO_H
#define RTC_FRAMEINFO_H

#include "common.hpp"

namespace rtc {

struct RTC_CPP_EXPORT FrameInfo {
	FrameInfo(uint8_t payloadType, uint32_t timestamp) : payloadType(payloadType), timestamp(timestamp){};
	uint8_t payloadType; // Indicates codec of the frame
	uint32_t timestamp = 0; // RTP Timestamp
};

} // namespace rtc

#endif // RTC_FRAMEINFO_H
