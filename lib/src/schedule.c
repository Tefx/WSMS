#include "schedule.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static inline int _earliest_start_time(task_t* task, int* finish_times) {
    int est = 0;
    int ft;
    for (int i = 0; i < task->num_prevs; ++i) {
        ft = finish_times[task->prevs[i]];
        iMAX(est, ft);
    }
    return est;
}

void schedule_init(schedule_t* schedule, int* placements, int* vm_types,
                   int num_tasks, int num_vms) {
    schedule->num_vms = num_vms;
    schedule->start_times = (int*)malloc(sizeof(int) * num_tasks);
    schedule->placements = (int*)malloc(sizeof(int) * num_tasks);
    schedule->vm_types = (int*)malloc(sizeof(int) * num_vms);
    memcpy(schedule->placements, placements, sizeof(int) * num_tasks);
    memcpy(schedule->vm_types, vm_types, sizeof(int) * num_vms);
}

void schedule_free(schedule_t* schedule) {
    free(schedule->start_times);
    free(schedule->placements);
    free(schedule->vm_types);
}

void schedule_complete_1(problem_t* problem, schedule_t* schedule,
                         int* order, machine_t* vms) {
    machine_t* _vms =
        vms ? vms : (machine_t*)malloc(sizeof(machine_t) * schedule->num_vms);
    int* finish_times = (int*)malloc(sizeof(int) * problem->num_tasks);
    int task_id, vm_id, type_id;
    int est, st, rt;
    machine_t* vm;

    for (int i = 0; i < schedule->num_vms; ++i) machine_init(_vms + i);

    for (int i = 0; i < problem->num_tasks; ++i) {
        task_id = order[i];
        vm_id = schedule->placements[task_id];
        type_id = schedule->vm_types[vm_id];
        est = _earliest_start_time(problem->tasks + task_id, finish_times);
        rt = problem_task_runtime(problem, task_id, type_id);
        st = machine_alloc_earliest(_vms + vm_id, est, rt,
                                    &problem_task_demands(problem, task_id),
                                    &problem_type_capacities(problem, type_id));
        schedule->start_times[task_id] = st;
        finish_times[task_id] = st + rt;
    }

    int t0 = INT_MAX, t1 = 0;
    int ot, ct;
    double cost = 0;

    for (int i = 0; i < schedule->num_vms; ++i) {
        vm = _vms + i;
        ot = machine_open_time(vm);
        ct = machine_close_time(vm);
        t0 = MIN(t0, ot);
        t1 = MAX(t1, ct);
        cost += problem_charge(problem, i, ct - ot);
    }

    schedule->objectives = (objectives_t){(t1 - t0), cost};

    free(finish_times);
    if (!vms) {
        for (int i = 0; i < schedule->num_vms; ++i) machine_free(_vms + i);
        free(_vms);
    }
}
