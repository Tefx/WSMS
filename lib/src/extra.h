#ifndef WSMS_EXTRA_H
#define WSMS_EXTRA_H

#include "problem.h"

void assign_upward_ranks(problem_t* problem, int* ranks);
void sort_tasks_by_priorities(int* priorities, int* results, int num_tasks);
#endif /* ifndef WSMS_EXTRA_H_ */
