#include "platform.h"
#include <stdlib.h>
#include <string.h>


void machine_init(machine_t *machine, int num_tasks) {
    bin_init(machine_bin(machine), RES_DIM, MIN(num_tasks, 4096));
    machine_item(machine)->start_node = NULL;
    machine_item(machine)->finish_node = NULL;
    machine_item(machine)->start_time = 0;
    machine_item(machine)->length = 0;
}

void machine_destory(machine_t *machine) { bin_destory(machine_bin(machine)); }

int machine_earliest_position(machine_t *machine, task_t *task, int est,
                              res_t capacities) {
    return bin_earliest_position_res(machine_bin(machine), task_item(task),
                                     est, capacities);
}

int machine_earliest_position_forward(machine_t *machine, task_t *task, int est,
                                      res_t capacities) {
    return bin_earliest_position_forward_res(
            machine_bin(machine), task_item(task), est, capacities);
}

int machine_place_task(machine_t *machine, task_t *task) {
    return bin_place_item_res(machine_bin(machine), task_item(task));
}

void machine_shift_task(machine_t *machine, task_t *task, int delta) {
    bin_shift_item(machine_bin(machine), task_item(task), delta);
}

int machine_extendable_interval_start(machine_t *machine, task_t *task,
                                      res_t capacities) {
    return bin_extendable_interval_start(machine_bin(machine), task_item(task),
                                         capacities);
}

int machine_extendable_interval_finish(machine_t *machine, task_t *task,
                                       res_t capacities) {
    return bin_extendable_interval_finish(machine_bin(machine), task_item(task),
                                          capacities);
}

int platform_earliest_position(platform_t *platform, machine_t *machine,
                               int est, plim_t plim) {
    return bin_earliest_position(platform_bin(platform), machine_item(machine),
                                 est, plim);
}

int platform_place_machine(platform_t *platform, machine_t *machine) {
    return bin_place_item(platform_bin(platform), machine_item(machine));
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
                                       plim_t plim) {
    return bin_extendable_interval_start(platform_bin(platform),
                                         machine_item(machine), plim);
}

int platform_extendable_interval_finish(platform_t *platform,
                                        machine_t *machine, plim_t plim) {
    return bin_extendable_interval_finish(platform_bin(platform),
                                          machine_item(machine), plim);
}

