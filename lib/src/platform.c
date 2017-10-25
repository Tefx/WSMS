#include "platform.h"
#include <stdlib.h>
#include <string.h>

void task_prepare(task_t *task, problem_t *problem, int task_id, int type_id) {
    task_t *item = task_item(task);
    item->latest_available_node = NULL;
    item->length = problem_task_runtime(problem, task_id, type_id);
    item->demands = problem_task_demands(problem, task_id);
}

void machine_init_external_pool(machine_t *machine, mempool_t *pool) {
    bin_init(machine_bin(machine), RES_DIM, pool);
    machine_item(machine)->start_time = 0;
    machine_item(machine)->length = 0;
    machine_item(machine)->latest_available_node = NULL;
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

static inline int _earliest_start_time(task_info_t *task, int *finish_times) {
    register int est = 0;
    register int ft;
    int num_prevs = task->num_prevs;
    int *prevs = task->prevs;
    while (num_prevs) {
        ft = finish_times[prevs[--num_prevs]];
        iMAX(est, ft);
    }
    return est;
}

void platform_simulate(problem_t *problem, int *placements, int *vm_types,
                       int num_vms, int *order, bool always_forward) {
    int num_tasks = problem->num_tasks;
    int *finish_times = (int *)malloc(sizeof(int) * num_tasks);
    machine_t **vms = (machine_t **)malloc(sizeof(machine_t) * num_vms);

    mempool_t *pool = bin_prepare_pool(RES_DIM, num_tasks);

    for (int i = 0; i < num_vms; ++i) {
        machine_init_external_pool(vms[i], pool);
    }

    task_t task;
    machine_t *vm;
    int task_id, vm_id, type_id;
    vlen_t *capacities;
    int est;

    for (int i = 0; i < num_vms; ++i) {
        task_id = order[i];
        vm_id = placements[task_id];
        type_id = vm_types[vm_id];
        vm = vms[vm_id];

        task_prepare(&task, problem, task_id, type_id);
        est =
            _earliest_start_time(problem_task(problem, task_id), finish_times);
        capacities = problem_type_capacities(problem, type_id);
        task_set_start_time(
            &task,
            always_forward
                ? machine_earliest_position_forward(vm, &task, est, capacities)
                : machine_earliest_position(vm, &task, est, capacities));
        finish_times[task_id] = machine_place_task(vm, &task);
    }

    /*for (int i = 0; i < num_vms; ++i) {*/
        /*machine_item(vms[i]).demands = problem->types[vm_types[i]].demands);*/
        /*machine_item(vms+i).runtime(machine_runtime(vms+i)));*/
        /*machine_item(vms+i).start_time = machine_start_time(vms+i);*/
    /*}*/
}
