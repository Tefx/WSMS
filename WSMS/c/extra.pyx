# cython: profile=False, cdivision=True, boundscheck=False, wraparound=False, initializedcheck=False
from WSMS.c.problem cimport Problem
from cpython cimport array
import array

def sort_by_upward_ranks(Problem problem):
    cdef array.array ranks = array.array("i")
    cdef array.array order = array.array("i")
    cdef int num_tasks = problem.num_tasks()
    array.resize(ranks, num_tasks)
    array.resize(order, num_tasks)
    assign_upward_ranks(&problem.c, ranks.data.as_ints)
    sort_by_priorities(ranks.data.as_ints, order.data.as_ints, num_tasks)
    return order
