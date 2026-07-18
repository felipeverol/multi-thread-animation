#include "util.h"

#include <stdlib.h>

void add_ms(struct timespec *ts, int ms) {
    ts->tv_sec += ms / 1000;
    ts->tv_nsec += (long)(ms % 1000) * 1000000L;
    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec += 1;
    }
}

int rand_range_r(unsigned *seed, int min, int max) {
    if (max <= min) return min;
    return min + (int)(rand_r(seed) % (unsigned)(max - min + 1));
}

void sleep_ms_interruptible(int ms, const atomic_bool *running) {
    const int slice_ms = 50;
    int remaining = ms;
    while (remaining > 0 && atomic_load(running)) {
        int step = remaining < slice_ms ? remaining : slice_ms;
        struct timespec req = {.tv_sec = step / 1000, .tv_nsec = (long)(step % 1000) * 1000000L};
        nanosleep(&req, NULL);
        remaining -= step;
    }
}

long now_ms_since(const struct timespec *start) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long sec = now.tv_sec - start->tv_sec;
    long nsec = now.tv_nsec - start->tv_nsec;
    return sec * 1000L + nsec / 1000000L;
}
