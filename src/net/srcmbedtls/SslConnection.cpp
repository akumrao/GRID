/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "net/netInterface.h"
#include "net/SslConnection.h"
#include "base/logger.h"
//#include "net/sslmanager.h"
#include <assert.h>

using namespace std;


namespace base {
namespace net {


SslConnection::SslConnection()
    : TcpConnectionBase(nullptr, true)
//    , _sslContext(nullptr)
//    , _sslSession(nullptr)
    , _sslAdapter(this)
  
{
    LTrace("Create")
     _sslAdapter.initClient();
 
}


SslConnection::SslConnection( bool server)
    : TcpConnectionBase(nullptr, true)
  //  , _sslSession(nullptr)
    , _sslAdapter(this)
    ,serverMode(server)
{
    if(server)
        _sslAdapter.initServer();
    LTrace("Create")
}

/*
SslConnection::SslConnection(Listener* listener, SSLContext::Ptr context, SSLSession::Ptr session)
    :  TcpConnection(listener)
    , _sslContext(context)
    , _sslSession(session)
    , _sslAdapter(this)
    ,listener(listener)
{
    LTrace("Create")
}
*/

SslConnection::~SslConnection()
{
    _sslAdapter.shutdown();
    LTrace("Destroy")
}






void SslConnection::send(const char* data, size_t len)
{
 
    int ret;
    
    do
    {
        
        ret = mbedtls_ssl_read( &_sslAdapter._ssl, (unsigned char*) data, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            SError <<  "failed\n  ! mbedtls_ssl_read returned "  <<  ret;
            break;
        }

        if( ret == 0 )
        {
         
            break;
        }

     
    }
    while( 1 );
    
    return ;
}


void SslConnection::tcpsend(const char* data, size_t len, onSendCallback _cb)
{
//     assert(_sslAdapter._ssl);
    _sslAdapter.cb = _cb;
    send( data, len);
    return ;
}
//
// Callbacks

void SslConnection::on_tls_read(const char* data, size_t len)
{
    SInfo << "On SSL read: " << len << " data"  <<  data ; 

    int ret =0;
    while( ( ret = mbedtls_ssl_write( & _sslAdapter._ssl, (const unsigned char*) data, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            SError <<  " failed\n  ! mbedtls_ssl_write failed " <<  ret ;
            break;
        }
    }
    
}

void SslConnection::on_read(const char* data, size_t len)
{
   // LTrace("SslConnection::on_read ", len)
   // LTrace("SslConnection::on_read ", data)

//    listener->on_read(this,data,len );
    
    throw(" Never code should reach here, please override this function");
  
}

void SslConnection::on_connect()
{
    LTrace("SSL On connect")
 
    _sslAdapter.handshake();        
}


} // namespace net
} // namespace base

