
#ifndef RTC_IMPL_CERTIFICATE_H
#define RTC_IMPL_CERTIFICATE_H

#include "description.hpp" // for CertificateFingerprint
#include "common.hpp"
#include "configuration.hpp" // for CertificateType
#include "init.hpp"
#include "tls.hpp"

#include <future>
#include <tuple>

namespace rtc::impl {

class Certificate {
public:
	static Certificate* FromString(string crt_pem, string key_pem);
	static Certificate* FromFile(const string &crt_pem_file, const string &key_pem_file,
	                            const string &pass = "");
	static Certificate* Generate(CertificateType type, const string &commonName);


#if USE_MBEDTLS
	Certificate(mbedtls_x509_crt *crt, mbedtls_pk_context *pk);
	std::tuple< mbedtls_x509_crt*, mbedtls_pk_context*  > credentials() const;
        
        ~Certificate()
        {
           mbedtls::pk_free(mPk);
           mbedtls::crt_free(mCrt); 
        }
                
#else // OPENSSL
	Certificate(X509 *x509, EVP_PKEY *pkey, std::vector<X509*> chain = {});
	std::tuple<X509 *, EVP_PKEY *> credentials() const;
	std::vector<X509 *> chain() const;
        
        ~Certificate()
        {
            X509_free(mX509);
            for (X509* tmp : mChain) {
                 X509_free(tmp);
            }
            mChain.clear();
            EVP_PKEY_free(mPKey);
        }
#endif

	CertificateFingerprint fingerprint() const;

private:
	//const init_token mInitToken = Init::Instance().token();


#if USE_MBEDTLS
	mbedtls_x509_crt *mCrt;
	mbedtls_pk_context *mPk;
#else
	X509 *mX509{nullptr};
	EVP_PKEY *mPKey{nullptr};
	std::vector<X509*> mChain;
#endif

	const string mFingerprint;
};


#if USE_MBEDTLS
string make_fingerprint(mbedtls_x509_crt *crt, CertificateFingerprint::Algorithm fingerprintAlgorithm);
#else
string make_fingerprint(X509 *x509, CertificateFingerprint::Algorithm fingerprintAlgorithm);
#endif

//using certificate_ptr = shared_ptr<Certificate>;
//using future_certificate_ptr = std::shared_future<certificate_ptr>;

Certificate* make_certificate(CertificateType type = CertificateType::Default);

} // namespace rtc::impl

#endif
