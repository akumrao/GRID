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
} // namespace net
}