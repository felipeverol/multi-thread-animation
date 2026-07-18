#ifndef EVENTLOG_H
#define EVENTLOG_H

#include <pthread.h>
#include <time.h>

#define LOG_CAPACITY 64
#define LOG_MSG_LEN 128

typedef struct {
    char messages[LOG_CAPACITY][LOG_MSG_LEN];
    int next_slot;
    int count;
    pthread_mutex_t lock;
    struct timespec start_time;
} EventLog;

void eventlog_init(EventLog *log, const struct timespec *start_time);
void eventlog_destroy(EventLog *log);

void log_event(EventLog *log, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

int log_get_recent(EventLog *log, char out[][LOG_MSG_LEN], int max_lines);

#endif
