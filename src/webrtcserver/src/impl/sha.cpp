

#include "sha.hpp"

#if RTC_ENABLE_WEBSOCKET

#if USE_GNUTLS

#include <nettle/sha1.h>

#elif USE_MBEDTLS

#include <mbedtls/sha1.h>

#else

#ifndef OPENSSL_API_COMPAT
#define OPENSSL_API_COMPAT 0x10100000L
#endif

#include <openssl/sha.h>

#endif

namespace rtc::impl {

namespace {

binary Sha1(const byte *data, size_t size) {
#if USE_GNUTLS

	binary output(SHA1_DIGEST_SIZE);
	struct sha1_ctx ctx;
	sha1_init(&ctx);
	sha1_update(&ctx, size, reinterpret_cast<const uint8_t *>(data));
	sha1_digest(&ctx, SHA1_DIGEST_SIZE, reinterpret_cast<uint8_t *>(output.data()));
	return output;

#elif USE_MBEDTLS

	binary output(20);
	mbedtls_sha1(reinterpret_cast<const unsigned char *>(data), size,
	             reinterpret_cast<unsigned char *>(output.data()));
	return output;

#else

	binary output(SHA_DIGEST_LENGTH);
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, data, size);
	SHA1_Final(reinterpret_cast<unsigned char *>(output.data()), &ctx);
	return output;

#endif
}

} // namespace

binary Sha1(const binary &input) { return Sha1(input.data(), input.size()); }

binary Sha1(const string &input) {
	return Sha1(reinterpret_cast<const byte *>(input.data()), input.size());
}

} // namespace rtc::impl

#endif
