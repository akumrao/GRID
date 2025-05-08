
#include "sdpcommon.h"

#include "base/logger.h"
#include <Agent.h>



using namespace base;
using namespace stun;
using namespace std::chrono_literals;
using std::chrono::system_clock;

namespace rtc {


    
    int ice_generate_candidate_sdp(const ice_candidate_t *candidate, char *buffer, size_t size) {
	const char *type = NULL;
	const char *suffix = NULL;
	switch (candidate->type) {
	case ICE_CANDIDATE_TYPE_HOST:
		type = "host";
		break;
	case ICE_CANDIDATE_TYPE_PEER_REFLEXIVE:
		type = "prflx";
		break;
	case ICE_CANDIDATE_TYPE_SERVER_REFLEXIVE:
		type = "srflx";
		suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	case ICE_CANDIDATE_TYPE_RELAYED:
		type = "relay";
		suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	default:
		SError << "Unknown candidate type";
		return -1;
	}
	return snprintf(buffer, size, "a=candidate:%s %u UDP %u %s %s typ %s%s%s",
	                candidate->foundation, candidate->component, candidate->priority,
	                candidate->hostname, candidate->service, type, suffix ? " " : "",
	                suffix ? suffix : "");
    }




} // namespace rtc::impl
