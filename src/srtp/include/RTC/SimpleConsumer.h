#ifndef MS_RTC_SIMPLE_CONSUMER_HPP
#define MS_RTC_SIMPLE_CONSUMER_HPP

#include "RTC/Consumer.h"
#include "RTC/RtpStreamSend.h"
#include "RTC/SeqManager.h"

namespace rtc
{
	class SimpleConsumer : public rtc::Consumer, public rtc::RtpStreamSend::Listener
	{
	public:
		SimpleConsumer(const std::string& id, rtc::Consumer::Listener* listener, json& data);
		~SimpleConsumer() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) const override;
		void FillJsonScore(json& jsonObject) const override;
		void HandleRequest(Channel::Request* request) override;
		bool IsActive() const override;
		void ProducerRtpStream(rtc::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerNewRtpStream(rtc::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerRtpStreamScore(rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void ProducerRtcpSenderReport(rtc::RtpStream* rtpStream, bool first) override;
		uint8_t GetBitratePriority() const override;
		uint32_t IncreaseLayer(uint32_t bitrate, bool considerLoss) override;
		void ApplyLayers() override;
		uint32_t GetDesiredBitrate() const override;
		void SendRtpPacket(rtc::RtpPacket* packet) override;
		std::vector<rtc::RtpStreamSend*> GetRtpStreams() override;
		void GetRtcp(rtc::RTCP::CompoundPacket* packet, rtc::RtpStreamSend* rtpStream, uint64_t nowMs) override;
		void NeedWorstRemoteFractionLost(uint32_t mappedSsrc, uint8_t& worstRemoteFractionLost) override;
		void ReceiveNack(rtc::RTCP::FeedbackRtpNackPacket* nackPacket) override;
		void ReceiveKeyFrameRequest(rtc::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc) override;
		void ReceiveRtcpReceiverReport(rtc::RTCP::ReceiverReport* report) override;
		uint32_t GetTransmissionRate(uint64_t nowMs) override;
		float GetRtt() const override;

	private:
		void UserOnTransportConnected() override;
		void UserOnTransportDisconnected() override;
		void UserOnPaused() override;
		void UserOnResumed() override;
		void CreateRtpStream();
		void RequestKeyFrame();
		void EmitScore() const;

		/* Pure virtual methods inherited from RtpStreamSend::Listener. */
	public:
		void OnRtpStreamScore(rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void OnRtpStreamRetransmitRtpPacket(rtc::RtpStreamSend* rtpStream, rtc::RtpPacket* packet) override;

	private:
		// Allocated by this.
		rtc::RtpStreamSend* rtpStream{ nullptr };
		// Others.
		std::vector<rtc::RtpStreamSend*> rtpStreams;
		rtc::RtpStream* producerRtpStream{ nullptr };
		bool keyFrameSupported{ false };
		bool syncRequired{ false };
		rtc::SeqManager<uint16_t> rtpSeqManager;
		bool managingBitrate{ false };
	};

	/* Inline methods. */

	inline bool SimpleConsumer::IsActive() const
	{
		
		return (
			rtc::Consumer::IsActive() &&
			this->producerRtpStream &&
			this->producerRtpStream->GetScore() > 0u
		);
		
	}

	inline std::vector<rtc::RtpStreamSend*> SimpleConsumer::GetRtpStreams()
	{
		return this->rtpStreams;
	}
} // namespace rtc

#endif
