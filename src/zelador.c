#include "zelador.h"

#include "util.h"

void *zelador_thread(void *arg) {
    ZeladorArgs *za = (ZeladorArgs *)arg;
    House *h = za->house;
    int idx = 0;

    while (atomic_load(&h->running)) {
        atomic_store(&h->zelador_room_idx, idx);

        bool flagra = room_inspect(&h->rooms[idx], &h->log, &h->running);
        if (flagra) atomic_fetch_add(&h->flagra_count, 1);

        atomic_store(&h->zelador_room_idx, -1);

        idx = (idx + 1) % h->num_rooms;
        sleep_ms_interruptible(h->cfg.inspection_interval_ms, &h->running);
    }
    return NULL;
}
