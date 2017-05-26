cdef extern from "extra.h":
    struct problem_t
    void wsmse_assign_upward_ranks(problem_t* problem, int* ranks)
    void wsmse_sort_tasks_by_priorities(int* priorities, int* results, int num_tasks)
    int wsmse_earliest_start_time(problem_t * problem, int task_id, int* finish_times)
