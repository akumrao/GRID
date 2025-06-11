
#include <stdio.h>
#include <Agent.h>
#include <uv.h>
#include "base/logger.h"
#include <Utils.h>
#include <algorithm>

#include "sdpcommon.h"


using namespace base;





#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

namespace stun {

    
    
//void thread_function() {
//    std::thread::id thread_id = std::this_thread::get_id();
//    std::cout << "Thread ID: " << thread_id << std::endl;
//}

    
    bool comp(Candidate a, Candidate b) {
        return a.priority() > b.priority();
    }
    
    int64_t current_timestamp() { // millisecond time
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

    static int agentCount = 0;
    Agent::Agent( Description &localdesp, Description &remotedesp,  candidate_callback candidateCallback):localdesp(localdesp), remotedesp(remotedesp), mCandidateCallback(candidateCallback)
    {

        random_bytes(&ice_tiebreaker, sizeof(ice_tiebreaker));
        _timer.cb_timeout = std::bind(&Agent::onTimer, this);
        _timer.Start(20,20);
        agentNo = ++agentCount;
         m_next_timestamp = current_timestamp();
       
    }

    Agent::~Agent() {
        
         _timer.Stop();

    }

    bool Agent::getInterfaces( ) {
        
        
        static int port = 7000;
        socket = new testUdpServer("0.0.0.0", ++port , this );
        socket->start();
        
        
        agent_change_state(JUICE_STATE_GATHERING);

        //char buf[512];
        uv_interface_address_t *info;
        int count, i;

        uv_interface_addresses(&info, &count);
        i = count;

        STrace << "AgentNo " << agentNo <<  " Number of interfaces: " <<  count;
        while (i--) {
            uv_interface_address_t interface_a = info[i];
        

            Candidate candidate;
            candidate.mType = Candidate::Type::Host;
        
            if(!interface_a.is_internal)
            {
                STrace  << "AgentNo " << agentNo <<  " Name: " <<  interface_a.name;
                if (interface_a.address.address4.sin_family == AF_INET) {
                   // uv_ip4_name(&interface_a.address.address4, buf, sizeof (buf));
                   // std::memcpy(&candidate.resolved.addr , &interface_a.address.address4, sizeof(interface_a.address.address4));
                    interface_a.address.address4.sin_port =  htons(port);
                    IP::CopyAddress((const struct sockaddr* )&interface_a.address.address4,  candidate.resolved);
                } else if (interface_a.address.address4.sin_family == AF_INET6) {
                    //uv_ip6_name(&interface_a.address.address6, buf, sizeof (buf));
                   // std::memcpy(&candidate.resolved.addr , &interface_a.address.address6, sizeof(interface_a.address.address6));
                    //candidate.resolved.len = sizeof(interface_a.address.address6);
                    interface_a.address.address6.sin6_port =  htons(port);
                    IP::CopyAddress((const struct sockaddr*) &interface_a.address.address6,  candidate.resolved);
                }
                 
                char ip[40];  uint16_t port;
                 
                IP::AddressToString( candidate.resolved,  ip,  port);
                   
                ice_create_host_candidate(&candidate);   
                
                SInfo << "AgentNo " << agentNo <<  " getInterfaces " << "  " << ip  << ":" <<  port;
            }

        }

        uv_free_interface_addresses(info, count);
        
        std::sort(localdesp.desc.candidates.begin(), localdesp.desc.candidates.end(), comp);
        
        agent_change_state(JUICE_STATE_CONNECTED);
                
        return 0;

    }

    int Agent::ice_create_host_candidate( Candidate *candidate ) {

   
        
        if (ice_create_local_candidate( 1, localdesp.desc.candidates.size(),  candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( candidate, &localdesp   );
    }
    
    
    int Agent::ice_create_local_reflexive_candidate( Candidate *candidate) {

        SInfo <<  "AgentNo " << agentNo << " ice_create_local_reflexive_candidate";
       
        if (candidate->mType !=  Candidate::Type::ServerReflexive && candidate->mType  !=  Candidate::Type::PeerReflexive) {
		LError("Invalid type for local reflexive candidate");
		return -1;
	}
        
        
        if (ice_find_candidate_from_addr(&localdesp, &candidate->resolved, candidate->resolved.addr.ss_family == AF_INET6 ? Candidate::Type::Unknown : candidate->mType )) {
                  LTrace("A local candidate exists for the mapped address");
                  return 0;
        }
        
        
        if (candidate->mType == Candidate::Type::PeerReflexive && ice_candidates_count(&localdesp.desc, Candidate::Type::PeerReflexive) >= MAX_PEER_REFLEXIVE_CANDIDATES_COUNT) {
		LInfo("Local description has the maximum number of peer reflexive candidates, ignoring");
		return 0;
	}
        

        if (ice_create_local_candidate(1, localdesp.desc.candidates.size(), candidate)) {
            printf("Failed to create host candidate");
        }
        
        ice_add_candidate( candidate, &localdesp   );
    }

    int Agent::ice_create_local_candidate( int component, int index,  Candidate *candidate) {
      //  memset(candidate, 0, sizeof (*candidate));

        candidate->mComponent = component;
        //candidate->resolved = *record;
        candidate->mFoundation =  "-";

        candidate->mPriority  = ice_compute_priority(candidate->mType, candidate->resolved.addr.ss_family, candidate->mComponent, index);
        
        
//        if (getnameinfo((struct sockaddr *) &record->addr, record->len, candidate->hostname, 256,
//                candidate->service, 32, NI_NUMERICHOST | NI_NUMERICSERV | NI_DGRAM)) {
//            printf("getnameinfo failed, errno=%d", sockerrno);
//            return -1;
//        }
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
    
    
    
        int  Agent::agent_add_remote_peer_reflexive_candidate( uint32_t priority, const addr_record_t *record)
        {
            
            if (ice_find_candidate_from_addr(&remotedesp, record, Candidate::Type::Unknown)) {
                    STrace << "AgentNo " << agentNo << " A remote candidate exists for the remote address";
                    return 0;
            }
            Candidate candidate;
            candidate.mType = Candidate::Type::PeerReflexive;
            candidate.resolved =  *record;
            
//            char buf[512];
//            uint16_t port;
//            
//            if(record->addr.ss_family == AF_INET6)
//            {
//                uv_ip6_name((sockaddr_in6* )&record->addr, buf, sizeof (buf));
//		port = ntohs( ((sockaddr_in6 *)&record->addr)->sin6_port);
//                                
//            }
//            else if(record->addr.ss_family == AF_INET )
//            {
//                 uv_ip4_name((sockaddr_in*)&record->addr, buf, sizeof (buf));
//                 port =  ntohs( ((sockaddr_in *)&record->addr)->sin_port); 
//            }
            
            if (ice_create_local_candidate( 1, localdesp.desc.candidates.size(),  &candidate)) {
                    LError("Failed to create reflexive candidate");
                    return -1;
            }
            if (ice_candidates_count(&remotedesp.desc , Candidate::Type::PeerReflexive ) >=    MAX_PEER_REFLEXIVE_CANDIDATES_COUNT) {
                    LInfo( "Remote description has the maximum number of peer reflexive candidates, ignoring");
                    return 0;
            }
            if (ice_add_candidate(&candidate, &remotedesp)) {
                    LError("Failed to add candidate to remote description");
                    return -1;
            }

            SDebug << "AgentNo " << agentNo << " Obtained a new remote reflexive candidate, priority=" << (unsigned long)priority;

            Candidate *remote = &remotedesp.desc.candidates[remotedesp.desc.candidates.size() -1];
            remote->mPriority = priority;

            return agent_add_candidate_pairs_for_remote( remote);
    }
    
    int Agent::ice_add_remote_candidate(const Candidate *candidate)
    {
        
        //ice_add_candidate( (Candidate *)candidate, &remotedesp  );
        
        
        if (agent_add_candidate_pairs_for_remote((Candidate *)candidate)) {
         	LWarn("Failed to add candidate pair");
		return -2;
	}
        
    } 
    
    
    int Agent::ice_add_candidate( Candidate *candidate, Description *description) 
    {
        
       
        
//	if (candidate->cand.type == ICE_CANDIDATE_TYPE_UNKNOWN)
//		return -1;

	if (description->desc.candidates.size()  >= ICE_MAX_CANDIDATES_COUNT) {
	        SError << "Description already has the maximum number of candidates";
		return -1;
	}
        
        candidate->mMid = localMid;

	if (candidate->mFoundation == "-")
		candidate->mFoundation = std::to_string(description->desc.candidates.size() + 1);


	//ice_candidate_t *pos = description->candidates + description->localCanSdp.candidates_count;
	//*pos = *candidate;
        description->desc.candidates.push_back(*candidate);
        
	//++description->desc.candidates.size();
	
        
       // char buffer[4096];
        
        //ice_generate_candidate_sdp(candidate, buffer, 4096);
        
        SInfo<< "AgentNo " << agentNo << " local ice_add_candidate  " << string(*candidate);
        
        mCandidateCallback(*candidate);
        
        return 0;

    }

    
    void Agent::agent_update_ordered_pairs() 
    {
	LTrace("Updating ordered candidate pairs");
	for (int i = 0; i < m_candidate_pairs_count; ++i) {
		ice_candidate_pair_t **begin = m_ordered_pairs;
		ice_candidate_pair_t **end = begin + i;
		ice_candidate_pair_t **prev = end;
		uint64_t priority = m_candidate_pairs[i].priority;
		while (--prev >= begin && (*prev)->priority < priority)
			*(prev + 1) = *prev;

		*(prev + 1) = m_candidate_pairs + i;
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
            for (int i = 0; i < description->candidates.size(); ++i) {
                    const Candidate *candidate = &description->candidates [i];
                    if (candidate->mType  == type)
                            ++count;
            }
            return count;
    }


    
    int  Agent::agent_add_candidate_pair( Candidate *local, // local may be NULL
                             Candidate *remote) {
	ice_candidate_pair_t pair;
	bool is_controlling = m_mode == AGENT_MODE_CONTROLLING;
	if (ice_create_candidate_pair(local, remote, is_controlling, &pair)) {
		LError("Failed to create candidate pair");
		return -1;
	}

	if (m_candidate_pairs_count >= MAX_CANDIDATE_PAIRS_COUNT) {
		LWarn("Session already has the maximum number of candidate pairs");
		return -1;
	}

	//LTrace("Adding new candidate pair, priority=%" PRIu64, pair.priority);

	// Add pair
	ice_candidate_pair_t *pos = m_candidate_pairs + m_candidate_pairs_count;
	*pos = pair;
	++m_candidate_pairs_count;

	agent_update_ordered_pairs();

	if (m_entries_count == MAX_STUN_ENTRIES_COUNT) {
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

	STrace << "AgentNo " << agentNo <<  " Registering STUN entry  " <<   m_entries_count << " for candidate pair checking";
	agent_stun_entry_t *entry = m_entries + m_entries_count;
	entry->type = AGENT_STUN_ENTRY_TYPE_CHECK;
	entry->state = AGENT_STUN_ENTRY_STATE_IDLE;
	entry->mode = AGENT_MODE_UNKNOWN;
	entry->pair = pos;
	entry->record = pos->remote->resolved;
	//entry->relay_entry = relay_entry;
	random_bytes(entry->transaction_id, STUN_TRANSACTION_ID_SIZE);
	entry->transaction_id_expired = false;
	++m_entries_count;
//
//	if (remote->mType  == Candidate::Type::Host)
//		agent_translate_host_candidate_entry( entry);

	if (m_mode == AGENT_MODE_CONTROLLING) {
		for (int i = 0; i < m_candidate_pairs_count; ++i) {
			ice_candidate_pair_t *ordered_pair = m_ordered_pairs[i];
			if (ordered_pair == pos) {
				STrace << "AgentNo " << agentNo << " Candidate pair has priority";
				break;
			}
			if (ordered_pair->state == ICE_CANDIDATE_PAIR_STATE_SUCCEEDED) {
				// We found a succeeded pair with higher priority, ignore this one
				STrace << "AgentNo " << agentNo << " Candidate pair doesn't have priority, keeping it frozen";
				return 0;
			}
		}
	}
//
//	// There is only one component, therefore we can unfreeze if no pair is nominated
	if (*remotedesp.desc.ice_ufrag != '\0' &&
	    (!m_selected_pair || !m_selected_pair->nominated)) {
		STrace << "AgentNo " << agentNo << " Unfreezing the new candidate pair";
		agent_unfreeze_candidate_pair( pos);
	}

	return 0;
    }



    int Agent::agent_unfreeze_candidate_pair( ice_candidate_pair_t *pair)
    {
            if (pair->state != ICE_CANDIDATE_PAIR_STATE_FROZEN)
                    return 0;

            for (int i = 0; i < m_entries_count; ++i) {
                    agent_stun_entry_t *entry = m_entries + i;
                    if (pair == pair) {
                            pair->state = ICE_CANDIDATE_PAIR_STATE_PENDING;
                            entry->state = AGENT_STUN_ENTRY_STATE_PENDING;
                            agent_arm_transmission( entry, 0); // transmit now
                            return 0;
                    }
            }

            SWarn << "AgentNo " << agentNo << "Unable to unfreeze the pair: no matching entry";
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
            agent_stun_entry_t *other = m_entries;
            while (other != m_entries + m_entries_count) {
                    if (other != entry) {
                            int64_t other_transmission = other->next_transmission;
                            int64_t timediff = entry->next_transmission - other_transmission;
                            if (other_transmission && abs((int)timediff) < STUN_PACING_TIME) {
                                    entry->next_transmission = other_transmission + STUN_PACING_TIME;
                                    other = m_entries;
                                    continue;
                            }
                    }
                    ++other;
            }
    }
    
    
    void Agent::onTimer()
    {
        
        int64_t cur =  current_timestamp();
                
        if( m_next_timestamp <=  cur )
        int ret =  agent_bookkeeping( cur) ;
        
        
//        if (expired())
//            // Attempt to re-allocate
//            sendAllocate();
//
//        else if (timeRemaining() < lifetime() * 0.33)
//            sendRefresh();
//
//        _observer.onTimer(*this);
        

    }

    
    
    bool Agent::is_stun_datagram(const void *data, size_t size) {
	// RFC 8489: The most significant 2 bits of every STUN message MUST be zeroes. This can be used
	// to differentiate STUN packets from other protocols when STUN is multiplexed with other
	// protocols on the same port.
	if (!size || *((uint8_t *)data) & 0xC0) {
		STrace << "AgentNo " << agentNo << " Not a STUN message: first 2 bits are not zeroes";
		return false;
	}

	if (size < sizeof(struct stun_header)) {
		STrace << "AgentNo " << agentNo << " Not a STUN message: message too short, size=" <<  size;
		return false;
	}

	const struct stun_header *header = (const struct stun_header *) data;
	if (ntohl(header->magic) != STUN_MAGIC) {
		STrace << "AgentNo " << agentNo << " Not a STUN message: magic number invalid";
		return false;
	}

	// RFC 8489: The message length MUST contain the size of the message in bytes, not including the
	// 20-byte STUN header.  Since all STUN attributes are padded to a multiple of 4 bytes, the last
	// 2 bits of this field are always zero.  This provides another way to distinguish STUN packets
	// from packets of other protocols.
	const size_t length = ntohs(header->length);
	if (length & 0x03) {
		STrace << "AgentNo " << agentNo << " Not a STUN message: invalid length " << length << " not multiple of 4";
		return false;
	}
	if (size != sizeof(struct stun_header) + length) {
		STrace << "AgentNo " << agentNo << " Not a STUN message: invalid length "<< length << " while expecting "<<  size - sizeof(struct stun_header);
		return false;
	}

	return true;
}
    
    int Agent::onStunMessage( char *buf, size_t len, const addr_record_t *src,  const addr_record_t *relayed)
    {
	STrace << "AgentNo " << agentNo << " Received datagram, size "<<  len;

	if(m_state == JUICE_STATE_DISCONNECTED || m_state == JUICE_STATE_GATHERING)
		return 0;

	if (is_stun_datagram(buf, len)) {
                
                stun::Message msg;
                stun::Reader reader;
                if( reader.process((uint8_t*) buf, len, &msg))
                {
                    SError << "AgentNo " << agentNo << " STUN message reading failed";
		    return -1;
                }
  
  
		return agent_dispatch_stun( buf, len, &msg, src, relayed);
	}


        // For Turn
        /*
	agent_stun_entry_t *entry = agent_find_entry_from_record(agent, src, relayed);
	if (!entry) {
		LWarn("Received a datagram from unknown address, ignoring");
		return -1;
	}
	switch (entry->type) {
	case AGENT_STUN_ENTRY_TYPE_RELAY:
		if (is_channel_data(buf, len)) {
			LDebug("Received ChannelData datagram");
			return agent_process_channel_data(agent, entry, buf, len);
		}
		break;

	case AGENT_STUN_ENTRY_TYPE_CHECK:
		LDebug("Received application datagram");
		if (agent->config.cb_recv)
			agent->config.cb_recv(agent, buf, len, agent->config.user_ptr);
		return 0;

	default:
		break;
	}

	LWarn("Received unexpected non-STUN datagram, ignoring");
	return -1;
        
        */
}

    
    
    
int Agent::agent_dispatch_stun( char *buf, size_t size, stun::Message  *msg,  const addr_record_t *src, const addr_record_t *relayed)
{
    
   //msg->hasAttribute(stun::STUN_ATTR_USE_CANDIDATE)
    
    if (msg->msg_method  == STUN_METHOD_BINDING && msg->hasAttribute(stun::STUN_ATTR_MESSAGE_INTEGRITY )) {
            STrace << "AgentNo " << agentNo << " On STUN message is from the remote peer";
            // Verify the message now
            if (agent_verify_stun_binding( buf, size, msg)) {
                    SWarn << "AgentNo " << agentNo << " On STUN message verification failed";
                    return -1;
            }
            
            Priority *result;
            if( !msg->find(&result ))
            {
                SError  << "AgentNo " << agentNo << " On Stun Message. Priority attribute is not in stun message";
                exit(0);
            }
            
  
            
            if (agent_add_remote_peer_reflexive_candidate( result->value, src)) {
                    SWarn << "AgentNo " << agentNo << " On Stun Message. Failed to add remote peer reflexive candidate from STUN message";
            }
    }

    agent_stun_entry_t *entry = NULL;
    if (STUN_IS_RESPONSE(msg->msg_class)) {
            LTrace("STUN message is a response, looking for transaction ID");
            entry = agent_find_entry_from_transaction_id(msg->transaction_id);
            if (!entry) {
                    SError << "AgentNo " << agentNo << " On Stun Message. No STUN entry matching transaction ID, ignoring";
                    return -1;
            }
    } else {
            LTrace("STUN message is a request or indication, looking for remote address");
            entry = agent_find_entry_from_record( src, relayed);
            if (entry) {
                   STrace << "AgentNo " << agentNo << " On Stun Message. Found STUN entry matching remote address";
            } else {
                    // This may happen normally, for instance when there is no space left for reflexive
                    // candidates
                    SDebug << "AgentNo " << agentNo << " On Stun Message.  No STUN entry matching remote address, ignoring";
                    return 0;
            }
    }
    
    char ip[40];  uint16_t port;
    IP::AddressToString(entry->record, ip, port); 
    STrace << "On Message AgentNo " << agentNo  <<    " type server[1]/relay[2] " <<   entry->type  <<   " mode controlled[1]/controlling[2] " <<  entry->mode  << " state PENDING[0]/CANCELLED[1]/FAILED[2]SUCCEEDED[3]KEEPALIVE[4]IDLE[5] " <<  entry->state << " add " << ip << port;

    switch (msg->msg_method) {
    case STUN_METHOD_BINDING:
            // Message was verified earlier, no need to re-verify
            if (entry->type == AGENT_STUN_ENTRY_TYPE_CHECK && !msg->hasAttribute(stun::STUN_ATTR_MESSAGE_INTEGRITY ) &&
                (msg->msg_class == STUN_CLASS_REQUEST || msg->msg_class == STUN_CLASS_RESP_SUCCESS)) {
                    SWarn << "AgentNo " << agentNo << " On Stun Message. Missing integrity in STUN Binding message from remote peer, ignoring";
                    return -1;
            }
           return agent_process_stun_binding( msg, entry, src, relayed);

    case STUN_METHOD_ALLOCATE:
    case STUN_METHOD_REFRESH:
            if (agent_verify_credentials( entry, buf, size, msg)) {
                    SWarn << "AgentNo " << agentNo << " On Stun Message. Ignoring TURN Allocate message with invalid credentials (Is server authentication disabled?";
                    return -1;
            }
           // return agent_process_turn_allocate(agent, msg, entry);
            SError << "Turn Relay agent not supported " ;
            exit(0);

    case STUN_METHOD_CREATE_PERMISSION:
            if (agent_verify_credentials( entry, buf, size, msg)) {
                    SWarn << "AgentNo " << agentNo << " On Stun Message. Ignoring TURN CreatePermission message with invalid credentials";
                    return -1;
            }
           // return agent_process_turn_create_permission(agent, msg, entry);
            SError << "AgentNo " << agentNo << " Turn Relay agent not supported " ;
            exit(0);

    case STUN_METHOD_CHANNEL_BIND:
            if (agent_verify_credentials( entry, buf, size, msg)) {
                    LWarn("Ignoring TURN ChannelBind message with invalid credentials");
                    return -1;
            }
           // return agent_process_turn_channel_bind(agent, msg, entry);
            SError << "AgentNo " << agentNo << " Turn Relay agent not supported " ;
            exit(0);
    case STUN_METHOD_DATA:
            //return agent_process_turn_data(agent, msg, entry);
            SError << "AgentNo " << agentNo << " Turn Relay agent not supported " ;
            exit(0);

    default:
            SWarn << "AgentNo " << agentNo << " On Stun Message. Unknown STUN method 0x%X, ignoring " <<  msg->msg_method;
            return -1;
    }
}








int Agent::agent_verify_stun_binding( char *buf, size_t size, stun::Message *msg)
{
	if (msg->msg_method != STUN_METHOD_BINDING)
		return -1;

	if (msg->msg_class == STUN_CLASS_INDICATION || msg->msg_class == STUN_CLASS_RESP_ERROR)
		return 0;

	if (!msg->hasAttribute(stun::STUN_ATTR_MESSAGE_INTEGRITY )) {
		LWarn("Missing integrity in STUN message");
		return -1;
	}

	// Check username (The USERNAME attribute is not present in responses)
	if (msg->msg_class == STUN_CLASS_REQUEST) {
		char username[STUN_MAX_USERNAME_LEN];
		strcpy(username, msg->credentials.username);
		char *separator = strchr(username, ':');
		if (!separator) {
			SWarn << "STUN username invalid, username= " <<  username;
			return -1;
		}
		*separator = '\0';
		const char *local_ufrag = username;
		const char *remote_ufrag = separator + 1;
		if (strcmp(local_ufrag, localdesp.desc.ice_ufrag) != 0) {
			SWarn << "STUN local ufrag check failed, expected= " << localdesp.desc.ice_ufrag  << " actual= " << local_ufrag;
			return -1;
		}
		// RFC 8445 7.3. STUN Server Procedures:
		// It is possible (and in fact very likely) that the initiating agent will receive a Binding
		// request prior to receiving the candidates from its peer.  If this happens, the agent MUST
		// immediately generate a response.
		if (*remotedesp.desc.ice_ufrag != '\0' &&
		    strcmp(remote_ufrag, remotedesp.desc.ice_ufrag) != 0) {
			SWarn << "STUN remote ufrag check failed, expected= " << remotedesp.desc.ice_ufrag <<  " actual= " <<  remote_ufrag;
			return -1;
		}
	}
	// Check password
	const char *password =
	    msg->msg_class == STUN_CLASS_REQUEST ?  localdesp.desc.ice_pwd : remotedesp.desc.ice_pwd;
	if (*password == '\0') {
		LWarn("STUN integrity check failed, unknown password");
		return -1;
	}
//	if (!stun_check_integrity(buf, size, msg, password)) {
//		SWarn << "STUN integrity check failed, password= " <<  password;
//		return -1;
//	}
//        
        
        stun::Message rmsg;
        stun::Reader reader;
        int r = reader.process((uint8_t*) buf, size , &rmsg);  // // TBD:  No need to reparse the buffer// Use compute_message_integrity function directly
        if( r == 0)
        {
            bool ret = reader.computeMessageIntegrity(&rmsg, password);  
            if(!ret)
            {
               SWarn << "STUN integrity check failed, password= " <<  password;
		return -1;
            }
        }
        
        
	return 0;
}




int Agent::agent_verify_credentials( const agent_stun_entry_t *entry, char *buf,   size_t size, stun::Message *msg)
{
	// RFC 8489: If the response is an error response with an error code of 400 (Bad Request) and
	// does not contain either the MESSAGE-INTEGRITY or MESSAGE-INTEGRITY-SHA256 attribute, then the
	// response MUST be discarded, as if it were never received.  This means that retransmits, if
	// applicable, will continue.
	if (msg->msg_class == STUN_CLASS_INDICATION ||
	    (msg->msg_class == STUN_CLASS_RESP_ERROR && msg->error_code != 400))
		return 0;

	if (!msg->hasAttribute(stun::STUN_ATTR_MESSAGE_INTEGRITY )) {
		LWarn("Missing integrity in STUN message");
		return -1;
	}
//	if (!entry->turn) {
//		LWarn("No credentials for entry");
//		return -1;
//	}
//	stun_credentials_t *credentials = &entry->turn->credentials;
//	const char *password = entry->turn->password;

	// Prepare credentials
//	strcpy(msg->credentials.realm, credentials->realm);
//	strcpy(msg->credentials.nonce, credentials->nonce);
//	strcpy(msg->credentials.username, credentials->username);

	// Check credentials
//	if (!stun_check_integrity(buf, size, msg, password)) {
//		LWarn("STUN integrity check failed");
//		return -1;
//	}
	return 0;
}


static bool addr_is_equal(const struct sockaddr *a, const struct sockaddr *b, bool compare_ports) {
	if (a->sa_family != b->sa_family)
		return false;

	switch (a->sa_family) {
	case AF_INET: {
		const struct sockaddr_in *ain = (const struct sockaddr_in *)a;
		const struct sockaddr_in *bin = (const struct sockaddr_in *)b;
		if (memcmp(&ain->sin_addr, &bin->sin_addr, 4) != 0)
			return false;
		if (compare_ports && ain->sin_port != bin->sin_port)
			return false;
		break;
	}
	case AF_INET6: {
		const struct sockaddr_in6 *ain6 = (const struct sockaddr_in6 *)a;
		const struct sockaddr_in6 *bin6 = (const struct sockaddr_in6 *)b;
		if (memcmp(&ain6->sin6_addr, &bin6->sin6_addr, 16) != 0)
			return false;
		if (compare_ports && ain6->sin6_port != bin6->sin6_port)
			return false;
		break;
	}
	default:
		return false;
	}

	return true;
}

const char *stun_get_error_reason(unsigned int code) {
	switch (code) {
	case 0:
		return "";
	case 300:
		return "Try Alternate";
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthenticated";
	case 403:
		return "Forbidden";
	case 420:
		return "Unknown Attribute";
	case 437:
		return "Allocation Mismatch";
	case 438:
		return "Stale Nonce";
	case 440:
		return "Address Family not Supported";
	case 441:
		return "Wrong credentials";
	case 442:
		return "Unsupported Transport Protocol";
	case 443:
		return "Peer Address Family Mismatch";
	case 486:
		return "Allocation Quota Reached";
	case 500:
		return "Server Error";
	case 508:
		return "Insufficient Capacity";
	default:
		return "Error";
	}
}

Candidate *Agent::ice_find_candidate_from_addr(Description *description,  const addr_record_t *record,  Candidate::Type type)
{
	
     
    for( int i =0; i < description->desc.candidates.size(); ++i)
    {
    
        Candidate *cur = & description->desc.candidates[i];
    
    
	//Candidate *end = cur + description->candidates_count;
	//while (cur != end) 
        //{
		if ((type == Candidate::Type::Unknown || cur->mType == type) &&
		    addr_is_equal((struct sockaddr *)&record->addr, (struct sockaddr *)&cur->resolved.addr,
		                  true))
			return cur;
		//++cur;
	//}
    }
	return NULL;
}



agent_stun_entry_t *Agent::agent_find_entry_from_transaction_id( const uint8_t *transaction_id) 
{
	for (int i = 0; i < m_entries_count; ++i) {
		agent_stun_entry_t *entry = m_entries + i;
		if (memcmp(transaction_id, entry->transaction_id, STUN_TRANSACTION_ID_SIZE) == 0) {
			STrace << "STUN entry " << i  << " matching incoming transaction ID ";
			return entry;
		}
//		if (entry->turn) {
//			if (turn_retrieve_transaction_id(&entry->turn->map, transaction_id, NULL)) {
//				LTrace("STUN entry %d matching incoming transaction ID (TURN)", i);
//				return entry;
//			}
//		}
	}
	return NULL;
}


static inline bool pair_is_relayed(const ice_candidate_pair_t *pair) {
	return pair->local && pair->local->type() == Candidate::Type::Relayed;
}

static inline bool entry_is_relayed(const agent_stun_entry_t *entry) {
	return entry->pair && pair_is_relayed(entry->pair);
}

static bool addr_record_is_equal(const addr_record_t *a, const addr_record_t *b, bool compare_ports) {
	return addr_is_equal((const struct sockaddr *)&a->addr, (const struct sockaddr *)&b->addr,
	                     compare_ports);
}

agent_stun_entry_t *Agent::agent_find_entry_from_record( const addr_record_t *record, const addr_record_t *relayed) 
{
	agent_stun_entry_t *selected_entry = m_selected_entry;

	if (selected_entry && selected_entry->pair && selected_entry->pair->nominated) {
		// As an optimization, try to match the nominated entry first
		if (relayed) {
//			if (entry_is_relayed(selected_entry) &&
//			    addr_record_is_equal(&selected_entry->pair->local->resolved, relayed, true) &&
//			    addr_record_is_equal(&selected_entry->record, record, true)) {
//				LDebug("STUN selected entry matching incoming relayed address");
//				return selected_entry;
			}
		} else {
			if (!entry_is_relayed(selected_entry) &&
			    addr_record_is_equal(&selected_entry->record, record, true)) {
				SDebug << "AgentNo " << agentNo <<   " STUN selected entry matching incoming address" ;
				return selected_entry;
			}
		
	}

	if (relayed) {
//		for (int i = 0; i < agent->entries_count; ++i) {
//			agent_stun_entry_t *entry = agent->entries + i;
//			if (entry_is_relayed(entry) &&
//			    addr_record_is_equal(&entry->pair->local->resolved, relayed, true) &&
//			    addr_record_is_equal(&entry->record, record, true)) {
//				LDebug("STUN entry %d matching incoming relayed address", i);
//				return entry;
//			}
//		}
	} else {
		// Try to match pairs by priority first
		ice_candidate_pair_t *matching_pair = NULL;
		for (int i = 0; i < m_candidate_pairs_count; ++i) {
			ice_candidate_pair_t *pair = m_ordered_pairs[i];
			if (!pair_is_relayed(pair) &&
			    addr_record_is_equal(&pair->remote->resolved, record, true)) {
				matching_pair = pair;
				break;
			}
		}

		if (matching_pair) {
			// Just find the corresponding entry
			for (int i = 0; i < m_entries_count; ++i) {
				agent_stun_entry_t *entry = m_entries + i;
				if (entry->pair == matching_pair) {
					SDebug << "AgentNo " << agentNo << " STUN entry " << i  << " pair matching incoming address";
					return entry;
				}
			}
		}

		// Try to match entries directly
		for (int i = 0; i < m_entries_count; ++i) {
			agent_stun_entry_t *entry = m_entries + i;
			if (!entry_is_relayed(entry) && addr_record_is_equal(&entry->record, record, true)) {
				SDebug << "AgentNo " << agentNo << " STUN entry " << i << "  matching incoming address";
				return entry;
			}
		}
	}
	return NULL;
}




int Agent::agent_process_stun_binding( stun::Message *msg,   agent_stun_entry_t *entry, const addr_record_t *src,    const addr_record_t *relayed) 
{

	switch (msg->msg_class) {
	case STUN_CLASS_REQUEST: {
		LDebug("Received STUN Binding request");
		if (entry->type != AGENT_STUN_ENTRY_TYPE_CHECK)
			return -1;

		ice_candidate_pair_t *pair = entry->pair;
		if (msg->ice_controlling == msg->ice_controlled) {
			LWarn("Controlling and controlled attributes mismatch in request");
			agent_send_stun_binding( entry, STUN_CLASS_RESP_ERROR, 400, msg->transaction_id,     NULL);
			return -1;
		}
		// RFC8445 7.3.1.1. Detecting and Repairing Role Conflicts:
		// If the agent is in the controlling role, and the ICE-CONTROLLING attribute is present in
		// the request:
		//  * If the agent's tiebreaker value is larger than or equal to the contents of the
		//  ICE-CONTROLLING attribute, the agent generates a Binding error response and includes an
		//  ERROR-CODE attribute with a value of 487 (Role Conflict) but retains its role.
		//  * If the agent's tiebreaker value is less than the contents of the ICE-CONTROLLING
		//  attribute, the agent switches to the controlled role.
		if (m_mode == AGENT_MODE_CONTROLLING && msg->ice_controlling) {
			LWarn("ICE role conflict (both controlling)");
			if (ice_tiebreaker >= msg->ice_controlling) {
				LDebug("Asking remote peer to switch roles");
				agent_send_stun_binding( entry, STUN_CLASS_RESP_ERROR, 487,  msg->transaction_id, NULL);
			} else {
				LDebug("Switching to controlled role");
				m_mode = AGENT_MODE_CONTROLLED;
				agent_update_candidate_pairs();
			}
			break;
		}
		// If the agent is in the controlled role, and the ICE-CONTROLLED attribute is present in
		// the request:
		//  * If the agent's tiebreaker value is larger than or equal to the contents of the
		//  ICE-CONTROLLED attribute, the agent switches to the controlling role.
		//  * If the agent's tiebreaker value is less than the contents of the ICE-CONTROLLED
		//  attribute, the agent generates a Binding error response and includes an ERROR-CODE
		//  attribute with a value of 487 (Role Conflict) but retains its role.
		if (m_mode == AGENT_MODE_CONTROLLED && msg->ice_controlled) {
			LWarn("ICE role conflict (both controlled)");
			if (ice_tiebreaker >= msg->ice_controlling) {
				LDebug("Switching to controlling role");
				m_mode = AGENT_MODE_CONTROLLING;
				agent_update_candidate_pairs();
			} else {
				LDebug("Asking remote peer to switch roles");
				agent_send_stun_binding( entry, STUN_CLASS_RESP_ERROR, 487, msg->transaction_id, NULL);
			}
			break;
		}
		if (msg->hasAttribute(STUN_ATTR_USE_CANDIDATE )) {
			if (!msg->ice_controlling) {
				LWarn("STUN message use_candidate missing ice_controlling attribute");
				agent_send_stun_binding( entry, STUN_CLASS_RESP_ERROR, 400,      msg->transaction_id, NULL);
				return -1;
			}
			// RFC 8445 7.3.1.5. Updating the Nominated Flag:
			// If the state of this pair is Succeeded, it means that the check previously sent by
			// this pair produced a successful response and generated a valid pair. The agent sets
			// the nominated flag value of the valid pair to true.
			if (pair->state == ICE_CANDIDATE_PAIR_STATE_SUCCEEDED) {
				LDebug("Got a nominated pair (controlled)");
				pair->nominated = true;
			} else if (!pair->nomination_requested) {
				LDebug("Pair nomination requested (controlled)");
				pair->nomination_requested = true;
			}
		}
		// Response
		if (agent_send_stun_binding( entry, STUN_CLASS_RESP_SUCCESS, 0, msg->transaction_id, src))
                {
			LError("Failed to send STUN Binding response");
			return -1;
		}
		// Triggered check
		// RFC 8445: If the state of that pair is Succeeded, nothing further is done. If the state
		// of that pair is In-Progress, [...] the agent MUST [...] trigger a new connectivity check
		// of the pair. [...] If the state of that pair is Waiting, Frozen, or Failed, the agent
		// MUST [...] trigger a new connectivity check of the pair.
		if (pair->state != ICE_CANDIDATE_PAIR_STATE_SUCCEEDED && *remotedesp.desc.ice_ufrag != '\0') {
			LDebug("Triggered pair check");
			pair->state = ICE_CANDIDATE_PAIR_STATE_PENDING;
			entry->state = AGENT_STUN_ENTRY_STATE_PENDING;
			agent_arm_transmission(entry, STUN_PACING_TIME);
		}
		break;
	}
	case STUN_CLASS_RESP_SUCCESS: {
		SDebug<< "Received STUN Binding success response from " << (entry->type == AGENT_STUN_ENTRY_TYPE_CHECK ? "peer" : "server");

		if (entry->type == AGENT_STUN_ENTRY_TYPE_SERVER)
			LInfo("STUN server binding successful");

		if (entry->state != AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE) {
			entry->state = AGENT_STUN_ENTRY_STATE_SUCCEEDED;
			entry->next_transmission = 0;
		}

		if (!m_selected_pair || !m_selected_pair->nominated) {
			// We want to send keepalives now
			entry->state = AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE;
			agent_arm_keepalive( entry);
		}

		if (msg->mapped->len && !relayed) {
			LTrace("Response has mapped address");

			Candidate::Type type = (entry->type == AGENT_STUN_ENTRY_TYPE_CHECK)
			                                ? Candidate::Type::PeerReflexive
			                                : Candidate::Type::ServerReflexive;
                        
                        
                        Candidate candidate;
                        candidate.mType = type;
                        candidate.resolved = *msg->mapped;
                        
                        char buf[512];
                        uint16_t port;

                        if(candidate.resolved.addr.ss_family == AF_INET6)
                        {
                            uv_ip6_name((sockaddr_in6* )&candidate.resolved.addr, buf, sizeof (buf));
                            port = ntohs( ((sockaddr_in6 *)&candidate.resolved.addr)->sin6_port);

                        }
                        else if(candidate.resolved.addr.ss_family == AF_INET )
                        {
                             uv_ip4_name((sockaddr_in*)&candidate.resolved.addr, buf, sizeof (buf));
                             port =  ntohs( ((sockaddr_in *)&candidate.resolved.addr)->sin_port); 
                        }
            
                        
			if (ice_create_local_reflexive_candidate( &candidate)) {
				LWarn("Failed to add local peer reflexive candidate from STUN mapped address");
			}
		}

		if (entry->type == AGENT_STUN_ENTRY_TYPE_CHECK) {
			ice_candidate_pair_t *pair = entry->pair;
			if (!pair) {
				LError("STUN entry for candidate pair checking has no candidate pair");
				return -1;
			}

			// 7.2.5.2.1. Non-Symmetric Transport Addresses:
			// The ICE agent MUST check that the source and destination transport addresses in the
			// Binding request and response are symmetric. [...] If the addresses are not symmetric,
			// the agent MUST set the candidate pair state to Failed.
			if (!addr_record_is_equal(src, &entry->record, true)) {
				LDebug(
				    "Candidate pair check failed (non-symmetric source address in response)");
				entry->state = AGENT_STUN_ENTRY_STATE_FAILED;
				entry->next_transmission = 0;
				if (pair)
					pair->state = ICE_CANDIDATE_PAIR_STATE_FAILED;
				break;
			}

			if (pair->state != ICE_CANDIDATE_PAIR_STATE_SUCCEEDED) {
				LDebug("Candidate pair check succeeded");
				pair->state = ICE_CANDIDATE_PAIR_STATE_SUCCEEDED;
			}

			if (!pair->local && msg->mapped->len)
				pair->local = ice_find_candidate_from_addr(&localdesp, msg->mapped, Candidate::Type::Unknown);

			// Update consent timestamp
			pair->consent_expiry = current_timestamp() + CONSENT_TIMEOUT;

			// RFC 8445 7.3.1.5. Updating the Nominated Flag:
			// [...] once the check is sent and if it generates a successful response, and
			// generates a valid pair, the agent sets the nominated flag of the pair to true.
			if (pair->nomination_requested) {
				SDebug  << "Got a nominated pair " <<  (m_mode == AGENT_MODE_CONTROLLING ? "controlling" : "controlled");
				pair->nominated = true;
			}
		} else if (entry->type == AGENT_STUN_ENTRY_TYPE_SERVER) {
                    agent_update_gathering_done();
                    SInfo << "AgentNo " << agentNo << "agent_update_gathering_done()";    
		}       
		break;
	}
	case STUN_CLASS_RESP_ERROR: {
		if (msg->error_code != STUN_ERROR_INTERNAL_VALIDATION_FAILED) {
			if (msg->error_code == 487)
				SDebug << "AgentNo " << agentNo << "Got STUN Binding error response, code= " <<    (unsigned int)msg->error_code;
			else
				SWarn  << "AgentNo " << agentNo << "Got STUN Binding error response, code= " <<    (unsigned int)msg->error_code;
		}

		if (entry->type == AGENT_STUN_ENTRY_TYPE_CHECK) {
			if (msg->error_code == 487) {
				if (entry->mode == m_mode) {
					// RFC 8445 7.2.5.1. Role Conflict:
					// If the Binding request generates a 487 (Role Conflict) error response, and if
					// the ICE agent included an ICE-CONTROLLED attribute in the request, the agent
					// MUST switch to the controlling role. If the agent included an ICE-CONTROLLING
					// attribute in the request, the agent MUST switch to the controlled role. Once
					// the agent has switched its role, the agent MUST [...] set the candidate pair
					// state to Waiting [and] change the tiebreaker value.
					SWarn <<  "AgentNo " << agentNo <<  " ICE role conflict";
					SDebug<< "AgentNo " << agentNo << " Switching roles to as requested " <<  (entry->mode == AGENT_MODE_CONTROLLING ? "controlled" : "controlling" );
					m_mode = entry->mode == AGENT_MODE_CONTROLLING ? AGENT_MODE_CONTROLLED
					                                                    : AGENT_MODE_CONTROLLING;
					random_bytes(&ice_tiebreaker, sizeof(ice_tiebreaker));
					agent_update_candidate_pairs(); // expires transaction IDs

					if (entry->state != AGENT_STUN_ENTRY_STATE_IDLE) { // Check might not be started
						entry->state = AGENT_STUN_ENTRY_STATE_PENDING;
						agent_arm_transmission( entry, 0);
					}
				} else {
					SDebug << "AgentNo " <<  " Already switched roles to as requested" << (m_mode == AGENT_MODE_CONTROLLING ? "controlling" : "controlled");
				}
			} else {
				// 7.2.5.2.4. Unrecoverable STUN Response:
				// If the Binding request generates a STUN error response that is unrecoverable
				// [RFC5389], the ICE agent SHOULD set the candidate pair state to Failed.
				LDebug("Chandidate pair check failed (unrecoverable error)");
				entry->state = AGENT_STUN_ENTRY_STATE_FAILED;
				entry->next_transmission = 0;
				if (entry->pair)
					entry->pair->state = ICE_CANDIDATE_PAIR_STATE_FAILED;
			}
		} else if (entry->type == AGENT_STUN_ENTRY_TYPE_SERVER) {
			SError<< "AgentNo " <<  " STUN server binding failed (unrecoverable error)";
			entry->state = AGENT_STUN_ENTRY_STATE_FAILED;
			agent_update_gathering_done();
                        SInfo  << "AgentNo " << agentNo << "agent_update_gathering_done()";    
		}
		break;
	}
	case STUN_CLASS_INDICATION: {
		LTrace("Received STUN Binding indication");
		break;
	}
	default: {
		LWarn("Got STUN unexpected binding message, class=%u", (unsigned int)msg->msg_class);
		return -1;
	}
	}
	return 0;
}


void Agent::agent_update_candidate_pairs()
{
	bool is_controlling = m_mode == AGENT_MODE_CONTROLLING;
	for (int i = 0; i < m_candidate_pairs_count; ++i) {
		ice_candidate_pair_t *pair = m_candidate_pairs + i;
		ice_update_candidate_pair(pair, is_controlling);
	}
	agent_update_ordered_pairs();

	// Expire all transaction IDs for checks
	for (int i = 0; i < m_entries_count; ++i) {
		agent_stun_entry_t *entry = m_entries + i;
		if (entry->type == AGENT_STUN_ENTRY_TYPE_CHECK) {
			entry->transaction_id_expired = true;
		}
	}
}


// TURN refresh period
#define TURN_LIFETIME 600000                        // msecs (10 min)
#define TURN_REFRESH_PERIOD (TURN_LIFETIME - 60000) // msecs (lifetime - 1 min)
// RFC 8445: Agents SHOULD use a Tr value of 15 seconds. Agents MAY use a bigger value but MUST NOT
// use a value smaller than 15 seconds.
#define STUN_KEEPALIVE_PERIOD 15000 // msecs

void Agent::agent_arm_keepalive(agent_stun_entry_t *entry) 
{
	if (entry->state == AGENT_STUN_ENTRY_STATE_SUCCEEDED)
		entry->state = AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE;

	if (entry->state != AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE)
		return;

	int64_t period;
	switch (entry->type) {
	case AGENT_STUN_ENTRY_TYPE_RELAY:
		period = localdesp.desc.candidates.size()  > 0 ? TURN_REFRESH_PERIOD : STUN_KEEPALIVE_PERIOD;
		break;
	case AGENT_STUN_ENTRY_TYPE_SERVER:
		period = STUN_KEEPALIVE_PERIOD;
		break;
	default:
		period = STUN_KEEPALIVE_PERIOD;

		break;
	}

	entry->transaction_id_expired = true;
	agent_arm_transmission( entry, period);
}




int Agent::agent_send_stun_binding( agent_stun_entry_t *entry, stun_class_t msg_class, unsigned int error_code, const uint8_t *transaction_id, const addr_record_t *mapped) {
	// Send STUN Binding
	SDebug << "Sending STUN Binding "  <<     (msg_class == STUN_CLASS_REQUEST  ? "request" : (msg_class == STUN_CLASS_INDICATION ? "indication" : "response"));

//	stun_message_t msg;
//	memset(&msg, 0, sizeof(msg));
//	msg.msg_class = msg_class;
//	msg.msg_method = STUN_METHOD_BINDING;

	if ((msg_class == STUN_CLASS_RESP_SUCCESS || msg_class == STUN_CLASS_RESP_ERROR) &&
	    !transaction_id) {
		SError << "AgentNo " << agentNo << " No transaction ID specified for STUN response";
		return -1;
	}
        
        
                /* write */
        stun::Message response(msg_class,STUN_METHOD_BINDING );
        
//        response.addAttribute(new stun::XorMappedAddress("192.168.0.19", 55164));
//        response.addAttribute(new stun::MessageIntegrity(20));
//        response.addAttribute(new stun::Fingerprint());
//
//  stun::Writer writer;
//  writer.writeMessage(&response, "75C96DDDFC38D194FEDF75986CF962A2D56F3B65F1F7");
//  

	if (transaction_id)
		response.setTransactionID((uint8_t*)transaction_id);
	else if (msg_class == STUN_CLASS_INDICATION)
		response.setTransactionID();
	else
		response.setTransactionID(entry->transaction_id);

	const char *password = NULL;
	if (entry->type == AGENT_STUN_ENTRY_TYPE_CHECK) {
		// RFC 8445 7.2.2. Forming Credentials:
		// A connectivity-check Binding request MUST utilize the STUN short-term credential
		// mechanism. The username for the credential is formed by concatenating the username
		// fragment provided by the peer with the username fragment of the ICE agent sending the
		// request, separated by a colon (":"). The password is equal to the password provided by
		// the peer.
		switch (msg_class) {
		case STUN_CLASS_REQUEST: {
			if (*remotedesp.desc.ice_ufrag == '\0' || *remotedesp.desc.ice_pwd == '\0') {
				SError << "AgentNo " << agentNo << " Missing remote ICE credentials, dropping STUN binding request";
				return 0;
			}
			snprintf(response.credentials.username, STUN_MAX_USERNAME_LEN, "%s:%s",remotedesp.desc.ice_ufrag, localdesp.desc.ice_ufrag);
			password = remotedesp.desc.ice_pwd;
                        
                        Username *iceUser = new stun::Username(response.credentials.username);
                        response.addAttribute(iceUser);   
                        
                        if(m_mode == AGENT_MODE_CONTROLLING)
                        {
                            response.ice_controlling = ice_tiebreaker;
                            IceControlling *iceControlling = new stun::IceControlling();
                            iceControlling->tie_breaker =ice_tiebreaker; 
                            response.addAttribute(iceControlling);   
                            
                        }
                        else if(m_mode == AGENT_MODE_CONTROLLED)
                        {
                            response.ice_controlled = ice_tiebreaker;
                            IceControlled *ice_controlled = new stun::IceControlled();
                            ice_controlled->tie_breaker =ice_tiebreaker; 
                            response.addAttribute(ice_controlled);   
                        }


			// RFC 8445 7.1.1. PRIORITY
			// The PRIORITY attribute MUST be included in a Binding request and be set to the value
			// computed by the algorithm in Section 5.1.2 for the local candidate, but with the
			// candidate type preference of peer-reflexive candidates.
			int family = entry->record.addr.ss_family;
			int index = entry->pair && entry->pair->local
			                ? (int)(entry->pair->local - &localdesp.desc.candidates[0] )    //arvind
			                : 0;
                        
                        Priority *priority = new stun::Priority();
                        
			priority->value  =   ice_compute_priority( Candidate::Type::PeerReflexive, family, 1, index);
                        
                        response.addAttribute(priority);

			// RFC 8445 8.1.1. Nominating Pairs:
			// Once the controlling agent has picked a valid pair for nomination, it repeats the
			// connectivity check that produced this valid pair [...], this time with the
			// USE-CANDIDATE attribute.
			bool use_candidate = m_mode == AGENT_MODE_CONTROLLING && entry->pair &&
			                    entry->pair->nomination_requested && !entry->pair->nominated;
                        
                        if(use_candidate)
                        response.addAttribute(new stun::UseCandidate);

			entry->mode = m_mode; // save current mode in case of conflict
			break;
		}
		case STUN_CLASS_RESP_SUCCESS:
		case STUN_CLASS_RESP_ERROR: {
			password = localdesp.desc.ice_pwd;
			response.error_code = error_code;
			if (mapped)
				response.mapped = mapped;

			break;
		}
		case STUN_CLASS_INDICATION: {
			// RFC8445 11. Keepalives:
			// When STUN is being used for keepalives, a STUN Binding Indication is used. The
			// Indication MUST NOT utilize any authentication mechanism. It SHOULD contain the
			// FINGERPRINT attribute to aid in demultiplexing, but it SHOULD NOT contain any other
			// attributes.
		}
		}
	}

//	char buffer[BUFFER_SIZE];
//	int size = stun_write(buffer, BUFFER_SIZE, &msg, password);
//	if (size <= 0) {
//		JLOG_ERROR("STUN message write failed");
//		return -1;
//	}
//
//	if (entry->relay_entry) {
//		// The datagram must be sent through the relay
//		LDebug("Sending STUN message via relay");
//		int ret;
//		if (entry->pair && entry->pair->nominated)
//			ret = agent_channel_send(agent, entry->relay_entry, &entry->record, buffer, size, 0);
//		else
//			ret = agent_relay_send(agent, entry->relay_entry, &entry->record, buffer, size, 0);
//
//		if (ret < 0) {
//			LWarn("STUN message send via relay failed");
//			return -1;
//		}
//		return 0;
//	}
//
//	// Direct send
//	int ret = agent_direct_send(agent, &entry->record, buffer, size, 0);
//	if (ret < 0) {
//		if (ret == SENETUNREACH)
//			LInfo("STUN binding failed: Network unreachable");
//		else
//			LWarn("STUN message send failed");
//
//		return -1;
//	}
        
        
        if(response.error_code)
        {
            response.addAttribute(new stun::ErrorIce(response.error_code));
        }
        
        if (response.msg_class == STUN_CLASS_REQUEST ||
	    (response.msg_class == STUN_CLASS_RESP_ERROR &&
	     (response.error_code == 401 || response.error_code == 438) // Unauthenticated or Stale Nonce
	     )) {
		// TBD
            
            SError << " TBD not yet implemented";
	}
        
        
        response.addAttribute(new stun::Software("libjuice"));
        response.addAttribute(new stun::Fingerprint());


        stun::Writer writer;
        writer.writeMessage(&response, (password ? password: ""));

        printf("---------------\n");
        for (size_t i = 0; i < writer.buffer.size(); ++i) {
            if (i == 0 || i % 4 == 0) {
                printf("\n");
            }
            printf("%02X ", writer.buffer[i]);
        }
        printf("\n---------------\n");
        
        
        char ip[40];  uint16_t port;
        IP::AddressToString(entry->record, ip, port) ;
        
        
        SInfo <<  "AgentNo " << agentNo << " send stun request to " << ip << ":" << port; 
             

        
        /* and read it again. */
        stun::Message msg;
  
        stun::Reader reader;

        reader.process((uint8_t*)&writer.buffer[0],writer.buffer.size(), &msg); /* we do -1 to exclude the string terminating nul char. */
            
        socket->send(&writer.buffer[0], writer.buffer.size(), entry->record);
            

        
	return 0;
}


void Agent::agent_update_gathering_done()
{
    
    //STrace <<  "Updating gathering status";
     SInfo  << "AgentNo " << agentNo << " agent_update_gathering_done()";
     
	for (int i = 0; i < m_entries_count; ++i) {
		agent_stun_entry_t *entry = m_entries + i;
		if (entry->type != AGENT_STUN_ENTRY_TYPE_CHECK &&
		    entry->state == AGENT_STUN_ENTRY_STATE_PENDING) {
			STrace<< "AgentNo " << agentNo << " STUN server or relay entry "<< i << " is still pending" ;
			return;
		}
	}
	if (!m_gathering_done) {
		 SInfo  << "AgentNo " << agentNo << " Candidate gathering done";
		localdesp.desc.finished = true;
		m_gathering_done = true;

		agent_update_pac_timer();
                // callback
		//if (agent->config.cb_gathering_done)
			//agent->config.cb_gathering_done(agent, agent->config.user_ptr);
	}
    
}


void Agent::agent_change_state( juice_state_t state)
{
    m_state = state;
    SInfo  << "AgentNo " << agentNo << " agent_change_state " << state;
}


int Agent::agent_bookkeeping( int64_t &now) 
{

	m_next_timestamp = now + (int64_t)60000;

	if (m_state == JUICE_STATE_DISCONNECTED || m_state == JUICE_STATE_GATHERING)
		return 0;

        STrace  << "AgentNo " << agentNo << " Bookkeeping..." ; 
        
	for (int i = 0; i < m_entries_count; ++i) {
		agent_stun_entry_t *entry = m_entries + i;

                char ip[40];  uint16_t port;
                IP::AddressToString(entry->record, ip, port); 
                    
                STrace << "AgentNo " << agentNo  << " ent " << i <<    " type server[1]/relay[2] " <<   entry->type  <<   " mode controlled[1]/controlling[2] " <<  entry->mode  << " state PENDING[0]/CANCELLED[1]/FAILED[2]SUCCEEDED[3]KEEPALIVE[4]IDLE[5] " <<  entry->state << " add " << ip << port;
		// STUN requests transmission or retransmission
		if (entry->state == AGENT_STUN_ENTRY_STATE_PENDING) {
			if (entry->next_transmission > now)
				continue;

			if (entry->retransmissions >= 0) {

				if (entry->transaction_id_expired) {
                                        random_bytes(entry->transaction_id, STUN_TRANSACTION_ID_SIZE);
					entry->transaction_id_expired = false;
				}
				int ret;
				switch (entry->type) {
				case AGENT_STUN_ENTRY_TYPE_RELAY:
					//ret = agent_send_turn_allocate_request(entry, STUN_METHOD_ALLOCATE);
					break;

				default:
					ret = agent_send_stun_binding( entry, STUN_CLASS_REQUEST, 0, NULL, NULL);
					break;
				}

				if (ret >= 0) {
					--entry->retransmissions;
					if (entry->retransmissions < 0) {
						entry->next_transmission = now + LAST_STUN_RETRANSMISSION_TIMEOUT;
					} else {
						entry->next_transmission = now + entry->retransmission_timeout;
						entry->retransmission_timeout *= 2;
					}
					continue;
				}
			}

			// Failure sending or end of retransmissions
			SDebug  << "AgentNo " << agentNo << "STUN entry " <<  i << " Failed" ;
			entry->state = AGENT_STUN_ENTRY_STATE_FAILED;
			entry->next_transmission = 0;

			switch (entry->type) {
			case AGENT_STUN_ENTRY_TYPE_RELAY:
				LTrace("TURN allocation failed");
				agent_update_gathering_done();
				break;

			case AGENT_STUN_ENTRY_TYPE_SERVER:
				STrace << "AgentNo " << agentNo  << " STUN server binding failed";
				agent_update_gathering_done();
				break;

			default:
				if (entry->pair) {
					SWarn << "AgentNo " << agentNo  <<" Candidate pair check failed";
					entry->pair->state = ICE_CANDIDATE_PAIR_STATE_FAILED;
				}
				break;
			}
		}
		// STUN keepalives
		else if (entry->state == AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE) {
#if JUICE_DISABLE_CONSENT_FRESHNESS
			// No expiration
#else
			// Consent freshness expiration
			if (entry->pair && entry->pair->consent_expiry <= now) {
				SInfo  << "AgentNo " << agentNo << "STUN entry " << i << " Consent expired for candidate pair";
				entry->pair->state = ICE_CANDIDATE_PAIR_STATE_FAILED;
				entry->state = AGENT_STUN_ENTRY_STATE_FAILED;
				entry->next_transmission = 0;
				continue;
			}
#endif

			if (entry->next_transmission > now)
				continue;

			SDebug  << "AgentNo " << agentNo << "STUN entry " << i << " Sending keepalive";

                        random_bytes(entry->transaction_id, STUN_TRANSACTION_ID_SIZE);
			entry->transaction_id_expired = false;

			int ret;
			switch (entry->type) {
			case AGENT_STUN_ENTRY_TYPE_RELAY:
				// RFC 8445 5.1.1.4. Keeping Candidates Alive:
				// Refreshes for allocations are done using the Refresh transaction, as described in
				// [RFC5766]
				//ret = agent_send_turn_allocate_request( entry, STUN_METHOD_REFRESH);
				break;
			case AGENT_STUN_ENTRY_TYPE_SERVER:
				// RFC 8445 5.1.1.4. Keeping Candidates Alive:
				// For server-reflexive candidates learned through a Binding request, the bindings
				// MUST be kept alive by additional Binding requests to the server.
				ret = agent_send_stun_binding( entry, STUN_CLASS_REQUEST, 0, NULL, NULL);
				break;
			default:
#if JUICE_DISABLE_CONSENT_FRESHNESS
				// RFC 8445 11. Keepalives:
				// All endpoints MUST send keepalives for each data session. [...] STUN keepalives
				// MUST be used when an ICE agent is a full ICE implementation and is communicating
				// with a peer that supports ICE (lite or full). [...] When STUN is being used for
				// keepalives, a STUN Binding Indication is used [RFC5389].
				ret = agent_send_stun_binding(agent, entry, STUN_CLASS_INDICATION, 0, NULL, NULL);
#else
				// RFC 7675 4. Design Considerations:
				// STUN binding requests sent for consent freshness also serve the keepalive purpose
				// (i.e., to keep NAT bindings alive). Because of that, dedicated keepalives (e.g.,
				// STUN Binding Indications) are not sent on candidate pairs where consent requests
				// are sent, in accordance with Section 20.2.3 of [RFC5245].
				ret = agent_send_stun_binding( entry, STUN_CLASS_REQUEST, 0, NULL, NULL);
#endif
				break;
			}

			if (ret < 0) {
				SWarn  << "AgentNo " << agentNo << "Sending keepalive failed";
				agent_arm_transmission( entry, STUN_KEEPALIVE_PERIOD);
				continue;
			}

			agent_arm_keepalive(entry);

		} else {
			// Entry does not transmit, unset next transmission
			entry->next_transmission = 0;
		}
	}

	int pending_count = 0;
	ice_candidate_pair_t *nominated_pair = NULL;
	ice_candidate_pair_t *selected_pair = NULL;
	for (int i = 0; i < m_candidate_pairs_count; ++i) {
		ice_candidate_pair_t *pair = m_ordered_pairs[i];
		if (pair->nominated) {
			// RFC 8445 8.1.1. Nominating Pairs:
			// If more than one candidate pair is nominated by the controlling agent, and if the
			// controlled agent accepts multiple nominations requests, the agents MUST produce the
			// selected pairs and use the pairs with the highest priority.
			if (!nominated_pair) {
				nominated_pair = pair;
				selected_pair = pair;
			}
		} else if (pair->state == ICE_CANDIDATE_PAIR_STATE_SUCCEEDED) {
			if (!selected_pair)
				selected_pair = pair;
		} else if (pair->state == ICE_CANDIDATE_PAIR_STATE_PENDING) {
			if (m_mode == AGENT_MODE_CONTROLLING && selected_pair) {
				// A higher-priority pair will be used, we can stop checking.
				// Entries will be synchronized after the current loop.
				STrace << "AgentNo " << agentNo << " Cancelling check for lower-priority pair";
				pair->state = ICE_CANDIDATE_PAIR_STATE_FROZEN;
			} else {
				++pending_count;
			}
		}
	}

	if (m_mode == AGENT_MODE_CONTROLLING && nominated_pair) {
		// RFC 8445 8.1.1. Nominating Pairs:
		// Once the controlling agent has successfully nominated a candidate pair, the agent MUST
		// NOT nominate another pair for same component of the data stream within the ICE session.
		for (int i = 0; i < m_candidate_pairs_count; ++i) {
			ice_candidate_pair_t *pair = m_ordered_pairs[i];
			if (pair != nominated_pair && pair->state == ICE_CANDIDATE_PAIR_STATE_PENDING) {
				// Entries will be synchronized after the current loop.
				STrace << "AgentNo " << agentNo << " Cancelling check for non-nominated pair";
				pair->state = ICE_CANDIDATE_PAIR_STATE_FROZEN;
			}
		}
		pending_count = 0;
	}

	// Cancel entries of frozen pairs
	for (int i = 0; i < m_entries_count; ++i) {
		agent_stun_entry_t *entry = m_entries + i;
		if (entry->pair && entry->pair->state == ICE_CANDIDATE_PAIR_STATE_FROZEN &&
		    entry->state != AGENT_STUN_ENTRY_STATE_IDLE &&
		    entry->state != AGENT_STUN_ENTRY_STATE_CANCELLED) {
			SDebug  << "AgentNo " << agentNo << "STUN entry " <<  i << " Cancelled";
			entry->state = AGENT_STUN_ENTRY_STATE_CANCELLED;
			entry->next_transmission = 0;
		}
	}

	if (nominated_pair && nominated_pair->state == ICE_CANDIDATE_PAIR_STATE_FAILED) {
		SWarn << "AgentNo " << agentNo << " Lost connectivity";
		 agent_change_state(JUICE_STATE_FAILED);
		//atomic_store(&selected_entry, NULL); // disallow sending
                 m_selected_entry = NULL;
		return 0;
	}

	if (selected_pair) {
		// Change selected entry if this is a new selected pair
		if (m_selected_pair != selected_pair) {
			LDebug(selected_pair->nominated ? "New selected and nominated pair"
			                                    : "New selected pair");
			m_selected_pair = selected_pair;

			// Start nomination timer if controlling
			if (m_mode == AGENT_MODE_CONTROLLING)
				nomination_timestamp = now + 2000;

			for (int i = 0; i < m_entries_count; ++i) {
				agent_stun_entry_t *entry = m_entries + i;
				if (entry->pair == selected_pair) {
					//atomic_store(&agent->selected_entry, entry);
                                        m_selected_entry =  entry; 
					break;
				}
			}
		}

		if (nominated_pair) {
			// Completed
			// Do not allow direct transition from connecting to completed
			if (m_state == JUICE_STATE_CONNECTING)
				agent_change_state(JUICE_STATE_CONNECTED);

			agent_change_state(JUICE_STATE_COMPLETED);

			agent_stun_entry_t *nominated_entry = NULL;
			agent_stun_entry_t *relay_entry = NULL;
			for (int i = 0; i < m_entries_count; ++i) {
				agent_stun_entry_t *entry = m_entries + i;
				if (entry->pair && entry->pair == nominated_pair) {
					nominated_entry = entry;
					//relay_entry = nominated_entry->relay_entry;
					break;
				}
			}

			// Enable keepalive for the entry of the nominated pair
			if (nominated_entry &&
			    nominated_entry->state != AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE) {
				nominated_entry->state = AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE;
				agent_arm_keepalive( nominated_entry);
			}

			// If the entry of the nominated candidate is relayed locally, we need also to
			// refresh the corresponding TURN session regularly
			if (relay_entry && relay_entry->state != AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE) {
				relay_entry->state = AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE;
				agent_arm_keepalive(relay_entry);
			}

			// Disable keepalives for other entries
			for (int i = 0; i < m_entries_count; ++i) {
				agent_stun_entry_t *entry = m_entries + i;
				if (entry != nominated_entry && entry != relay_entry &&
				    entry->state == AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE)
					entry->state = AGENT_STUN_ENTRY_STATE_SUCCEEDED;
			}

		} else {
			// Connected
			agent_change_state(JUICE_STATE_CONNECTED);

			if (m_mode == AGENT_MODE_CONTROLLING && !selected_pair->nomination_requested) {
				if (pending_count == 0 ||
				    (nomination_timestamp && now >= nomination_timestamp)) {
					// Nominate selected
					SDebug << "AgentNo " << agentNo << " Requesting pair nomination (controlling)";
					selected_pair->nomination_requested = true;
					for (int i = 0; i < m_entries_count; ++i) {
						agent_stun_entry_t *entry = m_entries + i;
						if (entry->pair && entry->pair == selected_pair) {
							entry->state =
							    AGENT_STUN_ENTRY_STATE_PENDING;      // we don't want keepalives
							entry->transaction_id_expired = true;	 // this is a new request
							agent_arm_transmission( entry, 0); // transmit now
							break;
						}
					}
				} else if (nomination_timestamp &&
				           m_next_timestamp > nomination_timestamp) {
					m_next_timestamp = nomination_timestamp;
				}
			}
		}

	} else if (pending_count == 0 && pac_timestamp) {
		// RFC 8863: While the timer is still running, the ICE agent MUST NOT update a checklist
		// state from Running to Failed, even if there are no pairs left in the checklist to check.
		if (now >= pac_timestamp) {
			SWarn  << "AgentNo " << agentNo << " Connectivity timer expired";
			agent_change_state(JUICE_STATE_FAILED);
			//atomic_store(&selected_entry, NULL); // disallow sending
                        m_selected_entry =  NULL;
			return 0;
		} else if (m_next_timestamp > pac_timestamp) {
			m_next_timestamp = pac_timestamp;
		}
	}

	for (int i = 0; i < m_entries_count; ++i) {
		agent_stun_entry_t *entry = m_entries + i;
		if (entry->next_transmission && m_next_timestamp > entry->next_transmission)
			m_next_timestamp = entry->next_transmission;

#if JUICE_DISABLE_CONSENT_FRESHNESS
		// No expiration
#else
		if (entry->state == AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE && entry->pair &&
		    m_next_timestamp > entry->pair->consent_expiry)
			m_next_timestamp = selected_pair->consent_expiry;
#endif
	}
        
         SInfo  << "AgentNo " << agentNo << " Bookkeeping end";

	return 0;
}


#define ICE_PAC_TIMEOUT 39500 // msecs
/* perform connectivity check*/
void  Agent::agent_update_pac_timer() {
	if (pac_timestamp)
		return;

	// RFC 8863: The ICE agent will start its timer once it believes ICE connectivity checks are
	// starting. This occurs when the agent has sent the values needed to perform connectivity
	// checks (e.g., the Username Fragment and Password [...]) and has received some indication that
	// the remote side is ready to start connectivity checks, typically via receipt of the values
	// mentioned above.
	if (*remotedesp.desc.ice_ufrag != '\0' && m_gathering_done) {
		LInfo("Connectivity timer started");
		pac_timestamp = current_timestamp() + ICE_PAC_TIMEOUT;
	}
}






int  Agent::agent_set_remote_description() {
	
	LTrace("agent_set_remote_description");


	agent_update_pac_timer();

	if (remotedesp.desc.ice_lite && m_mode != AGENT_MODE_CONTROLLING) {
		// RFC 8445 6.1.1. Determining Role:
		// The full agent MUST take the controlling role, and the lite agent MUST take the
		// controlled role.
		LDebug("Remote ICE agent is lite, assuming controlling mode");
		m_mode = AGENT_MODE_CONTROLLING;
	} else if (m_mode == AGENT_MODE_UNKNOWN) {
		LDebug("Assuming controlled mode");
		m_mode = AGENT_MODE_CONTROLLED;
	}

	// There is only one component, therefore we can unfreeze already existing pairs now
	SDebug  << "AgentNo " << agentNo << "Unfreezing %d existing candidate pairs " <<  (int)m_candidate_pairs_count;
        
	for (int i = 0; i < m_candidate_pairs_count; ++i) {
		agent_unfreeze_candidate_pair(m_candidate_pairs + i);
	}
	LDebug("Adding %d candidates from remote description", (int)remotedesp.desc.candidates.size());
	for (int i = 0; i < remotedesp.desc.candidates.size(); ++i) {
		Candidate *remote = &remotedesp.desc.candidates[ i];
		if (agent_add_candidate_pairs_for_remote( remote))
			LWarn("Failed to add candidate pair");
	}

	

}



int Agent::agent_resolve_servers( addrinfo* start)
{
    
    
//	// TURN server resolution
//	  if (config.turn_servers_count > 0) 
//          {
//		............
//	}


    addrinfo* res = start ;
    int i = 0 ;
    for (;res != NULL; res = res->ai_next) 
    { 

            STrace << "AgentNo " << agentNo <<  " Registering STUN server request  " <<   m_entries_count ;

            agent_stun_entry_t *entry = m_entries + m_entries_count;
            entry->type = AGENT_STUN_ENTRY_TYPE_SERVER;
            entry->state = AGENT_STUN_ENTRY_STATE_PENDING;
            entry->pair = NULL;

            //entry->record = records[i];

            IP::CopyAddress( res->ai_addr, entry->record);

            random_bytes(entry->transaction_id, STUN_TRANSACTION_ID_SIZE);
            entry->transaction_id_expired = false;
            ++m_entries_count;
            
            
            char ip[40];  uint16_t port;
            IP::AddressToString(entry->record, ip, port) ;
            SInfo << "AgentNo " << agentNo << " add " << ip << ":" <<port;

            agent_arm_transmission( entry, STUN_PACING_TIME * i++);
            onTimer();
            break; // Arvind remote it later
    }
		
    //agent_update_gathering_done();
	
    return 0;
}



void Agent::StartAgent( std::string &stunip, uint16_t &stunport)
{
    

        /* write */
    stun::Message response(stun::STUN_BINDING_REQUEST);
    response.setTransactionID();
    response.addAttribute(new stun::Software("libjuice"));
    response.addAttribute(new stun::Fingerprint());


    stun::Writer writer;
    writer.writeMessage(&response, "");

    printf("---------------\n");
    for (size_t i = 0; i < writer.buffer.size(); ++i) {
        if (i == 0 || i % 4 == 0) {
            printf("\n");
        }
        printf("%02X ", writer.buffer[i]);
    }
    printf("\n---------------\n");


   // socket->send(&writer.buffer[0], writer.buffer.size(), stunip, stunport);

//

    //    tesTcpServer tsvsocket;
    //    tsvsocket.start("0.0.0.0", 6000);
    //    
    //    
    //    tesTcpClient socket;
    //    socket.start(STUN_SERVER_IP , STUN_SERVER_PORT);
    //    
    //    socket.sendit( &writer.buffer[0], writer.buffer.size());    

    
}



} /* namespace stun */
