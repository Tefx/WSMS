//
// Created by zhaomengz on 1/5/17.
//

#include "l_problem.h"
#include <memory.h>
#include <stdlib.h>
#include "common.h"

#define div_and_ceil(x, y) (((x) + (y)-1) / (y))

// Problem related
//
void problem_init(problem_t* problem, int num_tasks, int num_types,
                  int total_limit, int charge_unit) {
    problem->num_tasks = num_tasks;
    problem->tasks = (task_t*)malloc(sizeof(task_t) * num_tasks);
    problem->num_types = num_types;
    problem->vm_types = (vm_type_t*)malloc(sizeof(vm_type_t) * num_types);
    problem->total_limit = total_limit;
    problem->charge_unit = charge_unit;
    problem->rt_matrix = (int*)malloc(sizeof(int) * num_tasks * num_types);
}

void problem_free(problem_t* problem) {
    for (int i = 0; i < problem->num_tasks; ++i) {
        free(problem->tasks[i].prev_ids);
        free(problem->tasks[i].next_ids);
    }
    free(problem->tasks);
    free(problem->vm_types);
    free(problem->rt_matrix);
}

void problem_add_task(problem_t* problem, int task_id, resources_t res_demands,
                      int ref_runtime, int* prev_ids, int num_prevs,
                      int* next_ids, int num_nexts) {
    task_t* task = problem->tasks + task_id;
    task->res_demands = res_demands;
    task->ref_runtime = ref_runtime;
    task->num_prevs = num_prevs;
    task->num_nexts = num_nexts;
    task->prev_ids = (int*)malloc(sizeof(int) * num_prevs);
    memcpy(task->prev_ids, prev_ids, sizeof(int) * num_prevs);
    task->next_ids = (int*)malloc(sizeof(int) * num_nexts);
    memcpy(task->next_ids, next_ids, sizeof(int) * num_nexts);
}

void problem_add_type(problem_t* problem, int vt_id, resources_t capacities,
                      double ref_speed, double price, int limit) {
    vm_type_t* type = problem->vm_types + vt_id;
    type->capacities = capacities;
    type->ref_speed = ref_speed;
    type->price = price;
    type->limit = limit;
    for (int i = 0; i < problem->num_tasks; ++i)
        problem->rt_matrix[i * (problem->num_types) + vt_id] =
            (int)(problem->tasks[i].ref_runtime / ref_speed);
}

resources_t task_demands(problem_t* problem, int task_id) {
    return problem->tasks[task_id].res_demands;
}

resources_t type_capacities(problem_t* problem, int type_id) {
    return problem->vm_types[type_id].capacities;
}

double type_cost(problem_t* problem, int type_id, int runtime) {
    double price = problem->vm_types[type_id].price;
    double units = div_and_ceil(runtime, problem->charge_unit);
    return price * units;
}

double vm_cost(problem_t* problem, machine_t* vm) {
    return type_cost(problem, vm->type_id, machine_runtime(vm));
}

// Schedule related
//
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

static inline int earliest_start_time(task_t* task, int* finish_times) {
    int est = 0;
    int ft;
    for (int i = 0; i < task->num_prevs; ++i) {
        ft = finish_times[task->prev_ids[i]];
        est = est > ft ? est : ft;
    }
    return est;
}

// Filling strat_times, in scheduling order
void schedule_complete_1(problem_t* problem, schedule_t* schedule,
                         int* scheduling_order, machine_t* vms) {
    machine_t* _vms =
        vms ? vms : (machine_t*)malloc(sizeof(machine_t) * schedule->num_vms);
    machine_t* _vms_ap = _vms + schedule->num_vms;
    int* finish_times = (int*)malloc(sizeof(int) * problem->num_tasks);
    int task_id, vm_id, type_id;
    int est, st, rt;

    for (int i = 0; i < schedule->num_vms; ++i)
        machine_init(_vms + i, schedule->vm_types[i]);

    for (int i = 0; i < problem->num_tasks; ++i) {
        task_id = scheduling_order[i];
        vm_id = schedule->placements[task_id];
        type_id = schedule->vm_types[vm_id];
        est = earliest_start_time(problem->tasks + task_id, finish_times);
        rt = runtime_on(problem, task_id, type_id);
        st = machine_alloc_earliest(_vms + vm_id, est, rt,
                                    &(problem->tasks[task_id].res_demands),
                                    &(problem->vm_types[type_id].capacities));
        schedule->start_times[task_id] = st;
        finish_times[task_id] = st + rt;
    }

    int t0 = INT_MAX, t1 = 0;
    int ot, ct;
    double cost = 0;

    for (machine_t* vm = _vms; vm < _vms_ap; ++vm) {
        ot = machine_open_time(vm);
        ct = machine_close_time(vm);
        t0 = MIN(t0, ot);
        t1 = MAX(t1, ct);
        cost += vm_cost(problem, vm);
    }

    schedule->object = (object_t){(t1 - t0), cost};

    free(finish_times);
    if (!vms) {
        for (machine_t* vm = _vms; vm < _vms_ap; ++vm) machine_free(vm);
        free(_vms);
    }
}

static int _compare_int(const void* a, const void* b) {
    const double* da = (const double*)a;
    const double* db = (const double*)b;

    return (*da > *db) - (*da < *db);
}

int num_covms(machine_t* vms, int num_vms) {
    int* open_times = (int*)malloc(sizeof(int) * num_vms);
    int* close_times = (int*)malloc(sizeof(int) * num_vms);
    for (int i = 0; i < num_vms; ++i) {
        open_times[i] = machine_open_time(vms + i);
        close_times[i] = machine_close_time(vms + i);
    }
    qsort(open_times, num_vms, sizeof(int), _compare_int);
    qsort(close_times, num_vms, sizeof(int), _compare_int);
    int peak_num = 0, cur_num = 0;
    int cur_time = 0;
    int j = 0;
    for (int i = 0; i < num_vms; ++i) {
        cur_time = open_times[i];
        while (close_times[j++] <= cur_time) cur_num--;
        cur_num++;
        peak_num = MAX(peak_num, cur_num);
    }
    free(open_times);
    free(close_times);
    return peak_num;
}
