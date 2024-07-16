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




//#include "Settings.h" // not possible, always you have to have to hardcode the path

#define FROMFILE 1

#define UNSAFE 1

#define DEBUG_LEVEL 1




using namespace std;


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
}

SSLAdapter::~SSLAdapter()
{
    LTrace("Destroy")

}

void SSLAdapter::initClient()
{
    mbedtls_ssl_init(&_ssl);
    
    mbedtls_entropy_init(&_entropy);
    mbedtls_ctr_drbg_init(&_ctr_drbg);
    mbedtls_x509_crt_init(&_cacert);
    mbedtls_ssl_config_init(&_ssl_conf);
        
    const char *DRBG_PERS = "mbed TLS helloword client";
    
    
  
    if (mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                       (const unsigned char *) DRBG_PERS,
                       sizeof (DRBG_PERS)) != 0)

    {

        SError << " Failed mbedtls_ctr_drbg_seed ";
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if FROMFILE

    std::string CertFile = "/mnt/key/certificate.crt";
  
    mbedtls_x509_crt cacert;

    mbedtls_x509_crt_init( &cacert );
    
    int ret = 0;

    if( ( ret = mbedtls_x509_crt_parse_file( &cacert, CertFile.c_str() ) ) != 0 )
    {
        //mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
        //goto exit;
        
        SError << "mbedtls_x509_crt_parse returned " << ret ;
        
    }


// remove the following line
// mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_NONE );
   
    
    
    if( mbedtls_ssl_config_defaults(&_ssl_conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT) != 0) 
    {
      SError << "mbedtls_ssl_config_defaults failed";
    }
            
            
            
   
        mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, NULL);
        mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);

       
            
    
#else

 if (           mbedtls_x509_crt_parse(&_cacert, (const unsigned char *) SSL_CA_PEM,
                    sizeof (SSL_CA_PEM)) != 0 ||
                mbedtls_ssl_config_defaults(&_ssl_conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
            _error = true;
            return;
        }

        mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, NULL);
        mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);





#endif

    #if UNSAFE
        mbedtls_ssl_conf_authmode(&_ssl_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
#endif

        
   #if DEBUG_LEVEL > 0
        mbedtls_ssl_conf_verify(&_ssl_conf, my_verify, NULL);
        mbedtls_ssl_conf_dbg(&_ssl_conf, my_debug, NULL);
        mbedtls_debug_set_threshold(DEBUG_LEVEL);
 #endif     

        
        

    setup(&_ssl_conf, nullptr);
        
        
    //

//    if (server)
//    {
//
//    }
    
}




int SSLAdapter::ssl_recv(void *ctx, unsigned char *buf, size_t len) {
   
    SSLAdapter *stream = static_cast<SSLAdapter *>(ctx);
    stream->_socket->on_read((const char*) buf, len);
    
    stream->_socket->on_read((const char*) buf, len);
    
    return len;
   
}

int SSLAdapter::ssl_send(void *ctx, const unsigned char *buf, size_t len) {
    
    SSLAdapter *stream = static_cast<SSLAdapter *>(ctx);
    
     int ret = stream->_socket->Write((const char*)buf, len, stream->cb);
    stream->cb = nullptr;
    
    return ret;
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

    mbedtls_ssl_set_bio(&_ssl, this, ssl_send, ssl_recv, NULL );

    return true;
}


void SSLAdapter::initServer()  //(SSL* ssl)
{
    LTrace("Init server")

}

void SSLAdapter::shutdown()
{
    SInfo << "Shutdown";
            
   int ret = mbedtls_ssl_close_notify(&_ssl);

    if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        
         SError << "Shutdown blocked";
        return;
        
    }
    /* Ignore other errors, the connection may be closed or unusable */

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


void SSLAdapter::handshake()
{
    int ret = 0;
    
    while( ( ret = mbedtls_ssl_handshake( &_ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {

            SError << " failed\n  ! mbedtls_ssl_handshake returned "  << getTLSError(ret);
            return;
        }
    }



    /*
     * 5. Verify the server certificate
     */

    uint32_t flags;
    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &_ssl ) ) != 0 )
    {
        char vrfy_buf[512];

       

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );
        
         SError << " Failed ssl cert verification " << vrfy_buf;

    }

    
}



bool SSLAdapter::isConnected() const {

   _ssl.state == MBEDTLS_SSL_HANDSHAKE_OVER;
}

std::string SSLAdapter::getTLSError(int err)  {
    
    char buf[128];
    
    mbedtls_strerror(err, buf, sizeof buf);
  

    return buf;
}


/* Arvind TBD. Below code does not work with lower version of OpenSSL.
  In future I will replace TLS with DTLS
*/







}  // namespace net
}  // namespace base
