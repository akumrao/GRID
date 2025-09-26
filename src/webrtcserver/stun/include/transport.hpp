

#ifndef RTC_IMPL_TRANSPORT_H
#define RTC_IMPL_TRANSPORT_H

//#include "common.h"

#if DATACHANNEL
#include "init.hpp"
#endif 
//#include "internals.h"
#include "message.hpp"
#include "reliability.h"
#include <atomic>
#include <functional>
#include <memory>

#include "configuration.hpp"
#include "net/dns.h"
#include "candidate.hpp"



using namespace base::net;


namespace rtc {

    

 
 
    
class Transport: public GetAddrInfoReq, GetNameInfoReq
{
public:
    
    
       Transport( Configuration &Config): mConfig(Config)
        {
            
        }
        void resolveStunServer();
        void cbDnsResolve(addrinfo* res) override;
        void cbNameResolve( const char* hostname, const char* service,  void* ptr) override;
        void resolveIp( Candidate *certificate );
        const Configuration &mConfig;
     
        
	enum class State { Disconnected, Connecting, Connected, Completed, Failed };
	using state_callback = std::function<void(State state)>;

	Transport(const Configuration &Config, state_callback callback = nullptr );
	virtual ~Transport();

	void registerIncoming();
	void unregisterIncoming();
	State state() const;

	void onRecv(message_callback callback);
	void onStateChange(state_callback callback);

	virtual void start();
	virtual void stop();
	virtual bool send(message_ptr message);

protected:
	void recv(message_ptr message);
	void changeState(State state);
	virtual void incoming(message_ptr message);
	virtual bool outgoing(message_ptr message);

private:
    
    #if DATACHANNEL
	const init_token mInitToken = Init::Instance().token();
    #endif
        

	//shared_ptr<Transport> mLower;
	synchronized_callback<State> mStateChangeCallback;
	synchronized_callback<message_ptr> mRecvCallback;

	std::atomic<State> mState = State::Disconnected;
};

} // namespace rtc::impl

#endif
