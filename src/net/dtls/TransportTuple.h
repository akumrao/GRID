#ifndef RTC_TRANSPORT_TUPLE_HPP
#define RTC_TRANSPORT_TUPLE_HPP

//#include "common.h"
//#include "Utils.h"
#include "net/netInterface.h"
#include "net/IP.h"
#include "net/TcpConnection.h"
#include "net/UdpSocket.h"
#include <json/json.hpp>
#include <string>

using json = nlohmann::json;


namespace base
{
    
    
namespace net

{

	class TransportTuple
	{
	protected:
		using onSendCallback = const std::function<void(bool sent)>;

	public:
		enum class Protocol
		{
			UDP = 1,
			TCP
		};

	public:
		TransportTuple(net::UdpSocket* udpSocket, const struct sockaddr* udpRemoteAddr);
		explicit TransportTuple(net::TcpConnection* tcpConnection);
		explicit TransportTuple(const TransportTuple* tuple);

	public:
		void FillJson(json& jsonObject) const;
		void Dump() const;
		void StoreUdpRemoteAddress();
		bool Compare(const TransportTuple* tuple) const;
		void SetLocalAnnouncedIp(std::string& localAnnouncedIp);
		void Send(const uint8_t* data, size_t len, net::TransportTuple::onSendCallback cb = nullptr);
		Protocol GetProtocol() const;
		const struct sockaddr* GetLocalAddress() const;
		const struct sockaddr* GetRemoteAddress() const;
		size_t GetRecvBytes() const;
		size_t GetSentBytes() const;

	private:
		// Passed by argument.
		net::UdpSocket* udpSocket{ nullptr };
		struct sockaddr* udpRemoteAddr{ nullptr };
		net::TcpConnection* tcpConnection{ nullptr };
		std::string localAnnouncedIp;
		// Others.
		struct sockaddr_storage udpRemoteAddrStorage;
#if !defined(__linux__) && defined(DUALSTACK)
		addr_record_t record_win{0};
#endif
		Protocol protocol;
	};



	inline size_t TransportTuple::GetRecvBytes() const
	{
		if (this->protocol == Protocol::UDP)
			return this->udpSocket->GetRecvBytes();
		else
			return this->tcpConnection->GetRecvBytes();
	}

	inline size_t TransportTuple::GetSentBytes() const
	{
		if (this->protocol == Protocol::UDP)
			return this->udpSocket->GetSentBytes();
		else
			return this->tcpConnection->GetSentBytes();
	}
} // namespace net

}  // base
#endif
