#include "DtlsTransport.h"
#include "base/error.h"
#include "base/logger.h"

#include "IceServer.h"
#include "base/application.h"
#include "net/certificate.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <uv.h>

extern ConfCert config;
using namespace base;

#if USE_MBEDTLS

namespace rtc {

    inline void DtlsTransport::OnTimer(Timer * /*timer*/) {
        SInfo << "OnTimer ";
        
        if (is_cancelled || handshakeDone) {
            return;
        }
        
        handshake();
    }

    void ssl_set_timer(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
        DtlsTransport *t_ctx = static_cast<DtlsTransport *>(ctx);

        if (fin_ms == 0) {
            if (t_ctx->timer) {
                t_ctx->timer->Stop();
            }
            t_ctx->is_cancelled = 1;
            return;
        }

        t_ctx->intermediate_ms = int_ms;
        t_ctx->final_ms = fin_ms;

        if (t_ctx->timer && t_ctx->timer->GetUVloop()) {
            t_ctx->start_time = uv_now(t_ctx->timer->GetUVloop());
        }
        t_ctx->is_cancelled = 0;

        if (t_ctx->timer) {
            t_ctx->timer->Start(fin_ms, 0);
        }
    }

    int ssl_get_timer(void *ctx) {
        DtlsTransport *t_ctx = static_cast<DtlsTransport *>(ctx);
        if (t_ctx->is_cancelled || !t_ctx->timer)
            return -1;

        uint64_t elapsed = uv_now(t_ctx->timer->GetUVloop()) - t_ctx->start_time;
        if (elapsed >= t_ctx->final_ms)
            return 2;
        if (elapsed >= t_ctx->intermediate_ms)
            return 1;
        return 0;
    }

    const mbedtls_ssl_srtp_profile srtpSupportedProtectionProfiles[] = {
        MBEDTLS_TLS_SRTP_AES128_CM_HMAC_SHA1_80,
        MBEDTLS_TLS_SRTP_UNSET,
    };



    void DtlsTransport::CreateSslCtx() {
        return;
    }

    void mbedtls_debug_callback(void *ctx, int level,
            const char *file, int line,
            const char *str) {
        // Print the debug string or stream it to stderr/stdout
        ((void) ctx); // Unused context parameter
        std::cout << "MbedTLS [Level " << level << "] (" << file << ":" << line << ") " << str;
    }

    DtlsTransport::DtlsTransport(Listener *listener) : listener(listener) {
        if (!config.mCertificate)
            throw std::invalid_argument("DTLS certificate is null");

        /* 
        for MBEDTLS_SSL_VERSION_TLS1_3)
                if (psa_crypto_init() != PSA_SUCCESS) {
                std::cerr << "Failed to initialize PSA Crypto API." << std::endl;
                return ;
            }
         */

        mbedtls_entropy_init(&mEntropy);
        mbedtls_ctr_drbg_init(&mDrbg);
        mbedtls_ssl_init(&mSsl);
        mbedtls_ssl_config_init(&mConf);
        mbedtls_ctr_drbg_set_prediction_resistance(&mDrbg, MBEDTLS_CTR_DRBG_PR_ON);

        //  mbedtls_ssl_conf_min_tls_version(&mConf, MBEDTLS_SSL_VERSION_TLS1_3);
        // mbedtls_ssl_conf_max_tls_version(&mConf, MBEDTLS_SSL_VERSION_TLS1_3);

        // mbedtls_debug_set_threshold(3);  4 is verbose 

        // 3. Register the callback with the SSL configuration
        // The last argument (nullptr) is passed as the 'void *ctx' to your callback
        //mbedtls_ssl_conf_dbg(&mConf, mbedtls_debug_callback, nullptr);

        mbedtls_ssl_conf_authmode(&mConf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    }


    inline bool DtlsTransport::CheckRemoteFingerprint() {
        return true;
    }

    void DtlsTransport::SendApplicationData(const uint8_t *data, size_t len) {
        if (!len || !this->handshakeDone)
            return;

        STrace << "Send size=" << len;

        if (len > static_cast<size_t>(mbedtls_ssl_get_max_out_record_payload(&mSsl)))
            return;

        int rv = mbedtls_ssl_write(&mSsl, reinterpret_cast<const unsigned char *>(data), len);
        if (rv < 0) {
            swrap_error_handler(rv);
            return;
        }

        // Flush encrypted DTLS records out of app_bio_ to network
        stay_uptodate();
    }

    void DtlsTransport::ProcessDtlsData(const uint8_t *data, size_t len) {
        if (!data || len == 0) return;
        STrace << "ProcessDtlsData " << len;

        uint8_t contentType = data[0];

        if (contentType == 23 && !this->handshakeDone) {
            SWarn << "Dropping stray client media/application packet (Content Type 23) received before Handshake completion!";
            return;
        }         
        
        TLS_BIO_write(app_bio_, reinterpret_cast<const char *>(data), static_cast<int>(len));

        if (!this->handshakeDone) {
            if (!handshake())
                return;
        }

        std::vector<uint8_t> read_buf(len);

        while (true) {
            std::fill(read_buf.begin(), read_buf.end(), 0);

            int rv = mbedtls_ssl_read(&mSsl, read_buf.data(), static_cast<int>(len));
            rv = swrap_error_handler(rv);

            if (rv > 0) {
                if (this->listener) {
                    this->listener->OnDtlsTransportApplicationDataReceived(
                            this, read_buf.data(), static_cast<size_t>(rv));
                }
            } else if (rv == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                SInfo << "MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY";
                this->state = DtlsState::CLOSED;
                if (this->listener) {
                    this->listener->OnDtlsTransportClosed(this);
                }
                break;
            } else if (rv == MBEDTLS_ERR_SSL_WANT_READ ||
                    rv == MBEDTLS_ERR_SSL_WANT_WRITE) {
                break;
            } else {
                break;
            }
        }
    }

    void DtlsTransport::Run(Role localRole) {
        try {
            mbedtls::check(mbedtls_ctr_drbg_seed(&mDrbg, mbedtls_entropy_func,
                    &mEntropy, NULL, 0));

            mbedtls::check(mbedtls_ssl_config_defaults(
                    &mConf,
                    localRole == Role::CLIENT ? MBEDTLS_SSL_IS_CLIENT
                    : MBEDTLS_SSL_IS_SERVER,
                    MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT));

            mbedtls_ssl_conf_max_version(&mConf, MBEDTLS_SSL_MAJOR_VERSION_3,
                    MBEDTLS_SSL_MINOR_VERSION_3); // TLS 1.2
            mbedtls_ssl_conf_authmode(&mConf, MBEDTLS_SSL_VERIFY_OPTIONAL);
            mbedtls_ssl_conf_verify(&mConf, DtlsTransport::CertificateCallback, this);
            mbedtls_ssl_conf_rng(&mConf, mbedtls_ctr_drbg_random, &mDrbg);

            auto [crt, pk] = config.mCertificate->credentials();
            mbedtls::check(mbedtls_ssl_conf_own_cert(&mConf, crt, pk));

            mbedtls_ssl_conf_ca_chain(&mConf, crt, NULL);
            mbedtls_ssl_conf_dtls_cookies(&mConf, NULL, NULL, NULL);
            mbedtls_ssl_conf_dtls_srtp_protection_profiles(
                    &mConf, srtpSupportedProtectionProfiles);

            mbedtls::check(mbedtls_ssl_setup(&mSsl, &mConf));

            mbedtls_ssl_set_export_keys_cb(&mSsl, DtlsTransport::ExportKeysCallback, this);

            ssl_bio_ = SSL_BIO_new(BIO_BIO);
            app_bio_ = SSL_BIO_new(BIO_BIO);
            TLS_BIO_make_bio_pair(ssl_bio_, app_bio_);

            mbedtls_ssl_set_bio(&mSsl, ssl_bio_, TLS_BIO_net_send, TLS_BIO_net_recv, NULL);

            this->timer = new Timer(this);
            mbedtls_ssl_set_timer_cb(&mSsl, this, ssl_set_timer, ssl_get_timer);

        } catch (...) {
            mbedtls_entropy_free(&mEntropy);
            mbedtls_ctr_drbg_free(&mDrbg);
            mbedtls_ssl_free(&mSsl);
            mbedtls_ssl_config_free(&mConf);
            throw;
        }

        assertm(localRole == Role::CLIENT || localRole == Role::SERVER,
                "local DTLS role must be 'client' or 'server'");

        Role previousLocalRole = this->localRole;

        if (localRole == previousLocalRole) {
            LError("same local DTLS role provided, doing nothing");

            return;
        }

        // If the previous local DTLS role was 'client' or 'server' do reset.
        if (previousLocalRole == Role::CLIENT || previousLocalRole == Role::SERVER) {
            LInfo("resetting DTLS due to local role change");

            //  Reset();  TBD // arvind
        }

        // Update local role.
        this->localRole = localRole;

        SInfo << ((localRole == Role::CLIENT) ? "running [role:client/active]" : "running [role:server/passive]");

        handshake();
    }

    bool DtlsTransport::handshake() {
        SInfo << "handshake state " << static_cast<int>(this->state);

        if (handshakeDone)
            return true;

        if (this->state != DtlsState::CONNECTING) {
            this->state = DtlsState::CONNECTING;
            if (this->listener) {
                this->listener->OnDtlsTransportConnecting(this);
            }
        }

        int rv = mbedtls_ssl_handshake(&mSsl);
        rv = swrap_error_handler(rv);
        if (rv == 0) {
            handshakeDone = true;
            SInfo << "SSL Handshake over";
            int verify_status = static_cast<int>(mbedtls_ssl_get_verify_result(&mSsl));
            if (verify_status) {
                char buf[512];
                mbedtls_x509_crt_verify_info(buf, sizeof(buf), "::", static_cast<uint32_t>(verify_status));
                if (verify_status & MBEDTLS_X509_BADCERT_NOT_TRUSTED) {
                    SWarn << " Peer certificate is not generally trusted, but accepted: " << buf;
                } else {
                    SError << " Failed ssl cert verification because expired or CN mismatch: " << buf;
                }
            }
            
            ssl_set_timer(this, 0, 0);

            this->state = DtlsState::CONNECTED;
            if (this->listener) {
                this->listener->OnDtlsTransportConnected(this);
            }
        }
        return handshakeDone;
    }

    int DtlsTransport::swrap_error_handler(const int code) {
        if (code == MBEDTLS_ERR_SSL_WANT_WRITE || code == MBEDTLS_ERR_SSL_WANT_READ) {
            stay_uptodate();
        } else if (code == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
            return code;
        } else if (code == MBEDTLS_ERR_SSL_FEATURE_UNAVAILABLE) {
            SError << "This browser or client need more MbedDtls extensions enabled";
        }

        return code;
    }

    void DtlsTransport::stay_uptodate() {
        size_t pending = TLS_BIO_ctrl_pending(app_bio_);
        if (pending > 0) {
            char *mybuf = static_cast<char *>(std::malloc(pending));
            if (!mybuf) {
                SError << "Failed memory allocation in stay_uptodate";
                return;
            }







            int rv = TLS_BIO_read(app_bio_, mybuf, static_cast<int>(pending));

            if (rv < 0 || rv != pending) {
                SWarn << "TLS_BIO_read SSL error ";
            }
            //assert(rv == pending);
            
            
            if (rv > 0 && this->listener) {
                this->listener->OnDtlsTransportSendData(this, reinterpret_cast<uint8_t *>(mybuf),
                        static_cast<size_t>(rv));
            } else if (rv < 0) {
                SError << "TLS_BIO_read failed in stay_uptodate";
            }
            std::free(mybuf);
        }
    }

    int DtlsTransport::CertificateCallback(void *ctx, mbedtls_x509_crt *crt,
            int /*depth*/, uint32_t * /*flags*/) {
        auto t = static_cast<DtlsTransport *>(ctx);
        SInfo << "CertificateCallback";

        std::string fingerprint = rtc::make_fingerprint(crt, t->remoteFingerprint.algorithm);
        return !t->checkFingerprint(fingerprint);
    }

    void DtlsTransport::ExportKeysCallback(void *ctx,
            mbedtls_ssl_key_export_type /*type*/,
            const unsigned char *secret,
            size_t secret_len,
            const unsigned char client_random[32],
            const unsigned char server_random[32],
            mbedtls_tls_prf_types tls_prf_type) {
        auto dtlsTransport = static_cast<DtlsTransport *>(ctx);
        std::memcpy(dtlsTransport->mMasterSecret, secret, std::min(secret_len, sizeof(dtlsTransport->mMasterSecret)));
        std::memcpy(dtlsTransport->mRandBytes, client_random, 32);
        std::memcpy(dtlsTransport->mRandBytes + 32, server_random, 32);
        dtlsTransport->mTlsProfile = tls_prf_type;
    }

    inline bool DtlsTransport::ProcessHandshake() {
        return true;
    }

    void DtlsTransport::ClassDestroy() {
    }

    DtlsTransport::~DtlsTransport() {
        SInfo << "~DtlsTransport() begin";

        shutdown();
                
        if (localRole == Role::SERVER)
            mbedtls_ssl_session_reset(&mSsl);

        mbedtls_entropy_free(&mEntropy);
        mbedtls_ctr_drbg_free(&mDrbg);
        mbedtls_ssl_free(&mSsl);
        mbedtls_ssl_config_free(&mConf);

        if (ssl_bio_) TLS_BIO_free(ssl_bio_);
        if (app_bio_) TLS_BIO_free(app_bio_);
        
        delete this->timer;
        this->timer = nullptr;

        SInfo << "~DtlsTransport over()";
    }

        
    void DtlsTransport::shutdown() {
        SInfo << "Shutdown";
        int ret;
        char error_buf[100];

        // Loop until the notify alert is sent or fails unrecoverably
        SInfo << "Initiating secure closure with mbedtls_ssl_close_notify.";

        do {
            ret = mbedtls_ssl_close_notify(&mSsl);
        } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
                ret == MBEDTLS_ERR_SSL_WANT_READ);

        if (ret == 0) {
            SInfo << "Secure close_notify alert packet transmitted successfully";
            //  ctx->close_notified = 1;
        }

        stay_uptodate();

        if (ret == 0) {
            SInfo << "Success: close_notify packet queued to libuv";
        } else {
            mbedtls_strerror(ret, error_buf, sizeof(error_buf));
            SError << " Error during close_notify: " << error_buf;
        }
        
        SInfo << "Shutdown over";
    }

} // namespace rtc

#else

#include <openssl/asn1.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

#define LOG_OPENSSL_ERROR(desc)                                                \
  do {                                                                         \
    if (ERR_peek_error() == 0)                                                 \
      SError << "OpenSSL error [desc:] " << desc;                              \
    else {                                                                     \
      int64_t err;                                                             \
      while ((err = ERR_get_error()) != 0) {                                   \
        SError << "OpenSSL error [desc:] " << desc << " [error:] "             \
               << ERR_error_string(err, nullptr);                              \
      }                                                                        \
      ERR_clear_error();                                                       \
    }                                                                          \
  } while (false)

inline static int onSslCertificateVerify(int /*preverify_ok*/,
        X509_STORE_CTX *ctx) {
    SSL *ssl = static_cast<SSL *>(
            X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    rtc::DtlsTransport *t =
            static_cast<rtc::DtlsTransport *>(SSL_get_ex_data(ssl, 0));

    X509 *crt = X509_STORE_CTX_get_current_cert(ctx);
    std::string fingerprint =
            rtc::make_fingerprint(crt, t->remoteFingerprint.algorithm);
    return t->checkFingerprint(fingerprint);
}

inline static void onSslInfo(const SSL *ssl, int where, int ret) {
    static_cast<rtc::DtlsTransport *>(SSL_get_ex_data(ssl, 0))
            ->OnSslInfo(where, ret);
}

inline static unsigned int onSslDtlsTimer(SSL * /*ssl*/, unsigned int timerUs) {
    if (timerUs == 0)
        return 100000;
    else if (timerUs >= 4000000)
        return 4000000;
    else
        return 2 * timerUs;
}

namespace rtc {
    static constexpr int DtlsMtu{1350};
    static constexpr int SslReadBufferSize{65536};

    SSL_CTX *DtlsTransport::sslCtx{nullptr};
    uint8_t DtlsTransport::sslReadBuffer[SslReadBufferSize];

    std::map<std::string, DtlsTransport::Role> DtlsTransport::string2Role = {
        {"auto", DtlsTransport::Role::AUTO},
        {"client", DtlsTransport::Role::CLIENT},
        {"server", DtlsTransport::Role::SERVER}
    };

    std::vector<DtlsTransport::SrtpProfileMapEntry> DtlsTransport::srtpProfiles = {
        {DtlsTransport::Profile::AEAD_AES_256_GCM, "SRTP_AEAD_AES_256_GCM"},
        {DtlsTransport::Profile::AEAD_AES_128_GCM, "SRTP_AEAD_AES_128_GCM"},
        {DtlsTransport::Profile::AES_CM_128_HMAC_SHA1_80, "SRTP_AES128_CM_SHA1_80"},
        {DtlsTransport::Profile::AES_CM_128_HMAC_SHA1_32,
            "SRTP_AES128_CM_SHA1_32"}
    };

    //	std::vector<DtlsTransport::Fingerprint>
    // DtlsTransport::localFingerprints;

    void DtlsTransport::ClassDestroy() {

        SInfo << "DtlsTransport::ClassDestroy()";

        //		if (DtlsTransport::privateKey)
        //			EVP_PKEY_free(DtlsTransport::privateKey);
        //		if (DtlsTransport::certificate)
        //			X509_free(DtlsTransport::certificate);
        if (DtlsTransport::sslCtx)
            SSL_CTX_free(DtlsTransport::sslCtx);
    }

    void DtlsTransport::onDtlError() {
        if (DtlsTransport::sslCtx) {
            SSL_CTX_free(DtlsTransport::sslCtx);
            DtlsTransport::sslCtx = nullptr;
        }

        //		if (ecdh)
        //			EC_KEY_free(ecdh);

        base::uv::throwError("SSL context creation failed");
    }

    void DtlsTransport::CreateSslCtx() {
        std::string dtlsSrtpProfiles;
        int ret;

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
        DtlsTransport::sslCtx = SSL_CTX_new(DTLS_method());
#elif (OPENSSL_VERSION_NUMBER >= 0x10001000L)
        DtlsTransport::sslCtx = SSL_CTX_new(DTLSv1_method());
#else
#error "too old OpenSSL version"
#endif

        if (!DtlsTransport::sslCtx) {
            LOG_OPENSSL_ERROR("SSL_CTX_new() failed");
            onDtlError();
            return;
        }

        auto [x509, pkey] = config.mCertificate->credentials();

        ret = SSL_CTX_use_certificate(DtlsTransport::sslCtx, x509);
        if (ret == 0) {
            LOG_OPENSSL_ERROR("SSL_CTX_use_certificate() failed");
            onDtlError();
            return;
        }

        ret = SSL_CTX_use_PrivateKey(DtlsTransport::sslCtx, pkey);
        if (ret == 0) {
            LOG_OPENSSL_ERROR("SSL_CTX_use_PrivateKey() failed");
            onDtlError();
            return;
        }

        ret = SSL_CTX_check_private_key(DtlsTransport::sslCtx);
        if (ret == 0) {
            LOG_OPENSSL_ERROR("SSL_CTX_check_private_key() failed");
            onDtlError();
            return;
        }
        // End new lines

        // Set options.
        SSL_CTX_set_options(DtlsTransport::sslCtx,
                SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_TICKET |
                SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_QUERY_MTU);

        // Don't use sessions cache.
        SSL_CTX_set_session_cache_mode(DtlsTransport::sslCtx, SSL_SESS_CACHE_OFF);

        // Read always as much into the buffer as possible.
        // NOTE: This is the default for DTLS, but a bug in non latest OpenSSL
        // versions makes this call required.
        SSL_CTX_set_read_ahead(DtlsTransport::sslCtx, 1);

        SSL_CTX_set_verify_depth(DtlsTransport::sslCtx, 1);

        // Require certificate from peer.
        SSL_CTX_set_verify(DtlsTransport::sslCtx,
                SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                onSslCertificateVerify);

        // Set SSL info callback.
        SSL_CTX_set_info_callback(DtlsTransport::sslCtx, onSslInfo);

        // Set ciphers.
        ret = SSL_CTX_set_cipher_list(
                DtlsTransport::sslCtx, "ALL:!ADH:!LOW:!EXP:!MD5:!aNULL:!eNULL:@STRENGTH");

        if (ret == 0) {
            LOG_OPENSSL_ERROR("SSL_CTX_set_cipher_list() failed");

            onDtlError();
            return;
        }

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
#elif (OPENSSL_VERSION_NUMBER >= 0x10002000L)
        SSL_CTX_set_ecdh_auto(DtlsTransport::sslCtx, 1);
#else
        EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
        if (!ecdh) {
            LOG_OPENSSL_ERROR("EC_KEY_new_by_curve_name() failed");
            onDtlError();
            return;
        }
        if (SSL_CTX_set_tmp_ecdh(DtlsTransport::sslCtx, ecdh) != 1) {
            LOG_OPENSSL_ERROR("SSL_CTX_set_tmp_ecdh() failed");
            EC_KEY_free(ecdh);
            onDtlError();
            return;
        }
        EC_KEY_free(ecdh);
#endif

        for (auto it = DtlsTransport::srtpProfiles.begin();
                it != DtlsTransport::srtpProfiles.end(); ++it) {
            if (it != DtlsTransport::srtpProfiles.begin())
                dtlsSrtpProfiles += ":";

            SrtpProfileMapEntry *profileEntry = std::addressof(*it);
            dtlsSrtpProfiles += profileEntry->name;
        }

        SDebug << "setting SRTP profiles for DTLS: " << dtlsSrtpProfiles;

        ret = SSL_CTX_set_tlsext_use_srtp(DtlsTransport::sslCtx, dtlsSrtpProfiles.c_str());
        if (ret != 0) {
            LError("SSL_CTX_set_tlsext_use_srtp() failed when entering ", dtlsSrtpProfiles.c_str());
            LOG_OPENSSL_ERROR("SSL_CTX_set_tlsext_use_srtp() failed");
        }
    }

    DtlsTransport::DtlsTransport(Listener *listener) : listener(listener) {
        this->ssl = SSL_new(DtlsTransport::sslCtx);

        if (!this->ssl) {
            LOG_OPENSSL_ERROR("SSL_new() failed");
            goto error;
        }

        SSL_set_ex_data(this->ssl, 0, static_cast<void *>(this));

        this->sslBioFromNetwork = BIO_new(BIO_s_mem());
        if (!this->sslBioFromNetwork) {
            LOG_OPENSSL_ERROR("BIO_new() failed");
            SSL_free(this->ssl);
            goto error;
        }

        this->sslBioToNetwork = BIO_new(BIO_s_mem());
        if (!this->sslBioToNetwork) {
            LOG_OPENSSL_ERROR("BIO_new() failed");
            BIO_free(this->sslBioFromNetwork);
            SSL_free(this->ssl);
            goto error;
        }

        SSL_set_bio(this->ssl, this->sslBioFromNetwork, this->sslBioToNetwork);
        SSL_set_mtu(this->ssl, DtlsMtu);
        DTLS_set_link_mtu(this->ssl, DtlsMtu);
        DTLS_set_timer_cb(this->ssl, onSslDtlsTimer);

        this->timer = new Timer(this);
        return;

error:
        if (this->sslBioFromNetwork) BIO_free(this->sslBioFromNetwork);
        if (this->sslBioToNetwork) BIO_free(this->sslBioToNetwork);
        if (this->ssl) SSL_free(this->ssl);

        base::uv::throwError("DtlsTransport instance creation failed");
    }

    DtlsTransport::~DtlsTransport() {
        if (IsRunning()) {
            SSL_shutdown(this->ssl);
            SendPendingOutgoingDtlsData();
        }

        if (this->ssl) {
            SSL_free(this->ssl);
            this->ssl = nullptr;
            this->sslBioFromNetwork = nullptr;
            this->sslBioToNetwork = nullptr;
        }

        delete this->timer;
        this->timer = nullptr;
    }

    void DtlsTransport::Dump() const {
        std::string state{"new"};
        std::string role{"none "};

        switch (this->state) {
            case DtlsState::CONNECTING: state = "connecting"; break;
            case DtlsState::CONNECTED:  state = "connected";  break;
            case DtlsState::FAILED:     state = "failed";     break;
            case DtlsState::CLOSED:     state = "closed";     break;
            default:;
        }

        switch (this->localRole) {
            case Role::AUTO:   role = "auto";   break;
            case Role::SERVER: role = "server"; break;
            case Role::CLIENT: role = "client"; break;
            default:;
        }

        LTrace("<DtlsTransport>");
        LTrace("  state           : ", state);
        LTrace("  role            : ", role);
        LTrace("  handshake done: : ", (this->handshakeDone ? "yes" : "no"));
        LTrace("</DtlsTransport>");
    }

    void DtlsTransport::Run(Role localRole) {
        assertm(localRole == Role::CLIENT || localRole == Role::SERVER,
                "local DTLS role must be 'client' or 'server'");

        Role previousLocalRole = this->localRole;

        if (localRole == previousLocalRole) {
            LError("same local DTLS role provided, doing nothing");
            return;
        }

        if (previousLocalRole == Role::CLIENT || previousLocalRole == Role::SERVER) {
            LTrace("resetting DTLS due to local role change");
            Reset();
        }

        this->localRole = localRole;
        this->state = DtlsState::CONNECTING;
        if (this->listener) {
            this->listener->OnDtlsTransportConnecting(this);
        }

        switch (this->localRole) {
            case Role::CLIENT:
            {
                SInfo << "running [role:client/active]";
                SSL_set_connect_state(this->ssl);
                SSL_do_handshake(this->ssl);
                SendPendingOutgoingDtlsData();
                SetTimeout();
                break;
            }
            case Role::SERVER:
            {
                SInfo << "running [role:server/passive]";
                SSL_set_accept_state(this->ssl);
                SSL_do_handshake(this->ssl);
                break;
            }
            default:
            {
                SError << "invalid local DTLS role";
                std::exit(0);
            }
        }
    }

    void DtlsTransport::ProcessDtlsData(const uint8_t *data, size_t len) {
        SInfo << "ProcessDtlsData " << len;

        if (!IsRunning()) {
            LError("cannot process data while not running");
            return;
        }

        int written = BIO_write(this->sslBioFromNetwork, static_cast<const void *>(data), static_cast<int>(len));
        if (written != static_cast<int>(len)) {
            LWarn("OpenSSL BIO_write() wrote less ", static_cast<size_t>(written), " than given data ", len);
        }

        int read = SSL_read(this->ssl, static_cast<void *>(DtlsTransport::sslReadBuffer), SslReadBufferSize);

        SendPendingOutgoingDtlsData();

        if (!CheckStatus(read))
            return;

        if (!SetTimeout())
            return;

        if (read > 0) {
            if (!this->handshakeDone) {
                LWarn("ignoring application data received while DTLS handshake not done");
                return;
            }

            if (this->listener) {
                this->listener->OnDtlsTransportApplicationDataReceived(
                        this, reinterpret_cast<uint8_t *>(DtlsTransport::sslReadBuffer),
                        static_cast<size_t>(read));
            }
        }
    }

    void DtlsTransport::SendApplicationData(const uint8_t *data, size_t len) {
        if (this->state != DtlsState::CONNECTED) {
            LWarn("cannot send application data while DTLS is not fully connected");
            return;
        }

        if (len == 0) {
            LWarn("ignoring 0 length data");
            return;
        }

        int written = SSL_write(this->ssl, static_cast<const void *>(data), static_cast<int>(len));

        if (written < 0) {
            LOG_OPENSSL_ERROR("SSL_write() failed");
            if (!CheckStatus(written))
                return;
        } else if (written != static_cast<int>(len)) {
            LWarn("OpenSSL SSL_write() wrote less ", written, " than given data bytes ", len);
        }

        SendPendingOutgoingDtlsData();
    }

    void DtlsTransport::Reset() {
        if (!IsRunning())
            return;

        LWarn("resetting DTLS transport");

        if (this->timer) {
            this->timer->Stop();
        }

        SSL_shutdown(this->ssl);

        this->localRole = Role::NONE;
        this->state = DtlsState::NEW;
        this->handshakeDone = false;
        this->handshakeDoneNow = false;

        int ret = SSL_clear(this->ssl);
        if (ret == 0)
            ERR_clear_error();
    }

    inline bool DtlsTransport::CheckStatus(int returnCode) {
        SInfo << "DtlsTransport::CheckStatus()";

        bool wasHandshakeDone = this->handshakeDone;
        int err = SSL_get_error(this->ssl, returnCode);

        switch (err) {
            case SSL_ERROR_NONE: break;
            case SSL_ERROR_SSL: LOG_OPENSSL_ERROR("SSL status: SSL_ERROR_SSL"); break;
            case SSL_ERROR_WANT_READ: break;
            case SSL_ERROR_WANT_WRITE: LWarn("SSL status: SSL_ERROR_WANT_WRITE"); break;
            case SSL_ERROR_WANT_X509_LOOKUP: LTrace("SSL status: SSL_ERROR_WANT_X509_LOOKUP"); break;
            case SSL_ERROR_SYSCALL: LOG_OPENSSL_ERROR("SSL status: SSL_ERROR_SYSCALL"); break;
            case SSL_ERROR_ZERO_RETURN: break;
            case SSL_ERROR_WANT_CONNECT: LWarn("SSL status: SSL_ERROR_WANT_CONNECT"); break;
            case SSL_ERROR_WANT_ACCEPT: LWarn("SSL status: SSL_ERROR_WANT_ACCEPT"); break;
            default: LWarn("SSL status: unknown error");
        }

        if (this->handshakeDoneNow) {
            this->handshakeDoneNow = false;
            this->handshakeDone = true;

            SInfo << "handshake done";
            if (this->timer) {
                this->timer->Stop();
            }

            if (!wasHandshakeDone)
                return ProcessHandshake();

            return true;
        } else if (((SSL_get_shutdown(this->ssl) & SSL_RECEIVED_SHUTDOWN) != 0) ||
                err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL) {
            if (this->state == DtlsState::CONNECTED) {
                LTrace("disconnected");
                Reset();
                this->state = DtlsState::CLOSED;
                if (this->listener) {
                    this->listener->OnDtlsTransportClosed(this);
                }
            } else {
                LWarn("connection failed");
                Reset();
                this->state = DtlsState::FAILED;
                if (this->listener) {
                    this->listener->OnDtlsTransportFailed(this);
                }
            }
            return false;
        } else {
            return true;
        }
    }

    inline void DtlsTransport::SendPendingOutgoingDtlsData() {
        if (BIO_eof(this->sslBioToNetwork))
            return;

        char *data{nullptr};
        int64_t read = BIO_get_mem_data(this->sslBioToNetwork, &data);

        if (read <= 0 || !data)
            return;

        if (this->listener) {
            this->listener->OnDtlsTransportSendData(
                    this, reinterpret_cast<uint8_t *>(data), static_cast<size_t>(read));
        }

        (void) BIO_reset(this->sslBioToNetwork);
    }

    inline bool DtlsTransport::SetTimeout() {

        SInfo << "DtlsTransport::SetTimeout()";

        assertm(this->state == DtlsState::CONNECTING ||
                this->state == DtlsState::CONNECTED,
                "invalid DTLS state");

        int64_t ret;
        uv_timeval_t dtlsTimeout{0, 0};
        uint64_t timeoutMs;

        // NOTE: If ret == 0 then ignore the value in dtlsTimeout.
        // NOTE: No DTLSv_1_2_get_timeout() or DTLS_get_timeout() in
        // OpenSSL 1.1.0-dev.
        ret = DTLSv1_get_timeout(this->ssl,
                static_cast<void *> (&dtlsTimeout)); // NOLINT

        if (ret == 0)
            return true;

        timeoutMs = (dtlsTimeout.tv_sec * static_cast<uint64_t> (1000)) +
                (dtlsTimeout.tv_usec / 1000);

        if (timeoutMs == 0) {
            return true;
        } else if (timeoutMs < 30000) {
            LDebug("DTLS timer set in ms", timeoutMs);
            if (this->timer) {
                this->timer->Start(timeoutMs);
            }
            return true;
        } else {
            SWarn << "DTLS timeout too high ms, resetting DLTS: " << timeoutMs;
            Reset();
            this->state = DtlsState::FAILED;
            if (this->listener) {
                this->listener->OnDtlsTransportFailed(this);
            }
            return false;
        }
    }

    inline void DtlsTransport::OnSslInfo(int where, int ret) {

        int w = where & -SSL_ST_MASK;
        const char *role;

        if ((w & SSL_ST_CONNECT) != 0)
            role = "client";
        else if ((w & SSL_ST_ACCEPT) != 0)
            role = "server";
        else
            role = "undefined";

        if ((where & SSL_CB_LOOP) != 0) {
            LTrace("role: ", role, " action: ", SSL_state_string_long(this->ssl));
        } else if ((where & SSL_CB_ALERT) != 0) {
            const char *alertType;

            switch (*SSL_alert_type_string(ret)) {
                case 'W': alertType = "warning"; break;
                case 'F': alertType = "fatal";   break;
                default:  alertType = "undefined";
            }

            if ((where & SSL_CB_READ) != 0) {
                LWarn("received DTLS ", alertType, " alert: ", SSL_alert_desc_string_long(ret));
            } else if ((where & SSL_CB_WRITE) != 0) {
                LTrace("sending DTLS ", alertType, " alert: ", SSL_alert_desc_string_long(ret));
            } else {
                LTrace("DTLS ", alertType, " alert: ", SSL_alert_desc_string_long(ret));
            }
        } else if ((where & SSL_CB_EXIT) != 0) {
            if (ret == 0) {
                LTrace("role: ", role, " failed: ", SSL_state_string_long(this->ssl));
            } else if (ret < 0) {
                LTrace("role: ", role, " waiting: ", SSL_state_string_long(this->ssl));
            }
        } else if ((where & SSL_CB_HANDSHAKE_START) != 0) {
            LTrace("DTLS handshake start");
        } else if ((where & SSL_CB_HANDSHAKE_DONE) != 0) {
            LTrace("DTLS handshake done");

            this->handshakeDoneNow = true;
        }

        // NOTE: checking SSL_get_shutdown(this->ssl) & SSL_RECEIVED_SHUTDOWN here
        // upon receipt of a close alert does not work (the flag is set after this
        // callback).
    }

    inline void DtlsTransport::OnTimer(Timer * /*timer*/) {

        // Workaround for https://github.com/openssl/openssl/issues/7998.
        if (this->handshakeDone) {
            LDebug("handshake is done so return");

            return;
        }

        DTLSv1_handle_timeout(this->ssl);

        // If required, send DTLS data.
        SendPendingOutgoingDtlsData();

        // Set the DTLS timer again.
        SetTimeout();
    }

    inline bool DtlsTransport::CheckRemoteFingerprint() {

        SInfo << "DtlsTransport::CheckRemoteFingerprint()";

        assertm(this->remoteFingerprint.algorithm !=
                CertificateFingerprint::Algorithm::NONE,
                "remote fingerprint not set");

        X509 *certificate = SSL_get_peer_certificate(this->ssl);
        if (!certificate) {
            SWarn << "no certificate was provided by the peer";

            return false;
        }

        uint8_t binaryFingerprint[EVP_MAX_MD_SIZE];
        unsigned int size{0};
        char hexFingerprint[(EVP_MAX_MD_SIZE * 3) + 1];
        const EVP_MD *hashFunction;

        switch (this->remoteFingerprint.algorithm) {
            case CertificateFingerprint::Algorithm::Sha1:   hashFunction = EVP_sha1();   break;
            case CertificateFingerprint::Algorithm::Sha224: hashFunction = EVP_sha224(); break;
            case CertificateFingerprint::Algorithm::Sha256: hashFunction = EVP_sha256(); break;
            case CertificateFingerprint::Algorithm::Sha384: hashFunction = EVP_sha384(); break;
            case CertificateFingerprint::Algorithm::Sha512: hashFunction = EVP_sha512(); break;
            default:
                MS_ABORT("unknown algorithm");
        }

        // Compare the remote fingerprint with the value given via signaling.
        ret = X509_digest(certificate, hashFunction, binaryFingerprint, &size);

        if (ret == 0) {
            SError << "X509_digest() failed";

            X509_free(certificate);

            return false;
        }

        // Convert to hexadecimal format in uppercase with colons.
        for (unsigned int i{0}; i < size; ++i) {
            std::sprintf(hexFingerprint + (i * 3), "%.2X:", binaryFingerprint[i]);
        }
        hexFingerprint[(size * 3) - 1] = '\0';

        if (this->remoteFingerprint.value != hexFingerprint) {
            SWarn << "fingerprint in the remote certificate " << hexFingerprint
                    << " does not match the announced one  ( "
                    << this->remoteFingerprint.value.c_str() << " )";

            X509_free(certificate);

            return false;
        }

        SInfo << "valid remote fingerprint";

        // Get the remote certificate in PEM format.

        BIO *bio = BIO_new(BIO_s_mem());

        // Ensure the underlying BUF_MEM structure is also freed.
        // NOTE: Avoid stupid "warning: value computed is not used [-Wunused-value]"
        // since BIO_set_close() always returns 1.
        (void) BIO_set_close(bio, BIO_CLOSE);

        ret = PEM_write_bio_X509(bio, certificate);

        if (ret != 1) {
            LOG_OPENSSL_ERROR("PEM_write_bio_X509() failed");

            X509_free(certificate);
            BIO_free(bio);

            return false;
        }

        BUF_MEM *mem;
        BIO_get_mem_ptr(bio, &mem);

        if (!mem || !mem->data || mem->length == 0u) {
            LOG_OPENSSL_ERROR("BIO_get_mem_ptr() failed");

            X509_free(certificate);
            BIO_free(bio);

            return false;
        }

        this->remoteCert = std::string(mem->data, mem->length);

        X509_free(certificate);
        BIO_free(bio);

        return true;
    }

    inline bool DtlsTransport::ProcessHandshake() {

        SInfo << "DtlsTransport::ProcessHandshake()";

        assertm(this->handshakeDone, "handshake not done yet");
        assertm(this->remoteFingerprint.algorithm !=
                CertificateFingerprint::Algorithm::NONE,
                "remote fingerprint not set");

        // Validate the remote fingerprint.
        if (!CheckRemoteFingerprint()) {
            Reset();

            // Set state and notify the listener.
            this->state = DtlsState::FAILED;
            if (this->listener) {
                this->listener->OnDtlsTransportFailed(this);
            }
            return false;
        }

        //		// Get the negotiated SRTP profile.
        //		rtc::SrtpSession::Profile srtpProfile =
        // GetNegotiatedSrtpProfile();
        //
        //		if (srtpProfile != rtc::SrtpSession::Profile::NONE)
        //		{
        //			// Extract the SRTP keys (will notify the listener with
        // them). 			ExtractSrtpKeys(srtpProfile);
        //
        //			return true;
        //		}
        //
        //		// NOTE: We assume that "use_srtp" DTLS extension is required
        // even if
        // there is no audio/video.

        this->state = DtlsState::CONNECTED;
        this->listener->OnDtlsTransportConnected(this);

        SWarn << "SRTP profile not negotiated";

        return true;

        Reset();

        // Set state and notify the listener.
        this->state = DtlsState::FAILED;
        this->listener->OnDtlsTransportFailed(this);

        return false;
    }







} // namespace rtc

#endif

namespace rtc {
    void DtlsTransport::ClassInit() {
        SInfo << "DtlsTransport::ClassInit()";
        config.init();
        CreateSslCtx();
    }

    bool DtlsTransport::checkFingerprint(const std::string &fingerprint) {
        if (remoteFingerprint.value.empty())
            return false;

        //	if (config.disableFingerprintVerification) {
        //		SInfo << "Skipping fingerprint validation";
        //		return true;
        //	}

        auto expectedFingerprint = remoteFingerprint.value;
        if (expectedFingerprint == fingerprint) {
            SInfo << "Valid fingerprint \"" << fingerprint << "\"";
            return true;
        }

        SError << "Invalid fingerprint \"" << fingerprint << "\", expected \""
                << expectedFingerprint << "\"";
        return false;
    }

    bool DtlsTransport::SetRemoteFingerprint(CertificateFingerprint fingerprint) {

        assertm(fingerprint.algorithm != CertificateFingerprint::Algorithm::NONE,
                "no fingerprint algorithm provided");

        this->remoteFingerprint = fingerprint;

        // The remote fingerpring may have been set after DTLS handshake was done,
        // so we may need to process it now.
        if (this->handshakeDone && this->state != DtlsState::CONNECTED) {
            LTrace("handshake already done, processing it right now");

            return ProcessHandshake();
        }

        return true;
    }

}
