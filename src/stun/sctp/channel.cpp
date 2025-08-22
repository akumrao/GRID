
#include "channel.h"
//#include "internals.hpp"
#include "base/logger.h"

using namespace base;

namespace rtc {

void Channel::triggerOpen() {
	mOpenTriggered = true;
	try {
		openCallback();
	} catch (const std::exception &e) {
		SWarn << "Uncaught exception in callback: " << e.what();
	}
	flushPendingMessages();
}

void Channel::triggerClosed() {
	try {
		closedCallback();
	} catch (const std::exception &e) {
		SWarn << "Uncaught exception in callback: " << e.what();
	}
}

void Channel::triggerError(string error) {
	try {
		errorCallback(std::move(error));
	} catch (const std::exception &e) {
		SWarn << "Uncaught exception in callback: " << e.what();
	}
}

void Channel::triggerAvailable(size_t count) {
	if (count == 1) {
		try {
			availableCallback();
		} catch (const std::exception &e) {
			SWarn << "Uncaught exception in callback: " << e.what();
		}
	}

	flushPendingMessages();
}

void Channel::triggerBufferedAmount(size_t amount) {
	size_t previous = bufferedAmount.exchange(amount);
	size_t threshold = bufferedAmountLowThreshold.load();
	if (previous > threshold && amount <= threshold) {
		try {
			bufferedAmountLowCallback();
		} catch (const std::exception &e) {
			SWarn << "Uncaught exception in callback: " << e.what();
		}
	}
}

void Channel::flushPendingMessages() {
	if (!mOpenTriggered)
		return;

	while (messageCallback) {
		auto next = receive();
		if (!next)
			break;

		try {
			messageCallback(*next);
		} catch (const std::exception &e) {
			SWarn << "Uncaught exception in callback: " << e.what();
		}
	}
}

void Channel::resetOpenCallback() {
	mOpenTriggered = false;
	openCallback = nullptr;
}

void Channel::resetCallbacks() {
	mOpenTriggered = false;
	openCallback = nullptr;
	closedCallback = nullptr;
	errorCallback = nullptr;
	availableCallback = nullptr;
	bufferedAmountLowCallback = nullptr;
	messageCallback = nullptr;
}


Channel::Channel()
{
}
Channel::~Channel() {
    resetCallbacks(); 
}



size_t Channel::maxMessageSize() const { return 0; }

//size_t Channel::bufferedAmount() const { 
//    return bufferedAmount;
//}

void Channel::onOpen(std::function<void()> callback) {

    openCallback = callback;

}

void Channel::onClosed(std::function<void()> callback) {closedCallback = callback; }

void Channel::onError(std::function<void(string error)> callback) {
	errorCallback = callback;
}

void Channel::onMessage(std::function<void(message_variant data)> callback) {
	messageCallback = callback;
	flushPendingMessages();
}

void Channel::onMessage(std::function<void(binary data)> binaryCallback,
                        std::function<void(string data)> stringCallback) {
	onMessage([binaryCallback, stringCallback](variant<binary, string> data) {
		std::visit(overloaded{binaryCallback, stringCallback}, std::move(data));
	});
}

void Channel::onBufferedAmountLow(std::function<void()> callback) {
	bufferedAmountLowCallback = callback;
}

void Channel::setBufferedAmountLowThreshold(size_t amount) {
	bufferedAmountLowThreshold = amount;
}



optional<message_variant> Channel::receive() { return receive(); }

optional<message_variant> Channel::peek() { return peek(); }

size_t Channel::availableAmount() const { return availableAmount(); }

void Channel::onAvailable(std::function<void()> callback) { availableCallback = callback; }




























} // namespace rtc::impl
