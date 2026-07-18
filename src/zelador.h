#ifndef ZELADOR_H
#define ZELADOR_H

#include "house.h"

typedef struct {
    House *house;
} ZeladorArgs;

void *zelador_thread(void *arg);

#endif
