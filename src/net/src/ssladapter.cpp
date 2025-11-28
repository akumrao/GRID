/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "net/ssladapter.h"
#include "base/logger.h"
//#include "net/sslmanager.h"
#include "net/netInterface.h"
#include "net/SslConnection.h"
#include <assert.h>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>



#if SETTINGFromCONFIG
#include "Settings.h"
#endif 

#define FROMFILE 1

using namespace std;



#if USE_MBEDTLS


namespace base
{
namespace net
{


#if DEBUG_LEVEL > 0
    /**
     * Debug callback for mbed TLS
     * Just prints on the USB serial port
     */
    void SSLAdapter::my_debug(void *ctx, int level, const char *file, int line,   const char *str)
    {
        const char *p, *basename;
        (void) ctx;

        /* Extract basename from file */
        for(p = basename = file; *p != '\0'; p++) {
            if(*p == '/' || *p == '\\') {
                basename = p + 1;
            }
        }

        printf("%s:%04d: |%d| %s", basename, line, level, str);
    }
    /**
     * Certificate verification callback for mbed TLS
     * Here we only use it to display information on each cert in the chain
     */
     int  SSLAdapter::my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
    {
        char buf[1024];
        (void) data;

        printf("\nVerifying certificate at depth %d:\n", depth);
        mbedtls_x509_crt_info(buf, sizeof (buf) - 1, "  ", crt);
        printf("%s", buf);

        if (*flags == 0)
            printf("No verification issue for this certificate\n");
        else
        {
            mbedtls_x509_crt_verify_info(buf, sizeof (buf), "  ! ", *flags);
            printf("%s\n", buf);
        }

        return 0;
    }
#endif
//    void  SSLAdapter::onError(Socket *s, socket_error_t err) {
//        (void) s;
//        printf("MBED: Socket Error: %s (%d)\r\n", socket_strerror(err), err);
//        if (_stream.getTLSError()) {
//            char buf[128];
//            int ret = _stream.getTLSError(buf, sizeof buf);
//            printf("MBED: TLS Error: %04x: %s\r\n", -ret, buf);
//        }
//
//        _stream.close();
//        _error = true;
//        printf("{{%s}}\r\n",(error()?"failure":"success"));
//        printf("{{end}}\r\n");
//    }
//    




SSLAdapter::SSLAdapter(SslConnection *socket)
    : _socket(socket)
{
    
    ssl_bio_ = 0;
    app_bio_ = 0;
//    oprn_state = STATE_INIT;
    
}

SSLAdapter::~SSLAdapter()
{
    
    if (app_bio_) {
        TLS_BIO_free_all(app_bio_);
    }
    if (ssl_bio_) {
        TLS_BIO_free_all(ssl_bio_);
    }
    
    LTrace("Destroy")

}

void SSLAdapter::initSSL()
{
    mbedtls_ssl_init(&_ssl);
    
    mbedtls_entropy_init(&_entropy);
    mbedtls_ctr_drbg_init(&_ctr_drbg);
    mbedtls_x509_crt_init(&_cacert);
    mbedtls_ssl_config_init(&_ssl_conf);
    
    
    if(server)
    {
       mbedtls_pk_init( &pkey );

    }
        
    const char *DRBG_PERS = "mbed TLS helloword client";
    
    
  
    if (mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                       nullptr, 0) != 0)

    {

        SError << " Failed mbedtls_ctr_drbg_seed ";
    }



//    if (TLS_ANY_VERSION != ssl->version) {
//        if (TLS1_2_VERSION == ssl->version)
//            version = MBEDTLS_SSL_MINOR_VERSION_3;
//        else if (TLS1_1_VERSION == ssl->version)
//            version = MBEDTLS_SSL_MINOR_VERSION_2;
//        else
//            version = MBEDTLS_SSL_MINOR_VERSION_1;
//
//        mbedtls_ssl_conf_max_version(&ssl_pm->conf, MBEDTLS_SSL_MAJOR_VERSION_3, version);
//        mbedtls_ssl_conf_min_version(&ssl_pm->conf, MBEDTLS_SSL_MAJOR_VERSION_3, version);
//    } else {
    
    
    if(server)
    { 
       
        if( ( mbedtls_ssl_config_defaults( &_ssl_conf,
                    MBEDTLS_SSL_IS_SERVER,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
        {
             SError << "mbedtls_ssl_config_defaults failed";
             exit(0);
        }
        
 
    }
    else
    {
         
        if( mbedtls_ssl_config_defaults(&_ssl_conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT) != 0) 
        {
          SError << "mbedtls_ssl_config_defaults failed";
          exit(0);
        }
               
        
    }
    
    
        
          
    
        //mbedtls_ssl_conf_max_version(&_ssl_conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
        //mbedtls_ssl_conf_min_version(&_ssl_conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_1);
   // }
     
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if FROMFILE

    std::string CertFile = "/mnt/key/certificate.crt";
  

    
    int ret = 0;

    if( ( ret = mbedtls_x509_crt_parse_file( &_cacert, CertFile.c_str() ) ) != 0 )
    {
        //mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
        //goto exit;
        
        SError << "mbedtls_x509_crt_parse returned " << ret ;
        
        
          exit(0);
    }


    
    if(server)
    {
       const char * KeyFile = "/mnt/key/private_key.pem";
       const char *pwd = "12345678";

        //ret =  mbedtls_pk_parse_keyfile( &pkey,  KeyFile ,pwd );// mbedtls
        ret = mbedtls_pk_parse_keyfile(&pkey, KeyFile, pwd, mbedtls_ctr_drbg_random, &_ctr_drbg); 
        if( ret != 0 )
        {
             SError << "mbedtls_x509_crt_parse returned " << ret ;
             exit(0);
        }


    }   
   
            
            
   
       
       
            
    
#else

    
    
    int ret = mbedtls_x509_crt_parse( &_cacert, (const unsigned char *) mbedtls_test_srv_crt,
                          mbedtls_test_srv_crt_len );
    if( ret != 0 )
    {
        SError << "mbedtls_x509_crt_parse returned " << ret ;
        exit(0);
    }

    ret = mbedtls_x509_crt_parse( &_cacert, (const unsigned char *) mbedtls_test_cas_pem,
                          mbedtls_test_cas_pem_len );
    if( ret != 0 )
    {
        SError << "mbedtls_x509_crt_parse returned " << ret ;
        exit(0);
    }

    ret =  mbedtls_pk_parse_key( &pkey, (const unsigned char *) mbedtls_test_srv_key,
                         mbedtls_test_srv_key_len, NULL, 0,  mbedtls_ctr_drbg_random, &_ctr_drbg );
    if( ret != 0 )
    {
         SError << "mbedtls_x509_crt_parse returned " << ret ;
         exit(0);
    }
    
    
    
    
    


#endif

     

 
        mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, NULL);
        
        if(server)
        if( (  mbedtls_ssl_conf_own_cert( &_ssl_conf, &_cacert, &pkey ) ) != 0 )
        {
            SError << "failed\n  ! mbedtls_ssl_conf_own_cert returned ";
            exit(0);
        }
         
        #if UNSAFE
        mbedtls_ssl_conf_authmode(&_ssl_conf, MBEDTLS_SSL_VERIFY_OPTIONAL ); //MBEDTLS_SSL_VERIFY_OPTIONAL); MBEDTLS_SSL_VERIFY_NONE
        #endif
         mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg); 
      

       // mbedtls_ssl_conf_own_cert(&_ssl_conf, &_cacert, NULL);
      
        //static const auto host = "127.0.0.1";
        setup(&_ssl_conf, nullptr);

        
        if(server)
            mbedtls_ssl_session_reset( &_ssl );

        
   #if DEBUG_LEVEL > 0
        mbedtls_ssl_conf_verify(&_ssl_conf, my_verify, NULL);
        mbedtls_ssl_conf_dbg(&_ssl_conf, my_debug, NULL);
        mbedtls_debug_set_threshold(DEBUG_LEVEL);
 #endif     


        
        
    //

//    if (server)
//    {
//
//    }
    
}






bool SSLAdapter::setup(const mbedtls_ssl_config *conf, const char *hostname)
{
    int ret;

    ret = mbedtls_ssl_setup(&_ssl, conf);
    if (ret != 0) {
        _ssl_error = ret;
        return false;
    }

    if (hostname != NULL) {
        ret = mbedtls_ssl_set_hostname(&_ssl, hostname);
        if (ret != 0) {
            _ssl_error = ret;
            return false;
        }
    }
    
    
    ssl_bio_ = SSL_BIO_new(BIO_BIO);
    app_bio_ = SSL_BIO_new(BIO_BIO);
    TLS_BIO_make_bio_pair(ssl_bio_, app_bio_);

    

    mbedtls_ssl_set_bio(&_ssl, ssl_bio_, TLS_BIO_net_send, TLS_BIO_net_recv, NULL);

    return true;
}


//void SSLAdapter::initServer()  //(SSL* ssl)
//{
//    server =true;
//
//}

void SSLAdapter::shutdown()
{
    SInfo << "Shutdown";
            
   int ret = mbedtls_ssl_close_notify(&_ssl);

    if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        
         SError << "Shutdown blocked";
        return;
        
    }
    /* Ignore other errors, the connection may be closed or unusable */

   
    mbedtls_ctr_drbg_free(&_ctr_drbg);
    mbedtls_entropy_free(&_entropy);
    mbedtls_ssl_config_free(&_ssl_conf);
    mbedtls_x509_crt_free(& _cacert );
    
    if(server)
    {
        mbedtls_pk_free( &pkey );
    }
        
    
    mbedtls_ssl_free(&_ssl);



    
    
}

//bool SSLAdapter::initialized() const
//{
//    //return !!_ssl;
//    
//    return true;
//}

//bool SSLAdapter::ready() const
//{
//    // return _ssl && SSL_is_init_finished(_ssl);
//    return true;
//}
//
//int SSLAdapter::available() const
//{
//    //assert(_ssl);
//    //return SSL_pending(_ssl);
//    return 1;
//}


int SSLAdapter::handshake()
{
    

  if (handshake_state == STATE_HANDSHAKE_DONE) {
    return STATE_HANDSHAKE_DONE;
  }
  handshake_state = STATE_HANDSHAKING;

  int rv = 0;
  rv = mbedtls_ssl_handshake(&_ssl); 
  rv = swrap_error_handler( rv);
  if (rv == 0) {
    handshake_state = STATE_HANDSHAKE_DONE;

    int verify_status = (int)mbedtls_ssl_get_verify_result(&_ssl);
    if (verify_status) {
      char buf[512];
      mbedtls_x509_crt_verify_info(buf, sizeof(buf), "::", (uint32_t)verify_status);
      //mbedtls_printf("%s\n", buf);
       SError << " Failed ssl cert verification " << buf;
    }
    
    SInfo << "SSL Handshake over";
    
    if (_bufferOut.size() > 0)
    {
    
        addOutgoingData( &_bufferOut[0],  _bufferOut.size());
        _bufferOut.clear();
    }
    
    
  //  _socket->on_tls_connect();

    // notify to the JS layer "onhandshakedone".
   // jerry_value_t fn = iotjs_jval_get_property(jthis, "onhandshakedone");

  }
  return handshake_state;
    
    

    
}



//bool SSLAdapter::isConnected() const {
//
//   _ssl.state == MBEDTLS_SSL_HANDSHAKE_OVER;
//}


std::string SSLAdapter::getTLSError(int err)  {
    
    char buf[128];
    
    mbedtls_strerror(err, buf, sizeof buf);
  

    return buf;
}


/* Arvind TBD. Below code does not work with lower version of OpenSSL.
  In future I will replace TLS with DTLS
*/


void  SSLAdapter::addOutgoingData(const char* data, size_t len)
{

        // SInfo << "Send " <<  data   << " len "  << len;
    //if (len > MBEDTLS_SSL_MAX_CONTENT_LEN) // mbedtld2.5    
    if (len > MBEDTLS_SSL_OUT_CONTENT_LEN) 
    {
        SError <<  "encode data is too large, change the values in config.h to increase the size" ;
      //  return;
    }

    

    if( handshake_state == STATE_HANDSHAKING )
    {  // handshake();
        std::copy(data, data + len, std::back_inserter(_bufferOut));
        return;
    }    
    
    size_t rv;
    

    rv = (size_t)mbedtls_ssl_write( &_ssl, (const unsigned char *) data,  len);
    // if (r <= 0) { handleError(r); }  // we are not sure why Mbedtls does not handle error, though openssl does it 
    // I have refered https://github.com/yodaos-project/ShadowNode.git
    
    
    size_t pending = 0;

    char *encoded_data = nullptr;


    if( (pending = TLS_BIO_ctrl_pending(app_bio_) ) > 0 ) {

        encoded_data = (char*)malloc(pending);
       // encoded_data.len = pending;

        rv = TLS_BIO_read(app_bio_, encoded_data , pending);
       // data2encode->len = rv;
        //assert(rv == len);
        _socket->Write(  encoded_data, rv , cb);
        cb = nullptr;
        free(encoded_data);
    }
    else
    {
        SError <<  "SSL Error Encoding" ;
    }

}
   

void SSLAdapter::addIncomingData(const char *data, size_t len)
{
   
     TLS_BIO_write( app_bio_,data , len);
     
     
       
    if( handshake_state == STATE_HANDSHAKING )
    {   handshake();
        return;
    }    
 


    while (true)
    {
    
        memset((void*) data, 0, len);
    
        int rv = -1;
        rv = mbedtls_ssl_read(&_ssl, (unsigned char *)data, len);
        rv =swrap_error_handler( rv);

        if (rv > 0) {

            _socket->on_read(data,  rv) ; 

        } else if (rv == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
         // jerry_value_t fn = iotjs_jval_get_property(jthis, "onclose");
            SInfo << "MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY";
          break;
        } else if (rv == MBEDTLS_ERR_SSL_WANT_READ ||
                   rv == MBEDTLS_ERR_SSL_WANT_WRITE) {
          break;
        } else {
            //SError << getTLSError(rv);
          break;
        }
   }
   
}

//handle only non fatal error currently
int SSLAdapter::swrap_error_handler( const int code)
{
    
    
    if (code == MBEDTLS_ERR_SSL_WANT_WRITE || code == MBEDTLS_ERR_SSL_WANT_READ) {
        stay_uptodate();
    } 
    else if (code == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
        return code;
    }
    
    
    return code;
    
}



void SSLAdapter::stay_uptodate( )
{
 

    size_t pending = TLS_BIO_ctrl_pending(app_bio_);
    if( pending > 0) {

        //Need to free the memory
        char *mybuf;
        
        mybuf = (char*)malloc(pending);

        int rv = TLS_BIO_read(app_bio_, mybuf, pending);
        assert( rv == pending );

        _socket->Write( mybuf, rv, cb);
        
        //SInfo << "stay_uptodate "  <<  rv ;
        
        
        //assert(rv == pending);

        free(mybuf);
        mybuf = 0;
    }
    
    
    

  }
    
    
    
    
    
    
    
    
    
    
    




}  // namespace net
}  // namespace base


#else



namespace base
{
namespace net
{

const char defaultCertificate[]
    = "-----BEGIN CERTIFICATE-----\n"
      "MIICwjCCAaqgAwIBAgIJAOKZBPq4tcY7MA0GCSqGSIb3DQEBCwUAMBkxFzAVBgNV\n"
      "BAMTDkFydmluZFNjb3BlQXBwMB4XDTIwMDYxMzA0MjE0N1oXDTMwMDYxMTA0MjE0\n"
      "N1owGTEXMBUGA1UEAxMOQXJ2aW5kU2NvcGVBcHAwggEiMA0GCSqGSIb3DQEBAQUA\n"
      "A4IBDwAwggEKAoIBAQDAPJVsM+7tQxKy2IBp+8i2aCuv3xl1wftDxXqG7GYuatDi\n"
      "d8rwHBH68JcnTU09T8RHi+Ezj+0YPYV4IDGTUDufxK1snv5V6wdKESZM2ZYvzDID\n"
      "uHCiXrtl5Tee+tnh1XYk4CXk9h+SsB/X70FXIW98XqR+2iVl1ezwjEeu7X1ET9wh\n"
      "1UHOiLB0do5+dSDo/nNIP+K+QnG/YC9vYUViozWO1JvZ0KgEybOrTbRKWsHKRyRN\n"
      "OyZtXNUMcLt2vJLCm5dmDfPCeqbEagyNZLPpucd+HRoZ1U1aXvZ36l30sJlvIjnk\n"
      "eLkccd3Bv85fhAvzK5WoAqSsB0nFOAYmDIVcyDcfAgMBAAGjDTALMAkGA1UdEwQC\n"
      "MAAwDQYJKoZIhvcNAQELBQADggEBAC5KyEK9/Z4VM2CSNbFm6IzND0AACqYT2e8d\n"
      "HsT5/cLo+Zc7NWvMagq+myAAYEptarbvHNVWS/gsYWSg5+pHhrs1VPCXZTLjelGG\n"
      "nSqEZSXl4ANV9yNP/KdG8z8zruHKsqwJ0LDLem2KOnA0WzcEO1IRH59EnVsV4CkT\n"
      "Cs1DH2i20NCZklwFREd3AOgkPR7pruxITN6hQ6MH/MHC6FyQbbvJEl7ceV1adON/\n"
      "XJNYomKwCVkxLss8PV/TcyPA9CWJA/c9blh/GPRAerqbBF7OwPVKmt3RxBr02tGT\n"
      "TFTokCgkm2d9DYtf0rtQOOL82zZB/YmgQytMYxaiUCf31xJTR/I=\n"
      "-----END CERTIFICATE-----";


static SSL_CTX *ctxClient = nullptr;

static SSL_CTX *ctxServer = nullptr;

SSL_CTX *InitCTX(bool server)
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;


    std::string KeyFile = "/var/tmp/key/private_key.pem";
    #if SETTINGFromCONFIG
    KeyFile = Settings::configuration.dtlsPrivateKeyFile;
    #endif

    SSL_library_init();


    OpenSSL_add_all_algorithms(); /* load & register all cryptos, etc. */
    SSL_load_error_strings(); /* load all error messages */

    if (server)
        method = TLSv1_2_server_method();
    else
        method = TLSv1_2_client_method();
    ctx = SSL_CTX_new(method); /* create new context from method */
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    SSL_CTX_set_cipher_list(ctx, "ALL:eNULL");

    // Arvind TBD
    // SSL_CTX_set_session_cache_mode (ctx, SSL_SESS_CACHE_BOTH);
    // SSL_CTX_set_timeout (ctx, 300);  client side check code before enable it


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if FROMFILE

    std::string CertFile = "/var/tmp/key/certificate.crt";
    #if SETTINGFromCONFIG
    CertFile = Settings::configuration.dtlsCertificateFile;
    #endif


    if (SSL_CTX_load_verify_locations(ctx, CertFile.c_str(), nullptr) != 1) ERR_print_errors_fp(stderr);

    if (SSL_CTX_set_default_verify_paths(ctx) != 1) ERR_print_errors_fp(stderr);


    if (SSL_CTX_use_certificate_file(ctx, CertFile.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
#else

    X509 *cert = NULL;
    BIO *bio = NULL;

    bio = BIO_new_mem_buf((char *) defaultCertificate, -1);

    if (bio == NULL)
    {
        fprintf(stderr, "BIO_new_mem_buf failed\n");
        abort();
    }

    /* use it to read the PEM formatted certificate from memory into an X509
     * structure that SSL can use
     */
    cert = PEM_read_bio_X509(bio, NULL, 0, NULL);
    if (cert == NULL)
    {
        fprintf(stderr, "PEM_read_bio_X509 failed...\n");
        abort();
    }


    if (SSL_CTX_use_certificate(ctx, cert) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (bio) BIO_free(bio);

    if (cert) X509_free(cert);

#endif


    //

    if (server)
    {
        // New lines //for server side only

#if FROMFILE
        SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *) "12345678");
#endif

        if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            abort();
        }

        if (!SSL_CTX_check_private_key(ctx))
        {
            fprintf(stderr, "Private key does not match the public certificate\n");
            abort();
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // if (server) {
    //     //New lines //for server side only

    //     if (SSL_CTX_load_verify_locations(ctx, CertFile, KeyFile) != 1)
    //         ERR_print_errors_fp(stderr);

    //     if (SSL_CTX_set_default_verify_paths(ctx) != 1)
    //         ERR_print_errors_fp(stderr);
    // }
    // //End new lines

    // /* set the local certificate from CertFile */
    // if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
    //     ERR_print_errors_fp(stderr);
    //     abort();
    // }
    // /* set the private key from KeyFile (may be the same as CertFile) */
    // SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *) "12345678");
    // if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
    //     ERR_print_errors_fp(stderr);
    //     abort();
    // }
    // /* verify private key */
    // if (!SSL_CTX_check_private_key(ctx)) {
    //     fprintf(stderr, "Private key does not match the public certificate\n");
    //     abort();
    // }

    // New lines - Force the client-side have a certificate
    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    // SSL_CTX_set_verify_depth(ctx, 4);
    // End new lines

    return ctx;
}

void ShowCerts(SSL *ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if (cert != NULL)
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}

SSLAdapter::SSLAdapter(SslConnection *socket)
    : _socket(socket),
      _ssl(nullptr),
      _readBIO(nullptr),
      _writeBIO(nullptr){LTrace("SSLAdapter")}

      SSLAdapter::~SSLAdapter()
{
    // LTrace("Destroy")
    if (_ssl)
    {
        SSL_free(_ssl);
        _ssl = nullptr;
    }
    // LTrace("Destroy: OK")
}
      
      
void  SSLAdapter::initSSL()
{
    if(server)
        initServer();
    else        
        initClient();
    
}
   
void SSLAdapter::initClient()
{
    LTrace("Init client")
        /*assert(_socket);
        if (!_socket->context())
            _socket->useContext(SSLManager::instance().defaultClientContext());
        assert(!_socket->context()->isForServerUse());
         */

    if (!ctxClient)
    {
        ctxClient = InitCTX(false); /* initialize SSL */
    }


    _ssl = SSL_new(ctxClient);

    // TODO: Improve automatic SSL session handling.
    // Maybe add a stored session to the network manager.
    //            if (_socket->currentSession())
    //               SSL_set_session(_ssl, _socket->currentSession()->sslSession());

    _readBIO = BIO_new(BIO_s_mem());
    _writeBIO = BIO_new(BIO_s_mem());
    SSL_set_bio(_ssl, _readBIO, _writeBIO);
    SSL_set_connect_state(_ssl);
   // SSL_do_handshake(_ssl);
}

void SSLAdapter::initServer()  //(SSL* ssl)
{
    LTrace("Init server")
        /*assert(_socket);
        if (!_socket->context())
            _socket->useContext(SSLManager::instance().defaultServerContext());
        assert(_socket->context()->isForServerUse());*/

    if (!ctxServer)
    {
        ctxServer = InitCTX(true); /* initialize SSL */
    }


    _ssl = SSL_new(ctxServer);
    _readBIO = BIO_new(BIO_s_mem());
    _writeBIO = BIO_new(BIO_s_mem());
    SSL_set_bio(_ssl, _readBIO, _writeBIO);
    SSL_set_accept_state(_ssl);
    SSL_do_handshake(_ssl);
}

void SSLAdapter::shutdown()
{
    LTrace("Shutdown") if (_ssl)
    {
        // LTrace("Shutdown SSL")

        // Don't shut down the socket more than once.
        int shutdownState = SSL_get_shutdown(_ssl);
        bool shutdownSent = (shutdownState & SSL_SENT_SHUTDOWN) == SSL_SENT_SHUTDOWN;
        if (!shutdownSent)
        {
            // A proper clean shutdown would require us to
            // retry the shutdown if we get a zero return
            // value, until SSL_shutdown() returns 1.
            // However, this will lead to problems with
            // most web browsers, so we just set the shutdown
            // flag by calling SSL_shutdown() once and be
            // done with it.
            int rc = SSL_shutdown(_ssl);
            if (rc <= 0) handleError(rc);
        }
    }
}

bool SSLAdapter::initialized() const
{
    return !!_ssl;
}

bool SSLAdapter::ready() const
{
    return _ssl && SSL_is_init_finished(_ssl);
}

int SSLAdapter::available() const
{
    assert(_ssl);
    return SSL_pending(_ssl);
}

void SSLAdapter::addIncomingData(const char *data, size_t len)
{
    // LTrace("Add incoming data: ", len)
    assert(_readBIO);
    BIO_write(_readBIO, data, (int) len);
    flush();
    flush();
}

void SSLAdapter::addOutgoingData(const std::string &s)
{
    addOutgoingData(s.c_str(), s.size());
}

void SSLAdapter::addOutgoingData(const char *data, size_t len)
{
    std::copy(data, data + len, std::back_inserter(_bufferOut));
    flush();
}

void SSLAdapter::handshake()
{
    int r = SSL_do_handshake(_ssl);
    if (r <= 0) handleError(r);
}

void SSLAdapter::flush()
{
    // LTrace("Flushing")

    // Keep trying to handshake until initialized
    if (!ready()) return handshake();

    // Read any decrypted remote data from SSL and emit to the app
    flushReadBIO();

    // Write any local data to SSL for excryption
    if (_bufferOut.size() > 0)
    {
        int r = SSL_write(_ssl, &_bufferOut[0], (int) _bufferOut.size());
        if (r <= 0) { handleError(r); }
        _bufferOut.clear();
        // flushWriteBIO();
    }

    // send any encrypted data from SSL to the remote peer
    flushWriteBIO();
}

/* Arvind TBD. Below code does not work with lower version of OpenSSL.
  In future I will replace TLS with DTLS
*/
void SSLAdapter::flushReadBIO()
{
    size_t npending = BIO_ctrl_pending(_readBIO);
    if (npending > 0)
    {
        int nread;
        char buffer[npending];
        while ((nread = SSL_read(_ssl, buffer, npending)) > 0)
        {
            // LTrace("On Read ", buffer)
            //  _socket->listener->on_read(_socket, buffer, nread); // arvind
            _socket->on_read(buffer, nread);  // arvind
        }
    }
}

/*
void SSLAdapter::flushReadBIO() {
    size_t npending = BIO_ctrl_pending(_readBIO);
    if (npending > 0) {
        int nread = 0;
        int ntotal = 0;

        char buffer[npending];
        while ((nread = SSL_read(_ssl, &buffer[ntotal], npending)) > 0) {
           // LTrace("On Read ", buffer)
            //  _socket->listener->on_read(_socket, buffer, nread); // arvind
           // _socket->on_read(buffer, nread); // arvind
            ntotal = ntotal + nread;
        }
        _socket->on_read(buffer, ntotal); // arvind
    }
}*/

void SSLAdapter::flushWriteBIO()
{
    size_t npending = BIO_ctrl_pending(_writeBIO);
    if (npending > 0)
    {
        char buffer[npending];
        int nread = BIO_read(_writeBIO, buffer, npending);
        if (nread > 0)
        {
            _socket->Write(buffer, nread, cb);  // arvind
            cb = nullptr;
        }
    }
}

void SSLAdapter::handleError(int rc)
{
    if (rc > 0) return;
    int error = SSL_get_error(_ssl, rc);
    switch (error)
    {
    case SSL_ERROR_ZERO_RETURN:
        LTrace("SSL_ERROR_ZERO_RETURN");
        return;
    case SSL_ERROR_WANT_READ:
        // LTrace("SSL_ERROR_WANT_READ")
        flushWriteBIO();
        break;
    case SSL_ERROR_WANT_WRITE:
        LTrace("SSL_ERROR_WANT_WRITE");
        assert(0 && "not implemented");
        break;
    case SSL_ERROR_WANT_CONNECT:
    case SSL_ERROR_WANT_ACCEPT:
    case SSL_ERROR_WANT_X509_LOOKUP:
        assert(0 && "should not occur");
        break;
    default:
        char buffer[256];
        ERR_error_string_n(ERR_get_error(), buffer, sizeof(buffer));
        std::string msg(buffer);
        SError << msg;
        // throw std::runtime_error("SSL connection failed: " + msg);
        //_socket->setError("SSL connection failed: " + msg);  //arvind
        break;
    }
}


}  // namespace net
}  // namespace base

#endif

