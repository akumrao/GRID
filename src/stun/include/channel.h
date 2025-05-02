/**
 * Copyright (c) 2019-2021 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RTC_CHANNEL_H
#define RTC_CHANNEL_H

#include "common.h"

#include <atomic>
#include <functional>

namespace rtc {


class RTC_CPP_EXPORT Channel  {
public:
	virtual ~Channel();

	virtual void close() = 0;
	virtual bool send(std::vector<unsigned char> data) = 0; // returns false if buffered
	virtual bool send(const unsigned char *data, size_t size) = 0;

	virtual bool isOpen() const = 0;
	virtual bool isClosed() const = 0;
	virtual size_t maxMessageSize() const; // max message size in a call to send
	virtual size_t bufferAmount() const; // total size buffered to send

	void onOpen(std::function<void()> callback);
	void onClosed(std::function<void()> callback);
	void onError(std::function<void(string error)> callback);

	void onMessage(std::function<void(std::vector<unsigned char> data)> callback);
	void onMessage(std::function<void(std::vector<unsigned char> data)> binaryCallback,
	               std::function<void(string data)> stringCallback);

	void onBufferedAmountLow(std::function<void()> callback);
	void setBufferedAmountLowThreshold(size_t amount);

	void resetCallbacks();

	// Extended API
	std::vector<unsigned char> receive(); // only if onMessage unset
	std::vector<unsigned char> peek();    // only if onMessage unset
	size_t availableAmount() const;      // total size available to receive
	void onAvailable(std::function<void()> callback);
        
        
        
        
        
        
        
        
        
//        
//        virtual std::vector<unsigned char> receive() = 0;
//	virtual std::vector<unsigned char> peek() = 0;
//	virtual size_t availableAmount() const = 0;

	virtual void triggerOpen();
	virtual void triggerClosed();
	virtual void triggerError(string error);
	virtual void triggerAvailable(size_t count);
	virtual void triggerBufferedAmount(size_t amount);

	virtual void flushPendingMessages();
	void resetOpenCallback();
	//void resetCallbacks();

	std::function<void()> openCallback;
	std::function<void()> closedCallback;
	std::function<void(string error)> errorCallback;
	std::function<void()> availableCallback;
	std::function<void()> bufferedAmountLowCallback;

	std::function<void(std::vector<unsigned char> data)> messageCallback;

	std::atomic<size_t> bufferedAmount{ 0};
	std::atomic<size_t> bufferedAmountLowThreshold{0};

protected:
	std::atomic<bool> mOpenTriggered{ false};
        
        
        
        
        
        
        
        
        
        
        
        

protected:
	
};

} // namespace rtc

#endif // RTC_CHANNEL_H
