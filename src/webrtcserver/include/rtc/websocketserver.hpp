

#ifndef RTC_WEBSOCKETSERVER_H
#define RTC_WEBSOCKETSERVER_H

#if RTC_ENABLE_WEBSOCKET

#include "common.hpp"
#include "configuration.hpp"
#include "websocket.hpp"

namespace rtc {

namespace impl {

struct WebSocketServer;

}

class RTC_CPP_EXPORT WebSocketServer final : private CheshireCat<impl::WebSocketServer> {
public:
	using Configuration = WebSocketServerConfiguration;

	WebSocketServer();
	WebSocketServer(Configuration config);
	~WebSocketServer();

	void stop();

	uint16_t port() const;

	void onClient(std::function<void(shared_ptr<WebSocket>)> callback);

private:
	using CheshireCat<impl::WebSocketServer>::impl;
};

} // namespace rtc

#endif

#endif // RTC_WEBSOCKET_H
