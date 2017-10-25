#ifndef WSMS_SIMULATION_H
#define WSMS_SIMULATION_H

#include "common.h"
#include "bin.h"
#include "problem.h"

typedef struct task_t{
    item_t item;
    int task_id;
} task_t;

void task_prepare(task_t* task, problem_t *problem, int type_id) {
    task->item.length = problem_task_runtime(problem, task->task_id, type_id);
}

#endif
