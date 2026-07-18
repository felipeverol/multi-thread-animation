#ifndef ROOM_H
#define ROOM_H

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "config.h"
#include "eventlog.h"

typedef struct {
    int id;
    int capacity;
    int party_threshold;

    int occupant_count;
    int occupant_ids[MAX_CAPACITY];
    bool being_inspected;
    bool eviction_in_progress;

    pthread_mutex_t lock;
    pthread_cond_t cond_state;
    sem_t capacity_sem;
} Room;

typedef struct {
    int id, capacity, party_threshold, occupant_count;
    int occupant_ids[MAX_CAPACITY];
    bool being_inspected, eviction_in_progress;
} RoomSnapshot;

void room_init(Room *r, int id, int capacity, int party_threshold);
void room_destroy(Room *r);

bool room_enter(Room *r, int resident_id, const atomic_bool *running, EventLog *log);

void room_leave(Room *r, int resident_id, EventLog *log);

bool room_wait_party_end(Room *r, int party_ms, const atomic_bool *running);

bool room_inspect(Room *r, EventLog *log, const atomic_bool *running);

void room_snapshot(Room *r, RoomSnapshot *out);

#endif
