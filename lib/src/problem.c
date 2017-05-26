
#include "problem.h"
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
}

void problem_free(problem_t* problem) {
    for (int i = 0; i < problem->num_tasks; ++i) {
        free(problem->tasks[i].prevs);
        free(problem->tasks[i].nexts);
    }
    free(problem->tasks);
    free(problem->types);
    free(problem->rt_matrix);
}

void problem_add_task(problem_t* problem, int task_id, res_t demands,
                      int* prev_ids, int num_prevs, int* next_ids,
                      int num_nexts) {
    task_info_t* task = problem->tasks + task_id;
    memcpy(task->demands, demands, sizeof(vlen_t) * RES_DIM);
    task->num_prevs = num_prevs;
    task->num_nexts = num_nexts;
    task->prevs = (int*)malloc(sizeof(int) * (num_prevs + num_nexts));
    memcpy(task->prevs, prev_ids, sizeof(int) * num_prevs);
    task->nexts = (int*)malloc(sizeof(int) * num_nexts);
    memcpy(task->nexts, next_ids, sizeof(int) * num_nexts);
}

void problem_add_type(problem_t* problem, int vt_id, res_t capacities,
                      double price, int limit) {
    type_info_t* type = problem->types + vt_id;
    memcpy(type->capacities, capacities, sizeof(vlen_t) * RES_DIM);
    type->price = price * problem->charge_unit;
    type->limit = limit;
    type->demands[0] = 1;
}

/*int _compare_types(const void* type_a, const void* type_b) {*/
    /*vlen_t* a = ((type_info_t*)type_a)->capacities;*/
    /*vlen_t* b = ((type_info_t*)type_b)->capacities;*/
    /*int res = 0;*/
    /*for (int i = 0; i < RES_DIM; ++i) {*/
        /*res = a[i] - b[i];*/
        /*if (res != 0) break;*/
    /*}*/
    /*return res;*/
/*}*/

/*void _sort_types(problem_t* problem) {*/
    /*qsort(problem->types, problem->num_types, sizeof(type_info_t), _compare_types);*/
/*}*/

void problem_set_runtime(problem_t* problem, int task_id, int type_id,
                         int runtime) {
    problem->rt_matrix[type_id][task_id] = runtime;
}

int problem_task_average_runtime(problem_t* problem, int task_id) {
    int sum = 0;
    for (int i = 0; i < problem->num_types; ++i)
        sum += problem_task_runtime(problem, task_id, i);
    return sum / problem->num_types;
}
