/*
A Worker represents a  C++ subprocess that runs in a single CPU core. It can handle many routers.
A Router holds producers and consumers that exchange audio/video RTP between them. In certain common usages, a router can be understood as a “multi-party conference room”.
Since a router belongs to a worker, a router uses a single CPU (and may share it with other routers in the same worker).
A router behaves as an SFU (Selective Forwarding Unit). This is:
it forwards RTP packets between producers and consumers,
it selects which spatial and temporal layers to forward based on consumer settings and network capability,
it requests RTP packet retransmission to producer endpoints when there is packet loss,
it holds a buffer with packets from producer endpoints and retransmits them to consumer endpoints when requested by those,
however it does neither decode nor transcode media packets, so it can not generate video key frames on demand, but just requests them to the producer endpoints.
 
 Depending on the host CPU capabilities, a worker C++ subprocess can tipically handle over ~500 consumers in total. If for example there are 4 peers in a room, all them sending audio and video and all them consuming the audio and video of the other peers, this would mean that:
Each peer receives audio and video from 3 peers, so 3x2 = 6 consumers in total.
There are 4 peers, so 4x6 = 24 consumers in total.

 */


#include "Router.h"
#include "base/logger.h"
#include "base/error.h"
#include "base/platform.h"
//#include "Utils.h"

#include "WebRtcTransport.h"

using namespace base;

namespace rtc
{
	/* Instance methods. */

	Router::Router(const std::string& id, int agentNo) : id(id), agentNo(agentNo)
	{
	
	}

	Router::~Router()
	{


		// Close all Transports.
	
	}


	void Router::HandleRequest(bool server, const Configuration &config, int localPort, int remotePort , std::string localIP, std::string remoteIp , CertificateFingerprint &dtlsRemoteFingerprint )
	{

            auto* webRtcTransport = new rtc::WebRtcTransport(id ,agentNo, config, this, localPort,  remotePort ,  localIP, remoteIp);
            webRtcTransport->HandleRequest(server , dtlsRemoteFingerprint);
            
            mapTransports[id] = webRtcTransport;
        }
        
        void Router::OnDtlsTransportStatus(DtlsTransport::DtlsState state) 
        {
      
            if (state == DtlsTransport::DtlsState::CONNECTED)
            {
               char tmp[256];
               sprintf(tmp, "%darvind", count++ );
               mapTransports[id]->SendSctpData((const uint8_t *)tmp, 7);
               
            }    
        }
        
        void Router::OnReceiveData(byte * data, size_t len)
        {
            SInfo << "AgentNo " << agentNo << " " << (char*) data;  
            
            sleep(5);
            
           {
              const uint8_t tmp[256] ="arvind";
              sprintf((char*)tmp, "%darvind", count++ );
              mapTransports[id]->SendSctpData(tmp, 7);
           }   
             
            
        }

        void Router::OnSctpState(SctpTransport::State state)
        {
            
        }
        
        void Router::OnSctpTransportMessageReceived(SctpTransport* sctpAssociation ,message_ptr message )
        {
            
        }
        
        void Router::Close( )
        {
    
            
           	for (auto& kv : this->mapTransports)
		{
			auto* transport = kv.second;

			delete transport;
		}
		this->mapTransports.clear();

		// Close all RtpObservers.
	
		this->mapDataProducers.clear();
        }



} // namespace rtc
