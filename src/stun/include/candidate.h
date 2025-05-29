

#ifndef RTC_CANDIDATE_H
#define RTC_CANDIDATE_H

#include "common.h"
#include <string>
#include <Types.h>

using namespace stun;

namespace rtc {

//    typedef enum ice_candidate_type {
//	ICE_CANDIDATE_TYPE_UNKNOWN,
//	ICE_CANDIDATE_TYPE_HOST,
//	ICE_CANDIDATE_TYPE_SERVER_REFLEXIVE,
//	ICE_CANDIDATE_TYPE_PEER_REFLEXIVE,
//	ICE_CANDIDATE_TYPE_RELAYED,
//} ice_candidate_type_t;


#define ICE_CANDIDATE_PREF_HOST 126
#define ICE_CANDIDATE_PREF_PEER_REFLEXIVE 110
#define ICE_CANDIDATE_PREF_SERVER_REFLEXIVE 100
#define ICE_CANDIDATE_PREF_RELAYED 0



 
//typedef struct ice_candidate {
//	ice_candidate_type_t type;
//	uint32_t priority;
//	int component;
//	char foundation[32 + 1]; // 1 to 32 characters
//	char transport[32 + 1];
//	char hostname[256 + 1];
//	//char service[32 + 1];
//        uint16_t port;
//
//} ice_candidate_t;    
    
class RTC_CPP_EXPORT Candidate {
public:
	//enum class Family { Unresolved, Ipv4, Ipv6 };
	enum class Type { Unknown, Host, ServerReflexive, PeerReflexive, Relayed };
	enum class TransportType { Unknown, Udp, TcpActive, TcpPassive, TcpSo, TcpUnknown };

	Candidate();
	Candidate(string candidate);
	Candidate(string candidate, string mid);

	void hintMid(string mid);
	void changeAddress(string addr);
	void changeAddress(string addr, uint16_t port);
	void changeAddress(string addr, string service);

	enum class ResolveMode { Simple, Lookup };
	bool resolve(ResolveMode mode = ResolveMode::Simple);

	Type type() const;
	TransportType transportType() const;
	uint32_t priority() const;
	string candidate() const;
	string mid() const;
	operator string() const;

	bool operator==(const Candidate &other) const;
	bool operator!=(const Candidate &other) const;

	bool isResolved() const;
	int family() const;
	string address() const;
	uint16_t port() const;
private:
	void parse(string candidate);
public:
	string mFoundation;
	uint32_t mComponent, mPriority;
	string mTypeString, mTransportString;
	Type mType;
	TransportType mTransportType;
	string mNode, mService;
	string mTail;

	string mMid;

	// Extracted on resolution
	int mFamily{-1};
	string mAddress;
	uint16_t mPort;
        
        addr_record_t resolved;

  //  ice_candidate_t cand;
        
};

RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, const Candidate &candidate);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out, const Candidate::Type &type);
RTC_CPP_EXPORT std::ostream &operator<<(std::ostream &out,
                                        const Candidate::TransportType &transportType);

} // namespace rtc

#endif
