

#ifndef RTC_IMPL_TRANSPORT_H
#define RTC_IMPL_TRANSPORT_H

//#include "common.h"
//#include "init.h"
//#include "internals.h"
//#include "message.h"
#include "reliability.h"
#include <atomic>
#include <functional>
#include <memory>


namespace rtc {

    
    using binary = std::vector<unsigned char>;
    
    struct FrameInfo {
	FrameInfo(uint8_t payloadType, uint32_t timestamp) : payloadType(payloadType), timestamp(timestamp){};
	uint8_t payloadType; // Indicates codec of the frame
	uint32_t timestamp = 0; // RTP Timestamp
};

    
    
struct Message : binary {
	enum Type { Binary, String, Control, Reset };

	Message(const Message &message) = default;
	Message(size_t size, Type type_ = Binary) : binary(size), type(type_) {}



	Message(binary &&data, Type type_ = Binary) : binary(std::move(data)), type(type_) {}

	Type type;
	unsigned int stream = 0; // Stream id (SCTP stream or SSRC)
	unsigned int dscp = 0;   // Differentiated Services Code Point
	shared_ptr<Reliability> reliability;
	shared_ptr<FrameInfo> frameInfo;
};
    
    
    
    
using message_ptr = shared_ptr<Message>;
using message_callback = std::function<void(message_ptr message)>;
using message_vector = std::vector<message_ptr>;

inline size_t message_size_func(const message_ptr &m) {
	return m->type == Message::Binary || m->type == Message::String ? m->size() : 0;
}

    
    
class Transport {
public:
	enum class State { Disconnected, Connecting, Connected, Completed, Failed };
	using state_callback = std::function<void(State state)>;

	Transport(shared_ptr<Transport> lower = nullptr, state_callback callback = nullptr);
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
	//const init_token mInitToken = Init::Instance().token();

	shared_ptr<Transport> mLower;
        
        std::function<void(State state)> mStateChangeCallback;
	message_callback mRecvCallback;

	std::atomic<State> mState{State::Disconnected};
};

} // namespace rtc::impl

#endif
