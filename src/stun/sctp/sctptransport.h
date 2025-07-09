
#ifndef RTC_IMPL_SCTP_TRANSPORT_H
#define RTC_IMPL_SCTP_TRANSPORT_H

#include "common.h"
#include "configuration.h"
//#include "global.h"
//#include "processor.hpp"
//#include "queue.hpp"
#include "transport.h"

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include "usrsctp.h"

namespace rtc {

    struct SctpSettings {
	// For the following settings, not set means optimized default
	size_t recvBufferSize{1024 * 1024};                // in bytes
	size_t sendBufferSize{1024 * 1024};                // in bytes
	size_t maxChunksOnQueue{10 * 1024};              // in chunks
	size_t initialCongestionWindow{10};       // in MTUs
	size_t maxBurst{10};                      // in MTUs
	unsigned int congestionControlModule{0}; // 0: RFC2581, 1: HSTCP, 2: H-TCP, 3: RTCC
	std::chrono::milliseconds delayedSackTime{20};
	std::chrono::milliseconds minRetransmitTimeout{200};
	std::chrono::milliseconds maxRetransmitTimeout{10000};
	std::chrono::milliseconds initialRetransmitTimeout{1000};
	unsigned int maxRetransmitAttempts{5};
	std::chrono::milliseconds heartbeatInterval{10000};
};

//RTC_CPP_EXPORT void SetSctpSettings(SctpSettings s);
    
const uint16_t DEFAULT_SCTP_PORT = 5000; // SCTP port to use by default

const uint16_t MAX_SCTP_STREAMS_COUNT = 1024; // Max number of negotiated SCTP streams
                                              // RFC 8831 recommends 65535 but usrsctp needs a lot
                                              // of memory, Chromium historically limits to 1024.


    
class SctpTransport final : public Transport, public std::enable_shared_from_this<SctpTransport> {
public:
	static void Init();
	static void SetSettings(const SctpSettings &s);
	static void Cleanup();

	using amount_callback = std::function<void(uint16_t streamId, size_t amount)>;

	struct Ports {
		uint16_t local = DEFAULT_SCTP_PORT;
		uint16_t remote = DEFAULT_SCTP_PORT;
	};

	SctpTransport(shared_ptr<Transport> lower, const Configuration &config, Ports ports,
	              message_callback recvCallback, amount_callback bufferedAmountCallback,
	              state_callback stateChangeCallback);
	~SctpTransport();

	void onBufferedAmount(amount_callback callback);

	void start() override;
	void stop() override;
	bool send(message_ptr message) override; // false if buffered
	bool flush();
	void closeStream(unsigned int stream);
	void close();

	unsigned int maxStream() const;

	// Stats
	void clearStats();
	size_t bytesSent();
	size_t bytesReceived();
	std::chrono::milliseconds rtt();

private:
	// Order seems wrong but these are the actual values
	// See https://datatracker.ietf.org/doc/html/draft-ietf-rtcweb-data-channel-13#section-8
	enum PayloadId : uint32_t {
		PPID_CONTROL = 50,
		PPID_STRING = 51,
		PPID_BINARY_PARTIAL = 52,
		PPID_BINARY = 53,
		PPID_STRING_PARTIAL = 54,
		PPID_STRING_EMPTY = 56,
		PPID_BINARY_EMPTY = 57
	};

	struct sockaddr_conn getSockAddrConn(uint16_t port);

	void connect();
	void shutdown();
	void incoming(message_ptr message) override;
	bool outgoing(message_ptr message) override;

	void doRecv();
	void doFlush();
	void enqueueRecv();
	void enqueueFlush();
	bool trySendQueue();
	bool trySendMessage(message_ptr message);
	//void updateBufferedAmount(uint16_t streamId, ptrdiff_t delta);
	//void triggerBufferedAmount(uint16_t streamId, size_t amount);
	void sendReset(uint16_t streamId);

	void handleUpcall() noexcept;
	int handleWrite(unsigned char *data, size_t len, uint8_t tos, uint8_t set_df) noexcept;

	void processData(binary &&data, uint16_t streamId, PayloadId ppid);
	void processNotification(const union sctp_notification *notify, size_t len);

	const size_t mMaxMessageSize;
	const Ports mPorts;
	struct socket *mSock;
	uint16_t mNegotiatedStreamsCount{MAX_SCTP_STREAMS_COUNT};

	//Processor mProcessor;
	std::atomic<int> mPendingRecvCount{0};
	std::atomic<int> mPendingFlushCount{0};
	std::mutex mRecvMutex;
	//std::recursive_mutex mSendMutex; // buffered amount callback is synchronous
	std::queue<message_ptr> mSendQueue;
	bool mSendShutdown = false;
	std::map<uint16_t, size_t> mBufferedAmount;
	amount_callback mBufferedAmountCallback;

	std::mutex mWriteMutex;
	std::condition_variable mWrittenCondition;
	std::atomic<bool> mWritten{ false};     // written outside lock
	std::atomic<bool> mWrittenOnce{ false}; // same

	binary mPartialMessage, mPartialNotification;
	binary mPartialStringData, mPartialBinaryData;

	// Stats
	std::atomic<size_t> mBytesSent{0}, mBytesReceived{ 0};

	static void UpcallCallback(struct socket *sock, void *arg, int flags);
	static int WriteCallback(void *sctp_ptr, void *data, size_t len, uint8_t tos, uint8_t set_df);
	static void DebugCallback(const char *format, ...);

	class InstancesSet;
	static std::unique_ptr<InstancesSet> Instances;
};

} // namespace rtc::impl

#endif
