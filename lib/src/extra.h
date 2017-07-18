#ifndef WSMS_EXTRA_H
#define WSMS_EXTRA_H

#include "common.h"
#include "problem.h"

void wsmse_assign_upward_ranks(problem_t* problem, int* ranks);
void wsmse_sort_tasks_by_priorities(int* priorities, int* results, int num_tasks);
int wsmse_earliest_start_time(problem_t * problem, int task_id, int* finish_times);

#endif /* ifndef WSMS_EXTRA_H_ */
