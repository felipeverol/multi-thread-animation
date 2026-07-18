#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdatomic.h>
#include <time.h>

void add_ms(struct timespec *ts, int ms);

int rand_range_r(unsigned *seed, int min, int max);

void sleep_ms_interruptible(int ms, const atomic_bool *running);

long now_ms_since(const struct timespec *start);

#endif
