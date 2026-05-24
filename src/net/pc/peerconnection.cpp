

#include "peerconnection.h"
#include "base/logger.h"

#include "net/certificate.h"

#include "icetransport.hpp"
//#include "internals.hpp"
//#include "peerconnection_imp.hpp"
#include "sctptransport.hpp"
//#include "threadpool.hpp"
#include "track.hpp"
//#include "datachannel.h"

#if RTC_ENABLE_MEDIA
#include "impl/dtlssrtptransport.hpp"
#endif

#include <iomanip>
#include <set>
#include <thread>


using namespace base;

using namespace std::placeholders;

namespace rtc {

//PeerConnection::PeerConnection() : PeerConnection(Configuration()) {}

PeerConnection::PeerConnection( Configuration &config1): config(config1)
{
       
    STrace << "Creating PeerConnection";
       if(!config.gconfig->mCertificate)
	if (config.gconfig->certificatePemFile.size() && config.gconfig->keyPemFile.size()) {
           config.gconfig->mCertificate =  
		    config.gconfig->certificatePemFile.find(PemBeginCertificateTag) != string::npos
		        ? Certificate::FromString(config.gconfig->certificatePemFile, config.gconfig->keyPemFile)
		        : Certificate::FromFile(config.gconfig->certificatePemFile, config.gconfig->keyPemFile,
		                                config.gconfig->keyPemPass);
	} else if (!config.gconfig->certificatePemFile.size() && !config.gconfig->keyPemFile.size()) {
		config.gconfig->mCertificate = make_certificate(config.certificateType);
	} else {
		throw std::invalid_argument(
		    "Either none or both certificate and key PEM files must be specified");
	}

	if (config.portRangeEnd && config.portRangeBegin > config.portRangeEnd)
		throw std::invalid_argument("Invalid port range");

	if (config.mtu)
        {
		if (config.mtu < 576) // Min MTU for IPv4
			throw std::invalid_argument("Invalid MTU value");

		if (config.mtu > 1500) { // Standard Ethernet
			SWarn << "MTU set to " << config.mtu;
		} else {
			STrace << "MTU set to " << config.mtu;
		}
	}
}

PeerConnection::~PeerConnection() {
	STrace << "Destroying PeerConnection";
	close();
	//delete mCertificate;
}

void PeerConnection::close() {
	STrace << "Closing PeerConnection";

	mNegotiationNeeded = false;
        
        auto ice = std::atomic_exchange(&mIceTransport, decltype(mIceTransport)(nullptr));

	// Close data channels asynchronously
	//mProcessor->enqueue(&PeerConnection::closeDataChannels, this);

	//closeTransports();
}


bool PeerConnection::negotiationNeeded() const {
//        mNegotiationNeeded.exchange() = true;
	return true;
}

optional<Description> PeerConnection::localDescription() const {
	std::lock_guard<std::recursive_mutex> lock(mLocalDescriptionMutex);
	return mLocalDescription;
}

optional<Description> PeerConnection::remoteDescription() const {
	std::lock_guard<std::recursive_mutex> lock(mRemoteDescriptionMutex);
	return mRemoteDescription;
}

#if DATACHANNEL

size_t PeerConnection::remoteMaxMessageSize() const { 

const size_t localMax = config.maxMessageSize;

size_t remoteMax = DEFAULT_REMOTE_MAX_MESSAGE_SIZE;
	std::lock_guard lock(mRemoteDescriptionMutex);
	//if (mRemoteDescription)
		if (auto *application = mRemoteDescription->application())
			if (auto max = application->maxMessageSize()) {
				// RFC 8841: If the SDP "max-message-size" attribute contains a maximum message
				// size value of zero, it indicates that the SCTP endpoint will handle messages
				// of any size, subject to memory capacity, etc.
				remoteMax = max > 0 ? max : std::numeric_limits<size_t>::max();
			}

	return std::min(remoteMax, localMax);

}

#endif

//bool PeerConnection::hasMedia() const {
//	auto local = localDescription();
//	return local && local->hasAudioOrVideo();
//}



//void PeerConnection::processLocalDescription(Description *description) {
//    
//    	const uint16_t localSctpPort = 5000;
//        //const size_t DEFAULT_LOCAL_MAX_MESSAGE_SIZE = 256 * 1024;
//	const size_t localMaxMessageSize = config.maxMessageSize;//(DEFAULT_LOCAL_MAX_MESSAGE_SIZE);
//        
//        
//        description->clearMedia();
//        
//	uint16_t remoteSctpPort;
////	if (auto remote = remoteDescription())
////		remoteSctpPort = remote->sctpPort();
//
////	//std::lock_guard lock(mLocalDescriptionMutex);
////	mLocalDescription.emplace(std::move(description));
//        
//        
//        	// Add application for data channels
//        if (!description->hasApplication())
//        {
//                    //std::shared_lock<std::mutex> lock(mDataChannelsMutex);
//                    if (!mDataChannels.size() || !mUnassignedDataChannels.size()) {
//                            // Prevents mid collision with remote or local tracks
//                            unsigned int m = 0;
//                            while (description->hasMid(std::to_string(m)))
//                                    ++m;
//
//                            Description::Application app(std::to_string(m));
//                            app.setSctpPort(localSctpPort);
//                            app.setMaxMessageSize(localMaxMessageSize);
//
//                            SDebug << "Adding application to local description, mid=\"" << app.mid()      << "\"";
//
//                            description->addMedia(std::move(app));
//                    }
//        }
//
//        // There might be no media at this point, for instance if the user deleted tracks
//        if (description->mediaCount() == 0)
//                throw std::runtime_error("No DataChannel or Track to negotiate");
//
//
//	// Set local fingerprint (wait for certificate if necessary)
//	description->setFingerprint(mCertificate->fingerprint());
//        
//       
//        //std::cout << "Issuing local description: " << description->generateSdp("\r\n") << std::endl << std::flush;
//        
//
//	STrace << "Issuing local description: " << description->generateSdp("\r\n");
//
//
//	//updateTrackSsrcCache(description); TBD
//
////	{
////		// Set as local description
////		std::lock_guard<std::recursive_mutex> lock(mLocalDescriptionMutex);
////
////		std::vector<Candidate> existingCandidates;
////		if (mLocalDescription.desc.candidates.size()) {
////			existingCandidates = mLocalDescription.extractCandidates();
////			//mCurrentLocalDescription.emplace(std::move(*mLocalDescription));
////		}
////
////		//mLocalDescription.emplace(description);
////		mLocalDescription.addCandidates(std::move(existingCandidates));
////	}
//
////	mProcessor.enqueue(&PeerConnection::trigger<Description>, shared_from_this(),
////	                   &localDescriptionCallback, std::move(description));
//
////	// Reciprocated tracks might need to be open
////	if (auto dtlsTransport = std::atomic_load(&mDtlsTransport);
////	    dtlsTransport && dtlsTransport->state() == Transport::State::Connected)
////		mProcessor.enqueue(&PeerConnection::openTracks, shared_from_this());
//                     
//         //   description->setFingerprint(mCertificate.get()->fingerprint());
//        
//        
//        
//
//	mLocalDescriptionCallback(*description);
//}



void PeerConnection::processLocalDescription(Description &description) {
    
        SInfo << "processLocalDescription";
        
	const uint16_t localSctpPort = DEFAULT_SCTP_PORT;
	const size_t localMaxMessageSize =0;
	    //config.maxMessageSize.value_or(DEFAULT_LOCAL_MAX_MESSAGE_SIZE);

	// Clean up the application entry the ICE transport might have added already (libnice)
	description.clearMedia();

	if (auto remote = remoteDescription()) {
		// Reciprocate remote description
		for (int i = 0; i < remote->mediaCount(); ++i)
			std::visit( // reciprocate each media
			    rtc::overloaded{
			        [&](Description::Application *remoteApp) {
				        std::shared_lock lock(mDataChannelsMutex);
				        if (!mDataChannels.empty() || !mUnassignedDataChannels.empty()) {
					        // Prefer local description
					        Description::Application app(remoteApp->mid());
					        app.setSctpPort(localSctpPort);
					        app.setMaxMessageSize(localMaxMessageSize);

					        SDebug << "Adding application to local description, mid=\""
					                   << app.mid() << "\"";

					        description.addMedia(std::move(app));
					        return;
				        }

				        auto reciprocated = remoteApp->reciprocate();
				        reciprocated.hintSctpPort(localSctpPort);
				        reciprocated.setMaxMessageSize(localMaxMessageSize);

				        SDebug << "Reciprocating application in local description, mid=\""
				                   << reciprocated.mid() << "\"";

				        description.addMedia(std::move(reciprocated));
			        },
			        [&](Description::Media *remoteMedia) {
//				        std::unique_lock lock(mTracksMutex); // we may emplace a track
//				        if (auto it = mTracks.find(remoteMedia->mid()); it != mTracks.end()) {
//					        // Prefer local description
//					        if (auto track = it->second.lock()) {
////						        auto media = track->description();
////
////						        SDebug << "Adding media to local description, mid=\""
////						                   << media.mid() << "\", removed=" << std::boolalpha
////						                   << media.isRemoved();
////
////						        description.addMedia(std::move(media));
//
//					        } else {
//						        auto reciprocated = remoteMedia->reciprocate();
//						        reciprocated.markRemoved();
//
//						        SDebug << "Adding media to local description, mid=\""
//						                   << reciprocated.mid()
//						                   << "\", removed=true (track is destroyed)";
//
//						        description.addMedia(std::move(reciprocated));
//					        }
//					        return;
//				        }

				        auto reciprocated = remoteMedia->reciprocate();
#if !RTC_ENABLE_MEDIA
				        if (!reciprocated.isRemoved()) {
					        // No media support, mark as removed
					        SWarn << "Rejecting track (not compiled with media support)";
					        reciprocated.markRemoved();
				        }
#endif

				        SDebug << "Reciprocating media in local description, mid=\""
				                   << reciprocated.mid() << "\", removed=" << std::boolalpha
				                   << reciprocated.isRemoved();

				        // Create incoming track
//				        auto track =
//				            std::make_shared<Track>(weak_from_this(), std::move(reciprocated));
//				        mTracks.emplace(std::make_pair(track->mid(), track));
//				        mTrackLines.emplace_back(track);
//				        triggerTrack(track); // The user may modify the track description
//
//				        auto handler = getMediaHandler();
//				        if (handler)
//					        handler->media(track->description());
//
//				        if (track->description().isRemoved())
//					        track->close();
//
//				        description.addMedia(track->description());
			        },
			    },
			    remote->media(i));

		// We need to update the SSRC cache for newly-created incoming tracks
		//updateTrackSsrcCache(*remote);
	}

	if (description.type() == Description::Type::Offer) {
		// This is an offer, add locally created data channels and tracks
		// Add media for local tracks
//		std::shared_lock lock(mTracksMutex);
//		for (auto it = mTrackLines.begin(); it != mTrackLines.end(); ++it) {
//			if (auto track = it->lock()) {
////				if (description.hasMid(track->mid()))
////					continue;
//
////				auto media = track->description();
////
////				SDebug << "Adding media to local description, mid=\"" << media.mid()
////				           << "\", removed=" << std::boolalpha << media.isRemoved();
////
////				description.addMedia(std::move(media));
//			}
//		}

		// Add application for data channels
		if (!description.hasApplication()) {
			std::shared_lock lock(mDataChannelsMutex);
			if (!mDataChannels.empty() || !mUnassignedDataChannels.empty()) {
				// Prevents mid collision with remote or local tracks
				unsigned int m = 0;
				while (description.hasMid(std::to_string(m)))
					++m;

				Description::Application app(std::to_string(m));
				app.setSctpPort(localSctpPort);
				app.setMaxMessageSize(localMaxMessageSize);

				SDebug << "Adding application to local description, mid=\"" << app.mid()
				           << "\"";

				description.addMedia(std::move(app));
			}
		}

		// There might be no media at this point, for instance if the user deleted tracks
		if (description.mediaCount() == 0)
			throw std::runtime_error("No DataChannel or Track to negotiate");
	}

	// Set local fingerprint (wait for certificate if necessary)
	description.setFingerprint(config.gconfig->mCertificate->fingerprint());

	STrace << "Issuing local description: " << description;

	if (description.mediaCount() == 0)
		throw std::logic_error("Local description has no media line");

	//updateTrackSsrcCache(description);

	{
		// Set as local description
		std::lock_guard lock(mLocalDescriptionMutex);

		std::vector<Candidate> existingCandidates;
		if (mLocalDescription) {
			existingCandidates = mLocalDescription->extractCandidates();
			//mCurrentLocalDescription.emplace(std::move(*mLocalDescription));
		}

		mLocalDescription.emplace(description);
		mLocalDescription->addCandidates(std::move(existingCandidates));
	}

//	mProcessor.enqueue(&PeerConnection::trigger<Description>, shared_from_this(),
//	                   &localDescriptionCallback, std::move(description));
        
        //localDescriptionCallback(description);
        mLocalDescriptionCallback(description);

	// Reciprocated tracks might need to be open
//	if (auto dtlsTransport = std::atomic_load(&mDtlsTransport);
//	    dtlsTransport && dtlsTransport->state() == Transport::State::Connected)
//		mProcessor.enqueue(&PeerConnection::openTracks, shared_from_this());
}




string PeerConnection::localBundleMid() {
	//std::lock_guard lock(mLocalDescriptionMutex);
	return  mLocalDescription->bundleMid();
}


void PeerConnection::setLocalDescription(Description::Type type) {
	

        std::unique_lock signalingLock(signalingMutex); 
                
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

        
    mIceTransport = initIceTransport();


   char sdp[4096];
   if (mIceTransport->agent.get_local_description(sdp, 4096) < 0)
		    throw std::runtime_error("Failed to generate local SDP");

			// RFC 5763: The endpoint that is the offerer MUST use the setup
			// attribute value of setup:actpass. See
			// https://www.rfc-editor.org/rfc/rfc5763.html#section-5
	Description local(string(sdp), type,
							type == Description::Type::Offer
								? Description::Role::ActPass
                                              : mIceTransport->mRole);
	local.addIceOption("trickle");

		
    //mIceTransport->getLocalDescription(type, local);
        
	processLocalDescription(local);

	changeSignalingState(newSignalingState);

    if (mGatheringState == GatheringState::New) 
    {
		mIceTransport->gatherLocalCandidates(localBundleMid());
    }
        
     STrace << "AgentNo " << mIceTransport->agent.agentNo << " SetLocalDescription, type=" << Description::typeToString(type);
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
    
          std::unique_lock signalingLock(signalingMutex);
////	SDebug << "Setting remote description: " << string(description);
////        mRemoteDescription = std::move(description);
////        iceTransport->setRemoteDescription(mRemoteDescription );            
////        return ;
//        
//	if (description.type() == Description::Type::Rollback) {
//		// This is mostly useless because we accept any offer
//		STrace << "Rolling back pending remote description";
//		//changeSignalingState(SignalingState::Stable);
//		return;
//	}
//
//	//validateRemoteDescription(description);
//
//	// Get the new signaling state
//	SignalingState signalingState = mSignalingState.load();
//	SignalingState newSignalingState;
//	switch (signalingState) {
//	case SignalingState::Stable:
//		description.hintType(Description::Type::Offer);
//		if (description.type() != Description::Type::Offer) {
//			std::ostringstream oss;
//			oss << "Unexpected remote " << description.type() << " description in signaling state "
//			    << signalingState;
//			throw std::logic_error(oss.str());
//		}
//		newSignalingState = SignalingState::HaveRemoteOffer;
//		break;
//
//	case SignalingState::HaveLocalOffer:
//		description.hintType(Description::Type::Answer);
//		if (description.type() == Description::Type::Offer) {
//			// The ICE agent will automatically initiate a rollback when a peer that had previously
//			// created an offer receives an offer from the remote peer
//			setLocalDescription(Description::Type::Rollback);
//			newSignalingState = SignalingState::HaveRemoteOffer;
//			break;
//		}
//		if (description.type() != Description::Type::Answer &&
//		    description.type() != Description::Type::Pranswer) {
//			std::ostringstream oss;
//			oss << "Unexpected remote " << description.type() << " description in signaling state "
//			    << signalingState;
//			throw std::logic_error(oss.str());
//		}
//		newSignalingState = SignalingState::Stable;
//		break;
//
//	case SignalingState::HaveRemotePranswer:
//		description.hintType(Description::Type::Answer);
//		if (description.type() != Description::Type::Answer &&
//		    description.type() != Description::Type::Pranswer) {
//			std::ostringstream oss;
//			oss << "Unexpected remote " << description.type() << " description in signaling state "
//			    << signalingState;
//			throw std::logic_error(oss.str());
//		}
//		newSignalingState = SignalingState::Stable;
//		break;
//
//	default: {
//		std::ostringstream oss;
//		oss << "Unexpected remote description in signaling state " << signalingState;
//		throw std::logic_error(oss.str());
//	}
//	}
//
//	// Candidates will be added at the end, extract them for now
//	auto remoteCandidates = description.extractCandidates();
//	auto type = description.type();
//	//processRemoteDescription(std::move(description));
//
//	//changeSignalingState(newSignalingState);
//
//	if (type == Description::Type::Offer) {
//		// This is an offer, we need to answer
//		setLocalDescription(Description::Type::Answer);
//	} else {
//		// This is an answer
////		auto iceTransport = std::atomic_load(&mIceTransport);
////		auto sctpTransport = std::atomic_load(&mSctpTransport);
////		if (!sctpTransport && iceTransport && iceTransport->role() == Description::Role::Active) {
////			// Since we assumed passive role during DataChannel creation, we need to shift the
////			// stream numbers by one to shift them from odd to even.
////			std::unique_lock lock(mDataChannelsMutex); // we are going to swap the container
////			decltype(mDataChannels) newDataChannels;
////			auto it = mDataChannels.begin();
////			while (it != mDataChannels.end()) {
////				auto channel = it->second.lock();
////				if (channel->stream() % 2 == 1)
////					channel->mStream -= 1;
////				newDataChannels.emplace(channel->stream(), channel);
////				++it;
////			}
////			std::swap(mDataChannels, newDataChannels);
////		}
//	}
////
////	for (const auto &candidate : remoteCandidates)
////		addRemoteCandidate(candidate);
//        
//        
        
        //////////////////////////////////////
        
        
       validateRemoteDescription(description);

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
                    
                    
//			rollbackLocalDescription();
//			>changeSignalingState(SignalingState::Stable);
//			signalingState = SignalingState::Stable;
//			newSignalingState = SignalingState::HaveRemoteOffer;
//			break;
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

	mIceTransport = initIceTransport();
	if (!mIceTransport)
		return; // closed

	mIceTransport->setRemoteDescription(description); // ICE transport might reject the description

	processRemoteDescription(std::move(description));
	changeSignalingState(newSignalingState);
	signalingLock.unlock();

	for (const auto &candidate : remoteCandidates)
		addRemoteCandidate(candidate);

	if (!config.disableAutoNegotiation) {
		switch (newSignalingState) {
		case SignalingState::Stable:
			// We might need to make a new offer
			if (negotiationNeeded())
				setLocalDescription(Description::Type::Offer);
			break;

		case SignalingState::HaveRemoteOffer:
			// We need to answer
			setLocalDescription(Description::Type::Answer);
			break;

		default:
			break;
		}
	}
        
        
        
        
        
        
        
}

void PeerConnection::addRemoteCandidate(Candidate candidate) {
    
        std::unique_lock signalingLock(signalingMutex);
	
	processRemoteCandidate(candidate);
}



void PeerConnection::processRemoteCandidate(Candidate candidate) {

    Candidate *cand = nullptr;

    auto iceTransport = initIceTransport();
    
    SInfo << "AgentNo " << iceTransport->agent.agentNo << " Adding remote candidate: " << string(candidate);
    
	{
		// Set as remote candidate
		std::lock_guard lock(mRemoteDescriptionMutex);
		if (!mRemoteDescription)
			throw std::logic_error("Got a remote candidate without remote description");

		if (!iceTransport)
			throw std::logic_error("Got a remote candidate without ICE transport");

		candidate.hintMid(mRemoteDescription->bundleMid());

		if (mRemoteDescription->hasCandidate(candidate))
			return; // already in description, ignore

		//if(candidate.resolve())
		//{
                   cand = mRemoteDescription->addCandidate(candidate);
               // }
	}

	if (cand )
        {
           
	     iceTransport->addRemoteCandidate(candidate);
	} else {
		// We might need a lookup, do it asynchronously
//		// We don't use the thread pool because we have no control on the timeout
//		if ((iceTransport = std::atomic_load(&mIceTransport))) {
//			weak_ptr<IceTransport> weakIceTransport{iceTransport};
//			std::thread t([weakIceTransport, candidate = std::move(candidate)]() mutable {
//				utils::this_thread::set_name("RTC resolver");
//				if (candidate.isResolved())
//					if (auto iceTransport = weakIceTransport.lock())
//						iceTransport->addRemoteCandidate(std::move(candidate));
//			});
//			t.detach();
//		}
            
            //exit(0);
	}



    /*
        Candidate  *cand  = mRemoteDescription->addCandidate(candidate);
            if(cand)
		iceTransport->addRemoteCandidate(candidate);
    
    */
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

#if DATACHANNEL
uint16_t PeerConnection::maxDataChannelId() const { return maxDataChannelStream(); }




uint16_t PeerConnection::maxDataChannelStream() const {
	auto sctpTransport = std::atomic_load(&mSctpTransport);
	return sctpTransport ? sctpTransport->maxStream() : (MAX_SCTP_STREAMS_COUNT - 1);
}

shared_ptr<DataChannel> PeerConnection::emplaceDataChannel(string label, DataChannelInit init) {
	std::unique_lock lock(mDataChannelsMutex); // we are going to emplace

	// If the DataChannel is user-negotiated, do not negotiate it in-band
	auto channel =
	    init.negotiated
	        ? std::make_shared<DataChannel>(weak_from_this(), std::move(label),
	                                        std::move(init.protocol), std::move(init.reliability))
	        : std::make_shared<OutgoingDataChannel>(weak_from_this(), std::move(label),
	                                                std::move(init.protocol),
	                                                std::move(init.reliability));

	// If the user supplied a stream id, use it, otherwise assign it later
	if (init.id) {
		uint16_t stream = *init.id;
		if (stream > maxDataChannelStream())
			throw std::invalid_argument("DataChannel stream id is too high");

		channel->assignStream(stream);
		mDataChannels.emplace(std::make_pair(stream, channel));

	} else {
		mUnassignedDataChannels.push_back(channel);
	}

	lock.unlock(); // we are going to call assignDataChannels()

	// If SCTP is connected, assign and open now
	auto sctpTransport = std::atomic_load(&mSctpTransport);
	if (sctpTransport && sctpTransport->state() == SctpTransport::State::Connected) {
		assignDataChannels();
		channel->open(sctpTransport);
	}

        
	return channel;
}


void PeerConnection::assignDataChannels() {
	std::unique_lock lock(mDataChannelsMutex); // we are going to emplace

        auto iceTransport = initIceTransport();
            
	//auto iceTransport = std::atomic_load(&iceTransport);
	if (!iceTransport)
		throw std::logic_error("Attempted to assign DataChannels without ICE transport");

	const uint16_t maxStream = maxDataChannelStream();
	for (auto it = mUnassignedDataChannels.begin(); it != mUnassignedDataChannels.end(); ++it) {
		auto channel = it->lock();
		if (!channel)
			continue;

		// RFC 8832: The peer that initiates opening a data channel selects a stream identifier
		// for which the corresponding incoming and outgoing streams are unused.  If the side is
		// acting as the DTLS client, it MUST choose an even stream identifier; if the side is
		// acting as the DTLS server, it MUST choose an odd one. See
		// https://www.rfc-editor.org/rfc/rfc8832.html#section-6
		uint16_t stream = (iceTransport->role() == Description::Role::Active) ? 0 : 1;
		while (true) {
			if (stream > maxStream)
				throw std::runtime_error("Too many DataChannels");

			if (mDataChannels.find(stream) == mDataChannels.end())
				break;

			stream += 2;
		}

		SDebug << "Assigning stream " << stream << " to DataChannel";

		channel->assignStream(stream);
		mDataChannels.emplace(std::make_pair(stream, channel));
	}

	mUnassignedDataChannels.clear();
}

shared_ptr<DataChannel> PeerConnection::createDataChannel(string label, DataChannelInit init) {
	auto channel = emplaceDataChannel(std::move(label), std::move(init));
	//auto channel = std::make_shared<DataChannel>();

//	if (!config.disableAutoNegotiation && signalingState == SignalingState::Stable) {
//		// We might need to make a new offer
//		if (negotiationNeeded())
//			setLocalDescription(Description::Type::Offer);
//	}

	return channel;
}


// Helper for PeerConnection::initXTransport methods: start and emplace the transport
template <typename T>
shared_ptr<T> emplaceTransport(PeerConnection *pc, shared_ptr<T> *member, shared_ptr<T> transport) {
	std::atomic_store(member, transport);
	try {
		//transport->start();
	} catch (...) {
		std::atomic_store(member, decltype(transport)(nullptr));
		throw;
	}

//	if (pc->closing.load() || pc->state.load() == PeerConnection::State::Closed) {
//		std::atomic_store(member, decltype(transport)(nullptr));
//		transport->stop();
//		return nullptr;
//	}

	return transport;
}


//shared_ptr<DtlsTransport> PeerConnection::initDtlsTransport() 
//{
//	try {
//		if (auto transport = std::atomic_load(&mDtlsTransport))
//			return transport;
//
//		STrace << "Starting DTLS transport";
//
//		CertificateFingerprint::Algorithm fingerprintAlgorithm;
//		{
//			std::lock_guard lock(mRemoteDescriptionMutex);
//			if (mRemoteDescription->fingerprint().value.size()) {
//				mRemoteFingerprintAlgorithm = mRemoteDescription->fingerprint().algorithm;
//			}
//			fingerprintAlgorithm = mRemoteFingerprintAlgorithm;
//		}
//
//		//auto lower = std::atomic_load(&mIceTransport);
//		//if (!lower)
//			//throw std::logic_error("No underlying ICE transport for DTLS transport");
//
//		auto &certificate = mCertificate;
//		auto verifierCallback = weak_bind(&PeerConnection::checkFingerprint, this, _1);
////		auto dtlsStateChangeCallback =
////		    [this, weak_this = weak_from_this()](DtlsTransport::State transportState) {
////			    auto shared_this = weak_this.lock();
////			    if (!shared_this)
////				    return;
////
////			    switch (transportState) {
////			    case DtlsTransport::State::Connected:
////                            {
////				    auto remote = remoteDescription(); 
////                                    if(remote.hasApplication())
////					    initSctpTransport();
////				    else{
////					    changeState(State::Connected);
////                                    }
////                                    
////
////				   // mProcessor.enqueue(&PeerConnection::openTracks, shared_from_this());
////                                    
////			    break;
////                            }
////			    case DtlsTransport::State::Failed:
////				    changeState(State::Failed);
////				  //  mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
////				    break;
////			    case DtlsTransport::State::Disconnected:
////				    changeState(State::Disconnected);
////				   // mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
////				    break;
////			    default:
////				    // Ignore
////				    break;
////			    }
////		    };
//
////		shared_ptr<DtlsTransport> transport;
////		auto local = localDescription();
////		if (config.forceMediaTransport || ( local.hasAudioOrVideo())) {
////#if RTC_ENABLE_MEDIA
////			STrace << "This connection requires media support";
////
////			// DTLS-SRTP
//////			transport = std::make_shared<DtlsSrtpTransport>(
//////			    lower, certificate, config.mtu, fingerprintAlgorithm, verifierCallback,
//////			    weak_bind(&PeerConnection::forwardMedia, this, _1), dtlsStateChangeCallback);
////#else
////			//PLOG_WARNING << "Ignoring media support (not compiled with media support)";
////#endif
////		}
////
////		if (!transport) {
////			// DTLS only
////			transport = std::make_shared<DtlsTransport>(config, iceTransport->role(), certificate, config.mtu,
////			                                            fingerprintAlgorithm, verifierCallback,
////			                                            dtlsStateChangeCallback);    
////		}
////
////		return emplaceTransport(this, &mDtlsTransport, std::move(transport));
//
//	} catch (const std::exception &e) {
//		SError << e.what();
//		changeState(State::Failed);
//		throw std::runtime_error("DTLS transport initialization failed");
//	}
//}



//shared_ptr<SctpTransport> PeerConnection::initSctpTransport()
//{
//	try {
//		if (auto transport = std::atomic_load(&mSctpTransport))
//			return transport;
//
//		STrace << "Starting SCTP transport";
//
//		auto lower = std::atomic_load(&mDtlsTransport);
//		if (!lower)
//			throw std::logic_error("No underlying DTLS transport for SCTP transport");
//
//		auto local = localDescription();
//		if (!local->application())
//			throw std::logic_error("Starting SCTP transport without local application description");
//
//		auto remote = remoteDescription();
//		if (!remote->application())
//			throw std::logic_error(
//			    "Starting SCTP transport without remote application description");
//
//		SctpTransport::Ports ports = {};
//		ports.local = local->application()->sctpPort();
//		ports.remote = remote->application()->sctpPort();
//
////		auto transport = std::make_shared<SctpTransport>(
////		    lower, config, std::move(ports), weak_bind(&PeerConnection::forwardMessage, this, _1),
////		    weak_bind(&PeerConnection::forwardBufferedAmount, this, _1, _2),
////		    [this, weak_this = weak_from_this()](SctpTransport::State transportState) {
////			    auto shared_this = weak_this.lock();
////			    if (!shared_this)
////				    return;
////
////			    switch (transportState) {
////			    case SctpTransport::State::Connected:
////				    changeState(State::Connected);
////				    assignDataChannels();
////				    openDataChannels();
////				    break;
////			    case SctpTransport::State::Failed:
////				    changeState(State::Failed);
////				    remoteClose();
////				    break;
////			    case SctpTransport::State::Disconnected:
////				    changeState(State::Disconnected);
////				    remoteClose();
////				    break;
////			    default:
////				    // Ignore
////				    break;
////			    }
////		    });
//
//		//return emplaceTransport(this, &mSctpTransport, std::move(transport));
//
//	} catch (const std::exception &e) {
//		SError << e.what();
//		changeState(State::Failed);
//		throw std::runtime_error("SCTP transport initialization failed");
//	}
//}















void PeerConnection::onDataChannel(
   std::function<void(std::shared_ptr<DataChannel> dataChannel)> callback) {
	mDataChannelCallback = callback;
        
      //  triggerPendingDataChannels();
}

//void PeerConnection::triggerPendingDataChannels() {
////	while (mDataChannelCallback) {
////		auto next = mPendingDataChannels.pop();
////		if (!next)
////			break;
////
////		auto impl = std::move(*next);
////
////		try {
////			dataChannelCallback(std::make_shared<rtc::DataChannel>(impl));
////		} catch (const std::exception &e) {
////			PLOG_WARNING << "Uncaught exception in callback: " << e.what();
////		}
////
////		impl->triggerOpen();
////	}
//    
//    //mDataChannelCallback.triggerOpen();
//    
//  
//}

#endif 

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

void PeerConnection::onRecv(recv_callback callback) {
	mRecvChangeCallback = callback;
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

 void PeerConnection::processDtlsState(DtlsTransport::DtlsState state)
 {
    if( state == DtlsTransport::DtlsState::CONNECTED)
    {
     
        auto transport = std::atomic_load(&mIceTransport);

        auto remote = remoteDescription();
        auto local = localDescription();

        if( remote->application())
        {

           SctpTransport::Ports ports = {};
           ports.local = local->application()->sctpPort();
           ports.remote = remote->application()->sctpPort();
           mSctpTransport.reset( transport->agent.socket->Connected(ports));

        }
     
    }
	
 }

 
  void PeerConnection::processSctpState(SctpTransport::State  state)
 {
//    if( state == SctpTransport::State::Connected)
//    {
//        
//        int x = 1;
//     
////        auto transport = std::atomic_load(&mIceTransport);
////
////        auto remote = remoteDescription();
////        auto local = localDescription();
////
////        if( remote->application())
////        {
////
////           SctpTransport::Ports ports = {};
////           ports.local = local->application()->sctpPort();
////           ports.remote = remote->application()->sctpPort();
////           mSctpTransport.reset( transport->agent.socket->Connected(ports));
////
////        }
//     
//    }
      
      
      	    switch (state) {
            case SctpTransport::State::Connected:
                    changeState(State::Connected);
                    assignDataChannels();
                    openDataChannels();
                    break;
            case SctpTransport::State::Failed:
                    changeState(State::Failed);
                    remoteClose();
                    break;
            case SctpTransport::State::Disconnected:
                    changeState(State::Disconnected);
                    remoteClose();
                    break;
            default:
                    {
                            // Ignore
                            break;
                    }
            };
	
 }
  
void PeerConnection::processLocalCandidate(Candidate candidate) 
{
//	//std::lock_guard lock(mLocalDescriptionMutex);
////	if (!mLocalDescription)
////		throw std::logic_error("Got a local candidate without local description");
//
//	if (config.iceTransportPolicy == TransportPolicy::Relay &&
//		candidate.type() != Candidate::Type::Relayed) {
//		STrace << "Not issuing local candidate because of transport policy: " << candidate;
//		return;
//	}
//
        auto iceTransport = initIceTransport();
	STrace << "AgentNo " << iceTransport->agent.agentNo << " Issuing local candidate: " << candidate;
//
//	//candidate.resolve(Candidate::ResolveMode::Simple);
//	// mLocalDescription.addCandidate(candidate, false);
//         
//        {
//        
//            if(mLocalCandidateCallback)
//            mLocalCandidateCallback(candidate);
//
//        }
//	//mProcessor.enqueue(&PeerConnection::trigger<Candidate>, shared_from_this(),
//	//				   &localCandidateCallback, std::move(candidate));
    
    
        
        STrace << "processLocalCandidate" <<   std::string(candidate);
        

    
	std::lock_guard lock(mLocalDescriptionMutex);
	if (!mLocalDescription)
		throw std::logic_error("Got a local candidate without local description");

	if (config.iceTransportPolicy == TransportPolicy::Relay &&
	    candidate.type() != Candidate::Type::Relayed) {
		STrace << "Not issuing local candidate because of transport policy: " << candidate;
		return;
	}   


          
          Candidate  *cand  =  mLocalDescription->addCandidate(candidate);
          if(cand)
	  {
//              mProcessor.enqueue(&PeerConnection::trigger<Candidate>, shared_from_this(),
//	                   &localCandidateCallback, std::move(*cand));
              if(mLocalCandidateCallback)
              mLocalCandidateCallback(*cand);
          }

    
    
}

void PeerConnection::iceState(IceTransport::State state) {

    switch (state) {
        case IceTransport::State::Connecting:
            changeIceState(IceState::Checking);
            changeState(State::Connecting);
            break;
        case IceTransport::State::Connected:
            changeIceState(IceState::Connected);
           // initDtlsTransport();
            break;
        case IceTransport::State::Completed:
            changeIceState(IceState::Completed);
            break;
        case IceTransport::State::Failed:
            changeIceState(IceState::Failed);
            changeState(State::Failed);
          //  mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
            break;
        case IceTransport::State::Disconnected:
            changeIceState(IceState::Disconnected);
            changeState(State::Disconnected);
            //mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
            break;
        default:
            // Ignore
            break;
    };
       
}


bool PeerConnection::changeIceState(IceState newState) 
{
	if (mIceState == newState)
		return false;

        
        mIceState = newState;
        
         if(mKIceStateChangeCallback)
            mKIceStateChangeCallback(newState);
	
	return true;
}



bool PeerConnection::changeState(State newState) 
{
	if(mState == newState)
        {
            return false;
        }
               
     mState =  newState;
   
     
    if(mStateChangeCallback)
    mStateChangeCallback(newState);
     
                
	return true;
}


bool PeerConnection::changeGatheringState(GatheringState newState) {
	
    
    if (mGatheringState == newState)
		return false;
        
     mGatheringState =  newState;
   
     
    if(mGatheringStateChangeCallback)
    mGatheringStateChangeCallback(newState);
            
	return true;
}

void PeerConnection::iceGathering(IceTransport::GatheringState state) {
    
    
    
    
    
   
    
    #if DATACHANNEL
    
//     switch (state) 
//    {
//        case JUICE_STATE_CONNECTED:
//         initDtlsTransport();
//            
//        break;
//        
//        default:
//            break; 
//         
//    };
    
    #endif
    
    



    switch (state) {
        case IceTransport::GatheringState::InProgress:
            changeGatheringState(GatheringState::InProgress);
            break;
        case IceTransport::GatheringState::Complete:
           /// endLocalCandidates();
            changeGatheringState(GatheringState::Complete);
            break;
        default:
            // Ignore
            break;
    }
}

shared_ptr<IceTransport> PeerConnection::initIceTransport() {

    

		if (auto transport = std::atomic_load(&mIceTransport))
			return transport;

		STrace << "Starting ICE transport";

//		IceTransport *transport = new IceTransport(
//		    mConfig, mLocalDescription , mRemoteDescription,
//                     std::bind(&PeerConnection::processLocalCandidate, this, _1),    
//		    std::bind(&PeerConnection::iceState, this, _1),
//		    std::bind(&PeerConnection::iceGathering, this, _1), std::bind(&PeerConnection::recv, this, _1, _2)   ) ;
//
////		std::atomic_store(&mIceTransport, transport);
////		if (mState == State::Closed) {
////			mIceTransport.reset();
////			throw std::runtime_error("Connection is closed");
////		}
////		transport->start();
//		return transport;
//
//	} catch (const std::exception &e) {
//		STrace << e.what();
//		//changeState(State::Failed);
//		throw std::runtime_error("ICE transport initialization failed");
//	}
//    

               auto transport = std::make_shared<IceTransport>(
		    config,  weak_bind(&PeerConnection::processDtlsState, this, _1), weak_bind(&PeerConnection::processSctpState, this, _1), weak_bind(&PeerConnection::forwardMessage, this, _1), weak_bind(&PeerConnection::processLocalCandidate, this, _1),
		    [this, weak_this = weak_from_this()](IceTransport::State transportState) {
			    auto shared_this = weak_this.lock();
			    if (!shared_this)
				    return;
			    switch (transportState) {
			    case IceTransport::State::Connecting:
				    changeIceState(IceState::Checking);
				    changeState(State::Connecting);
				    break;
			    case IceTransport::State::Connected:
                            {
                                
                                    //SInfo << "initDtlsTransport"
                                    
                                      auto iceTransport = initIceTransport();

                                        bool is_controlling = iceTransport->agent.m_mode == AGENT_MODE_CONTROLLING;

                                          if( config.console)
                                          iceTransport->mRole = iceTransport->agent.m_mode == AGENT_MODE_CONTROLLING ? Description::Role::Active: Description::Role::Passive;

                                        SInfo << "\033[36m" << "AgentNo " << iceTransport->agent.agentNo <<  " initDtlsTransport "  << " is_controlling " << is_controlling << "\033[0m";
                                          if(iceTransport->agent.m_selected_pair)
                                       iceTransport->agent.socket->InitDtls( is_controlling, iceTransport->agent.m_selected_pair->local->address() ,  iceTransport->agent.m_selected_pair->remote->resolved , mRemoteDescription->fingerprint());

                                  

				    changeIceState(IceState::Connected);
				  //  initDtlsTransport();
                                    

				    break;
                            }
			    case IceTransport::State::Completed:
				    changeIceState(IceState::Completed);
				    break;
			    case IceTransport::State::Failed:
				    changeIceState(IceState::Failed);
				    changeState(State::Failed);
				   // mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
				    break;
			    case IceTransport::State::Disconnected:
				    changeIceState(IceState::Disconnected);
				    changeState(State::Disconnected);
//				    mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
				    break;
			    default:
				    // Ignore
				    break;
			    }
		    },
		    [this, weak_this = weak_from_this()](IceTransport::GatheringState gatheringState) {
			    auto shared_this = weak_this.lock();
			    if (!shared_this)
				    return;
			    switch (gatheringState) {
			    case IceTransport::GatheringState::InProgress:
				    changeGatheringState(GatheringState::InProgress);
				    break;
			    case IceTransport::GatheringState::Complete:
				    endLocalCandidates();
				    changeGatheringState(GatheringState::Complete);
				    break;
			    default:
				    // Ignore
				    break;
			    }
		    });
                    
                   
                    return transport;
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

bool PeerConnection::getSelectedCandidatePair(Candidate *local, Candidate *remote)
{
    auto iceTransport = initIceTransport();
    return iceTransport->getSelectedCandidatePair(local, remote);
}


void PeerConnection::recv(unsigned char * data , size_t size)
{
     mRecvChangeCallback( data, size );
}

//int PeerConnection::send(unsigned char * data , size_t size)
//{
//    return miceTransport->agent.agent_send(data, size,0);
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

std::ostream &operator<<(std::ostream &out, juice_state_t state) {
	//using GatheringState = PeerConnection::GatheringState;
	const char *str;
	switch (state) {
	case JUICE_STATE_DISCONNECTED:
		str = "disconnecte";
		break;
	case JUICE_STATE_GATHERING:
		str = "gathering";
		break;
	case JUICE_STATE_CONNECTING:
		str = "connecting";
		break;
                
        case JUICE_STATE_CONNECTED:
                str = "connected";
                 break;
                
        case JUICE_STATE_COMPLETED:
		str = "complete";
		break;  
                
         case JUICE_STATE_FAILED:
		str = "failed";
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


 #if DATACHANNEL

//
//bool PeerConnection::checkFingerprint(const std::string &fingerprint) {
//	std::lock_guard lock(mRemoteDescriptionMutex);
//	mRemoteFingerprint = fingerprint;
//
//	if (!mRemoteDescription->fingerprint().value.size()
//			|| mRemoteFingerprintAlgorithm != mRemoteDescription->fingerprint().algorithm)
//		return false;
//
//	if (config.disableFingerprintVerification) {
//		STrace << "Skipping fingerprint validation";
//		return true;
//	}
//
//	auto expectedFingerprint = mRemoteDescription->fingerprint().value;
//	if (expectedFingerprint == fingerprint) {
//		STrace << "Valid fingerprint \"" << fingerprint << "\"";
//		return true;
//	}
//
//	SError << "Invalid fingerprint \"" << fingerprint << "\", expected \""
//	           << expectedFingerprint << "\"";
//	return false;
//}





void PeerConnection::forwardMessage(message_ptr message) {
	if (!message) {
		remoteCloseDataChannels();
		return;
	}

	auto iceTransport = initIceTransport();;
	auto sctpTransport = std::atomic_load(&mSctpTransport);
	if (!iceTransport || !sctpTransport)
		return;

	const uint16_t stream = uint16_t(message->stream);
	auto [channel, found] = findDataChannel(stream);

	if (DataChannel::IsOpenMessage(message)) {
		if (found) {
			// The stream is already used, the receiver must close the DataChannel
			SWarn << "Got open message on already used stream " << stream;
			if (channel && !channel->isClosed())
				channel->close();
			else
				sctpTransport->closeStream(message->stream);

			return;
		}

		const uint16_t remoteParity = (iceTransport->role() == Description::Role::Active) ? 1 : 0;
		if (stream % 2 != remoteParity) {
			// The odd/even rule is violated, the receiver must close the DataChannel
			SWarn << "Got open message violating the odd/even rule on stream " << stream;
			sctpTransport->closeStream(message->stream);
			return;
		}

		channel = std::make_shared<IncomingDataChannel>(weak_from_this(), sctpTransport);
		channel->assignStream(stream);
		channel->openCallback =	    weak_bind(&PeerConnection::triggerDataChannel, this, weak_ptr<DataChannel>{channel}); // arvind

		std::unique_lock lock(mDataChannelsMutex); // we are going to emplace
		mDataChannels.emplace(stream, channel);
	} else if (!found) {
		if (message->type == Message::Reset)
			return; // ignore

		// Invalid, close the DataChannel
		SWarn << "Got unexpected message on stream " << stream;
		sctpTransport->closeStream(message->stream);
		return;
	}

	if (message->type == Message::Reset) {
		// Incoming stream is reset, unregister it
		removeDataChannel(stream);
	}

	if (channel) {
		// Forward the message
		channel->incoming(message);
	} else {
		// DataChannel was destroyed, ignore
		SDebug << "Ignored message on stream " << stream << ", DataChannel is destroyed";
	}
}



void PeerConnection::forwardBufferedAmount(uint16_t stream, size_t amount) {
	[[maybe_unused]] auto [channel, found] = findDataChannel(stream);
	if (channel)
		channel->triggerBufferedAmount(amount);
}










void PeerConnection::openDataChannels() {
	if (auto transport = std::atomic_load(&mSctpTransport))
		iterateDataChannels([&](shared_ptr<DataChannel> channel) {
			if (!channel->isOpen())
				channel->open(transport);
		});
}

void PeerConnection::closeDataChannels() {
	iterateDataChannels([&](shared_ptr<DataChannel> channel) { channel->close(); });
}

void PeerConnection::remoteCloseDataChannels() {
	iterateDataChannels([&](shared_ptr<DataChannel> channel) { channel->remoteClose(); });
}


void PeerConnection::remoteClose() 
{
	close();
	//if (state.load() != State::Closed) 
        {
		// Close data channels and tracks asynchronously
		closeDataChannels();

//		closeTransports();
	}
}



//CertificateFingerprint PeerConnection::remoteFingerprint() {
//	std::lock_guard lock(mRemoteDescriptionMutex);
//	if (mRemoteFingerprint.size())
//		return {CertificateFingerprint{mRemoteFingerprintAlgorithm, mRemoteFingerprint}};
//	else
//		return {};
//}


std::pair<shared_ptr<DataChannel>, bool> PeerConnection::findDataChannel(uint16_t stream) {
	std::shared_lock lock(mDataChannelsMutex); // read-only
	if (auto it = mDataChannels.find(stream); it != mDataChannels.end())
		return std::make_pair(it->second.lock(), true);
	else
		return std::make_pair(nullptr, false);
}


void PeerConnection::iterateDataChannels(
    std::function<void(shared_ptr<DataChannel> channel)> func) {
	std::vector<shared_ptr<DataChannel>> locked;
	{
		std::shared_lock lock(mDataChannelsMutex); // read-only
		locked.reserve(mDataChannels.size());
		for (auto it = mDataChannels.begin(); it != mDataChannels.end(); ++it) {
			auto channel = it->second.lock();
			if (channel && !channel->isClosed())
                        {
                           // PLOG_INFO << it->first << " mid " <<  remoteMedia->mid();
			    locked.push_back(std::move(channel));
                        }
		}
	}

	for (auto &channel : locked) {
		try {
			func(std::move(channel));
		} catch (const std::exception &e) {
			SWarn << e.what();
		}
	}
}



bool PeerConnection::removeDataChannel(uint16_t stream) {
	std::unique_lock lock(mDataChannelsMutex); // we are going to erase
	return mDataChannels.erase(stream) != 0;
}




void PeerConnection::triggerDataChannel(weak_ptr<DataChannel> weakDataChannel) {
	auto dataChannel = weakDataChannel.lock();
	if (!dataChannel)
		return;
       mDataChannelCallback( std::move(dataChannel));
	//mProcessor->enqueue(mDataChannelCallback.wrap(), std::move(dataChannel));
}





#endif

void PeerConnection::endLocalCandidates() {
	std::lock_guard lock(mLocalDescriptionMutex);

		mLocalDescription->endCandidates();
}


void PeerConnection::validateRemoteDescription(const Description &description) 
{
	if (*description.desc.ice_ufrag == '\0')
		throw std::invalid_argument("Remote description has no ICE user fragment");

	if (*description.desc.ice_pwd == '\0' )
		throw std::invalid_argument("Remote description has no ICE password");

	if (!description.fingerprint().value.size())
		throw std::invalid_argument("Remote description has no valid fingerprint");

	if (description.mediaCount() == 0)
		throw std::invalid_argument("Remote description has no media line");

	int activeMediaCount = 0;
	for (int i = 0; i < description.mediaCount(); ++i)
		std::visit(rtc::overloaded{[&](const Description::Application *application) {
			                           if (!application->isRemoved())
				                           ++activeMediaCount;
		                           },
		                           [&](const Description::Media *media) {
			                           if (!media->isRemoved() ||
			                               media->direction() != Description::Direction::Inactive)
				                           ++activeMediaCount;
		                           }},
		           description.media(i));

	if (activeMediaCount == 0)
		throw std::invalid_argument("Remote description has no active media");

	STrace << "Remote description looks valid";
}


void PeerConnection::processRemoteDescription(Description description) 
{
	// Update the SSRC cache for existing tracks
	//updateTrackSsrcCache(description);

	{
		// Set as remote description
		std::lock_guard lock(mRemoteDescriptionMutex);

		std::vector<Candidate> existingCandidates;
          //SInfo << "Issuing local candidate: " <<  std::string(candidate);
		if (mRemoteDescription)
			existingCandidates = mRemoteDescription->extractCandidates();

		mRemoteDescription.emplace(description);
		mRemoteDescription->addCandidates(std::move(existingCandidates));
	}

//	auto dtlsTransport = std::atomic_load(&mDtlsTransport);
//	if (description.hasApplication()) {
//		auto sctpTransport = std::atomic_load(&mSctpTransport);
//		if (!sctpTransport && dtlsTransport &&
//		    dtlsTransport->state() == Transport::State::Connected)
//			initSctpTransport();
//	} else {
//		//mProcessor.enqueue(&PeerConnection::remoteCloseDataChannels, shared_from_this());
//	}

	//if (dtlsTransport && dtlsTransport->state() == Transport::State::Connected)
	//	mProcessor.enqueue(&PeerConnection::openTracks, shared_from_this());

}


} // namespace rtc
