

#include "candidate.hpp"

//#include "impl/internals.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <unordered_map>
#include "sdpcommon.h"
#include "base/logger.h"
#include <sys/types.h>


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>

using std::array;
using std::string;
using namespace base;

namespace {

inline bool match_prefix(const string &str, const string &prefix) {
	return str.size() >= prefix.size() &&
	       std::mismatch(prefix.begin(), prefix.end(), str.begin()).first == prefix.end();
}

inline void trim_begin(string &str) {
	str.erase(str.begin(),
	          std::find_if(str.begin(), str.end(), [](char c) { return !std::isspace(c); }));
}

inline void trim_end(string &str) {
	str.erase(
	    std::find_if(str.rbegin(), str.rend(), [](char c) { return !std::isspace(c); }).base(),
	    str.end());
}

} // namespace

namespace rtc {

Candidate::Candidate()
    : mFoundation("none"), mComponent(0), mPriority(0), mTypeString("unknown"),
      mTransportString("UDP"), mType(Type::Unknown), mTransportType(TransportType::Unknown),
      mNode("0.0.0.0"), mService("9")  {}

Candidate::Candidate(string candidate) : Candidate() {
	if (!candidate.empty())
		parse(std::move(candidate));
}

Candidate::Candidate(string candidate, string mid) : Candidate()
{
	if (!candidate.empty())
		parse(std::move(candidate));
        
	if(!mid.empty())
		mMid = mid;
}

/* Candidate Candidate::operator=(const Candidate &other)
 {
     int x = 1;
     
    mTail =  other.mTail;
    
    mFoundation = other.mFoundation;
    mComponent = other.mComponent;
    mPriority = other.mPriority; 
    mTypeString = other.mTypeString;
    mType= other.mType;
    mTransportType = other.mTransportType;
    mNode = other.mNode;
    mService = other.mService;
    resolved = other.resolved;
    mMid =  other.mMid;
   
   
}
*/

void Candidate::parse(string candidate) {
	using TypeMap_t = std::unordered_map<string, Type>;
	using TcpTypeMap_t = std::unordered_map<string, TransportType>;

	static const TypeMap_t TypeMap = {{"host", Type::Host},
	                                  {"srflx", Type::ServerReflexive},
	                                  {"prflx", Type::PeerReflexive},
	                                  {"relay", Type::Relayed}};

	static const TcpTypeMap_t TcpTypeMap = {{"active", TransportType::TcpActive},
	                                        {"passive", TransportType::TcpPassive},
	                                        {"so", TransportType::TcpSo}};

	const std::array<string , 2> prefixes{"a=", "candidate:"};
	for (string prefix : prefixes)
		if (match_prefix(candidate, prefix))
			candidate.erase(0, prefix.size());

	STrace << "Parsing candidate: " << candidate;

	// See RFC 8839 for format
	std::istringstream iss(candidate);
	string typ_;
	if (!(iss >> mFoundation >> mComponent >> mTransportString >> mPriority &&
	      iss >> mNode >> mService >> typ_ >> mTypeString && typ_ == "typ"))
		throw std::invalid_argument("Invalid candidate format");

	std::getline(iss, mTail);
	trim_begin(mTail);
	trim_end(mTail);

	if (auto it = TypeMap.find(mTypeString); it != TypeMap.end())
		mType = it->second;
	else
		mType = Type::Unknown;

	if (mTransportString == "UDP" || mTransportString == "udp") {
		mTransportType = TransportType::Udp;
	} else if (mTransportString == "TCP" || mTransportString == "tcp") {
		// Peek tail to find TCP type
		std::istringstream tiss(mTail);
		string tcptype_, tcptype;
		if (tiss >> tcptype_ >> tcptype && tcptype_ == "tcptype") {
			if (auto it = TcpTypeMap.find(tcptype); it != TcpTypeMap.end())
				mTransportType = it->second;
			else
				mTransportType = TransportType::TcpUnknown;

		} else {
			mTransportType = TransportType::TcpUnknown;
		}
	} else {
		mTransportType = TransportType::Unknown;
	}
}

void Candidate::hintMid(string mid) {
	if (!mMid.length())
		mMid = std::move(mid);
}

Candidate::Type Candidate::type() const { return mType; }

Candidate::TransportType Candidate::transportType() const { return mTransportType; }

uint32_t Candidate::priority() const { return mPriority; }

string Candidate::candidate() const {
	const char sp{' '};
	std::ostringstream oss;
	oss << "candidate:";
	oss << mFoundation << sp << mComponent << sp << mTransportString << sp << mPriority << sp;
	if (isResolved())
        {

            char ip[40];  uint16_t port;
            base::net::IP::AddressToString(resolved, ip,40, port);
	    oss << ip << sp << port;
        }
	else
		oss << mNode << sp << mService;

	const char *type = nullptr;
	char *suffix = nullptr;
	int ret = ice_type_suffix(this, &type, &suffix);

	if (!ret) {
	  char tmp[100];
	  // No need to redeclare 'type' and 'suffix' here, just reuse them
	  snprintf(tmp, 99, "%s%s%s", type, suffix ? " " : "",
		   suffix ? suffix : "");
	  oss << sp << "typ" << sp << tmp;
        }

	if (!mTail.empty())
		oss << sp << mTail;

	return oss.str();
}

string Candidate::mid() const { return mMid;}

Candidate::operator string() const {
	std::ostringstream line;
	line << "a=" << candidate();
	return line.str();
}

//bool Candidate::operator==(const Candidate &other) const {
//    
//    if(isResolved() && other.isResolved() )
//    {
//
//    	return ((mFoundation == other.mFoundation  &&  IP::addr_record_is_equal( &resolved,  &other.resolved,  true)) );
//    }
//    else
//    {
//        SError << " This is not allowed state , exiting stun";
//        exit(0);
//       // return (mFoundation == other.mFoundation && mService == other.mService && mNode == other.mNode);
//    }
//    
//}
//
//bool Candidate::operator!=(const Candidate &other) const {
//	return mFoundation != other.mFoundation;
//}

bool Candidate::isResolved() const {
    return resolved.len ;
}

bool Candidate::resolve()
{
   return IP::StringToAddress(mNode.c_str(), std::stoi( mService), resolved);
}

int Candidate::family() const { return resolved.addr.ss_family; }

string Candidate::address() 
{
    
    if(isResolved())
    {
        char ip[40];  uint16_t port;
        base::net::IP::AddressToString(resolved, ip, 40, port);
        return ip;
    }
    else
        return "";
}

uint16_t Candidate::port() const {
    if(isResolved())
    {
        char ip[40];  uint16_t port;
        base::net::IP::AddressToString(resolved, ip, 40, port);
        return port;
    }
    else
        return 0;
}

std::ostream &operator<<(std::ostream &out, const Candidate &candidate) {
	return out << string(candidate);
}

std::ostream &operator<<(std::ostream &out, const Candidate::Type &type) {
	switch (type) {
	case Candidate::Type::Host:
		return out << "host";
	case Candidate::Type::PeerReflexive:
		return out << "prflx";
	case Candidate::Type::ServerReflexive:
		return out << "srflx";
	case Candidate::Type::Relayed:
		return out << "relay";
	default:
		return out << "unknown";
	}
}

std::ostream &operator<<(std::ostream &out, const Candidate::TransportType &transportType) {
	switch (transportType) {
	case Candidate::TransportType::Udp:
		return out << "UDP";
	case Candidate::TransportType::TcpActive:
		return out << "TCP_active";
	case Candidate::TransportType::TcpPassive:
		return out << "TCP_passive";
	case Candidate::TransportType::TcpSo:
		return out << "TCP_so";
	case Candidate::TransportType::TcpUnknown:
		return out << "TCP_unknown";
	default:
		return out << "unknown";
	}
}

} // namespace rtc
