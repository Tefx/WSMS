#include "platform.h"
#include <stdlib.h>
#include <string.h>

#define res2vol(res, vol) memcpy(vol, res, sizeof(vlen_t) * RES_DIM);

void task_init(task_t *task) { task_item(task) = (task_t)item_new(RES_DIM); }

void task_destory(task_t *task) { item_free(task_item(task)); }

void task_set(task_t *task, int length, res_t demands) {
    task_item(task)->start_node = task_item(task)->finish_node = NULL;
    task_item(task)->length = length;
    res2vol(demands, item_demands(task_item(task)));
}

void machine_init(machine_t *machine) {
    *machine = (machine_t)malloc(sizeof(bin_t) + item_real_size(LIM_DIM));
    bin_init(machine_bin(machine), RES_DIM);
    item_t *item = machine_item(machine);
    item->start_node = item->finish_node = NULL;
    item->start_time = item->length = 0;
}

void machine_destory(machine_t *machine) {
    bin_destory(machine_bin(machine));
    free(*machine);
}

void machine_set(machine_t *machine, int demands) {
    item_demands(machine_item(machine))[0] = demands;
}

void platform_init(platform_t *platform) {
    platform_bin(platform) = (bin_t *)malloc(sizeof(bin_t));
    bin_init(platform_bin(platform), LIM_DIM);
}

void platform_destory(platform_t *platform) {
    bin_destory(platform_bin(platform));
    free(platform_bin(platform));
}

int machine_earliest_position(machine_t *machine, task_t *task, int est,
                              res_t capacities) {
    res_t capacities_v;
    res2vol(capacities, capacities_v);
    return bin_earliest_position(machine_bin(machine), task_item(task), est,
                                 capacities_v);
}

int machine_place_task(machine_t *machine, task_t *task) {
    return bin_place_item(machine_bin(machine), task_item(task));
}

void machine_shift_task(machine_t *machine, task_t *task, int delta) {
    bin_shift_item(machine_bin(machine), task_item(task), delta);
}

int machine_extendable_interval_start(machine_t *machine, task_t *task,
                                      res_t capacities) {
    res_t capacities_v;
    res2vol(capacities, capacities_v);
    return bin_extendable_interval_start(machine_bin(machine), task_item(task),
                                         capacities_v);
}

int machine_extendable_interval_finish(machine_t *machine, task_t *task,
                                       res_t capacities) {
    res_t capacities_v;
    res2vol(capacities, capacities_v);
    return bin_extendable_interval_finish(machine_bin(machine), task_item(task),
                                          capacities_v);
}

int platform_earliest_position(platform_t *platform, machine_t *machine,
                               int est, int total_limit) {
    plim_t plim_v;
    plim_v[0] = total_limit;
    machine_item(machine)->length = machine_runtime(machine);
    return bin_earliest_position(platform_bin(platform), machine_item(machine),
                                 est, plim_v);
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
                                       int total_limit) {
    plim_t plim_v;
    plim_v[0] = total_limit;
    return bin_extendable_interval_start(
        platform_bin(platform), machine_item(machine), plim_v);
}

int platform_extendable_interval_finish(platform_t *platform,
                                        machine_t *machine, int total_limit) {
    plim_t plim_v;
    plim_v[0] = total_limit;
    return bin_extendable_interval_finish(
        platform_bin(platform), machine_item(machine), plim_v);
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
