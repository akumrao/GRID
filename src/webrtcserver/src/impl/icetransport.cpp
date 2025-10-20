

#include "icetransport.hpp"
#include "configuration.hpp"
#include "internals.hpp"
#include "transport.hpp"
#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>

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

using namespace std::chrono_literals;
using std::chrono::system_clock;

namespace rtc::impl {

#if !USE_NICE // libjuice

const int MAX_TURN_SERVERS_COUNT = 2;

void IceTransport::Init() {
	// Dummy
}

void IceTransport::Cleanup() {
	// Dummy
}

IceTransport::IceTransport(const Configuration &config, candidate_callback candidateCallback,
                           state_callback stateChangeCallback,
                           gathering_state_callback gatheringStateChangeCallback)
    : Transport(nullptr, std::move(stateChangeCallback)), mRole(Description::Role::ActPass),
      mMid("0"), mGatheringState(GatheringState::New),
      mCandidateCallback(std::move(candidateCallback)),
      mGatheringStateChangeCallback(std::move(gatheringStateChangeCallback)),
      mAgent(nullptr, nullptr) {

	PLOG_DEBUG << "Initializing ICE transport (libjuice)";

	juice_log_level_t level;
	//auto logger = plog::get();
	switch (plog::debug) {
	case plog::none:
		level = JUICE_LOG_LEVEL_NONE;
		break;
	case plog::fatal:
		level = JUICE_LOG_LEVEL_FATAL;
		break;
	case plog::error:
		level = JUICE_LOG_LEVEL_ERROR;
		break;
	case plog::warning:
		level = JUICE_LOG_LEVEL_WARN;
		break;
	case plog::info:
	case plog::debug: // juice debug is output as verbose
		level = JUICE_LOG_LEVEL_VERBOSE;
		break;
	default:
		level = JUICE_LOG_LEVEL_VERBOSE;
		break;
	}
	//juice_set_log_handler(IceTransport::LogCallback);
	juice_set_log_level(level);
        juice_set_log_level(JUICE_LOG_LEVEL_VERBOSE);

	juice_config_t jconfig = {};
	jconfig.cb_state_changed = IceTransport::StateChangeCallback;
	jconfig.cb_candidate = IceTransport::CandidateCallback;
	jconfig.cb_gathering_done = IceTransport::GatheringDoneCallback;
	jconfig.cb_recv = IceTransport::RecvCallback;
	jconfig.user_ptr = this;

	if (config.enableIceTcp) {
		PLOG_WARNING << "ICE-TCP is not supported with libjuice";
	}

	if (config.enableIceUdpMux) {
		PLOG_DEBUG << "Enabling ICE UDP mux";
		jconfig.concurrency_mode = JUICE_CONCURRENCY_MODE_MUX;
	} else {
		jconfig.concurrency_mode = JUICE_CONCURRENCY_MODE_POLL;
	}

	// Randomize servers order
	std::vector<IceServer> servers = config.iceServers;
	std::shuffle(servers.begin(), servers.end(), utils::random_engine());

	// Pick a STUN server
	for (auto &server : servers) {
		if (!server.hostname.empty() && server.type == IceServer::Type::Stun) {
			if (server.port == 0)
				server.port = 3478; // STUN UDP port
			PLOG_INFO << "Using STUN server \"" << server.hostname << ":" << server.port << "\"";
			jconfig.stun_server_host = server.hostname.c_str();
			jconfig.stun_server_port = server.port;
			break;
		}
	}

	// Bind address
	if (config.bindAddress) {
		jconfig.bind_address = config.bindAddress->c_str();
	}

	// Port range
	if (config.portRangeBegin > 1024 ||
	    (config.portRangeEnd != 0 && config.portRangeEnd != 65535)) {
		jconfig.local_port_range_begin = config.portRangeBegin;
		jconfig.local_port_range_end = config.portRangeEnd;
	}

	// Create agent
	mAgent = decltype(mAgent)(juice_create(&jconfig), juice_destroy);
	if (!mAgent)
		throw std::runtime_error("Failed to create the ICE agent");

	// Add TURN servers
	for (const auto &server : servers)
		if (!server.hostname.empty() && server.type != IceServer::Type::Stun)
			addIceServer(server);
}

void IceTransport::setIceAttributes(string uFrag, string pwd) {
	if (juice_set_local_ice_attributes(mAgent.get(), uFrag.c_str(), pwd.c_str()) < 0) {
		throw std::invalid_argument("Invalid ICE attributes");
	}
}

void IceTransport::addIceServer(IceServer server) {
	if (server.hostname.empty())
		return;

	if (server.type != IceServer::Type::Turn) {
		PLOG_WARNING << "Only TURN servers are supported as additional ICE servers";
		return;
	}

	if (server.relayType != IceServer::RelayType::TurnUdp) {
		PLOG_WARNING << "TURN transports TCP and TLS are not supported with libjuice";
		return;
	}

	if (mTurnServersAdded >= MAX_TURN_SERVERS_COUNT)
		return;

	if (server.port == 0)
		server.port = 3478; // TURN UDP port

	PLOG_INFO << "Using TURN server \"" << server.hostname << ":" << server.port << "\"";
	juice_turn_server_t turn_server = {};
	turn_server.host = server.hostname.c_str();
	turn_server.username = server.username.c_str();
	turn_server.password = server.password.c_str();
	turn_server.port = server.port;

	if (juice_add_turn_server(mAgent.get(), &turn_server) != 0)
		throw std::runtime_error("Failed to add TURN server");

	++mTurnServersAdded;
}

IceTransport::~IceTransport() {
	PLOG_DEBUG << "Destroying ICE transport";
	mAgent.reset();
}

Description::Role IceTransport::role() const { return mRole; }

Description IceTransport::getLocalDescription(Description::Type type) const {
	char sdp[JUICE_MAX_SDP_STRING_LEN];
	if (juice_get_local_description(mAgent.get(), sdp, JUICE_MAX_SDP_STRING_LEN) < 0)
		throw std::runtime_error("Failed to generate local SDP");

	// RFC 5763: The endpoint that is the offerer MUST use the setup attribute value of
	// setup:actpass.
	// See https://www.rfc-editor.org/rfc/rfc5763.html#section-5
	Description desc(string(sdp), type,
	                 type == Description::Type::Offer ? Description::Role::ActPass : mRole);
	desc.addIceOption("trickle");
	return desc;
}

void IceTransport::setRemoteDescription(const Description &description) {
	// RFC 5763: The answerer MUST use either a setup attribute value of setup:active or
	// setup:passive.
	// See https://www.rfc-editor.org/rfc/rfc5763.html#section-5
	if (description.type() == Description::Type::Answer &&
	    description.role() == Description::Role::ActPass)
		throw std::invalid_argument("Illegal role actpass in remote answer description");

	// RFC 5763: Note that if the answerer uses setup:passive, then the DTLS handshake
	// will not begin until the answerer is received, which adds additional latency.
	// setup:active allows the answer and the DTLS handshake to occur in parallel. Thus,
	// setup:active is RECOMMENDED.
	if (mRole == Description::Role::ActPass)
		mRole = description.role() == Description::Role::Active ? Description::Role::Passive
		                                                        : Description::Role::Active;

	if (mRole == description.role())
		throw std::invalid_argument("Incompatible roles with remote description");

	mMid = description.bundleMid();
	if (juice_set_remote_description(mAgent.get(),
	                                 description.generateApplicationSdp("\r\n").c_str()) < 0)
		throw std::invalid_argument("Invalid ICE settings from remote SDP");
}

bool IceTransport::addRemoteCandidate(const Candidate &candidate) {
	// Don't try to pass unresolved candidates for more safety
	if (!candidate.isResolved())
		return false;

	return juice_add_remote_candidate(mAgent.get(), string(candidate).c_str()) >= 0;
}

void IceTransport::gatherLocalCandidates(string mid, std::vector<IceServer> additionalIceServers) {
	mMid = std::move(mid);

	std::shuffle(additionalIceServers.begin(), additionalIceServers.end(), utils::random_engine());
	for (const auto &server : additionalIceServers)
		addIceServer(server);

	// Change state now as candidates calls can be synchronous
	changeGatheringState(GatheringState::InProgress);

	if (juice_gather_candidates(mAgent.get()) < 0) {
		throw std::runtime_error("Failed to gather local ICE candidates");
	}
}

optional<string> IceTransport::getLocalAddress() const {
	char str[JUICE_MAX_ADDRESS_STRING_LEN];
	if (juice_get_selected_addresses(mAgent.get(), str, JUICE_MAX_ADDRESS_STRING_LEN, NULL, 0) ==
	    0) {
		return std::make_optional(string(str));
	}
	return nullopt;
}
optional<string> IceTransport::getRemoteAddress() const {
	char str[JUICE_MAX_ADDRESS_STRING_LEN];
	if (juice_get_selected_addresses(mAgent.get(), NULL, 0, str, JUICE_MAX_ADDRESS_STRING_LEN) ==
	    0) {
		return std::make_optional(string(str));
	}
	return nullopt;
}

bool IceTransport::getSelectedCandidatePair(Candidate *local, Candidate *remote) {
	char sdpLocal[JUICE_MAX_CANDIDATE_SDP_STRING_LEN];
	char sdpRemote[JUICE_MAX_CANDIDATE_SDP_STRING_LEN];
	if (juice_get_selected_candidates(mAgent.get(), sdpLocal, JUICE_MAX_CANDIDATE_SDP_STRING_LEN,
	                                  sdpRemote, JUICE_MAX_CANDIDATE_SDP_STRING_LEN) == 0) {
		if (local) {
			*local = Candidate(sdpLocal, mMid);
			local->resolve(Candidate::ResolveMode::Simple);
		}
		if (remote) {
			*remote = Candidate(sdpRemote, mMid);
			remote->resolve(Candidate::ResolveMode::Simple);
		}
		return true;
	}
	return false;
}

bool IceTransport::send(message_ptr message) {
	auto s = state();
	if (!message || (s != State::Connected && s != State::Completed))
		return false;

	PLOG_VERBOSE << "Send size=" << message->size();
	return outgoing(message);
}

bool IceTransport::outgoing(message_ptr message) {
	// Explicit Congestion Notification takes the least-significant 2 bits of the DS field
	int ds = int(message->dscp << 2);
	return juice_send_diffserv(mAgent.get(), reinterpret_cast<const char *>(message->data()),
	                           message->size(), ds) >= 0;
}

void IceTransport::changeGatheringState(GatheringState state) {
	if (mGatheringState.exchange(state) != state)
		mGatheringStateChangeCallback(mGatheringState);
}

void IceTransport::processStateChange(unsigned int state) {
	switch (state) {
	case JUICE_STATE_DISCONNECTED:
		changeState(State::Disconnected);
		break;
	case JUICE_STATE_CONNECTING:
		changeState(State::Connecting);
		break;
	case JUICE_STATE_CONNECTED:
		changeState(State::Connected);
		break;
	case JUICE_STATE_COMPLETED:
		changeState(State::Completed);
		break;
	case JUICE_STATE_FAILED:
		changeState(State::Failed);
		break;
	};
}

void IceTransport::processCandidate(const string &candidate) {
	mCandidateCallback(Candidate(candidate, mMid));
}

void IceTransport::processGatheringDone() { changeGatheringState(GatheringState::Complete); }

void IceTransport::StateChangeCallback(juice_agent_t *, juice_state_t state, void *user_ptr) {
	auto iceTransport = static_cast<rtc::impl::IceTransport *>(user_ptr);
	try {
		iceTransport->processStateChange(static_cast<unsigned int>(state));
	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
	}
}

void IceTransport::CandidateCallback(juice_agent_t *, const char *sdp, void *user_ptr) {
	auto iceTransport = static_cast<rtc::impl::IceTransport *>(user_ptr);
	try {
		iceTransport->processCandidate(sdp);
	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
	}
}

void IceTransport::GatheringDoneCallback(juice_agent_t *, void *user_ptr) {
	auto iceTransport = static_cast<rtc::impl::IceTransport *>(user_ptr);
	try {
		iceTransport->processGatheringDone();
	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
	}
}

void IceTransport::RecvCallback(juice_agent_t *, const char *data, size_t size, void *user_ptr) {
	auto iceTransport = static_cast<rtc::impl::IceTransport *>(user_ptr);
	try {
		PLOG_VERBOSE << "Incoming size=" << size;
		auto b = reinterpret_cast<const byte *>(data);
		iceTransport->incoming(make_message(b, b + size));
	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
	}
}

void IceTransport::LogCallback(juice_log_level_t level, const char *message) {
	plog::Severity severity;
	switch (level) {
	case JUICE_LOG_LEVEL_FATAL:
		severity = plog::fatal;
		break;
	case JUICE_LOG_LEVEL_ERROR:
		severity = plog::error;
		break;
	case JUICE_LOG_LEVEL_WARN:
		severity = plog::warning;
		break;
	case JUICE_LOG_LEVEL_INFO:
		severity = plog::info;
		break;
	default:
		severity = plog::verbose; // libjuice debug as verbose
		break;
	}
//	PLOG(severity) << "juice: " << message;
}

#else // USE_NICE == 1

#endif

} // namespace rtc::impl
