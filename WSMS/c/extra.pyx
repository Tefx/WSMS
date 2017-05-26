from WSMS.c.problem cimport Problem
from cpython cimport array
import array

def sort_tasks_by_upward_ranks(Problem problem):
    cdef array.array ranks = array.array("i")
    cdef array.array order = array.array("i")
    cdef int num_tasks = problem.num_tasks
    array.resize(ranks, num_tasks)
    array.resize(order, num_tasks)
    wsmse_assign_upward_ranks(&problem.c, ranks.data.as_ints)
    wsmse_sort_tasks_by_priorities(ranks.data.as_ints, order.data.as_ints, num_tasks)
    return order

def earliest_start_time(Problem problem, int task_id, array.array finish_times):
    return wsmse_earliest_start_time(&problem.c, task_id, finish_times.data.as_ints)
