
#ifndef JUICE_TIMESTAMP_H
#define JUICE_TIMESTAMP_H

#include <stdint.h>
#include <stdlib.h>

typedef int64_t timestamp_t;
typedef timestamp_t timediff_t;

timestamp_t current_timestamp();

#endif
