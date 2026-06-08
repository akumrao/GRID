#ifndef MS_RTC_TRANSPORT_CONGESTION_CONTROL_CLIENT_HPP
#define MS_RTC_TRANSPORT_CONGESTION_CONTROL_CLIENT_HPP

#include "common.h"
#include "RTC/BweType.h"
#include "RTC/RTCP/FeedbackRtpTransport.h"
#include "RTC/RTCP/ReceiverReport.h"
#include "RTC/RtpPacket.h"
#include "RTC/RtpProbationGenerator.h"
#include "RTC/TrendCalculator.h"
#include "base/Timer.h"
#include <libwebrtc/api/transport/goog_cc_factory.h>
#include <libwebrtc/api/transport/network_types.h>
#include <libwebrtc/call/rtp_transport_controller_send.h>
#include <libwebrtc/modules/pacing/packet_router.h>

using namespace base;

namespace rtc
{
	class TransportCongestionControlClient : public webrtc::PacketRouter,
	                                         public webrtc::TargetTransferRateObserver,
	                                         public Timer::Listener
	{
	public:
		struct Bitrates
		{
			uint32_t desiredBitrate{ 0u };
			uint32_t effectiveDesiredBitrate{ 0u };
			uint32_t minBitrate{ 0u };
			uint32_t maxBitrate{ 0u };
			uint32_t startBitrate{ 0u };
			uint32_t maxPaddingBitrate{ 0u };
			uint32_t availableBitrate{ 0u };
		};

	public:
		class Listener
		{
		public:
			virtual void OnTransportCongestionControlClientBitrates(
			  rtc::TransportCongestionControlClient* tccClient,
			  rtc::TransportCongestionControlClient::Bitrates& bitrates) = 0;
			virtual void OnTransportCongestionControlClientSendRtpPacket(
			  rtc::TransportCongestionControlClient* tccClient,
			  rtc::RtpPacket* packet,
			  const webrtc::PacedPacketInfo& pacingInfo) = 0;
		};

	public:
		TransportCongestionControlClient(
		  rtc::TransportCongestionControlClient::Listener* listener,
		  rtc::BweType bweType,
		  uint32_t initialAvailableBitrate);
		virtual ~TransportCongestionControlClient();

	public:
		rtc::BweType GetBweType() const;
		void TransportConnected();
		void TransportDisconnected();
		void InsertPacket(webrtc::RtpPacketSendInfo& packetInfo);
		webrtc::PacedPacketInfo GetPacingInfo();
		void PacketSent(webrtc::RtpPacketSendInfo& packetInfo, uint64_t nowMs);
		void ReceiveEstimatedBitrate(uint32_t bitrate);
		void ReceiveRtcpReceiverReport(const webrtc::RTCPReportBlock& report, float rtt, uint64_t nowMs);
		void ReceiveRtcpTransportFeedback(const rtc::RTCP::FeedbackRtpTransportPacket* feedback);
		void SetDesiredBitrate(uint32_t desiredBitrate, bool force);
		const Bitrates& GetBitrates() const;
		uint32_t GetAvailableBitrate() const;
		void RescheduleNextAvailableBitrateEvent();

	private:
		void MayEmitAvailableBitrateEvent(uint32_t previousAvailableBitrate);

		// jmillan: missing.
		// void OnRemoteNetworkEstimate(NetworkStateEstimate estimate) override;

		/* Pure virtual methods inherited from webrtc::TargetTransferRateObserver. */
	public:
		void OnTargetTransferRate(webrtc::TargetTransferRate targetTransferRate) override;

		/* Pure virtual methods inherited from webrtc::PacketRouter. */
	public:
		void SendPacket(rtc::RtpPacket* packet, const webrtc::PacedPacketInfo& pacingInfo) override;
		rtc::RtpPacket* GeneratePadding(size_t size) override;

		/* Pure virtual methods inherited from rtc::Timer. */
	public:
		void OnTimer(Timer* timer) override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Allocated by this.
		webrtc::NetworkControllerFactoryInterface* controllerFactory{ nullptr };
		webrtc::RtpTransportControllerSend* rtpTransportControllerSend{ nullptr };
		rtc::RtpProbationGenerator* probationGenerator{ nullptr };
		Timer* processTimer{ nullptr };
		// Others.
		rtc::BweType bweType;
		uint32_t initialAvailableBitrate{ 0u };
		Bitrates bitrates;
		bool availableBitrateEventCalled{ false };
		uint64_t lastAvailableBitrateEventAtMs{ 0u };
		rtc::TrendCalculator desiredBitrateTrend;
	};

	/* Inline instance methods. */

	inline rtc::BweType TransportCongestionControlClient::GetBweType() const
	{
		return this->bweType;
	}

	inline const rtc::TransportCongestionControlClient::Bitrates& TransportCongestionControlClient::GetBitrates() const
	{
		return this->bitrates;
	}
} // namespace rtc

#endif
