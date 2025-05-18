
#include <stdio.h>
#include <Agent.h>
#include <uv.h>
#include "base/logger.h"
#include <Utils.h>

#include "sdpcommon.h"


using namespace base;





#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

namespace stun {

    
    int64_t current_timestamp() {
#ifdef _WIN32
	return (timestamp_t)GetTickCount();
#else // POSIX
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts))
		return 0;
	return (int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000;
#endif
}
    /* --------------------------------------------------------------------- */

    Agent::Agent( Description &localdesp, Description &remotedesp,  candidate_callback candidateCallback):localdesp(localdesp), remotedesp(remotedesp), mCandidateCallback(candidateCallback)
    {

        int x = 1;
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
        
        if (ice_create_local_candidate( 1, localdesp.desc.candidates_count,  ip, port, family, &candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( &candidate, &localdesp   );
    }
    
    
    int Agent::ice_create_reflexive_candidate( char *ip,  uint16_t port, int family ) {

        Candidate candidate;
        candidate.mType = Candidate::Type::ServerReflexive;
         
        if (ice_create_local_candidate(1, localdesp.desc.candidates_count,  ip, port, family, &candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( &candidate, &localdesp   );
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
    
    
    
    int Agent::ice_remote_candidate(const Candidate *candidate)
    {
        
        ice_add_candidate( (Candidate *)candidate, &remotedesp  );
        
        
        if (agent_add_candidate_pairs_for_remote((Candidate *)candidate)) {
         	LWarn("Failed to add candidate pair");
		return -2;
	}
        
    } 
    
    
    int Agent::ice_add_candidate( Candidate *candidate, Description *description) 
    {
        
       
        
//	if (candidate->cand.type == ICE_CANDIDATE_TYPE_UNKNOWN)
//		return -1;

	if (description->desc.candidates_count  >= ICE_MAX_CANDIDATES_COUNT) {
	        SError << "Description already has the maximum number of candidates";
		return -1;
	}
        
        candidate->mMid = localMid;

	if (candidate->mFoundation == "-")
		candidate->mFoundation = std::to_string(description->desc.candidates_count + 1);


	//ice_candidate_t *pos = description->candidates + description->localCanSdp.candidates_count;
	//*pos = *candidate;
        description->desc.candidates.push_back(*candidate);
        
	++description->desc.candidates_count;
	
        
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
    
    
    
    
    
    
    
    void Agent::agent_update_ordered_pairs() 
    {
	LTrace("Updating ordered candidate pairs");
	for (int i = 0; i < candidate_pairs_count; ++i) {
		ice_candidate_pair_t **begin = ordered_pairs;
		ice_candidate_pair_t **end = begin + i;
		ice_candidate_pair_t **prev = end;
		uint64_t priority = candidate_pairs[i].priority;
		while (--prev >= begin && (*prev)->priority < priority)
			*(prev + 1) = *prev;

		*(prev + 1) = candidate_pairs + i;
	}
    }

    
    
    int  Agent::agent_add_candidate_pairs_for_remote( Candidate *remote) 
    {
	// Here is the trick: local non-relayed candidates are undifferentiated for sending.
	// Therefore, we don't need to match remote candidates with local ones.
	if (agent_add_candidate_pair( NULL, remote))
		return -1;

//	// However, we need still to differenciate local relayed candidates
//	for (int i = 0; i < local.candidates_count; ++i) {
//		Candidate *local = local.candidates + i;
//		if (local->type == ICE_CANDIDATE_TYPE_RELAYED &&
//		    local->resolved.addr.ss_family == remote->resolved.addr.ss_family)
//			if (agent_add_candidate_pair( local, remote))
//				return -1;
//	}

	return 0;
    }
    
    
    
    
    int Agent::ice_create_candidate_pair(Candidate *local, Candidate *remote, bool is_controlling,
                                  ice_candidate_pair_t *pair) { // local or remote might be NULL
            if (local && remote && local->resolved.addr.ss_family != remote->resolved.addr.ss_family) {
                    LError("Mismatching candidates address families");
                    return -1;
            }

            memset(pair, 0, sizeof(*pair));
            pair->local = local;
            pair->remote = remote;
            pair->state = ICE_CANDIDATE_PAIR_STATE_FROZEN;
            return ice_update_candidate_pair(pair, is_controlling);
    }

    int Agent::ice_update_candidate_pair(ice_candidate_pair_t *pair, bool is_controlling) {
            // Compute pair priority according to RFC 8445, extended to support generic pairs missing local
            // or remote See https://www.rfc-editor.org/rfc/rfc8445.html#section-6.1.2.3
            if (!pair->local && !pair->remote)
                    return 0;
            uint64_t local_priority =
                pair->local
                    ? pair->local->mPriority
                    : ice_compute_priority( Candidate::Type::Host, pair->remote->resolved.addr.ss_family,
                                           pair->remote->mComponent , 0);
            uint64_t remote_priority =
                pair->remote
                    ? pair->remote->mPriority
                    : ice_compute_priority( Candidate::Type::Host, pair->local->resolved.addr.ss_family,
                                           pair->local->mComponent, 0);
            uint64_t g = is_controlling ? local_priority : remote_priority;
            uint64_t d = is_controlling ? remote_priority : local_priority;
            uint64_t min = g < d ? g : d;
            uint64_t max = g > d ? g : d;
            pair->priority = (min << 32) + (max << 1) + (g > d ? 1 : 0);
            return 0;
    }

    int Agent::ice_candidates_count(const ice_description_t *description, Candidate::Type type ) 

    {
            int count = 0;
            for (int i = 0; i < description->candidates_count; ++i) {
                    const Candidate *candidate = &description->candidates [i];
                    if (candidate->mType  == type)
                            ++count;
            }
            return count;
    }


    
    int  Agent::agent_add_candidate_pair( Candidate *local, // local may be NULL
                             Candidate *remote) {
	ice_candidate_pair_t pair;
	bool is_controlling = mode == AGENT_MODE_CONTROLLING;
	if (ice_create_candidate_pair(local, remote, is_controlling, &pair)) {
		LError("Failed to create candidate pair");
		return -1;
	}

	if (candidate_pairs_count >= MAX_CANDIDATE_PAIRS_COUNT) {
		LWarn("Session already has the maximum number of candidate pairs");
		return -1;
	}

	//LTrace("Adding new candidate pair, priority=%" PRIu64, pair.priority);

	// Add pair
	ice_candidate_pair_t *pos = candidate_pairs + candidate_pairs_count;
	*pos = pair;
	++candidate_pairs_count;

	agent_update_ordered_pairs();

	if (entries_count == MAX_STUN_ENTRIES_COUNT) {
		LWarn("No free STUN entry left for candidate pair checking");
		return -1;
	}

//	agent_stun_entry_t *relay_entry = NULL;
//	if (local && local->type == ICE_CANDIDATE_TYPE_RELAYED) {
//		for (int i = 0; i < agent->entries_count; ++i) {
//			agent_stun_entry_t *other_entry = agent->entries + i;
//			if (other_entry->type == AGENT_STUN_ENTRY_TYPE_RELAY &&
//			    addr_record_is_equal(&other_entry->relayed, &local->resolved, true)) {
//				relay_entry = other_entry;
//				break;
//			}
//		}
//		if (!relay_entry) {
//			LError("Relay entry not found");
//			return -1;
//		}
//	}

	STrace << "Registering STUN entry  " <<   entries_count << " for candidate pair checking";
	agent_stun_entry_t *entry = entries + entries_count;
	entry->type = AGENT_STUN_ENTRY_TYPE_CHECK;
	entry->state = AGENT_STUN_ENTRY_STATE_IDLE;
	entry->mode = AGENT_MODE_UNKNOWN;
	entry->pair = pos;
	entry->record = pos->remote->resolved;
	//entry->relay_entry = relay_entry;
	random_bytes(entry->transaction_id, STUN_TRANSACTION_ID_SIZE);
	entry->transaction_id_expired = false;
	++entries_count;
//
//	if (remote->mType  == Candidate::Type::Host)
//		agent_translate_host_candidate_entry( entry);

	if (mode == AGENT_MODE_CONTROLLING) {
		for (int i = 0; i < candidate_pairs_count; ++i) {
			ice_candidate_pair_t *ordered_pair = ordered_pairs[i];
			if (ordered_pair == pos) {
				LTrace("Candidate pair has priority");
				break;
			}
			if (ordered_pair->state == ICE_CANDIDATE_PAIR_STATE_SUCCEEDED) {
				// We found a succeeded pair with higher priority, ignore this one
				LTrace("Candidate pair doesn't have priority, keeping it frozen");
				return 0;
			}
		}
	}
//
//	// There is only one component, therefore we can unfreeze if no pair is nominated
	if (*remotedesp.desc.ice_ufrag != '\0' &&
	    (!selected_pair || !selected_pair->nominated)) {
		LTrace("Unfreezing the new candidate pair");
		agent_unfreeze_candidate_pair( pos);
	}

	return 0;
    }



    int Agent::agent_unfreeze_candidate_pair( ice_candidate_pair_t *pair)
    {
            if (pair->state != ICE_CANDIDATE_PAIR_STATE_FROZEN)
                    return 0;

            for (int i = 0; i < entries_count; ++i) {
                    agent_stun_entry_t *entry = entries + i;
                    if (pair == pair) {
                            pair->state = ICE_CANDIDATE_PAIR_STATE_PENDING;
                            entry->state = AGENT_STUN_ENTRY_STATE_PENDING;
                            agent_arm_transmission( entry, 0); // transmit now
                            return 0;
                    }
            }

            LWarn("Unable to unfreeze the pair: no matching entry");
            return -1;
    }



    void Agent::agent_arm_transmission(agent_stun_entry_t *entry, int64_t delay ) 
    {
            if (entry->state != AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE)
                    entry->state = AGENT_STUN_ENTRY_STATE_PENDING;

            // Arm transmission
            entry->next_transmission = current_timestamp() + delay;

            if (entry->state == AGENT_STUN_ENTRY_STATE_PENDING) {
                    entry->retransmission_timeout = MIN_STUN_RETRANSMISSION_TIMEOUT;
                    entry->retransmissions = entry->type == AGENT_STUN_ENTRY_TYPE_CHECK
                                                 ? MAX_STUN_CHECK_RETRANSMISSION_COUNT
                                                 : MAX_STUN_SERVER_RETRANSMISSION_COUNT;
            }

            // Find a time slot
            agent_stun_entry_t *other = entries;
            while (other != entries + entries_count) {
                    if (other != entry) {
                            int64_t other_transmission = other->next_transmission;
                            int64_t timediff = entry->next_transmission - other_transmission;
                            if (other_transmission && abs((int)timediff) < STUN_PACING_TIME) {
                                    entry->next_transmission = other_transmission + STUN_PACING_TIME;
                                    other = entries;
                                    continue;
                            }
                    }
                    ++other;
            }
    }


} /* namespace stun */
