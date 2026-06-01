

#include "DtlsTransport.h"
#include "base/error.h"
#include "base/logger.h"
// #include "Settings.h"
// #include "Utils.h"

#include "IceServer.h"
#include "base/application.h"
#include "net/certificate.h"
#include <chrono>
#include <cstdio>  // std::sprintf(), std::fopen()
#include <cstring> // std::memcpy(), std::strcmp()
#include <uv.h>

extern ConfCert config;
using namespace base;


//#define USE_MBEDTLS 1

#if USE_MBEDTLS

namespace rtc {

    inline void DtlsTransport::OnTimer(Timer * /*timer*/) { // dtls

        status = 2;
        SInfo << "mbedtls_ssl_handshake ";
        handshake();
    }

    void on_uv_timer(uv_timer_t *handle) {
        //        DtlsTransport *c = (DtlsTransport *) handle->data;
        //
        //        if (!c->timer_int_passed)
        //            c->timer_int_passed = 1;
        //        else {
        //            c->timer_fin_passed = 1;
        //            uv_timer_stop(handle);
        //        }
        //
        //        SInfo << "mbedtls_ssl_handshake ";
        //        // mbedtls_ssl_handshake(&c->mSsl);
        //        c->handshake();

        DtlsTransport *t_ctx = (DtlsTransport *) handle->data;
        t_ctx->status = 2;

        SInfo << "mbedtls_ssl_handshake ";
        t_ctx->handshake();
    }

    void ssl_set_timer(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
        //        DtlsTransport *c = (DtlsTransport *) ctx;
        //
        //        if (fin_ms == 0) {
        //            uv_timer_stop(&c->timer1);
        //            return;
        //        }
        //        c->timer_int_passed = c->timer_fin_passed = 0;
        //
        //
        //        uv_timer_start(&c->timer1, on_uv_timer, int_ms, fin_ms - int_ms);

        DtlsTransport *t_ctx = (DtlsTransport *) ctx;
        t_ctx->intermediate_ms = int_ms;
        t_ctx->final_ms = fin_ms;

        if (fin_ms == 0) {
            // uv_timer_stop(&t_ctx->timer1);
            t_ctx->timer->Stop();
            std::cout << "uv_timer_stop" << std::endl << std::flush;
            t_ctx->status = -1;
            return;
        }

        t_ctx->start_time = uv_now(t_ctx->timer->GetUVloop());
        t_ctx->status = 0;

        // uv_timer_start(&t_ctx->timer1, on_uv_timer, fin_ms, 0);
        
       /// if(t_ctx->GetLocalRole() == DtlsTransport::Role::CLIENT )
        t_ctx->timer->Start(fin_ms, 0);
        // std::cout << "dtls_set_timer" << std::endl << std::flush;
    }

    int ssl_get_timer(void *ctx) {
        //        DtlsTransport *c = (DtlsTransport *) ctx;
        //        if (c->timer_fin_passed) return 2;
        //        if (c->timer_int_passed) return 1;
        //        return 0;

        DtlsTransport *t_ctx = (DtlsTransport *) ctx;
        if (t_ctx->status == -1)
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
                                       const char *str)
    {
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
        
         mbedtls_debug_set_threshold(2);

    // 3. Register the callback with the SSL configuration
    // The last argument (nullptr) is passed as the 'void *ctx' to your callback
        mbedtls_ssl_conf_dbg(&mConf, mbedtls_debug_callback, nullptr);
    
        mbedtls_ssl_conf_authmode(&mConf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    }

    DtlsTransport::~DtlsTransport() {
        // stop();

        shutdown();

        SDebug << "Destroying DTLS transport";
        mbedtls_entropy_free(&mEntropy);
        mbedtls_ctr_drbg_free(&mDrbg);
        mbedtls_ssl_free(&mSsl);
        mbedtls_ssl_config_free(&mConf);
    }

    inline bool DtlsTransport::CheckRemoteFingerprint() {
        return true;
    }

    //        void DtlsTransport::Reset()
    //	{
    //        }

    void DtlsTransport::SendApplicationData(const uint8_t *data, size_t len) {

        if (!len || !this->handshakeDone)
            return;

        STrace << "Send size=" << len;

        int ret;
        // std::lock_guard lock(mSslMutex);
        if (len > size_t(mbedtls_ssl_get_max_out_record_payload(&mSsl)))
            return;

        //
        //                int ret;
        //                do {
        //                        std::lock_guard lock(mSslMutex);
        //                        if (len >
        //                        size_t(mbedtls_ssl_get_max_out_record_payload(&mSsl)))
        //                                return ;
        //
        //                       // mCurrentDscp = message->dscp;
        //                        ret = mbedtls_ssl_write(&mSsl,
        //                        reinterpret_cast<const unsigned char *>(data),
        //                                                len);
        //                } while (!mbedtls::check(ret));
        //
        //                return ;

        size_t rv;

        rv = (size_t) mbedtls_ssl_write(&mSsl, (const unsigned char *) data, len);
        // if (r <= 0) { swrap_error_handler(r); }  // we are not sure why Mbedtls
        // does not handle error, though openssl does it I have refered
        // https://github.com/yodaos-project/ShadowNode.git
        /*
         if (rv <= 0)
         {
              SError << "mbedtls_ssl_write failed";
         }
         */

        size_t pending = 0;

        char *encoded_data = nullptr;

        if ((pending = TLS_BIO_ctrl_pending(app_bio_)) > 0) {

            encoded_data = (char *) malloc(pending);
            // encoded_data.len = pending;

            rv = TLS_BIO_read(app_bio_, encoded_data, pending);
            // data2encode->len = rv;
            // assert(rv == len);
            //_socket->Write(  encoded_data, rv , cb);

            this->listener->OnDtlsTransportSendData(this, (uint8_t *) encoded_data,
                    static_cast<size_t> (rv));

            // cb = nullptr;
            free(encoded_data);
        } else {
            SError << "SSL Error Encoding";
        }
    }

    void DtlsTransport::ProcessDtlsData(const uint8_t *data, size_t len) {
        SInfo << "ProcessDtlsData " << len;

        TLS_BIO_write(app_bio_, (const char *) data, len);

        if (!this->handshakeDone) //// handshake shoud be callled from client only
        {
            if (!handshake())
                return;
        }

        while (true) {

            memset((void *) data, 0, len);

            int rv = -1;
            rv = mbedtls_ssl_read(&mSsl, (unsigned char *) data, len);
            rv = swrap_error_handler(rv);

            if (rv > 0) {

                // _socket->on_read(data,  rv) ;

                this->listener->OnDtlsTransportApplicationDataReceived(
                        this, (uint8_t *) data, static_cast<size_t> (rv));

            } else if (rv == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                // jerry_value_t fn = iotjs_jval_get_property(jthis, "onclose");
                SInfo << "MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY";

                this->state = DtlsState::CLOSED;
                this->listener->OnDtlsTransportClosed(this);

                break;
            } else if (rv == MBEDTLS_ERR_SSL_WANT_READ ||
                    rv == MBEDTLS_ERR_SSL_WANT_WRITE) {
                break;
            } else {
                // SError << getTLSError(rv);
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
            
           // mbedtls_ssl_set_mtu(&mSsl, 1200);

            mbedtls::check(mbedtls_ssl_setup(&mSsl, &mConf));

            mbedtls_ssl_set_export_keys_cb(&mSsl, DtlsTransport::ExportKeysCallback,
                    this);

            // mbedtls_ssl_set_bio(&mSsl, this, WriteCallback, ReadCallback, NULL);

            ssl_bio_ = SSL_BIO_new(BIO_BIO);
            app_bio_ = SSL_BIO_new(BIO_BIO);
            TLS_BIO_make_bio_pair(ssl_bio_, app_bio_);

            mbedtls_ssl_set_bio(&mSsl, ssl_bio_, TLS_BIO_net_send, TLS_BIO_net_recv,
                    NULL);

            // mbedtls_ssl_set_timer_cb(&mSsl, this, SetTimerCallback,
            // GetTimerCallback);

            // uv_timer_init(Application::uvGetLoop(), &timer1);
            // timer1.data = this;
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
            LTrace("resetting DTLS due to local role change");

            //  Reset();  TBD // arvind
        }

        // Update local role.
        this->localRole = localRole;
        
        SInfo <<   ((localRole == Role::CLIENT)? "running [role:client]":"running [role:server]"); 

        handshake();
    }

    bool DtlsTransport::handshake() //// handshake shoud be callled from client only
    {

        SInfo << "handshake state " << (int) this->state;

        if (handshakeDone)
            return true;

        if( this->state != DtlsState::CONNECTING)
        {
            this->state = DtlsState::CONNECTING;
            this->listener->OnDtlsTransportConnecting(this);
            
            // mbedtls_ssl_set_mtu(&mSsl, 1200);
            
        }

        int rv = 0;
        rv = mbedtls_ssl_handshake(&mSsl);
        rv = swrap_error_handler(rv);
        if (rv == 0) {
            handshakeDone = true;
             SInfo << "SSL Handshake over";
            int verify_status = (int) mbedtls_ssl_get_verify_result(&mSsl);
            if (verify_status) {
                char buf[512];
                mbedtls_x509_crt_verify_info(buf, sizeof (buf),
                        "::", (uint32_t) verify_status);
                // mbedtls_printf("%s\n", buf);
                if (verify_status & MBEDTLS_X509_BADCERT_NOT_TRUSTED) {
                    // printf("Peer certificate is not generally trusted, but accepted:
                    // %s\n", buf);
                    SWarn << " Peer certificate is not generally trusted, but accepted"
                            << buf;
                    // This is expected for self-signed certificates. Proceed securely.
                } else {
                    // printf("Verification failed with critical errors: %s\n", buf);
                    //  Abort connection as there are unexpected errors like expiration or
                    //  CN mismatch
                    SError
                            << " Failed ssl cert verification because expired or CN mismatch "
                            << buf;
                }
            }

            this->state = DtlsState::CONNECTED;
            this->listener->OnDtlsTransportConnected(this);
        }
        return handshakeDone;
    }

    // handle only non fatal error currently

    int DtlsTransport::swrap_error_handler(const int code) {

        if (code == MBEDTLS_ERR_SSL_WANT_WRITE || code == MBEDTLS_ERR_SSL_WANT_READ) {
            stay_uptodate();
        } else if (code == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
            return code;
        }
        else if(  code ==MBEDTLS_ERR_SSL_FEATURE_UNAVAILABLE)
       {
            SError << "This browser or client need more MbedDtls extensions enabled  ";
       }

        return code;
    }

    void DtlsTransport::stay_uptodate() {

        size_t pending = TLS_BIO_ctrl_pending(app_bio_);
        if (pending > 0) {

            // Need to free the memory
            char *mybuf;

            mybuf = (char *) malloc(pending);

            int rv = TLS_BIO_read(app_bio_, mybuf, pending);
            assert(rv == pending);

            // _socket->Write( mybuf, rv, cb);

            this->listener->OnDtlsTransportSendData(this, (uint8_t *) mybuf,
                    static_cast<size_t> (rv));

            // SInfo << "stay_uptodate "  <<  rv ;

            // assert(rv == pending);

            free(mybuf);
            mybuf = 0;
        }
    }

    //inline void DtlsTransport::SendPendingOutgoingDtlsData() {}


    //inline bool DtlsTransport::SetTimeout() { return true; }

    int DtlsTransport::CertificateCallback(void *ctx, mbedtls_x509_crt *crt,
            int /*depth*/, uint32_t * /*flags*/) {
        auto t = static_cast<DtlsTransport *> (ctx);

        SInfo <<  "CertificateCallback";
        
        string fingerprint =
                rtc::make_fingerprint(crt, t->remoteFingerprint.algorithm);
        //        std::transform(fingerprint.begin(), fingerprint.end(),
        //        fingerprint.begin(),
        //                [](char c) {
        //                    return char(std::toupper(c)); });
        return !t->checkFingerprint(fingerprint);
    }

    void DtlsTransport::ExportKeysCallback(void *ctx,
            mbedtls_ssl_key_export_type /*type*/,
            const unsigned char *secret,
            size_t secret_len,
            const unsigned char client_random[32],
            const unsigned char server_random[32],
            mbedtls_tls_prf_types tls_prf_type) {
        auto dtlsTransport = static_cast<DtlsTransport *> (ctx);
        std::memcpy(dtlsTransport->mMasterSecret, secret, secret_len);
        std::memcpy(dtlsTransport->mRandBytes, client_random, 32);
        std::memcpy(dtlsTransport->mRandBytes + 32, server_random, 32);
        dtlsTransport->mTlsProfile = tls_prf_type;
    }

    inline bool DtlsTransport::ProcessHandshake() {
        //
        return true;
    }

    void DtlsTransport::ClassDestroy() {

        // TBD
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
            mbedtls_strerror(ret, error_buf, sizeof (error_buf));
            SError << " Error during close_notify " << error_buf;
        }

        /* Ignore other errors, the connection may be closed or unusable */

        //    mbedtls_ctr_drbg_free(&_ctr_drbg);
        //    mbedtls_entropy_free(&_entropy);
        //    mbedtls_ssl_config_free(&_ssl_conf);
        //    mbedtls_x509_crt_free(& _cacert );
        //
        //    if(server)
        //    {
        //        mbedtls_pk_free( &pkey );
        //    }
        //
        //
        //    mbedtls_ssl_free(&_ssl);
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

/* Static methods for OpenSSL callbacks. */



inline static int onSslCertificateVerify(int /*preverify_ok*/,
        X509_STORE_CTX *ctx) {
    SSL *ssl = static_cast<SSL *> (
            X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    rtc::DtlsTransport *t =
            static_cast<rtc::DtlsTransport *> (SSL_get_ex_data(ssl, 0));

    X509 *crt = X509_STORE_CTX_get_current_cert(ctx);
    string fingerprint =
            rtc::make_fingerprint(crt, t->remoteFingerprint.algorithm);
    return t->checkFingerprint(fingerprint);
}

inline static void onSslInfo(const SSL *ssl, int where, int ret) {
    static_cast<rtc::DtlsTransport *> (SSL_get_ex_data(ssl, 0))
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
    /* Static. */

    static constexpr int DtlsMtu{1350};
    static constexpr int SslReadBufferSize{65536};
    //// AES-HMAC: http://tools.ietf.org/html/rfc3711
    //static constexpr size_t SrtpMasterKeyLength{16};
    //static constexpr size_t SrtpMasterSaltLength{14};
    //static constexpr size_t SrtpMasterLength{SrtpMasterKeyLength +
    //                                         SrtpMasterSaltLength};
    //// AES-GCM: http://tools.ietf.org/html/rfc7714
    //static constexpr size_t SrtpAesGcm256MasterKeyLength{32};
    //static constexpr size_t SrtpAesGcm256MasterSaltLength{12};
    //static constexpr size_t SrtpAesGcm256MasterLength{
    //    SrtpAesGcm256MasterKeyLength + SrtpAesGcm256MasterSaltLength};
    //static constexpr size_t SrtpAesGcm128MasterKeyLength{16};
    //static constexpr size_t SrtpAesGcm128MasterSaltLength{12};
    //static constexpr size_t SrtpAesGcm128MasterLength{
    //    SrtpAesGcm128MasterKeyLength + SrtpAesGcm128MasterSaltLength};





    // X509* DtlsTransport::certificate{ nullptr };
    // EVP_PKEY* DtlsTransport::privateKey{ nullptr };
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
        EC_KEY * ecdh{nullptr};
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

        if (!DtlsTransport::sslCtx) {
            LOG_OPENSSL_ERROR("SSL_CTX_new() failed");

            onDtlError();
            return;
        }

        //                if (
        //		  Settings::configuration.dtlsCertificateFile.empty() ||
        //		  Settings::configuration.dtlsPrivateKeyFile.empty())

        auto [x509, pkey] = config.mCertificate->credentials();

        {

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
        }


        //
        // if(server)
        if (1) {
            // New lines //for server side only

            ret = SSL_CTX_check_private_key(DtlsTransport::sslCtx);

            if (ret == 0) {
                LOG_OPENSSL_ERROR("SSL_CTX_check_private_key() failed");

                onDtlError();
                return;
            }
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

        if (!ecdh) {
            LOG_OPENSSL_ERROR("EC_KEY_new_by_curve_name() failed");

            goto error;
        }

        if (SSL_CTX_set_tmp_ecdh(DtlsTransport::sslCtx, ecdh) != 1) {
            LOG_OPENSSL_ERROR("SSL_CTX_set_tmp_ecdh() failed");

            goto error;
        }

        EC_KEY_free(ecdh);
        ecdh = nullptr;
#endif

        // Set the "use_srtp" DTLS extension.
        for (auto it = DtlsTransport::srtpProfiles.begin();
                it != DtlsTransport::srtpProfiles.end(); ++it) {
            if (it != DtlsTransport::srtpProfiles.begin())
                dtlsSrtpProfiles += ":";

            SrtpProfileMapEntry *profileEntry = std::addressof(*it);
            dtlsSrtpProfiles += profileEntry->name;
        }

        SDebug << "setting SRTP profiles for DTLS: " << dtlsSrtpProfiles;

        // NOTE: This function returns 0 on success.
        ret = SSL_CTX_set_tlsext_use_srtp(DtlsTransport::sslCtx,
                dtlsSrtpProfiles.c_str());

        if (ret != 0) {
            LError("SSL_CTX_set_tlsext_use_srtp() failed when entering ",
                    dtlsSrtpProfiles.c_str());
            LOG_OPENSSL_ERROR("SSL_CTX_set_tlsext_use_srtp() failed");

            // goto error;
        }

        return;
    }

    /* Instance methods. */

    DtlsTransport::DtlsTransport(Listener *listener) : listener(listener) {

        /* Set SSL. */

        this->ssl = SSL_new(DtlsTransport::sslCtx);

        if (!this->ssl) {
            LOG_OPENSSL_ERROR("SSL_new() failed");

            goto error;
        }

        // Set this as custom data.
        SSL_set_ex_data(this->ssl, 0, static_cast<void *> (this));
        // SSL_set_ex_data(this->ssl, 0, this);

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

        // Set the MTU so that we don't send packets that are too large with no
        // fragmentation.
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

    DtlsTransport::~DtlsTransport() {

        if (IsRunning()) {
            // Send close alert to the peer.
            SSL_shutdown(this->ssl);
            SendPendingOutgoingDtlsData();
        }

        if (this->ssl) {
            SSL_free(this->ssl);

            this->ssl = nullptr;
            this->sslBioFromNetwork = nullptr;
            this->sslBioToNetwork = nullptr;
        }

        // Close the DTLS timer.
        delete this->timer;
    }

    void DtlsTransport::Dump() const {

        std::string state{"new"};
        std::string role{"none "};

        switch (this->state) {
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

        switch (this->localRole) {
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

    void DtlsTransport::Run(Role localRole) {

        assertm(localRole == Role::CLIENT || localRole == Role::SERVER,
                "local DTLS role must be 'client' or 'server'");

        Role previousLocalRole = this->localRole;

        if (localRole == previousLocalRole) {
            LError("same local DTLS role provided, doing nothing");

            return;
        }

        // If the previous local DTLS role was 'client' or 'server' do reset.
        if (previousLocalRole == Role::CLIENT || previousLocalRole == Role::SERVER) {
            LTrace("resetting DTLS due to local role change");

            Reset();
        }

        // Update local role.
        this->localRole = localRole;

        // Set state and notify the listener.
        this->state = DtlsState::CONNECTING;
        this->listener->OnDtlsTransportConnecting(this);

        switch (this->localRole) {
            case Role::CLIENT:
            {
                SInfo << "running [role:client]";

                SSL_set_connect_state(this->ssl);
                SSL_do_handshake(this->ssl);
                SendPendingOutgoingDtlsData();
                SetTimeout();

                break;
            }

            case Role::SERVER:
            {
                SInfo << "running [role:server]";

                SSL_set_accept_state(this->ssl);
                SSL_do_handshake(this->ssl);

                break;
            }

            default:
            {
                SError << "invalid local DTLS role";
                exit(0);
            }
        }
    }

    void DtlsTransport::ProcessDtlsData(const uint8_t *data, size_t len) {

        SInfo << "ProcessDtlsData " << len;
            
        int written;
        int read;

        if (!IsRunning()) {
            LError("cannot process data while not running");

            return;
        }

        // Write the received DTLS data into the sslBioFromNetwork.
        written = BIO_write(this->sslBioFromNetwork, static_cast<const void *> (data),
                static_cast<int> (len));

        if (written != static_cast<int> (len)) {
            LWarn("OpenSSL BIO_write() wrote less ", static_cast<size_t> (written),
                    " than given data ", len);
        }

        // Must call SSL_read() to process received DTLS data.
        read = SSL_read(this->ssl, static_cast<void *> (DtlsTransport::sslReadBuffer),
                SslReadBufferSize);

        // Send data if it's ready.
        SendPendingOutgoingDtlsData();

        // Check SSL status and return if it is bad/closed.
        if (!CheckStatus(read))
            return;

        // Set/update the DTLS timeout.
        if (!SetTimeout())
            return;

        // Application data received. Notify to the listener.
        if (read > 0) {
            // It is allowed to receive DTLS data even before validating remote
            // fingerprint.
            if (!this->handshakeDone) {
                LWarn("ignoring application data received while DTLS handshake not done");

                return;
            }

            // Notify the listener.
            this->listener->OnDtlsTransportApplicationDataReceived(
                    this, (uint8_t *) DtlsTransport::sslReadBuffer,
                    static_cast<size_t> (read));
        }
    }

    void DtlsTransport::SendApplicationData(const uint8_t *data, size_t len) {

        // We cannot send data to the peer if its remote fingerprint is not validated.
        if (this->state != DtlsState::CONNECTED) {
            LWarn("cannot send application data while DTLS is not fully connected");

            return;
        }

        if (len == 0) {
            LWarn("ignoring 0 length data");

            return;
        }

        int written;

        written = SSL_write(this->ssl, static_cast<const void *> (data),
                static_cast<int> (len));

        if (written < 0) {
            LOG_OPENSSL_ERROR("SSL_write() failed");

            if (!CheckStatus(written))
                return;
        } else if (written != static_cast<int> (len)) {
            LWarn("OpenSSL SSL_write() wrote less ", written, " than given data bytes ",
                    len);
        }

        // Send data.
        SendPendingOutgoingDtlsData();
    }

    void DtlsTransport::Reset() {

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

        this->localRole = Role::NONE;
        this->state = DtlsState::NEW;
        this->handshakeDone = false;
        this->handshakeDoneNow = false;

        // Reset SSL status.
        // NOTE: For this to properly work, SSL_shutdown() must be called before.
        // NOTE: This may fail if not enough DTLS handshake data has been received,
        // but we don't care so just clear the error queue.
        ret = SSL_clear(this->ssl);

        if (ret == 0)
            ERR_clear_error();
    }

    inline bool DtlsTransport::CheckStatus(int returnCode) {

        SInfo << "DtlsTransport::CheckStatus()";

        int err;
        bool wasHandshakeDone = this->handshakeDone;

        err = SSL_get_error(this->ssl, returnCode);

        switch (err) {
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
                LTrace("SSL status: SSL_ERROR_WANT_X509_LOOKUP");
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
        if (this->handshakeDoneNow) {
            this->handshakeDoneNow = false;
            this->handshakeDone = true;
            
            SInfo << "handshake done";

            // Stop the timer.
            this->timer->Stop();

            // Process the handshake just once (ignore if DTLS renegotiation).
            if (!wasHandshakeDone) // if (!wasHandshakeDone &&
                // this->remoteFingerprint.algorithm !=
                // FingerprintAlgorithm::NONE)
                return ProcessHandshake();

            return true;
        }// Check if the peer sent close alert or a fatal error happened.
        else if (((SSL_get_shutdown(this->ssl) & SSL_RECEIVED_SHUTDOWN) != 0) ||
                err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL) {
            if (this->state == DtlsState::CONNECTED) {
                LTrace("disconnected");

                Reset();

                // Set state and notify the listener.
                this->state = DtlsState::CLOSED;
                this->listener->OnDtlsTransportClosed(this);
            } else {
                LWarn("connection failed");

                Reset();

                // Set state and notify the listener.
                this->state = DtlsState::FAILED;
                this->listener->OnDtlsTransportFailed(this);
            }

            return false;
        } else {
            return true;
        }
    }

    inline void DtlsTransport::SendPendingOutgoingDtlsData() {

        if (BIO_eof(this->sslBioToNetwork))
            return;

        int64_t read;
        char *data{nullptr};

        read = BIO_get_mem_data(this->sslBioToNetwork, &data); // NOLINT

        if (read <= 0)
            return;

        // SDebug << read << " bytes of DTLS data ready to sent to the peer" ;

        // Notify the listener.
        this->listener->OnDtlsTransportSendData(
                this, reinterpret_cast<uint8_t *> (data), static_cast<size_t> (read));

        // Clear the BIO buffer.
        // NOTE: the (void) avoids the -Wunused-value warning.
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

            this->timer->Start(timeoutMs);

            return true;
        }// NOTE: Don't start the timer again if the timeout is greater than 30
            // seconds.
        else {
            SWarn << "DTLS timeout too high ms, resetting DLTS" << timeoutMs;

            Reset();

            // Set state and notify the listener.
            this->state = DtlsState::FAILED;
            this->listener->OnDtlsTransportFailed(this);

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
                case 'W':
                    alertType = "warning";
                    break;

                case 'F':
                    alertType = "fatal";
                    break;

                default:
                    alertType = "undefined";
            }

            if ((where & SSL_CB_READ) != 0) {
                LWarn("received DTLS ", alertType,
                        " alert: ", SSL_alert_desc_string_long(ret));
            } else if ((where & SSL_CB_WRITE) != 0) {
                LTrace("sending DTLS ", alertType,
                        " alert: ", SSL_alert_desc_string_long(ret));
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

        X509 *certificate;
        uint8_t binaryFingerprint[EVP_MAX_MD_SIZE];
        unsigned int size{0};
        char hexFingerprint[(EVP_MAX_MD_SIZE * 3) + 1];
        const EVP_MD *hashFunction;
        int ret;

        certificate = SSL_get_peer_certificate(this->ssl);

        if (!certificate) {
            SWarn << "no certificate was provided by the peer";

            return false;
        }

        switch (this->remoteFingerprint.algorithm) {
            case CertificateFingerprint::Algorithm::Sha1:
                hashFunction = EVP_sha1();
                break;

            case CertificateFingerprint::Algorithm::Sha224:
                hashFunction = EVP_sha224();
                break;

            case CertificateFingerprint::Algorithm::Sha256:
                hashFunction = EVP_sha256();
                break;

            case CertificateFingerprint::Algorithm::Sha384:
                hashFunction = EVP_sha384();
                break;

            case CertificateFingerprint::Algorithm::Sha512:
                hashFunction = EVP_sha512();
                break;

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

        SDebug << "valid remote fingerprint";

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

        BIO_get_mem_ptr(bio, &mem); // NOLINT[cppcoreguidelines-pro-type-cstyle-cast]

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
            this->listener->OnDtlsTransportFailed(this);

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

/*********************************************************************************************************************************************************************/
//common file
/*********************************************************************************************************************************************************************/

namespace rtc {

    void DtlsTransport::ClassInit() {
        STrace << "DtlsTransport::ClassInit()";

        // Generate a X509 certificate and private key (unless PEM files are
        // provided).
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

        //                if (TransportExIndex < 0) {
        //                    TransportExIndex = SSL_get_ex_new_index(0, NULL, NULL,
        //                    NULL, NULL);
        //                }

        // Generate certificate fingerprints.
        // GenerateFingerprints();
    }

    bool DtlsTransport::checkFingerprint(const std::string &fingerprint) {

        // mRemoteFingerprint = fingerprint;

        if (!remoteFingerprint.value.size())

            return false;

        //	if (config.disableFingerprintVerification) {
        //		STrace << "Skipping fingerprint validation";
        //		return true;
        //	}

        auto expectedFingerprint = remoteFingerprint.value;
        if (expectedFingerprint == fingerprint) {
            STrace << "Valid fingerprint \"" << fingerprint << "\"";
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
