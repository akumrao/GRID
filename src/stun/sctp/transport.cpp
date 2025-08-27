
#include "transport.hpp"
#include "base/logger.h"
using namespace base;

namespace rtc {
Transport_del::Transport_del(shared_ptr<Transport_del> lower, state_callback callback)
    : mLower(std::move(lower)), mStateChangeCallback(std::move(callback)) {}

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


}