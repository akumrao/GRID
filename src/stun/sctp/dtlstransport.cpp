

#include "dtlstransport.hpp"
//#include "dtlssrtptransport.hpp"
//#include "icetransport.hpp"
//#include "internals.hpp"
#include "threadpool.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <exception>

#if !USE_GNUTLS
#ifdef _WIN32
#include <winsock2.h> // for timeval
#else
#include <sys/time.h> // for timeval
#endif
#endif

using namespace std::chrono;

namespace rtc {

void DtlsTransport::enqueueRecv() {
	if (mPendingRecvCount > 0)
		return;

	if (auto shared_this = weak_from_this().lock()) {
		++mPendingRecvCount;
		ThreadPool::Instance().enqueue(&DtlsTransport::doRecv, std::move(shared_this));
	}
}



#if USE_MBEDTLS

const mbedtls_ssl_srtp_profile srtpSupportedProtectionProfiles[] = {
    MBEDTLS_TLS_SRTP_AES128_CM_HMAC_SHA1_80,
    MBEDTLS_TLS_SRTP_UNSET,
};

DtlsTransport::DtlsTransport(shared_ptr<IceTransport> lower, certificate_ptr certificate,
                             optional<size_t> mtu,
                             CertificateFingerprint::Algorithm fingerprintAlgorithm,
                             verifier_callback verifierCallback, state_callback stateChangeCallback)
    : Transport(lower, std::move(stateChangeCallback)), mMtu(mtu), mCertificate(certificate),
      mFingerprintAlgorithm(fingerprintAlgorithm), mVerifierCallback(std::move(verifierCallback)),
      mIsClient(lower->role() == Description::Role::Active) {

	SDebug << "Initializing DTLS transport (MbedTLS)";

	if (!mCertificate)
		throw std::invalid_argument("DTLS certificate is null");

	mbedtls_entropy_init(&mEntropy);
	mbedtls_ctr_drbg_init(&mDrbg);
	mbedtls_ssl_init(&mSsl);
	mbedtls_ssl_config_init(&mConf);
	mbedtls_ctr_drbg_set_prediction_resistance(&mDrbg, MBEDTLS_CTR_DRBG_PR_ON);

	try {
		mbedtls::check(mbedtls_ctr_drbg_seed(&mDrbg, mbedtls_entropy_func, &mEntropy, NULL, 0));

		mbedtls::check(mbedtls_ssl_config_defaults(
		                   &mConf, mIsClient ? MBEDTLS_SSL_IS_CLIENT : MBEDTLS_SSL_IS_SERVER,
		                   MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT));

		mbedtls_ssl_conf_max_version(&mConf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3); // TLS 1.2
		mbedtls_ssl_conf_authmode(&mConf, MBEDTLS_SSL_VERIFY_OPTIONAL);
		mbedtls_ssl_conf_verify(&mConf, DtlsTransport::CertificateCallback, this);
		mbedtls_ssl_conf_rng(&mConf, mbedtls_ctr_drbg_random, &mDrbg);

		auto [crt, pk] = mCertificate->credentials();
		mbedtls::check(mbedtls_ssl_conf_own_cert(&mConf, crt.get(), pk.get()));

		mbedtls_ssl_conf_dtls_cookies(&mConf, NULL, NULL, NULL);
		mbedtls_ssl_conf_dtls_srtp_protection_profiles(&mConf, srtpSupportedProtectionProfiles);

		mbedtls::check(mbedtls_ssl_setup(&mSsl, &mConf));

		mbedtls_ssl_set_export_keys_cb(&mSsl, DtlsTransport::ExportKeysCallback, this);
		mbedtls_ssl_set_bio(&mSsl, this, WriteCallback, ReadCallback, NULL);
		mbedtls_ssl_set_timer_cb(&mSsl, this, SetTimerCallback, GetTimerCallback);

	} catch (...) {
		mbedtls_entropy_free(&mEntropy);
		mbedtls_ctr_drbg_free(&mDrbg);
		mbedtls_ssl_free(&mSsl);
		mbedtls_ssl_config_free(&mConf);
		throw;
	}

	// Set recommended medium-priority DSCP value for handshake
	// See https://www.rfc-editor.org/rfc/rfc8837.html#section-5
	mCurrentDscp = 10; // AF11: Assured Forwarding class 1, low drop probability
}

DtlsTransport::~DtlsTransport() {
	stop();

	SDebug << "Destroying DTLS transport";
	mbedtls_entropy_free(&mEntropy);
	mbedtls_ctr_drbg_free(&mDrbg);
	mbedtls_ssl_free(&mSsl);
	mbedtls_ssl_config_free(&mConf);
}

void DtlsTransport::Init() {
	// Nothing to do
}

void DtlsTransport::Cleanup() {
	// Nothing to do
}

void DtlsTransport::start() {
	SDebug << "Starting DTLS transport";
	registerIncoming();
	changeState(State::Connecting);

	{
		std::lock_guard lock(mSslMutex);
		size_t mtu = mMtu.value_or(DEFAULT_MTU) - 8 - 40; // UDP/IPv6
		mbedtls_ssl_set_mtu(&mSsl, static_cast<unsigned int>(mtu));
		STrace << "DTLS MTU set to " << mtu;
	}

	enqueueRecv(); // to initiate the handshake
}

void DtlsTransport::stop() {
	SDebug << "Stopping DTLS transport";
	unregisterIncoming();
	mIncomingQueue.stop();
	enqueueRecv();
}

bool DtlsTransport::send(message_ptr message) {
	if (!message || state() != State::Connected)
		return false;

	STrace << "Send size=" << message->size();

	int ret;
	do {
		std::lock_guard lock(mSslMutex);
		if (message->size() > size_t(mbedtls_ssl_get_max_out_record_payload(&mSsl)))
			return false;

		mCurrentDscp = message->dscp;
		ret = mbedtls_ssl_write(&mSsl, reinterpret_cast<const unsigned char *>(message->data()),
		                        message->size());
	} while (!mbedtls::check(ret));

	return mOutgoingResult;
}

void DtlsTransport::incoming(message_ptr message) {
	if (!message) {
		mIncomingQueue.stop();
		return;
	}

	STrace << "Incoming size=" << message->size();
	mIncomingQueue.push(message);
	enqueueRecv();
}

bool DtlsTransport::outgoing(message_ptr message) {
	message->dscp = mCurrentDscp;

	bool result = Transport::outgoing(std::move(message));
	mOutgoingResult = result;
	return result;
}

bool DtlsTransport::demuxMessage(message_ptr) {
	// Dummy
	return false;
}

void DtlsTransport::postHandshake() {
	// Dummy
}

void DtlsTransport::doRecv() {
	std::lock_guard lock(mRecvMutex);
	--mPendingRecvCount;

	if (state() != State::Connecting && state() != State::Connected)
		return;

	try {
		const size_t bufferSize = 4096;
		char buffer[bufferSize];

		// Handle handshake if connecting
		if (state() == State::Connecting) {
			while (true) {
				int ret;
				{
					std::lock_guard lock(mSslMutex);
					ret = mbedtls_ssl_handshake(&mSsl);
				}

				if (ret == MBEDTLS_ERR_SSL_WANT_READ) {
					ThreadPool::Instance().schedule(mTimerSetAt + milliseconds(mFinMs),
					                                [weak_this = weak_from_this()]() {
						                                if (auto locked = weak_this.lock())
							                                locked->doRecv();
					                                });
					return;
				}

				if (mbedtls::check(ret, "Handshake failed")) {
					// RFC 8261: DTLS MUST support sending messages larger than the current path MTU
					// See https://www.rfc-editor.org/rfc/rfc8261.html#section-5
					{
						std::lock_guard lock(mSslMutex);
						mbedtls_ssl_set_mtu(&mSsl, static_cast<unsigned int>(bufferSize + 1));
					}

					SInfo << "DTLS handshake finished";
					changeState(State::Connected);
					postHandshake();
					break;
				}
			}
		}

		if (state() == State::Connected) {
			while (true) {
				int ret;
				{
					std::lock_guard lock(mSslMutex);
					ret = mbedtls_ssl_read(&mSsl, reinterpret_cast<unsigned char *>(buffer),
					                       bufferSize);
				}

				if (ret == MBEDTLS_ERR_SSL_WANT_READ) {
					return;
				}

				if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
					SDebug << "DTLS connection cleanly closed";
					break;
				}

				if (mbedtls::check(ret)) {
					if (ret == 0) {
						SDebug << "DTLS connection terminated";
						break;
					}
					auto *b = reinterpret_cast<byte *>(buffer);
					recv(make_message(b, b + ret));
				}
			}
		}
	} catch (const std::exception &e) {
		SError << "DTLS recv: " << e.what();
	}

	if (state() == State::Connected) {
		SInfo << "DTLS closed";
		changeState(State::Disconnected);
		recv(nullptr);
	} else {
		SError << "DTLS handshake failed";
		changeState(State::Failed);
	}
}

int DtlsTransport::CertificateCallback(void *ctx, mbedtls_x509_crt *crt, int /*depth*/,
                                       uint32_t * /*flags*/) {
	auto this_ = static_cast<DtlsTransport *>(ctx);
	string fingerprint = make_fingerprint(crt, this_->mFingerprintAlgorithm);
	std::transform(fingerprint.begin(), fingerprint.end(), fingerprint.begin(),
	               [](char c) { return char(std::toupper(c)); });
	return this_->mVerifierCallback(fingerprint) ? 0 : 1;
}

void DtlsTransport::ExportKeysCallback(void *ctx, mbedtls_ssl_key_export_type /*type*/,
                                       const unsigned char *secret, size_t secret_len,
                                       const unsigned char client_random[32],
                                       const unsigned char server_random[32],
                                       mbedtls_tls_prf_types tls_prf_type) {
	auto dtlsTransport = static_cast<DtlsTransport *>(ctx);
	std::memcpy(dtlsTransport->mMasterSecret, secret, secret_len);
	std::memcpy(dtlsTransport->mRandBytes, client_random, 32);
	std::memcpy(dtlsTransport->mRandBytes + 32, server_random, 32);
	dtlsTransport->mTlsProfile = tls_prf_type;
}

int DtlsTransport::WriteCallback(void *ctx, const unsigned char *buf, size_t len) {
	auto *t = static_cast<DtlsTransport *>(ctx);
	try {
		if (len > 0) {
			auto b = reinterpret_cast<const byte *>(buf);
			t->outgoing(make_message(b, b + len));
		}
		return int(len);

	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
		return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
	}
}

int DtlsTransport::ReadCallback(void *ctx, unsigned char *buf, size_t len) {
	auto *t = static_cast<DtlsTransport *>(ctx);
	try {
		while (t->mIncomingQueue.running()) {
			auto next = t->mIncomingQueue.pop();
			if (!next) {
				return MBEDTLS_ERR_SSL_WANT_READ;
			}

			message_ptr message = std::move(*next);
			if (t->demuxMessage(message))
				continue;

			auto bufMin = std::min(len, size_t(message->size()));
			std::memcpy(buf, message->data(), bufMin);
			return int(len);
		}

		// Closed
		return 0;

	} catch (const std::exception &e) {
		PLOG_WARNING << e.what();
		return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
		;
	}
}

void DtlsTransport::SetTimerCallback(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
	auto dtlsTransport = static_cast<DtlsTransport *>(ctx);
	dtlsTransport->mIntMs = int_ms;
	dtlsTransport->mFinMs = fin_ms;

	if (fin_ms != 0) {
		dtlsTransport->mTimerSetAt = std::chrono::steady_clock::now();
	}
}

int DtlsTransport::GetTimerCallback(void *ctx) {
	auto dtlsTransport = static_cast<DtlsTransport *>(ctx);
	auto now = std::chrono::steady_clock::now();

	if (dtlsTransport->mFinMs == 0) {
		return -1;
	} else if (now >= dtlsTransport->mTimerSetAt + milliseconds(dtlsTransport->mFinMs)) {
		return 2;
	} else if (now >= dtlsTransport->mTimerSetAt + milliseconds(dtlsTransport->mIntMs)) {
		return 1;
	} else {
		return 0;
	}
}

#else // OPENSSL

BIO_METHOD *DtlsTransport::BioMethods = NULL;
int DtlsTransport::TransportExIndex = -1;
std::mutex DtlsTransport::GlobalMutex;

void DtlsTransport::Init() {
	std::lock_guard lock(GlobalMutex);

	openssl::init();

	if (!BioMethods) {
		BioMethods = BIO_meth_new(BIO_TYPE_BIO, "DTLS writer");
		if (!BioMethods)
			throw std::runtime_error("Failed to create BIO methods for DTLS writer");
		BIO_meth_set_create(BioMethods, BioMethodNew);
		BIO_meth_set_destroy(BioMethods, BioMethodFree);
		BIO_meth_set_write(BioMethods, BioMethodWrite);
		BIO_meth_set_ctrl(BioMethods, BioMethodCtrl);
	}
	if (TransportExIndex < 0) {
		TransportExIndex = SSL_get_ex_new_index(0, NULL, NULL, NULL, NULL);
	}
}

void DtlsTransport::Cleanup() {
	// Nothing to do
}

DtlsTransport::DtlsTransport( const Configuration & conf, Description::Role role, Certificate* certificate,
                             optional<size_t> mtu,
                             CertificateFingerprint::Algorithm fingerprintAlgorithm,
                             verifier_callback verifierCallback, state_callback stateChangeCallback)
    : Transport(conf, std::move(stateChangeCallback)), mMtu(mtu), mCertificate(certificate),
      mFingerprintAlgorithm(fingerprintAlgorithm), mVerifierCallback(std::move(verifierCallback))
      ,mIsClient( role == Description::Role::Active)
     {
	SDebug << "Initializing DTLS transport (OpenSSL)";

	if (!mCertificate)
		throw std::invalid_argument("DTLS certificate is null");

	try {
		mCtx = SSL_CTX_new(DTLS_method());
		if (!mCtx)
			throw std::runtime_error("Failed to create SSL context");

		// RFC 8261: SCTP performs segmentation and reassembly based on the path MTU.
		// Therefore, the DTLS layer MUST NOT use any compression algorithm.
		// See https://www.rfc-editor.org/rfc/rfc8261.html#section-5
		// RFC 8827: Implementations MUST NOT implement DTLS renegotiation
		// See https://www.rfc-editor.org/rfc/rfc8827.html#section-6.5
		SSL_CTX_set_options(mCtx, SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION | SSL_OP_NO_QUERY_MTU |
		                              SSL_OP_NO_RENEGOTIATION);

		SSL_CTX_set_min_proto_version(mCtx, DTLS1_VERSION);
		SSL_CTX_set_read_ahead(mCtx, 1);
		SSL_CTX_set_quiet_shutdown(mCtx, 0); // send the close_notify alert
		SSL_CTX_set_info_callback(mCtx, InfoCallback);

		SSL_CTX_set_verify(mCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
		                   CertificateCallback);
		SSL_CTX_set_verify_depth(mCtx, 1);

		openssl::check(SSL_CTX_set_cipher_list(mCtx, "ALL:!LOW:!EXP:!RC4:!MD5:@STRENGTH"),
		               "Failed to set SSL priorities");

#if OPENSSL_VERSION_NUMBER >= 0x30000000
		openssl::check(SSL_CTX_set1_groups_list(mCtx, "P-256"), "Failed to set SSL groups");
#else
		auto ecdh = unique_ptr<EC_KEY, decltype(&EC_KEY_free)>(
		    EC_KEY_new_by_curve_name(NID_X9_62_prime256v1), EC_KEY_free);
		SSL_CTX_set_tmp_ecdh(mCtx, ecdh.get());
#endif

		auto [x509, pkey] = mCertificate->credentials();
		SSL_CTX_use_certificate(mCtx, x509);
		SSL_CTX_use_PrivateKey(mCtx, pkey);
		openssl::check(SSL_CTX_check_private_key(mCtx), "SSL local private key check failed");

		mSsl = SSL_new(mCtx);
		if (!mSsl)
			throw std::runtime_error("Failed to create SSL instance");

		SSL_set_ex_data(mSsl, TransportExIndex, this);

		if (mIsClient)
			SSL_set_connect_state(mSsl);
		else
			SSL_set_accept_state(mSsl);

		mInBio = BIO_new(BIO_s_mem());
		mOutBio = BIO_new(BioMethods);
		if (!mInBio || !mOutBio)
			throw std::runtime_error("Failed to create BIO");

		BIO_set_mem_eof_return(mInBio, BIO_EOF);
		BIO_set_data(mOutBio, this);
		SSL_set_bio(mSsl, mInBio, mOutBio);

		// RFC 8827: The DTLS-SRTP protection profile SRTP_AES128_CM_HMAC_SHA1_80 MUST be supported
		// See https://www.rfc-editor.org/rfc/rfc8827.html#section-6.5
//		// Warning: SSL_set_tlsext_use_srtp() returns 0 on success and 1 on error
//#if RTC_ENABLE_MEDIA
//		// Try to use GCM suite
//		if (!DtlsSrtpTransport::IsGcmSupported() ||
//		    SSL_set_tlsext_use_srtp(
//		        mSsl, "SRTP_AEAD_AES_256_GCM:SRTP_AEAD_AES_128_GCM:SRTP_AES128_CM_SHA1_80")) {
//			PLOG_WARNING << "AES-GCM for SRTP is not supported, falling back to default profile";
//			if (SSL_set_tlsext_use_srtp(mSsl, "SRTP_AES128_CM_SHA1_80"))
//				throw std::runtime_error("Failed to set SRTP profile: " +
//				                         openssl::error_string(ERR_get_error()));
//		}
//#else
		if (SSL_set_tlsext_use_srtp(mSsl, "SRTP_AES128_CM_SHA1_80"))
			throw std::runtime_error("Failed to set SRTP profile: " +
			                         openssl::error_string(ERR_get_error()));
//#endif
	} catch (...) {
		if (mSsl)
			SSL_free(mSsl);
		if (mCtx)
			SSL_CTX_free(mCtx);
		throw;
	}

	// Set recommended medium-priority DSCP value for handshake
	// See https://www.rfc-editor.org/rfc/rfc8837.html#section-5
	mCurrentDscp = 10; // AF11: Assured Forwarding class 1, low drop probability
}

DtlsTransport::~DtlsTransport() {
	stop();

	SDebug << "Destroying DTLS transport";
	SSL_free(mSsl);
	SSL_CTX_free(mCtx);
}

void DtlsTransport::start() {
	SDebug << "Starting DTLS transport";
	registerIncoming();
	changeState(State::Connecting);

	int ret, err;
	{
		std::lock_guard lock(mSslMutex);

		size_t mtu = mMtu.value_or(DEFAULT_MTU) - 8 - 40; // UDP/IPv6
		SSL_set_mtu(mSsl, static_cast<unsigned int>(mtu));
		STrace << "DTLS MTU set to " << mtu;

		// Initiate the handshake
		ret = SSL_do_handshake(mSsl);
		err = SSL_get_error(mSsl, ret);
	}

	openssl::check_error(err, "Handshake failed");

	handleTimeout();
}

void DtlsTransport::stop() {
	SDebug << "Stopping DTLS transport";
	unregisterIncoming();
	mIncomingQueue.stop();
	enqueueRecv();
}

bool DtlsTransport::send(message_ptr message) {
	if (!message || state() != State::Connected)
		return false;

	STrace << "Send size=" << message->size();

	int ret, err;
	{
		std::lock_guard lock(mSslMutex);
		mCurrentDscp = message->dscp;
		ret = SSL_write(mSsl, message->data(), int(message->size()));
		err = SSL_get_error(mSsl, ret);
	}

	if (!openssl::check_error(err))
		return false;

	return mOutgoingResult;
}

void DtlsTransport::incoming(message_ptr message) {
	if (!message) {
		mIncomingQueue.stop();
		enqueueRecv();
		return;
	}

	STrace << "Incoming size=" << message->size();
	mIncomingQueue.push(message);
	enqueueRecv();
}

bool DtlsTransport::outgoing(message_ptr message) {
	message->dscp = mCurrentDscp;

	bool result = Transport::outgoing(std::move(message));
	mOutgoingResult = result;
	return result;
}

bool DtlsTransport::demuxMessage(message_ptr) {
	// Dummy
	return false;
}

void DtlsTransport::postHandshake() {
	// Dummy
}

void DtlsTransport::doRecv() {
	std::lock_guard lock(mRecvMutex);
	--mPendingRecvCount;

	if (state() != State::Connecting && state() != State::Connected)
		return;

	try {
		const size_t bufferSize = 4096;
		byte buffer[bufferSize];

		// Process pending messages
		while (mIncomingQueue.running()) {
			auto next = mIncomingQueue.pop();
			if (!next) {
				// No more messages pending, handle timeout if connecting
				if (state() == State::Connecting)
					handleTimeout();

				return;
			}

			message_ptr message = std::move(*next);
			if (demuxMessage(message))
				continue;

			BIO_write(mInBio, message->data(), int(message->size()));

			if (state() == State::Connecting) {
				// Continue the handshake
				int ret, err;
				{
					std::lock_guard lock(mSslMutex);
					ret = SSL_do_handshake(mSsl);
					err = SSL_get_error(mSsl, ret);
				}

				if (openssl::check_error(err, "Handshake failed")) {
					// RFC 8261: DTLS MUST support sending messages larger than the current path MTU
					// See https://www.rfc-editor.org/rfc/rfc8261.html#section-5
					{
						std::lock_guard lock(mSslMutex);
						SSL_set_mtu(mSsl, bufferSize + 1);
					}

					SInfo << "DTLS handshake finished";
					postHandshake();
					changeState(State::Connected);
				}
			}

			if (state() == State::Connected) {
				int ret, err;
				{
					std::lock_guard lock(mSslMutex);
					ret = SSL_read(mSsl, buffer, bufferSize);
					err = SSL_get_error(mSsl, ret);
				}

				if (err == SSL_ERROR_ZERO_RETURN) {
					SDebug << "TLS connection cleanly closed";
					break;
				}

				if (openssl::check_error(err))
					recv(make_message(buffer, buffer + ret));
			}
		}

		std::lock_guard lock(mSslMutex);
		SSL_shutdown(mSsl);

	} catch (const std::exception &e) {
		SError << "DTLS recv: " << e.what();
	}

	if (state() == State::Connected) {
		SInfo << "DTLS closed";
		changeState(State::Disconnected);
		recv(nullptr);
	} else {
		SError << "DTLS handshake failed";
		changeState(State::Failed);
	}
}

void DtlsTransport::handleTimeout() {
	std::lock_guard lock(mSslMutex);

	// Warning: This function breaks the usual return value convention
	int ret = DTLSv1_handle_timeout(mSsl);
	if (ret < 0) {
		throw std::runtime_error("Handshake timeout"); // write BIO can't fail
	} else if (ret > 0) {
		STrace << "DTLS retransmit done";
	}

	struct timeval tv = {};
	if (DTLSv1_get_timeout(mSsl, &tv)) {
		auto timeout = milliseconds(tv.tv_sec * 1000 + tv.tv_usec / 1000);
		// Also handle handshake timeout manually because OpenSSL actually
		// doesn't... OpenSSL backs off exponentially in base 2 starting from the
		// recommended 1s so this allows for 5 retransmissions and fails after
		// roughly 30s.
		if (timeout > 30s)
			throw std::runtime_error("Handshake timeout");

		STrace << "DTLS retransmit timeout is " << timeout.count() << "ms";
		ThreadPool::Instance().schedule(timeout, [weak_this = weak_from_this()]() {
			if (auto locked = weak_this.lock())
				locked->doRecv();
		});
	}
}

int DtlsTransport::CertificateCallback(int /*preverify_ok*/, X509_STORE_CTX *ctx) {
	SSL *ssl =
	    static_cast<SSL *>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
	DtlsTransport *t =
	    static_cast<DtlsTransport *>(SSL_get_ex_data(ssl, DtlsTransport::TransportExIndex));

	X509 *crt = X509_STORE_CTX_get_current_cert(ctx);
	string fingerprint = make_fingerprint(crt, t->mFingerprintAlgorithm);

	return t->mVerifierCallback(fingerprint) ? 1 : 0;
}

void DtlsTransport::InfoCallback(const SSL *ssl, int where, int ret) {
	DtlsTransport *t =
	    static_cast<DtlsTransport *>(SSL_get_ex_data(ssl, DtlsTransport::TransportExIndex));

	if (where & SSL_CB_ALERT) {
		if (ret != 256) { // Close Notify
			SError << "DTLS alert: " << SSL_alert_desc_string_long(ret);
		}
		t->mIncomingQueue.stop(); // Close the connection
	}
}

int DtlsTransport::BioMethodNew(BIO *bio) {
	BIO_set_init(bio, 1);
	BIO_set_data(bio, NULL);
	BIO_set_shutdown(bio, 0);
	return 1;
}

int DtlsTransport::BioMethodFree(BIO *bio) {
	if (!bio)
		return 0;
	BIO_set_data(bio, NULL);
	return 1;
}

int DtlsTransport::BioMethodWrite(BIO *bio, const char *in, int inl) {
	if (inl <= 0)
		return inl;
	auto transport = reinterpret_cast<DtlsTransport *>(BIO_get_data(bio));
	if (!transport)
		return -1;
	auto b = reinterpret_cast<const byte *>(in);
	transport->outgoing(make_message(b, b + inl));
	return inl; // can't fail
}

long DtlsTransport::BioMethodCtrl(BIO * /*bio*/, int cmd, long /*num*/, void * /*ptr*/) {
	switch (cmd) {
	case BIO_CTRL_FLUSH:
		return 1;
	case BIO_CTRL_DGRAM_QUERY_MTU:
		return 0; // SSL_OP_NO_QUERY_MTU must be set
	case BIO_CTRL_WPENDING:
	case BIO_CTRL_PENDING:
		return 0;
	default:
		break;
	}
	return 0;
}

#endif

} // namespace rtc::impl
