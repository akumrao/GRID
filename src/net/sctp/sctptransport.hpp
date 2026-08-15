#ifndef RTC_IMPL_SCTP_TRANSPORT_H
#define RTC_IMPL_SCTP_TRANSPORT_H

#include "common.h"
#include "configuration.h"
#include "global.hpp"
#include "message.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include "usrsctp.h"

namespace rtc {

    class SctpTransport {
    public:
        using amount_callback = std::function<void(uint16_t streamId, size_t amount)>;

        struct Ports {
            uint16_t local = DEFAULT_SCTP_PORT;
            uint16_t remote = DEFAULT_SCTP_PORT;
        };

        int onRecvSctpData_obj(struct socket* sock, union sctp_sockstore & addr, void* data, size_t len, struct sctp_rcvinfo &rcv, int flags, void* ulpInfo);

        enum class State {
            Disconnected, Connecting, Connected, Completed, Failed
        };

        State state() const;
        void changeState(State state);

        std::atomic<State> mState = State::Disconnected;

        enum class StreamDirection {
            INCOMING = 1,
            OUTGOING
        };

        class Listener {
        public:
            virtual void OnSctpState(SctpTransport::State) = 0;
            virtual void OnSctpTransportConnecting(SctpTransport* sctpAssociation) = 0;
            virtual void OnSctpTransportConnected(SctpTransport* sctpAssociation) = 0;
            virtual void OnSctpTransportFailed(SctpTransport* sctpAssociation) = 0;
            virtual void OnSctpTransportClosed(SctpTransport* sctpAssociation) = 0;
            virtual void OnSctpTransportSendData(
              SctpTransport* sctpAssociation, const uint8_t* data, size_t len) = 0;
            virtual void OnSctpTransportMessageReceived(SctpTransport* sctpAssociation, message_ptr message) = 0;
        };

        SctpTransport(Listener* listener, int agentNo, const Configuration &config, Ports ports);
        ~SctpTransport();

        bool send(message_ptr message);
        bool flush();
        void closeStream(unsigned int stream);
        void close();

        unsigned int maxStream() const;

        void clearStats();
        size_t bytesSent();
        size_t bytesReceived();
        std::chrono::milliseconds rtt();

        void start();
        void stop();
        void incoming(message_ptr message);
        void shutdown();

    private:

        enum PayloadId : uint32_t {
            PPID_CONTROL = 50,
            PPID_STRING = 51,
            PPID_BINARY_PARTIAL = 52,
            PPID_BINARY = 53,
            PPID_STRING_PARTIAL = 54,
            PPID_STRING_EMPTY = 56,
            PPID_BINARY_EMPTY = 57
        };

        uint16_t mNegotiatedStreamsCount{MAX_SCTP_STREAMS_COUNT};

        struct sockaddr_conn getSockAddrConn(uint16_t port);

    private:
        void connect();

        bool outgoing(message_ptr message);

        void recv(message_ptr message);

        void doRecv();
        void doFlush();
        void enqueueRecv();
        void enqueueFlush();
        bool trySendQueue();
        bool trySendMessage(message_ptr message);
        void sendReset(uint16_t streamId);

        void handleUpcall() noexcept;
        int handleWrite(byte *data, size_t len, uint8_t tos, uint8_t set_df) noexcept;

        void processData(binary &&data, uint16_t streamId, PayloadId ppid);
        void processNotification(const union sctp_notification *notify, size_t len);

        const size_t mMaxMessageSize;
        const Ports mPorts;

        std::recursive_mutex mSendMutex;
        std::queue<message_ptr> mSendQueue;

        std::mutex mWriteMutex;

        std::vector<byte> mPartialMessage, mPartialNotification;
        std::vector<byte> mPartialStringData, mPartialBinaryData;

        std::atomic<size_t> mBytesSent{0}, mBytesReceived{ 0};

        static void UpcallCallback(struct socket *sock, void *arg, int flags);
        static int WriteCallback(void *sctp_ptr, void *data, size_t len, uint8_t tos, uint8_t set_df);
        static void DebugCallback(const char *format, ...);

    public:
        void OnUsrSctpReceiveSctpData(
          uint16_t streamId, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len);
        void OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len);

        void SendSctpMessage(uint32_t ppid, message_ptr message);

    private:
        void HandleDataConsumer();

        void DataProducerClosed();

        void AddOutgoingStreams(bool force);

        void ResetSctpStream(uint16_t streamId, StreamDirection direction);

        void DataConsumerClosed();

    public:
        void OnUsrSctpSendSctpData(void* buffer, size_t len);

    private:
        int agentNo;
        Listener* listener{ nullptr};
        uint16_t os{ 1024};
        uint16_t mis{ 1024};
        size_t maxSctpMessageSize{ 262144};
        bool isDataChannel{ true};
        uint8_t* messageBuffer{ nullptr};

        uint16_t desiredOs{ 0};
        size_t messageBufferLen{ 0};
        uint16_t lastSsnReceived{ 0};

    public:
        struct socket* socket{ nullptr };
    };

} // namespace rtc

#endif
