
#ifndef RTC_IMPL_CERTIFICATE_H
#define RTC_IMPL_CERTIFICATE_H

#include "description.h" // for CertificateFingerprint
#include "common.hpp"
#include "configuration.h" // for CertificateType
#include "init.hpp"
#include "tls.hpp"

#include <future>
#include <tuple>

namespace rtc {

class Certificate_good {
public:
	static Certificate_good FromString(string crt_pem, string key_pem);
	static Certificate_good FromFile(const string &crt_pem_file, const string &key_pem_file,
	                            const string &pass = "");
	static Certificate_good Generate(CertificateType type, const string &commonName);

#if USE_GNUTLS
	Certificate_good(gnutls_x509_crt_t crt, gnutls_x509_privkey_t privkey);
	gnutls_certificate_credentials_t credentials() const;
#elif USE_MBEDTLS
	Certificate_good(shared_ptr<mbedtls_x509_crt> crt, shared_ptr<mbedtls_pk_context> pk);
	std::tuple<shared_ptr<mbedtls_x509_crt>, shared_ptr<mbedtls_pk_context>> credentials() const;
#else // OPENSSL
	Certificate_good(shared_ptr<X509> x509, shared_ptr<EVP_PKEY> pkey, std::vector<shared_ptr<X509>> chain = {});
	std::tuple<X509 *, EVP_PKEY *> credentials() const;
	std::vector<X509 *> chain() const;
#endif

	CertificateFingerprint fingerprint() const;

private:
	const init_token mInitToken = Init::Instance().token();

#if USE_GNUTLS
	Certificate_good(shared_ptr<gnutls_certificate_credentials_t> creds);
	const shared_ptr<gnutls_certificate_credentials_t> mCredentials;
#elif USE_MBEDTLS
	const shared_ptr<mbedtls_x509_crt> mCrt;
	const shared_ptr<mbedtls_pk_context> mPk;
#else
	const shared_ptr<X509> mX509;
	const shared_ptr<EVP_PKEY> mPKey;
	const std::vector<shared_ptr<X509>> mChain;
#endif

	const string mFingerprint;
};

#if USE_GNUTLS
string make_fingerprint(gnutls_certificate_credentials_t credentials, CertificateFingerprint::Algorithm fingerprintAlgorithm);
string make_fingerprint(gnutls_x509_crt_t crt, CertificateFingerprint::Algorithm fingerprintAlgorithm);
#elif USE_MBEDTLS
string make_fingerprint(mbedtls_x509_crt *crt, CertificateFingerprint::Algorithm fingerprintAlgorithm);
#else
string make_fingerprint(X509 *x509, CertificateFingerprint::Algorithm fingerprintAlgorithm);
#endif

using certificate_ptr = shared_ptr<Certificate_good>;
using future_certificate_ptr = std::shared_future<certificate_ptr>;

future_certificate_ptr make_certificate(CertificateType type = CertificateType::Default);

} // namespace rtc::impl

#endif
