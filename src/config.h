#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CAPACITY 16

typedef struct {
    int num_rooms;
    int num_residents;
    int room_capacity;
    int party_threshold;
    int inspection_interval_ms;
    int think_time_min_ms;
    int think_time_max_ms;
    int party_time_min_ms;
    int party_time_max_ms;
    int duration_s;
    int render_interval_ms;
    unsigned seed;
} Config;

void config_set_defaults(Config *cfg);

int config_parse_args(Config *cfg, int argc, char **argv);

int config_validate(const Config *cfg);

void config_print_help(const char *prog_name);

#endif
