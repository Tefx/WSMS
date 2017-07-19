#include "problem.h"
#include <float.h>
#include <stdlib.h>
#include <string.h>

void problem_init(problem_t* problem, int num_tasks, int num_types,
                  int total_limit, int charge_unit) {
    problem->num_tasks = num_tasks;

    problem->tasks = (task_info_t*)malloc(sizeof(task_info_t) * num_tasks);
    problem->num_types = num_types;
    problem->types = (type_info_t*)malloc(sizeof(type_info_t) * num_types);
    problem->limits[0] = total_limit;
    problem->charge_unit = charge_unit;

    problem->rt_matrix =
        (int**)malloc(sizeof(int*) * num_types * (num_tasks + 1));
    for (int i = 0; i < num_types; ++i)
        problem->rt_matrix[i] =
            (int*)(problem->rt_matrix) + (i + 1) * num_tasks;

    problem->adj_matrix = (char**)malloc(sizeof(char*) * num_tasks);
    for (int i = 0; i < num_tasks; ++i)
        problem->adj_matrix[i] =
            (char*)calloc(((num_tasks - 1) / CHAR_BIT - 1), 1);
}

void problem_destory(problem_t* problem) {
    for (int i = 0; i < problem->num_tasks; ++i) free(problem->tasks[i].prevs);
    free(problem->tasks);
    free(problem->types);
    free(problem->rt_matrix);
    free(problem->adj_matrix);
}

void problem_add_task(problem_t* problem, int task_id, vlen_t* demands,
                      int* prev_ids, int num_prevs, int* next_ids,
                      int num_nexts) {
    task_info_t* task = problem->tasks + task_id;
    memcpy(task->demands, demands, sizeof(vlen_t) * RES_DIM);
    task->num_prevs = num_prevs;
    task->num_nexts = num_nexts;
    task->prevs = (int*)malloc(sizeof(int) * (num_prevs + num_nexts));
    task->nexts = task->prevs + num_prevs;
    memcpy(task->prevs, prev_ids, sizeof(int) * num_prevs);
    memcpy(task->nexts, next_ids, sizeof(int) * num_nexts);

    for (int i = 0, prev_id; i < num_prevs; ++i) {
        prev_id = prev_ids[i];
        problem->adj_matrix[task_id][ADJ_INDEX(prev_id)] |= ADJ_MASK(prev_id);
        problem->adj_matrix[prev_id][ADJ_INDEX(task_id)] |= ADJ_MASK(task_id);
    }
}

void problem_add_type(problem_t* problem, int vt_id, vlen_t* capacities,
                      double price, int limit) {
    type_info_t* type = problem->types + vt_id;
    memcpy(type->capacities, capacities, sizeof(vlen_t) * RES_DIM);
    type->price = price * problem->charge_unit;
    type->limit = limit;
    type->demands[0] = 1;
}

void problem_set_runtime(problem_t* problem, int task_id, int type_id,
                         int runtime) {
    problem->rt_matrix[type_id][task_id] = runtime;
}

void problem_reverse_dag(problem_t* problem) {
    int tmp_num;
    int* tmp_dep;
    task_info_t* task;
    for (int task_id = 0; task_id < problem->num_tasks; ++task_id) {
        task = problem_task(problem, task_id);
        tmp_num = task->num_nexts;
        task->num_nexts = task->num_prevs;
        task->num_prevs = tmp_num;
        tmp_dep = task->prevs;
        task->prevs = task->nexts;
        task->nexts = tmp_dep;
    }
}

int problem_task_average_runtime(problem_t* problem, int task_id) {
    int sum = 0;
    for (int i = 0; i < problem->num_types; ++i)
        sum += problem_task_runtime(problem, task_id, i);
    return sum / problem->num_types;
}

int problem_cheapest_type(problem_t* problem) {
    double min_price = DBL_MAX;
    int cheapest_type = -1;
    for (int type_id = 0; type_id < problem->num_types; ++type_id)
        if (problem_type_price(problem, type_id) < min_price)
            cheapest_type = type_id;
    return cheapest_type;
}

int problem_cheapest_type_for_demands(problem_t* problem, vlen_t* demands) {
    double min_price = DBL_MAX;
    int cheapest_type = -1;
    for (int type_id = 0; type_id < problem->num_types; ++type_id) {
        if (res_lt(demands, problem_type_capacities(problem, type_id)) &&
            flt(problem_type_price(problem, type_id), min_price)) {
            min_price = problem_type_price(problem, type_id);
            cheapest_type = type_id;
        }
    }
    return cheapest_type;
}
