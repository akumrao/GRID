#ifndef MS_RTC_RTP_STREAM_SEND_HPP
#define MS_RTC_RTP_STREAM_SEND_HPP

#include "RTC/RateCalculator.h"
#include "RTC/RtpStream.h"
#include <vector>

namespace rtc
{
	class RtpStreamSend : public rtc::RtpStream
	{
	public:
		class Listener : public rtc::RtpStream::Listener
		{
		public:
			virtual void OnRtpStreamRetransmitRtpPacket(
			  rtc::RtpStreamSend* rtpStream, rtc::RtpPacket* packet) = 0;
		};

	public:
		struct StorageItem
		{
			// Cloned packet.
			rtc::RtpPacket* packet{ nullptr };
			// Memory to hold the cloned packet (with extra space for RTX encoding).
			uint8_t store[rtc::MtuSize + 100];
			// Last time this packet was resent.
			uint64_t resentAtMs{ 0u };
			// Number of times this packet was resent.
			uint8_t sentTimes{ 0u };
			// Whether the packet has been already RTX encoded.
			bool rtxEncoded{ false };
		};

	public:
		RtpStreamSend(
		  rtc::RtpStreamSend::Listener* listener, rtc::RtpStream::Params& params, size_t bufferSize);
		~RtpStreamSend() override;

		void FillJsonStats(json& jsonObject) override;
		void SetRtx(uint8_t payloadType, uint32_t ssrc) override;
		bool ReceivePacket(rtc::RtpPacket* packet) override;
		void ReceiveNack(rtc::RTCP::FeedbackRtpNackPacket* nackPacket);
		void ReceiveKeyFrameRequest(rtc::RTCP::FeedbackPs::MessageType messageType);
		void ReceiveRtcpReceiverReport(rtc::RTCP::ReceiverReport* report);
		rtc::RTCP::SenderReport* GetRtcpSenderReport(uint64_t nowMs);
		rtc::RTCP::SdesChunk* GetRtcpSdesChunk();
		void Pause() override;
		void Resume() override;
		uint32_t GetBitrate(uint64_t nowMs) override;
		uint32_t GetBitrate(uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer) override;
		uint32_t GetSpatialLayerBitrate(uint64_t nowMs, uint8_t spatialLayer) override;
		uint32_t GetLayerBitrate(uint64_t nowMs, uint8_t spatialLayer, uint8_t temporalLayer) override;

	private:
		void StorePacket(rtc::RtpPacket* packet);
		void ClearBuffer();
		void ResetStorageItem(StorageItem* storageItem);
		void UpdateBufferStartIdx();
		void FillRetransmissionContainer(uint16_t seq, uint16_t bitmask);
		void UpdateScore(rtc::RTCP::ReceiverReport* report);

	private:
		uint32_t lostPriorScore{ 0 }; // Packets lost at last interval for score calculation.
		uint32_t sentPriorScore{ 0 }; // Packets sent at last interval for score calculation.
		std::vector<StorageItem*> buffer;
		uint16_t bufferStartIdx{ 0 };
		size_t bufferSize{ 0 };
		std::vector<StorageItem> storage;
		uint16_t rtxSeq{ 0 };
		rtc::RtpDataCounter transmissionCounter;
	};

	/* Inline instance methods */

	inline uint32_t RtpStreamSend::GetBitrate(uint64_t nowMs)
	{
		return this->transmissionCounter.GetBitrate(nowMs);
	}
} // namespace rtc

#endif
