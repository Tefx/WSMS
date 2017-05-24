#include "schedule.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static inline int _earliest_start_time(task_info_t* task, int* finish_times) {
    register int est = 0;
    register int ft;
    int num_prevs = task->num_prevs;
    int* prevs = task->prevs;
    while (num_prevs) {
        ft = finish_times[prevs[--num_prevs]];
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

void schedule_autofill(schedule_t* schedule, problem_t* problem, int* order,
                       machine_t* vms, const bool forward) {
    int num_tasks = schedule->num_tasks;
    int num_vms = schedule->num_vms;
    int* placements = schedule->placements;
    int* vm_types = schedule->vm_types;
    int* start_times = schedule->start_times;
    int* finish_times = schedule->finish_times;
    task_info_t* tasks = problem->tasks;
    type_info_t* types = problem->types;
    int** rt_matrix = problem->rt_matrix;
    type_info_t* type_capacities =
        (type_info_t*)((char*)problem->types + offsetof(type_info_t, capacities));
    task_info_t* task_demands=
        (task_info_t*)((char*)problem->tasks + offsetof(task_info_t, demands));
    machine_t* _vms =
        vms ? vms : (machine_t*)malloc(sizeof(machine_t) * num_vms);
    task_t task;
    machine_t* vm;
    register int task_id, vm_id, type_id;
    register volume_t capacities, demands;
    register int est, rt;

    for (int i = 0; i < num_vms; ++i) {
        vm = _vms + i;
        machine_init(vm, num_tasks);
        machine_set(vm, types[vm_types[i]].demands);
    }

    for (int i = 0; i < num_tasks; ++i) {
        task_id = order[i];
        vm_id = placements[task_id];
        type_id = vm_types[vm_id];
        vm = _vms + vm_id;
        /*capacities = types[type_id].capacities;*/
        /*demands = tasks[task_id].demands;*/
        capacities = (volume_t)(type_capacities + type_id);
        demands = (volume_t)(task_demands + task_id);
        rt = rt_matrix[type_id][task_id];
        task_set(&task, rt, demands);
        est = _earliest_start_time(tasks + task_id, finish_times);
        start_times[task_id] =
            forward
                ? machine_earliest_position_forward(vm, &task, est, capacities)
                : machine_earliest_position(vm, &task, est, capacities);
        finish_times[task_id] = machine_place_task(vm, &task);
    }


    int t0 = INT_MAX, t1 = 0;
    int ot, ct;
    double cost = 0;

    for (int i = 0; i < num_vms; ++i) {
        vm = _vms + i;
        ot = machine_open_time(vm);
        ct = machine_close_time(vm);
        t0 = MIN(t0, ot);
        t1 = MAX(t1, ct);
        cost += problem_charge(problem, vm_types[i], ct - ot);
    }

    schedule->objectives = (objectives_t){(t1 - t0), cost};

    if (!vms) {
        for (int i = 0; i < num_vms; ++i) machine_destory(_vms + i);
        free(_vms);
    }
}
