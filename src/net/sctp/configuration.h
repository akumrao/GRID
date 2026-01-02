

#ifndef RTC_ICE_CONFIGURATION_H
#define RTC_ICE_CONFIGURATION_H

#include "common.h"

#include <vector>


namespace RTC {

const size_t MAX_NUMERICNODE_LEN = 48; // Max IPv6 string representation length
const size_t MAX_NUMERICSERV_LEN = 6;  // Max port string representation length

const uint16_t DEFAULT_SCTP_PORT = 5000; // SCTP port to use by default

const uint16_t MAX_SCTP_STREAMS_COUNT = 1024; // Max number of negotiated SCTP streams
                                              // RFC 8831 recommends 65535 but usrsctp needs a lot
                                              // of memory, Chromium historically limits to 1024.

const size_t DEFAULT_LOCAL_MAX_MESSAGE_SIZE = 256 * 1024; // Default local max message size
const size_t DEFAULT_REMOTE_MAX_MESSAGE_SIZE = 65536;     // Remote max message size if not in SDP

const size_t DEFAULT_WS_MAX_MESSAGE_SIZE = 256 * 1024;   // Default max message size for WebSockets

const size_t RECV_QUEUE_LIMIT = 1024; // Max per-channel queue size (messages)

const int MIN_THREADPOOL_SIZE = 4; // Minimum number of threads in the global thread pool (>= 2)

const size_t DEFAULT_MTU = RTC_DEFAULT_MTU; // defined in rtc.h

} // namespace RTC




namespace RTC {

struct RTC_CPP_EXPORT IceServer_conf {
	enum class Type { Stun, Turn };
	enum class RelayType { TurnUdp, TurnTcp, TurnTls };

	// Any type
	IceServer_conf(const string &url);

	// STUN
	IceServer_conf(string hostname_, uint16_t port_);
	IceServer_conf(string hostname_, string service_);

	// TURN
	IceServer_conf(string hostname_, uint16_t port, string username_, string password_,
	          RelayType relayType_ = RelayType::TurnUdp);
	IceServer_conf(string hostname_, string service_, string username_, string password_,
	          RelayType relayType_ = RelayType::TurnUdp);

	string hostname;
        string ip;
	uint16_t port;
	Type type;
	string username;
	string password;
	RelayType relayType;
};



enum class CertificateType {
	Default = RTC_CERTIFICATE_DEFAULT, // ECDSA
	Ecdsa = RTC_CERTIFICATE_ECDSA,
	Rsa = RTC_CERTIFICATE_RSA
};

enum class TransportPolicy { All = RTC_TRANSPORT_POLICY_ALL, Relay = RTC_TRANSPORT_POLICY_RELAY };

//const size_t DEFAULT_LOCAL_MAX_MESSAGE_SIZE = 256 * 1024;

//#define RTC_DEFAULT_MTU 1280 // IPv6 minimum guaranteed MTU
//#define DEFAULT_MTU  RTC_DEFAULT_MTU
        
struct RTC_CPP_EXPORT Configuration {
	// ICE settings
	std::vector<IceServer_conf> iceServers;


	// Options
	CertificateType certificateType = CertificateType::Default;
	TransportPolicy iceTransportPolicy = TransportPolicy::All;
	bool enableIceTcp = false;    // libnice only
	bool enableIceUdpMux = false; // libjuice only
	bool disableAutoNegotiation = false;
	bool disableAutoGathering = false;
	bool forceMediaTransport = false;
	bool disableFingerprintVerification = false;

	// Port range
	uint16_t portRangeBegin = 1024;
	uint16_t portRangeEnd = 65535;

	// Network MTU
	size_t mtu{DEFAULT_MTU};

	// Local maximum message size for Data Channels
	size_t maxMessageSize{DEFAULT_LOCAL_MAX_MESSAGE_SIZE};

	// Certificates and private keys
	//string certificatePemFile;  // moved to certificate config file
	//string keyPemFile;
	//string keyPemPass;
};



} // namespace RTC

#endif
