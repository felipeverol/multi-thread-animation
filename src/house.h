#ifndef HOUSE_H
#define HOUSE_H

#include <stdatomic.h>
#include <time.h>

#include "config.h"
#include "eventlog.h"
#include "room.h"

typedef struct {
    Room *rooms;
    int num_rooms;
    EventLog log;
    Config cfg;
    atomic_bool running;
    struct timespec start_time;
    atomic_int flagra_count;
    atomic_int zelador_room_idx;
} House;

void house_init(House *h, const Config *cfg);
void house_destroy(House *h);

void house_request_shutdown(House *h);

#endif
