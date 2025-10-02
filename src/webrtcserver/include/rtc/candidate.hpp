

#ifndef RTC_CANDIDATE_H
#define RTC_CANDIDATE_H

#include "common.hpp"
#include <Types.h>
#include "net/IP.h"
#include <string>
using namespace base::net;

namespace rtc {

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

	enum class ResolveMode { Simple, Lookup };
	bool resolve(ResolveMode mode = ResolveMode::Simple);
   //   void resolve();

	Type type() const;
	TransportType transportType() const;
	uint32_t priority() const;
	string candidate() const;
	string mid() const;
	operator string() const;

//	bool operator==(const Candidate &other) const;
//	bool operator!=(const Candidate &other) const;
        
        Candidate operator=(const Candidate &other);
        

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

	string mMid{"0"};

	// Extracted on resolution
      //  Family mFamily;
//string mAddress;
	//uint16_t mPort;
	//int mFamily{-1};
	//char mAddress[50];
	//uint16_t mPort;
        mutable addr_record_t resolved{0};
};

RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, const Candidate &candidate);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, const Candidate::Type &type);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out,
                                        const Candidate::TransportType &transportType);

} // namespace rtc

#endif
