
#ifndef JUICE_CRC32_H
#define JUICE_CRC32_H

#include "juice.h"

#include <stdint.h>
#include <stdlib.h>

JUICE_EXPORT uint32_t juice_crc32(const void *data, size_t size);

#define CRC32(data, size) juice_crc32(data, size)

#endif // JUICE_CRC32_H
