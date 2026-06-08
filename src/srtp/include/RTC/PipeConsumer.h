#ifndef MS_RTC_PIPE_CONSUMER_HPP
#define MS_RTC_PIPE_CONSUMER_HPP

#include "RTC/Consumer.h"
#include "RTC/RtpStreamSend.h"
#include "RTC/SeqManager.h"

namespace rtc
{
	class PipeConsumer : public rtc::Consumer, public rtc::RtpStreamSend::Listener
	{
	public:
		PipeConsumer(const std::string& id, rtc::Consumer::Listener* listener, json& data);
		~PipeConsumer() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) const override;
		void FillJsonScore(json& jsonObject) const override;
		void HandleRequest(Channel::Request* request) override;
		void ProducerRtpStream(rtc::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerNewRtpStream(rtc::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerRtpStreamScore(rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void ProducerRtcpSenderReport(rtc::RtpStream* rtpStream, bool first) override;
		uint8_t GetBitratePriority() const override;
		uint32_t IncreaseLayer(uint32_t bitrate, bool considerLoss) override;
		void ApplyLayers() override;
		uint32_t GetDesiredBitrate() const override;
		void SendRtpPacket(rtc::RtpPacket* packet) override;
		void GetRtcp(rtc::RTCP::CompoundPacket* packet, rtc::RtpStreamSend* rtpStream, uint64_t nowMs) override;
		std::vector<rtc::RtpStreamSend*> GetRtpStreams() override;
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
		void CreateRtpStreams();
		void RequestKeyFrame();

		/* Pure virtual methods inherited from RtpStreamSend::Listener. */
	public:
		void OnRtpStreamScore(rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void OnRtpStreamRetransmitRtpPacket(rtc::RtpStreamSend* rtpStream, rtc::RtpPacket* packet) override;

	private:
		// Allocated by this.
		std::vector<rtc::RtpStreamSend*> rtpStreams;
		// Others.
		std::unordered_map<uint32_t, rtc::RtpStreamSend*> mapMappedSsrcRtpStream;
		bool keyFrameSupported{ false };
		std::unordered_map<rtc::RtpStreamSend*, bool> mapRtpStreamSyncRequired;
		std::unordered_map<rtc::RtpStreamSend*, rtc::SeqManager<uint16_t>> mapRtpStreamRtpSeqManager;
	};

	// Inline methods.

	inline std::vector<rtc::RtpStreamSend*> PipeConsumer::GetRtpStreams()
	{
		return this->rtpStreams;
	}
} // namespace rtc

#endif
