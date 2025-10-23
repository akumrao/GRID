#include <cstdio>
#include <cstring>

#include <mbedtls/x509.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl.h>
#include <mbedtls/net.h>


int main()
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static const auto host = "127.0.0.1";
  static const auto port = "1234";

  int status;

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // TRUST CHAIN CONFIGURATION

  mbedtls_x509_crt x509_certificate;
  mbedtls_x509_crt_init(&x509_certificate);

  if ((status = mbedtls_x509_crt_parse_file(&x509_certificate, "./ca-cer.pem")) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_x509_crt_parse_file failed to parse CA certificate (-0x%X)\n", -status);
    goto quite_x509_certificate;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // ENTROPY/RANDOMNESS SOURCE AND PSEUDORANDOM NUMBER GENERATOR (PRNG) CONFIGURATION

  mbedtls_entropy_context entropy_context;
  mbedtls_entropy_init(&entropy_context);

  mbedtls_ctr_drbg_context drbg_context;
  mbedtls_ctr_drbg_init(&drbg_context);

  if ((status = mbedtls_ctr_drbg_seed(&drbg_context, mbedtls_entropy_func, &entropy_context, nullptr, 0)) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_ctr_drbg_seed (-0x%X)\n", -status);
    goto quite_entropy;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // TLS CONFIGURATION

  mbedtls_ssl_config ssl_config;
  mbedtls_ssl_config_init(&ssl_config);

  if ((status = mbedtls_ssl_config_defaults(&ssl_config,
                                            MBEDTLS_SSL_IS_CLIENT,
                                            MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_ssl_config_defaults failed to load default SSL config (-0x%X)\n", -status);
    goto quite_ssl_config;
  }

  // Only use TLS 1.2
  mbedtls_ssl_conf_max_version(&ssl_config, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
  mbedtls_ssl_conf_min_version(&ssl_config, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
  // Only use this cipher suite
  // static const int tls_cipher_suites[2] = {MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256, 0};
  // mbedtls_ssl_conf_ciphersuites(&ssl_config, tls_cipher_suites);

  // By limiting ourselves to TLS v1.2 and the previous cipher suites, we can compile mbedTLS without the unused ciphers
  // and reduce its size

  // Load CA certificate
  mbedtls_ssl_conf_ca_chain(&ssl_config, &x509_certificate, nullptr);
  // Strictly ensure that certificates are signed by the CA
  mbedtls_ssl_conf_authmode(&ssl_config, MBEDTLS_SSL_VERIFY_OPTIONAL);
  mbedtls_ssl_conf_rng(&ssl_config, mbedtls_ctr_drbg_random, &drbg_context);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // TLS CONTEXT

  mbedtls_ssl_context ssl_context;
  mbedtls_ssl_init(&ssl_context);

  if ((status = mbedtls_ssl_setup(&ssl_context, &ssl_config)) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_ssl_setup failed to setup SSL context (-0x%X)\n", -status);
    goto quite_ssl_context;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // ESTABLISH SECURE TLS CONNECTION

  mbedtls_net_context net_context;
  mbedtls_net_init(&net_context);

  mbedtls_ssl_set_bio(&ssl_context, &net_context, mbedtls_net_send, mbedtls_net_recv, nullptr);

  if ((status = mbedtls_net_connect(&net_context, host, port, MBEDTLS_NET_PROTO_TCP)) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_net_connect (-0x%X)\n", -status);
    goto quite_net_context;
  }

  // Verify that that certificate actually belongs to the host
  if ((status = mbedtls_ssl_set_hostname(&ssl_context, host)) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_ssl_set_hostname (-0x%X)\n", -status);
    goto quite_close_context;
  }

  while ((status = mbedtls_ssl_handshake(&ssl_context)) != 0)
  {
    if (status != MBEDTLS_ERR_SSL_WANT_READ && status != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      std::fprintf(stderr, "[!] mbedtls_ssl_handshake (-0x%X)\n", -status);
      goto quite_close_context;
    }
  }

  if ((status = mbedtls_ssl_get_verify_result(&ssl_context)) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_ssl_get_verify_result (-0x%X)\n", -status);
    goto quite_close_context;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // EXCHANGE SOME MESSAGES

  static const unsigned char write_buffer[] = "Hello world2!\n";
  static const auto write_buffer_length = sizeof(write_buffer) - 1; // last byte is the null terminator

  do
  {
    status = mbedtls_ssl_write(&ssl_context, write_buffer + status, write_buffer_length - status);

    if (status == 0)
    {
      break;
    }

    if (status < 0)
    {
      switch (status)
      {
      case MBEDTLS_ERR_SSL_WANT_READ:
      case MBEDTLS_ERR_SSL_WANT_WRITE:
      case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
      case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
        {
          continue;
        }
      default:
        {
          std::fprintf(stderr, "[!] mbedtls_ssl_write (-0x%X)\n", -status);
          goto quite_close_context;
        }
      }
    }

    std::printf("[*] %d bytes sent to the server\n", status);
  }
  while (true);

  do
  {
    unsigned char read_buffer[64];
    static const auto read_buffer_length = sizeof(read_buffer);

    std::memset(read_buffer, 0, sizeof(read_buffer));

    status = mbedtls_ssl_read(&ssl_context, read_buffer, read_buffer_length);

    if (status == 0)
    {
      break;
    }

    if (status < 0)
    {
      switch (status)
      {
      case MBEDTLS_ERR_SSL_WANT_READ:
      case MBEDTLS_ERR_SSL_WANT_WRITE:
      case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
      case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
        {
          continue;
        }
      default:
        {
          std::fprintf(stderr, "[!] mbedtls_ssl_read (-0x%X)\n", -status);
          goto quite_close_context;
        }
      }
    }

    auto line_terminator_received = false;

    for (auto i = 0; i < status; ++i)
    {
      if (read_buffer[i] == '\n')
      {
        line_terminator_received = true;
        break;
      }
    }

    if (line_terminator_received)
    {
      if (status > 1)
      {
        std::fprintf(stderr, "[*] Received chunk '%.*s'\n", status - 1, reinterpret_cast<char *>(read_buffer));
      }
      break;
    }

    std::fprintf(stderr, "[*] Received chunk '%.*s'\n", status, reinterpret_cast<char *>(read_buffer));
  }
  while (true);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // CLEANUP AND EXIT

quite_close_context:

  // In our protocol, the connection will be closed by the server first
#if 0
  if ((status = mbedtls_ssl_close_notify(&ssl_context)) != 0)
  {
    std::fprintf(stderr, "[!] mbedtls_ssl_close_notify (-0x%X)\n", -status);
  }
#endif

quite_net_context:
  mbedtls_net_free(&net_context);

quite_ssl_context:
  mbedtls_ssl_free(&ssl_context);

quite_ssl_config:
  mbedtls_ssl_config_free(&ssl_config);

quite_entropy:
  mbedtls_ctr_drbg_free(&drbg_context);
  mbedtls_entropy_free(&entropy_context);

quite_x509_certificate:
  mbedtls_x509_crt_free(&x509_certificate);
}
