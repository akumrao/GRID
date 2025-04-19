

#ifndef JUICE_CONST_TIME_H
#define JUICE_CONST_TIME_H

#include <stdint.h>
#include <stdlib.h>

int const_time_memcmp(const void *a, const void *b, size_t len);
int const_time_strcmp(const void *a, const void *b);

#endif
