#ifndef MS_RTC_RTP_STREAM_RECV_HPP
#define MS_RTC_RTP_STREAM_RECV_HPP

#include "RTC/NackGenerator.h"
#include "RTC/RTCP/XrDelaySinceLastRr.h"
#include "RTC/RateCalculator.h"
#include "RTC/RtpStream.h"
#include "base/Timer.h"
#include <vector>

using namespace base;

namespace rtc
{
	class RtpStreamRecv : public rtc::RtpStream,
	                      public rtc::NackGenerator::Listener,
	                      public Timer::Listener
	{
	public:
		class Listener : public rtc::RtpStream::Listener
		{
		public:
			virtual void OnRtpStreamSendRtcpPacket(
			  rtc::RtpStreamRecv* rtpStream, rtc::RTCP::Packet* packet) = 0;
			virtual void OnRtpStreamNeedWorstRemoteFractionLost(
			  rtc::RtpStreamRecv* rtpStream, uint8_t& worstRemoteFractionLost) = 0;
		};

	public:
		class TransmissionCounter
		{
		public:
			TransmissionCounter(uint8_t spatialLayers, uint8_t temporalLayers);
			void Update(rtc::RtpPacket* packet);
			uint32_t GetBitrate(uint64_t nowMs);
			uint32_t GetBitrate(uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer);
			uint32_t GetSpatialLayerBitrate(uint64_t nowMs, uint8_t spatialLayer);
			uint32_t GetLayerBitrate(uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer);
			size_t GetPacketCount() const;
			size_t GetBytes() const;

		private:
			std::vector<std::vector<rtc::RtpDataCounter>> spatialLayerCounters;
		};

	public:
		RtpStreamRecv(rtc::RtpStreamRecv::Listener* listener, rtc::RtpStream::Params& params);
		~RtpStreamRecv();

		void FillJsonStats(json& jsonObject) override;
		bool ReceivePacket(rtc::RtpPacket* packet) override;
		bool ReceiveRtxPacket(rtc::RtpPacket* packet);
		rtc::RTCP::ReceiverReport* GetRtcpReceiverReport();
		rtc::RTCP::ReceiverReport* GetRtxRtcpReceiverReport();
		void ReceiveRtcpSenderReport(rtc::RTCP::SenderReport* report);
		void ReceiveRtxRtcpSenderReport(rtc::RTCP::SenderReport* report);
		void ReceiveRtcpXrDelaySinceLastRr(rtc::RTCP::DelaySinceLastRr::SsrcInfo* ssrcInfo);
		void RequestKeyFrame();
		void Pause() override;
		void Resume() override;
		uint32_t GetBitrate(uint64_t nowMs) override;
		uint32_t GetBitrate(uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer) override;
		uint32_t GetSpatialLayerBitrate(uint64_t nowMs, uint8_t spatialLayer) override;
		uint32_t GetLayerBitrate(uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer) override;

	private:
		void CalculateJitter(uint32_t rtpTimestamp);
		void UpdateScore();

		/* Pure virtual methods inherited from Timer. */
	protected:
		void OnTimer(Timer* timer) override;

		/* Pure virtual methods inherited from rtc::NackGenerator. */
	protected:
		void OnNackGeneratorNackRequired(const std::vector<uint16_t>& seqNumbers) override;
		void OnNackGeneratorKeyFrameRequired() override;

	private:
		uint32_t expectedPrior{ 0 };      // Packets expected at last interval.
		uint32_t expectedPriorScore{ 0 }; // Packets expected at last interval for score calculation.
		uint32_t receivedPrior{ 0 };      // Packets received at last interval.
		uint32_t receivedPriorScore{ 0 }; // Packets received at last interval for score calculation.
		uint32_t lastSrTimestamp{ 0 };    // The middle 32 bits out of 64 in the NTP
		                                  // timestamp received in the most recent
		                                  // sender report.
		uint64_t lastSrReceived{ 0 };     // Wallclock time representing the most recent
		                                  // sender report arrival.
		uint32_t transit{ 0 };            // Relative transit time for prev packet.
		uint32_t jitter{ 0 };
		uint8_t firSeqNumber{ 0 };
		uint32_t reportedPacketLost{ 0 };
		std::unique_ptr<rtc::NackGenerator> nackGenerator;
		Timer* inactivityCheckPeriodicTimer{ nullptr };
		bool inactive{ false };
		TransmissionCounter transmissionCounter;      // Valid media + valid RTX.
		rtc::RtpDataCounter mediaTransmissionCounter; // Just valid media.
	};

	/* Inline instance methods */

	inline uint32_t RtpStreamRecv::GetBitrate(uint64_t nowMs)
	{
		return this->transmissionCounter.GetBitrate(nowMs);
	}

	inline uint32_t RtpStreamRecv::GetBitrate(uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer)
	{
		return this->transmissionCounter.GetBitrate(nowMs, spatialLayer, temporalLayer);
	}

	inline uint32_t RtpStreamRecv::GetSpatialLayerBitrate(uint64_t nowMs, uint8_t spatialLayer)
	{
		return this->transmissionCounter.GetSpatialLayerBitrate(nowMs, spatialLayer);
	}

	inline uint32_t RtpStreamRecv::GetLayerBitrate(
	  uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer)
	{
		return this->transmissionCounter.GetLayerBitrate(nowMs, spatialLayer, temporalLayer);
	}
} // namespace rtc

#endif
