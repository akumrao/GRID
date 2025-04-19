

#ifndef RTC_IMPL_WEBSOCKETSERVER_H
#define RTC_IMPL_WEBSOCKETSERVER_H

#if RTC_ENABLE_WEBSOCKET

#include "certificate.hpp"
#include "common.hpp"
#include "init.hpp"
#include "message.hpp"
#include "tcpserver.hpp"
#include "websocket.hpp"

#include "rtc/websocket.hpp"
#include "rtc/websocketserver.hpp"

#include <atomic>
#include <thread>

namespace rtc::impl {

struct WebSocketServer final : public std::enable_shared_from_this<WebSocketServer> {
	using Configuration = rtc::WebSocketServer::Configuration;

	WebSocketServer(Configuration config_);
	~WebSocketServer();

	void stop();

	const Configuration config;
	unique_ptr<TcpServer> tcpServer;
	synchronized_callback<shared_ptr<rtc::WebSocket>> clientCallback;

private:
	const init_token mInitToken = Init::Instance().token();

	void runLoop();

	certificate_ptr mCertificate;
	std::thread mThread;
	std::atomic<bool> mStopped;
};

} // namespace rtc::impl

#endif

#endif // RTC_IMPL_WEBSOCKET_H
