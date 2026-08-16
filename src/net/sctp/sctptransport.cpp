#include "sctptransport.hpp"
#include "DepUsrSCTP.h"

#include "base/logger.h"
#include "base/error.h"
#include "UtilStun.h"

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <exception>
#include <iostream>

//#define USE_PMTUD 0

static constexpr size_t SctpMtu{ 1200};

using namespace std::chrono_literals;
using namespace std::chrono;

using namespace base;

namespace rtc {

    uint16_t EventTypes[] ={
        SCTP_ADAPTATION_INDICATION,
        SCTP_ASSOC_CHANGE,
        SCTP_ASSOC_RESET_EVENT,
        SCTP_REMOTE_ERROR,
        SCTP_SHUTDOWN_EVENT,
        SCTP_SEND_FAILED_EVENT,
        SCTP_STREAM_RESET_EVENT,
        SCTP_STREAM_CHANGE_EVENT
    };

    inline static int onRecvSctpData(
      struct socket* sock,
      union sctp_sockstore addr,
      void* data,
      size_t len,
      struct sctp_rcvinfo rcv,
      int flags,
      void* ulpInfo) {
        auto* sctpAssociation = static_cast<rtc::SctpTransport*> (ulpInfo);

        if (sctpAssociation == nullptr) {
            std::free(data);
            return 0;
        }

        sctpAssociation->onRecvSctpData_obj(sock, addr, data, len, rcv, flags, ulpInfo);
        std::free(data);
        return 1;
    }

    SctpTransport::SctpTransport(Listener* listener, int agentNo, const Configuration &config, Ports ports)
    : mMaxMessageSize(config.maxMessageSize),
    mPorts(ports),
    agentNo(agentNo),
    listener(listener) {
        SInfo << "AgentNo " << agentNo << " Initializing SCTP transport";

        usrsctp_register_address(static_cast<void*> (this));

        int ret;

        this->socket = usrsctp_socket(
          AF_CONN, SOCK_STREAM, IPPROTO_SCTP, onRecvSctpData, nullptr, 0, static_cast<void*> (this));

        if (this->socket == nullptr)
            base::uv::throwError("usrsctp_socket() failed: ", errno);

        usrsctp_set_ulpinfo(this->socket, static_cast<void*> (this));

        // Make the socket non-blocking.
        ret = usrsctp_set_non_blocking(this->socket, 1);

        if (ret < 0)
            base::uv::throwError("usrsctp_set_non_blocking() failed: ", errno);

        struct linger lingerOpt;

        lingerOpt.l_onoff = 1;
        lingerOpt.l_linger = 0;

        ret = usrsctp_setsockopt(this->socket, SOL_SOCKET, SO_LINGER, &lingerOpt, sizeof (lingerOpt));

        if (ret < 0)
            base::uv::throwError("usrsctp_setsockopt(SO_LINGER) failed: ", errno);

        // Set SCTP_ENABLE_STREAM_RESET.
        struct sctp_assoc_value av;

        av.assoc_value =
          SCTP_ENABLE_RESET_STREAM_REQ | SCTP_ENABLE_RESET_ASSOC_REQ | SCTP_ENABLE_CHANGE_ASSOC_REQ;

        ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_ENABLE_STREAM_RESET, &av, sizeof (av));

        if (ret < 0) {
            base::uv::throwError("usrsctp_setsockopt(SCTP_ENABLE_STREAM_RESET) failed: ", errno);
        }

        // Set SCTP_NODELAY.
        uint32_t noDelay = 1;

        ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_NODELAY, &noDelay, sizeof (noDelay));

        if (ret < 0)
            base::uv::throwError("usrsctp_setsockopt(SCTP_NODELAY) failed: ", errno);

        // Enable events.
        struct sctp_event event;

        std::memset(&event, 0, sizeof (event));
        event.se_on = 1;

        for (size_t i{0}; i < sizeof (EventTypes) / sizeof (uint16_t); ++i) {
            event.se_type = EventTypes[i];

            ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof (event));

            if (ret < 0)
                base::uv::throwError("usrsctp_setsockopt(SCTP_EVENT) failed: ", errno);
        }

        // Init message.
        struct sctp_initmsg initmsg;

        std::memset(&initmsg, 0, sizeof (initmsg));
        initmsg.sinit_num_ostreams = this->os;
        initmsg.sinit_max_instreams = this->mis;

        ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof (initmsg));

        if (ret < 0)
            base::uv::throwError("usrsctp_setsockopt(SCTP_INITMSG) failed: ", errno);

        // Server side.
        struct sockaddr_conn sconn;

        std::memset(&sconn, 0, sizeof (sconn));
        sconn.sconn_family = AF_CONN;
        sconn.sconn_port = htons(5000);
        sconn.sconn_addr = static_cast<void*> (this);
#ifdef HAVE_SCONN_LEN
        rconn.sconn_len = sizeof (sconn);
#endif

        ret = usrsctp_bind(this->socket, reinterpret_cast<struct sockaddr*> (&sconn), sizeof (sconn));

        if (ret < 0)
            base::uv::throwError("usrsctp_bind() failed:", errno);

        SInfo << "SctpTransport with socket " << this->socket << " this " << this;

        DepUsrSCTP::IncreaseSctpTransports();

         mWorkerThread = std::thread(&SctpTransport::workerLoop, this);
    }

    SctpTransport::~SctpTransport() {
        SInfo << "~SctpTransport() begin";

        // Stop accepting new tasks and wake worker thread
        {
            std::lock_guard<std::mutex> lock(mQueueMutex);
            mRunning = false;
           // std::queue<Task> empty;
           // std::swap(mTaskQueue, empty);
        }
        mQueueCv.notify_all();

//        // Close socket to unblock any pending usrsctp calls in worker thread
//        if (this->socket) {
//            usrsctp_shutdown(this->socket, SHUT_RDWR);
//            usrsctp_close(this->socket);
//            this->socket = nullptr;
//        }

        if (mWorkerThread.joinable()) {
            mWorkerThread.join();
        }
        

        usrsctp_deregister_address(static_cast<void*>(this));
        DepUsrSCTP::DecreaseSctpTransports();
        
        
        SInfo << "~SctpTransport() over";
    }



    void SctpTransport::start() {
        enqueueTask([this]() { doConnect(); });
    }

    void SctpTransport::stop() {
        shutdown();
    }

    void SctpTransport::shutdown() {
        
        SInfo << "shutdown " ;
        enqueueTask([this]() { doShutdown(); });
    }

    void SctpTransport::closeStream(unsigned int stream) {
        
         SInfo << "closeStream closeStream " << stream;
        
        enqueueTask([this, stream]() {
          //  doResetStream(uint16_t(stream), StreamDirection::OUTGOING);
  
            doSend(make_message(0, Message::Reset, stream));
        });
    }

    bool SctpTransport::send(message_ptr message) {
        enqueueTask([this, message]() {
            doSend(message);
        });
        return true;
    }

    void SctpTransport::incoming(message_ptr message) {
        if (mState == State::Failed)
            return;

        if (!message) {
            SInfo << "AgentNo " << agentNo << " SCTP disconnected stream "  << message->stream;
            changeState(State::Disconnected);
            return;
        }

       // SDebug << "Incoming size=" << message->size();

        usrsctp_conninput(this, message->data(), message->size(), 0);
    }


    void SctpTransport::doConnect() {
        if (mState != State::Disconnected || !this->socket) return;

        struct sockaddr_conn rconn;
        std::memset(&rconn, 0, sizeof (rconn));
        rconn.sconn_family = AF_CONN;
        rconn.sconn_port = htons(5000);
        rconn.sconn_addr = static_cast<void*> (this);

        int ret = usrsctp_connect(this->socket, reinterpret_cast<struct sockaddr*> (&rconn), sizeof (rconn));

        if (ret < 0 && errno != EINPROGRESS) {
            changeState(State::Failed);
            listener->OnSctpTransportFailed(this);
            return;
        }

        sctp_paddrparams peerAddrParams;
        std::memset(&peerAddrParams, 0, sizeof (peerAddrParams));
        std::memcpy(&peerAddrParams.spp_address, &rconn, sizeof (rconn));
        peerAddrParams.spp_flags = SPP_PMTUD_DISABLE;
        peerAddrParams.spp_pathmtu = 1200 - sizeof (struct sctp_common_header);

        usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &peerAddrParams, sizeof (peerAddrParams));

        changeState(State::Connecting);
        listener->OnSctpTransportConnecting(this);
    }


    void SctpTransport::OnUsrSctpSendSctpData(void* buffer, size_t len) {
        const uint8_t* data = static_cast<uint8_t*> (buffer);

        this->listener->OnSctpTransportSendData(this, data, len);
    }


    bool SctpTransport::doSend(message_ptr message) {
        if (mState != State::Connected)
            return false;

        uint32_t ppid;
        switch (message->type) {
            case Message::String:
                ppid = !message->empty() ? PPID_STRING : PPID_STRING_EMPTY;
                break;
            case Message::Binary:
                ppid = !message->empty() ? PPID_BINARY : PPID_BINARY_EMPTY;
                break;
            case Message::Control:
                ppid = PPID_CONTROL;
                break;
            case Message::Reset:
                sendReset(uint16_t(message->stream));
                return true;
            default:
                return true;
        }

        STrace << "SCTP try send size=" << message->size();

        const Reliability reliability = message->reliability ? *message->reliability : Reliability();

        struct sctp_sendv_spa spa = {};

        // set sndinfo
        spa.sendv_flags |= SCTP_SEND_SNDINFO_VALID;
        spa.sendv_sndinfo.snd_sid = uint16_t(message->stream);
        spa.sendv_sndinfo.snd_ppid = htonl(ppid);
        spa.sendv_sndinfo.snd_flags |= SCTP_EOR; // implicit here

        // set prinfo
        spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
        if (reliability.unordered)
            spa.sendv_sndinfo.snd_flags |= SCTP_UNORDERED;

        if (reliability.maxPacketLifeTime.count()) {
            spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
            spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_TTL;
            spa.sendv_prinfo.pr_value = reliability.maxPacketLifeTime.count();
        } else if (reliability.maxRetransmits) {
            spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
            spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_RTX;
            spa.sendv_prinfo.pr_value = reliability.maxRetransmits;
        }            // else {
            // 	spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_NONE;
            // }
            // Deprecated
        else switch (reliability.typeDeprecated) {
                    //	case Reliability::Type::Rexmit:
                    //		spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
                    //		spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_RTX;
                    //		spa.sendv_prinfo.pr_value = to_uint32(std::get<int>(reliability.rexmit));
                    //		break;
                    //	case Reliability::Type::Timed:
                    //		spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
                    //		spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_TTL;
                    //		spa.sendv_prinfo.pr_value = to_uint32(std::get<milliseconds>(reliability.rexmit).count());
                    //		break;
                default:
                    spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_NONE;
                    break;

            }

        ssize_t ret;
        if (!message->empty()) {
            ret = usrsctp_sendv(this->socket, message->data(), message->size(), nullptr, 0, &spa, sizeof (spa),
              SCTP_SENDV_SPA, 0);
        } else {
            const char zero = 0;
            ret = usrsctp_sendv(this->socket, &zero, 1, nullptr, 0, &spa, sizeof (spa), SCTP_SENDV_SPA, 0);
        }

        if (ret < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                STrace << "SCTP sending not possible";
                return false;
            }

            SError << "SCTP sending failed, errno=" << errno;
            throw std::runtime_error("Sending failed, errno=" + std::to_string(errno));
        }

        STrace << "SCTP sent size=" << message->size();
        if (message->type == Message::Binary || message->type == Message::String)
            mBytesSent += message->size();
        return true;
    }




    void SctpTransport::doShutdown() {
        if (!this->socket) return;
        
        SInfo << "doShutdown";
        // Perform clean usrsctp teardown
        usrsctp_shutdown(this->socket, SHUT_RDWR);
        usrsctp_set_ulpinfo(this->socket, nullptr);
        usrsctp_close(this->socket);


        changeState(State::Disconnected);
        
        
        SInfo << "doShutdown over";
    }

    void SctpTransport::recv(message_ptr message) {
        try {
            listener->OnSctpTransportMessageReceived(this, message);
        } catch (const std::exception &e) {
            SWarn << e.what();
        }
    }

    void SctpTransport::processData(binary &&data, uint16_t sid, PayloadId ppid) {
        STrace << "Process data, size=" << data.size();

        switch (ppid) {
            case PPID_CONTROL:
                recv(make_message(std::move(data), Message::Control, sid));
                break;

            case PPID_STRING_PARTIAL:
                mPartialStringData.insert(mPartialStringData.end(), data.begin(), data.end());
                mPartialStringData.resize(mMaxMessageSize);
                break;

            case PPID_STRING:
                if (mPartialStringData.empty()) {
                    mBytesReceived += data.size();
                    recv(make_message(std::move(data), Message::String, sid));
                } else {
                    mPartialStringData.insert(mPartialStringData.end(), data.begin(), data.end());
                    mPartialStringData.resize(mMaxMessageSize);
                    mBytesReceived += mPartialStringData.size();
                    auto message = make_message(std::move(mPartialStringData), Message::String, sid);
                    mPartialStringData.clear();
                    recv(std::move(message));
                }
                break;

            case PPID_STRING_EMPTY:
                recv(make_message(std::move(mPartialStringData), Message::String, sid));
                mPartialStringData.clear();
                break;

            case PPID_BINARY_PARTIAL:
                mPartialBinaryData.insert(mPartialBinaryData.end(), data.begin(), data.end());
                mPartialBinaryData.resize(mMaxMessageSize);
                break;

            case PPID_BINARY:
                if (mPartialBinaryData.empty()) {
                    mBytesReceived += data.size();
                    recv(make_message(std::move(data), Message::Binary, sid));
                } else {
                    mPartialBinaryData.insert(mPartialBinaryData.end(), data.begin(), data.end());
                    mPartialBinaryData.resize(mMaxMessageSize);
                    mBytesReceived += mPartialBinaryData.size();
                    auto message = make_message(std::move(mPartialBinaryData), Message::Binary, sid);
                    mPartialBinaryData.clear();
                    recv(std::move(message));
                }
                break;

            case PPID_BINARY_EMPTY:
                recv(make_message(std::move(mPartialBinaryData), Message::Binary, sid));
                mPartialBinaryData.clear();
                break;

            default:
                SWarn << "Unknown PPID: " << uint32_t(ppid);
                return;
        }
    }

    int SctpTransport::onRecvSctpData_obj(struct socket* /*sock*/, union sctp_sockstore & /*addr*/, void* data, size_t len, struct sctp_rcvinfo &rcv, int flags, void* /*ulpInfo*/) {
        if (flags & MSG_NOTIFICATION) {
            const byte* bytes = static_cast<const byte*> (data);
            mPartialNotification.insert(mPartialNotification.end(), bytes, bytes + len);

            if (flags & MSG_EOR) {
                binary notification;
                mPartialNotification.swap(notification);
                auto n = reinterpret_cast<union sctp_notification *> (notification.data());
                processNotification(n, notification.size());
            }
        } else {
            SDebug << "data chunk received [length:" << len << ", streamId:" << rcv.rcv_sid << ", SSN:" << rcv.rcv_ssn << ", TSN:" << rcv.rcv_tsn << ", PPID:" << ntohl(rcv.rcv_ppid) << ", context:" << rcv.rcv_context << ", flags:" << flags << "]";

            const byte* bytes = static_cast<const byte*> (data);
            mPartialMessage.insert(mPartialMessage.end(), bytes, bytes + len);
            if (mPartialMessage.size() > mMaxMessageSize) {
                SWarn << "SCTP message is too large, truncating it";
                mPartialMessage.resize(mMaxMessageSize);
            }

            if (flags & MSG_EOR) {
                binary message;
                mPartialMessage.swap(message);

                processData(std::move(message), rcv.rcv_sid, PayloadId(ntohl(rcv.rcv_ppid)));
            }
        }

        return 0;
    }

    void SctpTransport::processNotification(const union sctp_notification *notify, size_t len) {
        if (len != size_t(notify->sn_header.sn_length)) {
            SWarn << "Unexpected notification length, expected=" << notify->sn_header.sn_length
              << ", actual=" << len;
            return;
        }

        auto type = notify->sn_header.sn_type;
        STrace << "Processing notification, type=" << type;

        switch (type) {
            case SCTP_ASSOC_CHANGE:
            {
                SInfo << "SCTP association change event";
                const struct sctp_assoc_change &sac = notify->sn_assoc_change;
                if (sac.sac_state == SCTP_COMM_UP) {
                    SDebug << "SCTP negotiated streams: incoming=" << sac.sac_inbound_streams
                      << ", outgoing=" << sac.sac_outbound_streams;
                    mNegotiatedStreamsCount = std::min(sac.sac_inbound_streams, sac.sac_outbound_streams);

                    SInfo << "AgentNo " << agentNo << " SCTP connected";
                    changeState(State::Connected);

                    listener-> OnSctpTransportConnected(this);

                } else {
                    if (state() == State::Connected) {
                        SInfo << "AgentNo " << agentNo << " SCTP disconnected";
                        recv(nullptr);
                        listener->OnSctpTransportClosed(this);
                     //   changeState(State::Disconnected);
                    } else {
                        SError << "AgentNo " << agentNo << " SCTP connection failed";
                        changeState(State::Failed);
                        listener->OnSctpTransportFailed(this);
                    }
                }
                break;
            }

            case SCTP_SENDER_DRY_EVENT:
            {
                SInfo << "SCTP sender dry event";
                break;
            }

            case SCTP_STREAM_RESET_EVENT:
            {
                 SInfo << "SCTP sSCTP_STREAM_RESET_EVENT";
                 
                const struct sctp_stream_reset_event &reset_event = notify->sn_strreset_event;
                const int count = (reset_event.strreset_length - sizeof (reset_event)) / sizeof (uint16_t);
                const uint16_t flags = reset_event.strreset_flags;

#if DEBUGSTCP
                {
                    std::ostringstream desc;
                    desc << "flags=";
                    if (flags & SCTP_STREAM_RESET_OUTGOING_SSN && flags & SCTP_STREAM_RESET_INCOMING_SSN)
                        desc << "outgoing|incoming";
                    else if (flags & SCTP_STREAM_RESET_OUTGOING_SSN)
                        desc << "outgoing";
                    else if (flags & SCTP_STREAM_RESET_INCOMING_SSN)
                        desc << "incoming";
                    else
                        desc << "0";

                    desc << ", streams=[";
                    for (int i = 0; i < count; ++i) {
                        uint16_t streamId = reset_event.strreset_stream_list[i];
                        desc << (i != 0 ? "," : "") << streamId;
                    }
                    desc << "]";

                    STrace << "SCTP reset event, " << desc.str();
                }
#endif                

                // RFC 8831 6.7. Closing a Data Channel
                // If one side decides to close the data channel, it resets the corresponding outgoing
                // stream. When the peer sees that an incoming stream was reset, it also resets its
                // corresponding outgoing stream.
                // See https://www.rfc-editor.org/rfc/rfc8831.html#section-6.7
                if (flags & SCTP_STREAM_RESET_INCOMING_SSN) {
                    for (int i = 0; i < count; ++i) {
                        uint16_t streamId = reset_event.strreset_stream_list[i];
                        recv(make_message(0, Message::Reset, streamId));
                    }
                }
                break;
            }
            
            
            
             case SCTP_REMOTE_ERROR:
            {
                static const size_t BufferSize{ 1024};
                static char buffer[BufferSize];

                uint32_t errLen = notify->sn_remote_error.sre_length - sizeof (struct sctp_remote_error);

                for (uint32_t i{0}; i < errLen; i++) {
                    std::snprintf(buffer, BufferSize, "0x%02x", notify->sn_remote_error.sre_data[i]);
                }

                SWarn << " remote SCTP association error type: " << " type " << notify->sn_remote_error.sre_error << " data " << buffer;

                break;
            }

            case SCTP_SHUTDOWN_EVENT:
            {
                //SInfo << "remote SCTP association shutdown";

                if (this->mState != State::Disconnected) {
                  //  this->listener->OnSctpTransportClosed(this);
                   // changeState(State::Disconnected);
                }

                break;
            }

            case SCTP_SEND_FAILED_EVENT:
            {
                SInfo << "remote SCTP association Failed";
                  
                static const size_t BufferSize{ 1024};
                static char buffer[BufferSize];

                uint32_t failLen =
                  notify->sn_send_failed_event.ssfe_length - sizeof (struct sctp_send_failed_event);

                for (uint32_t i{0}; i < failLen; ++i) {
                    std::snprintf(buffer, BufferSize, "0x%02x", notify->sn_send_failed_event.ssfe_data[i]);
                }

                SWarn << " SCTP message sent failure [streamId: ]" << notify->sn_send_failed_event.ssfe_info.snd_sid << " ppid " << ntohl(notify->sn_send_failed_event.ssfe_info.snd_ppid) <<
                  "sent " << ((notify->sn_send_failed_event.ssfe_flags & SCTP_DATA_SENT) ? "yes" : "no") << " err " << notify->sn_send_failed_event.ssfe_error <<
                  " info " << buffer;

                break;
            }

            default:
                break;
        }
    }

    SctpTransport::State SctpTransport::state() const {
        return mState;
    }

    void SctpTransport::changeState(State state) {
        try {
            if (mState.exchange(state) != state)
                mState = state;
        } catch (const std::exception &e) {
            SWarn << e.what();
        }

        switch (state) {
            case State::Disconnected:
            {
                SInfo << "AgentNo " << agentNo << "SctpTransport disconnected";
                break;
            }
            case State::Connecting:
            {
                SInfo << "AgentNo " << agentNo << "SctpTransport Connecting";
                break;
            }
            case State::Connected:
            {
                SInfo << "AgentNo " << agentNo << "SctpTransport Connected";
                break;
            }
            case State::Completed:
            {
                SInfo << "AgentNo " << agentNo << "SctpTransport Completed";
                break;
            }
            case State::Failed:
            {
                SInfo << "AgentNo " << agentNo << "SctpTransport Failed";
                break;
            }
        };

        listener->OnSctpState(state);
    }

    unsigned int SctpTransport::maxStream() const {
        unsigned int streamsCount = std::min(os, mis);
        ;
        return streamsCount > 0 ? streamsCount - 1 : 0;
    }

    void SctpTransport::enqueueTask(Task task) {
        if (!mRunning) return;
        {
            std::lock_guard<std::mutex> lock(mQueueMutex);
            mTaskQueue.push(std::move(task));
        }
        mQueueCv.notify_one();
    }

    void SctpTransport::workerLoop() {
        while (mRunning || !mTaskQueue.empty()) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mQueueMutex);
                mQueueCv.wait(lock, [this]() {
                    return !mTaskQueue.empty() || !mRunning;
                });

                if (mTaskQueue.empty()) continue;

                task = std::move(mTaskQueue.front());
                mTaskQueue.pop();
            }

            if (task) {
                task();
            }
        }
        SInfo << "workerLoop() exit";
    }

    void SctpTransport::sendReset(uint16_t streamId) {
        SInfo << "AgentNo " << " sendReset start streamId " << streamId;

        if (mState != State::Connected)
            return;

        SDebug << "SCTP resetting stream " << streamId;

        using srs_t = struct sctp_reset_streams;
        const size_t len = sizeof (srs_t) + sizeof (uint16_t);
        byte buffer[len] = {};
        srs_t &srs = *reinterpret_cast<srs_t *> (buffer);
        srs.srs_flags = SCTP_STREAM_RESET_OUTGOING;
        srs.srs_number_streams = 1;
        srs.srs_stream_list[0] = streamId;

        if (usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_RESET_STREAMS, &srs, len) == 0) {
        } else if (errno == EINVAL) {
            SDebug << "SCTP stream " << streamId << " already reset";
        } else {
            SWarn << "SCTP reset stream " << streamId << " failed, errno=" << errno;
        }
    }

//    void SctpTransport::start() {
//        SInfo << "AgentNo " << agentNo << " start";
//        doConnect();
//    }









// void SctpTransport::ResetSctpStream(uint16_t streamId, StreamDirection direction) {
//        if (direction == StreamDirection::OUTGOING && streamId > this->os - 1)
//            return;
//
//        int ret;
//        struct sctp_assoc_value av;
//        socklen_t len = sizeof (av);
//
//        ret = usrsctp_getsockopt(this->socket, IPPROTO_SCTP, SCTP_RECONFIG_SUPPORTED, &av, &len);
//
//        if (ret == 0) {
//            if (av.assoc_value != 1) {
//                SDebug << "stream reconfiguration not negotiated";
//
//                return;
//            }
//        } else {
//            SWarn << "could not retrieve whether stream reconfiguration has been negotiated:" << " error " << std::strerror(errno);
//            return;
//        }
//
//        len = sizeof (sctp_assoc_t) + (2 + 1) * sizeof (uint16_t);
//
//        auto* srs = static_cast<struct sctp_reset_streams*> (std::malloc(len));
//
//        switch (direction) {
//            case StreamDirection::INCOMING:
//                srs->srs_flags = SCTP_STREAM_RESET_INCOMING;
//                break;
//
//            case StreamDirection::OUTGOING:
//                srs->srs_flags = SCTP_STREAM_RESET_OUTGOING;
//                break;
//        }
//
//        srs->srs_number_streams = 1;
//        srs->srs_stream_list[0] = streamId;
//
//        ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_RESET_STREAMS, srs, len);
//
//        if (ret == 0) {
//            SDebug << "SCTP_RESET_STREAMS sent [streamId:%d]" << streamId;
//        } else {
//            SDebug << "usrsctp_setsockopt(SCTP_RESET_STREAMS) failed: %s " << " error " << std::strerror(errno);
//        }
//
//        std::free(srs);
//    }










} // namespace rtc
