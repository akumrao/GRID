#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
/*
 
HMAC_MD5("key", "The quick brown fox jumps over the lazy dog")    = 80070713463e7749b90c2dc24911e275

HMAC_SHA1("key", "The quick brown fox jumps over the lazy dog")   = de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9    //len 20

HMAC_SHA256("key", "The quick brown fox jumps over the lazy dog") = f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8
 
 */

int main() {
  printf("\n\ntest_hmac_sha1\n\n");

  std::string data = "The quick brown fox jumps over the lazy dog";

  std::string key = "key";
  unsigned char result[20];  // fixed lenght 20
  unsigned int len;
 // HMAC_CTX ctx;

  //HMAC_CTX_init(&ctx);

  HMAC_CTX *ctx = HMAC_CTX_new();
  
  if (!HMAC_Init_ex(ctx, (const unsigned char*)key.c_str(), key.size(), EVP_sha1(), NULL)) {
    printf("Error: cannot init the HMAC.\n");
  }

  HMAC_Update(ctx, (const unsigned char*)data.c_str(), data.size());
  HMAC_Final(ctx, result, &len);

  HMAC_CTX_free(ctx);

  printf("Hash: len= %d\n", len);
  for(unsigned int i = 0; i < len; ++i) {
    printf("%02X ", result[i]);
  }
  printf("\n\n");
}
