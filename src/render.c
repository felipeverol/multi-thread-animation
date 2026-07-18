#include "render.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "eventlog.h"
#include "room.h"
#include "util.h"

#define DISPLAY_LOG_LINES 8

static size_t append(char *buf, size_t cap, size_t off, const char *fmt, ...) {
    if (off >= cap) return off;
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf + off, cap - off, fmt, args);
    va_end(args);
    if (n < 0) return off;
    size_t new_off = off + (size_t)n;
    return new_off < cap ? new_off : cap;
}

void *render_thread(void *arg) {
    RenderArgs *ra = (RenderArgs *)arg;
    House *h = ra->house;

    size_t cap = 1024 + (size_t)h->num_rooms * 256 + DISPLAY_LOG_LINES * (LOG_MSG_LEN + 4);
    char *buf = malloc(cap);
    RoomSnapshot *snaps = malloc(sizeof(RoomSnapshot) * (size_t)h->num_rooms);
    char log_lines[DISPLAY_LOG_LINES][LOG_MSG_LEN];

    while (atomic_load(&h->running)) {
        for (int i = 0; i < h->num_rooms; i++) room_snapshot(&h->rooms[i], &snaps[i]);
        int n_log = log_get_recent(&h->log, log_lines, DISPLAY_LOG_LINES);
        int zelador_idx = atomic_load(&h->zelador_room_idx);
        int flagras = atomic_load(&h->flagra_count);
        long elapsed = now_ms_since(&h->start_time) / 1000;

        size_t off = 0;
        off = append(buf, cap, off, "\033[H\033[2J");
        off = append(buf, cap, off,
                     "=== FESTA NA REPUBLICA ===  t=%02ld:%02ld  flagras: %d  (Ctrl+C para encerrar)\n\n",
                     elapsed / 60, elapsed % 60, flagras);

        for (int i = 0; i < h->num_rooms; i++) {
            RoomSnapshot *s = &snaps[i];
            const char *status = s->eviction_in_progress ? "EVAC" : (s->being_inspected ? "INSP" : "    ");
            off = append(buf, cap, off, " Quarto %2d [%s] %d/%d  ocupantes:",
                         s->id, status, s->occupant_count, s->capacity);
            if (s->occupant_count == 0) {
                off = append(buf, cap, off, " -");
            } else {
                for (int j = 0; j < s->occupant_count; j++) {
                    off = append(buf, cap, off, " %d", s->occupant_ids[j]);
                }
            }
            if (zelador_idx == i) off = append(buf, cap, off, "   <- zelador aqui");
            off = append(buf, cap, off, "\n");
        }

        off = append(buf, cap, off, "\n Ultimos eventos:\n");
        if (n_log == 0) {
            off = append(buf, cap, off, "   (nenhum ainda)\n");
        } else {
            for (int i = 0; i < n_log; i++) {
                off = append(buf, cap, off, "   %s\n", log_lines[i]);
            }
        }

        ssize_t written = write(STDOUT_FILENO, buf, off);
        (void)written;

        struct timespec req = {
            .tv_sec = h->cfg.render_interval_ms / 1000,
            .tv_nsec = (long)(h->cfg.render_interval_ms % 1000) * 1000000L,
        };
        nanosleep(&req, NULL);
    }

    free(buf);
    free(snaps);
    return NULL;
}
