#ifndef MS_RTC_TRANSPORT_HPP
#define MS_RTC_TRANSPORT_HPP
//#define ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR

#include "common.h"
#include "base/application.h"
#include "sctptransport.hpp"
#include "DtlsTransport.h"
#include "DataConsumer.h"
#include "DataProducer.h"

#include "RTC/Consumer.h"
#include "RTC/DataConsumer.h"
#include "RTC/DataProducer.h"
#include "RTC/Producer.h"
#include "RTC/RTCP/CompoundPacket.h"
#include "RTC/RTCP/Packet.h"
#include "RTC/RTCP/ReceiverReport.h"
#include "RTC/RateCalculator.h"
#include "RTC/RtpHeaderExtensionIds.h"
#include "RTC/RtpListener.h"
#include "RTC/RtpPacket.h"
//#include "RTC/SctpAssociation.h"
//#include "RTC/SctpListener.h"
#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
#include "RTC/SenderBandwidthEstimator.h"
#endif
//#include "RTC/TransportCongestionControlClient.h"
//#include "RTC/TransportCongestionControlServer.h"
#include "base/Timer.h"
#include <string>
#include <unordered_map>
//#include "track.hpp"


#if 1

    
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <map>

#endif


#define SRTP 1

using namespace base;

using json = nlohmann::json;

namespace rtc
{

#if 1
class SafeH264RtpDumper {
private:
    std::ofstream outFile;
    const uint8_t annexBStartCode[4] = {0x00, 0x00, 0x00, 0x01};
    
    // Sequence tracking variables
    uint16_t nextExpectedSequence{0};
    bool isFirstPacket{true};

    // Buffer to hold out-of-order packets (sorted automatically by sequence number)
    std::map<uint16_t, std::vector<uint8_t>> packetBufferMap;
    const size_t MAX_BUFFER_SIZE = 30; // Max packets to hold while waiting for missing ones

public:
    SafeH264RtpDumper(const std::string& filename) {
        outFile.open(filename, std::ios::binary | std::ios::out);
    }

    ~SafeH264RtpDumper() {
        // Flush remaining buffered packets before closing
        FlushRemainingBuffer();
        if (outFile.is_open()) outFile.close();
    }

    // Call this whenever a network packet arrives from mediasoup
    void OnRtpPacketReceived(const uint8_t* packetBuffer, size_t packetLength) {
        if (packetLength < 12) return;

        // Extract 16-bit RTP Sequence Number (Bytes 2 and 3 of RTP header)
        uint16_t sequenceNumber = (packetBuffer[2] << 8) | packetBuffer[3];

        // Store copy of the packet in our sorting map
        packetBufferMap[sequenceNumber] = std::vector<uint8_t>(packetBuffer, packetBuffer + packetLength);

        // Initialize sequence tracking on the very first packet
        if (isFirstPacket) {
            nextExpectedSequence = sequenceNumber;
            isFirstPacket = false;
        }

        // Process buffered packets that are ready and in the correct order
        ProcessOrderedQueue();
    }

private:
    void ProcessOrderedQueue() {
        while (!packetBufferMap.empty()) {
            auto it = packetBufferMap.begin();
            uint16_t currentSeq = it->first;

            // Scenario A: The packet we were waiting for has arrived
            if (currentSeq == nextExpectedSequence) {
                DecodeAndWritePayload(it->second.data(), it->second.size());
                nextExpectedSequence++;
                packetBufferMap.erase(it);
            }
            // Scenario B: Packet is from the past (duplicate or very late arrival), discard it
            else if (IsSequenceOlder(currentSeq, nextExpectedSequence)) {
                packetBufferMap.erase(it);
            }
            // Scenario C: There is a gap (currentSeq > nextExpectedSequence)
            else {
                // If our sorting buffer is full, we must give up on the missing packet(s)
                if (packetBufferMap.size() > MAX_BUFFER_SIZE) {
                    std::clog << "[Warning] Missing packets detected between seq " 
                              << nextExpectedSequence << " and " << currentSeq << ". Skipping gap." << std::endl;
                    
                    // Advance our timeline to the oldest packet currently sitting in the buffer
                    nextExpectedSequence = currentSeq;
                    continue; // Loop again to process it
                }
                // Buffer is not full yet; stop and wait for the missing packet to arrive
                break; 
            }
        }
    }

    // Accounts for 16-bit sequence number rollover wrapping around at 65535
    bool IsSequenceOlder(uint16_t current, uint16_t expected) {
        return (current != expected) && ((uint16_t)(expected - current) < 32768);
    }

    void FlushRemainingBuffer() {
        while (!packetBufferMap.empty()) {
            auto it = packetBufferMap.begin();
            DecodeAndWritePayload(it->second.data(), it->second.size());
            packetBufferMap.erase(it);
        }
    }

    // Contains the RFC 6184 H.264 parsing logic from the previous solution
    void DecodeAndWritePayload(const uint8_t* packetBuffer, size_t packetLength) {
        size_t headerLength = 12 + ((packetBuffer[0] & 0x0F) * 4);
        if ((packetBuffer[0] >> 4 & 0x01)) { // Extension header check
            size_t extIndex = headerLength;
            uint16_t extLength = (packetBuffer[extIndex + 2] << 8) | packetBuffer[extIndex + 3];
            headerLength += 4 + (extLength * 4);
        }

        if (packetLength <= headerLength) return;

        const uint8_t* payload = packetBuffer + headerLength;
        size_t payloadLength = packetLength - headerLength;

        if (packetBuffer[0] >> 5 & 0x01) { // Padding check
            uint8_t paddingLength = packetBuffer[packetLength - 1];
            if (paddingLength < payloadLength) payloadLength -= paddingLength;
        }

        if (payloadLength < 2) return;
        uint8_t nalType = payload[0] & 0x1F;

        if (nalType >= 1 && nalType <= 23) { // Single NAL
            WriteToFile(payload, payloadLength);
        } 
        else if (nalType == 28) { // FU-A Fragment
            bool isStart = payload[1] & 0x80;
            if (isStart) {
                uint8_t reconNalHeader = (payload[0] & 0xE0) | (payload[1] & 0x1F);
                outFile.write(reinterpret_cast<const char*>(annexBStartCode), 4);
                outFile.write(reinterpret_cast<const char*>(&reconNalHeader), 1);
            }
            if (payloadLength > 2) {
                outFile.write(reinterpret_cast<const char*>(payload + 2), payloadLength - 2);
            }
        } 
        else if (nalType == 24) { // STAP-A Aggregation
            size_t idx = 1;
            while (idx < payloadLength - 2) {
                uint16_t nalSize = (payload[idx] << 8) | payload[idx + 1];
                idx += 2;
                if (idx + nalSize <= payloadLength) {
                    WriteToFile(payload + idx, nalSize);
                }
                idx += nalSize;
            }
        }
    }

    void WriteToFile(const uint8_t* data, size_t length) {
        outFile.write(reinterpret_cast<const char*>(annexBStartCode), 4);
        outFile.write(reinterpret_cast<const char*>(data), length);
    }
};

    

#endif  
    
    
    
    
    
    
	class Transport : 
	                public SctpTransport::Listener,
//                        public Track::Listener,
	                public Timer::Listener
	{
	protected:
		using onSendCallback = const std::function<void(bool sent)>;

	public:
		class Listener
		{
		public:
		
                    virtual void OnDtlsTransportStatus(std::string id, DtlsTransport::DtlsState state) = 0 ;
                    virtual void OnSctpState(std::string id, SctpTransport::State state)= 0 ;
                    virtual void OnSctpTransportMessageReceived(std::string id, SctpTransport* sctpAssociation ,message_ptr message )= 0 ;
                    virtual void OnReceiveData(std::string id, byte * data, size_t len) = 0 ;
                    virtual void OnClose(std::string id) = 0;
                    //virtual void OnTransportDataProducerSctpMessageReceived(  rtc::Transport* transport,  rtc::DataProducer* dataProducer,	  uint32_t ppid,  const uint8_t* msg,   size_t len) = 0;
		};

	private:
		struct TraceEventTypes
		{
			bool probation{ false };
			bool bwe{ false };
		};
                
              

	public:

               const Configuration &config;
		Transport(const std::string& id, int agentNo, const Configuration &config, Listener* listener);
		virtual ~Transport();

	public:
		virtual void HandleRequest();
                
                #if 1
                SafeH264RtpDumper h246dump{"test.264"};
		#endif

	public:
		// Must be called from the subclass.
            
                rtc::SctpTransport*  Connected(rtc::SctpTransport::Ports &port);
		void Connected();
		void Disconnected();
		void DataReceived(size_t len);
		void DataSent(size_t len);

		void ReceiveSctpData(byte* data, size_t len);

	private:
//		void SetNewProducerIdFromRequest(Channel::Request* request, std::string& producerId) const;
//		rtc::Producer* GetProducerFromRequest(Channel::Request* request) const;
//		void SetNewConsumerIdFromRequest(Channel::Request* request, std::string& consumerId) const;
//		rtc::Consumer* GetConsumerFromRequest(Channel::Request* request) const;

//		void SetNewDataProducerIdFromRequest(Channel::Request* request, std::string& dataProducerId) const;
//		rtc::DataProducer* GetDataProducerFromRequest(Channel::Request* request) const;
//		void SetNewDataConsumerIdFromRequest(Channel::Request* request, std::string& dataConsumerId) const;
//		rtc::DataConsumer* GetDataConsumerFromRequest(Channel::Request* request) const;
//		virtual bool IsConnected() const                                                 = 0;

//		void DistributeAvailableOutgoingBitrate();
//		void ComputeOutgoingDesiredBitrate(bool forceBitrate = false);
//		void EmitTraceEventProbationType(rtc::RtpPacket* packet) const;
//		void EmitTraceEventBweType(rtc::TransportCongestionControlClient::Bitrates& bitrates) const;

		/* Pure virtual methods inherited from rtc::Producer::Listener. */
	public:
                virtual void SendSctpData(const uint8_t* data, size_t len)             = 0;
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
	public:
//	
	public:
//		void OnTransportCongestionControlServerSendRtcpPacket(
//		  rtc::TransportCongestionControlServer* tccServer, rtc::RTCP::Packet* packet) override;

#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
		/* Pure virtual methods inherited from rtc::SenderBandwidthEstimator::Listener. */
	public:
//		void OnSenderBandwidthEstimatorAvailableBitrate(
//		  rtc::SenderBandwidthEstimator* senderBwe,
//		  uint32_t availableBitrate,
//		  uint32_t previousAvailableBitrate) override;
#endif

		/* Pure virtual methods inherited from Timer::Listener. */
	public:
		void OnTimer(Timer* timer) override;

	public:
		// Passed by argument.
		const std::string id;
              
                Listener* iceListener{ nullptr };
                 int agentNo{0};
                rtc::SctpTransport* sctptransport{ nullptr };

	private:
            
            
            
		// Passed by argument.
		// Allocated by this.
		std::unordered_map<std::string, rtc::Producer*> mapProducers;
		std::unordered_map<std::string, rtc::Consumer*> mapConsumers;
//		std::unordered_map<std::string, rtc::DataProducer*> mapDataProducers;
//		std::unordered_map<std::string, rtc::DataConsumer*> mapDataConsumers;
		std::unordered_map<uint32_t, rtc::Consumer*> mapSsrcConsumer;
		std::unordered_map<uint32_t, rtc::Consumer*> mapRtxSsrcConsumer;
//		rtc::SctpAssociation* sctpAssociation{ nullptr };
		Timer* rtcpTimer{ nullptr };
//		rtc::TransportCongestionControlClient* tccClient{ nullptr };
//		rtc::TransportCongestionControlServer* tccServer{ nullptr };
#ifdef ENABLE_RTC_SENDER_BANDWIDTH_ESTIMATOR
		rtc::SenderBandwidthEstimator* senderBwe{ nullptr };
#endif
		// Others.
		bool destroying{ false };
		struct rtc::RtpHeaderExtensionIds recvRtpHeaderExtensionIds;
		rtc::RtpListener rtpListener;
//		rtc::SctpListener sctpListener;
		rtc::RateCalculator recvTransmission;
		rtc::RateCalculator sendTransmission;
		rtc::RtpDataCounter recvRtpTransmission;
		rtc::RtpDataCounter sendRtpTransmission;
		rtc::RtpDataCounter recvRtxTransmission;
		rtc::RtpDataCounter sendRtxTransmission;
		rtc::RtpDataCounter sendProbationTransmission;
		uint16_t transportWideCcSeq{ 0u };
		uint32_t initialAvailableOutgoingBitrate{ 600000u };
		uint32_t maxIncomingBitrate{ 0u };
		struct TraceEventTypes traceEventTypes;
                
                
                #if SRTP

                public:
                void ReceiveRtpPacket(rtc::RtpPacket* packet);
		void ReceiveRtcpPacket(rtc::RTCP::Packet* packet);
                		virtual void SendRtpPacket(rtc::RtpPacket* packet, onSendCallback cb = nullptr) = 0;
		void HandleRtcpPacket(rtc::RTCP::Packet* packet);
		void SendRtcp(uint64_t nowMs);
		virtual void SendRtcpPacket(rtc::RTCP::Packet* packet)                 = 0;
		virtual void SendRtcpCompoundPacket(rtc::RTCP::CompoundPacket* packet) = 0;
                
                rtc::Consumer* GetConsumerByMediaSsrc(uint32_t ssrc) const;
		rtc::Consumer* GetConsumerByRtxSsrc(uint32_t ssrc) const;
                
                
                /*
		void SendRtcpPacket(rtc::RTCP::Packet* packet){} ;
		void SendRtcpCompoundPacket(rtc::RTCP::CompoundPacket* packet) {};
                void SendRtpPacket(rtc::RtpPacket* packet, Track::onSendCallback cb = nullptr) {};
		*/                
                
                #endif
                
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
