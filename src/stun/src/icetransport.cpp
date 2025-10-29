

#include "icetransport.h"
#include "configuration.h"
#include "sdpcommon.h"
#include "Utils.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include "base/logger.h"


#include <sys/types.h>

using namespace base;
using namespace stun;
using namespace std::chrono_literals;
using std::chrono::system_clock;

namespace rtc {


const int MAX_TURN_SERVERS_COUNT = 2;



IceTransport::IceTransport( Configuration &config,  Description &localdescription,  Description &remoteDes, candidate_callback candidateCallback,
                           state_callback stateChangeCallback,
                           gathering_state_callback gatheringStateChangeCallback,  recv_callback recvcallback )

:  Transport(nullptr, std::move(stateChangeCallback)), mRole(Description::Role::ActPass),
      mMid("0"), mGatheringState(GatheringState::New),
      mCandidateCallback(std::move(candidateCallback)),
      mGatheringStateChangeCallback(std::move(gatheringStateChangeCallback)),
      agent(  (Configuration &)config, this)  
{

SDebug << "Initializing ICE transport";	
}

void IceTransport::setIceAttributes(string uFrag, string pwd) {
    
    if (agent.set_local_ice_attributes( uFrag.c_str(), pwd.c_str()) < 0) {
		throw std::invalid_argument("Invalid ICE attributes");
    }
    
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

//	if (mTurnServersAdded >= MAX_TURN_SERVERS_COUNT)
//		return;
//
//	if (server.port == 0)
//		server.port = 3478; // TURN UDP port
//
//	PLOG_INFO << "Using TURN server \"" << server.hostname << ":" << server.port << "\"";
//	juice_turn_server_t turn_server = {};
//	turn_server.host = server.hostname.c_str();
//	turn_server.username = server.username.c_str();
//	turn_server.password = server.password.c_str();
//	turn_server.port = server.port;
//
//	if (juice_add_turn_server(mAgent.get(), &turn_server) != 0)
//		throw std::runtime_error("Failed to add TURN server");
//
//	++mTurnServersAdded;
}

IceTransport::~IceTransport() {
	SInfo << "Destroying ICE transport";
//	mAgent.reset();
}

Description::Role IceTransport::role() const { return mRole; }

Description IceTransport::getLocalDescription(Description::Type type)  {
	char sdp[4096];
	if (agent.get_local_description( sdp, 4096) < 0 )
		throw std::runtime_error("Failed to generate local SDP");

	// RFC 5763: The endpoint that is the offerer MUST use the setup attribute value of
	// setup:actpass.
	// See https://www.rfc-editor.org/rfc/rfc5763.html#section-5
	Description desc;
        desc.readSdp( sdp, type, type == Description::Type::Offer ? Description::Role::ActPass : mRole);
	desc.addIceOption("trickle");
	return desc;
}

void IceTransport::setRemoteDescription(const Description &description) {
	// RFC 5763: The answerer MUST use either a setup attribute value of setup:active or
	// setup:passive.
	// See https://www.rfc-editor.org/rfc/rfc5763.html#section-5
//	if (description.type() == Description::Type::Answer &&
//	    description.role() == Description::Role::ActPass)
//		throw std::invalid_argument("Illegal role actpass in remote answer description");

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
	if (agent.agent_set_remote_description(description.generateApplicationSdp("\r\n").c_str()) < 0)
		throw std::invalid_argument("Invalid ICE settings from remote SDP");
}

bool IceTransport::addRemoteCandidate(const Candidate *candidate) {
	// Don't try to pass unresolved candidates for more safety
	//if (!candidate->isResolved())
		//return false;

	return agent.ice_add_remote_candidate( string(*candidate).c_str()) >= 0;
        
       // return agent.add_remote_candidate(mAgent.get(), string(candidate).c_str()) >= 0;
}

void IceTransport::gatherLocalCandidates(string mid, std::vector<IceServer> additionalIceServers) {
	mMid = std::move(mid);
        
        agent.localMid = mMid;
         

	//std::shuffle(additionalIceServers.begin(), additionalIceServers.end(), utils::random_engine());
	for (const auto &server : additionalIceServers)
		addIceServer(server);

	// Change state now as candidates calls can be synchronous
	changeGatheringState(GatheringState::InProgress);

        
         if (agent.gather_candidates() < 0) {
		throw std::runtime_error("Failed to gather local ICE candidates");
 	  }
}

optional<string> IceTransport::getLocalAddress()  {
    
    
    Candidate local_cand, remote_cand;
    if (getSelectedCandidatePair( &local_cand, &remote_cand))
            return nullopt;

    char ip[40];  uint16_t port;
    base::net::IP::AddressToString(local_cand.resolved, ip, 40, port);
    return ip;

}
optional<string> IceTransport::getRemoteAddress()  {
   
    
    	Candidate local_cand, remote_cand;
	if (getSelectedCandidatePair( &local_cand, &remote_cand))
		return nullopt;
   
    
        char ip[40];  uint16_t port;
        base::net::IP::AddressToString(remote_cand.resolved, ip, 40, port);
        return ip;

}

bool IceTransport::getSelectedCandidatePair(Candidate *local, Candidate *remote) {

    if(!agent.agent_get_selected_candidate_pair(local, remote))
    {
        return true;    
    }
    
    //kausal 9721238934

    return false;
}



bool IceTransport::send(message_ptr message) {
	auto s = state();
	if (!message || (s != State::Connected && s != State::Completed))
		return false;

	return outgoing(message);
}

bool IceTransport::outgoing(message_ptr message) {
	// Explicit Congestion Notification takes the least-significant 2 bits of the DS field
	//int ds = int(message->dscp << 2);
//	return juice_send_diffserv(mAgent.get(), reinterpret_cast<const char *>(message->data()),
//	                           message->size(), ds) >= 0;
        
    if( agent.agent_send( reinterpret_cast<uint8_t *>(message->data()), message->size(),0 ))
    {
        return true;
    }

    return false;
}



void IceTransport::onStateChangeCallback( juice_state_t state)
{
    processStateChange(static_cast<unsigned int>(state));
}
void IceTransport::onCandidateCallback( Candidate *candidate)
{
    mCandidateCallback(*candidate);
}
void IceTransport::onGatheringDoneCallback()
{
    processGatheringDone();
}
void IceTransport::onRecvCallback( unsigned char *data, size_t size)
{
    try {
                STrace << "onRecvCallback size="<<  size << " data " <<   data;
                auto b = reinterpret_cast<const byte *>(data);
                incoming(make_message(b, b + size));
        } catch (const std::exception &e) {
                SError << e.what();
        }
    
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



void IceTransport::changeGatheringState(GatheringState state) {
    
    int x = 0;
	if (mGatheringState.exchange(state) != state)
		mGatheringStateChangeCallback(mGatheringState);
}

void IceTransport::processGatheringDone() { 
    
    changeGatheringState(GatheringState::Complete); 

}


} // namespace rtc::impl
