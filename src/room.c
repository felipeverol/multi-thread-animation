#include "room.h"

#include <errno.h>
#include <string.h>
#include <time.h>

#include "util.h"

#define WAIT_SLICE_MS 150

void room_init(Room *r, int id, int capacity, int party_threshold) {
    r->id = id;
    r->capacity = capacity;
    r->party_threshold = party_threshold;
    r->occupant_count = 0;
    memset(r->occupant_ids, 0, sizeof(r->occupant_ids));
    r->being_inspected = false;
    r->eviction_in_progress = false;

    pthread_mutex_init(&r->lock, NULL);

    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&r->cond_state, &cattr);
    pthread_condattr_destroy(&cattr);

    sem_init(&r->capacity_sem, 0, (unsigned)capacity);
}

void room_destroy(Room *r) {
    pthread_mutex_destroy(&r->lock);
    pthread_cond_destroy(&r->cond_state);
    sem_destroy(&r->capacity_sem);
}

static void monotonic_deadline(struct timespec *ts, int ms) {
    clock_gettime(CLOCK_MONOTONIC, ts);
    add_ms(ts, ms);
}

bool room_enter(Room *r, int resident_id, const atomic_bool *running, EventLog *log) {
    for (;;) {
        if (!atomic_load(running)) return false;

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        add_ms(&ts, WAIT_SLICE_MS);
        int rc = sem_timedwait(&r->capacity_sem, &ts);
        if (rc == 0) break;
        if (errno == ETIMEDOUT || errno == EINTR) continue;
        return false;
    }

    pthread_mutex_lock(&r->lock);
    while (r->being_inspected && atomic_load(running)) {
        struct timespec ts;
        monotonic_deadline(&ts, WAIT_SLICE_MS);
        pthread_cond_timedwait(&r->cond_state, &r->lock, &ts);
    }
    if (!atomic_load(running)) {
        pthread_mutex_unlock(&r->lock);
        sem_post(&r->capacity_sem);
        return false;
    }

    r->occupant_ids[r->occupant_count++] = resident_id;
    pthread_cond_broadcast(&r->cond_state);
    int occ = r->occupant_count, cap = r->capacity;
    pthread_mutex_unlock(&r->lock);

    log_event(log, "Morador %d entrou no quarto %d (%d/%d)", resident_id, r->id, occ, cap);
    return true;
}

void room_leave(Room *r, int resident_id, EventLog *log) {
    pthread_mutex_lock(&r->lock);
    for (int i = 0; i < r->occupant_count; i++) {
        if (r->occupant_ids[i] == resident_id) {
            r->occupant_ids[i] = r->occupant_ids[--r->occupant_count];
            break;
        }
    }
    pthread_cond_broadcast(&r->cond_state);
    int occ = r->occupant_count, cap = r->capacity;
    pthread_mutex_unlock(&r->lock);

    sem_post(&r->capacity_sem);
    log_event(log, "Morador %d saiu do quarto %d (%d/%d)", resident_id, r->id, occ, cap);
}

bool room_wait_party_end(Room *r, int party_ms, const atomic_bool *running) {
    struct timespec end_ts;
    monotonic_deadline(&end_ts, party_ms);

    pthread_mutex_lock(&r->lock);
    while (!r->eviction_in_progress && atomic_load(running)) {
        int rc = pthread_cond_timedwait(&r->cond_state, &r->lock, &end_ts);
        if (rc == ETIMEDOUT) break;
    }
    bool evicted = r->eviction_in_progress;
    pthread_mutex_unlock(&r->lock);
    return evicted;
}

bool room_inspect(Room *r, EventLog *log, const atomic_bool *running) {
    bool flagra = false;

    pthread_mutex_lock(&r->lock);
    r->being_inspected = true;

    if (r->occupant_count >= r->party_threshold) {
        flagra = true;
        log_event(log, "ZELADOR flagrou festa no quarto %d (%d ocupantes, limite %d)",
                   r->id, r->occupant_count, r->party_threshold);
        r->eviction_in_progress = true;
        pthread_cond_broadcast(&r->cond_state);

        while (r->occupant_count > 0 && atomic_load(running)) {
            struct timespec ts;
            monotonic_deadline(&ts, WAIT_SLICE_MS);
            pthread_cond_timedwait(&r->cond_state, &r->lock, &ts);
        }
        r->eviction_in_progress = false;
        log_event(log, "Quarto %d esvaziado, despejo concluido", r->id);
    }

    r->being_inspected = false;
    pthread_cond_broadcast(&r->cond_state);
    pthread_mutex_unlock(&r->lock);

    return flagra;
}

void room_snapshot(Room *r, RoomSnapshot *out) {
    pthread_mutex_lock(&r->lock);
    out->id = r->id;
    out->capacity = r->capacity;
    out->party_threshold = r->party_threshold;
    out->occupant_count = r->occupant_count;
    memcpy(out->occupant_ids, r->occupant_ids, sizeof(int) * (size_t)r->occupant_count);
    out->being_inspected = r->being_inspected;
    out->eviction_in_progress = r->eviction_in_progress;
    pthread_mutex_unlock(&r->lock);
}
