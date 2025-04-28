
#include "net/dns.h"

#include "base/logger.h"

namespace base {

    namespace net {

      
               void GetAddrInfoReq::on_resolved(uv_getaddrinfo_t* handle, int status, struct addrinfo* res) {
                //struct getaddrinfo_req* req;
                
                GetAddrInfoReq *obj = (GetAddrInfoReq*) handle->data;
                
                if (status < 0 || !res) {
                    LTrace(  "getaddrinfo callback error ", uv_err_name(status));
                    
                    if(res)
                    uv_freeaddrinfo(res);
                    delete handle;
                    
                    obj->cbDnsResolve(nullptr, "");
                    return;
                }
                
                char addr[40] = {'\0'};

                struct addrinfo* start =  res;
                
                for (;res != NULL; res = res->ai_next) 
                { 
                    
                    if (res->ai_family == AF_INET) {
                        // ipv4
                        //char c[17] = { '\0' };
                        uv_ip4_name((sockaddr_in*)(res->ai_addr), addr, 16);
                    } else if (res->ai_family == AF_INET6) {
                        // ipv6
                        //char c[40] = { '\0' };
                        uv_ip6_name((sockaddr_in6*)(res->ai_addr), addr, 39);
                    }
                    LTrace("address ",  addr);
                    // uv_tcp_connect(connect_req, socket, (const struct sockaddr*) res->ai_addr, on_connect);

                    obj->cbDnsResolve(res, addr);
                }
                

                uv_freeaddrinfo(res);
                
                delete handle;

            }

            void GetAddrInfoReq::resolve(const std::string& host, int port, uv_loop_t * loop) {

                req = new uv_getaddrinfo_t; 
                req->data = this;
                int r;

                struct addrinfo hints;
//                hints.ai_family = PF_INET;
//                hints.ai_socktype = SOCK_STREAM;
//                hints.ai_protocol = IPPROTO_TCP;
//                hints.ai_flags = 0;
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_protocol = IPPROTO_UDP;
                hints.ai_flags = AI_ADDRCONFIG;

                r = uv_getaddrinfo(loop,
                        req,
                        on_resolved,
                        host.c_str(),
                        util::itostr(port).c_str(),
                                  &hints);
                assert(r == 0);

            }
        
        
           

    } // namespace net
} // base

