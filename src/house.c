#include "house.h"

#include <stdlib.h>

void house_init(House *h, const Config *cfg) {
    h->cfg = *cfg;
    h->num_rooms = cfg->num_rooms;
    h->rooms = calloc((size_t)cfg->num_rooms, sizeof(Room));
    atomic_init(&h->running, true);
    atomic_init(&h->flagra_count, 0);
    atomic_init(&h->zelador_room_idx, -1);
    clock_gettime(CLOCK_MONOTONIC, &h->start_time);

    eventlog_init(&h->log, &h->start_time);

    for (int i = 0; i < cfg->num_rooms; i++) {
        room_init(&h->rooms[i], i + 1, cfg->room_capacity, cfg->party_threshold);
    }
}

void house_destroy(House *h) {
    for (int i = 0; i < h->num_rooms; i++) {
        room_destroy(&h->rooms[i]);
    }
    free(h->rooms);
    eventlog_destroy(&h->log);
}

void house_request_shutdown(House *h) {
    atomic_store(&h->running, false);
    for (int i = 0; i < h->num_rooms; i++) {
        Room *r = &h->rooms[i];
        pthread_mutex_lock(&r->lock);
        pthread_cond_broadcast(&r->cond_state);
        pthread_mutex_unlock(&r->lock);
    }
}
