


#ifndef STUN_Agent_H
#define STUN_Agent_H

#include <Attribute.h>
#include <Types.h>

//using namespace rtc;



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

//typedef void (*juice_cb_state_changed_t)(juice_agent_t *agent, juice_state_t state, void *user_ptr);
//typedef void (*juice_cb_candidate_t)(juice_agent_t *agent, const char *sdp, void *user_ptr);
//typedef void (*juice_cb_gathering_done_t)(juice_agent_t *agent, void *user_ptr);
//typedef void (*juice_cb_recv_t)(juice_agent_t *agent, const char *data, size_t size,
//                                void *user_ptr);

  class Agent {
  public:
    Agent();
    ~Agent();
    bool getInterfaces();


  public:
    uint16_t type;
    uint16_t length;
    uint32_t cookie;
    //uint32_t transaction[3];
    uint8_t transaction_id[STUN_TRANSACTION_ID_SIZE];
    std::vector<Attribute*> attributes;
    std::vector<uint8_t> buffer;
//    Description *description;
    
  };

} /* namespace stun */

#endif
