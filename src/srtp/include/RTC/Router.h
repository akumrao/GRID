#ifndef MS_RTC_ROUTER_HPP
#define MS_RTC_ROUTER_HPP

#include "common.h"
#include "Channel/Request.h"
#include "RTC/Consumer.h"
#include "RTC/DataConsumer.h"
#include "RTC/DataProducer.h"
#include "RTC/Producer.h"
#include "RTC/RtpObserver.h"
#include "RTC/RtpPacket.h"
#include "RTC/RtpStream.h"
#include "RTC/Transport.h"
#include <json.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

using json = nlohmann::json;

namespace rtc
{
	class Router : public rtc::Transport::Listener
	{
	public:
		explicit Router(const std::string& id);
		virtual ~Router();

	public:
		void FillJson(json& jsonObject) const;
		void HandleRequest(Channel::Request* request);

	private:
		void SetNewTransportIdFromRequest(Channel::Request* request, std::string& transportId) const;
		rtc::Transport* GetTransportFromRequest(Channel::Request* request) const;
		void SetNewRtpObserverIdFromRequest(Channel::Request* request, std::string& rtpObserverId) const;
		rtc::RtpObserver* GetRtpObserverFromRequest(Channel::Request* request) const;
		rtc::Producer* GetProducerFromRequest(Channel::Request* request) const;

		/* Pure virtual methods inherited from rtc::Transport::Listener. */
	public:
		void OnTransportNewProducer(rtc::Transport* transport, rtc::Producer* producer) override;
		void OnTransportProducerClosed(rtc::Transport* transport, rtc::Producer* producer) override;
		void OnTransportProducerPaused(rtc::Transport* transport, rtc::Producer* producer) override;
		void OnTransportProducerResumed(rtc::Transport* transport, rtc::Producer* producer) override;
		void OnTransportProducerNewRtpStream(
		  rtc::Transport* transport,
		  rtc::Producer* producer,
		  rtc::RtpStream* rtpStream,
		  uint32_t mappedSsrc) override;
		void OnTransportProducerRtpStreamScore(
		  rtc::Transport* transport,
		  rtc::Producer* producer,
		  rtc::RtpStream* rtpStream,
		  uint8_t score,
		  uint8_t previousScore) override;
		void OnTransportProducerRtcpSenderReport(
		  rtc::Transport* transport, rtc::Producer* producer, rtc::RtpStream* rtpStream, bool first) override;
		void OnTransportProducerRtpPacketReceived(
		  rtc::Transport* transport, rtc::Producer* producer, rtc::RtpPacket* packet) override;
		void OnTransportNeedWorstRemoteFractionLost(
		  rtc::Transport* transport,
		  rtc::Producer* producer,
		  uint32_t mappedSsrc,
		  uint8_t& worstRemoteFractionLost) override;
		void OnTransportNewConsumer(
		  rtc::Transport* transport, rtc::Consumer* consumer, std::string& producerId) override;
		void OnTransportConsumerClosed(rtc::Transport* transport, rtc::Consumer* consumer) override;
		void OnTransportConsumerProducerClosed(rtc::Transport* transport, rtc::Consumer* consumer) override;
		void OnTransportConsumerKeyFrameRequested(
		  rtc::Transport* transport, rtc::Consumer* consumer, uint32_t mappedSsrc) override;
		void OnTransportNewDataProducer(rtc::Transport* transport, rtc::DataProducer* dataProducer) override;
		void OnTransportDataProducerClosed(rtc::Transport* transport, rtc::DataProducer* dataProducer) override;
		void OnTransportDataProducerSctpMessageReceived(
		  rtc::Transport* transport,
		  rtc::DataProducer* dataProducer,
		  uint32_t ppid,
		  const uint8_t* msg,
		  size_t len) override;
		void OnTransportNewDataConsumer(
		  rtc::Transport* transport, rtc::DataConsumer* dataConsumer, std::string& dataProducerId) override;
		void OnTransportDataConsumerClosed(rtc::Transport* transport, rtc::DataConsumer* dataConsumer) override;
		void OnTransportDataConsumerDataProducerClosed(
		  rtc::Transport* transport, rtc::DataConsumer* dataConsumer) override;

	public:
		// Passed by argument.
		const std::string id;

	private:
		// Allocated by this.
		std::unordered_map<std::string, rtc::Transport*> mapTransports;
		std::unordered_map<std::string, rtc::RtpObserver*> mapRtpObservers;
		// Others.
		std::unordered_map<rtc::Producer*, std::unordered_set<rtc::Consumer*>> mapProducerConsumers;
		std::unordered_map<rtc::Consumer*, rtc::Producer*> mapConsumerProducer;
		std::unordered_map<rtc::Producer*, std::unordered_set<rtc::RtpObserver*>> mapProducerRtpObservers;
		std::unordered_map<std::string, rtc::Producer*> mapProducers;
		std::unordered_map<rtc::DataProducer*, std::unordered_set<rtc::DataConsumer*>> mapDataProducerDataConsumers;
		std::unordered_map<rtc::DataConsumer*, rtc::DataProducer*> mapDataConsumerDataProducer;
		std::unordered_map<std::string, rtc::DataProducer*> mapDataProducers;
	};
} // namespace rtc

#endif
