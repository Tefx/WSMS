#ifndef WSMS_PLATFORM_H
#define WSMS_PLATFORM_H

#include "bin.h"
#include "common.h"

typedef struct machine_t {
    bin_t *bin;
    bin_node_t *finger;
    bin_item_t item_in_platform;
} machine_t;

typedef struct platform_t {
    bin_t *bin;
    bin_node_t *finger;
} platform_t;

void machine_init(machine_t *machine);
void machine_free(machine_t *machine);

#define machine_open_time(machine) bin_open_time((machine)->bin)
#define machine_close_time(machine) bin_close_time((machine)->bin)
#define machine_runtime(machine) bin_span((machine)->bin)

int machine_alloc_earliest(machine_t *machine, int est, int rt,
                           resources_t *demands, resources_t *capacities);
int machine_earliest_slot(machine_t *machine, int est, int rt,
                          resources_t *demands, resources_t *capacities);
void machine_place_task(machine_t *machine, int st, int rt,
                        resources_t *demands);

#define machine_shift(machine, delta) bin_shift((machine)->bin, delta)

void platform_init(platform_t *platform);
void platform_free(platform_t *platform);

int platform_earliest_slot(platform_t *platform, int est, int rt,
                           int total_limit);

void platform_alloc_machine(platform_t *platform, machine_t *machine, int st,
                            int rt);
void platform_free_machine(platform_t *platform, machine_t *machine);

void platform_extend_machine(platform_t *platform, machine_t *machine, int st,
                             int ft);
void platform_shift_machine(platform_t *platform, machine_t *machine,
                            int delta);
void platform_extendable_interval(platform_t *platform, machine_t *machine,
                                  int *start, int *finish, int total_limit);

int machine_pnumber(machine_t *machines, int num_machines);

#endif  // ifndef WSMS_PLATFORM_H
