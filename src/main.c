#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "house.h"
#include "render.h"
#include "resident.h"
#include "util.h"
#include "zelador.h"

static House *g_house = NULL;

static void sigint_handler(int signum) {
    (void)signum;
    if (g_house != NULL) atomic_store(&g_house->running, false);
}

int main(int argc, char **argv) {
    Config cfg;
    config_set_defaults(&cfg);

    int parse_rc = config_parse_args(&cfg, argc, argv);
    if (parse_rc == -1) return 0;
    if (parse_rc != 0) return 1;

    if (config_validate(&cfg) != 0) return 1;

    House house;
    house_init(&house, &cfg);
    g_house = &house;

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    pthread_t *resident_threads = calloc((size_t)cfg.num_residents, sizeof(pthread_t));
    ResidentArgs *resident_args = calloc((size_t)cfg.num_residents, sizeof(ResidentArgs));
    for (int i = 0; i < cfg.num_residents; i++) {
        resident_args[i].house = &house;
        resident_args[i].resident_id = i + 1;
        resident_args[i].rng_seed = cfg.seed + (unsigned)i * 2654435761u + 1u;
        pthread_create(&resident_threads[i], NULL, resident_thread, &resident_args[i]);
    }

    ZeladorArgs zelador_args = {.house = &house};
    pthread_t zelador_tid;
    pthread_create(&zelador_tid, NULL, zelador_thread, &zelador_args);

    RenderArgs render_args = {.house = &house};
    pthread_t render_tid;
    pthread_create(&render_tid, NULL, render_thread, &render_args);

    if (cfg.duration_s > 0) {
        sleep_ms_interruptible(cfg.duration_s * 1000, &house.running);
    } else {
        while (atomic_load(&house.running)) {
            struct timespec req = {.tv_sec = 0, .tv_nsec = 200000000L};
            nanosleep(&req, NULL);
        }
    }

    house_request_shutdown(&house);

    for (int i = 0; i < cfg.num_residents; i++) pthread_join(resident_threads[i], NULL);
    pthread_join(zelador_tid, NULL);
    pthread_join(render_tid, NULL);

    printf("\nSimulacao encerrada. Total de flagrantes: %d\n", atomic_load(&house.flagra_count));

    free(resident_threads);
    free(resident_args);
    house_destroy(&house);
    g_house = NULL;
    return 0;
}
