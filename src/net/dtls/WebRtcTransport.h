#ifndef WEBRTC_TRANSPORT_HPP
#define WEBRTC_TRANSPORT_HPP

#include "DtlsTransport.h"

//#include "SrtpSession.h"
//#include "RTC/StunPacket.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include "TransportTuple.h"
#include "net/UdpSocket.h"
#include <vector>


namespace RTC
{
	class WebRtcTransport :  public  base::net::UdpServer::Listener,
	                       // public RTC::TcpServer::Listener,
	                       // public RTC::TcpConnection::Listener,
                                public base::net::Listener,
	                        public RTC::DtlsTransport::Listener
	{
	private:
		struct ListenIp
		{
			std::string ip;
			std::string announcedIp;
                        int port;
		};

	public:
		WebRtcTransport(const std::string& id );
		~WebRtcTransport() ;

	public:


	private:
		bool IsConnected() const ;
		void MayRunDtlsTransport();
                void HandleRequest();

		//void SendSctpData(const uint8_t* data, size_t len) override;
		void OnPacketReceived(base::net::TransportTuple* tuple, const char* data, size_t len);
		void OnStunDataReceived(base::net::TransportTuple* tuple, const char* data, size_t len);
		void OnDtlsDataReceived(const base::net::TransportTuple* tuple, const char* data, size_t len);


		/* Pure virtual methods inherited from RTC::UdpSocket::Listener. */
	public:
		void OnUdpSocketPacketReceived(
		   base::net::UdpServer* socket, const  char* data, size_t len,  struct sockaddr* remoteAddr) override;

		/* Pure virtual methods inherited from RTC::TcpServer::Listener. */
	public:
		//void OnRtcTcpConnectionClosed(RTC::TcpServer* tcpServer, RTC::TcpConnection* connection) override;
		void on_close( base::net::Listener* conn);
		/* Pure virtual methods inherited from RTC::TcpConnection::Listener. */
	public:
//		void OnTcpConnectionPacketReceived(
//		  RTC::TcpConnection* connection, const uint8_t* data, size_t len) override;  //
                 void on_read( base::net::Listener* conn, const char* data, size_t len);
		/* Pure virtual methods inherited from RTC::IceServer::Listener. */
	public:


		/* Pure virtual methods inherited from RTC::DtlsTransport::Listener. */
	public:
		void OnDtlsTransportConnecting(const RTC::DtlsTransport* dtlsTransport) override;
                void OnDtlsTransportConnected( const RTC::DtlsTransport* /*dtlsTransport*/) override;
		
		void OnDtlsTransportFailed(const RTC::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportClosed(const RTC::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportSendData(
		  const RTC::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;
		void OnDtlsTransportApplicationDataReceived(
		  const RTC::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;

	private:
		// Allocated by this.

		// Map of UdpSocket/TcpServer and local announced IP (if any).
		std::unordered_map<base::net::UdpServer*, std::string> udpSockets;
		std::unordered_map<base::net::TcpServer*, std::string> tcpServers;
		RTC::DtlsTransport* dtlsTransport{ nullptr };
		
		// Others.
		bool connectCalled{ false }; // Whether connect() was succesfully called.

		RTC::DtlsTransport::Role dtlsRole{ RTC::DtlsTransport::Role::AUTO };
	};
} // namespace RTC

#endif
