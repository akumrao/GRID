

#ifndef JUICE_RANDOM_H
#define JUICE_RANDOM_H

#include <stdint.h>
#include <stdlib.h>

void juice_random(void *buf, size_t size);
void juice_random_str64(char *buf, size_t size);

uint32_t juice_rand32(void);
uint64_t juice_rand64(void);

#endif // JUICE_RANDOM_H
