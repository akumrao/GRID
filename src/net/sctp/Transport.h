#ifndef MS_RTC_TRANSPORT_HPP
#define MS_RTC_TRANSPORT_HPP
#define ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR

#include "common.h"
#include "base/application.h"
#include "sctptransport.hpp"
#include "DtlsTransport.h"
#include "DataConsumer.h"
#include "DataProducer.h"

//#include "RTC/SctpTransport.h"
//#include "RTC/SctpListener.h"
//
//#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
//#include "RTC/SenderBandwidthEstimator.h"
//#endif
//#include "RTC/TransportCongestionControlClient.h"
//#include "RTC/TransportCongestionControlServer.h"
#include "base/Timer.h"
#include <string>
#include <unordered_map>

using namespace base;

using json = nlohmann::json;

namespace rtc
{
	class Transport : 
	                public SctpTransport::Listener,
	                public Timer::Listener
	{
	protected:
		using onSendCallback = const std::function<void(bool sent)>;

	public:
		class Listener
		{
		public:
		
                    virtual void OnDtlsTransportStatus(DtlsTransport::DtlsState state) {} ;
                    virtual void OnSctpState(SctpTransport::State state){};
                    virtual void OnSctpTransportMessageReceived(SctpTransport* sctpAssociation ,message_ptr message ){};
			//virtual void OnTransportDataProducerSctpMessageReceived(  rtc::Transport* transport,  rtc::DataProducer* dataProducer,	  uint32_t ppid,  const uint8_t* msg,   size_t len) = 0;
		};

	private:
		struct TraceEventTypes
		{
			bool probation{ false };
			bool bwe{ false };
		};
                
                const Configuration &config;

	public:

                
		Transport(const std::string& id, int agentNo, const Configuration &config, Listener* listener);
		virtual ~Transport();

	public:
		virtual void HandleRequest();

	public:
		// Must be called from the subclass.
            
                rtc::SctpTransport*  Connected(rtc::SctpTransport::Ports &port);
		void Connected();
		void Disconnected();
		void DataReceived(size_t len);
		void DataSent(size_t len);
		//void ReceiveRtpPacket(rtc::RtpPacket* packet);
		//void ReceiveRtcpPacket(rtc::RTCP::Packet* packet);
		void ReceiveSctpData(byte * data, size_t len);

	private:
//		void SetNewProducerIdFromRequest(Channel::Request* request, std::string& producerId) const;
//		rtc::Producer* GetProducerFromRequest(Channel::Request* request) const;
//		void SetNewConsumerIdFromRequest(Channel::Request* request, std::string& consumerId) const;
//		rtc::Consumer* GetConsumerFromRequest(Channel::Request* request) const;
//		rtc::Consumer* GetConsumerByMediaSsrc(uint32_t ssrc) const;
//		rtc::Consumer* GetConsumerByRtxSsrc(uint32_t ssrc) const;
//		void SetNewDataProducerIdFromRequest(Channel::Request* request, std::string& dataProducerId) const;
//		rtc::DataProducer* GetDataProducerFromRequest(Channel::Request* request) const;
//		void SetNewDataConsumerIdFromRequest(Channel::Request* request, std::string& dataConsumerId) const;
//		rtc::DataConsumer* GetDataConsumerFromRequest(Channel::Request* request) const;
//		virtual bool IsConnected() const                                                 = 0;
//		virtual void SendRtpPacket(rtc::RtpPacket* packet, onSendCallback cb = nullptr) = 0;
//		void HandleRtcpPacket(rtc::RTCP::Packet* packet);
//		void SendRtcp(uint64_t nowMs);
//		virtual void SendRtcpPacket(rtc::RTCP::Packet* packet)                 = 0;
//		virtual void SendRtcpCompoundPacket(rtc::RTCP::CompoundPacket* packet) = 0;
		virtual void SendSctpData(const uint8_t* data, size_t len)             = 0;
//		void DistributeAvailableOutgoingBitrate();
//		void ComputeOutgoingDesiredBitrate(bool forceBitrate = false);
//		void EmitTraceEventProbationType(rtc::RtpPacket* packet) const;
//		void EmitTraceEventBweType(rtc::TransportCongestionControlClient::Bitrates& bitrates) const;

		/* Pure virtual methods inherited from rtc::Producer::Listener. */
	public:
//		void OnProducerPaused(rtc::Producer* producer) override;
//		void OnProducerResumed(rtc::Producer* producer) override;
//		void OnProducerNewRtpStream(
//		  rtc::Producer* producer, rtc::RtpStream* rtpStream, uint32_t mappedSsrc) override;
//		void OnProducerRtpStreamScore(
//		  rtc::Producer* producer, rtc::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
//		void OnProducerRtcpSenderReport(
//		  rtc::Producer* producer, rtc::RtpStream* rtpStream, bool first) override;
//		void OnProducerRtpPacketReceived(rtc::Producer* producer, rtc::RtpPacket* packet) override;
//		void OnProducerSendRtcpPacket(rtc::Producer* producer, rtc::RTCP::Packet* packet) override;
//		void OnProducerNeedWorstRemoteFractionLost(
//		  rtc::Producer* producer, uint32_t mappedSsrc, uint8_t& worstRemoteFractionLost) override;

		/* Pure virtual methods inherited from rtc::Consumer::Listener. */
//	public:
//		void OnConsumerSendRtpPacket(rtc::Consumer* consumer, rtc::RtpPacket* packet) override;
//		void OnConsumerRetransmitRtpPacket(rtc::Consumer* consumer, rtc::RtpPacket* packet) override;
//		void OnConsumerKeyFrameRequested(rtc::Consumer* consumer, uint32_t mappedSsrc) override;
//		void OnConsumerNeedBitrateChange(rtc::Consumer* consumer) override;
//		void OnConsumerNeedZeroBitrate(rtc::Consumer* consumer) override;
//		void OnConsumerProducerClosed(rtc::Consumer* consumer) override;

		/* Pure virtual methods inherited from rtc::DataProducer::Listener. */
	public:
		void OnDataProducerSctpMessageReceived(
		  rtc::DataProducer* dataProducer, uint32_t ppid, const uint8_t* msg, size_t len) ;

		/* Pure virtual methods inherited from rtc::DataConsumer::Listener. */
	public:
		void OnDataConsumerSendSctpMessage(
		  rtc::DataConsumer* dataConsumer, uint32_t ppid, const uint8_t* msg, size_t len) ;
		void OnDataConsumerDataProducerClosed(rtc::DataConsumer* dataConsumer) ;

		/* Pure virtual methods inherited from rtc::SctpTransport::Listener. */
	public:
                void OnSctpState(SctpTransport::State);
		void OnSctpTransportConnecting(rtc::SctpTransport* sctpAssociation) ;
		void OnSctpTransportConnected(rtc::SctpTransport* sctpAssociation) ;
		void OnSctpTransportFailed(rtc::SctpTransport* sctpAssociation) ;
		void OnSctpTransportClosed(rtc::SctpTransport* sctpAssociation) ;
		void OnSctpTransportSendData(  rtc::SctpTransport* sctpAssociation, const uint8_t* data, size_t len) ;
//		void OnSctpTransportMessageReceived(
//		  rtc::SctpTransport* sctpAssociation,
//		  uint16_t streamId,
//		  uint32_t ppid,
//		  const uint8_t* msg,
//		  size_t len) ;
                
             void OnSctpTransportMessageReceived(SctpTransport* sctpAssociation ,message_ptr message );

		/* Pure virtual methods inherited from rtc::TransportCongestionControlClient::Listener. */
//	public:
//		void OnTransportCongestionControlClientBitrates(
//		  rtc::TransportCongestionControlClient* tccClient,
//		  rtc::TransportCongestionControlClient::Bitrates& bitrates) override;
//		void OnTransportCongestionControlClientSendRtpPacket(
//		  rtc::TransportCongestionControlClient* tccClient,
//		  rtc::RtpPacket* packet,
//		  const webrtc::PacedPacketInfo& pacingInfo) override;
//
//		/* Pure virtual methods inherited from rtc::TransportCongestionControlServer::Listener. */
//	public:
//		void OnTransportCongestionControlServerSendRtcpPacket(
//		  rtc::TransportCongestionControlServer* tccServer, rtc::RTCP::Packet* packet) override;
//
//#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
//		/* Pure virtual methods inherited from rtc::SenderBandwidthEstimator::Listener. */
//	public:
//		void OnSenderBandwidthEstimatorAvailableBitrate(
//		  rtc::SenderBandwidthEstimator* senderBwe,
//		  uint32_t availableBitrate,
//		  uint32_t previousAvailableBitrate) override;
//#endif

		/* Pure virtual methods inherited from Timer::Listener. */
	public:
		void OnTimer(Timer* timer) override;

	public:
		// Passed by argument.
		const std::string id;
                Listener* iceListener{ nullptr };
                
                rtc::SctpTransport* sctptransport{ nullptr };

	private:
		// Passed by argument.
                int agentNo;
		// Allocated by this.
//		std::unordered_map<std::string, rtc::Producer*> mapProducers;
//		std::unordered_map<std::string, rtc::Consumer*> mapConsumers;
//		std::unordered_map<std::string, rtc::DataProducer*> mapDataProducers;
//		std::unordered_map<std::string, rtc::DataConsumer*> mapDataConsumers;
//		std::unordered_map<uint32_t, rtc::Consumer*> mapSsrcConsumer;
//		std::unordered_map<uint32_t, rtc::Consumer*> mapRtxSsrcConsumer;
		
//		Timer* rtcpTimer{ nullptr };
//		rtc::TransportCongestionControlClient* tccClient{ nullptr };
//		rtc::TransportCongestionControlServer* tccServer{ nullptr };
//#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
//		rtc::SenderBandwidthEstimator* senderBwe{ nullptr };
//#endif
		// Others.
//		bool destroying{ false };
//		struct rtc::RtpHeaderExtensionIds recvRtpHeaderExtensionIds;
//		rtc::RtpListener rtpListener;
//		rtc::SctpListener sctpListener;
//		rtc::RateCalculator recvTransmission;
//		rtc::RateCalculator sendTransmission;
//		rtc::RtpDataCounter recvRtpTransmission;
//		rtc::RtpDataCounter sendRtpTransmission;
//		rtc::RtpDataCounter recvRtxTransmission;
//		rtc::RtpDataCounter sendRtxTransmission;
//		rtc::RtpDataCounter sendProbationTransmission;
//		uint16_t transportWideCcSeq{ 0u };
//		uint32_t initialAvailableOutgoingBitrate{ 600000u };
//		uint32_t maxIncomingBitrate{ 0u };
//		struct TraceEventTypes traceEventTypes;
	};

	/* Inline instance methods. */

//	inline void Transport::DataReceived(size_t len)  // TBD
//	{
//		this->recvTransmission.Update(len, base::Application::GetTimeMs());
//	}
//
//	inline void Transport::DataSent(size_t len) // TBD
//	{
//		this->sendTransmission.Update(len, base::Application::GetTimeMs());
//	}
} // namespace rtc

#endif
