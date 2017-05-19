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

void schedule_init(schedule_t* schedule, int num_tasks) {
    schedule->num_tasks = num_tasks;
    schedule->placements = (int*)malloc(sizeof(int) * num_tasks);
    schedule->vm_types = NULL;
    schedule->start_times = (int*)malloc(sizeof(int) * num_tasks);
    schedule->finish_times = (int*)malloc(sizeof(int) * num_tasks);
}

void schedule_free(schedule_t* schedule) {
    if (schedule->placements) free(schedule->placements);
    if (schedule->vm_types) free(schedule->vm_types);
    if (schedule->start_times) free(schedule->start_times);
    if (schedule->finish_times) free(schedule->finish_times);
}

void schedule_set_placements(schedule_t* schedule, int* placements) {
    memcpy(schedule->placements, placements, sizeof(int) * schedule->num_tasks);
}

void schedule_set_vm_types(schedule_t* schedule, int* vm_types, int num_vms) {
    schedule->num_vms = num_vms;
    schedule->vm_types = (int*)malloc(sizeof(int) * num_vms);
    memcpy(schedule->vm_types, vm_types, sizeof(int) * num_vms);
}

void schedule_set_start_times(schedule_t* schedule, int* start_times) {
    memcpy(schedule->start_times, start_times,
           sizeof(int) * schedule->num_tasks);
}

void schedule_set_finish_times(schedule_t* schedule, int* finish_times) {
    memcpy(schedule->finish_times, finish_times,
           sizeof(int) * schedule->num_tasks);
}

void schedule_autofill_start_times(schedule_t* schedule, problem_t* problem) {
    for (int i = 0; i < schedule->num_tasks; ++i)
        schedule->start_times[i] =
            schedule->finish_times[i] -
            problem_task_runtime(problem, i, TYPL(schedule, i));
}

void schedule_autofill_finish_times(schedule_t* schedule, problem_t* problem) {
    for (int i = 0; i < schedule->num_tasks; ++i)
        schedule->finish_times[i] =
            schedule->start_times[i] +
            problem_task_runtime(problem, i, TYPL(schedule, i));
}

// the fields of *placements* and *vm_types* must have been set.
void schedule_autofill_1(schedule_t* schedule, problem_t* problem, int* order,
                         machine_t* vms) {
    machine_t* _vms =
        vms ? vms : (machine_t*)malloc(sizeof(machine_t) * schedule->num_vms);
    int task_id, vm_id, type_id;
    int est, st, rt;
    machine_t* vm;

    for (int i = 0; i < schedule->num_vms; ++i) machine_init(_vms + i);

    for (int i = 0; i < schedule->num_tasks; ++i) {
        task_id = order[i];
        vm_id = schedule->placements[task_id];
        type_id = schedule->vm_types[vm_id];
        est = _earliest_start_time(problem->tasks + task_id,
                                   schedule->finish_times);
        rt = problem_task_runtime(problem, task_id, type_id);
        st = machine_alloc_earliest(_vms + vm_id, est, rt,
                                    &problem_task_demands(problem, task_id),
                                    &problem_type_capacities(problem, type_id));
        schedule->start_times[task_id] = st;
        schedule->finish_times[task_id] = st + rt;
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
        cost += problem_charge(problem, TYP(schedule, i), ct - ot);
    }

    schedule->objectives = (objectives_t){(t1 - t0), cost};

    if (!vms) {
        for (int i = 0; i < schedule->num_vms; ++i) machine_free(_vms + i);
        free(_vms);
    }
}
