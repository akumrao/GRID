
#ifndef RTC_IMPL_POLL_INTERRUPTER_H
#define RTC_IMPL_POLL_INTERRUPTER_H

#include "common.hpp"
#include "socket.hpp"

#if RTC_ENABLE_WEBSOCKET

namespace rtc::impl {

// Utility class to interrupt poll()
class PollInterrupter final {
public:
	PollInterrupter();
	~PollInterrupter();

	PollInterrupter(const PollInterrupter &other) = delete;
	void operator=(const PollInterrupter &other) = delete;

	void prepare(struct pollfd &pfd);
	void process(struct pollfd &pfd);
	void interrupt();

private:
#ifdef _WIN32
	socket_t mSock;
#else // assume POSIX
	int mPipeIn, mPipeOut;
#endif
};

} // namespace rtc::impl

#endif

#endif
