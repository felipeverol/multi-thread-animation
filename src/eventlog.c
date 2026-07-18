#include "eventlog.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

void eventlog_init(EventLog *log, const struct timespec *start_time) {
    memset(log->messages, 0, sizeof(log->messages));
    log->next_slot = 0;
    log->count = 0;
    log->start_time = *start_time;
    pthread_mutex_init(&log->lock, NULL);
}

void eventlog_destroy(EventLog *log) {
    pthread_mutex_destroy(&log->lock);
}

void log_event(EventLog *log, const char *fmt, ...) {
    long elapsed_ms = now_ms_since(&log->start_time);
    long total_s = elapsed_ms / 1000;
    int mm = (int)(total_s / 60) % 100;
    int ss = (int)(total_s % 60);

    pthread_mutex_lock(&log->lock);
    char *slot = log->messages[log->next_slot];

    int prefix_len = snprintf(slot, LOG_MSG_LEN, "[%02d:%02d] ", mm, ss);
    if (prefix_len < 0) prefix_len = 0;
    if (prefix_len > LOG_MSG_LEN) prefix_len = LOG_MSG_LEN;

    va_list args;
    va_start(args, fmt);
    vsnprintf(slot + prefix_len, (size_t)(LOG_MSG_LEN - prefix_len), fmt, args);
    va_end(args);

    log->next_slot = (log->next_slot + 1) % LOG_CAPACITY;
    if (log->count < LOG_CAPACITY) log->count++;
    pthread_mutex_unlock(&log->lock);
}

int log_get_recent(EventLog *log, char out[][LOG_MSG_LEN], int max_lines) {
    pthread_mutex_lock(&log->lock);
    int n = log->count < max_lines ? log->count : max_lines;
    for (int i = 0; i < n; i++) {
        int idx_from_newest = n - 1 - i;
        int slot = (log->next_slot - 1 - idx_from_newest + LOG_CAPACITY) % LOG_CAPACITY;
        memcpy(out[i], log->messages[slot], LOG_MSG_LEN);
    }
    pthread_mutex_unlock(&log->lock);
    return n;
}
