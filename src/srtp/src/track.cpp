
#include "base/logger.h"
#include "track.hpp"

using namespace base;
//#include "impl/internals.hpp"
#include "peerconnection.h"

/*
Track::send()
  ↳ H264RtpPacketizer::outgoing() 
  ↳ RtpTransport::sendMedia()
  ↳ DtlsSrtpTransport::encrypt()  
  ↳ SrtpSession::protect()
  ↳ UdpSocket::send()

 
 UdpSocket::recv()
  ↳ DtlsSrtpTransport::decrypt()  
  ↳ RtpTransport::recvMedia()
  ↳ RtcpNackResponder::incoming() 

 
RtcpSrReporter::incoming() or RtcpReceivingSession::incoming()
  ↳ MediaHandler::sendOutcome()
  ↳ RtpTransport::sendMedia()
  ↳ SrtpTransport::encrypt()                     // Evaluates buffer headers to detect RTCP
      ↳ srtp_protect_rtcp(mTransmitSession, ...) // Third-party libsrtp C-function call
  ↳ IceTransport::send()                         // Routes to libjuice / transport layer
  ↳ UdpSocket::send()                            // Physical network transmission

  
 
 
UdpSocket::recv()                                // Platform network read event
  ↳ IceTransport::incoming()                     // Dispatched from libjuice
  ↳ SrtpTransport::decrypt()                     // Explicitly looks at multiplexed packet boundaries
      ↳ srtp_unprotect_rtcp(mReceiveSession, ...) // Third-party libsrtp C-function call
  ↳ RtpTransport::recvMedia()
  ↳ RtpTransport::recvRtcp()                    // Parses binary compounds into rtc::RtcpPacket structures
  ↳ MediaHandler::incoming()
      ↳ RtcpNackResponder::incoming()


 * 
 *   
 */

namespace rtc {

    //Track::Track()
    //   {}

    bool IsRtcp(const binary &data) {
        if (data.size() < 8)
            return false;

        uint8_t payloadType = std::to_integer<uint8_t>(data[1]) & 0x7F;
        STrace << "Demultiplexing RTCP and RTP with payload type, value=" << int(payloadType);

        // RFC 5761 Multiplexing RTP and RTCP 4. Distinguishable RTP and RTCP Packets
        // https://www.rfc-editor.org/rfc/rfc5761.html#section-4
        // It is RECOMMENDED to follow the guidelines in the RTP/AVP profile for the choice of RTP
        // payload type values, with the additional restriction that payload type values in the
        // range 64-95 MUST NOT be used. Specifically, dynamic RTP payload types SHOULD be chosen in
        // the range 96-127 where possible. Values below 64 MAY be used if that is insufficient
        // [...]
        return (payloadType >= 64 && payloadType <= 95); // Range 64-95 (inclusive) MUST be RTCP
    }

    Track::~Track() {
        ImpDesTrack();
    }

    string Track::mid() const {


        return Impmid();

    }

    Description::Direction Track::direction() const {
        return Impdirection();
    }

    Description::Media Track::description() const {
        return Impdescription();
    }

    void Track::setDescription(Description::Media description) {
        ImpsetDescription(std::move(description));
    }

    void Track::close() {
        close();
    }

    bool Track::send(message_variant data) {
        return outgoing(make_message(std::move(data)));
    }

    bool Track::send(const byte *data, size_t size) {
        return send(binary(data, data + size));
    }

    bool Track::isOpen(void) const {
        return ImpisOpen();
    }

    bool Track::isClosed(void) const {
        return ImpisClosed();
    }

    size_t Track::maxMessageSize() const {
        return maxMessageSize();
    }

    void Track::setMediaHandler(shared_ptr<MediaHandler> handler) {
        ImpsetMediaHandler(std::move(handler));
    }

    void Track::chainMediaHandler(shared_ptr<MediaHandler> handler) {
        if (auto first = getMediaHandler())
            first->addToChain(std::move(handler));
        else
            ImpsetMediaHandler(std::move(handler));
    }

    bool Track::requestKeyframe() {
        // only push PLI for video
        if (description().type() == "video")
            if (auto handler = getMediaHandler())
                return handler->requestKeyframe([this](message_ptr m) {
                    transportSend(m); });

        return false;
    }

    bool Track::requestBitrate(unsigned int bitrate) {
        if (auto handler = getMediaHandler())
            return handler->requestBitrate(bitrate,
                [this](message_ptr m) {
                    transportSend(m); });

        return false;
    }

    void Track::onFrame(std::function<void(binary data, FrameInfo frame) > callback) {
        ImponFrame(callback);
    }




    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Track::Track(weak_ptr<PeerConnection> pc, Description::Media desc)
    : mPeerConnection(pc), mMediaDescription(std::move(desc)) {

        // Discard messages by default if track is send only
        if (mMediaDescription.direction() == Description::Direction::SendOnly)
            messageCallback = [](message_variant) {
            };
    }

    void Track::ImpDesTrack() {
        SInfo << "Destroying Track";
        try {
            Impclose();
        } catch (const std::exception &e) {
            SError << e.what();
        }
    }

    string Track::Impmid() const {
        //std::shared_lock lock(mMutex);
        return mMediaDescription.mid();
    }

    Description::Direction Track::Impdirection() const {
        //std::shared_lock lock(mMutex);
        return mMediaDescription.direction();
    }

    Description::Media Track::Impdescription() const {
        //std::shared_lock lock(mMutex);
        return mMediaDescription;
    }

    void Track::ImpsetDescription(Description::Media desc) {
        {
            //std::unique_lock lock(mMutex);
            if (desc.mid() != mMediaDescription.mid())
                throw std::logic_error("Media description mid does not match track mid");

            mMediaDescription = std::move(desc);
        }

        if (auto handler = getMediaHandler())
            handler->media(description());
    }

    void Track::Impclose() {
        SInfo << "Closing Track";

        if (!mIsClosed.exchange(true))
            triggerClosed();

        setMediaHandler(nullptr);
        resetCallbacks();
    }

    message_variant Track::trackMessageToVariant(message_ptr message) {
        if (message->type == Message::Control)
            return to_variant(*message); // The same message may be frowarded into multiple Tracks
        else
            return to_variant(std::move(*message));
    }

    optional<message_variant> Track::receive() {
        if (auto next = mRecvQueue.front()) {
            mRecvQueue.pop();
            return trackMessageToVariant(next);
        }
        return nullopt;
    }

    optional<message_variant> Track::peek() {
        if (auto next = mRecvQueue.front()) {
            return trackMessageToVariant(next);
        }
        return nullopt;
    }

    size_t Track::availableAmount() const {
        return mRecvQueue.size();
    }

    bool Track::ImpisOpen(void) const {
#if RTC_ENABLE_MEDIA
        //	std::shared_lock lock(mMutex);
        return !mIsClosed && mDtlsSrtpTransport;
#else
        return false;
#endif
    }

    bool Track::ImpisClosed(void) const {
        return mIsClosed;
    }

    size_t Track::ImpmaxMessageSize() const {
        optional<size_t> mtu;
        if (auto pc = mPeerConnection.lock())
            mtu = pc->config.mtu;

        return mtu.value_or(DEFAULT_MTU) - 12 - 8 - 40; // SRTP/UDP/IPv6
    }

#if RTC_ENABLE_MEDIA

void Track::open(Transport *transport) {
        {
            std::lock_guard lock(mMutex);
            mDtlsSrtpTransport = transport;
        }

        if (!mIsClosed)
            triggerOpen();
    }
#endif

    void Track::incoming(message_ptr message) {
        if (!message)
            return;

        auto dir = direction();
        if ((dir == Description::Direction::SendOnly || dir == Description::Direction::Inactive) &&
                message->type != Message::Control) {
            SWarn << "COUNTER MEDIA BAD DIRECTION";
            return;
        }

        message_vector messages{std::move(message)};
        // TBD
        if (auto handler = getMediaHandler())
            handler->incomingChain(messages, [this, weak_this = weak_from_this()](message_ptr m){
                if (auto locked = weak_this.lock()) {
                    transportSend(m);
                }
            });

        for (auto &m : messages) {
            // Tail drop if queue is full
            if (mRecvQueue.size() > 20) {
                SWarn << "COUNTER QUEUE FULL";
                return;
            }

            mRecvQueue.push(m);
            triggerAvailable(mRecvQueue.size());
        }
    }

    bool Track::outgoing(message_ptr message) {
        if (mIsClosed)
            throw std::runtime_error("Track is closed");

        auto handler = getMediaHandler();

        // If there is no handler, the track expects RTP or RTCP packets
        if (!handler && IsRtcp(*message))
            message->type = Message::Control; // to allow sending RTCP packets irrelevant of direction

        auto dir = direction();
        if ((dir == Description::Direction::RecvOnly || dir == Description::Direction::Inactive) &&
                message->type != Message::Control) {
            SWarn << "COUNTER MEDIA BAD DIRECTION";
            return false;
        }

        if (handler) {
            message_vector messages{std::move(message)};
            // TBD
            handler->outgoingChain(messages, [this, weak_this = weak_from_this()](message_ptr m){
                if (auto locked = weak_this.lock()) {
                    transportSend(m);
                }
            });
            bool ret = false;
            for (auto &m : messages)
                ret = transportSend(std::move(m));

            return ret;

        } else {
            return transportSend(std::move(message));
        }
    }

    bool Track::transportSend([[maybe_unused]] message_ptr message) {

       
        const uint8_t* rtpData = reinterpret_cast<const uint8_t*> (message->data());

        if (message->type == Message::Control) {

            rtc::RTCP::Packet* packet = rtc::RTCP::Packet::Parse(rtpData, message->size());

            if (!packet) {
                SError << "rtc::RTCP::Packet::Parser failed ";

                return false;
            }
            SInfo << "Track::transportSend RTCP ";
            mDtlsSrtpTransport->SendRtcpPacket(packet);
            
             delete packet;

        } else {

            rtc::RtpPacket* packet = rtc::RtpPacket::Parse(rtpData, message->size());

            if (!packet) {
                SError << "rtc::RtpPacket::Parser failed ";

                return false;
            }
            
            SInfo <<  "no suitable Producer for received RTP packet [ssrc:"  << packet->GetSsrc() <<  ", payloadType: " << packet->GetPayloadType() << "] seq: " << packet->GetSequenceNumber()   << " isKeyFrame: "  <<  packet->IsKeyFrame();
                

            mDtlsSrtpTransport->SendRtpPacket(packet);
            
            delete packet;
        }




#if RTC_ENABLE_MEDIA
        //	shared_ptr<Transport> transport;
        //	{
        //		std::shared_lock lock(mMutex);
        //		transport = mDtlsSrtpTransport.lock();
        //		if (!transport)
        //			throw std::runtime_error("Track is closed");
        //
        //		// Set recommended medium-priority DSCP value
        //		// See https://www.rfc-editor.org/rfc/rfc8837.html#section-5
        //		if (mMediaDescription.type() == "audio")
        //			message->dscp = 46; // EF: Expedited Forwarding
        //		else
        //			message->dscp = 36; // AF42: Assured Forwarding class 4, medium drop probability
        //	}

        return true; // TBD  transport->sendMedia(message);
#else
        throw std::runtime_error("Track is disabled (not compiled with media support)");
#endif
    }

    void Track::ImpsetMediaHandler(shared_ptr<MediaHandler> handler) {
        {
            //std::unique_lock lock(mMutex);
            mMediaHandler = handler;
        }

        if (handler)
            handler->media(description());
    }

    shared_ptr<MediaHandler> Track::getMediaHandler() {
        //std::shared_lock lock(mMutex);
        return mMediaHandler;
    }

    void Track::ImponFrame(std::function<void(binary data, FrameInfo frame) > callback) {
        frameCallback = callback;
        flushPendingMessages();
    }

    void Track::flushPendingMessages() {
        if (!mOpenTriggered)
            return;

        while ((messageCallback || frameCallback) && mRecvQueue.size() ) {
            auto next = mRecvQueue.front();
            if (!next)
                break;
            mRecvQueue.pop();

            auto message = next;
            try {
                if (message->frameInfo != nullptr && frameCallback) {
                    frameCallback(std::move(*message), std::move(*message->frameInfo));
                } else if (message->frameInfo == nullptr && messageCallback) {
                    messageCallback(trackMessageToVariant(message));
                }
            } catch (const std::exception &e) {
                SWarn << "Uncaught exception in callback: " << e.what();
            }
        }
    }
























} // namespace rtc
