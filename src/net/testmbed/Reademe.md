



python2 test-server.py




https://gist.github.com/Barakat/675c041fd94435b270a25b5881987a30#file-testing-server-py


Certificate Authority

Generate unencrypted 2048-bits RSA private key for the certificate authority (CA)
$ openssl genrsa -out ca-prk.pem 2048
Generate certificate signing request (CSR) for the CA
$ openssl req -new -sha256 -key ca-prk.pem -out ca-csr.pem -subj "/C=SA/ST=Riyadh/L=Riyadh/O=Barakat CA"
Self-sign the CSR and to generate a certificate for the CA
$ openssl x509 -req -signkey ca-prk.pem -in ca-csr.pem -out ca-cer.pem -days 3650
Add the CA certificate to the client trust chain. Now every certificate signed by this certificate is trusted by the client
Server
Generate unencrypted 2048-bits RSA private key for the server (CA)
$ openssl genrsa -out server-prk.pem 2048
Generate CSR for the server
$ openssl req -new -sha256 -key server-prk.pem -out server-csr.pem -subj "/C=SA/ST=Riyadh/L=Riyadh/O=Barakat Server/CN=127.0.0.1"
Hand over the CSR to the CA for signing
Certificate Authority
View the server CSR and verify its content:
$ openssl req -in server-csr.pem -noout -text
Sign the server CSR
$ openssl x509 -req -sha256 -in server-csr.pem -CA ca-cer.pem -CAkey ca-prk.pem -CAcreateserial -out server-cer.pem -days 365
Hand over the signed server certificate to the requester
Server
Check if the certificate is signed properly
$ openssl x509 -in server-cer.pem -noout -text
Now the server certificate is ready for use

