

#ifndef RTC_IMPL_ICE_TRANSPORT_H
#define RTC_IMPL_ICE_TRANSPORT_H

#include "candidate.h"
#include "common.h"
#include "configuration.h"
#include "description.h"
//#include "peerconnection.h"
#include <Connection.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace rtc {
	

        
class IceTransport : public Transport {
public:
	static void Init();
	static void Cleanup();

        enum class State { Disconnected, Connecting, Connected, Completed, Failed };
        using state_callback = std::function<void(State state)>;

	enum class GatheringState { New = 0, InProgress = 1, Complete = 2 };

	using candidate_callback = std::function<void(const Candidate &candidate)>;
	using gathering_state_callback = std::function<void(GatheringState state)>;

	IceTransport( Configuration &config, Description &description,  candidate_callback candidateCallback,
	             state_callback stateChangeCallback,
	             gathering_state_callback gatheringStateChangeCallback);
	~IceTransport();

	Description::Role role() const;
	GatheringState gatheringState() const;

        Description *getLocalDescription(Description::Type type);
        
	void setRemoteDescription(const Description *description);
	bool addRemoteCandidate(const Candidate *candidate);
	void gatherLocalCandidates(string mid, std::vector<IceServer> additionalIceServers = {});
	void setIceAttributes(string uFrag, string pwd);





	bool getSelectedCandidatePair(Candidate *local, Candidate *remote);

private:


	void changeGatheringState(GatheringState state);

	void processStateChange(unsigned int state);
	void processCandidate(const string &candidate);
	void processGatheringDone();
	void processTimeout();

	void addIceServer(IceServer server);
        
        
        int ice_generate_sdp(Description *description,  char *buffer, size_t size);
        int ice_generate_candidate_sdp(const ice_candidate_t *candidate, char *buffer, size_t size);
        
        void cbDnsResolve(addrinfo* res, std::string ip, int port,  void* ptr) ;

	Description::Role mRole;
	string mMid;
	std::chrono::milliseconds mTrickleTimeout;
	std::atomic<GatheringState> mGatheringState;

	candidate_callback mCandidateCallback;
	gathering_state_callback mGatheringStateChangeCallback;

        int mTurnServersAdded;
        
        Description description;
        
};

} // namespace rtc::impl

#endif
