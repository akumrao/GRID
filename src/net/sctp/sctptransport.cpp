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
#include <limits>
#include <shared_mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#define USE_PMTUD 0

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

    /* Static methods for usrsctp callbacks. */

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

#define DATA_CHANNEL_CLOSED     0
#define DATA_CHANNEL_CONNECTING 1
#define DATA_CHANNEL_OPEN       2
#define DATA_CHANNEL_CLOSING    3

#define DC_TYPE_OPEN 0x03
#define DC_TYPE_ACK 0x02

#define DATA_CHANNEL_PPID_CONTROL   50
#define DATA_CHANNEL_PPID_DOMSTRING 51
#define DATA_CHANNEL_PPID_BINARY    52

#define DATA_CHANNEL_RELIABLE                0
#define DATA_CHANNEL_RELIABLE_STREAM         1
#define DATA_CHANNEL_UNRELIABLE              2
#define DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT 3
#define DATA_CHANNEL_PARTIAL_RELIABLE_TIMED  4

#define DATA_CHANNEL_FLAG_OUT_OF_ORDER_ALLOWED 0x0001

#ifndef _WIN32
#define SCTP_PACKED __attribute__((packed))
#else
#pragma pack (push, 1)
#define SCTP_PACKED
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning( push )
#pragma warning( disable : 4200 )
#endif /* defined(_WIN32) && !defined(__MINGW32__) */

struct rtcweb_datachannel_open_request {
        uint8_t msg_type; /* DATA_CHANNEL_OPEN_REQUEST */
        uint8_t channel_type;
        uint16_t flags;
        uint16_t reliability_params;
        int16_t priority;
        char label[];
    } SCTP_PACKED;
#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning( pop )
#endif /* defined(_WIN32) && !defined(__MINGW32__) */

    struct rtcweb_datachannel_open_response {
        uint8_t msg_type; /* DATA_CHANNEL_OPEN_RESPONSE */
        uint8_t error;
        uint16_t flags;
        uint16_t reverse_stream;
    } SCTP_PACKED;

    struct rtcweb_datachannel_ack {
        uint8_t msg_type; /* DATA_CHANNEL_ACK */
    } SCTP_PACKED;

#ifdef _WIN32
#pragma pack(pop)
#endif

    struct channel {
        uint8_t msg_type;
        uint8_t chan_type;
        uint16_t priority;
        uint32_t reliability;
        uint16_t label_len;
        uint16_t protocol_len;
        char *label;
        char *protocol;
    };

#undef SCTP_PACKED

    static std::map< rtc::SctpTransport*, struct channel > stpMap;

    static void print_status(rtc::SctpTransport *pc, struct channel *channel) {
        struct sctp_status status;
        socklen_t len;

        struct socket* sock = pc->socket;

        len = (socklen_t)sizeof (struct sctp_status);
        if (usrsctp_getsockopt(sock, IPPROTO_SCTP, SCTP_STATUS, &status, &len) < 0) {
            perror("getsockopt");
            return;
        }
        LInfo("Association state: ");
        switch (status.sstat_state) {
            case SCTP_CLOSED:
                LInfo("CLOSED");
                break;
            case SCTP_BOUND:
                LInfo("BOUND");
                break;
            case SCTP_LISTEN:
                LInfo("LISTEN");
                break;
            case SCTP_COOKIE_WAIT:
                LInfo("COOKIE_WAIT");
                break;
            case SCTP_COOKIE_ECHOED:
                LInfo("COOKIE_ECHOED");
                break;
            case SCTP_ESTABLISHED:
                LInfo("ESTABLISHED");
                break;
            case SCTP_SHUTDOWN_PENDING:
                LInfo("SHUTDOWN_PENDING");
                break;
            case SCTP_SHUTDOWN_SENT:
                LInfo("SHUTDOWN_SENT");
                break;
            case SCTP_SHUTDOWN_RECEIVED:
                LInfo("SHUTDOWN_RECEIVED");
                break;
            case SCTP_SHUTDOWN_ACK_SENT:
                LInfo("SHUTDOWN_ACK_SENT");
                break;
            default:
                LInfo("UNKNOWN");
                break;
        }
    }

    static void
    handle_open_request_message(rtc::SctpTransport *pc,
      uint8_t* raw_msg,
      size_t length,
      uint16_t i_stream) {
        struct channel *channel = &stpMap[pc];

        channel->chan_type = raw_msg[1];
        channel->priority = (raw_msg[2] << 8) + raw_msg[3];
        channel->reliability = (raw_msg[4] << 24) + (raw_msg[5] << 16) + (raw_msg[6] << 8) + raw_msg[7];
        channel->label_len = (raw_msg[8] << 8) + raw_msg[9];
        channel->protocol_len = (raw_msg[10] << 8) + raw_msg[11];

        std::string label(reinterpret_cast<char *> (raw_msg + 12), channel->label_len);
        std::string protocol(reinterpret_cast<char *> (raw_msg + 12 + channel->label_len), channel->protocol_len);

        SInfo << "Creating channel with stream id:" << i_stream << " channel type: " << channel->chan_type << " label:" << label << " protocol: " << protocol;

        switch (channel->chan_type) {
            case DATA_CHANNEL_RELIABLE:
                break;
            case DATA_CHANNEL_RELIABLE_STREAM:
                break;
            case DATA_CHANNEL_UNRELIABLE:
                break;
            case DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT:
                break;
            case DATA_CHANNEL_PARTIAL_RELIABLE_TIMED:
                break;
            default:
                break;
        }

        print_status(pc, channel);
    }

    static void
    handle_open_response_message(rtc::SctpTransport * /*pc*/,
      struct rtcweb_datachannel_open_response * /*rsp*/,
      size_t /*length*/, uint16_t /*i_stream*/) {
        return;
    }

    static constexpr size_t SctpMtu{ 1200};
    static constexpr uint16_t MaxSctpStreams{ 65535};

    SctpTransport::SctpTransport(Listener* listener, int agentNo, const Configuration &config, Ports ports)
    : mMaxMessageSize(config.maxMessageSize),
    mPorts(ports),
    agentNo(agentNo),
    listener(listener) {
        SInfo << "AgentNo " << agentNo << " Initializing SCTP transport";

        // Register ourselves in usrsctp.
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
    }

    SctpTransport::~SctpTransport() {
        usrsctp_set_ulpinfo(this->socket, nullptr);
        usrsctp_close(this->socket);

        // Deregister ourselves from usrsctp.
        usrsctp_deregister_address(static_cast<void*> (this));

        DepUsrSCTP::DecreaseSctpTransports();

        delete[] this->messageBuffer;
    }

    void SctpTransport::connect() {
        if (this->mState != State::Disconnected)
            return;

        try {
            int ret;
            struct sockaddr_conn rconn;

            std::memset(&rconn, 0, sizeof (rconn));
            rconn.sconn_family = AF_CONN;
            rconn.sconn_port = htons(5000);
            rconn.sconn_addr = static_cast<void*> (this);
#ifdef HAVE_SCONN_LEN
            rconn.sconn_len = sizeof (rconn);
#endif

            ret = usrsctp_connect(this->socket, reinterpret_cast<struct sockaddr*> (&rconn), sizeof (rconn));

            if (ret < 0 && errno != EINPROGRESS)
                base::uv::throwError("usrsctp_connect() failed: ", errno);

            // Disable MTU discovery.
            sctp_paddrparams peerAddrParams;

            std::memset(&peerAddrParams, 0, sizeof (peerAddrParams));
            std::memcpy(&peerAddrParams.spp_address, &rconn, sizeof (rconn));
            peerAddrParams.spp_flags = SPP_PMTUD_DISABLE;

            peerAddrParams.spp_pathmtu = SctpMtu - sizeof (struct sctp_common_header);

            ret = usrsctp_setsockopt(
              this->socket, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &peerAddrParams, sizeof (peerAddrParams));

            if (ret < 0)
                base::uv::throwError("usrsctp_setsockopt(SCTP_PEER_ADDR_PARAMS) failed: ", errno);

            changeState(State::Connecting);
            this->listener->OnSctpTransportConnecting(this);
        } catch (const std::exception& /*error*/) {
            changeState(State::Failed);
            this->listener->OnSctpTransportFailed(this);
        }
    }

    void SctpTransport::incoming(message_ptr message) {
        if (mState == State::Failed)
            return;

        if (!message) {
            SInfo << "AgentNo " << agentNo << " SCTP disconnected";
            changeState(State::Disconnected);
            return;
        }

        SDebug << "Incoming size=" << message->size();

        usrsctp_conninput(this, message->data(), message->size(), 0);
    }

    bool SctpTransport::send(message_ptr message) {
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

    void SctpTransport::closeStream(unsigned int stream) {

        if (stream < 0) {
            SError << "SCTP is not initialzed yet. You can try to supply correct value from config file";
            return;
        }

        ResetSctpStream(stream, StreamDirection::OUTGOING);
    }

    void SctpTransport::ResetSctpStream(uint16_t streamId, StreamDirection direction) {
        if (direction == StreamDirection::OUTGOING && streamId > this->os - 1)
            return;

        int ret;
        struct sctp_assoc_value av;
        socklen_t len = sizeof (av);

        ret = usrsctp_getsockopt(this->socket, IPPROTO_SCTP, SCTP_RECONFIG_SUPPORTED, &av, &len);

        if (ret == 0) {
            if (av.assoc_value != 1) {
                SDebug << "stream reconfiguration not negotiated";

                return;
            }
        } else {
            SWarn << "could not retrieve whether stream reconfiguration has been negotiated:" << " error " << std::strerror(errno);
            return;
        }

        len = sizeof (sctp_assoc_t) + (2 + 1) * sizeof (uint16_t);

        auto* srs = static_cast<struct sctp_reset_streams*> (std::malloc(len));

        switch (direction) {
            case StreamDirection::INCOMING:
                srs->srs_flags = SCTP_STREAM_RESET_INCOMING;
                break;

            case StreamDirection::OUTGOING:
                srs->srs_flags = SCTP_STREAM_RESET_OUTGOING;
                break;
        }

        srs->srs_number_streams = 1;
        srs->srs_stream_list[0] = streamId;

        ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_RESET_STREAMS, srs, len);

        if (ret == 0) {
            SDebug << "SCTP_RESET_STREAMS sent [streamId:%d]" << streamId;
        } else {
            SDebug << "usrsctp_setsockopt(SCTP_RESET_STREAMS) failed: %s " << " error " << std::strerror(errno);
        }

        std::free(srs);
    }

    void SctpTransport::AddOutgoingStreams(bool force) {
        uint16_t additionalOs{ 0};

        if (MaxSctpStreams - this->os >= 32)
            additionalOs = 32;
        else
            additionalOs = MaxSctpStreams - this->os;

        if (additionalOs == 0) {
            SDebug << "cannot add more outgoing streams [OS:%d] " << this->os;

            return;
        }

        auto nextDesiredOs = this->os + additionalOs;

        if (!force && nextDesiredOs == this->desiredOs)
            return;

        this->desiredOs = nextDesiredOs;

        if (this->mState != State::Connected) {
            SDebug << "SCTP not connected, deferring OS increase";

            return;
        }

        struct sctp_add_streams sas;

        std::memset(&sas, 0, sizeof (sas));
        sas.sas_instrms = 0;
        sas.sas_outstrms = additionalOs;

        SDebug << "adding %d outgoing streams " << additionalOs;

        int ret = usrsctp_setsockopt(
          this->socket, IPPROTO_SCTP, SCTP_ADD_STREAMS, &sas, static_cast<socklen_t> (sizeof (sas)));

        if (ret < 0)
            SDebug << "usrsctp_setsockopt(SCTP_ADD_STREAMS) failed: %s " << std::strerror(errno);
    }

    void SctpTransport::OnUsrSctpSendSctpData(void* buffer, size_t len) {
        const uint8_t* data = static_cast<uint8_t*> (buffer);

        this->listener->OnSctpTransportSendData(this, data, len);
    }

    void SctpTransport::OnUsrSctpReceiveSctpData(
      uint16_t i_stream, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len) {

        struct rtcweb_datachannel_ack *msg;
        switch (ppid) {
            case DATA_CHANNEL_PPID_CONTROL:
                if (len < sizeof (struct rtcweb_datachannel_ack)) {
                    return;
                }
                msg = (struct rtcweb_datachannel_ack *) data;
                switch (msg->msg_type) {
                    case DC_TYPE_OPEN:
                        if (len < sizeof (struct rtcweb_datachannel_open_request)) {
                            return;
                        }
                        SInfo << " channel open request streamid " << i_stream << " this " << this;

                        handle_open_request_message(this, (uint8_t*) data, len, i_stream);
                        break;

                    case DC_TYPE_ACK:
                        if (len < sizeof (struct rtcweb_datachannel_ack)) {
                            return;
                        }

                        SInfo << " channel ack streamid " << i_stream;
                        break;
                    default:
                        SError << "Unknown state ";
                        break;
                }
                break;
            case DATA_CHANNEL_PPID_DOMSTRING:
            case DATA_CHANNEL_PPID_BINARY:
                break;
            default:
                break;
        };

        if (ppid == 50) {
            SDebug << "ignoring SCTP data with ppid:50 (WebRTC DataChannel Control)";

            return;
        }
        SInfo << "streamId: " << i_stream << " this " << this << " ssn: " << ssn << " ppid: " << ppid << " data: " << data;
        if (this->messageBufferLen != 0 && ssn != this->lastSsnReceived) {
            SDebug << "message chunk received with different SSN while buffer not empty, buffer discarded [ssn:%" PRIu16 "  <<   last ssn" << ssn << "  last  received " << this->lastSsnReceived;

            this->messageBufferLen = 0;
        }

        this->lastSsnReceived = ssn;

        auto eor = static_cast<bool> (flags & MSG_EOR);

        if (this->messageBufferLen + len > this->maxSctpMessageSize) {
            SWarn << "ongoing received message exceeds max allowed message size [message size:%zu, :%zu, eor:%u] " << this->messageBufferLen + len << " max message size " << this->maxSctpMessageSize << " eor " << (eor ? 1 : 0);

            this->lastSsnReceived = 0;

            return;
        }

        if (eor && this->messageBufferLen == 0) {
            SDebug << "directly notifying listener [eor:1, buffer len:0]";
        } else if (eor && this->messageBufferLen != 0) {
            std::memcpy(this->messageBuffer + this->messageBufferLen, data, len);
            this->messageBufferLen += len;

            SDebug << "notifying listener [eor:1, buffer len:%zu] " << this->messageBufferLen;

            this->messageBufferLen = 0;
        } else if (!eor) {
            if (!this->messageBuffer)
                this->messageBuffer = new uint8_t[this->maxSctpMessageSize];

            std::memcpy(this->messageBuffer + this->messageBufferLen, data, len);
            this->messageBufferLen += len;

            SDebug << "data buffered [eor:0, buffer len:%zu] " << this->messageBufferLen;
        }
    }

    void SctpTransport::OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len) {
        if (notification->sn_header.sn_length != (uint32_t) len)
            return;

        switch (notification->sn_header.sn_type) {
            case SCTP_ADAPTATION_INDICATION:
            {
                SInfo << "SCTP adaptation indication " << notification->sn_adaptation_event.sai_adaptation_ind;
                break;
            }

            case SCTP_ASSOC_CHANGE:
            {
                switch (notification->sn_assoc_change.sac_state) {
                    case SCTP_COMM_UP:
                    {
                        SInfo << "SCTP association connected, streams  out:" << notification->sn_assoc_change.sac_outbound_streams << " in:" << notification->sn_assoc_change.sac_inbound_streams;

                        this->os = notification->sn_assoc_change.sac_outbound_streams;

                        if (this->desiredOs > this->os)
                            AddOutgoingStreams(/*force*/ true);

                        if (this->mState != State::Connected) {
                            SInfo << "OnSctpAssociationConnected ";
                            changeState(State::Connected);
                            this->listener->OnSctpTransportConnected(this);
                        }

                        break;
                    }

                    case SCTP_COMM_LOST:
                    {
                        if (notification->sn_header.sn_length > 0) {
                            static const size_t BufferSize{ 1024};
                            static char buffer[BufferSize];

                            uint32_t headerLen = notification->sn_header.sn_length;

                            for (uint32_t i{0}; i < headerLen; ++i) {
                                std::snprintf(
                                  buffer, BufferSize, " 0x%02x", notification->sn_assoc_change.sac_info[i]);
                            }

                            SDebug << "SCTP communication lost [info:%s] " << buffer;
                        } else {
                            SDebug << "SCTP communication lost";
                        }

                        if (this->mState != State::Disconnected) {
                            changeState(State::Disconnected);
                            this->listener->OnSctpTransportClosed(this);
                        }

                        break;
                    }

                    case SCTP_RESTART:
                    {
                        SDebug << "SCTP remote association restarted, streams out: " << notification->sn_assoc_change.sac_outbound_streams << " in " << notification->sn_assoc_change.sac_inbound_streams;

                        this->os = notification->sn_assoc_change.sac_outbound_streams;

                        if (this->desiredOs > this->os)
                            AddOutgoingStreams(/*force*/ true);

                        if (this->mState != State::Connected) {
                            this->mState = State::Connected;
                            this->listener->OnSctpTransportConnected(this);
                        }

                        break;
                    }

                    case SCTP_SHUTDOWN_COMP:
                    {
                        SDebug << "SCTP association gracefully closed";

                        if (this->mState != State::Disconnected) {
                            this->mState = State::Disconnected;
                            this->listener->OnSctpTransportClosed(this);
                        }

                        break;
                    }

                    case SCTP_CANT_STR_ASSOC:
                    {
                        if (notification->sn_header.sn_length > 0) {
                            static const size_t BufferSize{ 1024};
                            static char buffer[BufferSize];

                            uint32_t headerLen = notification->sn_header.sn_length;

                            for (uint32_t i{0}; i < headerLen; ++i) {
                                std::snprintf(
                                  buffer, BufferSize, " 0x%02x", notification->sn_assoc_change.sac_info[i]);
                            }

                            SDebug << "SCTP setup failed: " << buffer;
                        }

                        if (this->mState != State::Failed) {
                            changeState(State::Failed);
                            this->listener->OnSctpTransportFailed(this);
                        }

                        break;
                    }

                    default:;
                }

                break;
            }

            case SCTP_ASSOC_RESET_EVENT:
            {
                SDebug << "SCTP association reset event received";
                break;
            }

            case SCTP_REMOTE_ERROR:
            {
                static const size_t BufferSize{ 1024};
                static char buffer[BufferSize];

                uint32_t errLen = notification->sn_remote_error.sre_length - sizeof (struct sctp_remote_error);

                for (uint32_t i{0}; i < errLen; i++) {
                    std::snprintf(buffer, BufferSize, "0x%02x", notification->sn_remote_error.sre_data[i]);
                }

                SWarn << " remote SCTP association error type: " << " type " << notification->sn_remote_error.sre_error << " data " << buffer;

                break;
            }

            case SCTP_SHUTDOWN_EVENT:
            {
                SDebug << "remote SCTP association shutdown";

                if (this->mState != State::Disconnected) {
                    changeState(State::Disconnected);
                    this->listener->OnSctpTransportClosed(this);
                }

                break;
            }

            case SCTP_SEND_FAILED_EVENT:
            {
                static const size_t BufferSize{ 1024};
                static char buffer[BufferSize];

                uint32_t failLen =
                  notification->sn_send_failed_event.ssfe_length - sizeof (struct sctp_send_failed_event);

                for (uint32_t i{0}; i < failLen; ++i) {
                    std::snprintf(buffer, BufferSize, "0x%02x", notification->sn_send_failed_event.ssfe_data[i]);
                }

                SWarn << " SCTP message sent failure [streamId: ]" << notification->sn_send_failed_event.ssfe_info.snd_sid << " ppid " << ntohl(notification->sn_send_failed_event.ssfe_info.snd_ppid) <<
                  "sent " << ((notification->sn_send_failed_event.ssfe_flags & SCTP_DATA_SENT) ? "yes" : "no") << " err " << notification->sn_send_failed_event.ssfe_error <<
                  " info " << buffer;

                break;
            }

            case SCTP_STREAM_RESET_EVENT:
            {
                bool incoming{ false};
                bool outgoing{ false};
                uint16_t numStreams =
                  (notification->sn_strreset_event.strreset_length - sizeof (struct sctp_stream_reset_event)) /
                  sizeof (uint16_t);

                if (notification->sn_strreset_event.strreset_flags & SCTP_STREAM_RESET_INCOMING_SSN)
                    incoming = true;

                if (notification->sn_strreset_event.strreset_flags & SCTP_STREAM_RESET_OUTGOING_SSN)
                    outgoing = true;

                if (incoming && !outgoing && this->isDataChannel) {
                    for (uint16_t i{0}; i < numStreams; ++i) {
                        auto streamId = notification->sn_strreset_event.strreset_stream_list[i];

                        ResetSctpStream(streamId, StreamDirection::OUTGOING);
                    }
                }

                break;
            }

            case SCTP_STREAM_CHANGE_EVENT:
            {
                if (notification->sn_strchange_event.strchange_flags == 0) {
                    SDebug << "[sctp] SCTP stream changed, streams [out:" << notification->sn_strchange_event.strchange_outstrms << ", in:" << notification->sn_strchange_event.strchange_instrms << ", flags:" << notification->sn_strchange_event.strchange_flags << "]";
                } else if (notification->sn_strchange_event.strchange_flags & SCTP_STREAM_RESET_DENIED) {
                    SDebug << "[sctp] SCTP stream change denied, streams [out:" << notification->sn_strchange_event.strchange_outstrms << ", in:" << notification->sn_strchange_event.strchange_instrms << ", flags:" << notification->sn_strchange_event.strchange_flags << "]";
                    break;
                } else if (notification->sn_strchange_event.strchange_flags & SCTP_STREAM_RESET_FAILED) {
                    SDebug << "[sctp] SCTP stream change failed, streams [out:" << notification->sn_strchange_event.strchange_outstrms << ", in:" << notification->sn_strchange_event.strchange_instrms << ", flags:" << notification->sn_strchange_event.strchange_flags << "]";
                    break;
                }

                this->os = notification->sn_strchange_event.strchange_outstrms;

                break;
            }

            default:
            {
                SWarn << "[sctp] unhandled SCTP event received [type:" << notification->sn_header.sn_type << "]";
            }
        }
    }

    unsigned int SctpTransport::maxStream() const {
        unsigned int streamsCount = std::min(os, mis);
        ;
        return streamsCount > 0 ? streamsCount - 1 : 0;
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

    void SctpTransport::start() {
        SInfo << "AgentNo " << agentNo << " start";
        connect();
    }

    void SctpTransport::stop() {
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
                STrace << "SCTP association change event";
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
                        changeState(State::Disconnected);
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
                STrace << "SCTP sender dry event";
                break;
            }

            case SCTP_STREAM_RESET_EVENT:
            {
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

} // namespace rtc
