

#ifndef RTC_CANDIDATE_H
#define RTC_CANDIDATE_H

#include "common.hpp"
#include <Types.h>
#include "net/IP.h"

#define AGENT_DEBUG 1

using namespace base::net;
//using namespace stun;

using namespace std;

namespace rtc {

#define ICE_CANDIDATE_PREF_HOST 126
#define ICE_CANDIDATE_PREF_PEER_REFLEXIVE 110
#define ICE_CANDIDATE_PREF_SERVER_REFLEXIVE 100
#define ICE_CANDIDATE_PREF_RELAYED 0




class RTC_CPP_EXPORT Candidate {
public:
	//enum class Family { Unresolved, Ipv4, Ipv6 };
	enum class Type { Unknown, Host, ServerReflexive, PeerReflexive, Relayed };
	enum class TransportType { Unknown, Udp, TcpActive, TcpPassive, TcpSo, TcpUnknown };

	Candidate();
	Candidate(string candidate);
	Candidate(string candidate, string mid);

	void hintMid(string mid);
//	void changeAddress(string addr);
//	void changeAddress(string addr, uint16_t port);
//	void changeAddress(string addr, string service);

//	enum class ResolveMode { Simple, Lookup };
//	bool resolve(ResolveMode mode = ResolveMode::Simple);
        bool resolve();

	Type type() const;
	TransportType transportType() const;
	uint32_t priority() const;
	string candidate() const;
	string mid() const;
	operator string() const;

//	bool operator==(const Candidate &other) const;
//	bool operator!=(const Candidate &other) const;

	bool isResolved() const;
	int family() const;
	string address() ;
	uint16_t port() const;
public:
	void parse(string candidate);

	string mFoundation;
	uint32_t mComponent, mPriority;
	string mTypeString, mTransportString;
	Type mType;
	TransportType mTransportType;
	string mNode, mService;
	string mTail;
        string mMid;
        

	// Extracted on resolution
	//int mFamily{-1};
	//char mAddress[50];
	//uint16_t mPort;
        mutable addr_record_t resolved{0};
       
        #if AGENT_DEBUG
        std::string dump(bool full= false)
        {
            char ip[40];
            uint16_t port;
            IP::AddressToString(resolved, ip, 40, port);
            
            if(!full)
            return ip; 
            
            std::string ret =  ip + std::string(":") + std::to_string(port);
            return ret; 
        }
        #endif 
       
};

RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, const Candidate &candidate);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, const Candidate::Type &type);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out,
                                        const Candidate::TransportType &transportType);

} // namespace rtc

#endif
