
#include "sdpcommon.h"

#include "base/logger.h"
#include <Agent.h>



using namespace base;
using namespace stun;
using namespace std::chrono_literals;
using std::chrono::system_clock;

namespace rtc {

//	enum class Type { Unknown, Host, ServerReflexive, PeerReflexive, Relayed };
    
    int ice_generate_candidate_sdp(const Candidate *candidate, char *buffer, size_t size) {
	const char *type = NULL;
	const char *suffix = NULL;
	switch (candidate->mType) {
            case Candidate::Type::Host:
		type = "host";
		break;
	case Candidate::Type::PeerReflexive:
		type = "prflx";
		break;
	case Candidate::Type::ServerReflexive:
		type = "srflx";
		suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	case Candidate::Type::Relayed:
		type = "relay";
		suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	default:
		SError << "Unknown candidate type";
		return -1;
	}
	return snprintf(buffer, size, "a=candidate:%s %u UDP %u %s %u typ %s%s%s",    candidate->mFoundation, candidate->mComponent, candidate->mPriority, 
	                candidate->mAddress , candidate->mPort, type, suffix ? " " : "",
	                suffix ? suffix : "");
    }




} // namespace rtc::impl
