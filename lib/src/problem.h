#ifndef WMSM_PROBLEM_H
#define WMSM_PROBLEM_H

#include <limits.h>
#include "common.h"

typedef struct task_info_t {
    vlen_t demands[RES_DIM];
    int num_prevs;
    int num_nexts;
    int* prevs;
    int* nexts;
} task_info_t;

typedef struct type_info_t {
    vlen_t capacities[RES_DIM];
    double price;
    int limit;
    vlen_t demands[LIM_DIM];
} type_info_t;

typedef struct problem_t {
    task_info_t* tasks;
    type_info_t* types;
    int num_tasks;
    int num_types;
    vlen_t limits[LIM_DIM];
    int charge_unit;
    int** rt_matrix;
    char** adj_matrix;
} problem_t;

void problem_init(problem_t* problem, int num_tasks, int num_types,
                  int total_limit, int charge_unit);
void problem_destory(problem_t* problem);

void problem_add_task(problem_t* problem, int task_id, vlen_t* res_demands,
                      int* prev_ids, int num_prevs, int* next_ids,
                      int num_nexts);
void problem_add_type(problem_t* problem, int vt_id, vlen_t* capacities,
                      double price, int limit);
void problem_set_runtime(problem_t* problem, int task_id, int type_id,
                         int runtime);

#define ADJ_INDEX(task_id) ((task_id) / CHAR_BIT)
#define ADJ_MASK(task_id) (0x1 << ((task_id) % CHAR_BIT))

#define problem_task_is_entry(problem, task_id) \
    ((problem)->tasks[task_id].num_prevs == 0)

#define problem_task_is_exit(problem, task_id) \
    ((problem)->tasks[task_id].num_nexts == 0)

#define problem_task(problem, task_id) ((problem)->tasks + (task_id))
#define problem_task_demands(problem, task_id) \
    ((problem)->tasks[task_id].demands)
#define problem_task_runtime(problem, task_id, type_id) \
    ((problem)->rt_matrix[type_id][task_id])

#define problem_task_num_prevs(problem, task_id) \
    ((problem)->tasks[task_id].num_prevs)
#define problem_task_num_nexts(problem, task_id) \
    ((problem)->tasks[task_id].num_nexts)
#define problem_task_prevs(problem, task_id) ((problem)->tasks[task_id].prevs)
#define problem_task_nexts(problem, task_id) ((problem)->tasks[task_id].nexts)

/*inline bool problem_task_is_adjacent(problem_t* problem, int t0, int t1){*/
/*printf("%d %d %d 0x%X %p\n", t0, t1, ADJ_INDEX(t1), ADJ_MASK(t1),
 * problem->adj_matrix);*/
/*bool res = problem->adj_matrix[t0][ADJ_INDEX(t1)] & ADJ_MASK(t1);*/
/*printf(res?"true":"false");*/
/*return res;*/
/*}*/

#define problem_task_is_adjacent(problem, t0, t1) \
    ((problem->adj_matrix[t0][ADJ_INDEX(t1)] & ADJ_MASK(t1)) != 0)

#define problem_type(problem, type_id) ((problem)->types + type_id)
#define problem_type_demands(problem, type_id) \
    ((problem)->types[type_id].demands)
#define problem_type_capacities(problem, type_id) \
    ((problem)->types[type_id].capacities)
#define problem_type_price(problem, type_id) ((problem)->types[type_id].price)

#define problem_charge(problem, type_id, runtime) \
    (problem_type_price(problem, type_id) *       \
     _div_and_ceil(runtime, (problem)->charge_unit))

int problem_task_average_runtime(problem_t* problem, int task_id);

int problem_cheapest_type_for_demands(problem_t* problem, vlen_t* demands);

#endif  // ifndef WMSM_PROBLEM_H
