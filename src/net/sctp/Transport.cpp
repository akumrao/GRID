

#include "Transport.h"
#include "base/logger.h"
#include "base/error.h"
#include "Utils.h"
#include <iterator>                                              // std::ostream_iterator
#include <map>                                                   // std::multimap
#include <sstream>                                               // std::ostringstream

namespace RTC
{
	/* Instance methods. */

	Transport::Transport(const std::string& id, const Configuration &config, Listener* listener)
	  : id(id), listener(listener)
	{
            
            int x = 1;
//	
//
//		{
//			auto jsonNumSctpStreamsIt     = data.find("numSctpStreams");
//			auto jsonMaxSctpMessageSizeIt = data.find("maxSctpMessageSize");
//			auto jsonIsDataChannelIt      = data.find("isDataChannel");
//
//			// numSctpStreams is mandatory.
//			
//			if (
//				jsonNumSctpStreamsIt == data.end() ||
//				!jsonNumSctpStreamsIt->is_object()
//			)
//			
//			{
//				base::uv::throwError("wrong numSctpStreams (not an object)");
//			}
//
//			auto jsonOSIt  = jsonNumSctpStreamsIt->find("OS");
//			auto jsonMISIt = jsonNumSctpStreamsIt->find("MIS");
//
//			// numSctpStreams.OS and numSctpStreams.MIS are mandatory.
//			
//			if (
//				jsonOSIt == jsonNumSctpStreamsIt->end() ||
//				!Utils::Json::IsPositiveInteger(*jsonOSIt) ||
//				jsonMISIt == jsonNumSctpStreamsIt->end() ||
//				!Utils::Json::IsPositiveInteger(*jsonMISIt)
//			)
//			
//			{
//				base::uv::throwError("wrong numSctpStreams.OS and/or numSctpStreams.MIS (not a number)");
//			}
//
//			auto os  = jsonOSIt->get<uint16_t>();
//			auto mis = jsonMISIt->get<uint16_t>();
//
//			// maxSctpMessageSize is mandatory.
//			
//			if (
//				jsonMaxSctpMessageSizeIt == data.end() ||
//				!Utils::Json::IsPositiveInteger(*jsonMaxSctpMessageSizeIt)
//			)
//			
//			{
//				base::uv::throwError("wrong maxSctpMessageSize (not a number)");
//			}
//
//			auto maxSctpMessageSize = jsonMaxSctpMessageSizeIt->get<size_t>();
//
//			// isDataChannel is optional.
//			bool isDataChannel{ false };
//
//			if (jsonIsDataChannelIt != data.end() && jsonIsDataChannelIt->is_boolean())
//				isDataChannel = jsonIsDataChannelIt->get<bool>();
//
//			// This may throw.
//			this->sctpAssociation =
//			  new RTC::SctpTransport(this, os, mis, maxSctpMessageSize, isDataChannel);
//                        
//                        SInfo << "sctpAssociation " <<  sctpAssociation;
//		}
            
            
                RTC::SctpTransport::Ports ports = {};
		ports.local = 3868;
		ports.remote = 3868;

		//auto transport = std::make_shared<SctpTransport>(  this , config, ports );
                
		//    weak_bind(&PeerConnection::forwardBufferedAmount, this, _1, _2),
//		    [this, weak_this = weak_from_this()](SctpTransport::State transportState) {
//			    auto shared_this = weak_this.lock();
//			    if (!shared_this)
//				    return;
//
//			    switch (transportState) {
//			    case SctpTransport::State::Connected:
//				    changeState(State::Connected);
//				    assignDataChannels();
//				    mProcessor.enqueue(&PeerConnection::openDataChannels, shared_from_this());
//				    break;
//			    case SctpTransport::State::Failed:
//				    changeState(State::Failed);
//				    mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
//				    break;
//			    case SctpTransport::State::Disconnected:
//				    changeState(State::Disconnected);
//				    mProcessor.enqueue(&PeerConnection::remoteClose, shared_from_this());
//				    break;
//			    default:
//				    // Ignore
//				    break;
//			 
                                    
            

		// Create the RTCP timer.
//		this->rtcpTimer = new Timer(this);
	}

	Transport::~Transport()
	{
                SInfo << "~Transport delete sctp "  <<  sctpAssociation; 

		// Set the destroying flag.
		//this->destroying = true;

	

		// Delete all DataProducers.
//		for (auto& kv : this->mapDataProducers)
//		{
//			auto* dataProducer = kv.second;
//
//			delete dataProducer;
//		}
//		this->mapDataProducers.clear();

//		// Delete all DataConsumers.
//		for (auto& kv : this->mapDataConsumers)
//		{
//			auto* dataConsumer = kv.second;
//
//			delete dataConsumer;
//		}
//		this->mapDataConsumers.clear();
//
//		// Delete SCTP association.
//		delete this->sctpAssociation;
//		this->sctpAssociation = nullptr;

		// Delete the RTCP timer.
	
	}



	void Transport::HandleRequest()
	{
	
	}

	void Transport::Connected()
	{
	
//		// Tell all DataConsumers.
//		for (auto& kv : this->mapDataConsumers)
//		{
//			auto* dataConsumer = kv.second;
//
//			dataConsumer->TransportConnected();
//		}
//
//		// Tell the SctpTransport.
//		if (this->sctpAssociation)
//			this->sctpAssociation->TransportConnected();

	
	}

	void Transport::Disconnected()
	{
		
//				// Tell all DataConsumers.
//		for (auto& kv : this->mapDataConsumers)
//		{
//			auto* dataConsumer = kv.second;
//
//			dataConsumer->TransportDisconnected();
//		}


	}


	void Transport::ReceiveSctpData(const uint8_t* data, size_t len)
	{   
// 		
//                 SDebug << "ReceiveSctpData sctpAssociation:" << sctpAssociation;
//
//		if (!this->sctpAssociation)
//		{
//			MS_DEBUG_TAG(sctp, "ignoring SCTP packet (SCTP not enabled)");
//
//			return;
//		}
//
//		// Pass it to the SctpTransport.
//		this->sctpAssociation->ProcessSctpData(data, len);
	}



	



	inline void Transport::OnDataProducerSctpMessageReceived(
	  RTC::DataProducer* dataProducer, uint32_t ppid, const uint8_t* msg, size_t len)
	{
		
//                
//                SInfo << "OnDataProducerSctpMessageReceived dataProducer " <<  dataProducer->id  << " sctpAssociation "  << sctpAssociation;
//
//		this->listener->OnTransportDataProducerSctpMessageReceived(this, dataProducer, ppid, msg, len);
	}

	inline void Transport::OnDataConsumerSendSctpMessage(
	  RTC::DataConsumer* dataConsumer, uint32_t ppid, const uint8_t* msg, size_t len)
	{

                 SInfo << " OnDataProducerSctpMessageReceived dataConsumer " <<  dataConsumer->id << " sctpAssociation "  << sctpAssociation;
                 
		//this->sctpAssociation->SendSctpMessage(dataConsumer, ppid, msg, len);
	}


	inline void Transport::OnSctpTransportConnecting(RTC::SctpTransport* /*sctpAssociation*/)
	{
	

		// Notify the Node Transport.
		json data = json::object();
//
//		data["sctpState"] = "connecting";
//
//		Channel::Notifier::Emit(this->id, "sctpstatechange ", data);
	}

	inline void Transport::OnSctpTransportConnected(RTC::SctpTransport* /*sctpAssociation*/)
	{
		

//		// Tell all DataConsumers.
//		for (auto& kv : this->mapDataConsumers)
//		{
//			auto* dataConsumer = kv.second;
//
//			dataConsumer->SctpTransportConnected();
//		}
//
//		// Notify the Node Transport.
//		json data = json::object();
//
//		data["sctpState"] = "connected";
//
//		Channel::Notifier::Emit(this->id, "sctpstatechange", data);
	}

	inline void Transport::OnSctpTransportFailed(RTC::SctpTransport* /*sctpAssociation*/)
	{
	

//		// Tell all DataConsumers.
//		for (auto& kv : this->mapDataConsumers)
//		{
//			auto* dataConsumer = kv.second;
//
//			dataConsumer->SctpTransportClosed();
//		}
//
//		// Notify the Node Transport.
//		json data = json::object();
//
//		data["sctpState"] = "failed";
//
//		Channel::Notifier::Emit(this->id, "sctpstatechange", data);
	}

	inline void Transport::OnSctpTransportClosed(RTC::SctpTransport* /*sctpAssociation*/)
	{
	
//
//		// Tell all DataConsumers.
//		for (auto& kv : this->mapDataConsumers)
//		{
//			auto* dataConsumer = kv.second;
//
//			dataConsumer->SctpTransportClosed();
//		}
//
//		// Notify the Node Transport.
//		json data = json::object();
//
//		data["sctpState"] = "closed";
//
//		Channel::Notifier::Emit(this->id, "sctpstatechange", data);
	}

	inline void Transport::OnSctpTransportSendData(
	  RTC::SctpTransport* /*sctpAssociation*/, const uint8_t* data, size_t len)
	{
//		//
//                SDebug << "OnSctpTransportSendData sctpAssociation " << sctpAssociation;
//
//		// Ignore if destroying.
//		// NOTE: This is because when the child class (i.e. WebRtcTransport) is deleted,
//		// its destructor is called first and then the parent Transport's destructor,
//		// and we would end here calling SendSctpData() which is an abstract method.
//		if (this->destroying)
//			return;
//
//		if (this->sctpAssociation)
//			SendSctpData(data, len);
	}

	inline void Transport::OnSctpTransportMessageReceived(
	  RTC::SctpTransport* /*sctpAssociation*/,
	  uint16_t streamId,
	  uint32_t ppid,
	  const uint8_t* msg,
	  size_t len)
	{
                SInfo << "OnSctpTransportMessageReceived " ;
//		
//
//		RTC::DataProducer* dataProducer = this->sctpListener.GetDataProducer(sctpAssociation);
//
//		if (!dataProducer)
//		{
//			MS_WARN_TAG(
//			  sctp, "no suitable DataProducer for received SCTP message [streamId:%" PRIu16 "]", streamId);
//
//			return;
//		}
//
//		// Pass the SCTP message to the corresponding DataProducer.
//		dataProducer->ReceiveSctpMessage(ppid, msg, len);
	}

	


	inline void Transport::OnTimer(Timer* timer)
	{
		

//		// RTCP timer.
//		if (timer == this->rtcpTimer)
//		{
//			auto interval  = static_cast<uint64_t>(RTC::RTCP::MaxVideoIntervalMs);
//			uint64_t nowMs = base::Application::GetTimeMs();
//
//			SendRtcp(nowMs);
//
//			// Recalculate next RTCP interval.
//			if (!this->mapConsumers.empty())
//			{
//				// Transmission rate in kbps.
//				uint32_t rate{ 0 };
//
//				// Get the RTP sending rate.
//				for (auto& kv : this->mapConsumers)
//				{
//					auto* consumer = kv.second;
//
//					rate += consumer->GetTransmissionRate(nowMs) / 1000;
//				}
//
//				// Calculate bandwidth: 360 / transmission bandwidth in kbit/s.
//				if (rate != 0u)
//					interval = 360000 / rate;
//
//				if (interval > RTC::RTCP::MaxVideoIntervalMs)
//					interval = RTC::RTCP::MaxVideoIntervalMs;
//			}
//
//			/*
//			 * The interval between RTCP packets is varied randomly over the range
//			 * [0.5,1.5] times the calculated interval to avoid unintended synchronization
//			 * of all participants.
//			 */
//			interval *= static_cast<float>(Utils::Crypto::GetRandomUInt(5, 15)) / 10;
//
//			this->rtcpTimer->Start(interval);
//		}
	}
} // namespace RTC
