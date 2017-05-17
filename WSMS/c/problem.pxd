from WSMS.c.common cimport resources_t

cdef extern from "problem.h":
    struct task_t:
        resources_t demands
        int num_prevs
        int num_nexts
        int* prevs
        int* nexts

    struct mtype_t:
        resources_t capacities
        double price
        int limit

    struct problem_t:
        task_t* tasks
        mtype_t* types
        int num_tasks
        int num_types
        int total_limits
        int charge_unit
        int* rt_matrix

    void problem_init(problem_t* problem, int num_tasks, int num_types,
                      int total_limit, int charge_unit)
    void problem_free(problem_t* problem)

    void problem_add_task(problem_t* problem, int task_id,
                          resources_t res_demands,
                          int* prev_ids, int num_prevs,
                          int* next_ids, int num_nexts)
    void problem_add_type(problem_t* problem, int vt_id, resources_t capacities,
                          double price, int limit)
    void problem_set_runtime(problem_t* problem,
                        int task_id, int type_id, int runtime)

    task_t* problem_task(problem_t* problem, int task_id)
    resources_t problem_task_demands(problem_t* problem, int task_id)
    int problem_task_runtime(problem_t* problem, int task_id, int type_id)
    int problem_task_average_runtime(problem_t* problem, int task_id)

    resources_t problem_type_capacities(problem_t* Problem, int type_id)
    double problem_type_price(problem_t* problem, int type_id)

    double problem_charge(problem_t* problem, int type_id, int runtime)

cdef class Problem:
    cdef problem_t c
    cdef list r_task_ids
    cdef list r_type_ids
