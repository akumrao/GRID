#ifndef MS_RTC_WEBRTC_TRANSPORT_HPP
#define MS_RTC_WEBRTC_TRANSPORT_HPP

#include "RTC/DtlsTransport.h"
#include "RTC/IceCandidate.h"
#include "RTC/IceServer.h"
#include "RTC/SrtpSession.h"
#include "RTC/StunPacket.h"
#include "net/netInterface.h"
#include "RTC/TcpConnection.h"
#include "RTC/TcpServer.h"
#include "RTC/Transport.h"
#include "RTC/TransportTuple.h"
#include "RTC/UdpSocket.h"
#include <vector>

namespace rtc
{
	class WebRtcTransport : public rtc::Transport,
	                        public rtc::UdpSocket::Listener,
	                       // public rtc::TcpServer::Listener,
	                       // public rtc::TcpConnection::Listener,
                                public base::net::Listener,
	                        public rtc::IceServer::Listener,
	                        public rtc::DtlsTransport::Listener
	{
	private:
		struct ListenIp
		{
			std::string ip;
			std::string announcedIp;
		};

	public:
		WebRtcTransport(const std::string& id, rtc::Transport::Listener* listener, json& data);
		~WebRtcTransport() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) override;
		void HandleRequest(Channel::Request* request) override;

	private:
		bool IsConnected() const override;
		void MayRunDtlsTransport();
		void SendRtpPacket(rtc::RtpPacket* packet, rtc::Transport::onSendCallback cb = nullptr) override;
		void SendRtcpPacket(rtc::RTCP::Packet* packet) override;
		void SendRtcpCompoundPacket(rtc::RTCP::CompoundPacket* packet) override;
		void SendSctpData(const uint8_t* data, size_t len) override;
		void OnPacketReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnStunDataReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnDtlsDataReceived(const rtc::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtpDataReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtcpDataReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);

		/* Pure virtual methods inherited from rtc::UdpSocket::Listener. */
	public:
		void OnUdpSocketPacketReceived(
		  rtc::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;

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
		void OnIceServerSendStunPacket(
		  const rtc::IceServer* iceServer,
		  const rtc::StunPacket* packet,
		  rtc::TransportTuple* tuple) override;
		void OnIceServerSelectedTuple(const rtc::IceServer* iceServer, rtc::TransportTuple* tuple) override;
		void OnIceServerConnected(const rtc::IceServer* iceServer) override;
		void OnIceServerCompleted(const rtc::IceServer* iceServer) override;
		void OnIceServerDisconnected(const rtc::IceServer* iceServer) override;

		/* Pure virtual methods inherited from rtc::DtlsTransport::Listener. */
	public:
		void OnDtlsTransportConnecting(const rtc::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportConnected(
		  const rtc::DtlsTransport* dtlsTransport,
		  rtc::SrtpSession::Profile srtpProfile,
		  uint8_t* srtpLocalKey,
		  size_t srtpLocalKeyLen,
		  uint8_t* srtpRemoteKey,
		  size_t srtpRemoteKeyLen,
		  std::string& remoteCert) override;
		void OnDtlsTransportFailed(const rtc::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportClosed(const rtc::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportSendData(
		  const rtc::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;
		void OnDtlsTransportApplicationDataReceived(
		  const rtc::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;

	private:
		// Allocated by this.
		rtc::IceServer* iceServer{ nullptr };
		// Map of UdpSocket/TcpServer and local announced IP (if any).
		std::unordered_map<rtc::UdpSocket*, std::string> udpSockets;
		std::unordered_map<rtc::TcpServer*, std::string> tcpServers;
		rtc::DtlsTransport* dtlsTransport{ nullptr };
		rtc::SrtpSession* srtpRecvSession{ nullptr };
		rtc::SrtpSession* srtpSendSession{ nullptr };
		// Others.
		bool connectCalled{ false }; // Whether connect() was succesfully called.
		std::vector<rtc::IceCandidate> iceCandidates;
		rtc::DtlsTransport::Role dtlsRole{ rtc::DtlsTransport::Role::AUTO };
	};
} // namespace rtc

#endif
