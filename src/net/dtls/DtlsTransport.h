#ifndef RTC_DTLS_TRANSPORT_HPP
#define RTC_DTLS_TRANSPORT_HPP

#include "description.hpp"

// #include "SrtpSession.h"
#include "base/Timer.h"

#if USE_MBEDTLS
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509_crt.h"

#include "net/bio.h"

#else
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#endif

#include <map>
#include <string>
#include <vector>

#include <cassert>
#define assertm(exp, msg) assert(((void)msg, exp))
#define MS_ABORT(...) std::abort();

using namespace base;
namespace rtc {

class DtlsTransport : public Timer::Listener {
public:
  enum class DtlsState { NEW = 1, CONNECTING, CONNECTED, FAILED, CLOSED };

public:
  enum class Role { NONE = 0, AUTO = 1, CLIENT, SERVER };

public:
  enum class Profile {
    NONE = 0,
    AES_CM_128_HMAC_SHA1_80 = 1,
    AES_CM_128_HMAC_SHA1_32,
    AEAD_AES_256_GCM,
    AEAD_AES_128_GCM
  };

private:
  struct SrtpProfileMapEntry {
    Profile profile;
    const char *name;
  };

public:
  class Listener {
  public:
    // DTLS is in the process of negotiating a secure connection. Incoming
    // media can flow through.
    // NOTE: The caller MUST NOT call any method during this callback.
    virtual void
    OnDtlsTransportConnecting(const rtc::DtlsTransport *dtlsTransport) = 0;
    // DTLS has completed negotiation of a secure connection (including
    // DTLS-SRTP and remote fingerprint verification). Outgoing media can now
    // flow through. NOTE: The caller MUST NOT call any method during this
    // callback.
    virtual void
    OnDtlsTransportConnected(const rtc::DtlsTransport *dtlsTransport) = 0;
    // The DTLS connection has been closed as the result of an error (such as a
    // DTLS alert or a failure to validate the remote fingerprint).
    virtual void
    OnDtlsTransportFailed(const rtc::DtlsTransport *dtlsTransport) = 0;
    // The DTLS connection has been closed due to receipt of a close_notify
    // alert.
    virtual void
    OnDtlsTransportClosed(const rtc::DtlsTransport *dtlsTransport) = 0;
    // Need to send DTLS data to the peer.
    virtual void
    OnDtlsTransportSendData(const rtc::DtlsTransport *dtlsTransport,
                            const uint8_t *data, size_t len) = 0;
    // DTLS application data received.
    virtual void OnDtlsTransportApplicationDataReceived(
        const rtc::DtlsTransport *dtlsTransport, const uint8_t *data,
        size_t len) = 0;
  };

public:
  static void ClassInit();
  static void ClassDestroy();
  static Role StringToRole(const std::string &role);
  static bool IsDtls(const uint8_t *data, size_t len);

private:
  static void ReadCertificateAndPrivateKeyFromFiles();
  static void CreateSslCtx();
  static void GenerateFingerprints();
  static void onDtlError();

private:
#if USE_MBEDTLS

  BIO *app_bio_; // Our BIO, All IO should be through this
  BIO *ssl_bio_; // the ssl BIO used only by openSSL

  mbedtls_entropy_context mEntropy;
  mbedtls_ctr_drbg_context mDrbg;
  mbedtls_ssl_config mConf;

  std::mutex mSslMutex;

  // uint32_t mFinMs = 0, mIntMs = 0;
  std::chrono::time_point<std::chrono::steady_clock> mTimerSetAt;

  char mMasterSecret[48];
  char mRandBytes[64];
  mbedtls_tls_prf_types mTlsProfile = MBEDTLS_SSL_TLS_PRF_NONE;

  static int CertificateCallback(void *ctx, mbedtls_x509_crt *crt, int depth,
                                 uint32_t *flags);
  // static int WriteCallback(void *ctx, const unsigned char *buf, size_t len);
  // static int ReadCallback(void *ctx, unsigned char *buf, size_t len);
  static void ExportKeysCallback(void *ctx, mbedtls_ssl_key_export_type type,
                                 const unsigned char *secret, size_t secret_len,
                                 const unsigned char client_random[32],
                                 const unsigned char server_random[32],
                                 mbedtls_tls_prf_types tls_prf_type);
  static void SetTimerCallback(void *ctx, uint32_t int_ms, uint32_t fin_ms);
  static int GetTimerCallback(void *ctx);

  int swrap_error_handler(const int code);
  void stay_uptodate();

public:
  bool handshake();
  // int timer_int_passed;
  // int timer_fin_passed;

  uint32_t intermediate_ms{0};
  uint32_t final_ms{0};
  uint64_t start_time{0};
  int status{0};

  mbedtls_ssl_context mSsl;

 // uv_timer_t timer1;

  void shutdown();

#else
  static X509 *certificate;
  static EVP_PKEY *privateKey;
  static SSL_CTX *sslCtx;
#endif

  static uint8_t sslReadBuffer[];
  static std::map<std::string, Role> string2Role;
  static std::vector<SrtpProfileMapEntry> srtpProfiles;

public:
  // static int TransportExIndex;

  explicit DtlsTransport(Listener *listener);
  ~DtlsTransport() override;

public:
  void Dump() const;
  void Run(Role localRole);
  bool SetRemoteFingerprint(CertificateFingerprint fingerprint);
  void ProcessDtlsData(const uint8_t *data, size_t len);
  DtlsState GetState() const;
  Role GetLocalRole() const;
  void SendApplicationData(const uint8_t *data, size_t len);

private:
  bool IsRunning() const;
  void Reset();
  bool CheckStatus(int returnCode);
  void SendPendingOutgoingDtlsData();
  bool SetTimeout();
  bool ProcessHandshake();
  bool CheckRemoteFingerprint();

  /* Callbacks fired by OpenSSL events. */
public:
  void OnSslInfo(int where, int ret);

  /* Pure virtual methods inherited from Timer::Listener. */
public:
  void OnTimer(Timer *timer) override;

private:
  // Passed by argument.
  Listener *listener{nullptr};
  // Allocated by this.

#if USE_MBEDTLS

#else

  SSL *ssl{nullptr};
  BIO *sslBioFromNetwork{nullptr}; // The BIO from which ssl reads.
  BIO *sslBioToNetwork{nullptr};   // The BIO in which ssl writes.

#endif


  // Others.
  DtlsState state{DtlsState::NEW};
  Role localRole{Role::NONE};
  // Fingerprint remoteFingerprint;
  bool handshakeDone{false};
  bool handshakeDoneNow{false};
  std::string remoteCert;

public:
  Timer *timer{nullptr};
  CertificateFingerprint remoteFingerprint;

  bool checkFingerprint(const std::string &fingerprint);
};

/* Inline static methods. */

inline DtlsTransport::Role
DtlsTransport::StringToRole(const std::string &role) {
  auto it = DtlsTransport::string2Role.find(role);

  if (it != DtlsTransport::string2Role.end())
    return it->second;
  else
    return DtlsTransport::Role::NONE;
}

inline bool DtlsTransport::IsDtls(const uint8_t *data, size_t len) {

  return (
      // Minimum DTLS record length is 13 bytes.
      (len >= 13) &&
      // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
      (data[0] > 19 && data[0] < 64));
}

/* Inline instance methods. */

inline DtlsTransport::DtlsState DtlsTransport::GetState() const {
  return this->state;
}

inline DtlsTransport::Role DtlsTransport::GetLocalRole() const {
  return this->localRole;
}

inline bool DtlsTransport::IsRunning() const {
  switch (this->state) {
  case DtlsState::NEW:
    return false;
  case DtlsState::CONNECTING:
  case DtlsState::CONNECTED:
    return true;
  case DtlsState::FAILED:
  case DtlsState::CLOSED:
    return false;
  }

  // Make GCC 4.9 happy.
  return false;
}
} // namespace rtc

#endif
