
#include <stdio.h>
#include <Agent.h>
#include <uv.h>
#include "base/logger.h"
#include <Utils.h>

#include "sdpcommon.h"


using namespace base;





#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

namespace stun {

    /* --------------------------------------------------------------------- */

    Agent::Agent( Description &locadesp, candidate_callback &candidateCallback):locadesp(locadesp), mCandidateCallback(candidateCallback)
    {


    }

    Agent::~Agent() {

    }

    bool Agent::getInterfaces( int port) {

        char buf[512];
        uv_interface_address_t *info;
        int count, i;

        uv_interface_addresses(&info, &count);
        i = count;

        SInfo << "Number of interfaces: " <<  count;
        while (i--) {
            uv_interface_address_t interface_a = info[i];
        
            //SInfo << "Internal? %s\n", interface_a.is_internal ? "Yes" : "No");

            if(!interface_a.is_internal)
            {
                  SInfo << "Name: " <<  interface_a.name;
                  
                if (interface_a.address.address4.sin_family == AF_INET) {
                    uv_ip4_name(&interface_a.address.address4, buf, sizeof (buf));
                    ice_create_host_candidate(buf, port, AF_INET);
                    SInfo << "IPv4 address: " <<  buf;
                } else if (interface_a.address.address4.sin_family == AF_INET6) {
                    uv_ip6_name(&interface_a.address.address6, buf, sizeof (buf));
                    ice_create_host_candidate(buf, port, AF_INET6);
                    SInfo << "IPv6 address: " <<  buf;
                }
            }

        }

        uv_free_interface_addresses(info, count);
        return 0;

    }

    int Agent::ice_create_host_candidate( char *ip,  uint16_t port, int family ) {

        Candidate candidate;
        if (ice_create_local_candidate(ICE_CANDIDATE_TYPE_HOST, 1, locadesp.localCanSdp.candidates_count,  ip, port, family, &candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( &candidate, &locadesp   );
    }
    
    
    int Agent::ice_create_reflexive_candidate( char *ip,  uint16_t port, int family ) {

        Candidate candidate;
        if (ice_create_local_candidate(ICE_CANDIDATE_TYPE_SERVER_REFLEXIVE, 1, locadesp.localCanSdp.candidates_count,  ip, port, family, &candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( &candidate, &locadesp   );
    }

    int Agent::ice_create_local_candidate(ice_candidate_type_t type, int component, int index, char *ip,  uint16_t port, int family,  Candidate *candidate) {
        memset(candidate, 0, sizeof (*candidate));
        candidate->cand.type  = type;
        candidate->cand.component = component;
        //candidate->resolved = *record;
        strcpy(candidate->cand.foundation, "-");

        candidate->cand.priority = ice_compute_priority(candidate->cand.type, family, candidate->cand.component, index);
        
        strcpy (candidate->cand.hostname, ip);
        
        candidate->cand.port =  port;
        
//        if (getnameinfo((struct sockaddr *) &record->addr, record->len, candidate->hostname, 256,
//                candidate->service, 32, NI_NUMERICHOST | NI_NUMERICSERV | NI_DGRAM)) {
//            printf("getnameinfo failed, errno=%d", sockerrno);
//            return -1;
//        }
        return 0;
    }
    
    int Agent::ice_add_candidate(Candidate *candidate, Description *description) 
    {
        
        mCandidateCallback(*candidate);
        
	if (candidate->cand.type == ICE_CANDIDATE_TYPE_UNKNOWN)
		return -1;

	if (description->localCanSdp.candidates_count  >= ICE_MAX_CANDIDATES_COUNT) {
	        SError << "Description already has the maximum number of candidates";
		return -1;
	}

	if (strcmp(candidate->cand.foundation, "-") == 0)
		snprintf(candidate->cand.foundation, 32, "%u",
		         (unsigned int)(description->localCanSdp.candidates_count + 1));

	//ice_candidate_t *pos = description->candidates + description->localCanSdp.candidates_count;
	//*pos = *candidate;
        description->localCanSdp.candidates.push_back(candidate->cand);
        
	++description->localCanSdp.candidates_count;
	
        
        char buffer[4096];
        
        ice_generate_candidate_sdp(&candidate->cand, buffer, 4096);
        
        SInfo << buffer;
        return 0;

    }
    
    uint32_t Agent::ice_compute_priority(ice_candidate_type_t type, int family, int component, int index) {
	// Compute candidate priority according to RFC 8445
	// See https://www.rfc-editor.org/rfc/rfc8445.html#section-5.1.2.1
	uint32_t p = 0;

	switch (type) {
	case ICE_CANDIDATE_TYPE_HOST:
		p += ICE_CANDIDATE_PREF_HOST;
		break;
	case ICE_CANDIDATE_TYPE_PEER_REFLEXIVE:
		p += ICE_CANDIDATE_PREF_PEER_REFLEXIVE;
		break;
	case ICE_CANDIDATE_TYPE_SERVER_REFLEXIVE:
		p += ICE_CANDIDATE_PREF_SERVER_REFLEXIVE;
		break;
	case ICE_CANDIDATE_TYPE_RELAYED:
		p += ICE_CANDIDATE_PREF_RELAYED;
		break;
	default:
		break;
	}
	p <<= 16;

	switch (family) {
	case AF_INET:
		p += 32767;
		break;
	case AF_INET6:
		p += 65535;
		break;
	default:
		break;
	}
	p -= CLAMP(index, 0, 32767);
	p <<= 8;

	p += 256 - CLAMP(component, 1, 256);
	return p;
    }




} /* namespace stun */
