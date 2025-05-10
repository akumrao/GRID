

#ifndef RTC_SDP_COMMON_H
#define RTC_SDP_COMMON_H

#include "candidate.h"
#include "common.h"
#include "description.h"
//#include "peerconnection.h"
#include <Connection.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace rtc {
	
    int ice_generate_candidate_sdp(const Candidate *candidate, char *buffer, size_t size);


} // namespace rtc::impl

#endif
