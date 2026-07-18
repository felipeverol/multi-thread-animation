#ifndef RENDER_H
#define RENDER_H

#include "house.h"

typedef struct {
    House *house;
} RenderArgs;

void *render_thread(void *arg);

#endif
