

#ifndef JUICE_HASH_H
#define JUICE_HASH_H

#include <stdint.h>
#include <stdlib.h>

#define HASH_MD5_SIZE 16
#define HASH_SHA1_SIZE 24
#define HASH_SHA256_SIZE 32

void hash_md5(const void *message, size_t size, void *digest);
void hash_sha1(const void *message, size_t size, void *digest);
void hash_sha256(const void *message, size_t size, void *digest);

#endif
