/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef Net_SSLAdapter_H
#define Net_SSLAdapter_H


#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time            time
#define mbedtls_time_t          time_t
#define mbedtls_fprintf         fprintf
#define mbedtls_printf          printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif /* MBEDTLS_PLATFORM_C */

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) ||  \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_RSA_C) ||         \
    !defined(MBEDTLS_CERTS_C) || !defined(MBEDTLS_PEM_PARSE_C) || \
    !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_X509_CRT_PARSE_C)
    
int main( void )
{
    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_ENTROPY_C and/or "
           "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_CLI_C and/or "
           "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
           "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C "
           "not defined.\n");
    mbedtls_exit( 0 );
}
#else

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#endif


#if DEBUG_LEVEL > 0
#include "mbedtls/debug.h"
#endif



#include <string>
#include <vector>

#include "bio.h"


#include <functional>

namespace base {
    
using onSendCallback =  std::function<void(bool sent)>;
    
namespace net {



/// A wrapper for the OpenSSL SSL connection context
///
/// TODO: Decouple from SSLSocket implementation
class  SslConnection;
class  SSLAdapter
{
public:
    SSLAdapter(SslConnection* socket);
    ~SSLAdapter();

    /// Initializes the SSL context as a client.
    void initSSL();

    /// Initializes the SSL context as a server.
    //void initServer();

    /// Returns true when SSL context has been initialized.
   // bool initialized() const;

    /// Returns true when the handshake is complete.

    /// Start/continue the SSL handshake process.
    int handshake();

    /// Returns the number of bytes available in
    /// the SSL buffer for immediate reading.


    /// Issues an orderly SSL shutdown.
    void shutdown();
    
    
    //bool isConnected() const ;

    std::string getTLSError(int );

    void stay_uptodate( );
    int swrap_error_handler( const int err_code);
    
    void addIncomingData(const char *data, size_t len);
    void addOutgoingData(const char *data, size_t len);
    
    static void my_debug(void *ctx, int level, const char *file, int line,   const char *str);
    static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags);

    
    bool setup(const mbedtls_ssl_config *conf, const char *hostname);
    
    onSendCallback  cb{nullptr};
    
    bool server{false};

protected:
    
    


 BIO     *app_bio_; //Our BIO, All IO should be through this
 BIO     *ssl_bio_; //the ssl BIO used only by openSSL




enum uv_tls_state {
    STATE_INIT         = 0x0
    ,STATE_HANDSHAKING = 0x1
    ,STATE_HANDSHAKE_DONE  = 0x2 //read or write mode
    ,STATE_CLOSING     = 0x4 // This means closed state also
};


int  handshake_state{STATE_INIT};
    
protected:
    friend class SslConnection;

    SslConnection* _socket;
     
    
    std::vector<char> _bufferOut; ///<  The outgoing payload to be encrypted and sent
    
    
    
    
    mbedtls_ssl_context _ssl;           /**< TLS context            */
    int _ssl_error;                     /**< Last TLS error         */
    
    
    mbedtls_entropy_context _entropy;
    mbedtls_ctr_drbg_context _ctr_drbg;
    mbedtls_x509_crt _cacert;
    mbedtls_ssl_config _ssl_conf;
    
    
    mbedtls_pk_context pkey;
    
};


} // namespace net
} // namespace base


#endif // Net_SSLAdapter_H
