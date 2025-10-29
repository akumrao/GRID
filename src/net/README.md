Mbed does not have bio files, like openssl, I have to write my own. 


Mbed TLS sample programs
========================

This subdirectory mostly contains sample programs that illustrate specific features of the library, as well as a few test and support programs.

## Symmetric cryptography (AES) examples

* [`aes/crypt_and_hash.c`](aes/crypt_and_hash.c): file encryption and authentication, demonstrating the generic cipher interface and the generic hash interface.

## Hash (digest) examples

* [`hash/generic_sum.c`](hash/generic_sum.c): file hash calculator and verifier, demonstrating the message digest (`md`) interface.

* [`hash/hello.c`](hash/hello.c): hello-world program for MD5.

## Public-key cryptography examples
## X.509 certificate examples

* [`x509/cert_app.c`](x509/cert_app.c): connects to a TLS server and verifies its certificate chain.

* [`x509/cert_req.c`](x509/cert_req.c): generates a certificate signing request (CSR) for a private key.

* [`x509/cert_write.c`](x509/cert_write.c): signs a certificate signing request, or self-signs a certificate.

* [`x509/crl_app.c`](x509/crl_app.c): loads and dumps a certificate revocation list (CRL).

* [`x509/req_app.c`](x509/req_app.c): loads and dumps a certificate signing request (CSR).

