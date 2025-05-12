
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

    Agent::Agent( Description &locadesp, candidate_callback candidateCallback):locadesp(locadesp), mCandidateCallback(candidateCallback)
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
        
        candidate.mType = Candidate::Type::Host;
        
        if (ice_create_local_candidate( 1, locadesp.localCanSdp.candidates_count,  ip, port, family, &candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( &candidate, &locadesp   );
    }
    
    
    int Agent::ice_create_reflexive_candidate( char *ip,  uint16_t port, int family ) {

        Candidate candidate;
         candidate.mType = Candidate::Type::ServerReflexive;
         
        if (ice_create_local_candidate(1, locadesp.localCanSdp.candidates_count,  ip, port, family, &candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( &candidate, &locadesp   );
    }

    int Agent::ice_create_local_candidate( int component, int index, char *ip,  uint16_t port, int family,  Candidate *candidate) {
      //  memset(candidate, 0, sizeof (*candidate));

        candidate->mComponent = component;
        //candidate->resolved = *record;
        candidate->mFoundation =  "-";

        candidate->mPriority  = ice_compute_priority(candidate->mType, family, candidate->mComponent, index);
        
        candidate->mAddress =  ip;
        
        candidate->mPort=  port;
        
         candidate->mFamily = family;
        
//        if (getnameinfo((struct sockaddr *) &record->addr, record->len, candidate->hostname, 256,
//                candidate->service, 32, NI_NUMERICHOST | NI_NUMERICSERV | NI_DGRAM)) {
//            printf("getnameinfo failed, errno=%d", sockerrno);
//            return -1;
//        }
        return 0;
    }
    
    int Agent::ice_add_candidate(Candidate *candidate, Description *description) 
    {
        
       
        
//	if (candidate->cand.type == ICE_CANDIDATE_TYPE_UNKNOWN)
//		return -1;

	if (description->localCanSdp.candidates_count  >= ICE_MAX_CANDIDATES_COUNT) {
	        SError << "Description already has the maximum number of candidates";
		return -1;
	}

	if (candidate->mFoundation == "-")
		candidate->mFoundation = std::to_string(description->localCanSdp.candidates_count + 1);


	//ice_candidate_t *pos = description->candidates + description->localCanSdp.candidates_count;
	//*pos = *candidate;
        description->localCanSdp.candidates.push_back(*candidate);
        
	++description->localCanSdp.candidates_count;
	
        
        char buffer[4096];
        
        ice_generate_candidate_sdp(candidate, buffer, 4096);
        
        SInfo << buffer;
        
        mCandidateCallback(*candidate);
        
        return 0;

    }
    //	enum class Type { Unknown, Host, ServerReflexive, PeerReflexive, Relayed };
    uint32_t Agent::ice_compute_priority(Candidate::Type type, int family, int component, int index) {
	// Compute candidate priority according to RFC 8445
	// See https://www.rfc-editor.org/rfc/rfc8445.html#section-5.1.2.1
	uint32_t p = 0;

	switch (type) {
            case Candidate::Type::Host:
		p += ICE_CANDIDATE_PREF_HOST;
		break;
	case Candidate::Type::ServerReflexive:
		p += ICE_CANDIDATE_PREF_PEER_REFLEXIVE;
		break;
	case Candidate::Type::PeerReflexive:
		p += ICE_CANDIDATE_PREF_SERVER_REFLEXIVE;
		break;
	case Candidate::Type::Relayed:
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
