#include "config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void config_set_defaults(Config *cfg) {
    cfg->num_rooms = 5;
    cfg->num_residents = 12;
    cfg->room_capacity = 4;
    cfg->party_threshold = 3;
    cfg->inspection_interval_ms = 1500;
    cfg->think_time_min_ms = 500;
    cfg->think_time_max_ms = 2000;
    cfg->party_time_min_ms = 2000;
    cfg->party_time_max_ms = 6000;
    cfg->duration_s = 60;
    cfg->render_interval_ms = 200;
    cfg->seed = (unsigned)time(NULL);
}

void config_print_help(const char *prog_name) {
    printf(
        "Uso: %s [opcoes]\n"
        "\n"
        "Simulacao \"Festa na Republica\" - moradores, zelador e sincronizacao com\n"
        "mutex + variaveis de condicao + semaforos (pthreads).\n"
        "\n"
        "Opcoes:\n"
        "  -r, --rooms N          numero de quartos (default: 5)\n"
        "  -m, --residents N      numero de moradores (default: 12)\n"
        "  -c, --capacity N       capacidade fisica por quarto (default: 4)\n"
        "  -t, --threshold N      ocupantes minimos para 'festa ativa' (default: 3)\n"
        "  -i, --inspect-ms N     intervalo entre inspecoes do zelador (default: 1500)\n"
        "      --think-min-ms N   tempo minimo em area comum (default: 500)\n"
        "      --think-max-ms N   tempo maximo em area comum (default: 2000)\n"
        "      --party-min-ms N   tempo minimo de festa (default: 2000)\n"
        "      --party-max-ms N   tempo maximo de festa (default: 6000)\n"
        "  -d, --duration-s N     duracao da simulacao; 0 = ate Ctrl+C (default: 60)\n"
        "      --frame-ms N       intervalo de redesenho do visualizador (default: 200)\n"
        "  -s, --seed N           semente aleatoria (default: baseada no relogio)\n"
        "  -h, --help             mostra esta mensagem\n",
        prog_name);
}

int config_parse_args(Config *cfg, int argc, char **argv) {
    enum { OPT_THINK_MIN = 1000, OPT_THINK_MAX, OPT_PARTY_MIN, OPT_PARTY_MAX, OPT_FRAME_MS };

    static struct option long_opts[] = {
        {"rooms", required_argument, 0, 'r'},
        {"residents", required_argument, 0, 'm'},
        {"capacity", required_argument, 0, 'c'},
        {"threshold", required_argument, 0, 't'},
        {"inspect-ms", required_argument, 0, 'i'},
        {"think-min-ms", required_argument, 0, OPT_THINK_MIN},
        {"think-max-ms", required_argument, 0, OPT_THINK_MAX},
        {"party-min-ms", required_argument, 0, OPT_PARTY_MIN},
        {"party-max-ms", required_argument, 0, OPT_PARTY_MAX},
        {"duration-s", required_argument, 0, 'd'},
        {"frame-ms", required_argument, 0, OPT_FRAME_MS},
        {"seed", required_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };

    int opt;
    optind = 1;
    while ((opt = getopt_long(argc, argv, "r:m:c:t:i:d:s:h", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'r': cfg->num_rooms = atoi(optarg); break;
            case 'm': cfg->num_residents = atoi(optarg); break;
            case 'c': cfg->room_capacity = atoi(optarg); break;
            case 't': cfg->party_threshold = atoi(optarg); break;
            case 'i': cfg->inspection_interval_ms = atoi(optarg); break;
            case OPT_THINK_MIN: cfg->think_time_min_ms = atoi(optarg); break;
            case OPT_THINK_MAX: cfg->think_time_max_ms = atoi(optarg); break;
            case OPT_PARTY_MIN: cfg->party_time_min_ms = atoi(optarg); break;
            case OPT_PARTY_MAX: cfg->party_time_max_ms = atoi(optarg); break;
            case 'd': cfg->duration_s = atoi(optarg); break;
            case OPT_FRAME_MS: cfg->render_interval_ms = atoi(optarg); break;
            case 's': cfg->seed = (unsigned)strtoul(optarg, NULL, 10); break;
            case 'h': config_print_help(argv[0]); return -1;
            default: return 1;
        }
    }
    return 0;
}

int config_validate(const Config *cfg) {
    if (cfg->num_rooms <= 0) {
        fprintf(stderr, "erro: --rooms deve ser > 0\n");
        return -1;
    }
    if (cfg->num_residents <= 0) {
        fprintf(stderr, "erro: --residents deve ser > 0\n");
        return -1;
    }
    if (cfg->room_capacity <= 0 || cfg->room_capacity > MAX_CAPACITY) {
        fprintf(stderr, "erro: --capacity deve estar entre 1 e %d\n", MAX_CAPACITY);
        return -1;
    }
    if (cfg->party_threshold <= 0 || cfg->party_threshold > cfg->room_capacity) {
        fprintf(stderr, "erro: --threshold deve estar entre 1 e --capacity (%d)\n", cfg->room_capacity);
        return -1;
    }
    if (cfg->think_time_min_ms < 0 || cfg->think_time_max_ms < cfg->think_time_min_ms) {
        fprintf(stderr, "erro: intervalo think-min-ms/think-max-ms invalido\n");
        return -1;
    }
    if (cfg->party_time_min_ms < 0 || cfg->party_time_max_ms < cfg->party_time_min_ms) {
        fprintf(stderr, "erro: intervalo party-min-ms/party-max-ms invalido\n");
        return -1;
    }
    if (cfg->inspection_interval_ms < 0 || cfg->render_interval_ms <= 0) {
        fprintf(stderr, "erro: inspect-ms/frame-ms invalidos\n");
        return -1;
    }
    if (cfg->duration_s < 0) {
        fprintf(stderr, "erro: --duration-s deve ser >= 0\n");
        return -1;
    }
    return 0;
}
