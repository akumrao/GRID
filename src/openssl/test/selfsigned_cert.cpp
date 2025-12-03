/*
g++ -o selfsigned_cert -g selfsigned_cert.cpp -I./buildx64/include -lssl -lcrypto ./selfsigned_cert 
*/

#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <ctime>
#include <iostream>

// Function to handle OpenSSL errors
void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

EVP_PKEY* generate_key() {
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) handleErrors();

    RSA* rsa = RSA_new();
    if (!rsa) handleErrors();

    BIGNUM* e = BN_new();
    if (!e) handleErrors();

    if (!BN_set_word(e, RSA_F4)) handleErrors(); // RSA_F4 is 65537

    if (!RSA_generate_key_ex(rsa, 2048, e, NULL)) handleErrors(); // 2048-bit key

    if (!EVP_PKEY_assign_RSA(pkey, rsa)) handleErrors(); // Assign RSA key to EVP_PKEY
    BN_free(e); // Free the BIGNUM
    // rsa is now managed by pkey, no need to free it separately
    return pkey;
}

X509* generate_x509(EVP_PKEY* pkey) {
    X509* x509 = X509_new();
    if (!x509) handleErrors();

    // Set certificate version (V3)
    if (!X509_set_version(x509, 2)) handleErrors();

    // Set serial number
    if (!ASN1_INTEGER_set(X509_get_serialNumber(x509), 1)) handleErrors();

    // Set validity period (e.g., 365 days)
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L); // 365 days in seconds

    // Set public key
    if (!X509_set_pubkey(x509, pkey)) handleErrors();

    // Set subject name (and issuer name for self-signed)
    X509_NAME* name = X509_get_subject_name(x509);
    if (!name) handleErrors();

    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)"MyOrganization", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"localhost", -1, -1, 0);

    if (!X509_set_issuer_name(x509, name)) handleErrors(); // Self-signed, so issuer is subject

    // Sign the certificate
    if (!X509_sign(x509, pkey, EVP_sha256())) handleErrors();

    return x509;
}

int main() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    EVP_PKEY* pkey = generate_key();
    X509* x509 = generate_x509(pkey);

    // Write private key to file
    FILE* pkey_file = fopen("private.key", "wb");
    if (!pkey_file) {
        std::cerr << "Error opening private.key for writing." << std::endl;
        handleErrors();
    }
    PEM_write_PKCS8PrivateKey(pkey_file, pkey, NULL, NULL, 0, 0, NULL);
    fclose(pkey_file);

    // Write certificate to file
    FILE* x509_file = fopen("certificate.crt", "wb");
    if (!x509_file) {
        std::cerr << "Error opening certificate.crt for writing." << std::endl;
        handleErrors();
    }
    PEM_write_X509(x509_file, x509);
    fclose(x509_file);

    // Clean up
    X509_free(x509);
    EVP_PKEY_free(pkey);
    EVP_cleanup();
    ERR_free_strings();

    std::cout << "Self-signed certificate and private key generated successfully." << std::endl;

    return 0;
}
