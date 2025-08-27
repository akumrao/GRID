

#ifndef RTC_IMPL_ICE_TRANSPORT_H
#define RTC_IMPL_ICE_TRANSPORT_H

#include "candidate.h"
#include "common.h"
#include "configuration.h"
#include "description.h"
#include "transport.hpp"

#include <Agent.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

using namespace stun;

namespace rtc {
	

        
class IceTransport : public Transport {
public:
//	static void Init();
//	static void Cleanup();

        enum class State { Disconnected, Connecting, Connected, Completed, Failed };
        using state_callback = std::function<void(State state)>;

	//enum class GatheringState { New = 0, InProgress = 1, Complete = 2 };

	using candidate_callback = std::function<void(const Candidate candidate)>;
	using gathering_state_callback = std::function<void(juice_state_t state)>;
        using recv_callback = std::function<void(unsigned char * data , size_t size )>;

	IceTransport( Configuration &config, Description &localDesc, Description & remoteDesc,  candidate_callback candidateCallback,
	             state_callback stateChangeCallback,
	             gathering_state_callback gatheringStateChangeCallback, recv_callback recvcallback );
	~IceTransport();

	Description::Role role() const;
	//GatheringState gatheringState() const;

        Description *getLocalDescription(Description::Type type);
        
	void setRemoteDescription(const Description *description);
	void addRemoteCandidate(const Candidate *candidate);
	void gatherLocalCandidates(string mid, std::vector<IceServer> additionalIceServers = {});
	void setIceAttributes(string uFrag, string pwd);



	bool getSelectedCandidatePair(Candidate *local, Candidate *remote);
        
        Agent agent;

private:


	//void changeGatheringState(GatheringState state);

	void processStateChange(unsigned int state);
	void processCandidate(const string &candidate);
	void processGatheringDone();
	void processTimeout();

	void addIceServer(IceServer server);
        
        
        //int ice_generate_sdp(Description *description,  char *buffer, size_t size);

        
        void cbDnsResolve(addrinfo* res) ;
        
        void cbNameResolve(  const char* hostname, const char* service,  void* ptr);



	Description::Role mRole;
	string mMid;
	std::chrono::milliseconds mTrickleTimeout;
	//std::atomic<GatheringState> mGatheringState;

	candidate_callback mCandidateCallback;
	gathering_state_callback mGatheringStateChangeCallback;

        int mTurnServersAdded;
        
        Description &localDes;
        Description &remoteDes;
        
public:    
        Description::Role role()
        {
            return Description::Role::ActPass;
        }
         
};

} // namespace rtc::impl

#endif
