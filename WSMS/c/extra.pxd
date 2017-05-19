cdef extern from "extra.h":
    struct problem_t
    void assign_upward_ranks(problem_t* problem, int* ranks)
    void sort_tasks_by_priorities(int* priorities, int* results, int num_tasks)
