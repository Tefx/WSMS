#include "platform.h"
#include <stdlib.h>
#include <string.h>

void machine_init_external_pool(machine_t *machine, mempool_t *pool) {
    bin_init(machine_bin(machine), RES_DIM, pool);
    machine_item(machine)->start_node = NULL;
    machine_item(machine)->finish_node = NULL;
    machine_item(machine)->start_time = 0;
    machine_item(machine)->length = 0;
}

void machine_init(machine_t *machine, int num_tasks) {
    mempool_t *pool = bin_prepare_pool(RES_DIM, num_tasks);
    machine_init_external_pool(machine, pool);
}

void machine_destory(machine_t *machine) {
    mp_free_pool(machine_bin(machine)->pool);
}

int machine_earliest_position(machine_t *machine, task_t *task, int est,
                              vlen_t *capacities) {
    return bin_earliest_position_res(machine_bin(machine), task_item(task), est,
                                     capacities);
}

int machine_earliest_position_forward(machine_t *machine, task_t *task, int est,
                                      vlen_t *capacities) {
    return bin_earliest_position_forward_res(machine_bin(machine),
                                             task_item(task), est, capacities);
}

int machine_place_task(machine_t *machine, task_t *task) {
    return bin_place_item_res(machine_bin(machine), task_item(task));
}

void machine_shift_task(machine_t *machine, task_t *task, int delta) {
    bin_shift_item(machine_bin(machine), task_item(task), delta);
}

int machine_extendable_interval_start(machine_t *machine, task_t *task,
                                      vlen_t *capacities) {
    return bin_extendable_interval_start(machine_bin(machine), task_item(task),
                                         capacities);
}

int machine_extendable_interval_finish(machine_t *machine, task_t *task,
                                       vlen_t *capacities) {
    return bin_extendable_interval_finish(machine_bin(machine), task_item(task),
                                          capacities);
}

int platform_earliest_position(platform_t *platform, machine_t *machine,
                               int est, vlen_t *plim) {
    return bin_earliest_position(platform_bin(platform), machine_item(machine),
                                 est, plim);
}

int platform_place_machine(platform_t *platform, machine_t *machine) {
    return bin_place_item(platform_bin(platform), machine_item(machine));
}

void platform_init(platform_t *platform, int limit) {
    mempool_t *pool = bin_prepare_pool(LIM_DIM, limit);
    bin_init(platform_bin(platform), LIM_DIM, pool);
}

void platform_destory(platform_t *platform) {
    mp_free_pool(platform_bin(platform)->pool);
}

void platform_shift_machine(platform_t *platform, machine_t *machine,
                            int delta) {
    bin_shift(machine_bin(machine), delta);
    bin_shift_item(platform_bin(platform), machine_item(machine), delta);
}

void platform_extend_machine(platform_t *platform, machine_t *machine) {
    bin_extend_item(platform_bin(platform), machine_item(machine),
                    machine_open_time(machine), machine_close_time(machine));
}

int platform_extendable_interval_start(platform_t *platform, machine_t *machine,
                                       vlen_t *plim) {
    return bin_extendable_interval_start(platform_bin(platform),
                                         machine_item(machine), plim);
}

int platform_extendable_interval_finish(platform_t *platform,
                                        machine_t *machine, vlen_t *plim) {
    return bin_extendable_interval_finish(platform_bin(platform),
                                          machine_item(machine), plim);
}
