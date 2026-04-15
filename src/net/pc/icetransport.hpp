

#ifndef RTC_IMPL_ICE_TRANSPORT_H
#define RTC_IMPL_ICE_TRANSPORT_H

#include "candidate.hpp"
#include "common.hpp"
#include "configuration.h"
#include "description.hpp"
#include "global.hpp"
//#include "peerconnection.h"
#include "Transport.h"

#if !USE_NICE
//#include <juice/juice.h>
#include <Agent.h>
#include "configuration.h"
#include "net/tls.h"
#include "json/json.hpp" 
#else
#include <nice/agent.h>
#endif

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "Transport.h"

namespace rtc {
    
  
        

class IceTransport : public rtc::Transport::Listener, IceListen {
//class IceTransport : public Transport, IceListen {
public:
	static void Init();
	static void Cleanup();

	enum class GatheringState { New = 0, InProgress = 1, Complete = 2 };

	using candidate_callback = std::function<void(const Candidate candidate)>;
	using gathering_state_callback = std::function<void(GatheringState state)>;
        using dtls_state_callback = std::function<void(DtlsTransport::DtlsState state )>;
        using sctp_state_callback = std::function<void(SctpTransport::State state )>;
        using sctp_forward_message_callback = std::function<void(message_ptr message )>;
        

                    enum class State { Disconnected, Connecting, Connected, Completed, Failed };
                    using state_callback = std::function<void(State state)>;
                    
                    
	IceTransport(const Configuration &config, dtls_state_callback dtlsstatecallback, sctp_state_callback sctpstatecallback, sctp_forward_message_callback sctpforwardmessagecallback, candidate_callback candidateCallback,
	             state_callback stateChangeCallback,
	             gathering_state_callback gatheringStateChangeCallback );
	~IceTransport();

	Description::Role role() const;
	GatheringState gatheringState() const;
	Description getLocalDescription(Description::Type type) ;
	void setRemoteDescription(const Description &description);
	bool addRemoteCandidate(const Candidate &candidate);
	void gatherLocalCandidates(string mid, std::vector<IceServer_conf> additionalIceServers = {});
	void setIceAttributes(string uFrag, string pwd);

	optional<string> getLocalAddress() ;
	optional<string> getRemoteAddress();

	bool send(message_ptr message) ; // false if dropped

	bool getSelectedCandidatePair(Candidate *local, Candidate *remote);
        
        void OnDtlsTransportStatus(DtlsTransport::DtlsState state);
        void OnSctpState(SctpTransport::State state);
        void OnSctpTransportMessageReceived(SctpTransport* sctpAssociation ,message_ptr message );
        
        //void startSctp();
     
        
        Description::Role mRole;

        std::atomic<State> mState = State::Disconnected;
        State state() const;
        virtual void incoming(message_ptr message);
	virtual bool outgoing(message_ptr message);
        
        
        
private:
	//bool outgoing(message_ptr message) ;

	void changeGatheringState(GatheringState state);

	void processStateChange(unsigned int state);
	void processCandidate(const string &candidate);
	void processGatheringDone();
	void processTimeout();
        
        void changeState(State state);

	void addIceServer(IceServer_conf server);

	
	string mMid;
	std::chrono::milliseconds mTrickleTimeout;
	std::atomic<GatheringState> mGatheringState;

	candidate_callback mCandidateCallback;
	gathering_state_callback mGatheringStateChangeCallback;
        
        dtls_state_callback mDtlsstatecallback;
        sctp_state_callback mSctpstatecallback;
        sctp_forward_message_callback mSctpforwardmessagecallback;

#if USE_libjuice
	unique_ptr<juice_agent_t, void (*)(juice_agent_t *)> mAgent;
	int mTurnServersAdded = 0;

#else
	void onStateChangeCallback( juice_state_t state);
        void onCandidateCallback( Candidate *candidate);
        void onGatheringDoneCallback();
        void onRecvCallback( unsigned char *data, size_t size);
    public:
        Agent agent;
#endif
};

} // namespace rtc

#endif
