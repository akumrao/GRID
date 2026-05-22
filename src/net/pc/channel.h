
#ifndef RTC_CHANNEL_H
#define RTC_CHANNEL_H

#include "common.hpp"

#include <atomic>
#include <functional>

namespace rtc {

//struct Channel;


class  Channel  {
public:
	virtual ~Channel();

	virtual void close() = 0;

        virtual bool send(message_variant data) = 0; // returns false if buffered
	virtual bool send(const byte *data, size_t size) = 0;


	virtual bool isOpen() const = 0;
	virtual bool isClosed() const = 0;
	virtual size_t maxMessageSize() const; // max message size in a call to send
	//virtual size_t bufferedAmount() const; // total size buffered to send

	void onOpen(std::function<void()> callback);
	void onClosed(std::function<void()> callback);
	void onError(std::function<void(string error)> callback);

	void onMessage(std::function<void(message_variant data)> callback);
	void onMessage(std::function<void(binary data)> binaryCallback,
	               std::function<void(string data)> stringCallback);

	void onBufferedAmountLow(std::function<void()> callback);
	void setBufferedAmountLowThreshold(size_t amount);

	void resetCallbacks();

	// Extended API
	virtual optional<message_variant> receive(); // only if onMessage unset
	virtual optional<message_variant> peek();    // only if onMessage unset
	virtual size_t availableAmount() const;      // total size available to receive
	void onAvailable(std::function<void()> callback);

protected:
	Channel();
        
     public:   
        virtual void triggerOpen();
	virtual void triggerClosed();
	virtual void triggerError(string error);
	virtual void triggerAvailable(size_t count);
	virtual void triggerBufferedAmount(size_t amount);

	virtual void flushPendingMessages();
	void resetOpenCallback();
	//void resetCallbacks();


	synchronized_stored_callback<> openCallback;
	synchronized_stored_callback<> closedCallback;
	synchronized_stored_callback<string> errorCallback;
	synchronized_stored_callback<> availableCallback;
	synchronized_stored_callback<> bufferedAmountLowCallback;

	synchronized_callback<message_variant> messageCallback;

	std::atomic<size_t> bufferedAmount = 0;
	std::atomic<size_t> bufferedAmountLowThreshold = 0;

protected:
	std::atomic<bool> mOpenTriggered = false;
        
        
        
        
        
};

} // namespace rtc

#endif // RTC_CHANNEL_H
