

#include "peerconnection.h"
#include "base/logger.h"

//#include "impl/certificate.h"
//#include "impl/dtlstransport.h"
//#include "impl/internals.h"
//#include "impl/peerconnection.h"
//#include "impl/sctptransport.h"
//#include "impl/threadpool.h"
//#include "impl/track.h"
//
//#if RTC_ENABLE_MEDIA
//#include "impl/dtlssrtptransport.hpp"
//#endif

#include <iomanip>
#include <set>
#include <thread>


using namespace base;

using namespace std::placeholders;

namespace rtc {

//PeerConnection::PeerConnection() : PeerConnection(Configuration()) {}

PeerConnection::PeerConnection( Configuration &config): mConfig(config)
{
       
	STrace << "Creating PeerConnection";

    	if (config.certificatePemFile.size() && config.keyPemFile.size()) {
//		std::promise<certificate_ptr> cert;
//		cert.set_value(std::make_shared<Certificate>(
//		    config.certificatePemFile->find(PemBeginCertificateTag) != string::npos
//		        ? Certificate::FromString(*config.certificatePemFile, *config.keyPemFile)
//		        : Certificate::FromFile(*config.certificatePemFile, *config.keyPemFile,
//		                                config.keyPemPass.value_or(""))));
		//mCertificate = cert.get_future();
	} else  {
		mCertificate = make_certificate(config.certificateType);
	} 
        
	if (config.portRangeEnd && config.portRangeBegin > config.portRangeEnd)
		throw std::invalid_argument("Invalid port range");

	if (config.mtu) {
		if (config.mtu < 576) // Min MTU for IPv4
			throw std::invalid_argument("Invalid MTU value");

		if (config.mtu > 1500) { // Standard Ethernet
			SWarn << "MTU set to " << config.mtu;
		} else {
			SError << "MTU set to " << config.mtu;
		}
	}
        negotiationNeeded();
}

PeerConnection::~PeerConnection() {
	STrace << "Destroying PeerConnection";
	close();
	
}

void PeerConnection::close() {
	STrace << "Closing PeerConnection";

	mNegotiationNeeded = false;

	// Close data channels asynchronously
	//mProcessor->enqueue(&PeerConnection::closeDataChannels, this);

	//closeTransports();
}


bool PeerConnection::negotiationNeeded() const {
//        mNegotiationNeeded.exchange() = true;
	return true;
}

Description PeerConnection::localDescription() const {
	std::lock_guard<std::recursive_mutex> lock(mLocalDescriptionMutex);
	return mLocalDescription;
}

Description PeerConnection::remoteDescription() const {
	std::lock_guard<std::recursive_mutex> lock(mRemoteDescriptionMutex);
	return mRemoteDescription;
}


//size_t PeerConnection::remoteMaxMessageSize() const { return impl()->remoteMaxMessageSize(); }

//bool PeerConnection::hasMedia() const {
//	auto local = localDescription();
//	return local && local->hasAudioOrVideo();
//}



void PeerConnection::processLocalDescription(Description *description) {
    
    	const uint16_t localSctpPort = 5000;
        //const size_t DEFAULT_LOCAL_MAX_MESSAGE_SIZE = 256 * 1024;
	const size_t localMaxMessageSize = mConfig.maxMessageSize;//(DEFAULT_LOCAL_MAX_MESSAGE_SIZE);
        
        
        description->clearMedia();
        
	uint16_t remoteSctpPort;
//	if (auto remote = remoteDescription())
//		remoteSctpPort = remote->sctpPort();

//	//std::lock_guard lock(mLocalDescriptionMutex);
//	mLocalDescription.emplace(std::move(description));
        
        
        	// Add application for data channels
        if (!description->hasApplication())
        {
                    //std::shared_lock<std::mutex> lock(mDataChannelsMutex);
                    if (!mDataChannels.size() || !mUnassignedDataChannels.size()) {
                            // Prevents mid collision with remote or local tracks
                            unsigned int m = 0;
                            while (description->hasMid(std::to_string(m)))
                                    ++m;

                            Description::Application app(std::to_string(m));
                            app.setSctpPort(localSctpPort);
                            app.setMaxMessageSize(localMaxMessageSize);

                            SDebug << "Adding application to local description, mid=\"" << app.mid()      << "\"";

                            description->addMedia(std::move(app));
                    }
        }

        // There might be no media at this point, for instance if the user deleted tracks
        if (description->mediaCount() == 0)
                throw std::runtime_error("No DataChannel or Track to negotiate");


	// Set local fingerprint (wait for certificate if necessary)
	description->setFingerprint(mCertificate->fingerprint());
        
       
        std::cout << "Issuing local description: " << description->generateSdp("\r\n") << std::endl << std::flush;
        

	STrace << "Issuing local description: " << description->generateSdp("\r\n");


	//updateTrackSsrcCache(description);

	{
		// Set as local description
		std::lock_guard<std::recursive_mutex> lock(mLocalDescriptionMutex);

		std::vector<Candidate> existingCandidates;
		if (mLocalDescription.localCanSdp.candidates_count) {
			existingCandidates = mLocalDescription.extractCandidates();
			//mCurrentLocalDescription.emplace(std::move(*mLocalDescription));
		}

		//mLocalDescription.emplace(description);
		mLocalDescription.addCandidates(std::move(existingCandidates));
	}

//	mProcessor.enqueue(&PeerConnection::trigger<Description>, shared_from_this(),
//	                   &localDescriptionCallback, std::move(description));

//	// Reciprocated tracks might need to be open
//	if (auto dtlsTransport = std::atomic_load(&mDtlsTransport);
//	    dtlsTransport && dtlsTransport->state() == Transport::State::Connected)
//		mProcessor.enqueue(&PeerConnection::openTracks, shared_from_this());
                     
         //   description->setFingerprint(mCertificate.get()->fingerprint());
        
        
        

	mLocalDescriptionCallback(*description);
}


string PeerConnection::localBundleMid() {
	//std::lock_guard lock(mLocalDescriptionMutex);
	return  mLocalDescription.bundleMid();
}


void PeerConnection::setLocalDescription(Description::Type type) {
	STrace << "Setting local description, type=" << Description::typeToString(type);

	SignalingState signalingState = mSignalingState.load();
	if (type == Description::Type::Rollback) {
		if (signalingState == SignalingState::HaveLocalOffer ||
		    signalingState == SignalingState::HaveLocalPranswer) {
			SDebug << "Rolling back pending local description";

			std::unique_lock< std::recursive_mutex> lock(mLocalDescriptionMutex);
//			if (mCurrentLocalDescription) {
//				std::vector<Candidate> existingCandidates;
//				if (mLocalDescription)
//					existingCandidates = mLocalDescription->extractCandidates();
//
//				mLocalDescription.emplace(std::move(*mCurrentLocalDescription));
//				mLocalDescription->addCandidates(std::move(existingCandidates));
//				mCurrentLocalDescription.reset();
//			}
			lock.unlock();

//			changeSignalingState(SignalingState::Stable);
		}
		return;
	}

	// Guess the description type if unspecified
	if (type == Description::Type::Unspec) {
		if (mSignalingState == SignalingState::HaveRemoteOffer)
			type = Description::Type::Answer;
		else
			type = Description::Type::Offer;
	}

//	// Only a local offer resets the negotiation needed flag
//	if (type == Description::Type::Offer && !mNegotiationNeeded.exchange(false)) {
//		SDebug << "No negotiation needed";
//		return;
//	}

	// Get the new signaling state
	SignalingState newSignalingState;
	switch (signalingState) {
	case SignalingState::Stable:
		if (type != Description::Type::Offer) {
			std::ostringstream oss;
			oss << "Unexpected local desciption type " << type << " in signaling state "
			    << signalingState;
			throw std::logic_error(oss.str());
		}
		newSignalingState = SignalingState::HaveLocalOffer;
		break;

	case SignalingState::HaveRemoteOffer:
	case SignalingState::HaveLocalPranswer:
		if (type != Description::Type::Answer && type != Description::Type::Pranswer) {
			std::ostringstream oss;
			oss << "Unexpected local description type " << type
			    << " description in signaling state " << signalingState;
			throw std::logic_error(oss.str());
		}
		newSignalingState = SignalingState::Stable;
		break;

	default: {
		std::ostringstream oss;
		oss << "Unexpected local description in signaling state " << signalingState << ", ignoring";
		SWarn << oss.str();
		return;
	}
	}

        IceTransport *iceTransport = initIceTransport();

	Description *local = iceTransport->getLocalDescription(type);
        
	processLocalDescription(local);

	changeSignalingState(newSignalingState);

        if (mGatheringState == GatheringState::New) {
		iceTransport->gatherLocalCandidates(localBundleMid());
	}
}


bool PeerConnection::changeSignalingState(SignalingState newState) {
	if (mSignalingState.exchange(newState) == newState)
		return false;

	std::ostringstream s;
	s << newState;
 	SInfo << "Changed signaling state to " << s.str();
	//mProcessor.enqueue(&PeerConnection::trigger<SignalingState>, shared_from_this(),
	//				   &signalingStateChangeCallback, newState);

	return true;
}

void PeerConnection::setRemoteDescription(Description description) {
	SDebug << "Setting remote description: " << string(description);

	if (description.type() == Description::Type::Rollback) {
		// This is mostly useless because we accept any offer
		STrace << "Rolling back pending remote description";
		//changeSignalingState(SignalingState::Stable);
		return;
	}

	//validateRemoteDescription(description);

	// Get the new signaling state
	SignalingState signalingState = mSignalingState.load();
	SignalingState newSignalingState;
	switch (signalingState) {
	case SignalingState::Stable:
		description.hintType(Description::Type::Offer);
		if (description.type() != Description::Type::Offer) {
			std::ostringstream oss;
			oss << "Unexpected remote " << description.type() << " description in signaling state "
			    << signalingState;
			throw std::logic_error(oss.str());
		}
		newSignalingState = SignalingState::HaveRemoteOffer;
		break;

	case SignalingState::HaveLocalOffer:
		description.hintType(Description::Type::Answer);
		if (description.type() == Description::Type::Offer) {
			// The ICE agent will automatically initiate a rollback when a peer that had previously
			// created an offer receives an offer from the remote peer
			setLocalDescription(Description::Type::Rollback);
			newSignalingState = SignalingState::HaveRemoteOffer;
			break;
		}
		if (description.type() != Description::Type::Answer &&
		    description.type() != Description::Type::Pranswer) {
			std::ostringstream oss;
			oss << "Unexpected remote " << description.type() << " description in signaling state "
			    << signalingState;
			throw std::logic_error(oss.str());
		}
		newSignalingState = SignalingState::Stable;
		break;

	case SignalingState::HaveRemotePranswer:
		description.hintType(Description::Type::Answer);
		if (description.type() != Description::Type::Answer &&
		    description.type() != Description::Type::Pranswer) {
			std::ostringstream oss;
			oss << "Unexpected remote " << description.type() << " description in signaling state "
			    << signalingState;
			throw std::logic_error(oss.str());
		}
		newSignalingState = SignalingState::Stable;
		break;

	default: {
		std::ostringstream oss;
		oss << "Unexpected remote description in signaling state " << signalingState;
		throw std::logic_error(oss.str());
	}
	}

	// Candidates will be added at the end, extract them for now
	auto remoteCandidates = description.extractCandidates();
	auto type = description.type();
	//processRemoteDescription(std::move(description));

	//changeSignalingState(newSignalingState);

	if (type == Description::Type::Offer) {
		// This is an offer, we need to answer
		setLocalDescription(Description::Type::Answer);
	} else {
		// This is an answer
//		auto iceTransport = std::atomic_load(&mIceTransport);
//		auto sctpTransport = std::atomic_load(&mSctpTransport);
//		if (!sctpTransport && iceTransport && iceTransport->role() == Description::Role::Active) {
//			// Since we assumed passive role during DataChannel creation, we need to shift the
//			// stream numbers by one to shift them from odd to even.
//			std::unique_lock lock(mDataChannelsMutex); // we are going to swap the container
//			decltype(mDataChannels) newDataChannels;
//			auto it = mDataChannels.begin();
//			while (it != mDataChannels.end()) {
//				auto channel = it->second.lock();
//				if (channel->stream() % 2 == 1)
//					channel->mStream -= 1;
//				newDataChannels.emplace(channel->stream(), channel);
//				++it;
//			}
//			std::swap(mDataChannels, newDataChannels);
//		}
	}
//
//	for (const auto &candidate : remoteCandidates)
//		addRemoteCandidate(candidate);
}

void PeerConnection::addRemoteCandidate(Candidate candidate) {
	STrace << "Adding remote candidate: " << string(candidate);
	//processRemoteCandidate(std::move(candidate));
}

//void PeerConnection::setMediaHandler(shared_ptr<MediaHandler> handler) {
//	impl()->setMediaHandler(std::move(handler));
//};
//
//shared_ptr<MediaHandler> PeerConnection::getMediaHandler() { return impl()->getMediaHandler(); };
//
//optional<string> PeerConnection::localAddress() const {
//	auto iceTransport = impl()->getIceTransport();
//	return iceTransport ? iceTransport->getLocalAddress() : nullopt;
//}
//
//optional<string> PeerConnection::remoteAddress() const {
//	auto iceTransport = impl()->getIceTransport();
//	return iceTransport ? iceTransport->getRemoteAddress() : nullopt;
//}
//
//uint16_t PeerConnection::maxDataChannelId() const { return impl()->maxDataChannelStream(); }
//
//shared_ptr<DataChannel> PeerConnection::createDataChannel(string label, DataChannelInit init) {
//	auto channelImpl = impl()->emplaceDataChannel(std::move(label), std::move(init));
//	auto channel = std::make_shared<DataChannel>(channelImpl);
//
//	if (!impl()->config.disableAutoNegotiation && impl()->signalingState.load() == SignalingState::Stable) {
//		// We might need to make a new offer
//		if (impl()->negotiationNeeded())
//			setLocalDescription(Description::Type::Offer);
//	}
//
//	return channel;
//}

//void PeerConnection::onDataChannel(
//    std::function<void(shared_ptr<DataChannel> dataChannel)> callback) {
//	mDataChannelCallback = callback;
//}

void PeerConnection::onLocalDescription(std::function<void(Description description)> callback) {
	mLocalDescriptionCallback = callback;
}

void PeerConnection::onLocalCandidate(std::function<void(Candidate candidate)> callback) {
	mLocalCandidateCallback = callback;
}

void PeerConnection::onStateChange(std::function<void(State state)> callback) {
	mStateChangeCallback = callback;
}

void PeerConnection::onGatheringStateChange(std::function<void(GatheringState state)> callback) {
	mGatheringStateChangeCallback = callback;
}

void PeerConnection::onSignalingStateChange(std::function<void(SignalingState state)> callback) {
	mSignalingStateChangeCallback = callback;
}

void PeerConnection::onIceStateChange(std::function<void(IceState state)> callback) {
	//impl()->iceStateChangeCallback = callback;
}


//std::shared_ptr<Track> PeerConnection::addTrack(Description::Media description) {
//#if !RTC_ENABLE_MEDIA
//	if (mTracks.empty()) {
//		PLOG_WARNING << "Tracks will be inative (not compiled with media support)";
//	}
//#endif
//
//	std::shared_ptr<Track> track;
//	if (auto it = mTracks.find(description.mid()); it != mTracks.end())
//		if (track = it->second.lock(); track)
//			track->setDescription(std::move(description));
//
//	if (!track) {
//		track = std::make_shared<Track>(std::move(description));
//		mTracks.emplace(std::make_pair(track->mid(), track));
//		mTrackLines.emplace_back(track);
//	}
//
//	// Renegotiation is needed for the new or updated track
//	mNegotiationNeeded = true;
//
//	return track;
//}
//
//void PeerConnection::onTrack(std::function<void(std::shared_ptr<Track>)> callback) {
//	mTrackCallback = callback;
//}
//
void PeerConnection::processLocalCandidate(Candidate candidate) 
{
	//std::lock_guard lock(mLocalDescriptionMutex);
//	if (!mLocalDescription)
//		throw std::logic_error("Got a local candidate without local description");

	if (mConfig.iceTransportPolicy == TransportPolicy::Relay &&
		candidate.type() != Candidate::Type::Relayed) {
		STrace << "Not issuing local candidate because of transport policy: " << candidate;
		return;
	}

	STrace << "Issuing local candidate: " << candidate;

	//candidate.resolve(Candidate::ResolveMode::Simple);
	mLocalDescription.addCandidate(candidate);

	//mProcessor.enqueue(&PeerConnection::trigger<Candidate>, shared_from_this(),
	//				   &localCandidateCallback, std::move(candidate));
}

void PeerConnection::iceState(IceTransport::State state) {

    switch (state) {
        case IceTransport::State::Connecting:
            //changeState(State::Connecting);
            break;
        case IceTransport::State::Failed:
           // changeState(State::Failed);
            break;
        case IceTransport::State::Connected:
         //   initDtlsTransport();
            break;
        case IceTransport::State::Disconnected:
          //  changeState(State::Disconnected);
            break;
        default:
            // Ignore
            break;
    };
}
void PeerConnection::iceGathering(IceTransport::GatheringState state) {

    switch (state) {
        case IceTransport::GatheringState::InProgress:
            //changeGatheringState(GatheringState::InProgress);
            break;
        case IceTransport::GatheringState::Complete:
           /// endLocalCandidates();
            //changeGatheringState(GatheringState::Complete);
            break;
        default:
            // Ignore
            break;
    }
}

IceTransport* PeerConnection::initIceTransport() 
{

	try {
		//if (auto transport = std::atomic_load(&mIceTransport))
		//	return transport;

		STrace << "Starting ICE transport";

		IceTransport *transport = new IceTransport(
		    mConfig, mLocalDescription , 
                     std::bind(&PeerConnection::processLocalCandidate, this, _1),    
		    std::bind(&PeerConnection::iceState, this, _1),
		    std::bind(&PeerConnection::iceGathering, this, _1)) ;

//		std::atomic_store(&mIceTransport, transport);
//		if (mState == State::Closed) {
//			mIceTransport.reset();
//			throw std::runtime_error("Connection is closed");
//		}
//		transport->start();
		return transport;

	} catch (const std::exception &e) {
		STrace << e.what();
		//changeState(State::Failed);
		throw std::runtime_error("ICE transport initialization failed");
	}
}

//void PeerConnection::onLocalCandidate(std::function<void(Candidate candidate)> callback) {
//	impl()->localCandidateCallback = callback;
//}
//
//void PeerConnection::onStateChange(std::function<void(State state)> callback) {
//	impl()->stateChangeCallback = callback;
//}
//
//
//void PeerConnection::onGatheringStateChange(std::function<void(GatheringState state)> callback) {
//	impl()->gatheringStateChangeCallback = callback;
//}
//
//void PeerConnection::onSignalingStateChange(std::function<void(SignalingState state)> callback) {
//	impl()->signalingStateChangeCallback = callback;
//}
//
//void PeerConnection::resetCallbacks() { impl()->resetCallbacks(); }
//
//bool PeerConnection::getSelectedCandidatePair(Candidate *locauto iceTransport = std::atomic_load(&mIceTransport);auto iceTransport = impl()->getIceTransport();
//	return iceTransport ? iceTransport->getSelectedCandidatePair(local, remote) : false;
//}

//void PeerConnection::clearStats() {
//	auto sctpTransport = std::atomic_load(&mSctpTransport);
//	if (sctpTransport)
//		return sctpTransport->clearStats();
//}

//size_t PeerConnection::bytesSent() {
//	auto sctpTransport = std::atomic_load(&mSctpTransport);
//	if (sctpTransport)
//		return sctpTransport->bytesSent();
//	return 0;
//}
//
//size_t PeerConnection::bytesReceived() {
//	auto sctpTransport = std::atomic_load(&mSctpTransport);
//	if (sctpTransport)
//		return sctpTransport->bytesReceived();
//	return 0;
//}

//std::optional<std::chrono::milliseconds> PeerConnection::rtt() {
//	auto sctpTransport = std::atomic_load(&mSctpTransport);
//	if (sctpTransport)
//		return sctpTransport->rtt();
//	return std::nullopt;
//}

//CertificateFingerprint PeerConnection::remoteFingerprint() {
//	return impl()->remoteFingerprint();
//}

std::ostream &operator<<(std::ostream &out, PeerConnection::State state) {
	using State = PeerConnection::State;
	const char *str;
	switch (state) {
	case State::New:
		str = "new";
		break;
	case State::Connecting:
		str = "connecting";
		break;
	case State::Connected:
		str = "connected";
		break;
	case State::Disconnected:
		str = "disconnected";
		break;
	case State::Failed:
		str = "failed";
		break;
	case State::Closed:
		str = "closed";
		break;
	default:
		str = "unknown";
		break;
	}
	return out << str;
}

std::ostream &operator<<(std::ostream &out, PeerConnection::IceState state) {
	using IceState = PeerConnection::IceState;
	const char *str;
	switch (state) {
	case IceState::New:
		str = "new";
		break;
	case IceState::Checking:
		str = "checking";
		break;
	case IceState::Connected:
		str = "connected";
		break;
	case IceState::Completed:
		str = "completed";
		break;
	case IceState::Failed:
		str = "failed";
		break;
	case IceState::Disconnected:
		str = "disconnected";
		break;
	case IceState::Closed:
		str = "closed";
		break;
	default:
		str = "unknown";
		break;
	}
	return out << str;
}

std::ostream &operator<<(std::ostream &out, PeerConnection::GatheringState state) {
	using GatheringState = PeerConnection::GatheringState;
	const char *str;
	switch (state) {
	case GatheringState::New:
		str = "new";
		break;
	case GatheringState::InProgress:
		str = "in-progress";
		break;
	case GatheringState::Complete:
		str = "complete";
		break;
	default:
		str = "unknown";
		break;
	}
	return out << str;
}

std::ostream &operator<<(std::ostream &out, PeerConnection::SignalingState state) {
	using SignalingState = PeerConnection::SignalingState;
	const char *str;
	switch (state) {
	case SignalingState::Stable:
		str = "stable";
		break;
	case SignalingState::HaveLocalOffer:
		str = "have-local-offer";
		break;
	case SignalingState::HaveRemoteOffer:
		str = "have-remote-offer";
		break;
	case SignalingState::HaveLocalPranswer:
		str = "have-local-pranswer";
		break;
	case SignalingState::HaveRemotePranswer:
		str = "have-remote-pranswer";
		break;
	default:
		str = "unknown";
		break;
	}
	return out << str;
}

} // namespace rtc
