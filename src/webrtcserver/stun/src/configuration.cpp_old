
#include "configuration.h"

//#include "impl/utils.hpp"

#include <cassert>
#include <regex>


namespace rtc {


IceServer::IceServer(const string &url) {
	static const char *rs =
	    R"(^(([^:.@/?#]+):)?(/{0,2}((([^:@]*)(:([^@]*))?)@)?(([^:/?#]*)(:([^/?#]*))?))?([^?#]*)(\?([^#]*))?(#(.*))?)";
	static const std::regex r(rs, std::regex::extended);

	std::smatch m;
	if (!std::regex_match(url, m, r) || m[10].length() == 0)
		throw std::invalid_argument("Invalid ICE server URL: " + url);



	string scheme = m[1].str();
	if (scheme == "stun:" || scheme == "STUN:")
		type = Type::Stun;
	else if (scheme == "turn:" || scheme == "TURN:")
		type = Type::Turn;
	else if (scheme == "turns:" || scheme == "TURNS:")
		type = Type::Turn;
	else
		type = Type::Stun;

	relayType = RelayType::TurnUdp;
        
        std::string query = m[15];
                
	if (query.size()) 
        {
		if (query.find("transport=udp") != string::npos)
			relayType = RelayType::TurnUdp;
		if (query.find("transport=tcp") != string::npos)
			relayType = RelayType::TurnTcp;
		if (query.find("transport=tls") != string::npos)
			relayType = RelayType::TurnTls;
	}

	username = m[6].str();
	password = m[8].str();
	hostname = m[10].str();
        
	string service = m[12].str();
        
        if(!service.size())
        service = relayType == RelayType::TurnTls ? "5349" : "3478";
        
        try {
	    port = uint16_t(std::stoul(service));
	} catch (...) {
	    throw std::invalid_argument("Invalid ICE server port in URL: " + service);
	}

//	while (!hostname.empty() && hostname.front() == '[')
//		hostname.erase(hostname.begin());
//	while (!hostname.empty() && hostname.back() == ']')
//		hostname.pop_back();
}

IceServer::IceServer(string hostname_, uint16_t port_)
    : hostname(std::move(hostname_)), port(port_), type(Type::Stun) {}

IceServer::IceServer(string hostname_, string service_)
    : hostname(std::move(hostname_)), type(Type::Stun) {
	try {
		port = uint16_t(std::stoul(service_));
	} catch (...) {
		throw std::invalid_argument("Invalid ICE server port: " + service_);
	}
}

IceServer::IceServer(string hostname_, uint16_t port_, string username_, string password_,
                     RelayType relayType_)
    : hostname(std::move(hostname_)), port(port_), type(Type::Turn), username(std::move(username_)),
      password(std::move(password_)), relayType(relayType_) {}

IceServer::IceServer(string hostname_, string service_, string username_, string password_,
                     RelayType relayType_)
    : hostname(std::move(hostname_)), type(Type::Turn), username(std::move(username_)),
      password(std::move(password_)), relayType(relayType_) {
	try {
		port = uint16_t(std::stoul(service_));
	} catch (...) {
		throw std::invalid_argument("Invalid ICE server port: " + service_);
	}
}

}

