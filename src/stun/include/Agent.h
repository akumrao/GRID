


#ifndef STUN_Agent_H
#define STUN_Agent_H

#include <Attribute.h>
#include <Types.h>
#include "candidate.h"
#include "description.h"

using namespace rtc;



#define STUN_TRANSACTION_ID_SIZE 12
namespace stun {

    
    typedef enum juice_state {
	JUICE_STATE_DISCONNECTED = 0,
	JUICE_STATE_GATHERING,
	JUICE_STATE_CONNECTING,
	JUICE_STATE_CONNECTED,
	JUICE_STATE_COMPLETED,
	JUICE_STATE_FAILED
} juice_state_t;

//typedef void (*_cb_state_changed_t)(juice_agent_t *agent, juice_state_t state, void *user_ptr);
//typedef void (*cb_candidate_t)(juice_agent_t *agent, const char *sdp, void *user_ptr);
//typedef void (*cb_gathering_done_t)(juice_agent_t *agent, void *user_ptr);
//typedef void (*cb_recv_t)(juice_agent_t *agent, const char *data, size_t size,
//                                void *user_ptr);

  class Agent {
  public:
      
    using candidate_callback = std::function<void(const Candidate candidate)>;
    //	using gathering_state_callback = std::function<void(GatheringState state)>;
      
         
    Agent(Description &locadesp, candidate_callback candidateCallback);
    ~Agent();
    bool getInterfaces( int port);

           
  public:
    uint16_t type;
    uint16_t length;
    uint32_t cookie;
    //uint32_t transaction[3];
    uint8_t transaction_id[STUN_TRANSACTION_ID_SIZE];
    std::vector<Attribute*> attributes;
    std::vector<uint8_t> buffer;
    Description &locadesp;
    
    int ice_create_host_candidate( char *ip,  uint16_t port , int family);
    int ice_create_reflexive_candidate( char *ip,  uint16_t port, int family );
    int ice_create_local_candidate(int component, int index, char *ip,  uint16_t port, int family, Candidate *candidate);
    uint32_t ice_compute_priority(Candidate::Type type, int family, int component, int index);
    int ice_add_candidate(Candidate *candidate, Description *description);
    
   candidate_callback mCandidateCallback;
    
  };

} /* namespace stun */

#endif
