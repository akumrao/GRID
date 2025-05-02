

#ifndef RTC_PEER_CONNECTION_H
#define RTC_PEER_CONNECTION_H

#include "candidate.h"
#include "common.h"
#include "configuration.h"
#include "datachannel.h"
#include "description.h"
#include "reliability.h"
#include "certificate.h"

#include "datachannel.h"

//#include "track.h"
#include "rtc.h"
#include <chrono>
#include <functional>

#include <mutex>

#include <atomic>

#include "icetransport.h"
#include <unordered_map>


//#include <shared_mutex>


namespace rtc {

 struct DataChannel;

struct RTC_CPP_EXPORT DataChannelInit {
//	Reliability reliability = {};
//	bool negotiated = false;
//	optional<uint16_t> id = nullopt;
//	string protocol = "";
};

struct RTC_CPP_EXPORT LocalDescriptionInit {
    string iceUfrag;
    string icePwd;
};

class RTC_CPP_EXPORT PeerConnection final : public std::enable_shared_from_this<PeerConnection> {
public:
	enum class State : int {
		New = RTC_NEW,
		Connecting = RTC_CONNECTING,
		Connected = RTC_CONNECTED,
		Disconnected = RTC_DISCONNECTED,
		Failed = RTC_FAILED,
		Closed = RTC_CLOSED
	};

	enum class IceState : int {
		New = RTC_ICE_NEW,
		Checking = RTC_ICE_CHECKING,
		Connected = RTC_ICE_CONNECTED,
		Completed = RTC_ICE_COMPLETED,
		Failed = RTC_ICE_FAILED,
		Disconnected = RTC_ICE_DISCONNECTED,
		Closed = RTC_ICE_CLOSED
	};

	enum class GatheringState : int {
		New = RTC_GATHERING_NEW,
		InProgress = RTC_GATHERING_INPROGRESS,
		Complete = RTC_GATHERING_COMPLETE
	};

	enum class SignalingState : int {
		Stable = RTC_SIGNALING_STABLE,
		HaveLocalOffer = RTC_SIGNALING_HAVE_LOCAL_OFFER,
		HaveRemoteOffer = RTC_SIGNALING_HAVE_REMOTE_OFFER,
		HaveLocalPranswer = RTC_SIGNALING_HAVE_LOCAL_PRANSWER,
		HaveRemotePranswer = RTC_SIGNALING_HAVE_REMOTE_PRANSWER,
	};

	PeerConnection();
	PeerConnection( Configuration &config);
	~PeerConnection();

	void close();

	Configuration &mConfig;
        
	//State state() const;
//	IceState iceState() const;
//	GatheringState gatheringState() const;
//	SignalingState signalingState() const;
	bool negotiationNeeded() const;
	bool hasMedia() const;
	Description localDescription() const;
	Description remoteDescription() const;
	size_t remoteMaxMessageSize() const;
	string localAddress() const;
	string remoteAddress() const;
	uint16_t maxDataChannelId() const;
	bool getSelectedCandidatePair(Candidate *local, Candidate *remote);

	void setLocalDescription(Description::Type type = Description::Type::Unspec);

	void setRemoteDescription(Description description);
	void addRemoteCandidate(Candidate candidate);
	void gatherLocalCandidates(std::vector<IceServer> additionalIceServers = {});

//	void setMediaHandler(shared_ptr<MediaHandler> handler);
//	shared_ptr<MediaHandler> getMediaHandler();

//	shared_ptr<DataChannel> createDataChannel(string label, DataChannelInit init = {});
//	void onDataChannel(std::function<void(std::shared_ptr<DataChannel> dataChannel)> callback);
//
//	 shared_ptr<Track> addTrack(Description::Media description);
//	void onTrack(std::function<void(std::shared_ptr<Track> track)> callback);
        
        void processLocalDescription(Description *description);
        
        bool changeSignalingState(SignalingState newState);
        
        string localBundleMid();

	void onLocalDescription(std::function<void(Description description)> callback);
	void onLocalCandidate(std::function<void(Candidate candidate)> callback);
	void onStateChange(std::function<void(State state)> callback);
	void onIceStateChange(std::function<void(IceState state)> callback);
	void onGatheringStateChange(std::function<void(GatheringState state)> callback);
	void onSignalingStateChange(std::function<void(SignalingState state)> callback);

	void resetCallbacks();
	CertificateFingerprint remoteFingerprint();
        
        shared_ptr<Certificate> mCertificate;


	// Stats
	void clearStats();
	size_t bytesSent();
	size_t bytesReceived();
	//optional<std::chrono::milliseconds> rtt();
        
        
        
//        std::atomic<State> mState;
//
//        State state = State::New;
//	IceState iceState = IceState::New;
//	GatheringState gatheringState = GatheringState::New;
	//SignalingState> signalingState = SignalingState::Stable;
	//std::atomic<bool> closing = false;

        std::atomic<State> mState{State::New};
        std::atomic<IceState> mIceState{IceState::New};
	std::atomic<GatheringState> mGatheringState{GatheringState::New};
	std::atomic<SignalingState> mSignalingState{SignalingState::Stable};
	std::atomic<bool> mNegotiationNeeded{false};
        


	//std::function<void<std::shared_ptr<DataChannel>> mDataChannelCallback;
	std::function<void(Description)> mLocalDescriptionCallback;
	std::function<void(Candidate)> mLocalCandidateCallback;
	std::function<void(State)> mStateChangeCallback;
	std::function<void(GatheringState )> mGatheringStateChangeCallback;
	std::function<void(SignalingState)> mSignalingStateChangeCallback;
	//std::function<void<std::shared_ptr<Track>> mTrackCallback;
        
        
        
        mutable std::recursive_mutex mLocalDescriptionMutex, mRemoteDescriptionMutex;
        Description mLocalDescription, mRemoteDescription;  

        void processLocalCandidate(Candidate candidate);
         
        IceTransport *initIceTransport();

        void iceState(IceTransport::State state);

        void iceGathering(IceTransport::GatheringState state);
               
        
        std::unordered_map<uint16_t, DataChannel*> mDataChannels; // by stream ID
	std::vector<DataChannel*> mUnassignedDataChannels;
	std::mutex mDataChannelsMutex;
        
};

RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, PeerConnection::State state);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, PeerConnection::IceState state);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, PeerConnection::GatheringState state);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, PeerConnection::SignalingState state);

} // namespace rtc

#endif
