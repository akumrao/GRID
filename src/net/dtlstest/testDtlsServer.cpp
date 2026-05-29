/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
// #include "base/test.h"
#include "base/time.h"
#include "net/SslConnection.h"
#include "DtlsTransport.h"
#include "net/certificate.h"
#include "json/confSettings.h"
#include "json/configuration.h"
#include "configuration.h"

#include "Router.h"


using std::endl;
using namespace base;
using namespace net;

extern ConfCert config;


#if 1

int main(int argc, char** argv) {
    
        Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


 
     //  net::SSLManager::initNoVerifyServer();

        Application app;
        //SecTcpServer *tcpServer = new SecTcpServer(nullptr, "0.0.0.0", 5001, false, true);
        
        rtc::Router router1("1",1);
       // rtc::Router router2("102");
        
        
        base::cnfg::Configuration cnfgSet;
        cnfgSet.load("./config.js");


        try {
            ConfSettings::SetConfiguration(cnfgSet.root);
        } catch (const std::exception& error) {

         //  Settings::exit();
            std::_Exit(-1);
        } 
    


        
        
                
        #if HTTPSSL
        
        config.certificatePemFile = ConfSettings::configuration.certFile;
        config.keyPemFile =ConfSettings::configuration.keyFile  ;

        #else
        SInfo << "http://localhost:8000";
        #endif
        
       // rtc::SctpTransport::Init();
//        rtc::SctpSettings mCurrentSctpSettings = {};
//        rtc::SctpTransport::SetSettings(mCurrentSctpSettings);
        
   
        rtc::DtlsTransport::ClassInit();
      
        rtc::Configuration transportconfig;
        
       rtc::CertificateFingerprint fingerPrint =  config.mCertificate->fingerprint();
        
        router1.HandleRequest(true, transportconfig, 8000, 9000, "127.0.0.1", "127.0.0.1",  fingerPrint);
       // router2.HandleRequest(false,transportconfig, 9000, 8000, "127.0.0.1", "127.0.0.1", fingerPrint);

        app.waitForShutdown([&](void*) {

             router1.Close();
            
             rtc::DtlsTransport::ClassDestroy();

        });



    return 0;
}

#endif

# if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "mbedtls/build_info.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"

#include <iostream>

#define BIND_PORT 4433

typedef struct {
  uv_udp_send_t send_req;
  struct sockaddr_in client_addr;
  unsigned char buffer[2048];
} tx_packet_t;

typedef struct {
  uv_timer_t uv_timer;
  uint32_t intermediate_ms{0};
  uint32_t final_ms{0};
  uint64_t start_time {0};
  int status;
} dtls_timer_ctx_t;

typedef struct {
  mbedtls_ssl_context ssl;
  dtls_timer_ctx_t timer_ctx;
  struct sockaddr_in client_addr;
  unsigned char rx_cache[4096];
  size_t rx_cache_len {0};
  int handshake_done {0};
} client_session_t;

typedef struct {
  uv_loop_t *loop;
  uv_udp_t udp_socket;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
  mbedtls_ssl_cookie_ctx cookie_ctx;
  client_session_t *session {nullptr};
} server_context_t;

// --- Timer Hooks ---
static void uv_timer_cb22(uv_timer_t *handle) {
  dtls_timer_ctx_t *ctx = (dtls_timer_ctx_t *)handle->data;
  ctx->status = 2;

  
 std::cout << "uv_timer_cb2" << std::endl << std::flush;

}

static void dtls_set_timer(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  if (fin_ms == 0) {
    std::cout << "uv_timer_stop" << std::endl << std::flush;
    uv_timer_stop(&t_ctx->uv_timer);
    t_ctx->status = -1;
    return;
  }
  t_ctx->start_time = uv_now(t_ctx->uv_timer.loop);
  t_ctx->status = 0;
  std::cout << "uv_timer_start" << std::endl << std::flush;
  uv_timer_start(&t_ctx->uv_timer, uv_timer_cb2, fin_ms, 0);
}

static int dtls_get_timer(void *ctx) {
  std::cout << "dtls_get_timer" << std::endl << std::flush;
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  if (t_ctx->status == -1)
    return -1;
  uint64_t elapsed = uv_now(t_ctx->uv_timer.loop) - t_ctx->start_time;
  if (elapsed >= t_ctx->final_ms)
    return 2;
  if (elapsed >= t_ctx->intermediate_ms)
    return 1;
  return 0;
}

// --- Dynamic Transmission Hooks ---
static int bio_send(void *ctx, const unsigned char *buf, size_t len) {
  client_session_t *session = (client_session_t *)ctx;
  server_context_t *server = (server_context_t *)session->ssl.MBEDTLS_PRIVATE(
      p_bio); // 3.x Macro Shielding

  tx_packet_t *packet = (tx_packet_t *)malloc(sizeof(tx_packet_t));
  if (!packet)
    return MBEDTLS_ERR_SSL_ALLOC_FAILED;

  memcpy(packet->buffer, buf, len);
  packet->client_addr = session->client_addr;
  uv_buf_t uv_buf = uv_buf_init((char *)packet->buffer, (unsigned int)len);

  int ret = uv_udp_send(&packet->send_req, &server->udp_socket, &uv_buf, 1,
                        (const struct sockaddr *)&packet->client_addr,
                        [](uv_udp_send_t *req, int status) { free(req); });
  if (ret < 0) {
    free(packet);
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }
  return (int)len;
}

static int bio_recv(void *ctx, unsigned char *buf, size_t len) {
  client_session_t *session = (client_session_t *)ctx;
  if (session->rx_cache_len == 0)
    return MBEDTLS_ERR_SSL_WANT_READ;

  size_t copy_len = (len < session->rx_cache_len) ? len : session->rx_cache_len;
  memcpy(buf, session->rx_cache, copy_len);
  if (copy_len < session->rx_cache_len) {
    memmove(session->rx_cache, session->rx_cache + copy_len,
            session->rx_cache_len - copy_len);
    session->rx_cache_len -= copy_len;
  } else {
    session->rx_cache_len = 0;
  }
  return (int)copy_len;
}

static void process_server_dtls(server_context_t *server) {
  client_session_t *session = server->session;
  int ret;
  unsigned char msg_buf[2048];

  if (!session->handshake_done) {
    ret = mbedtls_ssl_handshake(&session->ssl);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
      return;
    if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
      mbedtls_ssl_session_reset(&session->ssl);
      return;
    }
    if (ret != 0)
      return;
    session->handshake_done = 1;
    printf("Client Verified Securely!\n");
    return;
  }

  do {
    ret = mbedtls_ssl_read(&session->ssl, msg_buf, sizeof(msg_buf) - 1);
    if (ret > 0) {
      msg_buf[ret] = '\0';
      printf("Server Echo Handling: %s\n", msg_buf);
      mbedtls_ssl_write(&session->ssl, msg_buf, ret);
    }
  } while (ret > 0);
}

static void on_alloc(uv_handle_t *h, size_t s, uv_buf_t *b) {
  static char buf[2048];
  b->base = buf;
  b->len = sizeof(buf);
}

static void on_read(uv_udp_t *h, ssize_t nread, const uv_buf_t *buf,
                    const struct sockaddr *addr, unsigned f) {
  server_context_t *server = (server_context_t *)h->data;
  if (nread <= 0 || !addr)
    return;

  if (!server->session) {
    client_session_t *session =(client_session_t *)new client_session_t();
    mbedtls_ssl_init(&session->ssl);
    mbedtls_ssl_setup(&session->ssl, &server->conf);

    std::cout << "on_read uv_timer_init" << std::endl << std::flush;

    uv_timer_init(server->loop, &session->timer_ctx.uv_timer);
    session->timer_ctx.uv_timer.data = &session->timer_ctx;
    mbedtls_ssl_set_timer_cb(&session->ssl, &session->timer_ctx, dtls_set_timer,
                             dtls_get_timer);
    mbedtls_ssl_set_bio(&session->ssl, (void *) session, bio_send, bio_recv, NULL);

    // Mbed TLS 3.x internal pointer access adaptation
    //session->ssl.MBEDTLS_PRIVATE(p_bio) = server;

    session->client_addr = *(const struct sockaddr_in *)addr;
    server->session = session;

    mbedtls_ssl_set_client_transport_id(
        &session->ssl, (const unsigned char *)&session->client_addr.sin_addr,
        sizeof(session->client_addr.sin_addr));
  }

  client_session_t *session = server->session;
  if (session->rx_cache_len + nread <= sizeof(session->rx_cache)) {
    memcpy(session->rx_cache + session->rx_cache_len, buf->base, nread);
    session->rx_cache_len += nread;
    process_server_dtls(server);
  }
}

int main() {
  server_context_t server;
 // memset(&server, 0, sizeof(server_context_t));
  server.loop = uv_default_loop();
  server.udp_socket.data = &server;

  struct sockaddr_in bind_addr;
  uv_udp_init(server.loop, &server.udp_socket);
  uv_ip4_addr("0.0.0.0", BIND_PORT, &bind_addr);
  uv_udp_bind(&server.udp_socket, (const struct sockaddr *)&bind_addr, 0);

  mbedtls_ssl_config_init(&server.conf);
  mbedtls_ctr_drbg_init(&server.ctr_drbg);
  mbedtls_entropy_init(&server.entropy);
  mbedtls_ssl_cookie_init(&server.cookie_ctx);

  mbedtls_ctr_drbg_seed(&server.ctr_drbg, mbedtls_entropy_func, &server.entropy,
                        (const unsigned char *)"3x_srv", 6);
  mbedtls_ssl_config_defaults(&server.conf, MBEDTLS_SSL_IS_SERVER,
                              MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                              MBEDTLS_SSL_PRESET_DEFAULT);
  mbedtls_ssl_conf_rng(&server.conf, mbedtls_ctr_drbg_random, &server.ctr_drbg);
  mbedtls_ssl_conf_authmode(&server.conf, MBEDTLS_SSL_VERIFY_NONE);

  mbedtls_ssl_cookie_setup(&server.cookie_ctx, mbedtls_ctr_drbg_random,
                           &server.ctr_drbg);
  mbedtls_ssl_conf_dtls_cookies(&server.conf, mbedtls_ssl_cookie_write,
                                mbedtls_ssl_cookie_check, &server.cookie_ctx);

  printf("Mbed TLS 3.x Server listening on %d...\n", BIND_PORT);
  uv_udp_recv_start(&server.udp_socket, on_alloc, on_read);
  uv_run(server.loop, UV_RUN_DEFAULT);

  if (server.session) {
    mbedtls_ssl_free(&server.session->ssl);
    free(server.session);
  }
  mbedtls_ssl_config_free(&server.conf);
  mbedtls_ssl_cookie_free(&server.cookie_ctx);
  mbedtls_ctr_drbg_free(&server.ctr_drbg);
  mbedtls_entropy_free(&server.entropy);
  return 0;
}







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "mbedtls/build_info.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"
#include "psa/crypto.h" // CRITICAL FIX 1: Explicit header for 3.x PSA Crypto System

#define BIND_PORT 4433
#define RX_BUFFER_SIZE 2048

typedef struct {
  uv_udp_send_t send_req;
  struct sockaddr_in client_addr;
  unsigned char buffer[RX_BUFFER_SIZE];
} tx_packet_t;

typedef struct {
  uv_timer_t uv_timer;
  uint32_t intermediate_ms;
  uint32_t final_ms;
  uint64_t start_time;
  int status;
  void *server_context;
} dtls_timer_ctx_t;

typedef struct {
  mbedtls_ssl_context ssl;
  dtls_timer_ctx_t timer_ctx;
  struct sockaddr_in client_addr;
  unsigned char rx_cache[RX_BUFFER_SIZE];
  size_t rx_cache_len;
  int handshake_done;
} client_session_t;

typedef struct {
  uv_loop_t *loop;
  uv_udp_t udp_socket;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
  mbedtls_ssl_cookie_ctx cookie_ctx;
  client_session_t *session;
} server_context_t;

static void process_server_dtls(server_context_t *server);

static void uv_timer_cb2(uv_timer_t *handle) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)handle->data;
  t_ctx->status = 2;
  server_context_t *server = (server_context_t *)t_ctx->server_context;
  printf("[Timer] Server retransmission timer expired. Handling handshake "
         "flight...\n");
  process_server_dtls(server);
}

static void dtls_set_timer(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  if (fin_ms == 0) {
    uv_timer_stop(&t_ctx->uv_timer);
    t_ctx->status = -1;
    return;
  }
  t_ctx->start_time = uv_now(t_ctx->uv_timer.loop);
  t_ctx->status = 0;
  uv_timer_start(&t_ctx->uv_timer, uv_timer_cb2, fin_ms, 0);
}

static int dtls_get_timer(void *ctx) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  if (t_ctx->status == -1)
    return -1;
  uint64_t elapsed = uv_now(t_ctx->uv_timer.loop) - t_ctx->start_time;
  if (elapsed >= t_ctx->final_ms)
    return 2;
  if (elapsed >= t_ctx->intermediate_ms)
    return 1;
  return 0;
}

static int bio_send(void *ctx, const unsigned char *buf, size_t len) {
  client_session_t *session = (client_session_t *)ctx;
  server_context_t *server =
      (server_context_t *)session->ssl.MBEDTLS_PRIVATE(p_bio);

  tx_packet_t *packet = (tx_packet_t *)malloc(sizeof(tx_packet_t));
  if (!packet)
    return MBEDTLS_ERR_SSL_ALLOC_FAILED;

  memcpy(packet->buffer, buf, len);
  packet->client_addr = session->client_addr;
  uv_buf_t uv_buf = uv_buf_init((char *)packet->buffer, (unsigned int)len);

  int ret = uv_udp_send(&packet->send_req, &server->udp_socket, &uv_buf, 1,
                        (const struct sockaddr *)&packet->client_addr,
                        [](uv_udp_send_t *req, int status) { free(req); });
  if (ret < 0) {
    free(packet);
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }
  return (int)len;
}

static int bio_recv(void *ctx, unsigned char *buf, size_t len) {
  client_session_t *session = (client_session_t *)ctx;
  if (session->rx_cache_len == 0)
    return MBEDTLS_ERR_SSL_WANT_READ;

  size_t copy_len = (len < session->rx_cache_len) ? len : session->rx_cache_len;
  memcpy(buf, session->rx_cache, copy_len);
  if (copy_len < session->rx_cache_len) {
    memmove(session->rx_cache, session->rx_cache + copy_len,
            session->rx_cache_len - copy_len);
    session->rx_cache_len -= copy_len;
  } else {
    session->rx_cache_len = 0;
  }
  return (int)copy_len;
}

static void process_server_dtls(server_context_t *server) {
  client_session_t *session = server->session;
  if (!session)
    return;

  int ret;
  unsigned char msg_buf[RX_BUFFER_SIZE];

  if (!session->handshake_done) {
    ret = mbedtls_ssl_handshake(&session->ssl);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
      return;
    }
    if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
      printf("Server sent HelloVerifyRequest cookie challenge to client.\n");
      mbedtls_ssl_session_reset(&session->ssl);
      return;
    }
    if (ret != 0) {
      char err_buf[100];
      mbedtls_strerror(ret, err_buf, 100);
      printf("Server Handshake Error: %s (-0x%x)\n", err_buf, -ret);
      return;
    }
    session->handshake_done = 1;
    printf("SUCCESS: Client Verified Securely via DTLS!\n");
    return;
  }

  do {
    ret = mbedtls_ssl_read(&session->ssl, msg_buf, sizeof(msg_buf) - 1);
    if (ret > 0) {
      msg_buf[ret] = '\0';
      printf("Server Echoing Back Application Data: %s\n", msg_buf);
      mbedtls_ssl_write(&session->ssl, msg_buf, ret);
    }
  } while (ret > 0);
}

static void on_alloc(uv_handle_t *h, size_t s, uv_buf_t *b) {
  static char buf[RX_BUFFER_SIZE];
  b->base = buf;
  b->len = sizeof(buf);
}

static void on_read(uv_udp_t *h, ssize_t nread, const uv_buf_t *buf,
                    const struct sockaddr *addr, unsigned f) {
  server_context_t *server = (server_context_t *)h->data;
  if (nread <= 0 || !addr)
    return;

  if (!server->session) {
    client_session_t *session =
        (client_session_t *)calloc(1, sizeof(client_session_t));
    mbedtls_ssl_init(&session->ssl);
    mbedtls_ssl_setup(&session->ssl, &server->conf);

    uv_timer_init(server->loop, &session->timer_ctx.uv_timer);
    session->timer_ctx.uv_timer.data = &session->timer_ctx;
    session->timer_ctx.server_context = server;

    mbedtls_ssl_set_timer_cb(&session->ssl, &session->timer_ctx, dtls_set_timer,
                             dtls_get_timer);
    mbedtls_ssl_set_bio(&session->ssl, session, bio_send, bio_recv, NULL);

    session->ssl.MBEDTLS_PRIVATE(p_bio) = server;
    session->client_addr = *(const struct sockaddr_in *)addr;
    server->session = session;

    mbedtls_ssl_set_client_transport_id(
        &session->ssl, (const unsigned char *)&session->client_addr.sin_addr,
        sizeof(session->client_addr.sin_addr));
  }

  client_session_t *session = server->session;
  if (session->rx_cache_len + nread <= sizeof(session->rx_cache)) {
    memcpy(session->rx_cache + session->rx_cache_len, buf->base, nread);
    session->rx_cache_len += nread;
    process_server_dtls(server);
  }
}

int main() {
  int ret;
  server_context_t server;
  memset(&server, 0, sizeof(server_context_t));

  // CRITICAL FIX 1: Initialize PSA Crypto subsystem required by 3.x internals
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("CRITICAL: PSA Crypto initialization failed! (0x%x)\n", ret);
    return -1;
  }

  server.loop = uv_default_loop();
  server.udp_socket.data = &server;

  struct sockaddr_in bind_addr;
  uv_udp_init(server.loop, &server.udp_socket);
  uv_ip4_addr("0.0.0.0", BIND_PORT, &bind_addr);
  uv_udp_bind(&server.udp_socket, (const struct sockaddr *)&bind_addr, 0);

  mbedtls_ssl_config_init(&server.conf);
  mbedtls_ctr_drbg_init(&server.ctr_drbg);
  mbedtls_entropy_init(&server.entropy);
  mbedtls_ssl_cookie_init(&server.cookie_ctx);

  ret = mbedtls_ctr_drbg_seed(&server.ctr_drbg, mbedtls_entropy_func,
                              &server.entropy, (const unsigned char *)"3x_srv",
                              6);
  if (ret != 0)
    return ret;

  ret = mbedtls_ssl_config_defaults(&server.conf, MBEDTLS_SSL_IS_SERVER,
                                    MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0)
    return ret;

  mbedtls_ssl_conf_rng(&server.conf, mbedtls_ctr_drbg_random, &server.ctr_drbg);
  mbedtls_ssl_conf_authmode(&server.conf, MBEDTLS_SSL_VERIFY_NONE);

  // CRITICAL FIX 2: Enforce DTLS 1.2 bounds to eliminate unconfigured 3.x
  // fallback crashes
  mbedtls_ssl_conf_max_version(&server.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);
  mbedtls_ssl_conf_min_version(&server.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);

  ret = mbedtls_ssl_cookie_setup(&server.cookie_ctx, mbedtls_ctr_drbg_random,
                                 &server.ctr_drbg);
  if (ret != 0)
    return ret;
  mbedtls_ssl_conf_dtls_cookies(&server.conf, mbedtls_ssl_cookie_write,
                                mbedtls_ssl_cookie_check, &server.cookie_ctx);

  printf("Mbed TLS 3.x Server listening safely on Port %d...\n", BIND_PORT);
  uv_udp_recv_start(&server.udp_socket, on_alloc, on_read);
  uv_run(server.loop, UV_RUN_DEFAULT);

  if (server.session) {
    mbedtls_ssl_free(&server.session->ssl);
    free(server.session);
  }
  mbedtls_ssl_config_free(&server.conf);
  mbedtls_ssl_cookie_free(&server.cookie_ctx);
  mbedtls_ctr_drbg_free(&server.ctr_drbg);
  mbedtls_entropy_free(&server.entropy);
  return 0;
}



#endif

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "mbedtls/build_info.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"
#include "psa/crypto.h" // CRITICAL FIX 1: Explicit header for 3.x PSA Crypto System

#define BIND_PORT 4433
#define RX_BUFFER_SIZE 2048

typedef struct {
  uv_udp_send_t send_req;
  struct sockaddr_in client_addr;
  unsigned char buffer[RX_BUFFER_SIZE];
} tx_packet_t;

typedef struct {
  uv_timer_t uv_timer;
  uint32_t intermediate_ms;
  uint32_t final_ms;
  uint64_t start_time;
  int status;
  void *server_context;
} dtls_timer_ctx_t;

typedef struct {
  mbedtls_ssl_context ssl;
  dtls_timer_ctx_t timer_ctx;
  struct sockaddr_in client_addr;
  unsigned char rx_cache[RX_BUFFER_SIZE];
  size_t rx_cache_len;
  int handshake_done;
} client_session_t;

typedef struct {
  uv_loop_t *loop;
  uv_udp_t udp_socket;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
  mbedtls_ssl_cookie_ctx cookie_ctx;
  client_session_t *session;
} server_context_t;

static void process_server_dtls(server_context_t *server);

static void uv_timer_cb2(uv_timer_t *handle) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)handle->data;
  t_ctx->status = 2;
  server_context_t *server = (server_context_t *)t_ctx->server_context;
  printf("[Timer] Server retransmission timer expired. Handling handshake "
         "flight...\n");
  process_server_dtls(server);
}

static void dtls_set_timer(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  if (fin_ms == 0) {
    uv_timer_stop(&t_ctx->uv_timer);
    t_ctx->status = -1;
    return;
  }
  t_ctx->start_time = uv_now(t_ctx->uv_timer.loop);
  t_ctx->status = 0;
  uv_timer_start(&t_ctx->uv_timer, uv_timer_cb2, fin_ms, 0);
}

static int dtls_get_timer(void *ctx) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  if (t_ctx->status == -1)
    return -1;
  uint64_t elapsed = uv_now(t_ctx->uv_timer.loop) - t_ctx->start_time;
  if (elapsed >= t_ctx->final_ms)
    return 2;
  if (elapsed >= t_ctx->intermediate_ms)
    return 1;
  return 0;
}

static int bio_send(void *ctx, const unsigned char *buf, size_t len) {
  client_session_t *session = (client_session_t *)ctx;
  server_context_t *server =
      (server_context_t *)session->ssl.MBEDTLS_PRIVATE(p_bio);

  tx_packet_t *packet = (tx_packet_t *)malloc(sizeof(tx_packet_t));
  if (!packet)
    return MBEDTLS_ERR_SSL_ALLOC_FAILED;

  memcpy(packet->buffer, buf, len);
  packet->client_addr = session->client_addr;
  uv_buf_t uv_buf = uv_buf_init((char *)packet->buffer, (unsigned int)len);

  int ret = uv_udp_send(&packet->send_req, &server->udp_socket, &uv_buf, 1,
                        (const struct sockaddr *)&packet->client_addr,
                        [](uv_udp_send_t *req, int status) { free(req); });
  if (ret < 0) {
    free(packet);
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }
  return (int)len;
}

static int bio_recv(void *ctx, unsigned char *buf, size_t len) {
  client_session_t *session = (client_session_t *)ctx;
  if (session->rx_cache_len == 0)
    return MBEDTLS_ERR_SSL_WANT_READ;

  size_t copy_len = (len < session->rx_cache_len) ? len : session->rx_cache_len;
  memcpy(buf, session->rx_cache, copy_len);
  if (copy_len < session->rx_cache_len) {
    memmove(session->rx_cache, session->rx_cache + copy_len,
            session->rx_cache_len - copy_len);
    session->rx_cache_len -= copy_len;
  } else {
    session->rx_cache_len = 0;
  }
  return (int)copy_len;
}

static void process_server_dtls(server_context_t *server) {
  client_session_t *session = server->session;
  if (!session)
    return;

  int ret;
  unsigned char msg_buf[RX_BUFFER_SIZE];

  if (!session->handshake_done) {
    ret = mbedtls_ssl_handshake(&session->ssl);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
      return;
    }
    if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
      printf("Server sent HelloVerifyRequest cookie challenge to client.\n");
      mbedtls_ssl_session_reset(&session->ssl);
      return;
    }
    if (ret != 0) {
      char err_buf[100];
      mbedtls_strerror(ret, err_buf, 100);
      printf("Server Handshake Error: %s (-0x%x)\n", err_buf, -ret);
      return;
    }
    session->handshake_done = 1;
    printf("SUCCESS: Client Verified Securely via DTLS!\n");
    return;
  }

  do {
    ret = mbedtls_ssl_read(&session->ssl, msg_buf, sizeof(msg_buf) - 1);
    if (ret > 0) {
      msg_buf[ret] = '\0';
      printf("Server Echoing Back Application Data: %s\n", msg_buf);
      mbedtls_ssl_write(&session->ssl, msg_buf, ret);
    }
  } while (ret > 0);
}

static void on_alloc(uv_handle_t *h, size_t s, uv_buf_t *b) {
  static char buf[RX_BUFFER_SIZE];
  b->base = buf;
  b->len = sizeof(buf);
}

static void on_read(uv_udp_t *h, ssize_t nread, const uv_buf_t *buf,
                    const struct sockaddr *addr, unsigned f) {
  server_context_t *server = (server_context_t *)h->data;
  if (nread <= 0 || !addr)
    return;

  if (!server->session) {
    client_session_t *session =
        (client_session_t *)calloc(1, sizeof(client_session_t));
    mbedtls_ssl_init(&session->ssl);
    mbedtls_ssl_setup(&session->ssl, &server->conf);

    uv_timer_init(server->loop, &session->timer_ctx.uv_timer);
    session->timer_ctx.uv_timer.data = &session->timer_ctx;
    session->timer_ctx.server_context = server;

    mbedtls_ssl_set_timer_cb(&session->ssl, &session->timer_ctx, dtls_set_timer,
                             dtls_get_timer);
    mbedtls_ssl_set_bio(&session->ssl, session, bio_send, bio_recv, NULL);

   // session->ssl.MBEDTLS_PRIVATE(p_bio) = server;
    session->client_addr = *(const struct sockaddr_in *)addr;
    server->session = session;

    mbedtls_ssl_set_client_transport_id(
        &session->ssl, (const unsigned char *)&session->client_addr.sin_addr,
        sizeof(session->client_addr.sin_addr));
  }

  client_session_t *session = server->session;
  if (session->rx_cache_len + nread <= sizeof(session->rx_cache)) {
    memcpy(session->rx_cache + session->rx_cache_len, buf->base, nread);
    session->rx_cache_len += nread;
    process_server_dtls(server);
  }
}

int main() {
  int ret;
  server_context_t server;
  memset(&server, 0, sizeof(server_context_t));

  // CRITICAL FIX 1: Initialize PSA Crypto subsystem required by 3.x internals
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("CRITICAL: PSA Crypto initialization failed! (0x%x)\n", ret);
    return -1;
  }

  server.loop = uv_default_loop();
  server.udp_socket.data = &server;

  struct sockaddr_in bind_addr;
  uv_udp_init(server.loop, &server.udp_socket);
  uv_ip4_addr("0.0.0.0", BIND_PORT, &bind_addr);
  uv_udp_bind(&server.udp_socket, (const struct sockaddr *)&bind_addr, 0);

  mbedtls_ssl_config_init(&server.conf);
  mbedtls_ctr_drbg_init(&server.ctr_drbg);
  mbedtls_entropy_init(&server.entropy);
  mbedtls_ssl_cookie_init(&server.cookie_ctx);

  ret = mbedtls_ctr_drbg_seed(&server.ctr_drbg, mbedtls_entropy_func,
                              &server.entropy, (const unsigned char *)"3x_srv",
                              6);
  if (ret != 0)
    return ret;

  ret = mbedtls_ssl_config_defaults(&server.conf, MBEDTLS_SSL_IS_SERVER,
                                    MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0)
    return ret;

  mbedtls_ssl_conf_rng(&server.conf, mbedtls_ctr_drbg_random, &server.ctr_drbg);
  mbedtls_ssl_conf_authmode(&server.conf, MBEDTLS_SSL_VERIFY_NONE);

  // CRITICAL FIX 2: Enforce DTLS 1.2 bounds to eliminate unconfigured 3.x
  // fallback crashes
  mbedtls_ssl_conf_max_version(&server.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);
  mbedtls_ssl_conf_min_version(&server.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);

  ret = mbedtls_ssl_cookie_setup(&server.cookie_ctx, mbedtls_ctr_drbg_random,
                                 &server.ctr_drbg);
  if (ret != 0)
    return ret;
  mbedtls_ssl_conf_dtls_cookies(&server.conf, mbedtls_ssl_cookie_write,
                                mbedtls_ssl_cookie_check, &server.cookie_ctx);

  printf("Mbed TLS 3.x Server listening safely on Port %d...\n", BIND_PORT);
  uv_udp_recv_start(&server.udp_socket, on_alloc, on_read);
  uv_run(server.loop, UV_RUN_DEFAULT);

  if (server.session) {
    mbedtls_ssl_free(&server.session->ssl);
    free(server.session);
  }
  mbedtls_ssl_config_free(&server.conf);
  mbedtls_ssl_cookie_free(&server.cookie_ctx);
  mbedtls_ctr_drbg_free(&server.ctr_drbg);
  mbedtls_entropy_free(&server.entropy);
  return 0;
}

#endif