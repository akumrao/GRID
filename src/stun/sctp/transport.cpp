
#include "transport.hpp"
#include "base/logger.h"
using namespace base;

namespace rtc {
Transport_del::Transport_del(Configuration &Config, shared_ptr<Transport_del> lower, state_callback callback  )
    : mLower(std::move(lower)), mStateChangeCallback(std::move(callback)), mConfig(Config){}

Transport_del::~Transport_del() {
	unregisterIncoming();

	if (mLower) {
		mLower->stop();
		mLower.reset();
	}
}

void Transport_del::registerIncoming() {
	if (mLower) {
		STrace << "Registering incoming callback";
		mLower->onRecv(std::bind(&Transport_del::incoming, this, std::placeholders::_1));
	}
}

void Transport_del::unregisterIncoming() {
	if (mLower) {
		STrace << "Unregistering incoming callback";
		mLower->onRecv(nullptr);
	}
}

Transport_del::State Transport_del::state() const { return mState; }

void Transport_del::onRecv(message_callback callback) { mRecvCallback = std::move(callback); }

void Transport_del::onStateChange(state_callback callback) {
	mStateChangeCallback = std::move(callback);
}

void Transport_del::start() { registerIncoming(); }

void Transport_del::stop() { unregisterIncoming(); }

bool Transport_del::send(message_ptr message) { return outgoing(message); }

void Transport_del::recv(message_ptr message) {
	try {
		mRecvCallback(message);
	} catch (const std::exception &e) {
		SWarn << e.what();
	}
}

void Transport_del::changeState(State state) {
	try {
		if (mState.exchange(state) != state)
			mStateChangeCallback(state);
	} catch (const std::exception &e) {
		SWarn << e.what();
	}
}

void Transport_del::incoming(message_ptr message) { recv(message); }

bool Transport_del::outgoing(message_ptr message) {
	if (mLower)
		return mLower->send(message);
	else
		return false;
}





void Transport_del::resolveStunServer( )
{
    
    for( IceServer &icesv:  mConfig.iceServers  )
    {
        SInfo << "resolve " <<  icesv.hostname << ":" << icesv.port;
        resolve(icesv.hostname, icesv.port, Application::uvGetLoop(), &icesv);
        //break;
    }
   
}

void Transport_del::cbDnsResolve(addrinfo* start)
{
    
    //IceServer *icesv = (IceServer *)ptr;
   // icesv->ip = ip;

    // SInfo <<  "IceServer" <<  ip << ":" << port  ;
    
                    char addr[40] = {'\0'};
                int port =0; 

                struct addrinfo*  res = start;
                
                for (;res != NULL; res = res->ai_next) 
                { 
                    
                    if (res->ai_family == AF_INET) {
                        // ipv4
                        //char c[17] = { '\0' };
                        
                        sockaddr_in* tmp  =   (sockaddr_in*) res->ai_addr;
                        port= htons(tmp->sin_port);
                        uv_ip4_name(tmp, addr, 16);
                        
        
                        
                    } else if (res->ai_family == AF_INET6) {
                        // ipv6
                        //char c[40] = { '\0' };
                        sockaddr_in6* tmp  =   (sockaddr_in6*) res->ai_addr;
                        port= htons(tmp->sin6_port);
                        uv_ip6_name(tmp, addr, 39);
                    }
                    LTrace("address ",  addr);
                    // uv_tcp_connect(connect_req, socket, (const struct sockaddr*) res->ai_addr, on_connect);

                }
}

void Transport_del::cbNameResolve(  const char* hostname, const char* service,  void* ptr)
{
     SInfo <<  "resoved " <<  hostname << ":" << service  ;
}




 
void Transport_del::resolveIp( Candidate *cand )
{
   // SInfo << "resolveName " <<  icesv.hostname << ":" << icesv.port << " addd " << cand;
    
   resolveIP(cand->resolved.addr,   Application::uvGetLoop(),  cand) ;
}


























}