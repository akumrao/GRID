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

        ret =  mbedtls_pk_parse_keyfile( &pkey,  KeyFile ,pwd );
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
                         mbedtls_test_srv_key_len, NULL, 0 );
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

    if (len > MBEDTLS_SSL_MAX_CONTENT_LEN) 
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
