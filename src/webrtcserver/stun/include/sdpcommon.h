

#ifndef RTC_SDP_COMMON_H
#define RTC_SDP_COMMON_H

#include "candidate.hpp"
#include "common.h"
//#include "description.h"
//#include "peerconnection.h"
#include <Connection.h>
#include "description.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>


#define ICE_PARSE_ERROR -1
#define ICE_PARSE_IGNORED -2
#define ICE_PARSE_MISSING_UFRAG -3
#define ICE_PARSE_MISSING_PWD -4



namespace rtc {
    
    int ice_type_suffix(const Candidate *candidate,  char **type , char **suffix  );
  
    int ice_generate_candidate_sdp(const Candidate *candidate, char *buffer, size_t size);
    
    int ice_generate_sdp(const ice_description_t *description, char *buffer, size_t size);
    
    int ice_parse_sdp(const char *sdp, ice_description_t *description);

     bool comp(Candidate a, Candidate b);

} // namespace rtc::impl

#endif
