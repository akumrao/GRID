

#include "DtlsTransport.h"
#include "base/error.h"
#include "base/logger.h"
//#include "Settings.h"
//#include "Utils.h"
#include <openssl/asn1.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <uv.h>
#include <cstdio>  // std::sprintf(), std::fopen()
#include <cstring> // std::memcpy(), std::strcmp()

#include "net/certificate.h"


extern ConfCert config;

using namespace base;

#define LOG_OPENSSL_ERROR(desc)                                                                    \
	do                                                                                               \
	{                                                                                                \
		if (ERR_peek_error() == 0)                                                                     \
			SError <<  "OpenSSL error [desc:] " <<  desc;                                                 \
		else                                                                                           \
		{                                                                                              \
			int64_t err;                                                                                 \
			while ((err = ERR_get_error()) != 0)                                                         \
			{                                                                                            \
				SError << "OpenSSL error [desc:] " << desc  <<  " [error:] " <<  ERR_error_string(err, nullptr);   \
			}                                                                                            \
			ERR_clear_error();                                                                           \
		}                                                                                              \
	} while (false)

/* Static methods for OpenSSL callbacks. */

inline static int onSslCertificateVerify(int /*preverifyOk*/, X509_STORE_CTX* /*ctx*/)
{
	

	// Always valid since DTLS certificates are self-signed.
	return 1;
}

inline static void onSslInfo(const SSL* ssl, int where, int ret)
{
	static_cast<RTC::DtlsTransport*>(SSL_get_ex_data(ssl, 0))->OnSslInfo(where, ret);
}

inline static unsigned int onSslDtlsTimer(SSL* /*ssl*/, unsigned int timerUs)
{
	if (timerUs == 0)
		return 100000;
	else if (timerUs >= 4000000)
		return 4000000;
	else
		return 2 * timerUs;
}

namespace RTC
{
	/* Static. */

	
	static constexpr int DtlsMtu{ 1350 };
	static constexpr int SslReadBufferSize{ 65536 };
	// AES-HMAC: http://tools.ietf.org/html/rfc3711
	static constexpr size_t SrtpMasterKeyLength{ 16 };
	static constexpr size_t SrtpMasterSaltLength{ 14 };
	static constexpr size_t SrtpMasterLength{ SrtpMasterKeyLength + SrtpMasterSaltLength };
	// AES-GCM: http://tools.ietf.org/html/rfc7714
	static constexpr size_t SrtpAesGcm256MasterKeyLength{ 32 };
	static constexpr size_t SrtpAesGcm256MasterSaltLength{ 12 };
	static constexpr size_t SrtpAesGcm256MasterLength{ SrtpAesGcm256MasterKeyLength + SrtpAesGcm256MasterSaltLength };
	static constexpr size_t SrtpAesGcm128MasterKeyLength{ 16 };
	static constexpr size_t SrtpAesGcm128MasterSaltLength{ 12 };
	static constexpr size_t SrtpAesGcm128MasterLength{ SrtpAesGcm128MasterKeyLength + SrtpAesGcm128MasterSaltLength };
	

	/* Class variables. */

	//X509* DtlsTransport::certificate{ nullptr };
	//EVP_PKEY* DtlsTransport::privateKey{ nullptr };
	SSL_CTX* DtlsTransport::sslCtx{ nullptr };
	uint8_t DtlsTransport::sslReadBuffer[SslReadBufferSize];
	
//	std::map<std::string, DtlsTransport::FingerprintAlgorithm> DtlsTransport::string2FingerprintAlgorithm =
//	{
//		{ "sha-1",   DtlsTransport::FingerprintAlgorithm::SHA1   },
//		{ "sha-224", DtlsTransport::FingerprintAlgorithm::SHA224 },
//		{ "sha-256", DtlsTransport::FingerprintAlgorithm::SHA256 },
//		{ "sha-384", DtlsTransport::FingerprintAlgorithm::SHA384 },
//		{ "sha-512", DtlsTransport::FingerprintAlgorithm::SHA512 }
//	};
//	std::map<DtlsTransport::FingerprintAlgorithm, std::string> DtlsTransport::fingerprintAlgorithm2String =
//	{
//		{ DtlsTransport::FingerprintAlgorithm::SHA1,   "sha-1"   },
//		{ DtlsTransport::FingerprintAlgorithm::SHA224, "sha-224" },
//		{ DtlsTransport::FingerprintAlgorithm::SHA256, "sha-256" },
//		{ DtlsTransport::FingerprintAlgorithm::SHA384, "sha-384" },
//		{ DtlsTransport::FingerprintAlgorithm::SHA512, "sha-512" }
//	};
	std::map<std::string, DtlsTransport::Role> DtlsTransport::string2Role =
	{
		{ "auto",   DtlsTransport::Role::AUTO   },
		{ "client", DtlsTransport::Role::CLIENT },
		{ "server", DtlsTransport::Role::SERVER }
	};
	
//	std::vector<DtlsTransport::Fingerprint> DtlsTransport::localFingerprints;
	

	/* Class methods. */

	void DtlsTransport::ClassInit()
	{
            STrace << "DtlsTransport::ClassInit()";

		// Generate a X509 certificate and private key (unless PEM files are provided).
//		if (
//		  Settings::configuration.dtlsCertificateFile.empty() ||
//		  Settings::configuration.dtlsPrivateKeyFile.empty())
//		{
//		    GenerateCertificateAndPrivateKey();
//                    
//                   // SError << "No certificate files.";
//                   // base::uv::throwError("No certificate files");
//                    //exit(0);
//		}
////		else
////		{
////			ReadCertificateAndPrivateKeyFromFiles();
////		}
            
            
                 config.init();


		// Create a global SSL_CTX.
		CreateSslCtx();

		// Generate certificate fingerprints.
		//GenerateFingerprints();
	}

	void DtlsTransport::ClassDestroy()
	{
		

//		if (DtlsTransport::privateKey)
//			EVP_PKEY_free(DtlsTransport::privateKey);
//		if (DtlsTransport::certificate)
//			X509_free(DtlsTransport::certificate);
		if (DtlsTransport::sslCtx)
			SSL_CTX_free(DtlsTransport::sslCtx);
	}



//	void DtlsTransport::ReadCertificateAndPrivateKeyFromFiles()
//	{
//		
//
//		FILE* file{ nullptr };
//
//		file = fopen(Settings::configuration.dtlsCertificateFile.c_str(), "r");
//
//		if (!file)
//		{
//			LError("error reading DTLS certificate file: ", std::strerror(errno));
//
//			goto error;
//		}
//
//		DtlsTransport::certificate = PEM_read_X509(file, nullptr, nullptr, nullptr);
//
//		if (!DtlsTransport::certificate)
//		{
//			LOG_OPENSSL_ERROR("PEM_read_X509() failed");
//
//			goto error;
//		}
//
//		fclose(file);
//
//		file = fopen(Settings::configuration.dtlsPrivateKeyFile.c_str(), "r");
//
//		if (!file)
//		{
//			LError("error reading DTLS private key file: ", std::strerror(errno));
//
//			goto error;
//		}
//
//		DtlsTransport::privateKey = PEM_read_PrivateKey(file, nullptr, nullptr, nullptr);
//
//		if (!DtlsTransport::privateKey)
//		{
//			LOG_OPENSSL_ERROR("PEM_read_PrivateKey() failed");
//
//			goto error;
//		}
//
//		fclose(file);
//
//		return;
//
//	error:
//
//		base::uv::throwError("error reading DTLS certificate and private key PEM files");
//	}
        
        void DtlsTransport::onDtlError()
        {
            if (DtlsTransport::sslCtx)
		{
			SSL_CTX_free(DtlsTransport::sslCtx);
			DtlsTransport::sslCtx = nullptr;
		}

//		if (ecdh)
//			EC_KEY_free(ecdh);

		base::uv::throwError("SSL context creation failed");
                
        }
        
	void DtlsTransport::CreateSslCtx()
	{

 
            

		std::string dtlsSrtpProfiles;
		EC_KEY* ecdh{ nullptr };
		int ret;

/* Set the global DTLS context. */

// Both DTLS 1.0 and 1.2 (requires OpenSSL >= 1.1.0).
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
		DtlsTransport::sslCtx = SSL_CTX_new(DTLS_method());
// Just DTLS 1.0 (requires OpenSSL >= 1.0.1).
#elif (OPENSSL_VERSION_NUMBER >= 0x10001000L)
		DtlsTransport::sslCtx = SSL_CTX_new(DTLSv1_method());
#else
#error "too old OpenSSL version"
#endif
                
                if (!DtlsTransport::sslCtx)
                {
                        LOG_OPENSSL_ERROR("SSL_CTX_new() failed");

                       onDtlError(); return;
                }
                

//                if (
//		  Settings::configuration.dtlsCertificateFile.empty() ||
//		  Settings::configuration.dtlsPrivateKeyFile.empty())
                
                auto [x509, pkey] = config.mCertificate->credentials();
                                
		{

                    ret = SSL_CTX_use_certificate(DtlsTransport::sslCtx, x509);

                    if (ret == 0)
                    {
                            LOG_OPENSSL_ERROR("SSL_CTX_use_certificate() failed");

                            onDtlError(); return;
                    }

                    ret = SSL_CTX_use_PrivateKey(DtlsTransport::sslCtx, pkey);

                    if (ret == 0)
                    {
                            LOG_OPENSSL_ERROR("SSL_CTX_use_PrivateKey() failed");

                            onDtlError(); return;
                    }
                }
//                else
//                {    
//                    if (SSL_CTX_load_verify_locations(DtlsTransport::sslCtx, Settings::configuration.dtlsCertificateFile.c_str(), nullptr) != 1)
//                            ERR_print_errors_fp(stderr);
//
//                    if (SSL_CTX_set_default_verify_paths(DtlsTransport::sslCtx) != 1)
//                            ERR_print_errors_fp(stderr);
//
//                            /* set the local certificate from CertFile */
//                    if (SSL_CTX_use_certificate_file(DtlsTransport::sslCtx, Settings::configuration.dtlsCertificateFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
//                        ERR_print_errors_fp(stderr);
//                        abort();
//                    }
//                    
//                    
//                    SSL_CTX_set_default_passwd_cb_userdata(DtlsTransport::sslCtx, (void *) "12345678");
//                    
//                    if (SSL_CTX_use_PrivateKey_file(DtlsTransport::sslCtx, Settings::configuration.dtlsPrivateKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
//                        ERR_print_errors_fp(stderr);
//                        abort();
//                    }
//                
//                }
                
                
//                
                //if(server)
                if (1) {
                    //New lines //for server side only 

                    

                    ret = SSL_CTX_check_private_key(DtlsTransport::sslCtx);

                    if (ret == 0)
                    {
                            LOG_OPENSSL_ERROR("SSL_CTX_check_private_key() failed");

                            onDtlError(); return;
                    }

                
                }
                //End new lines

                        
             
		// Set options.
		SSL_CTX_set_options(
		  DtlsTransport::sslCtx,
		  SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_TICKET | SSL_OP_SINGLE_ECDH_USE |
		    SSL_OP_NO_QUERY_MTU);

		// Don't use sessions cache.
		SSL_CTX_set_session_cache_mode(DtlsTransport::sslCtx, SSL_SESS_CACHE_OFF);

		// Read always as much into the buffer as possible.
		// NOTE: This is the default for DTLS, but a bug in non latest OpenSSL
		// versions makes this call required.
		SSL_CTX_set_read_ahead(DtlsTransport::sslCtx, 1);

		SSL_CTX_set_verify_depth(DtlsTransport::sslCtx, 4);

		// Require certificate from peer.
		SSL_CTX_set_verify(
		  DtlsTransport::sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, onSslCertificateVerify);

		// Set SSL info callback.
		SSL_CTX_set_info_callback(DtlsTransport::sslCtx, onSslInfo);

		// Set ciphers.
		ret = SSL_CTX_set_cipher_list(
		  DtlsTransport::sslCtx, "ALL:!ADH:!LOW:!EXP:!MD5:!aNULL:!eNULL:@STRENGTH");

		if (ret == 0)
		{
			LOG_OPENSSL_ERROR("SSL_CTX_set_cipher_list() failed");

			onDtlError(); return;
		}

// Enable ECDH ciphers.
// DOC: http://en.wikibooks.org/wiki/OpenSSL/Diffie-Hellman_parameters
// NOTE: https://code.google.com/p/chromium/issues/detail?id=406458
// NOTE: https://bugs.ruby-lang.org/issues/12324
//
// Nothing to be done in OpenSSL >= 1.1.0.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
// For OpenSSL >= 1.0.2.
#elif (OPENSSL_VERSION_NUMBER >= 0x10002000L)
		SSL_CTX_set_ecdh_auto(DtlsTransport::sslCtx, 1);
// Older versions.
#else
		ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

		if (!ecdh)
		{
			LOG_OPENSSL_ERROR("EC_KEY_new_by_curve_name() failed");

			goto error;
		}

		if (SSL_CTX_set_tmp_ecdh(DtlsTransport::sslCtx, ecdh) != 1)
		{
			LOG_OPENSSL_ERROR("SSL_CTX_set_tmp_ecdh() failed");

			goto error;
		}

		EC_KEY_free(ecdh);
		ecdh = nullptr;
#endif

//		// Set the "use_srtp" DTLS extension.
//		for (auto it = DtlsTransport::srtpProfiles.begin(); it != DtlsTransport::srtpProfiles.end(); ++it)
//		{
//			if (it != DtlsTransport::srtpProfiles.begin())
//				dtlsSrtpProfiles += ":";
//
//			SrtpProfileMapEntry* profileEntry = std::addressof(*it);
//			dtlsSrtpProfiles += profileEntry->name;
//		}

//		SDebug << "setting SRTP profiles for DTLS: " <<  dtlsSrtpProfiles ;
//
//		// NOTE: This function returns 0 on success.
//		ret = SSL_CTX_set_tlsext_use_srtp(DtlsTransport::sslCtx, dtlsSrtpProfiles.c_str());
//
//		if (ret != 0)
//		{
//			LError("SSL_CTX_set_tlsext_use_srtp() failed when entering ", dtlsSrtpProfiles.c_str());
//			LOG_OPENSSL_ERROR("SSL_CTX_set_tlsext_use_srtp() failed");
//
//			goto error;
//		}

		return;
		
	}



	/* Instance methods. */

	DtlsTransport::DtlsTransport(Listener* listener) : listener(listener)
	{
		

		/* Set SSL. */

		this->ssl = SSL_new(DtlsTransport::sslCtx);

		if (!this->ssl)
		{
			LOG_OPENSSL_ERROR("SSL_new() failed");

			goto error;
		}

		// Set this as custom data.
		SSL_set_ex_data(this->ssl, 0, static_cast<void*>(this));

		this->sslBioFromNetwork = BIO_new(BIO_s_mem());

		if (!this->sslBioFromNetwork)
		{
			LOG_OPENSSL_ERROR("BIO_new() failed");

			SSL_free(this->ssl);

			goto error;
		}

		this->sslBioToNetwork = BIO_new(BIO_s_mem());

		if (!this->sslBioToNetwork)
		{
			LOG_OPENSSL_ERROR("BIO_new() failed");

			BIO_free(this->sslBioFromNetwork);
			SSL_free(this->ssl);

			goto error;
		}

		SSL_set_bio(this->ssl, this->sslBioFromNetwork, this->sslBioToNetwork);

		// Set the MTU so that we don't send packets that are too large with no fragmentation.
		SSL_set_mtu(this->ssl, DtlsMtu);
		DTLS_set_link_mtu(this->ssl, DtlsMtu);

		// Set callback handler for setting DTLS timer interval.
		DTLS_set_timer_cb(this->ssl, onSslDtlsTimer);

		// Set the DTLS timer.
		this->timer = new Timer(this);

		return;

	error:

		// NOTE: At this point SSL_set_bio() was not called so we must free BIOs as
		// well.
		if (this->sslBioFromNetwork)
			BIO_free(this->sslBioFromNetwork);

		if (this->sslBioToNetwork)
			BIO_free(this->sslBioToNetwork);

		if (this->ssl)
			SSL_free(this->ssl);

		// NOTE: If this is not catched by the caller the program will abort, but
		// this should never happen.
		base::uv::throwError("DtlsTransport instance creation failed");
	}

	DtlsTransport::~DtlsTransport()
	{
		

		if (IsRunning())
		{
			// Send close alert to the peer.
			SSL_shutdown(this->ssl);
			SendPendingOutgoingDtlsData();
		}

		if (this->ssl)
		{
			SSL_free(this->ssl);

			this->ssl               = nullptr;
			this->sslBioFromNetwork = nullptr;
			this->sslBioToNetwork   = nullptr;
		}

		// Close the DTLS timer.
		delete this->timer;
	}

	void DtlsTransport::Dump() const
	{
		

		std::string state{ "new" };
		std::string role{ "none " };

		switch (this->state)
		{
			case DtlsState::CONNECTING:
				state = "connecting";
				break;
			case DtlsState::CONNECTED:
				state = "connected";
				break;
			case DtlsState::FAILED:
				state = "failed";
				break;
			case DtlsState::CLOSED:
				state = "closed";
				break;
			default:;
		}

		switch (this->localRole)
		{
			case Role::AUTO:
				role = "auto";
				break;
			case Role::SERVER:
				role = "server";
				break;
			case Role::CLIENT:
				role = "client";
				break;
			default:;
		}

		LTrace("<DtlsTransport>");
		LTrace("  state           : ", state);
		LTrace("  role            : ", role);
		LTrace("  handshake done: : ", (this->handshakeDone ? "yes" : "no"));
		LTrace("</DtlsTransport>");
	}

	void DtlsTransport::Run(Role localRole)
	{
		

		assertm(
		  localRole == Role::CLIENT || localRole == Role::SERVER,
		  "local DTLS role must be 'client' or 'server'");

		Role previousLocalRole = this->localRole;

		if (localRole == previousLocalRole)
		{
			LError("same local DTLS role provided, doing nothing");

			return;
		}

		// If the previous local DTLS role was 'client' or 'server' do reset.
		if (previousLocalRole == Role::CLIENT || previousLocalRole == Role::SERVER)
		{
			LTrace( "resetting DTLS due to local role change");

			Reset();
		}

		// Update local role.
		this->localRole = localRole;

		// Set state and notify the listener.
		this->state = DtlsState::CONNECTING;
		this->listener->OnDtlsTransportConnecting(this);

		switch (this->localRole)
		{
			case Role::CLIENT:
			{
				LTrace( "running [role:client]");

				SSL_set_connect_state(this->ssl);
				SSL_do_handshake(this->ssl);
				SendPendingOutgoingDtlsData();
				SetTimeout();

				break;
			}

			case Role::SERVER:
			{
				LTrace( "running [role:server]");

				SSL_set_accept_state(this->ssl);
				SSL_do_handshake(this->ssl);

				break;
			}

			default:
			{
				MS_ABORT("invalid local DTLS role");
			}
		}
	}

//	bool DtlsTransport::SetRemoteFingerprint(Fingerprint fingerprint)
//	{
//		
//
//		assertm(
//		  fingerprint.algorithm != FingerprintAlgorithm::NONE, "no fingerprint algorithm provided");
//
//		this->remoteFingerprint = fingerprint;
//
//		// The remote fingerpring may have been set after DTLS handshake was done,
//		// so we may need to process it now.
//		if (this->handshakeDone && this->state != DtlsState::CONNECTED)
//		{
//			LTrace( "handshake already done, processing it right now");
//
//			return ProcessHandshake();
//		}
//
//		return true;
//	}

	void DtlsTransport::ProcessDtlsData(const uint8_t* data, size_t len)
	{
		

		int written;
		int read;

		if (!IsRunning())
		{
			LError("cannot process data while not running");

			return;
		}

		// Write the received DTLS data into the sslBioFromNetwork.
		written =
		  BIO_write(this->sslBioFromNetwork, static_cast<const void*>(data), static_cast<int>(len));

		if (written != static_cast<int>(len))
		{
			LWarn( "OpenSSL BIO_write() wrote less ", static_cast<size_t>(written), " than given data ", len);
			 
		}

		// Must call SSL_read() to process received DTLS data.
		read = SSL_read(this->ssl, static_cast<void*>(DtlsTransport::sslReadBuffer), SslReadBufferSize);

		// Send data if it's ready.
		SendPendingOutgoingDtlsData();

		// Check SSL status and return if it is bad/closed.
		if (!CheckStatus(read))
			return;

		// Set/update the DTLS timeout.
		if (!SetTimeout())
			return;

		// Application data received. Notify to the listener.
		if (read > 0)
		{
			// It is allowed to receive DTLS data even before validating remote fingerprint.
			if (!this->handshakeDone)
			{
				LWarn("ignoring application data received while DTLS handshake not done");

				return;
			}

			// Notify the listener.
			this->listener->OnDtlsTransportApplicationDataReceived(
			  this, (uint8_t*)DtlsTransport::sslReadBuffer, static_cast<size_t>(read));
		}
	}

	void DtlsTransport::SendApplicationData(const uint8_t* data, size_t len)
	{
		

		// We cannot send data to the peer if its remote fingerprint is not validated.
		if (this->state != DtlsState::CONNECTED)
		{
			LWarn("cannot send application data while DTLS is not fully connected");

			return;
		}

		if (len == 0)
		{
			LWarn("ignoring 0 length data");

			return;
		}

		int written;

		written = SSL_write(this->ssl, static_cast<const void*>(data), static_cast<int>(len));

		if (written < 0)
		{
			LOG_OPENSSL_ERROR("SSL_write() failed");

			if (!CheckStatus(written))
				return;
		}
		else if (written != static_cast<int>(len))
		{
			LWarn( "OpenSSL SSL_write() wrote less ", written, " than given data bytes ", len);
		}

		// Send data.
		SendPendingOutgoingDtlsData();
	}

	void DtlsTransport::Reset()
	{
		

		int ret;

		if (!IsRunning())
			return;

		LWarn("resetting DTLS transport");

		// Stop the DTLS timer.
		this->timer->Stop();

		// We need to reset the SSL instance so we need to "shutdown" it, but we
		// don't want to send a Close Alert to the peer, so just don't call
		// SendPendingOutgoingDTLSData().
		SSL_shutdown(this->ssl);

		this->localRole        = Role::NONE;
		this->state            = DtlsState::NEW;
		this->handshakeDone    = false;
		this->handshakeDoneNow = false;

		// Reset SSL status.
		// NOTE: For this to properly work, SSL_shutdown() must be called before.
		// NOTE: This may fail if not enough DTLS handshake data has been received,
		// but we don't care so just clear the error queue.
		ret = SSL_clear(this->ssl);

		if (ret == 0)
			ERR_clear_error();
	}

	inline bool DtlsTransport::CheckStatus(int returnCode)
	{
		

		int err;
		bool wasHandshakeDone = this->handshakeDone;

		err = SSL_get_error(this->ssl, returnCode);

		switch (err)
		{
			case SSL_ERROR_NONE:
				break;

			case SSL_ERROR_SSL:
				LOG_OPENSSL_ERROR("SSL status: SSL_ERROR_SSL");
				break;

			case SSL_ERROR_WANT_READ:
				break;

			case SSL_ERROR_WANT_WRITE:
				LWarn("SSL status: SSL_ERROR_WANT_WRITE");
				break;

			case SSL_ERROR_WANT_X509_LOOKUP:
				LTrace( "SSL status: SSL_ERROR_WANT_X509_LOOKUP");
				break;

			case SSL_ERROR_SYSCALL:
				LOG_OPENSSL_ERROR("SSL status: SSL_ERROR_SYSCALL");
				break;

			case SSL_ERROR_ZERO_RETURN:
				break;

			case SSL_ERROR_WANT_CONNECT:
				LWarn("SSL status: SSL_ERROR_WANT_CONNECT");
				break;

			case SSL_ERROR_WANT_ACCEPT:
				LWarn("SSL status: SSL_ERROR_WANT_ACCEPT");
				break;

			default:
				LWarn("SSL status: unknown error");
		}

		// Check if the handshake (or re-handshake) has been done right now.
		if (this->handshakeDoneNow)
		{
			this->handshakeDoneNow = false;
			this->handshakeDone    = true;

			// Stop the timer.
			this->timer->Stop();

			// Process the handshake just once (ignore if DTLS renegotiation).
			if (!wasHandshakeDone ) //if (!wasHandshakeDone && this->remoteFingerprint.algorithm != FingerprintAlgorithm::NONE)
				return ProcessHandshake();

			return true;
		}
		// Check if the peer sent close alert or a fatal error happened.
		else if (((SSL_get_shutdown(this->ssl) & SSL_RECEIVED_SHUTDOWN) != 0) || err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL)
		{
			if (this->state == DtlsState::CONNECTED)
			{
				LTrace( "disconnected");

				Reset();

				// Set state and notify the listener.
				this->state = DtlsState::CLOSED;
				this->listener->OnDtlsTransportClosed(this);
			}
			else
			{
				LWarn("connection failed");

				Reset();

				// Set state and notify the listener.
				this->state = DtlsState::FAILED;
				this->listener->OnDtlsTransportFailed(this);
			}

			return false;
		}
		else
		{
			return true;
		}
	}

	inline void DtlsTransport::SendPendingOutgoingDtlsData()
	{
		

		if (BIO_eof(this->sslBioToNetwork))
			return;

		int64_t read;
		char* data{ nullptr };

		read = BIO_get_mem_data(this->sslBioToNetwork, &data); // NOLINT

		if (read <= 0)
			return;

		//SDebug << read << " bytes of DTLS data ready to sent to the peer" ;

		// Notify the listener.
		this->listener->OnDtlsTransportSendData(
		  this, reinterpret_cast<uint8_t*>(data), static_cast<size_t>(read));

		// Clear the BIO buffer.
		// NOTE: the (void) avoids the -Wunused-value warning.
		(void)BIO_reset(this->sslBioToNetwork);
	}

	inline bool DtlsTransport::SetTimeout()
	{
		

		assertm(
		  this->state == DtlsState::CONNECTING || this->state == DtlsState::CONNECTED,
		  "invalid DTLS state");

		int64_t ret;
		uv_timeval_t dtlsTimeout{ 0, 0 };
		uint64_t timeoutMs;

		// NOTE: If ret == 0 then ignore the value in dtlsTimeout.
		// NOTE: No DTLSv_1_2_get_timeout() or DTLS_get_timeout() in OpenSSL 1.1.0-dev.
		ret = DTLSv1_get_timeout(this->ssl, static_cast<void*>(&dtlsTimeout)); // NOLINT

		if (ret == 0)
			return true;

		timeoutMs = (dtlsTimeout.tv_sec * static_cast<uint64_t>(1000)) + (dtlsTimeout.tv_usec / 1000);

		if (timeoutMs == 0)
		{
			return true;
		}
		else if (timeoutMs < 30000)
		{
			LDebug("DTLS timer set in ms", timeoutMs);

			this->timer->Start(timeoutMs);

			return true;
		}
		// NOTE: Don't start the timer again if the timeout is greater than 30 seconds.
		else
		{
			SWarn << "DTLS timeout too high ms, resetting DLTS" <<  timeoutMs;

			Reset();

			// Set state and notify the listener.
			this->state = DtlsState::FAILED;
			this->listener->OnDtlsTransportFailed(this);

			return false;
		}
	}

	inline bool DtlsTransport::ProcessHandshake()
	{
		

		assertm(this->handshakeDone, "handshake not done yet");
//		assertm(
//		  this->remoteFingerprint.algorithm != FingerprintAlgorithm::NONE, "remote fingerprint not set");

//		// Validate the remote fingerprint.
//		if (!CheckRemoteFingerprint())
//		{
//			Reset();
//
//			// Set state and notify the listener.
//			this->state = DtlsState::FAILED;
//			this->listener->OnDtlsTransportFailed(this);
//
//			return false;
//		}

//		// Get the negotiated SRTP profile.
//		RTC::SrtpSession::Profile srtpProfile = GetNegotiatedSrtpProfile();
//
//		if (srtpProfile != RTC::SrtpSession::Profile::NONE)
//		{
//			// Extract the SRTP keys (will notify the listener with them).
//			ExtractSrtpKeys(srtpProfile);
//
//			return true;
//		}
//
//		// NOTE: We assume that "use_srtp" DTLS extension is required even if
//		// there is no audio/video.
//		MS_WARN_2TAGS(dtls, srtp, "SRTP profile not negotiated");

		Reset();

		// Set state and notify the listener.
		this->state = DtlsState::FAILED;
		this->listener->OnDtlsTransportFailed(this);

		return false;
	}






	inline void DtlsTransport::OnSslInfo(int where, int ret)
	{
		

		int w = where & -SSL_ST_MASK;
		const char* role;

		if ((w & SSL_ST_CONNECT) != 0)
			role = "client";
		else if ((w & SSL_ST_ACCEPT) != 0)
			role = "server";
		else
			role = "undefined";

		if ((where & SSL_CB_LOOP) != 0)
		{
			LTrace( "role: ", role," action: ", SSL_state_string_long(this->ssl));
		}
		else if ((where & SSL_CB_ALERT) != 0)
		{
			const char* alertType;

			switch (*SSL_alert_type_string(ret))
			{
				case 'W':
					alertType = "warning";
					break;

				case 'F':
					alertType = "fatal";
					break;

				default:
					alertType = "undefined";
			}

			if ((where & SSL_CB_READ) != 0)
			{
				LWarn("received DTLS ",alertType, " alert: ",  SSL_alert_desc_string_long(ret));
			}
			else if ((where & SSL_CB_WRITE) != 0)
			{
				LTrace( "sending DTLS ", alertType, " alert: ",  SSL_alert_desc_string_long(ret));
			}
			else
			{
				LTrace( "DTLS ", alertType, " alert: ",  SSL_alert_desc_string_long(ret));
			}
		}
		else if ((where & SSL_CB_EXIT) != 0)
		{
			if (ret == 0)
                        {
				LTrace( "role: ", role, " failed: ",  SSL_state_string_long(this->ssl));
                        }
			else if (ret < 0)
                        {
				LTrace( "role: ", role, " waiting: ",  SSL_state_string_long(this->ssl));
                        }
		}
		else if ((where & SSL_CB_HANDSHAKE_START) != 0)
		{
			LTrace( "DTLS handshake start");
		}
		else if ((where & SSL_CB_HANDSHAKE_DONE) != 0)
		{
			LTrace( "DTLS handshake done");

			this->handshakeDoneNow = true;
		}

		// NOTE: checking SSL_get_shutdown(this->ssl) & SSL_RECEIVED_SHUTDOWN here upon
		// receipt of a close alert does not work (the flag is set after this callback).
	}

	inline void DtlsTransport::OnTimer(Timer* /*timer*/)
	{

		// Workaround for https://github.com/openssl/openssl/issues/7998.
		if (this->handshakeDone)
		{
			LDebug("handshake is done so return");

			return;
		}

		DTLSv1_handle_timeout(this->ssl);

		// If required, send DTLS data.
		SendPendingOutgoingDtlsData();

		// Set the DTLS timer again.
		SetTimeout();
	}
} // namespace RTC
