
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

    
   int ice_generate_candidate_sdp( Candidate *candidate, char *buffer, size_t size) {

       char *type = NULL;
	char *suffix = NULL;
       
       int ret = ice_type_suffix(candidate, &type, &suffix );
       if(ret < 0)
       {
           exit(0);
       }
       
       char *add =  (char *)candidate->address().c_str();
               
	return snprintf(buffer, size, "a=candidate:%s %u UDP %u %s %u typ %s%s%s",    candidate->mFoundation.c_str(), candidate->mComponent, candidate->mPriority, 
	                add , candidate->port(), type, suffix ? " " : "",
	                suffix ? suffix : "");
   }


       
    int ice_generate_sdp(const ice_description_t *description, char *buffer, size_t size) {
	if (!*description->ice_ufrag || !*description->ice_pwd)
		return -1;

	int len = 0;
	char *begin = buffer;
	char *end = begin + size;

	// Round 0: description
	// Round i with i>0 and i<count+1: candidate i-1
	// Round count + 1: end-of-candidates and ice-options lines
	for (int i = 0; i < description->candidates_count + 2; ++i) {
		int ret;
		if (i == 0) {
			ret = snprintf(begin, end - begin, "a=ice-ufrag:%s\r\na=ice-pwd:%s\r\n",
			               description->ice_ufrag, description->ice_pwd);
			if (description->ice_lite)
				ret = snprintf(begin, end - begin, "a=ice-lite\r\n");

		} else if (i < description->candidates_count + 1) {
			const Candidate *candidate = description->candidates + i - 1;
			if (candidate->mType == Candidate::Type::ServerReflexive ||
			    candidate->mType == Candidate::Type::ServerReflexive)
				continue;
			char tmp[4096];
			if (ice_generate_candidate_sdp((Candidate*)candidate, tmp, 4096) < 0)
				continue;
			ret = snprintf(begin, end - begin, "%s\r\n", tmp);
		} else { // i == description->candidates_count + 1
			// RFC 8445 10. ICE Option: An agent compliant to this specification MUST inform the
			// peer about the compliance using the 'ice2' option.
			if (description->finished)
				ret = snprintf(begin, end - begin, "a=end-of-candidates\r\na=ice-options:ice2\r\n");
			else
				ret = snprintf(begin, end - begin, "a=ice-options:ice2,trickle\r\n");
		}
		if (ret < 0)
			return -1;

		len += ret;

		if (begin < end)
			begin += ret >= end - begin ? end - begin - 1 : ret;
	}
	return len;
    }
  

    const char *skip_prefix(const char *str, const char *prefix) {
	size_t len = strlen(prefix);
	return strncmp(str, prefix, len) == 0 ? str + len : str;
    }
    
    bool match_prefix(const char *str, const char *prefix, const char **end) {
	*end = skip_prefix(str, prefix);
	return *end != str || !*prefix;
    }
    
    int parse_sdp_line(const char *line, ice_description_t *description)
    {
	const char *arg;
	if (match_prefix(line, "a=ice-ufrag:", &arg)) {
		sscanf(arg, "%256s", description->ice_ufrag);
		return 0;
	}
	if (match_prefix(line, "a=ice-pwd:", &arg)) {
		sscanf(arg, "%256s", description->ice_pwd);
		return 0;
	}
	if (match_prefix(line, "a=ice-lite", &arg)) {
		description->ice_lite = true;
		return 0;
	}
	if (match_prefix(line, "a=end-of-candidates", &arg)) {
		description->finished = true;
		return 0;
	}
	Candidate candidate;
        
        candidate.parse(line);
        
//        resolveIp((Candidate *)candidate);
//        
//	if (ice_parse_candidate_sdp(line, &candidate) == 0) {
//		ice_add_candidate(&candidate, description);  // arvind
//		return 0;
//	}
        
	return 0;
    }
    
    
    
    bool comp(Candidate a, Candidate b)
    {
        return a.priority() > b.priority();
    }
        
    int ice_parse_sdp(const char *sdp, ice_description_t *description)
    {
	memset(description, 0, sizeof(*description));
	description->ice_lite = false;
	description->candidates_count = 0;
	description->finished = false;

	char buffer[1024];
	size_t size = 0;
	while (*sdp) {
		if (*sdp == '\n') {
			if (size) {
				buffer[size++] = '\0';
				if (parse_sdp_line(buffer, description) == ICE_PARSE_ERROR)
					return ICE_PARSE_ERROR;

				size = 0;
			}
		} else if (*sdp != '\r' && size + 1 < 1024) {
			buffer[size++] = *sdp;
		}
		++sdp;
	}
	//ice_sort_candidates(description);
        
         std::sort(description->candidates,description->candidates +description->candidates_count , comp);

	STrace << "Parsed remote description: ufrag= " << description->ice_ufrag << " pwd= " << description->ice_pwd <<  " candidates= " <<  description->candidates_count;

	if (*description->ice_ufrag == '\0')
		return ICE_PARSE_MISSING_UFRAG;

	if (*description->ice_pwd == '\0')
		return ICE_PARSE_MISSING_PWD;

	return 0;
    }

} // namespace rtc::impl
