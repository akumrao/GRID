



#include "WebRtcTransport.h"
#include "base/logger.h"
#include "base/error.h"
#include "net/IP.h"
//#include "Utils.h"
//#include "Channel/Notifier.h"
#include <cmath> // std::pow()

using namespace base;
namespace rtc
{
	/* Static. */

	static constexpr uint16_t IceCandidateDefaultLocalPriority{ 10000 };
	// We just provide "host" candidates so type preference is fixed.
	static constexpr uint16_t IceTypePreference{ 64 };
	// We do not support non rtcp-mux so component is always 1.
	static constexpr uint16_t IceComponent{ 1 };

	static inline uint32_t generateIceCandidatePriority(uint16_t localPreference)
	{
		return std::pow(2, 24) * IceTypePreference + std::pow(2, 8) * localPreference +
		       std::pow(2, 0) * (256 - IceComponent);
	}

	/* Instance methods. */

	WebRtcTransport::WebRtcTransport(const std::string& id, const Configuration &config, rtc::Transport::Listener* listener, int localPort, int remotePort )
	  : rtc::Transport::Transport(id, config, listener)
	{
		
            uint16_t iceLocalPreferenceDecrement{ 0 };
            
            std::vector<ListenIp> listenIps;
            listenIps.resize(1);
            listenIps[0].announcedIp = "0.0.0.0";
            listenIps[0].ip = "127.0.0.1";
            listenIps[0].port = localPort;
            
            bool enableUdp{ true };
            bool enableTcp{ false };    
            
            bool preferUdp{ false };
            bool preferTcp{ false };
            
            iceServer = new rtc::IceServer();
            
            for (auto& listenIp : listenIps)
            {
                    if (enableUdp)
                    {
                            uint16_t iceLocalPreference =
                              IceCandidateDefaultLocalPriority - iceLocalPreferenceDecrement;

                            if (preferUdp)
                                    iceLocalPreference += 1000;

                            uint32_t icePriority = generateIceCandidatePriority(iceLocalPreference);

                            // This may throw.
                            auto* udpSocket = new base::net::UdpServer(this, listenIp.ip,  listenIp.port );
                            
                            udpSocket->bind();
                            

                            this->udpSockets[udpSocket] = listenIp.announcedIp;
                            
                            
                             addr_record_t mapped;
                            
                             IP::StringToAddress(listenIps[0].ip.data() , remotePort,  mapped);
                             
                            tuple = new TransportTuple(
                            udpSocket, reinterpret_cast<struct sockaddr*>(&mapped.addr)  );

                            tuple = iceServer->AddTuple(tuple);
                                   
                            iceServer->SetSelectedTuple(tuple) ;
                           
                    }

                    if (enableTcp)
                    {
                            uint16_t iceLocalPreference =
                              IceCandidateDefaultLocalPriority - iceLocalPreferenceDecrement;

                            if (preferTcp)
                                    iceLocalPreference += 1000;

                            uint32_t icePriority = generateIceCandidatePriority(iceLocalPreference);

                            // This may throw.
                            auto* tcpServer = new base::net::TcpServer(this, listenIp.ip,  listenIp.port);

                            this->tcpServers[tcpServer] = listenIp.announcedIp;

                    }

                    // Decrement initial ICE local preference for next IP.
                    iceLocalPreferenceDecrement += 100;
            }



	    // Create a DTLS transport.
            STrace << " Create a DTLS transport.";
			this->dtlsTransport = new rtc::DtlsTransport(this);
	}

	WebRtcTransport::~WebRtcTransport()
	{
            delete iceServer;

            // Must delete the DTLS transport first since it will generate a DTLS alert
            // to be sent.
            delete this->dtlsTransport;
            this->dtlsTransport = nullptr;


            for (auto& kv : this->udpSockets)
            {
                    auto* udpSocket = kv.first;

                    delete udpSocket;
            }
            this->udpSockets.clear();

            for (auto& kv : this->tcpServers)
            {
                    auto* tcpServer = kv.first;

                    delete tcpServer;
            }
            this->tcpServers.clear();

		
	}

	



	void WebRtcTransport::HandleRequest(bool server)
	{
		
            if(server)
                dtlsRole = rtc::DtlsTransport::Role::SERVER;
            else
                dtlsRole = rtc::DtlsTransport::Role::CLIENT;
            
             MayRunDtlsTransport();


//            dtlsRemoteRole = rtc::DtlsTransport::Role::AUTO;
//
//              // Set local DTLS role.
//            switch (dtlsRemoteRole)
//            {
//                    case rtc::DtlsTransport::Role::CLIENT:
//                    {
//                            this->dtlsRole = rtc::DtlsTransport::Role::SERVER;
//
//                            break;
//                    }
//                    case rtc::DtlsTransport::Role::SERVER:
//                    {
//                            this->dtlsRole = rtc::DtlsTransport::Role::CLIENT;
//
//                            break;
//                    }
//                    // If the peer has role "auto" we become "client" since we are ICE controlled.
//                    case rtc::DtlsTransport::Role::AUTO:
//                    {
//                            this->dtlsRole = rtc::DtlsTransport::Role::CLIENT;
//
//                            break;
//                    }
//                    case rtc::DtlsTransport::Role::NONE:
//                    {
//                            base::uv::throwError("invalid remote DTLS role");
//                    }
//            };
//
//            this->connectCalled = true;
//
//            // Pass the remote fingerprint to the DTLS transport.
//            if (this->dtlsTransport->SetRemoteFingerprint(dtlsRemoteFingerprint))
//            {
//                    // If everything is fine, we may run the DTLS transport if ready.
//                    MayRunDtlsTransport();
//            }
//
//
//
//            switch (this->dtlsRole)
//            {
//                    case rtc::DtlsTransport::Role::CLIENT:
//                            data["dtlsLocalRole"] = "client";
//                            break;
//
//                    case rtc::DtlsTransport::Role::SERVER:
//                            data["dtlsLocalRole"] = "server";
//                            break;
//
//                    default:
//                            MS_ABORT("invalid local DTLS role");
//            }


			
	}

	inline bool WebRtcTransport::IsConnected() const
	{
		

		assertm(this->dtlsTransport, "no dtlsTransport");

		
		return (
//			(
//				this->iceServer->GetState() == rtc::IceServer::IceState::CONNECTED ||
//				this->iceServer->GetState() == rtc::IceServer::IceState::COMPLETED
//			) &&
			this->dtlsTransport->GetState() == rtc::DtlsTransport::DtlsState::CONNECTED
		);
		
	}

	void WebRtcTransport::MayRunDtlsTransport()
	{
		

		assertm(this->dtlsTransport, "no dtlsTransport");

		// Do nothing if we have the same local DTLS role as the DTLS transport.
		// NOTE: local role in DTLS transport can be NONE, but not ours.
		if (this->dtlsTransport->GetLocalRole() == this->dtlsRole)
			return;

		// Check our local DTLS role.
		switch (this->dtlsRole)
		{
			// If still 'auto' then transition to 'server' if ICE is 'connected' or
			// 'completed'.
			case rtc::DtlsTransport::Role::AUTO:
			{
				
//				if (
//					this->iceServer->GetState() == rtc::IceServer::IceState::CONNECTED ||
//					this->iceServer->GetState() == rtc::IceServer::IceState::COMPLETED
//				)
				
				{
					LDebug( "transition from DTLS local role 'auto' to 'server' and running DTLS transport");

					this->dtlsRole = rtc::DtlsTransport::Role::SERVER;
					this->dtlsTransport->Run(rtc::DtlsTransport::Role::SERVER);
				}

				break;
			}

			// 'client' is only set if a 'connect' request was previously called with
			// remote DTLS role 'server'.
			//
			// If 'client' then wait for ICE to be 'completed' (got USE-CANDIDATE).
			//
			// NOTE: This is the theory, however let's be more flexible as told here:
			//   https://bugs.chromium.org/p/webrtc/issues/detail?id=3661
			case rtc::DtlsTransport::Role::CLIENT:
			{
				
//				if (
//					this->iceServer->GetState() == rtc::IceServer::IceState::CONNECTED ||
//					this->iceServer->GetState() == rtc::IceServer::IceState::COMPLETED
//				)
				
				{
					LTrace( "running DTLS transport in local role 'client'");

					this->dtlsTransport->Run(rtc::DtlsTransport::Role::CLIENT);
				}

				break;
			}

			// If 'server' then run the DTLS transport if ICE is 'connected' (not yet
			// USE-CANDIDATE) or 'completed'.
			case rtc::DtlsTransport::Role::SERVER:
			{
				
//				if (
//					this->iceServer->GetState() == rtc::IceServer::IceState::CONNECTED ||
//					this->iceServer->GetState() == rtc::IceServer::IceState::COMPLETED
//				)
//				
				{
					LTrace( "running DTLS transport in local role 'server'");

					this->dtlsTransport->Run(rtc::DtlsTransport::Role::SERVER);
				}

				break;
			}

			case rtc::DtlsTransport::Role::NONE:
			{
				MS_ABORT("local DTLS role not set");
			}
		}
	}





	void WebRtcTransport::SendSctpData(const uint8_t* data, size_t len)
	{
		

		
		if (!IsConnected())
		{
			SWarn <<  "DTLS not connected, cannot send SCTP data";

			return;
		}

		this->dtlsTransport->SendApplicationData(data, len);
	}

	inline void WebRtcTransport::OnPacketReceived(
	  base::net::TransportTuple* tuple, const char* data, size_t len)
	{
		
            SInfo << "OnPacketReceived" << len;

		assertm(this->dtlsTransport, "no dtlsTransport");

		// Increase receive transmission.
//		rtc::Transport::DataReceived(len);

		
		// Check if it's DTLS.
		if (rtc::DtlsTransport::IsDtls(data, len))
		{
			OnDtlsDataReceived(tuple, data, len);
		}
		else
		{
                    SInfo << "OnUdpSocketPacketReceived len: " << len << " data " << data;
                    
		    LWarn("ignoring received packet of unknown type");
		}
	}

//	inline void WebRtcTransport::OnStunDataReceived(
//	  base::net::TransportTuple* tuple, const uint8_t* data, size_t len)
//	{
//		
//
//
//
//		rtc::StunPacket* packet = rtc::StunPacket::Parse(data, len);
//
//		if (!packet)
//		{
//			LWarn("ignoring wrong STUN packet received");
//
//			return;
//		}
//
//		// Pass it to the IceServer.
//		this->iceServer->ProcessStunPacket(packet, tuple);
//
//		delete packet;
//	}

	inline void WebRtcTransport::OnDtlsDataReceived(
	  const base::net::TransportTuple* tuple, const char* data, size_t len)
	{
		
            SInfo << "OnDtlsDataReceived" << len;

            assertm(this->dtlsTransport, "no dtlsTransport");

		// Ensure it comes from a valid tuple.
		if (!this->iceServer->IsValidTuple(tuple))
		{
			LWarn( "ignoring DTLS data coming from an invalid tuple");

			return;
		}

		// Trick for clients performing aggressive ICE regardless we are ICE-Lite.
		this->iceServer->ForceSelectedTuple(tuple);

            // Check that DTLS status is 'connecting' or 'connected'.
            if (
              this->dtlsTransport->GetState() == rtc::DtlsTransport::DtlsState::CONNECTING ||
              this->dtlsTransport->GetState() == rtc::DtlsTransport::DtlsState::CONNECTED)
            {
                    //MS_DEBUG_DEV("DTLS data received, passing it to the DTLS transport");

                    this->dtlsTransport->ProcessDtlsData((const uint8_t*)data, len);
            }
            else
            {
                    LWarn( "Transport is not 'connecting' or 'connected', ignoring received DTLS data");

                    return;
            }
	}



	inline void WebRtcTransport::OnUdpSocketPacketReceived(
	  base::net::UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr)
	{
            SInfo << "OnUdpSocketPacketReceived len: " << len ;

            base::net::TransportTuple tuple(socket, remoteAddr);

            OnPacketReceived(&tuple, data, len);
	}

       inline void WebRtcTransport::on_close(base::net::Listener* conn)
	{
            SInfo << "on_close";

            net::TcpConnection* connection = (net::TcpConnection*) conn;

            base::net::TransportTuple tuple(connection);
//
		this->iceServer->RemoveTuple(&tuple);
	}


        void WebRtcTransport::on_read(base::net::Listener* conn, const char* data, size_t len) {

             SInfo << "on_read Len" <<  len;
             
//            rtc::TcpConnection* connection = (rtc::TcpConnection*) conn;
//            base::net::TransportTuple tuple(connection);
//
//            OnPacketReceived(&tuple, (const uint8_t*)data, len);   TBD

        }



	inline void WebRtcTransport::OnDtlsTransportConnecting(const rtc::DtlsTransport* /*dtlsTransport*/)
	{
		
                 SInfo << "OnDtlsTransportConnecting";

//		assertm(this->dtlsTransport, "no dtlsTransport");
//
//		LTrace( "DTLS connecting");
//
//		// Notify the Node WebRtcTransport.
//		json data = json::object();
//
//		data["dtlsState"] = "connecting";
//
//		Channel::Notifier::Emit(this->id, "dtlsstatechange", data);
	}

	inline void WebRtcTransport::OnDtlsTransportConnected( const rtc::DtlsTransport* dtlsTransport ) 
	{
		
                SInfo << "OnDtlsTransportConnected";
                
              //  const uint8_t tmp[7]="arvind";
               // OnDtlsTransportSendData( dtlsTransport, tmp, 7);
                
                
//		assertm(this->iceServer, "no iceServer");
//		assertm(this->dtlsTransport, "no dtlsTransport");
//
//		LTrace( "DTLS connected");
//
//		// Close it if it was already set and update it.
//		if (this->srtpSendSession)
//		{
//			delete this->srtpSendSession;
//			this->srtpSendSession = nullptr;
//		}
//		if (this->srtpRecvSession)
//		{
//			delete this->srtpRecvSession;
//			this->srtpRecvSession = nullptr;
//		}
//
//		try
//		{
//			this->srtpSendSession = new rtc::SrtpSession(
//			  rtc::SrtpSession::Type::OUTBOUND, srtpProfile, srtpLocalKey, srtpLocalKeyLen);
//		}
//		catch (const std::exception& error)
//		{
//			MS_ERROR("error creating SRTP sending session: %s", error.what());
//		}
//
//		try
//		{
//			this->srtpRecvSession = new rtc::SrtpSession(
//			  rtc::SrtpSession::Type::INBOUND, srtpProfile, srtpRemoteKey, srtpRemoteKeyLen);
//		}
//		catch (const std::exception& error)
//		{
//			MS_ERROR("error creating SRTP receiving session: %s", error.what());
//
//			delete this->srtpSendSession;
//			this->srtpSendSession = nullptr;
//		}
//
//		// Notify the Node WebRtcTransport.
//		json data = json::object();
//
//		data["dtlsState"]      = "connected";
//		data["dtlsRemoteCert"] = remoteCert;
//
//		Channel::Notifier::Emit(this->id, "dtlsstatechange", data);
//
//		// Tell the parent class.
		rtc::Transport::Connected();
	}

	inline void WebRtcTransport::OnDtlsTransportFailed(const rtc::DtlsTransport* /*dtlsTransport*/)
	{
		

		
		assertm(this->dtlsTransport, "no dtlsTransport");

		LWarn( "DTLS failed");



	
	}

	inline void WebRtcTransport::OnDtlsTransportClosed(const rtc::DtlsTransport* /*dtlsTransport*/)
	{
	
            
            assertm(this->dtlsTransport, "no dtlsTransport");
            LWarn( "DTLS remotely closed");



		// Tell the parent class.
	      rtc::Transport::Disconnected();
	}

	inline void WebRtcTransport::OnDtlsTransportSendData(
	  const rtc::DtlsTransport* /*dtlsTransport*/, const uint8_t* data, size_t len)
	{
		
            SInfo << "OnDtlsTransportSendData len:" << len;

            assertm(this->dtlsTransport, "no dtlsTransport");



		if (!this->iceServer->GetSelectedTuple())
		{
			LWarn( "no selected tuple set, cannot send DTLS packet");

			return;
		}

		this->iceServer->GetSelectedTuple()->Send(data, len);

            // Increase send transmission.
	    //  rtc::Transport::DataSent(len);
	}

	inline void WebRtcTransport::OnDtlsTransportApplicationDataReceived(
	  const rtc::DtlsTransport* /*dtlsTransport*/, const uint8_t* data, size_t len)
	{
		
            SInfo << "OnDtlsTransportApplicationDataReceived len:" << len;

            assertm(this->dtlsTransport, "no dtlsTransport");

            // Pass it to the parent transport.
            rtc::Transport::ReceiveSctpData((byte *)data, len);
	}
} // namespace rtc
