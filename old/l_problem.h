//
// Created by zhaomengz on 1/5/17.
//

#ifndef WMSM_PROBLEM_H
#define WMSM_PROBLEM_H

#include <float.h>
#include <sys/param.h>
#include "machine.h"

typedef struct task_t {
    resources_t res_demands;
    int ref_runtime;
    int num_prevs;
    int num_nexts;
    int* prev_ids;
    int* next_ids;
} task_t;

typedef struct vm_type_t {
    resources_t capacities;
    double price;
    double ref_speed;
    int limit;
} vm_type_t;

typedef struct problem_t {
    task_t* tasks;
    vm_type_t* vm_types;
    int num_tasks;
    int num_types;
    int total_limit;
    int charge_unit;
    int* rt_matrix;
} problem_t;

void problem_init(problem_t* problem, int num_tasks, int num_types,
                  int total_limit, int charge_unit);
void problem_add_task(problem_t* problem, int task_id, resources_t res_demands,
                      int ref_runtime, int* prev_ids, int num_prevs,
                      int* next_ids, int num_nexts);
void problem_add_type(problem_t* problem, int vt_id, resources_t capacities,
                      double ref_speed, double price, int limit);
void problem_free(problem_t* problem);

resources_t task_demands(problem_t* problem, int task_id);
resources_t type_capacities(problem_t* problem, int type_id);
double type_cost(problem_t* problem, int type_id, int runtime);
#define runtime_on(problem, task_id, type_id) \
    ((problem)->rt_matrix[(task_id) * ((problem)->num_types) + (type_id)])
double vm_cost(problem_t* problem, machine_t* vm);

typedef struct object_t {
    int makespan;
    double cost;
} object_t;

typedef struct schedule_t {
    int num_vms;
    int* start_times;
    int* placements;
    int* vm_types;
    object_t object;
} schedule_t;

void schedule_init(schedule_t* schedule, int* placements, int* vm_types,
                   int num_tasks, int num_vms);

void schedule_free(schedule_t* schedule);

void schedule_complete_1(problem_t* problem, schedule_t* schedule,
                         int* scheduling_order, machine_t* vms);

#endif  // WMSM_PROBLEM_H
