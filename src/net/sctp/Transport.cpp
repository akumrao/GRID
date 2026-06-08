


#include "Transport.h"
#include "base/logger.h"
#include "base/error.h"
#include "UtilStun.h"
#include <iterator>                                              // std::ostream_iterator
#include <map>                                                   // std::multimap
#include <sstream>                                               // std::ostringstream
#include "RTC/RTCP/FeedbackPsAfb.h"
#include "RTC/RTCP/FeedbackPsRemb.h"

#include "RTC/RtpProbationGenerator.h"
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
                 iceListener->OnClose(id);
                 
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
//			SDebug<<sctp, "ignoring SCTP packet (SCTP not enabled)");
//
//			return;
//		}
//
//		// Pass it to the SctpTransport.
                if(sctptransport)
		   sctptransport->incoming(make_message(data, data + len));
               else
                 iceListener->OnReceiveData(id, data, len);
		
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
            iceListener->OnSctpTransportMessageReceived( id, sctpAssociation, message );
        }

        void Transport::OnSctpState(SctpTransport::State state)
        {
            
            iceListener->OnSctpState(id, state);
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
        
#if SRTP
      
        void Transport::ReceiveRtcpPacket(rtc::RTCP::Packet* packet)
        {

            // Handle each RTCP packet.
            while (packet)
            {
                    HandleRtcpPacket(packet);

                    auto* previousPacket = packet;

                    packet = packet->GetNext();

                    delete previousPacket;
            }
        }
        
        	void Transport::HandleRtcpPacket(rtc::RTCP::Packet* packet)
	{
		

		switch (packet->GetType())
		{
			case rtc::RTCP::Type::RR:
			{
				auto* rr = static_cast<rtc::RTCP::ReceiverReportPacket*>(packet);

				for (auto it = rr->Begin(); it != rr->End(); ++it)
				{
					auto& report   = *it;
					auto* consumer = GetConsumerByMediaSsrc(report->GetSsrc());

					if (!consumer)
					{
                                            //TBD
//						// Special case for the RTP probator.
//						if (report->GetSsrc() == rtc::RtpProbationSsrc)
//						{
//							// TODO: We should pass the RR to the tccClient (and in fact just RR for the
//							// probation stream).
//
//							// TODO: Convert report to ReportBlock and pass to tccClient.
//							// RTCPReportBlock in libwebrtc/libwebrtc/modules/rtp_rtcp/include/rtp_rtcp_defines.h
//							//
//							// NOTE: consumer->GetRtt() is already implemented.
//							//
//							// NOTE: Better pass our RR to the tccClient and convert there to webrtc
//							// class.
//							//
//							// if (this->tccClient)
//							// {
//							// this->tccClient->ReceiveRtcpReceiverReport(report, consumer->GetRtt(),
//							// base::Application::GetTimeMs());
//							// }
//
//							continue;
//						}

					SDebug << "no Consumer found for received Receiver Report [ssrc: " <<  report->GetSsrc() << " [" ;

						continue;
					}

					consumer->ReceiveRtcpReceiverReport(report);
				}

				break;
			}

			case rtc::RTCP::Type::PSFB:
			{
				auto* feedback = static_cast<rtc::RTCP::FeedbackPsPacket*>(packet);

				switch (feedback->GetMessageType())
				{
					case rtc::RTCP::FeedbackPs::MessageType::PLI:
					{
						auto* consumer = GetConsumerByMediaSsrc(feedback->GetMediaSsrc());

						if (!consumer)
						{
                                                    SDebug <<   "no Consumer found for received PLI Feedback packet " <<  "[sender ssrc:" <<  feedback->GetMediaSsrc() << " media ssrc:" << feedback->GetMediaSsrc() << "]";

							break;
						}

                                                
                                                  SDebug <<   "PLI received, requesting key frame for Consumer " <<  "[sender ssrc:" <<  feedback->GetMediaSsrc() << " media ssrc:" << feedback->GetMediaSsrc() << "]";

						

						consumer->ReceiveKeyFrameRequest(
						  rtc::RTCP::FeedbackPs::MessageType::PLI, feedback->GetMediaSsrc());

						break;
					}

					case rtc::RTCP::FeedbackPs::MessageType::FIR:
					{
						// Must iterate FIR items.
						auto* fir = static_cast<rtc::RTCP::FeedbackPsFirPacket*>(packet);

						for (auto it = fir->Begin(); it != fir->End(); ++it)
						{
							auto& item     = *it;
							auto* consumer = GetConsumerByMediaSsrc(item->GetSsrc());

							if (!consumer)
							{
								SDebug<< "no Consumer found for received FIR Feedback packet " << "[sender ssrc:"  <<  feedback->GetMediaSsrc() <<  " media ssrc:%"  <<  feedback->GetMediaSsrc() <<  ", item ssrc:" <<  item->GetSsrc()<<  "]";

								continue;
							}

							SDebug<< "FIR received, requesting key frame for Consumer " <<   "[sender ssrc: " <<  feedback->GetMediaSsrc() << " media ssrc:" <<   feedback->GetMediaSsrc() <<  " item ssrc: "  << item->GetSsrc() <<  "]",

							consumer->ReceiveKeyFrameRequest(feedback->GetMessageType(), item->GetSsrc());
						}

						break;
					}

					case rtc::RTCP::FeedbackPs::MessageType::AFB:
					{
						auto* afb = static_cast<rtc::RTCP::FeedbackPsAfbPacket*>(feedback);

						// Store REMB info.
						if (afb->GetApplication() == rtc::RTCP::FeedbackPsAfbPacket::Application::REMB)
						{
							auto* remb = static_cast<rtc::RTCP::FeedbackPsRembPacket*>(afb);
                                                        //TBD

//							// Pass it to the TCC client.
//							if (this->tccClient)
//								this->tccClient->ReceiveEstimatedBitrate(remb->GetBitrate());

							break;
						}
						else
						{
                                                        SDebug<<  "ignoring unsupported " << rtc::RTCP::FeedbackPsPacket::MessageType2String(feedback->GetMessageType()).c_str() << "  Feedback packet " <<  "[sender ssrc: " <<  feedback->GetMediaSsrc() << ",  media ssrc:" <<   feedback->GetMediaSsrc();
					

							break;
						}
					}

					default:
					{
					
                                                
                                                SDebug<<  "ignoring unsupported " <<  rtc::RTCP::FeedbackPsPacket::MessageType2String(feedback->GetMessageType()).c_str() << "  Feedback packet " <<  "[sender ssrc: " <<  feedback->GetMediaSsrc() <<  ",  media ssrc:" <<   feedback->GetMediaSsrc();
					
					}
				}

				break;
			}

			case rtc::RTCP::Type::RTPFB:
			{
				auto* feedback = static_cast<rtc::RTCP::FeedbackRtpPacket*>(packet);
				auto* consumer = GetConsumerByMediaSsrc(feedback->GetMediaSsrc());

				// If no Consumer is found and this is not a Transport Feedback for the
				// probation SSRC or any Consumer RTX SSRC ignore it.
				//
				
				if (
					!consumer &&
					(
						(feedback->GetMessageType() != rtc::RTCP::FeedbackRtp::MessageType::TCC) &&
						(
						 feedback->GetMediaSsrc() != rtc::RtpProbationSsrc ||
						 !GetConsumerByRtxSsrc(feedback->GetMediaSsrc())
						)
					)
				)
				
				{
					SDebug<< "no Consumer found for received Feedback packet " <<   "[sender ssrc:" <<  feedback->GetMediaSsrc() <<  ", media ssrc:"  << feedback->GetMediaSsrc() <<  "]";
		
					break;
				}

				switch (feedback->GetMessageType())
				{
					case rtc::RTCP::FeedbackRtp::MessageType::NACK:
					{
						auto* nackPacket = static_cast<rtc::RTCP::FeedbackRtpNackPacket*>(packet);

						consumer->ReceiveNack(nackPacket);

						break;
					}

					case rtc::RTCP::FeedbackRtp::MessageType::TCC:
					{
						//auto* feedback = static_cast<rtc::RTCP::FeedbackRtpTransportPacket*>(packet);

                                                // TBD arvind
//						if (this->tccClient)
//							this->tccClient->ReceiveRtcpTransportFeedback(feedback);

#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
						// Pass it to the SenderBandwidthEstimator client.
						if (this->senderBwe)
							this->senderBwe->ReceiveRtcpTransportFeedback(feedback);
#endif

						break;
					}

					default:
					{
                                            SDebug<<  "ignoring unsupported " <<  rtc::RTCP::FeedbackRtpPacket::MessageType2String(feedback->GetMessageType()) << "  Feedback packet " <<  "[sender ssrc:% " <<  feedback->GetMediaSsrc() << ",  media ssrc: " <<   feedback->GetMediaSsrc();
						
					}
				}

				break;
			}

			case rtc::RTCP::Type::SR:
			{
				auto* sr = static_cast<rtc::RTCP::SenderReportPacket*>(packet);

				// Even if Sender Report packet can only contains one report.
				for (auto it = sr->Begin(); it != sr->End(); ++it)
				{
					auto& report   = *it;
					auto* producer = this->rtpListener.GetProducer(report->GetSsrc());

					if (!producer)
					{
						SDebug<<  "no Producer found for received Sender Report ssrc:" <<    report->GetSsrc();

						continue;
					}

					///producer->ReceiveRtcpSenderReport(report); TBD
				}

				break;
			}

			case rtc::RTCP::Type::SDES:
			{
				// According to RFC 3550 section 6.1 "a CNAME item MUST be included in
				// in each compound RTCP packet". So this is true even for compound
				// packets sent by endpoints that are not sending any RTP stream to us
				// (thus chunks in such a SDES will have an SSCR does not match with
				// any Producer created in this Transport).
				// Therefore, and given that we do nothing with SDES, just ignore them.

				break;
			}

			case rtc::RTCP::Type::BYE:
			{
				SDebug<< "ignoring received RTCP BYE";

				break;
			}

			case rtc::RTCP::Type::XR:
			{
				auto* xr = static_cast<rtc::RTCP::ExtendedReportPacket*>(packet);

				for (auto it = xr->Begin(); it != xr->End(); ++it)
				{
					auto& report = *it;

					switch (report->GetType())
					{
						case rtc::RTCP::ExtendedReportBlock::Type::DLRR:
						{
							auto* dlrr = static_cast<rtc::RTCP::DelaySinceLastRr*>(report);

							for (auto it2 = dlrr->Begin(); it2 != dlrr->End(); ++it2)
							{
								auto& ssrcInfo = *it2;

								// SSRC should be filled in the sub-block.
								if (ssrcInfo->GetSsrc() == 0)
									ssrcInfo->SetSsrc(xr->GetSsrc());

								auto* producer = this->rtpListener.GetProducer(ssrcInfo->GetSsrc());

								if (!producer)
								{
									SDebug << "no Producer found for received Sender Extended Report ssrc:%"  << ssrcInfo->GetSsrc();
									continue;
								}

								//producer->ReceiveRtcpXrDelaySinceLastRr(ssrcInfo); TBD
							}

							break;
						}

						default:;
					}
				}

				break;
			}

			default:
			{
				SDebug<<  "unhandled RTCP type received type:%" << static_cast<uint8_t>(packet->GetType());
			}
		}
	}

	void Transport::SendRtcp(uint64_t nowMs)
	{
		

		std::unique_ptr<rtc::RTCP::CompoundPacket> packet{ nullptr };

		for (auto& kv : this->mapConsumers)
		{
			auto* consumer = kv.second;

			for (auto* rtpStream : consumer->GetRtpStreams())
			{
				// Reset the Compound packet.
				packet.reset(new rtc::RTCP::CompoundPacket());

				consumer->GetRtcp(packet.get(), rtpStream, nowMs);

				// Send the RTCP compound packet if there is a sender report.
				if (packet->HasSenderReport())
				{
					packet->Serialize(rtc::RTCP::Buffer);
					SendRtcpCompoundPacket(packet.get());
				}
			}
		}

		// Reset the Compound packet.
		packet.reset(new rtc::RTCP::CompoundPacket());

		for (auto& kv : this->mapProducers)
		{
			auto* producer = kv.second;

//			producer->GetRtcp(packet.get(), nowMs);  //TBD

			// One more RR would exceed the MTU, send the compound packet now.
			if (packet->GetSize() + sizeof(RTCP::ReceiverReport::Header) > rtc::MtuSize)
			{
				packet->Serialize(rtc::RTCP::Buffer);
				SendRtcpCompoundPacket(packet.get());

				// Reset the Compound packet.
				packet.reset(new rtc::RTCP::CompoundPacket());
			}
		}

		if (packet->GetReceiverReportCount() != 0u)
		{
			packet->Serialize(rtc::RTCP::Buffer);
			SendRtcpCompoundPacket(packet.get());
		}
	}
        
        
        
	inline rtc::Consumer* Transport::GetConsumerByMediaSsrc(uint32_t ssrc) const
	{

		auto mapSsrcConsumerIt = this->mapSsrcConsumer.find(ssrc);

		if (mapSsrcConsumerIt == this->mapSsrcConsumer.end())
			return nullptr;

		auto* consumer = mapSsrcConsumerIt->second;

		return consumer;
	}

	inline rtc::Consumer* Transport::GetConsumerByRtxSsrc(uint32_t ssrc) const
	{

		auto mapRtxSsrcConsumerIt = this->mapRtxSsrcConsumer.find(ssrc);

		if (mapRtxSsrcConsumerIt == this->mapRtxSsrcConsumer.end())
			return nullptr;

		auto* consumer = mapRtxSsrcConsumerIt->second;

		return consumer;
	}
        
#endif
        
} // namespace rtc
