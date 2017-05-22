#include "extra.h"
#include <stdlib.h>

int _upwark_rank(problem_t* problem, int task_id, int* ranks) {
  if (ranks[task_id] < 0) {
    int rank = 0;
    int nrank;
    task_info_t* task = problem_task(problem, task_id);
    for (int i = 0; i < task->num_nexts; ++i) {
      nrank = _upwark_rank(problem, task->nexts[i], ranks);
      iMAX(rank, nrank);
    }
    rank += problem_task_average_runtime(problem, task_id);
    ranks[task_id] = rank;
  }
  return ranks[task_id];
}

void assign_upward_ranks(problem_t* problem, int* ranks) {
  for (int i = 0; i < problem->num_tasks; ++i)
    ranks[i] = -1;
  for (int i = 0; i < problem->num_tasks; ++i)
    _upwark_rank(problem, i, ranks);
}

int compare_priorities(const void* p1, const void* p2, void* pa) {
  const int a = ((const int*)pa)[*(const int*)p1];
  const int b = ((const int*)pa)[*(const int*)p2];
  return (a < b) - (a > b);
}

void sort_tasks_by_priorities(int* priorities, int* results, int num_tasks) {
  for (int i = 0; i < num_tasks; ++i)
    results[i] = i;
  qsort_r(results, num_tasks, sizeof(int), compare_priorities, priorities);
}
