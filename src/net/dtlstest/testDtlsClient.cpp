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
        

        rtc::Router router2("1", 1);
        
        
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
        //rtc::SctpSettings mCurrentSctpSettings = {};
        //rtc::SctpTransport::SetSettings(mCurrentSctpSettings);
        
   
        rtc::DtlsTransport::ClassInit();
      
        rtc::Configuration transportconfig;
        
       rtc::CertificateFingerprint fingerPrint =  config.mCertificate->fingerprint();
        
        //router1.HandleRequest(true, transportconfig, 8000, 9000, "127.0.0.1", "127.0.0.1",  fingerPrint);
        router2.HandleRequest(false,transportconfig, 9000, 8000, "192.168.0.19", "192.168.0.20", fingerPrint);
        
        
        //sleep(500);
        
        

//        uv_signal_t sig;
//        uv_signal_init(app.uvGetLoop(), &sig);
//        uv_signal_start(&sig, signal_cb, SIGINT);
        
  //      app.run();
//
      app.waitForShutdown([&](void*) 
        {

            router2.Close();
        
            rtc::DtlsTransport::ClassDestroy();
            
            // rtc::DtlsTransport::ClassDestroy();

        }
        
       );



    return 0;
}


#endif

#if 0


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/timing.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4433

// --- Context Structs ---

// Keeps outbound packet data alive until Windows completes the async UDP send
typedef struct {
  uv_udp_send_t send_req;
  unsigned char buffer[2048];
} tx_packet_t;

// Custom timer context mapped to Mbed TLS timing expectations
typedef struct {
  uv_timer_t uv_timer;
  uint32_t intermediate_ms;
  uint32_t final_ms;
  uint64_t start_time;
  int status;
} dtls_timer_ctx_t;

// Main application context
typedef struct {
  uv_loop_t *loop;
  uv_udp_t udp_socket;
  struct sockaddr_in server_addr;

  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;

  dtls_timer_ctx_t timer_ctx;

  // Inbound raw network cache for the BIO reader
  unsigned char rx_cache[2048];
  size_t rx_cache_len;

  mbedtls_ssl_states state{MBEDTLS_SSL_HANDSHAKE_WRAPUP};
} app_context_t;

// --- DTLS Timer Implementation ---

static void uv_timer_cb2(uv_timer_t *handle) {
  dtls_timer_ctx_t *ctx = (dtls_timer_ctx_t *)handle->data;
  ctx->status = 2; // Final timeout expired
}

static void dtls_set_timer(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  t_ctx->intermediate_ms = int_ms;
  t_ctx->final_ms = fin_ms;

  if (fin_ms == 0) {
    uv_timer_stop(&t_ctx->uv_timer);
    t_ctx->status = -1; // Timer cancelled
    return;
  }

  t_ctx->start_time = uv_now(t_ctx->uv_timer.loop);
  t_ctx->status = 0; // Timer running

  uv_timer_start(&t_ctx->uv_timer, uv_timer_cb2, fin_ms, 0);
}

static int dtls_get_timer(void *ctx) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  if (t_ctx->status == -1)
    return -1;

  uint64_t elapsed = uv_now(t_ctx->uv_timer.loop) - t_ctx->start_time;

  if (elapsed >= t_ctx->final_ms) {
    t_ctx->status = 2;
    return 2;
  }
  if (elapsed >= t_ctx->intermediate_ms) {
    return 1;
  }
  return 0;
}

// --- Mbed TLS BIO Callbacks ---

// Triggered by Mbed TLS when it wants to send DTLS packets to the network
static int bio_send(void *ctx, const unsigned char *buf, size_t len) {
  app_context_t *app = (app_context_t *)ctx;

  // CRASH FIX: Allocate on heap so buffer survives until libuv callback
  // triggers
  tx_packet_t *packet = (tx_packet_t *)malloc(sizeof(tx_packet_t));
  if (!packet)
    return MBEDTLS_ERR_SSL_ALLOC_FAILED;

  memcpy(packet->buffer, buf, len);
  uv_buf_t uv_buf = uv_buf_init((char *)packet->buffer, (unsigned int)len);

  void (*send_cb)(uv_udp_send_t *, int) = [](uv_udp_send_t *req, int status) {
    tx_packet_t *pkt = (tx_packet_t *)req;
    free(pkt); // CRASH FIX: Safe to free now that Windows is finished
  };

  int ret = uv_udp_send(&packet->send_req, &app->udp_socket, &uv_buf, 1,
                        (const struct sockaddr *)&app->server_addr, send_cb);
  if (ret < 0) {
    free(packet);
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }

  return (int)len;
}

// Triggered by Mbed TLS when it expects incoming DTLS packets
static int bio_recv(void *ctx, unsigned char *buf, size_t len) {
  app_context_t *app = (app_context_t *)ctx;

  // If no data arrived in the libuv event loop cache, signal non-blocking yield
  if (app->rx_cache_len == 0) {
    return MBEDTLS_ERR_SSL_WANT_READ;
  }

  size_t copy_len = (len < app->rx_cache_len) ? len : app->rx_cache_len;
  memcpy(buf, app->rx_cache, copy_len);

  // Shift remaining bytes forward if any
  if (copy_len < app->rx_cache_len) {
    memmove(app->rx_cache, app->rx_cache + copy_len,
            app->rx_cache_len - copy_len);
    app->rx_cache_len -= copy_len;
  } else {
    app->rx_cache_len = 0;
  }

  return (int)copy_len;
}

// --- Step-by-step Processing Driving the Protocol ---

static void process_dtls_state(app_context_t *app) {
  int ret;
  unsigned char msg_buf[1024];

  // Case A: Handshake is still in progress
  if (app->state != MBEDTLS_SSL_HANDSHAKE_OVER) {
    ret = mbedtls_ssl_handshake(&app->ssl);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
      return; // Wait for the next event loop tick
    }
    if (ret != 0) {
      printf("CRITICAL: DTLS Handshake failed (-0x%x)\n", -ret);
      uv_stop(app->loop);
      return;
    }
    printf("SUCCESS: DTLS Handshake completed securely!\n");

    app->state = MBEDTLS_SSL_HANDSHAKE_OVER;

    // Write initial data once handshake connects
    const char *hello = "Hello Secure World over Libuv!";
    mbedtls_ssl_write(&app->ssl, (const unsigned char *)hello, strlen(hello));
    return;
  }

  // Case B: Handshake completed, read secure data application payloads
  do {
    ret = mbedtls_ssl_read(&app->ssl, msg_buf, sizeof(msg_buf) - 1);
    if (ret > 0) {
      msg_buf[ret] = '\0';
      printf("Received Decrypted App Data: %s\n", msg_buf);
    }
  } while (ret > 0);
}

// --- Libuv Networking Callbacks ---

static void on_alloc(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
  static char shared_recv_buffer[2048];
  buf->base = shared_recv_buffer;
  buf->len = sizeof(shared_recv_buffer);
}

static void on_read(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                    const struct sockaddr *addr, unsigned flags) {
  app_context_t *app = (app_context_t *)handle->data;

  if (nread < 0) {
    printf("Network Read Error: %s\n", uv_err_name((int)nread));
    return;
  }
  if (nread == 0)
    return; // Empty packet notification

  // Cache the network packet so Mbed TLS BIO layer can read it
  if (app->rx_cache_len + nread <= sizeof(app->rx_cache)) {
    memcpy(app->rx_cache + app->rx_cache_len, buf->base, nread);
    app->rx_cache_len += nread;

    // Feed the freshly arrived network buffer to Mbed TLS
    process_dtls_state(app);
  } else {
    printf("Warning: RX Cache overflow, packet dropped\n");
  }
}

// --- Initialization and Setup ---

int main() {
  int ret;
  app_context_t app;
  memset(&app, 0, sizeof(app_context_t));

  app.loop = uv_default_loop();
  app.udp_socket.data = &app;
  app.timer_ctx.uv_timer.data = &app.timer_ctx;

  // Initialize Windows socket layer via libuv
  uv_udp_init(app.loop, &app.udp_socket);
  uv_timer_init(app.loop, &app.timer_ctx.uv_timer);
  uv_ip4_addr(SERVER_IP, SERVER_PORT, &app.server_addr);

  // Initialize Mbed TLS Context structures
  mbedtls_ssl_init(&app.ssl);
  mbedtls_ssl_config_init(&app.conf);
  mbedtls_ctr_drbg_init(&app.ctr_drbg);
  mbedtls_entropy_init(&app.entropy);

  // Seed Random Number Generator (RNG)
  ret = mbedtls_ctr_drbg_seed(&app.ctr_drbg, mbedtls_entropy_func, &app.entropy,
                              (const unsigned char *)"uv_dtls", 7);
  if (ret != 0)
    return ret;

  // Load DTLS configuration defaults as client
  ret = mbedtls_ssl_config_defaults(&app.conf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0)
    return ret;

  mbedtls_ssl_conf_rng(&app.conf, mbedtls_ctr_drbg_random, &app.ctr_drbg);

  // NOTE: In production, substitute MBEDTLS_SSL_VERIFY_NONE with certificate
  // validation
  mbedtls_ssl_conf_authmode(&app.conf, MBEDTLS_SSL_VERIFY_NONE);

  ret = mbedtls_ssl_setup(&app.ssl, &app.conf);
  if (ret != 0)
    return ret;

  // CRASH FIX: Assign libuv custom timer callbacks for handshake
  // retransmissions
  mbedtls_ssl_set_timer_cb(&app.ssl, &app.timer_ctx, dtls_set_timer,
                           dtls_get_timer);

  // Connect Mbed TLS to our libuv abstract translation BIO functions
  mbedtls_ssl_set_bio(&app.ssl, &app, bio_send, bio_recv, NULL);

  // Begin listening for asynchronous network returns via libuv loop
  uv_udp_recv_start(&app.udp_socket, on_alloc, on_read);

  printf("Starting DTLS Client Connection Session...\n");
  process_dtls_state(&app); // Trigger first ClientHello flight

  // Start blocking processing on single main thread loop execution
  uv_run(app.loop, UV_RUN_DEFAULT);

  // Cleanup Resources
  mbedtls_ssl_free(&app.ssl);
  mbedtls_ssl_config_free(&app.conf);
  mbedtls_ctr_drbg_free(&app.ctr_drbg);
  mbedtls_entropy_free(&app.entropy);

  return 0;
}







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "mbedtls/build_info.h" // Required in Mbed TLS 3.x instead of config.h
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4433

typedef struct {
  uv_udp_send_t send_req;
  unsigned char buffer[2048];
} tx_packet_t;

typedef struct {
  uv_timer_t uv_timer;
  uint32_t intermediate_ms;
  uint32_t final_ms;
  uint64_t start_time;
  int status;
} dtls_timer_ctx_t;

typedef struct {
  uv_loop_t *loop;
  uv_udp_t udp_socket;
  struct sockaddr_in server_addr;

  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;

  dtls_timer_ctx_t timer_ctx;

  unsigned char rx_cache[4096];
  size_t rx_cache_len;
  int handshake_complete; // Replacement flag for private app.ssl.state
} app_context_t;

// --- Mbed TLS 3.x Compliant Timing Adapters ---
static void uv_timer_cb2(uv_timer_t *handle) {
  dtls_timer_ctx_t *ctx = (dtls_timer_ctx_t *)handle->data;
  ctx->status = 2;

   std::cout << "uv_timer_stop" << std::endl << std::flush;
}

static void dtls_set_timer(void *ctx, uint32_t int_ms, uint32_t fin_ms) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)ctx;
  t_ctx->intermediate_ms = int_ms;
  t_ctx->final_ms = fin_ms;

  if (fin_ms == 0) {
    uv_timer_stop(&t_ctx->uv_timer);
    std::cout << "uv_timer_stop" << std::endl << std::flush;
    t_ctx->status = -1;
    return;
  }
  t_ctx->start_time = uv_now(t_ctx->uv_timer.loop);
  t_ctx->status = 0;
  std::cout << "dtls_set_timer" << std::endl << std::flush;
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

// --- Asynchronous Network I/O ---
static int bio_send(void *ctx, const unsigned char *buf, size_t len) {
  app_context_t *app = (app_context_t *)ctx;
  tx_packet_t *packet = (tx_packet_t *)malloc(sizeof(tx_packet_t));
  if (!packet)
    return MBEDTLS_ERR_SSL_ALLOC_FAILED;

  memcpy(packet->buffer, buf, len);
  uv_buf_t uv_buf = uv_buf_init((char *)packet->buffer, (unsigned int)len);

  int ret = uv_udp_send(&packet->send_req, &app->udp_socket, &uv_buf, 1,
                        (const struct sockaddr *)&app->server_addr,
                        [](uv_udp_send_t *req, int status) { free(req); });
  if (ret < 0) {
    free(packet);
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }
  return (int)len;
}

static int bio_recv(void *ctx, unsigned char *buf, size_t len) {
  app_context_t *app = (app_context_t *)ctx;
  if (app->rx_cache_len == 0)
    return MBEDTLS_ERR_SSL_WANT_READ;

  size_t copy_len = (len < app->rx_cache_len) ? len : app->rx_cache_len;
  memcpy(buf, app->rx_cache, copy_len);

  if (copy_len < app->rx_cache_len) {
    memmove(app->rx_cache, app->rx_cache + copy_len,
            app->rx_cache_len - copy_len);
    app->rx_cache_len -= copy_len;
  } else {
    app->rx_cache_len = 0;
  }
  return (int)copy_len;
}

static void process_dtls_state(app_context_t *app) {
  int ret;
  unsigned char msg_buf[1024];

  // Mbed TLS 3.x fix: Track handshakes using API states rather than structural
  // probes
  if (!app->handshake_complete) {
    ret = mbedtls_ssl_handshake(&app->ssl);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
      return;
    if (ret != 0) {
      printf("Handshake Fatal Error (-0x%x)\n", -ret);
      uv_stop(app->loop);
      return;
    }
    app->handshake_complete = 1;
    printf("Secure Connection Established!\n");

    const char *req = "Ping secure packet";
    mbedtls_ssl_write(&app->ssl, (const unsigned char *)req, strlen(req));
    return;
  }

  do {
    ret = mbedtls_ssl_read(&app->ssl, msg_buf, sizeof(msg_buf) - 1);
    if (ret > 0) {
      msg_buf[ret] = '\0';
      printf("Server Echo Received: %s\n", msg_buf);
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
  app_context_t *app = (app_context_t *)h->data;
  if (nread <= 0)
    return;

  if (app->rx_cache_len + nread <= sizeof(app->rx_cache)) {
    memcpy(app->rx_cache + app->rx_cache_len, buf->base, nread);
    app->rx_cache_len += nread;
    process_dtls_state(app);
  }
}

int main() {
  app_context_t app;
  memset(&app, 0, sizeof(app_context_t));


    ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("CRITICAL: PSA Crypto initialization failed! (0x%x)\n", ret);
    return -1;
  }

  app.loop = uv_default_loop();
  app.udp_socket.data = &app;
  app.timer_ctx.uv_timer.data = &app.timer_ctx;

  uv_udp_init(app.loop, &app.udp_socket);
  uv_timer_init(app.loop, &app.timer_ctx.uv_timer);
  uv_ip4_addr(SERVER_IP, SERVER_PORT, &app.server_addr);

  mbedtls_ssl_init(&app.ssl);
  mbedtls_ssl_config_init(&app.conf);
  mbedtls_ctr_drbg_init(&app.ctr_drbg);
  mbedtls_entropy_init(&app.entropy);

  mbedtls_ctr_drbg_seed(&app.ctr_drbg, mbedtls_entropy_func, &app.entropy,
                        (const unsigned char *)"3x_cli", 6);
  mbedtls_ssl_config_defaults(&app.conf, MBEDTLS_SSL_IS_CLIENT,
                              MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                              MBEDTLS_SSL_PRESET_DEFAULT);
  mbedtls_ssl_conf_rng(&app.conf, mbedtls_ctr_drbg_random, &app.ctr_drbg);
  mbedtls_ssl_conf_authmode(&app.conf, MBEDTLS_SSL_VERIFY_NONE);

  mbedtls_ssl_setup(&app.ssl, &app.conf);

  mbedtls_ssl_conf_max_version(&app.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);
  mbedtls_ssl_conf_min_version(&app.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);

  mbedtls_ssl_set_timer_cb(&app.ssl, &app.timer_ctx, dtls_set_timer,
                           dtls_get_timer);
  mbedtls_ssl_set_bio(&app.ssl, &app, bio_send, bio_recv, NULL);

  uv_udp_recv_start(&app.udp_socket, on_alloc, on_read);
  process_dtls_state(&app);

  uv_run(app.loop, UV_RUN_DEFAULT);

  mbedtls_ssl_free(&app.ssl);
  mbedtls_ssl_config_free(&app.conf);
  mbedtls_ctr_drbg_free(&app.ctr_drbg);
  mbedtls_entropy_free(&app.entropy);
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
#include "psa/crypto.h" // CRITICAL FIX 1: Explicit header for 3.x PSA Crypto System

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4433
#define RX_BUFFER_SIZE 2048

typedef struct {
  uv_udp_send_t send_req;
  unsigned char buffer[RX_BUFFER_SIZE];
} tx_packet_t;

typedef struct {
  uv_timer_t uv_timer;
  uint32_t intermediate_ms;
  uint32_t final_ms;
  uint64_t start_time;
  int status;
  void *app_context;
} dtls_timer_ctx_t;

typedef struct {
  uv_loop_t *loop;
  uv_udp_t udp_socket;
  struct sockaddr_in server_addr;

  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;

  dtls_timer_ctx_t timer_ctx;

  unsigned char rx_cache[RX_BUFFER_SIZE];
  size_t rx_cache_len;
  int handshake_complete;
} app_context_t;

static void process_dtls_state(app_context_t *app);

static void uv_timer_cb2(uv_timer_t *handle) {
  dtls_timer_ctx_t *t_ctx = (dtls_timer_ctx_t *)handle->data;
  t_ctx->status = 2;
  app_context_t *app = (app_context_t *)t_ctx->app_context;
  printf("[Timer] Retransmission timer triggered! Driving state...\n");
  process_dtls_state(app);
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
  app_context_t *app = (app_context_t *)ctx;
  tx_packet_t *packet = (tx_packet_t *)malloc(sizeof(tx_packet_t));
  if (!packet)
    return MBEDTLS_ERR_SSL_ALLOC_FAILED;

  memcpy(packet->buffer, buf, len);
  uv_buf_t uv_buf = uv_buf_init((char *)packet->buffer, (unsigned int)len);

  int ret = uv_udp_send(&packet->send_req, &app->udp_socket, &uv_buf, 1,
                        (const struct sockaddr *)&app->server_addr,
                        [](uv_udp_send_t *req, int status) { free(req); });
  if (ret < 0) {
    free(packet);
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }
  return (int)len;
}

static int bio_recv(void *ctx, unsigned char *buf, size_t len) {
  app_context_t *app = (app_context_t *)ctx;
  if (app->rx_cache_len == 0)
    return MBEDTLS_ERR_SSL_WANT_READ;

  size_t copy_len = (len < app->rx_cache_len) ? len : app->rx_cache_len;
  memcpy(buf, app->rx_cache, copy_len);
  if (copy_len < app->rx_cache_len) {
    memmove(app->rx_cache, app->rx_cache + copy_len,
            app->rx_cache_len - copy_len);
    app->rx_cache_len -= copy_len;
  } else {
    app->rx_cache_len = 0;
  }
  return (int)copy_len;
}

static void process_dtls_state(app_context_t *app) {
  int ret;
  unsigned char msg_buf[RX_BUFFER_SIZE];

  if (!app->handshake_complete) {
    ret = mbedtls_ssl_handshake(&app->ssl);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
      return;
    }
    if (ret != 0) {
      char error_buf[100];
      mbedtls_strerror(ret, error_buf, 100);
      printf("Client Handshake Failed: %s (-0x%x)\n", error_buf, -ret);
      uv_stop(app->loop);
      return;
    }
    app->handshake_complete = 1;
    printf("SUCCESS: Client Handshake Secured Successfully!\n");

    const char *req = "Ping secure packet via 3.x integration";
    mbedtls_ssl_write(&app->ssl, (const unsigned char *)req, strlen(req));
    return;
  }

  do {
    ret = mbedtls_ssl_read(&app->ssl, msg_buf, sizeof(msg_buf) - 1);
    if (ret > 0) {
      msg_buf[ret] = '\0';
      printf("Received Decrypted Server Echo Response: %s\n", msg_buf);
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
  app_context_t *app = (app_context_t *)h->data;
  if (nread <= 0)
    return;

  if (app->rx_cache_len + nread <= sizeof(app->rx_cache)) {
    memcpy(app->rx_cache + app->rx_cache_len, buf->base, nread);
    app->rx_cache_len += nread;
    process_dtls_state(app);
  }
}

int main() {
  int ret;
  app_context_t app;
  memset(&app, 0, sizeof(app_context_t));

  // CRITICAL FIX 1: Initialize PSA Crypto subsystem required by 3.x internals
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("CRITICAL: PSA Crypto initialization failed! (0x%x)\n", ret);
    return -1;
  }

  app.loop = uv_default_loop();
  app.udp_socket.data = &app;
  app.timer_ctx.uv_timer.data = &app.timer_ctx;
  app.timer_ctx.app_context = &app;

  uv_udp_init(app.loop, &app.udp_socket);
  uv_timer_init(app.loop, &app.timer_ctx.uv_timer);
  uv_ip4_addr(SERVER_IP, SERVER_PORT, &app.server_addr);

  mbedtls_ssl_init(&app.ssl);
  mbedtls_ssl_config_init(&app.conf);
  mbedtls_ctr_drbg_init(&app.ctr_drbg);
  mbedtls_entropy_init(&app.entropy);

  ret = mbedtls_ctr_drbg_seed(&app.ctr_drbg, mbedtls_entropy_func, &app.entropy,
                              (const unsigned char *)"3x_cli", 6);
  if (ret != 0)
    return ret;

  ret = mbedtls_ssl_config_defaults(&app.conf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0)
    return ret;

  mbedtls_ssl_conf_rng(&app.conf, mbedtls_ctr_drbg_random, &app.ctr_drbg);
  mbedtls_ssl_conf_authmode(&app.conf, MBEDTLS_SSL_VERIFY_NONE);

  // CRITICAL FIX 2: Enforce DTLS 1.2 bounds to eliminate unconfigured 3.x
  // fallback crashes
  mbedtls_ssl_conf_max_version(&app.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);
  mbedtls_ssl_conf_min_version(&app.conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);

  mbedtls_ssl_setup(&app.ssl, &app.conf);
  mbedtls_ssl_set_timer_cb(&app.ssl, &app.timer_ctx, dtls_set_timer,
                           dtls_get_timer);
  mbedtls_ssl_set_bio(&app.ssl, &app, bio_send, bio_recv, NULL);

  uv_udp_recv_start(&app.udp_socket, on_alloc, on_read);

  printf("Starting secure client sequence tracking...\n");
  process_dtls_state(&app);

  uv_run(app.loop, UV_RUN_DEFAULT);

  mbedtls_ssl_free(&app.ssl);
  mbedtls_ssl_config_free(&app.conf);
  mbedtls_ctr_drbg_free(&app.ctr_drbg);
  mbedtls_entropy_free(&app.entropy);
  return 0;
}
#endif