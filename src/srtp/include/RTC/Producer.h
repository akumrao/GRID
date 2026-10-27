#ifndef MS_RTC_PRODUCER_HPP
#define MS_RTC_PRODUCER_HPP

#include "common.h"
//#include "Channel/Request.h"
#include "RTC/KeyFrameRequestManager.h"
#include "RTC/RTCP/CompoundPacket.h"
#include "RTC/RTCP/Packet.h"
#include "RTC/RTCP/SenderReport.h"
#include "RTC/RTCP/XrDelaySinceLastRr.h"
#include "RTC/RtpDictionaries.h"
#include "RTC/RtpHeaderExtensionIds.h"
#include "RTC/RtpPacket.h"
#include "RTC/RtpStreamRecv.h"
#include <json.hpp>
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace rtc
{
	class Producer : public rtc::RtpStreamRecv::Listener, public rtc::KeyFrameRequestManager::Listener
	{
	public:
		class Listener
		{
		public:
			virtual void OnProducerPaused(rtc::Producer* producer)  = 0;
			virtual void OnProducerResumed(rtc::Producer* producer) = 0;
			virtual void OnProducerNewRtpStream(
			  rtc::Producer* producer, rtc::RtpStream* rtpStream, uint32_t mappedSsrc) = 0;
			virtual void OnProducerRtpStreamScore(
			  rtc::Producer* producer, rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) = 0;
			virtual void OnProducerRtcpSenderReport(
			  rtc::Producer* producer, rtc::RtpStream* rtpStream, bool first)                         = 0;
			virtual void OnProducerRtpPacketReceived(rtc::Producer* producer, rtc::RtpPacket* packet) = 0;
			virtual void OnProducerSendRtcpPacket(rtc::Producer* producer, rtc::RTCP::Packet* packet) = 0;
			virtual void OnProducerNeedWorstRemoteFractionLost(
			  rtc::Producer* producer, uint32_t mappedSsrc, uint8_t& worstRemoteFractionLost) = 0;
		};

	private:
		struct RtpEncodingMapping
		{
			std::string rid;
			uint32_t ssrc{ 0 };
			uint32_t mappedSsrc{ 0 };
		};

	private:
		struct RtpMapping
		{
			std::map<uint8_t, uint8_t> codecs;
			std::vector<RtpEncodingMapping> encodings;
		};

	private:
		struct VideoOrientation
		{
			bool camera{ false };
			bool flip{ false };
			uint16_t rotation{ 0 };
		};

	public:
		enum class ReceiveRtpPacketResult
		{
			DISCARDED = 0,
			MEDIA     = 1,
			RETRANSMISSION
		};

	private:
		struct TraceEventTypes
		{
			bool rtp{ false };
			bool keyframe{ false };
			bool nack{ false };
			bool pli{ false };
			bool fir{ false };
		};

	public:
		Producer(const std::string& id, rtc::Producer::Listener* listener, json& data);
		virtual ~Producer();

	public:
		void FillJson(json& jsonObject) const;
		void FillJsonStats(json& jsonArray) const;
		void HandleRequest();
		rtc::Media::Kind GetKind() const;
		const rtc::RtpParameters& GetRtpParameters() const;
		const struct rtc::RtpHeaderExtensionIds& GetRtpHeaderExtensionIds() const;
		rtc::RtpParameters::Type GetType() const;
		bool IsPaused() const;
		std::map<rtc::RtpStreamRecv*, uint32_t>& GetRtpStreams();
		ReceiveRtpPacketResult ReceiveRtpPacket(rtc::RtpPacket* packet);
		void ReceiveRtcpSenderReport(rtc::RTCP::SenderReport* report);
		void ReceiveRtcpXrDelaySinceLastRr(rtc::RTCP::DelaySinceLastRr::SsrcInfo* ssrcInfo);
		void GetRtcp(rtc::RTCP::CompoundPacket* packet, uint64_t nowMs);
		void RequestKeyFrame(uint32_t mappedSsrc);

	private:
		rtc::RtpStreamRecv* GetRtpStream(rtc::RtpPacket* packet);
		rtc::RtpStreamRecv* CreateRtpStream(
		  rtc::RtpPacket* packet, const rtc::RtpCodecParameters& mediaCodec, size_t encodingIdx);
		void NotifyNewRtpStream(rtc::RtpStreamRecv* rtpStream);
		void PreProcessRtpPacket(rtc::RtpPacket* packet);
		bool MangleRtpPacket(rtc::RtpPacket* packet, rtc::RtpStreamRecv* rtpStream) const;
		void PostProcessRtpPacket(rtc::RtpPacket* packet);
		void EmitScore() const;
		void EmitTraceEventRtpAndKeyFrameTypes(rtc::RtpPacket* packet, bool isRtx = false) const;
		void EmitTraceEventKeyFrameType(rtc::RtpPacket* packet, bool isRtx = false) const;
		void EmitTraceEventPliType(uint32_t ssrc) const;
		void EmitTraceEventFirType(uint32_t ssrc) const;
		void EmitTraceEventNackType() const;

		/* Pure virtual methods inherited from rtc::RtpStreamRecv::Listener. */
	public:
		void OnRtpStreamScore(rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void OnRtpStreamSendRtcpPacket(rtc::RtpStreamRecv* rtpStream, rtc::RTCP::Packet* packet) override;
		void OnRtpStreamNeedWorstRemoteFractionLost(
		  rtc::RtpStreamRecv* rtpStream, uint8_t& worstRemoteFractionLost) override;

		/* Pure virtual methods inherited from rtc::KeyFrameRequestManager::Listener. */
	public:
		void OnKeyFrameNeeded(rtc::KeyFrameRequestManager* keyFrameRequestManager, uint32_t ssrc) override;

	public:
		// Passed by argument.
		const std::string id;

	private:
		// Passed by argument.
		rtc::Producer::Listener* listener{ nullptr };
		// Allocated by this.
		std::map<uint32_t, rtc::RtpStreamRecv*> mapSsrcRtpStream;
		rtc::KeyFrameRequestManager* keyFrameRequestManager{ nullptr };
		// Others.
		rtc::Media::Kind kind;
		rtc::RtpParameters rtpParameters;
		rtc::RtpParameters::Type type{ rtc::RtpParameters::Type::NONE };
		struct RtpMapping rtpMapping;
		std::map<uint32_t, rtc::RtpStreamRecv*> mapRtxSsrcRtpStream;
		std::map<rtc::RtpStreamRecv*, uint32_t> mapRtpStreamMappedSsrc;
		std::map<uint32_t, uint32_t> mapMappedSsrcSsrc;
		struct rtc::RtpHeaderExtensionIds rtpHeaderExtensionIds;
		bool paused{ false };
		rtc::RtpPacket* currentRtpPacket{ nullptr };
		// Timestamp when last RTCP was sent.
		uint64_t lastRtcpSentTime{ 0 };
		uint16_t maxRtcpInterval{ 0 };
		// Video orientation.
		bool videoOrientationDetected{ false };
		struct VideoOrientation videoOrientation;
		struct TraceEventTypes traceEventTypes;
	};

	/* Inline methods. */

	inline rtc::Media::Kind Producer::GetKind() const
	{
		return this->kind;
	}

	inline const rtc::RtpParameters& Producer::GetRtpParameters() const
	{
		return this->rtpParameters;
	}

	inline const struct rtc::RtpHeaderExtensionIds& Producer::GetRtpHeaderExtensionIds() const
	{
		return this->rtpHeaderExtensionIds;
	}

	inline rtc::RtpParameters::Type Producer::GetType() const
	{
		return this->type;
	}

	inline bool Producer::IsPaused() const
	{
		return this->paused;
	}

	inline std::map<rtc::RtpStreamRecv*, uint32_t>& Producer::GetRtpStreams()
	{
		return this->mapRtpStreamMappedSsrc;
	}
} // namespace rtc

#endif
