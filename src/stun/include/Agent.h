


#ifndef STUN_Agent_H
#define STUN_Agent_H

#include <Attribute.h>
#include <Types.h>
#include "candidate.h"
#include "description.h"
#include "base/Timer.h"
#include <Message.h>
#include <Connection.h>

using namespace rtc;
using namespace base;
using namespace stun;

#define MIN_STUN_RETRANSMISSION_TIMEOUT 500 // msecs
#define LAST_STUN_RETRANSMISSION_TIMEOUT (MIN_STUN_RETRANSMISSION_TIMEOUT * 16)
#define MAX_STUN_CHECK_RETRANSMISSION_COUNT 6  // exponential backoff, total 39500ms
#define MAX_STUN_SERVER_RETRANSMISSION_COUNT 5 // total 23500ms


#define STUN_TRANSACTION_ID_SIZE 12
// Max STUN and TURN server entries
#define MAX_SERVER_ENTRIES_COUNT 2 // max STUN server entries
#define MAX_RELAY_ENTRIES_COUNT 2  // max TURN server entries
#define MAX_STUN_SERVER_RECORDS_COUNT MAX_SERVER_ENTRIES_COUNT
#define MAX_CANDIDATE_PAIRS_COUNT (ICE_MAX_CANDIDATES_COUNT * (1 + MAX_RELAY_ENTRIES_COUNT))
#define MAX_STUN_ENTRIES_COUNT (MAX_CANDIDATE_PAIRS_COUNT + MAX_STUN_SERVER_RECORDS_COUNT)
#define MAX_HOST_CANDIDATES_COUNT ((ICE_MAX_CANDIDATES_COUNT - MAX_STUN_SERVER_RECORDS_COUNT) / 2)
#define MAX_PEER_REFLEXIVE_CANDIDATES_COUNT MAX_HOST_CANDIDATES_COUNT


#define STUN_IS_RESPONSE(msg_class) (msg_class & 0x0100)

// RFC 8445: ICE agents SHOULD use a default Ta value, 50 ms, but MAY use another value based on the
// characteristics of the associated data.
#define STUN_PACING_TIME 50 // msecs

// Consent freshness
// RFC 7675: Consent expires after 30 seconds.
#define CONSENT_TIMEOUT 30000 // msecs

namespace stun {

    
    typedef enum juice_state {
	JUICE_STATE_DISCONNECTED = 0,
	JUICE_STATE_GATHERING,
	JUICE_STATE_CONNECTING,
	JUICE_STATE_CONNECTED,
	JUICE_STATE_COMPLETED,
	JUICE_STATE_FAILED
} juice_state_t;


typedef enum ice_candidate_pair_state {
	ICE_CANDIDATE_PAIR_STATE_PENDING,
	ICE_CANDIDATE_PAIR_STATE_SUCCEEDED,
	ICE_CANDIDATE_PAIR_STATE_FAILED,
	ICE_CANDIDATE_PAIR_STATE_FROZEN,
} ice_candidate_pair_state_t;


typedef struct ice_candidate_pair {
	Candidate *local;
	Candidate *remote;
	uint64_t priority;
	ice_candidate_pair_state_t state;
	bool nominated;
	bool nomination_requested;
	int64_t consent_expiry;
} ice_candidate_pair_t;



typedef enum agent_mode {
	AGENT_MODE_UNKNOWN,
	AGENT_MODE_CONTROLLED,
	AGENT_MODE_CONTROLLING
} agent_mode_t;


typedef enum agent_stun_entry_type {
	AGENT_STUN_ENTRY_TYPE_EMPTY,
	AGENT_STUN_ENTRY_TYPE_SERVER,
	AGENT_STUN_ENTRY_TYPE_RELAY,
	AGENT_STUN_ENTRY_TYPE_CHECK
} agent_stun_entry_type_t;

typedef enum agent_stun_entry_state {
	AGENT_STUN_ENTRY_STATE_PENDING,
	AGENT_STUN_ENTRY_STATE_CANCELLED,
	AGENT_STUN_ENTRY_STATE_FAILED,
	AGENT_STUN_ENTRY_STATE_SUCCEEDED,
	AGENT_STUN_ENTRY_STATE_SUCCEEDED_KEEPALIVE,
	AGENT_STUN_ENTRY_STATE_IDLE
} agent_stun_entry_state_t;

typedef struct agent_stun_entry {
	agent_stun_entry_type_t type;
	agent_stun_entry_state_t state;
	agent_mode_t mode;
	ice_candidate_pair_t *pair;
	addr_record_t record;
	addr_record_t relayed;
	uint8_t transaction_id[STUN_TRANSACTION_ID_SIZE];
	int64_t next_transmission;
	int64_t retransmission_timeout;
	int retransmissions;
	bool transaction_id_expired;

	// TURN
	//agent_turn_state_t *turn;
	//unsigned int turn_redirections;
	//struct agent_stun_entry *relay_entry;

} agent_stun_entry_t;


//typedef void (*_cb_state_changed_t)(juice_agent_t *agent, juice_state_t state, void *user_ptr);
//typedef void (*cb_candidate_t)(juice_agent_t *agent, const char *sdp, void *user_ptr);
//typedef void (*cb_gathering_done_t)(juice_agent_t *agent, void *user_ptr);
//typedef void (*cb_recv_t)(juice_agent_t *agent, const char *data, size_t size,
//                                void *user_ptr);

  class Agent {
  public:
      
    using candidate_callback = std::function<void(const Candidate candidate)>;
    //	using gathering_state_callback = std::function<void(GatheringState state)>;
      
         
    Agent(Description &localdesp, Description &remotedesp, candidate_callback candidateCallback);
    ~Agent();
    bool getInterfaces( );

           
  public:
    uint16_t type;
    uint16_t length;
    uint32_t cookie;
    //uint32_t transaction[3];
    uint8_t transaction_id[STUN_TRANSACTION_ID_SIZE];
    std::vector<Attribute*> attributes;
    std::vector<uint8_t> buffer;
    Description &localdesp;
    
    Description &remotedesp;
    
    //local candidate
    int ice_create_host_candidate( Candidate *candidate);
    int ice_create_local_reflexive_candidate( Candidate *candidate );
    int ice_create_local_candidate(int component, int index, Candidate *candidate);
    uint32_t ice_compute_priority(Candidate::Type type, int family, int component, int index);
        
        
    //Remote candidate
    int ice_add_remote_candidate(const Candidate *candidate);
    int  agent_add_remote_peer_reflexive_candidate( uint32_t priority, const addr_record_t *record); // peer-reflex only



    int ice_add_candidate( Candidate *candidate, Description *description);
        
        
   candidate_callback mCandidateCallback;
   

  //  ice_description_t local;
   // ice_description_t remote;

    ice_candidate_pair_t m_candidate_pairs[MAX_CANDIDATE_PAIRS_COUNT];
    ice_candidate_pair_t *m_ordered_pairs[MAX_CANDIDATE_PAIRS_COUNT];
    ice_candidate_pair_t *m_selected_pair;
    int m_candidate_pairs_count;

    
    
    void agent_update_ordered_pairs() ;
    
    int agent_add_candidate_pairs_for_remote( Candidate *remote) ;
    
    int agent_add_candidate_pair( Candidate *local, // local may be NULL
                             Candidate *remote);
    
        
    int ice_create_candidate_pair(Candidate *local, Candidate *remote, bool is_controlling,
                              ice_candidate_pair_t *pair);
 
    int ice_update_candidate_pair(ice_candidate_pair_t *pair, bool is_controlling);
    
    void agent_update_candidate_pairs();
    
    int ice_candidates_count(const ice_description_t *description, Candidate::Type type); 
    
    void agent_arm_transmission( agent_stun_entry_t *entry, int64_t delay); 
    
    int agent_unfreeze_candidate_pair( ice_candidate_pair_t *pair);
    
    
    agent_mode_t m_mode{AGENT_MODE_UNKNOWN};
    int m_entries_count;
    

    agent_stun_entry_t m_entries[MAX_STUN_ENTRIES_COUNT];
    
    std::string localMid;

    
    juice_state_t m_state;
    
    Timer _timer{ nullptr};
     
    void onTimer();
    
    /// ON return messages 
    int onStunMessage( char *buf, size_t len, const addr_record_t *src,  const addr_record_t *relayed);
    int agent_dispatch_stun( char *buf, size_t size, stun::Message  *msg,  const addr_record_t *src, const addr_record_t *relayed);
    int agent_verify_stun_binding( char *buf, size_t size, stun::Message *msg);
    int agent_verify_credentials( const agent_stun_entry_t *entry, char *buf,   size_t size, stun::Message *msg);
    
    Candidate* ice_find_candidate_from_addr(Description *description,  const addr_record_t *record,  Candidate::Type type);
    agent_stun_entry_t* agent_find_entry_from_transaction_id( const uint8_t *transaction_id) ;
    agent_stun_entry_t* agent_find_entry_from_record( const addr_record_t *record, const addr_record_t *relayed); 
    
    int agent_process_stun_binding( stun::Message *msg,   agent_stun_entry_t *entry, const addr_record_t *src,    const addr_record_t *relayed);
    
    int agent_send_stun_binding( agent_stun_entry_t *entry, stun_class_t msg_class, unsigned int error_code, const uint8_t *transaction_id, const addr_record_t *mapped);

    void StartAgent( std::string &stunip, uint16_t &stunport);
    
    testUdpServer *socket{nullptr};
        
    
    void agent_arm_keepalive(agent_stun_entry_t *entry);
    
    int agent_bookkeeping( int64_t *next_timestamp);
    
    void agent_update_gathering_done();
    
    void agent_change_state( juice_state_t state);
    
    void  agent_update_pac_timer();
    
    int  agent_set_remote_description();
    
    agent_stun_entry_t m_selected_entry;
    
    uint64_t ice_tiebreaker;  // random number
        
    int64_t nomination_timestamp;
    int64_t pac_timestamp; 
    bool  gathering_done{false};
  };

} /* namespace stun */

#endif
