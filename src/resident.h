#ifndef RESIDENT_H
#define RESIDENT_H

#include "house.h"

typedef struct {
    House *house;
    int resident_id;
    unsigned rng_seed;
} ResidentArgs;

void *resident_thread(void *arg);

#endif
