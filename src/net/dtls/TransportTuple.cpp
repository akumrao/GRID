//#define MS_CLASS "net::TransportTuple"
// #define MS_LOG_DEV_LEVEL 3

#include "TransportTuple.h"
//#include "LoggerTag.h"
#include "base/logger.h"

#include "net/IP.h"
//#include "Utils.h"
#include <string>

namespace base
{
    

namespace net
{
	/* Instance methods. */

	void TransportTuple::FillJson(json& jsonObject) const
	{
		//MS_TRACE();

		int family;
		std::string ip;
		uint16_t port;

		base::net::IP::GetAddressInfo((struct sockaddr*)GetLocalAddress(), family, ip, port);

		// Add localIp.
		if (this->localAnnouncedIp.empty())
			jsonObject["localIp"] = ip;
		else
			jsonObject["localIp"] = this->localAnnouncedIp;

		// Add localPort.
		jsonObject["localPort"] = port;

		base::net::IP::GetAddressInfo((struct sockaddr*)GetRemoteAddress(), family, ip, port);

		// Add remoteIp.
		jsonObject["remoteIp"] = ip;

		// Add remotePort.
		jsonObject["remotePort"] = port;

		// Add protocol.
		switch (GetProtocol())
		{
			case Protocol::UDP:
				jsonObject["protocol"] = "udp";
				break;

			case Protocol::TCP:
				jsonObject["protocol"] = "tcp";
				break;
		}
	}

	void TransportTuple::Dump() const
	{
		//MS_TRACE();

		STrace << "<TransportTuple>";

		int family;
		std::string ip;
		uint16_t port;

		base::net::IP::GetAddressInfo((struct sockaddr*)GetLocalAddress(), family, ip, port);

		STrace << "  localIp    : " <<  ip;
		
                STrace << "  localPort  : " <<  port;

		base::net::IP::GetAddressInfo((struct sockaddr*)GetRemoteAddress(), family, ip, port);

		STrace << "  remoteIp   :" << ip;
		STrace << "  remotePort :" <<  port;

		switch (GetProtocol())
		{
			case Protocol::UDP:
				STrace << "  protocol   : udp";
				break;

			case Protocol::TCP:
				STrace << "  protocol   : tcp" ;
				break;
		}

		 STrace << "</TransportTuple>";
	}




	/*  methods. */

	 TransportTuple::TransportTuple(net::UdpSocket* udpSocket, const struct sockaddr* udpRemoteAddr)
	  : udpSocket(udpSocket), udpRemoteAddr((struct sockaddr*)udpRemoteAddr), protocol(Protocol::UDP)
	{
	}

	 TransportTuple::TransportTuple(net::TcpConnection* tcpConnection)
	  : tcpConnection(tcpConnection), protocol(Protocol::TCP)
	{
	}

	 TransportTuple::TransportTuple(const TransportTuple* tuple)
	  : udpSocket(tuple->udpSocket), udpRemoteAddr(tuple->udpRemoteAddr),
	    tcpConnection(tuple->tcpConnection), localAnnouncedIp(tuple->localAnnouncedIp),
	    protocol(tuple->protocol)
	{
		if (protocol == TransportTuple::Protocol::UDP)
			StoreUdpRemoteAddress();
	}

	 void TransportTuple::StoreUdpRemoteAddress()
	{
		// Clone the given address into our address storage and make the sockaddr
		// pointer point to it.
		this->udpRemoteAddrStorage = base::net::IP::CopyAddress(this->udpRemoteAddr);
		

  #ifndef __linux__

                base::net::IP::CopyAddress(this->udpRemoteAddr, record_win);

               socklen_t lent;
                IP::addr_map_inet6_v4mapped(
                    (struct sockaddr_storage *)&record_win.addr,
                    &record_win.len);
  #endif

				this->udpRemoteAddr =
                    (struct sockaddr *)&this->udpRemoteAddrStorage;
	}

	 TransportTuple::Protocol TransportTuple::GetProtocol() const
	{
		return this->protocol;
	}

	 bool TransportTuple::Compare(const TransportTuple* tuple) const
	{
		if (this->protocol == Protocol::UDP && tuple->GetProtocol() == Protocol::UDP)
		{
			return (
			  this->udpSocket == tuple->udpSocket &&
			  base::net::IP::CompareAddresses(this->udpRemoteAddr, tuple->GetRemoteAddress()));
		}
		else if (this->protocol == Protocol::TCP && tuple->GetProtocol() == Protocol::TCP)
		{
			return (this->tcpConnection == tuple->tcpConnection);
		}
		else
		{
			return false;
		}
	}

	 void TransportTuple::SetLocalAnnouncedIp(std::string& localAnnouncedIp)
	{
		this->localAnnouncedIp = localAnnouncedIp;
	}

	 void TransportTuple::Send(
	  const uint8_t* data, size_t len, net::TransportTuple::onSendCallback cb)
	{

		
#ifndef __linux__
          if (this->protocol == Protocol::UDP)
             this->udpSocket->send((const char *)data, len, (const struct sockaddr *)&this->record_win.addr, cb);

#else
          if (this->protocol == Protocol::UDP)
            this->udpSocket->send((const char *)data, len, this->udpRemoteAddr, cb);
 
	
#endif
          else
            this->tcpConnection->Write((const char *)data, len, cb);  
	
	}

	 const struct sockaddr* TransportTuple::GetLocalAddress() const
	{
		if (this->protocol == Protocol::UDP)
			return this->udpSocket->GetLocalAddress();
		else
			return this->tcpConnection->GetLocalAddress();
	}

	 const struct sockaddr* TransportTuple::GetRemoteAddress() const
	{
		if (this->protocol == Protocol::UDP)
			return (const struct sockaddr*)this->udpRemoteAddr;
		else
			return this->tcpConnection->GetPeerAddress();
	}





} // namespace net
}
