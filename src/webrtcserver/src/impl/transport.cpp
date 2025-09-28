
#include "transport.hpp"

namespace rtc::impl {

Transport::Transport(shared_ptr<Transport> lower, state_callback callback)
    : mLower(std::move(lower)), mStateChangeCallback(std::move(callback)) 
{
    int x = 0;
}






void Transport::resolveStunServer( )
{
    
    for( const IceServer &icesv:  mConfig.iceServers  )
    {
        SInfo << "resolve " <<  icesv.hostname << ":" << icesv.port;
        resolve(icesv.hostname, icesv.port, Application::uvGetLoop(), (void*)&icesv);
        //break;
    }
   
}

void Transport::cbDnsResolve(addrinfo* start)
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

void Transport::cbNameResolve(  const char* hostname, const char* service,  void* ptr)
{
     SInfo <<  "resoved " <<  hostname << ":" << service  ;
}




 
void Transport::resolveIp( Candidate *cand )
{
   // SInfo << "resolveName " <<  icesv.hostname << ":" << icesv.port << " addd " << cand;
    
   resolveIP(cand->resolved.addr,   Application::uvGetLoop(),  cand) ;
}













Transport::~Transport() {
	unregisterIncoming();

	if (mLower) {
		mLower->stop();
		mLower.reset();
	}
}

void Transport::registerIncoming() {
	if (mLower) {
		PLOG_VERBOSE << "Registering incoming callback";
		mLower->onRecv(std::bind(&Transport::incoming, this, std::placeholders::_1));
	}
}

void Transport::unregisterIncoming() {
	if (mLower) {
		PLOG_VERBOSE << "Unregistering incoming callback";
		mLower->onRecv(nullptr);
	}
}

Transport::State Transport::state() const { return mState; }

void Transport::onRecv(message_callback callback) 
{
    mRecvCallback = std::move(callback); 
}

void Transport::onStateChange(state_callback callback) {
	mStateChangeCallback = std::move(callback);
}

void Transport::start() { registerIncoming(); }

void Transport::stop() { unregisterIncoming(); }

bool Transport::send(message_ptr message) 
{ 
    return outgoing(message); 
}

void Transport::recv(message_ptr message) {
	try {
		mRecvCallback(message);
	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
	}
}

void Transport::changeState(State state) {
	try {
		if (mState.exchange(state) != state)
			mStateChangeCallback(state);
	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
	}
}

void Transport::incoming(message_ptr message) {
    recv(message); 
}

bool Transport::outgoing(message_ptr message) {
	if (mLower)
		return mLower->send(message);
	else
		return false;
}

} // namespace rtc::impl
