#include "resident.h"

#include "eventlog.h"
#include "util.h"

void *resident_thread(void *arg) {
    ResidentArgs *ra = (ResidentArgs *)arg;
    House *h = ra->house;
    unsigned seed = ra->rng_seed;

    while (atomic_load(&h->running)) {
        int think_ms = rand_range_r(&seed, h->cfg.think_time_min_ms, h->cfg.think_time_max_ms);
        sleep_ms_interruptible(think_ms, &h->running);
        if (!atomic_load(&h->running)) break;

        int room_idx = rand_range_r(&seed, 0, h->num_rooms - 1);
        Room *r = &h->rooms[room_idx];

        if (!room_enter(r, ra->resident_id, &h->running, &h->log)) continue;

        int party_ms = rand_range_r(&seed, h->cfg.party_time_min_ms, h->cfg.party_time_max_ms);
        bool evicted = room_wait_party_end(r, party_ms, &h->running);
        if (evicted) {
            log_event(&h->log, "Morador %d foi despejado do quarto %d", ra->resident_id, r->id);
        }

        room_leave(r, ra->resident_id, &h->log);
    }
    return NULL;
}
