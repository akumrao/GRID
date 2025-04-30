

#include "icetransport.h"
#include "configuration.h"

#include "Utils.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>

#include "base/logger.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>

using namespace base;
using namespace stun;
using namespace std::chrono_literals;
using std::chrono::system_clock;

namespace rtc {


const int MAX_TURN_SERVERS_COUNT = 2;

void IceTransport::Init() {
	// Dummy
}

void IceTransport::Cleanup() {
	// Dummy
}

IceTransport::IceTransport(const Configuration &config,  Description &description, candidate_callback candidateCallback,
                           state_callback stateChangeCallback,
                           gathering_state_callback gatheringStateChangeCallback): description(description),
    
      mMid("0"), mGatheringState(GatheringState::New),
      mCandidateCallback(std::move(candidateCallback)),
      mGatheringStateChangeCallback(std::move(gatheringStateChangeCallback))
      {

	SDebug << "Initializing ICE transport (libjuice)";



//	// Pick a STUN server
//	for (auto &server : servers) {
//		if (!server.hostname.empty() && server.type == IceServer::Type::Stun) {
//			if (server.port == 0)
//				server.port = 3478; // STUN UDP port
//			SInfo << "Using STUN server \"" << server.hostname << ":" << server.port << "\"";
//			jconfig.stun_server_host = server.hostname.c_str();
//			jconfig.stun_server_port = server.port;
//			break;
//		}
//	}

	// Bind address


//	// Port range
//	if (config.portRangeBegin > 1024 ||
//	    (config.portRangeEnd != 0 && config.portRangeEnd != 65535)) {
//		jconfig.local_port_range_begin = config.portRangeBegin;
//		jconfig.local_port_range_end = config.portRangeEnd;
//	}
//
//	// Create agent
//	mAgent = decltype(mAgent)(juice_create(&jconfig), juice_destroy);
//	if (!mAgent)
//		throw std::runtime_error("Failed to create the ICE agent");
//
//	// Add TURN servers
//	for (const auto &server : servers)
//		if (!server.hostname.empty() && server.type != IceServer::Type::Stun)
//			addIceServer(server);
}

void IceTransport::setIceAttributes(string uFrag, string pwd) {
//	if (juice_set_local_ice_attributes(mAgent.get(), uFrag.c_str(), pwd.c_str()) < 0) {
//		throw std::invalid_argument("Invalid ICE attributes");
//	}
}

void IceTransport::addIceServer(IceServer server) {
	if (server.hostname.empty())
		return;

	if (server.type != IceServer::Type::Turn) {
		SWarn << "Only TURN servers are supported as additional ICE servers";
		return;
	}

	if (server.relayType != IceServer::RelayType::TurnUdp) {
		SWarn << "TURN transports TCP and TLS are not supported with libjuice";
		return;
	}

	if (mTurnServersAdded >= MAX_TURN_SERVERS_COUNT)
		return;

	if (server.port == 0)
		server.port = 3478; // TURN UDP port

	SInfo << "Using TURN server \"" << server.hostname << ":" << server.port << "\"";
//	juice_turn_server_t turn_server = {};
//	turn_server.host = server.hostname.c_str();
//	turn_server.username = server.username.c_str();
//	turn_server.password = server.password.c_str();
//	turn_server.port = server.port;
//
//	if (juice_add_turn_server(mAgent.get(), &turn_server) != 0)
//		throw std::runtime_error("Failed to add TURN server");

	++mTurnServersAdded;
}



int IceTransport::ice_generate_candidate_sdp(const ice_candidate_t *candidate, char *buffer, size_t size) {
	const char *type = NULL;
	const char *suffix = NULL;
	switch (candidate->type) {
	case ICE_CANDIDATE_TYPE_HOST:
		type = "host";
		break;
	case ICE_CANDIDATE_TYPE_PEER_REFLEXIVE:
		type = "prflx";
		break;
	case ICE_CANDIDATE_TYPE_SERVER_REFLEXIVE:
		type = "srflx";
		suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	case ICE_CANDIDATE_TYPE_RELAYED:
		type = "relay";
		suffix = "raddr 0.0.0.0 rport 0"; // This is needed for compatibility with Firefox
		break;
	default:
		SError << "Unknown candidate type";
		return -1;
	}
	return snprintf(buffer, size, "a=candidate:%s %u UDP %u %s %s typ %s%s%s",
	                candidate->foundation, candidate->component, candidate->priority,
	                candidate->hostname, candidate->service, type, suffix ? " " : "",
	                suffix ? suffix : "");
}


int IceTransport::ice_generate_sdp(Description *description,  char *buffer, size_t size)
{
	if (!*description->ice_ufrag || !*description->ice_pwd)
		return -1;

	int len = 0;
	char *begin = buffer;
	char *end = begin + size;

	// Round 0: description
	// Round i with i>0 and i<count+1: candidate i-1
	// Round count + 1: end-of-candidates and ice-options lines
	for (int i = 0; i < description->candidates_count + 2; ++i) {
		int ret;
		if (i == 0) {
			ret = snprintf(begin, end - begin, "a=ice-ufrag:%s\r\na=ice-pwd:%s\r\n",
			               description->ice_ufrag, description->ice_pwd);
			if (description->ice_lite)
				ret = snprintf(begin, end - begin, "a=ice-lite\r\n");

		} else if (i < description->candidates_count + 1) {
			const ice_candidate_t *candidate = description->candidates + i - 1;
			if (candidate->type == ICE_CANDIDATE_TYPE_UNKNOWN ||
			    candidate->type == ICE_CANDIDATE_TYPE_PEER_REFLEXIVE)
				continue;
			char tmp[4096];
			if (ice_generate_candidate_sdp(candidate, tmp, 4096) < 0)
				continue;
			ret = snprintf(begin, end - begin, "%s\r\n", tmp);
		} else { // i == description->candidates_count + 1
			// RFC 8445 10. ICE Option: An agent compliant to this specification MUST inform the
			// peer about the compliance using the 'ice2' option.
			if (description->finished)
				ret = snprintf(begin, end - begin, "a=end-of-candidates\r\na=ice-options:ice2\r\n");
			else
				ret = snprintf(begin, end - begin, "a=ice-options:ice2,trickle\r\n");
		}
		if (ret < 0)
			return -1;

		len += ret;

		if (begin < end)
			begin += ret >= end - begin ? end - begin - 1 : ret;
	}
	return len;
}







IceTransport::~IceTransport() {
	SDebug << "Destroying ICE transport";

}

Description::Role IceTransport::role() const { return mRole; }

Description *IceTransport::getLocalDescription(Description::Type type)  {
	char sdp[4096];
        
         
        random_str64(description.ice_ufrag, 4 + 1);
        random_str64(description.ice_pwd, 22 + 1);
        description.ice_lite = false;
        description.candidates_count = 0;
        description.finished = false;
        SInfo << "Created local description: ufrag= "<<  description.ice_ufrag  <<  " pwd "  <<   description.ice_pwd;
        
        if (ice_generate_sdp(&description, sdp, 4096) < 0)
        {
            throw std::runtime_error("Failed to generate local SDP");
        }

	// RFC 5763: The endpoint that is the offerer MUST use the setup attribute value of
	// setup:actpass.
	// See https://www.rfc-editor.org/rfc/rfc5763.html#section-5
        
        description.readSdp( string(sdp), type, type == Description::Type::Offer ? Description::Role::ActPass : mRole);
	description.addIceOption("trickle");
        
        
	return &description;
}

void IceTransport::setRemoteDescription(const Description *description) {
	// RFC 5763: The answerer MUST use either a setup attribute value of setup:active or
	// setup:passive.
	// See https://www.rfc-editor.org/rfc/rfc5763.html#section-5
	if (description->type() == Description::Type::Answer &&
	    description->role() == Description::Role::ActPass)
		throw std::invalid_argument("Illegal role actpass in remote answer description");

	// RFC 5763: Note that if the answerer uses setup:passive, then the DTLS handshake
	// will not begin until the answerer is received, which adds additional latency.
	// setup:active allows the answer and the DTLS handshake to occur in parallel. Thus,
	// setup:active is RECOMMENDED.
	if (mRole == Description::Role::ActPass)
		mRole = description->role() == Description::Role::Active ? Description::Role::Passive
		                                                        : Description::Role::Active;

	if (mRole == description->role())
		throw std::invalid_argument("Incompatible roles with remote description");

	mMid = description->bundleMid();
//	if (juice_set_remote_description(mAgent.get(),
//	                                 description.generateApplicationSdp("\r\n").c_str()) < 0)
//		throw std::invalid_argument("Invalid ICE settings from remote SDP");
}

bool IceTransport::addRemoteCandidate(const Candidate *candidate) {
	// Don't try to pass unresolved candidates for more safety
	if (!candidate->isResolved())
		return false;

	//return juice_add_remote_candidate(mAgent.get(), string(candidate).c_str()) >= 0;
}

void IceTransport::gatherLocalCandidates(string mid, std::vector<IceServer> additionalIceServers) {
	mMid = std::move(mid);

	//std::shuffle(additionalIceServers.begin(), additionalIceServers.end(), utils::random_engine());
	for (const auto &server : additionalIceServers)
		addIceServer(server);

	// Change state now as candidates calls can be synchronous
	changeGatheringState(GatheringState::InProgress);

//	if (juice_gather_candidates(mAgent.get()) < 0) {
//		throw std::runtime_error("Failed to gather local ICE candidates");
//	}
}



bool IceTransport::getSelectedCandidatePair(Candidate *local, Candidate *remote) {
	char sdpLocal[256];
	char sdpRemote[256];
//	if (juice_get_selected_candidates(mAgent.get(), sdpLocal, 256,
//	                                  sdpRemote, 256) == 0) {
//		if (local) {
//			*local = Candidate(sdpLocal, mMid);
//			local->resolve(Candidate::ResolveMode::Simple);
//		}
//		if (remote) {
//			*remote = Candidate(sdpRemote, mMid);
//			remote->resolve(Candidate::ResolveMode::Simple);
//		}
//		return true;
//	}
	return false;
}

//bool IceTransport::send(message_ptr message) {
//	auto s = state();
//	if (!message || (s != State::Connected && s != State::Completed))
//		return false;
//
//	PLOG_VERBOSE << "Send size=" << message->size();
//	return outgoing(message);
//}
//
//bool IceTransport::outgoing(message_ptr message) {
//	// Explicit Congestion Notification takes the least-significant 2 bits of the DS field
//	int ds = int(message->dscp << 2);
//	return juice_send_diffserv(mAgent.get(), reinterpret_cast<const char *>(message->data()),
//	                           message->size(), ds) >= 0;
//}

void IceTransport::changeGatheringState(GatheringState state) {
	if (mGatheringState.exchange(state) != state)
		mGatheringStateChangeCallback(mGatheringState);
}

void IceTransport::processStateChange(unsigned int state)
{
//	switch (state) {
//	case JUICE_STATE_DISCONNECTED:
//		changeState(State::Disconnected);
//		break;
//	case JUICE_STATE_CONNECTING:
//		changeState(State::Connecting);
//		break;
//	case JUICE_STATE_CONNECTED:
//		changeState(State::Connected);
//		break;
//	case JUICE_STATE_COMPLETED:
//		changeState(State::Completed);
//		break;
//	case JUICE_STATE_FAILED:
//		changeState(State::Failed);
//		break;
//	};
}

void IceTransport::processCandidate(const string &candidate) {
	mCandidateCallback(Candidate(candidate, mMid));
}

void IceTransport::processGatheringDone() { changeGatheringState(GatheringState::Complete); }
/*
void IceTransport::StateChangeCallback(juice_agent_t *, juice_state_t state, void *user_ptr) {
	auto iceTransport = static_cast<rtc::impl::IceTransport *>(user_ptr);
	try {
		iceTransport->processStateChange(static_cast<unsigned int>(state));
	} catch (const std::exception &e) {
		SWarn << e.what();
	}
}

void IceTransport::CandidateCallback(juice_agent_t *, const char *sdp, void *user_ptr) {
	auto iceTransport = static_cast<rtc::impl::IceTransport *>(user_ptr);
	try {
		iceTransport->processCandidate(sdp);
	} catch (const std::exception &e) {
		SWarn << e.what();
	}
}

void IceTransport::GatheringDoneCallback(juice_agent_t *, void *user_ptr) {
	auto iceTransport = static_cast<rtc::impl::IceTransport *>(user_ptr);
	try {
		iceTransport->processGatheringDone();
	} catch (const std::exception &e) {
		SWarn << e.what();
	}
}
*/




} // namespace rtc::impl
