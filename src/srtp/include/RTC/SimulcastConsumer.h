#ifndef MS_RTC_SIMULCAST_CONSUMER_HPP
#define MS_RTC_SIMULCAST_CONSUMER_HPP

#include "RTC/Codecs/PayloadDescriptorHandler.h"
#include "RTC/Consumer.h"
#include "RTC/RtpStreamSend.h"
#include "RTC/SeqManager.h"

namespace rtc
{
	class SimulcastConsumer : public rtc::Consumer, public rtc::RtpStreamSend::Listener
	{
	public:
		SimulcastConsumer(const std::string& id, rtc::Consumer::Listener* listener, json& data);
		~SimulcastConsumer() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) const override;
		void FillJsonScore(json& jsonObject) const override;
		void HandleRequest(Channel::Request* request) override;
		rtc::Consumer::Layers GetPreferredLayers() const override;
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
		void CreateRtpStream();
		void RequestKeyFrames();
		void RequestKeyFrameForTargetSpatialLayer();
		void RequestKeyFrameForCurrentSpatialLayer();
		void MayChangeLayers(bool force = false);
		bool RecalculateTargetLayers(int16_t& newTargetSpatialLayer, int16_t& newTargetTemporalLayer) const;
		void UpdateTargetLayers(int16_t newTargetSpatialLayer, int16_t newTargetTemporalLayer);
		bool CanSwitchToSpatialLayer(int16_t spatialLayer) const;
		void EmitScore() const;
		void EmitLayersChange() const;
		rtc::RtpStream* GetProducerCurrentRtpStream() const;
		rtc::RtpStream* GetProducerTargetRtpStream() const;
		rtc::RtpStream* GetProducerTsReferenceRtpStream() const;

		/* Pure virtual methods inherited from RtpStreamSend::Listener. */
	public:
		void OnRtpStreamScore(rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void OnRtpStreamRetransmitRtpPacket(rtc::RtpStreamSend* rtpStream, rtc::RtpPacket* packet) override;

	private:
		// Allocated by this.
		rtc::RtpStreamSend* rtpStream{ nullptr };
		// Others.
		std::unordered_map<uint32_t, int16_t> mapMappedSsrcSpatialLayer;
		std::vector<rtc::RtpStreamSend*> rtpStreams;
		std::vector<rtc::RtpStream*> producerRtpStreams; // Indexed by spatial layer.
		bool syncRequired{ false };
		rtc::SeqManager<uint16_t> rtpSeqManager;
		int16_t preferredSpatialLayer{ -1 };
		int16_t preferredTemporalLayer{ -1 };
		int16_t provisionalTargetSpatialLayer{ -1 };
		int16_t provisionalTargetTemporalLayer{ -1 };
		int16_t targetSpatialLayer{ -1 };
		int16_t targetTemporalLayer{ -1 };
		int16_t currentSpatialLayer{ -1 };
		int16_t tsReferenceSpatialLayer{ -1 }; // Used for RTP TS sync.
		std::unique_ptr<rtc::Codecs::EncodingContext> encodingContext;
		uint32_t tsOffset{ 0u }; // RTP Timestamp offset.
		bool keyFrameForTsOffsetRequested{ false };
		uint64_t lastBweDowngradeAtMs{ 0u }; // Last time we moved to lower spatial layer due to BWE.
	};

	/* Inline methods. */

	inline rtc::Consumer::Layers SimulcastConsumer::GetPreferredLayers() const
	{
		rtc::Consumer::Layers layers;

		layers.spatial  = this->preferredSpatialLayer;
		layers.temporal = this->preferredTemporalLayer;

		return layers;
	}

	inline bool SimulcastConsumer::IsActive() const
	{
		
		return (
			rtc::Consumer::IsActive() &&
			std::any_of(
				this->producerRtpStreams.begin(),
				this->producerRtpStreams.end(),
				[](const rtc::RtpStream* rtpStream)
				{
					return (rtpStream != nullptr && rtpStream->GetScore() > 0u);
				}
			)
		);
		
	}

	inline std::vector<rtc::RtpStreamSend*> SimulcastConsumer::GetRtpStreams()
	{
		return this->rtpStreams;
	}
} // namespace rtc

#endif
