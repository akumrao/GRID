
#include "sdpcommon.h"
#include "base/logger.h"
#include <Agent.h>



using namespace base;
using namespace stun;
using namespace std::chrono_literals;
using std::chrono::system_clock;

namespace rtc {

//	enum class Type { Unknown, Host, ServerReflexive, PeerReflexive, Relayed };
    
    
    
     int ice_type_suffix(const Candidate *candidate,  char **type , char **suffix)
     {

	switch (candidate->mType) {
            case Candidate::Type::Host:
		*type = "host";
		break;
	case Candidate::Type::PeerReflexive:
		*type = "prflx";
		break;
	case Candidate::Type::ServerReflexive:
		*type = "srflx";
		*suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	case Candidate::Type::Relayed:
		*type = "relay";
		*suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	default:
		SError << "Unknown candidate type";
		return -1;
	}

        return 0;
    }

    
    int ice_generate_candidate_sdp(const Candidate *candidate, char *buffer, size_t size) {
	
        char *type = NULL;
	char *suffix = NULL;
        
        int ret = ice_type_suffix(candidate, &type, &suffix );
        
	return snprintf(buffer, size, "a=candidate:%s %u UDP %u %s %u typ %s%s%s",    candidate->mFoundation.c_str(), candidate->mComponent, candidate->mPriority, 
	                candidate->mAddress , candidate->mPort, type, suffix ? " " : "",
	                suffix ? suffix : "");
    }




} // namespace rtc::impl
