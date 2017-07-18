from WSMS.c.common cimport res_t, plim_t, Resources, res_le, vlen_t
from libcpp cimport bool

cdef extern from "problem.h":
    struct task_info_t:
        res_t demands
        int num_prevs
        int num_nexts
        int* prevs
        int* nexts

    struct mtype_t:
        res_t capacities
        double price
        int limit
        plim_t demands;

    struct problem_t:
        task_info_t* tasks
        mtype_t* types
        int num_tasks
        int num_types
        plim_t limits;
        int charge_unit
        int* rt_matrix
        char** adj_matrix

    void problem_init(problem_t* problem, int num_tasks, int num_types,
                      int total_limit, int charge_unit)
    void problem_destory(problem_t* problem)

    void problem_add_task(problem_t* problem, int task_id,
                          res_t res_demands,
                          int* prev_ids, int num_prevs,
                          int* next_ids, int num_nexts)
    void problem_add_type(problem_t* problem, int vt_id, res_t capacities,
                          double price, int limit)
    void problem_set_runtime(problem_t* problem,
                        int task_id, int type_id, int runtime)

    bool problem_task_is_entry(problem_t* problem, int task_id)
    bool problem_task_is_exit(problem_t* problem, int task_id)
    bool problem_task_is_adjacent(problem_t* problem, int t0, int t1)

    task_info_t* problem_task(problem_t* problem, int task_id)
    vlen_t* problem_task_demands(problem_t* problem, int task_id)
    int problem_task_runtime(problem_t* problem, int task_id, int type_id)
    int problem_task_average_runtime(problem_t* problem, int task_id)

    plim_t problem_type_demands(problem_t* problem, int type_id)
    vlen_t* problem_type_capacities(problem_t* Problem, int type_id)
    double problem_type_price(problem_t* problem, int type_id)
    int problem_cheapest_type_for_demands(problem_t* problem, vlen_t* demands)

    double problem_charge(problem_t* problem, int type_id, int runtime)

cdef class Problem:
    cdef problem_t c
    cdef list r_task_ids
    cdef list r_type_ids
