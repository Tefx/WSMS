
#include "problem.h"
#include <stdlib.h>
#include <string.h>

void problem_init(problem_t* problem, int num_tasks, int num_types,
                  int total_limit, int charge_unit) {
    problem->num_tasks = num_tasks;

    problem->tasks = (task_t*)malloc(sizeof(task_t) * num_tasks);
    problem->num_types = num_types;
    problem->types = (mtype_t*)malloc(sizeof(mtype_t) * num_types);
    problem->total_limit = total_limit;
    problem->charge_unit = charge_unit;
    problem->rt_matrix = (int*)malloc(sizeof(int) * num_tasks * num_types);
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

void problem_add_task(problem_t* problem, int task_id, resources_t res_demands,
                      int* prev_ids, int num_prevs, int* next_ids,
                      int num_nexts) {
    task_t* task = problem->tasks + task_id;
    task->demands = res_demands;
    task->num_prevs = num_prevs;
    task->num_nexts = num_nexts;
    task->prevs = (int*)malloc(sizeof(int) * num_prevs);
    memcpy(task->prevs, prev_ids, sizeof(int) * num_prevs);
    task->nexts = (int*)malloc(sizeof(int) * num_nexts);
    memcpy(task->nexts, next_ids, sizeof(int) * num_nexts);
}

void problem_add_type(problem_t* problem, int vt_id, resources_t capacities,
                      double price, int limit) {
    mtype_t* type = problem->types + vt_id;
    type->capacities = capacities;
    type->price = price;
    type->limit = limit;
}

void problem_set_runtime(problem_t* problem, int task_id, int type_id,
                         int runtime) {
    problem->rt_matrix[task_id * problem->num_types + type_id] = runtime;
}

int problem_task_average_runtime(problem_t* problem, int task_id) {
    int sum = 0;
    for (int i = 0; i < problem->num_types; ++i)
        sum += problem_task_runtime(problem, task_id, i);
    return sum / problem->num_types;
}
