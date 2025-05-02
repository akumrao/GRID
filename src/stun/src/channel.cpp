/**
 * Copyright (c) 2019-2021 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "channel.h"

#include "base/logger.h"


using namespace base;

namespace rtc {

Channel::~Channel() { resetCallbacks(); }

//Channel::Channel(impl_ptr<impl::Channel> impl) : CheshireCat<impl::Channel>(std::move(impl)) {}

size_t Channel::maxMessageSize() const { return 0; }

size_t Channel::bufferAmount() const { bufferedAmount; }

void Channel::onOpen(std::function<void()> callback) { openCallback = callback; }

void Channel::onClosed(std::function<void()> callback) { closedCallback = callback; }

void Channel::onError(std::function<void(string error)> callback) {errorCallback = callback;
}

void Channel::onMessage(std::function<void(std::vector<unsigned char> data)> callback) {
	messageCallback = callback;
	flushPendingMessages();
}

//void Channel::onMessage(std::function<void(std::vector<unsigned char> data)> binaryCallback,
//                        std::function<void(string data)> stringCallback) {
//	onMessage([binaryCallback, stringCallback](variant<std::vector<unsigned char>, string> data) {
//		std::visit(overloaded{binaryCallback, stringCallback}, std::move(data));
//	});
//}

void Channel::onBufferedAmountLow(std::function<void()> callback) {
	bufferedAmountLowCallback = callback;
}

void Channel::setBufferedAmountLowThreshold(size_t amount) {
	bufferedAmountLowThreshold = amount;
}

void Channel::resetCallbacks() 
{
    mOpenTriggered = false;
    openCallback = nullptr;
    closedCallback = nullptr;
    errorCallback = nullptr;
    availableCallback = nullptr;
    bufferedAmountLowCallback = nullptr;
    messageCallback = nullptr;

}

std::vector<unsigned char> Channel::receive() { receive(); }

std::vector<unsigned char> Channel::peek() {peek(); }

size_t Channel::availableAmount() const { return availableAmount(); }

void Channel::onAvailable(std::function<void()> callback) { availableCallback = callback; }





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
		if (!next.size())
			break;

		try {
			messageCallback(next);
		} catch (const std::exception &e) {
			SWarn << "Uncaught exception in callback: " << e.what();
		}
	}
}

void Channel::resetOpenCallback() {
	mOpenTriggered = false;
	openCallback = nullptr;
}



















} // namespace rtc
