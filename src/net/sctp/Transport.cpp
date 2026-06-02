

#include "Transport.h"
#include "base/logger.h"
#include "base/error.h"
#include "UtilStun.h"
#include <iterator>                                              // std::ostream_iterator
#include <map>                                                   // std::multimap
#include <sstream>                                               // std::ostringstream

namespace rtc
{
	/* Instance methods. */

	Transport::Transport(const std::string& id, int agentNo, const Configuration &config, Listener* listener)
	  : id(id), iceListener(listener), agentNo(agentNo), config(config)
	{
            
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
//			  new rtc::SctpTransport(this, os, mis, maxSctpMessageSize, isDataChannel);
//                        
//                        SInfo << "sctpAssociation " <<  sctpAssociation;
//		}
            
            
//                rtc::SctpTransport::Ports ports = {};
//		ports.local = 3868;
//		ports.remote = 3868;

		//sctptransport = new rtc::SctpTransport(  this, agentNo , config, ports );
                
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
                SInfo << "~Transport delete sctp "  << sctptransport ; 
                
                
                delete sctptransport;

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
        
        rtc::SctpTransport*  Transport::Connected(rtc::SctpTransport::Ports &ports)
	{
            
            sctptransport = new rtc::SctpTransport(  this, agentNo , config, ports );
        
           // mSctpTransport = std::make_shared<rtc::SctpTransport*>(transport->agent.socket->sctptransport );
            
            sctptransport->start();
             
            return sctptransport;
        }

	void Transport::Connected()
	{
            
            
            rtc::SctpTransport::Ports ports = {};
	    ports.local = 3868;
	    ports.remote = 3868;

	    sctptransport = new rtc::SctpTransport(  this, agentNo , config, ports );
                
                    
            SInfo << "sctptransport->start";
            sctptransport->start();
	
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
		 SInfo << "~Disconnected "  << sctptransport ; 
                 
                 
               if(sctptransport)
               {
                   //sctptransport->incoming(make_message(data, data + len));
                   // TBD
               }
               else
                 iceListener->OnClose();
                 
//				// Tell all DataConsumers.
//		for (auto& kv : this->mapDataConsumers)
//		{
//			auto* dataConsumer = kv.second;
//
//			dataConsumer->TransportDisconnected();
//		}


	}


	void Transport::ReceiveSctpData(byte * data, size_t len)
	{   
// 		
                 SDebug << "AgentNo " << agentNo << " ReceiveSctpData: " << len;

//
//		if (!this->sctpAssociation)
//		{
//			MS_DEBUG_TAG(sctp, "ignoring SCTP packet (SCTP not enabled)");
//
//			return;
//		}
//
//		// Pass it to the SctpTransport.
                if(sctptransport)
		   sctptransport->incoming(make_message(data, data + len));
               else
                 iceListener->OnReceiveData(data, len);
		
	}



	



	inline void Transport::OnDataProducerSctpMessageReceived(
	  rtc::DataProducer* dataProducer, uint32_t ppid, const uint8_t* msg, size_t len)
	{
            
             SInfo << "OnDataProducerSctpMessageReceived";
	
//                SInfo << "OnDataProducerSctpMessageReceived dataProducer " <<  dataProducer->id  << " sctpAssociation "  << sctpAssociation;
//
		//this->iceListener->OnTransportDataProducerSctpMessageReceived(this, dataProducer, ppid, msg, len);
	}

	inline void Transport::OnDataConsumerSendSctpMessage(
	  rtc::DataConsumer* dataConsumer, uint32_t ppid, const uint8_t* msg, size_t len)
	{

                 SInfo << " OnDataProducerSctpMessageReceived dataConsumer " <<  dataConsumer->id << " sctptransport "  << sctptransport;
                 
		//this->sctpAssociation->SendSctpMessage(dataConsumer, ppid, msg, len);
	}


	inline void Transport::OnSctpTransportConnecting(rtc::SctpTransport* sctpAssociation)
	{
	
              //iceListener->OnDtlsTransportStatus(sctpAssociation);
            //iceListener->OnSctpTransportClosed()
		// Notify the Node Transport.
		//json data = json::object();
//
//		data["sctpState"] = "connecting";
//
//		Channel::Notifier::Emit(this->id, "sctpstatechange ", data);
	}

	inline void Transport::OnSctpTransportConnected(rtc::SctpTransport* sctpAssociation)
	{
		
            SInfo << "OnSctpTransportConnected";
          //   iceListener->OnDtlsTransportStatus(sctpAssociation);
                    
            //const uint8_t data[] ="arvind"; 
            
           // SInfo << "data " << data <<  " len " << sizeof(data);

          //  SendSctpData(data, sizeof(data));

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

	inline void Transport::OnSctpTransportFailed(rtc::SctpTransport* /*sctpAssociation*/)
	{
            SInfo << "OnSctpTransportFailed";

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

	inline void Transport::OnSctpTransportClosed(rtc::SctpTransport* /*sctpAssociation*/)
	{
             SInfo << "OnSctpTransportClosed";
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
	  rtc::SctpTransport* /*sctpAssociation*/, const uint8_t* data, size_t len)
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
		//if (this->sctpAssociation)
		SendSctpData(data, len);
	}

//	inline void Transport::OnSctpTransportMessageReceived(
//	  rtc::SctpTransport* /*sctpAssociation*/,
//	  uint16_t streamId,
//	  uint32_t ppid,
//	  const uint8_t* msg,
//	  size_t len)
//	{
//                SInfo << "OnSctpTransportMessageReceived " ;
////		
////
////		rtc::DataProducer* dataProducer = this->sctpListener.GetDataProducer(sctpAssociation);
////
////		if (!dataProducer)
////		{
////			MS_WARN_TAG(
////			  sctp, "no suitable DataProducer for received SCTP message [streamId:%" PRIu16 "]", streamId);
////
////			return;
////		}
////
////		// Pass the SCTP message to the corresponding DataProducer.
////		dataProducer->ReceiveSctpMessage(ppid, msg, len);
//	}

	void  Transport::OnSctpTransportMessageReceived(SctpTransport* sctpAssociation ,message_ptr message )
        {
            ///SInfo << "OnSctpTransportMessageReceived" << message->data() << " len " <<  message->size();
            iceListener->OnSctpTransportMessageReceived( sctpAssociation, message );
        }

        void Transport::OnSctpState(SctpTransport::State state)
        {
            
            iceListener->OnSctpState(state);
        }
        
	inline void Transport::OnTimer(Timer* timer)
	{
		

//		// RTCP timer.
//		if (timer == this->rtcpTimer)
//		{
//			auto interval  = static_cast<uint64_t>(rtc::RTCP::MaxVideoIntervalMs);
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
//				if (interval > rtc::RTCP::MaxVideoIntervalMs)
//					interval = rtc::RTCP::MaxVideoIntervalMs;
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
} // namespace rtc
