#include "platform.h"
#include <stdlib.h>
#include <string.h>

/*#define copy2vol(vol, src, dim) memcpy((vol), (src), sizeof(vlen_t) * (dim))*/

void machine_init(machine_t *machine, int num_tasks) {
    bin_init(machine_bin(machine), RES_DIM, MIN(num_tasks, 4096));
    machine_item(machine)->start_node = NULL;
    machine_item(machine)->finish_node = NULL;
    machine_item(machine)->start_time = 0;
    machine_item(machine)->length = 0;
}

void machine_destory(machine_t *machine) { bin_destory(machine_bin(machine)); }

void machine_set(machine_t *machine, plim_t demands) {
    machine_item(machine)->demands = demands;
}

int machine_earliest_position(machine_t *machine, task_t *task, int est,
                              res_t capacities) {
    return bin_earliest_position_small(machine_bin(machine), task_item(task),
                                       est, capacities);
}

int machine_earliest_position_forward(machine_t *machine, task_t *task, int est,
                                      res_t capacities) {
    return bin_earliest_position_forward_small(
        machine_bin(machine), task_item(task), est, capacities);
}

int machine_place_task(machine_t *machine, task_t *task) {
    return bin_place_item_small(machine_bin(machine), task_item(task));
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

static int _compare_int(const void *a, const void *b) {
    const double *da = (const double *)a;
    const double *db = (const double *)b;

    return (*da > *db) - (*da < *db);
}

int machine_pnumber(machine_t *machines, int num_machines) {
    int *open_times = (int *)malloc(sizeof(int) * num_machines);
    int *close_times = (int *)malloc(sizeof(int) * num_machines);
    for (int i = 0; i < num_machines; ++i) {
        open_times[i] = machine_open_time(machines + i);
        close_times[i] = machine_close_time(machines + i);
    }
    qsort(open_times, num_machines, sizeof(int), _compare_int);
    qsort(close_times, num_machines, sizeof(int), _compare_int);
    int peak_num = 0, cur_num = 0;
    int cur_time = 0;
    int j = 0;
    for (int i = 0; i < num_machines; ++i) {
        cur_time = open_times[i];
        while (close_times[j++] <= cur_time) cur_num--;
        cur_num++;
        peak_num = MAX(peak_num, cur_num);
    }
    free(open_times);
    free(close_times);
    return peak_num;
}
