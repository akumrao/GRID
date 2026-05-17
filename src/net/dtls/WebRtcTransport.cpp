



#include "WebRtcTransport.h"
#include "base/logger.h"
#include "base/error.h"
#include "net/IP.h"
//#include "Utils.h"
//#include "Channel/Notifier.h"
#include <cmath> // std::pow()
#include <Agent.h>

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

        
       
        /*with stun*/
        WebRtcTransport::WebRtcTransport(const std::string& id, int agentNo,  const Configuration &config, rtc::Transport::Listener* listener, std::string IP, int port, Agent *agent)
            : rtc::Transport::Transport(id, agentNo, config, listener), IP(IP), port(port), agent(agent)
        {
    
            
            
                    m_udpServer = new base::net::UdpServer(this, IP,  port );

                    m_udpServer->bind();

                    STrace << "AgentNo " << agentNo << " Create a DTLS transport.";
                    this->dtlsTransport = new rtc::DtlsTransport(this);
                    iceServer = new rtc::IceServer();

                   // this->udpSockets[udpSocket] = "listenIp.announcedIp";


                   /*
                         addr_record_t mapped;

                         IP::StringToAddress(remoteip.data() , remotePort,  mapped);

                        tuple = new TransportTuple(
                        udpSocket, reinterpret_cast<struct sockaddr*>(&mapped.addr)  );

                        tuple = iceServer->AddTuple(tuple);

                        iceServer->SetSelectedTuple(tuple) ;
                     * 
                     * */
 /*
            testUdpServer(std::string IP, int port, Agent *agent);

            void start() {
                udpServer = new UdpServer( this, IP, port);
                udpServer->bind();
            }

            char ip[40];  uint16_t port;
            base::net::IP::AddressToString(local, ip, 40, port);
     
            this->udpSockets[udpSocket] = ip;
            
            tuple = new TransportTuple(
                            udpSocket, reinterpret_cast<struct sockaddr*>(&remote.addr)  );

            tuple = iceServer->AddTuple(tuple);

            iceServer->SetSelectedTuple(tuple) ;
                            
   */        
            
        }
        
        /*with static*/
	WebRtcTransport::WebRtcTransport(const std::string& id, int agentNo, const Configuration &config, rtc::Transport::Listener* listener, int localPort, int remotePort , std::string localip, std::string remoteip )
	  : agent(agent), rtc::Transport::Transport(id, agentNo, config, listener)
	{
		
            uint16_t iceLocalPreferenceDecrement{ 0 };
            
            std::vector<ListenIp> listenIps;
            listenIps.resize(1);
            listenIps[0].announcedIp = "0.0.0.0";
            listenIps[0].ip = localip;
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

                         IP::StringToAddress(remoteip.data() , remotePort,  mapped);

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
            STrace << "AgentNo " << agentNo <<" Create a DTLS transport.";
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
                    udpSocket->Close();
                    
                    delete udpSocket;
            }
            this->udpSockets.clear();

            for (auto& kv : this->tcpServers)
            {
                    auto* tcpServer = kv.first;
                     tcpServer->Close();
                    delete tcpServer;
            }
            this->tcpServers.clear();

		
	}

        void WebRtcTransport::InitDtls(bool server, std::string announcedIp , addr_record_t &remotemapped , CertificateFingerprint dtlsRemoteFingerprint)
	{
            this->udpSockets[m_udpServer] = announcedIp;


            tuple = new TransportTuple(
            m_udpServer, reinterpret_cast<struct sockaddr*>(&remotemapped.addr)  );

            tuple = iceServer->AddTuple(tuple);

            iceServer->SetSelectedTuple(tuple) ;
            
            HandleRequest( server , dtlsRemoteFingerprint);
        }
       

	void WebRtcTransport::HandleRequest(bool server, CertificateFingerprint &dtlsRemoteFingerprint)
	{
		
            if(server)
                dtlsRole = rtc::DtlsTransport::Role::SERVER;
            else
                dtlsRole = rtc::DtlsTransport::Role::CLIENT;
            
            if (this->dtlsTransport->SetRemoteFingerprint(dtlsRemoteFingerprint))
            {
                    // If everything is fine, we may run the DTLS transport if ready.
                    MayRunDtlsTransport();
            }

            // MayRunDtlsTransport();


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
			SWarn << "AgentNo " << agentNo << "  DTLS not connected, cannot send SCTP data";

			return;
		}

                SInfo << "AgentNo " << agentNo << " SendAPP data " << len;

		this->dtlsTransport->SendApplicationData(data, len);
	}

	inline void WebRtcTransport::OnPacketReceived(
	  base::net::TransportTuple* tuple, const char* data, size_t len)
	{
		
            SDebug  << "AgentNo " << agentNo << " OnPacketReceived " << len;

		assertm(this->dtlsTransport, "no dtlsTransport");

		// Increase receive transmission.
//		rtc::Transport::DataReceived(len);

		
		// Check if it's DTLS.
		if (rtc::DtlsTransport::IsDtls( (const uint8_t*)data, len))
		{
			OnDtlsDataReceived(tuple, data, len);
		}
		else
		{
                    SInfo  << "AgentNo " << agentNo << " OnPacketReceived len: " << len << " data " << data;
                    
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
		
            SDebug  << "AgentNo " << agentNo << " OnDtlsDataReceived " << len;

            assertm(this->dtlsTransport, "no dtlsTransport");

		// Ensure it comes from a valid tuple.
		if (!this->iceServer->IsValidTuple(tuple))
		{
			LWarn( "ignoring DTLS data coming from an invalid tuple");

			return;
		}

		// Trick for clients performing aggressive ICE regardless we are ICE-Lite.
		this->iceServer->ForceSelectedTuple(tuple);

                
            if ( this->dtlsTransport->GetState() == rtc::DtlsTransport::DtlsState::CLOSED )
            {    
                 delete dtlsTransport;
                 dtlsTransport = new DtlsTransport(this);
                 dtlsTransport->SetRemoteFingerprint(config.gconfig->mCertificate->fingerprint());
                 MayRunDtlsTransport();
            }
                
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


        const uint8_t magicCookie[] = { 0x21, 0x12, 0xA4, 0x42 };
        inline bool IsStun(const uint8_t* data, size_t len)
        {

                return (
                        // STUN headers are 20 bytes.
                        (len >= 20) &&
                        // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
                        (data[0] < 3) &&
                        // Magic cookie must match.
                        (data[4] == magicCookie[0]) && (data[5] == magicCookie[1]) &&
                        (data[6] == magicCookie[2]) && (data[7] == magicCookie[3])
                );

        }


	inline void WebRtcTransport::OnUdpSocketPacketReceived(
	  base::net::UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr)
	{
            int family;

            std::string peerIp;
            uint16_t peerPort;

            IP::GetAddressInfo(
                        remoteAddr, family, peerIp, peerPort);
            SDebug  << "AgentNo " << agentNo <<  " OnUdpSocketPacketReceived " << peerIp << ":" << peerPort ;
            addr_record_t remotesrc;
            IP::CopyAddress(remoteAddr, remotesrc );

            #if 0
            std::string ret{"addr_record_is_equal "};
            {

                 char ip[40];
                 uint16_t port;
                 IP::AddressToString(remotesrc, ip, 40, port);
                 ret +=  ip + std::string(":") + std::to_string(port);

                 ret +=" < > ";


             }    

             #endif


            IP::addr_unmap_inet6_v4mapped((struct sockaddr *)&remotesrc.addr , &remotesrc.len);
                
                
            bool isStun = IsStun((const uint8_t*) data,  len);
            
            if(isStun)
            {

                agent->onStunMessage((unsigned char *)data, len,  &remotesrc, nullptr );
            }
            else
            {
                SDebug  << "AgentNo " << agentNo << " OnUdpSocketPacketReceived len: " << len <<  " dtls";
                     
                base::net::TransportTuple tuple(socket, (struct sockaddr *) &remotesrc.addr);

                OnPacketReceived(&tuple, data, len);
            }
	}

       inline void WebRtcTransport::on_close(base::net::Listener* conn)
	{
            SInfo  << "AgentNo " << agentNo << " on_close";

            net::TcpConnection* connection = (net::TcpConnection*) conn;

            base::net::TransportTuple tuple(connection);
//
		this->iceServer->RemoveTuple(&tuple);
	}


        void WebRtcTransport::on_read(base::net::Listener* conn, const char* data, size_t len) {

             SInfo  << "AgentNo " << agentNo << " on_read Len" <<  len;
             
//            rtc::TcpConnection* connection = (rtc::TcpConnection*) conn;
//            base::net::TransportTuple tuple(connection);
//
//            OnPacketReceived(&tuple, (const uint8_t*)data, len);   TBD

        }



	inline void WebRtcTransport::OnDtlsTransportConnecting(const rtc::DtlsTransport* /*dtlsTransport*/)
	{
		
                 SInfo  << "AgentNo " << agentNo << " OnDtlsTransportConnecting";

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
		
                SInfo  << "AgentNo " << agentNo << " OnDtlsTransportConnected";
                
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
                
                
                iceListener->OnDtlsTransportStatus(DtlsTransport::DtlsState::CONNECTED);
                
		///rtc::Transport::Connected();
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



             // agent->iceList->getRemoteDescriptio();
            
		// Tell the parent class.
	      rtc::Transport::Disconnected();
	}

	inline void WebRtcTransport::OnDtlsTransportSendData(
	  const rtc::DtlsTransport* /*dtlsTransport*/, const uint8_t* data, size_t len)
	{
		
            SInfo  << "AgentNo " << agentNo << " OnDtlsTransportSendData len:" << len;

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
		
///            SInfo  << "AgentNo " << agentNo << " ReceiveSctpData len:" << len;

            assertm(this->dtlsTransport, "no dtlsTransport");

            // Pass it to the parent transport.
            rtc::Transport::ReceiveSctpData((byte *)data, len);
	}
        
        
        int WebRtcTransport::agent_direct_send( uint8_t* data, uint32_t nbytes, addr_record_t &record )
        {
            return m_udpServer->send( (char*) data, nbytes , (const struct sockaddr*)&record.addr);
        }
      
} // namespace rtc
