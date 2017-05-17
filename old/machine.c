//
// Created by zhaomengz on 11/5/17.
//

#include "machine.h"
#include <malloc.h>

static inline void resources2volume(const resources_t* resources,
                                    volume_t volume) {
    volume[0] = resources->core;
    volume[1] = resources->memory;
}

void machine_init(machine_t *machine, int type_id) {
    machine->type_id = type_id;
    machine->bin = (bin_t*)malloc(sizeof(bin_t));
    bin_init(machine->bin);
}

void machine_free(machine_t *machine) {
    bin_destroy(machine->bin);
    free(machine->bin);
}

int machine_open_time(machine_t *machine) {
    return bin_open_time(machine->bin);
}

int machine_close_time(machine_t *machine) {
    return bin_close_time(machine->bin);
}

int machine_runtime(machine_t *machine) {
    return bin_is_empty(machine->bin)?0:(bin_open_time(machine->bin)-bin_close_time(machine->bin));
}

int machine_alloc_earliest(machine_t* machine, int est, int rt, resources_t* demands, resources_t* capacities){
    volume_t demands_v, capacities_v;
    resources2volume(demands, demands_v);
    resources2volume(capacities, capacities_v);
    return bin_alloc_earliest(machine->bin, est, rt, demands_v, capacities_v);
}
