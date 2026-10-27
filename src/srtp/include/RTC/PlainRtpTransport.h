#ifndef MS_RTC_PLAIN_RTP_TRANSPORT_HPP
#define MS_RTC_PLAIN_RTP_TRANSPORT_HPP

#include "RTC/Transport.h"
#include "RTC/TransportTuple.h"
#include "RTC/UdpSocket.h"

namespace rtc
{
	class PlainRtpTransport : public rtc::Transport, public rtc::UdpSocket::Listener
	{
	private:
		struct ListenIp
		{
			std::string ip;
			std::string announcedIp;
		};

	public:
		PlainRtpTransport(const std::string& id, rtc::Transport::Listener* listener, json& data);
		~PlainRtpTransport() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) override;
		void HandleRequest(Channel::Request* request) override;

	private:
		bool IsConnected() const override;
		void SendRtpPacket(rtc::RtpPacket* packet, rtc::Transport::onSendCallback cb = nullptr) override;
		void SendRtcpPacket(rtc::RTCP::Packet* packet) override;
		void SendRtcpCompoundPacket(rtc::RTCP::CompoundPacket* packet) override;
		void SendSctpData(const uint8_t* data, size_t len) override;
		void OnPacketReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtpDataReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtcpDataReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnSctpDataReceived(rtc::TransportTuple* tuple, const uint8_t* data, size_t len);

		/* Pure virtual methods inherited from rtc::UdpSocket::Listener. */
	public:
		void OnUdpSocketPacketReceived(
		  rtc::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;

	private:
		// Allocated by this.
		rtc::UdpSocket* udpSocket{ nullptr };
		rtc::UdpSocket* rtcpUdpSocket{ nullptr };
		rtc::TransportTuple* tuple{ nullptr };
		rtc::TransportTuple* rtcpTuple{ nullptr };
		// Others.
		ListenIp listenIp;
		bool rtcpMux{ true };
		bool comedia{ false };
		bool multiSource{ false };
		struct sockaddr_storage remoteAddrStorage;
		struct sockaddr_storage rtcpRemoteAddrStorage;
	};
} // namespace rtc

#endif
