#ifndef WMSM_PROBLEM_H
#define WMSM_PROBLEM_H

#include "common.h"

typedef struct task_t {
    resources_t demands;
    int num_prevs;
    int num_nexts;
    int* prevs;
    int* nexts;
} task_t;

typedef struct mtype_t {
    resources_t capacities;
    double price;
    int limit;
} mtype_t;

typedef struct problem_t {
    task_t* tasks;
    mtype_t* types;
    int num_tasks;
    int num_types;
    int total_limit;
    int charge_unit;
    int* rt_matrix;
} problem_t;

void problem_init(problem_t* problem, int num_tasks, int num_types,
                  int total_limit, int charge_unit);
void problem_free(problem_t* problem);

void problem_add_task(problem_t* problem, int task_id, resources_t res_demands,
                      int* prev_ids, int num_prevs, int* next_ids,
                      int num_nexts);
void problem_add_type(problem_t* problem, int vt_id, resources_t capacities,
                      double price, int limit);
void problem_set_runtime(problem_t* problem, int task_id, int type_id,
                         int runtime);

#define problem_task_is_entry(problem, task_id) \
    ((problem)->tasks[task_id].num_prevs == 0)

#define problem_task_is_exit(problem, task_id) \
    ((problem)->tasks[task_id].num_nexts == 0)

#define problem_task(problem, task_id) ((problem)->tasks + (task_id))
#define problem_task_demands(problem, task_id) \
    ((problem)->tasks[task_id].demands)
#define problem_task_runtime(problem, task_id, type_id) \
    ((problem)->rt_matrix[(task_id) * (problem)->num_types + (type_id)])

#define problem_type(problem, type_id) ((problem)->types + type_id)
#define problem_type_capacities(problem, type_id) \
    ((problem)->types[type_id].capacities)
#define problem_type_price(problem, type_id) ((problem)->types[type_id].price)

#define problem_charge(problem, type_id, runtime) \
    (problem_type_price(problem, type_id) *       \
     _div_and_ceil(runtime, (problem)->charge_unit))

int problem_task_average_runtime(problem_t* problem, int task_id);

#endif  // ifndef WMSM_PROBLEM_H
