

#ifndef RTC_ICE_CONFIGURATION_H
#define RTC_ICE_CONFIGURATION_H

#include "common.h"

#include <vector>

namespace rtc {

struct RTC_CPP_EXPORT IceServer {
	enum class Type { Stun, Turn };
	enum class RelayType { TurnUdp, TurnTcp, TurnTls };

	// Any type
	IceServer(const string &url);

	// STUN
	IceServer(string hostname_, uint16_t port_);
	IceServer(string hostname_, string service_);

	// TURN
	IceServer(string hostname_, uint16_t port, string username_, string password_,
	          RelayType relayType_ = RelayType::TurnUdp);
	IceServer(string hostname_, string service_, string username_, string password_,
	          RelayType relayType_ = RelayType::TurnUdp);

	string hostname;
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

const size_t DEFAULT_LOCAL_MAX_MESSAGE_SIZE = 256 * 1024;
        
struct RTC_CPP_EXPORT Configuration {
	// ICE settings
	std::vector<IceServer> iceServers;


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
	size_t mtu;

	// Local maximum message size for Data Channels
	size_t maxMessageSize{DEFAULT_LOCAL_MAX_MESSAGE_SIZE};

	// Certificates and private keys
	string certificatePemFile;
	string keyPemFile;
	string keyPemPass;
};



} // namespace rtc

#endif
