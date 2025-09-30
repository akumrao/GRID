

#ifndef RTC_IMPL_ICE_TRANSPORT_H
#define RTC_IMPL_ICE_TRANSPORT_H

#include "candidate.hpp"
#include "common.hpp"
#include "configuration.hpp"
#include "description.hpp"
#include "global.hpp"
#include "peerconnection.hpp"
#include "transport.hpp"

#if !USE_NICE
//#include <juice/juice.h>
#include <Agent.h>
#include "configuration.hpp"
#include "peerconnection.hpp"
#include "tls.h"
#include "json/json.hpp" 
#else
#include <nice/agent.h>
#endif

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace rtc::impl {
    
  
        

class IceTransport : public Transport, IceListen {
public:
	static void Init();
	static void Cleanup();

	enum class GatheringState { New = 0, InProgress = 1, Complete = 2 };

	using candidate_callback = std::function<void(const Candidate candidate)>;
	using gathering_state_callback = std::function<void(GatheringState state)>;
        
	IceTransport(const Configuration &config, candidate_callback candidateCallback,
	             state_callback stateChangeCallback,
	             gathering_state_callback gatheringStateChangeCallback);
	~IceTransport();

	Description::Role role() const;
	GatheringState gatheringState() const;
	Description getLocalDescription(Description::Type type) ;
	void setRemoteDescription(const Description &description);
	bool addRemoteCandidate(const Candidate &candidate);
	void gatherLocalCandidates(string mid, std::vector<IceServer> additionalIceServers = {});
	void setIceAttributes(string uFrag, string pwd);

	optional<string> getLocalAddress() ;
	optional<string> getRemoteAddress();

	bool send(message_ptr message) override; // false if dropped

	bool getSelectedCandidatePair(Candidate *local, Candidate *remote);

private:
	bool outgoing(message_ptr message) override;

	void changeGatheringState(GatheringState state);

	void processStateChange(unsigned int state);
	void processCandidate(const string &candidate);
	void processGatheringDone();
	void processTimeout();

	void addIceServer(IceServer server);

	Description::Role mRole;
	string mMid;
	std::chrono::milliseconds mTrickleTimeout;
	std::atomic<GatheringState> mGatheringState;

	candidate_callback mCandidateCallback;
	gathering_state_callback mGatheringStateChangeCallback;

#if USE_libjuice
	unique_ptr<juice_agent_t, void (*)(juice_agent_t *)> mAgent;
	int mTurnServersAdded = 0;

#else
	void onStateChangeCallback( juice_state_t state);
        void onCandidateCallback( Candidate *candidate);
        void onGatheringDoneCallback();
        void onRecvCallback( unsigned char *data, size_t size);
    
        Agent agent;
#endif
};

} // namespace rtc::impl

#endif
