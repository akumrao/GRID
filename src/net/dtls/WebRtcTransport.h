#ifndef WEBRTC_TRANSPORT_HPP
#define WEBRTC_TRANSPORT_HPP

#include "DtlsTransport.h"
#include "TransportTuple.h"
//#include "SrtpSession.h"
//#include "RTC/StunPacket.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include "Transport.h"
#include "TransportTuple.h"
#include "net/UdpSocket.h"
#include <vector>
#include "IceServer.h"

using namespace base::net;

namespace rtc
{
	class WebRtcTransport : public rtc::Transport,
			        public  base::net::UdpServer::Listener,
	                       // public rtc::TcpServer::Listener,
	                       // public rtc::TcpConnection::Listener,
                                public base::net::Listener,
	                        public rtc::DtlsTransport::Listener
	{
	private:
		struct ListenIp
		{
                    std::string ip;
                    std::string announcedIp;
                    int port;

		};

	public:
		WebRtcTransport(const std::string& id, const Configuration &config, rtc::Transport::Listener* listener, int localPort, int remotePort );
		~WebRtcTransport() override;;

	public:
                void HandleRequest(bool server);

	private:
		bool IsConnected() const ;
		void MayRunDtlsTransport();
                

		void SendSctpData(const uint8_t* data, size_t len) ;
		void OnPacketReceived(base::net::TransportTuple* tuple, const char* data, size_t len);
		void OnStunDataReceived(base::net::TransportTuple* tuple, const char* data, size_t len);
		void OnDtlsDataReceived(const base::net::TransportTuple* tuple, const char* data, size_t len);


		/* Pure virtual methods inherited from rtc::UdpSocket::Listener. */
	public:
		void OnUdpSocketPacketReceived(
		   base::net::UdpServer* socket, const  char* data, size_t len,  struct sockaddr* remoteAddr) override;

		/* Pure virtual methods inherited from rtc::TcpServer::Listener. */
	public:
		//void OnRtcTcpConnectionClosed(rtc::TcpServer* tcpServer, rtc::TcpConnection* connection) override;
		void on_close( base::net::Listener* conn);
		/* Pure virtual methods inherited from rtc::TcpConnection::Listener. */
	public:
//		void OnTcpConnectionPacketReceived(
//		  rtc::TcpConnection* connection, const uint8_t* data, size_t len) override;  //
                 void on_read( base::net::Listener* conn, const char* data, size_t len);
		/* Pure virtual methods inherited from rtc::IceServer::Listener. */
	public:


		/* Pure virtual methods inherited from rtc::DtlsTransport::Listener. */
	public:
		void OnDtlsTransportConnecting(const rtc::DtlsTransport* dtlsTransport) override;
                void OnDtlsTransportConnected( const rtc::DtlsTransport* /*dtlsTransport*/) override;
		
		void OnDtlsTransportFailed(const rtc::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportClosed(const rtc::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportSendData(
		  const rtc::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;
		void OnDtlsTransportApplicationDataReceived(
		  const rtc::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;

	private:
		// Allocated by this.

                rtc::IceServer* iceServer{ nullptr };        
                TransportTuple* tuple{ nullptr };
		// Map of UdpSocket/TcpServer and local announced IP (if any).
		std::unordered_map<base::net::UdpServer*, std::string> udpSockets;
		std::unordered_map<base::net::TcpServer*, std::string> tcpServers;
		rtc::DtlsTransport* dtlsTransport{ nullptr };
		
		// Others.
		bool connectCalled{ false }; // Whether connect() was succesfully called.

		rtc::DtlsTransport::Role dtlsRole{ rtc::DtlsTransport::Role::AUTO };
	};
} // namespace rtc

#endif
