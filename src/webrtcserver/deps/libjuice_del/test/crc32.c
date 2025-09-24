

#include "crc32.h"

#include <stdint.h>
#include <string.h>

int test_crc32(void) {
	const char *str = "The quick brown fox jumps over the lazy dog";
	uint32_t expected = 0x414fa339;

	if (CRC32(str, strlen(str)) != expected)
		return -1;

	return 0;
}
