/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


/* 
 * File:   DNS.h
 * Author: root
 *
 * Created on November 26, 2019, 2:29 PM
 */

#ifndef DNS_H
#define DNS_H

#include "uv.h"
#include <string>
#include "base/util.h"
#include "base/application.h"

namespace base {

    namespace net {

        struct GetAddrInfoReq {

            virtual void cbDnsResolve(addrinfo* res, std::string ip) {
               // LTrace("GetAddrInfoReq::cbDnsResolve");
            }

    
            static void on_resolved(uv_getaddrinfo_t* handle, int status, struct addrinfo* res) ;

            void resolve(const std::string& host, int port, uv_loop_t * loop = Application::uvGetLoop()) ;

            uv_getaddrinfo_t *req;
        };

    } // namespace net
} // base


#endif /* DNS_H */

