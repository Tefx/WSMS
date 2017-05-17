from libcpp cimport bool

cdef extern from "machine.h":
    struct bin_t
    struct machine_t:
        int type_id
        bin_t* bin

    void machine_init(machine_t* vm, int type_id)
    void machine_free(machine_t* vm)
    int machine_open_time(machine_t* vm)
    int machine_close_time(machine_t* vm)
    int machine_runtime(machine_t* vm)

cdef extern from "problem.h":
    struct resources_t:
        double core
        double memory

    bool res_richcmp(resources_t* r0, resources_t* r1, int op)
    void res_scale(resources_t* r0, resources_t* r1)

    struct task_t
    struct vm_type_t

    struct problem_t:
        task_t* tasks
        vm_type_t* vm_types
        int num_tasks
        int num_types
        int total_limit
        int charge_unit
        int* rt_matrix

    void problem_init(problem_t* problem,
                      int num_tasks,
                      int num_types,
                      int total_limit,
                      int charge_unit)
    void problem_add_task(problem_t* problem,
                          int task_id,
                          resources_t res_demands,
                          int ref_runtime,
                          int* prev_ids,
                          int num_prevs,
                          int* next_ids,
                          int num_nexts)
    void problem_add_type(problem_t* problem,
                          int vt_id,
                          resources_t capacities,
                          double ref_speed,
                          double price,
                          double limit)
    void problem_free(problem_t *problem)
    int runtime_on(problem_t* problem, int task_id, int vm_id)
    bool is_entry(problem_t* problem, int task_id)
    bool is_exit(problem_t* problem, int task_id)
    resources_t task_demands(problem_t* problem, int task_id)
    resources_t type_capacities(problem_t *problem, int type_id)
    int vm_cost(problem_t* problem, machine_t* vm)
    int vm_alloc_earliest(problem_t* problem,
                          machine_t* vm,
                          int type_id,
                          int task_id,
                          int est)

    struct object_t:
        int makespan;
        double cost;

    struct schedule_t:
        int num_vms
        int* start_times
        int* placements
        int* vm_types
        object_t object

    void schedule_init(schedule_t* schedule,
                       int* placements,
                       int* vm_types,
                       int num_tasks,
                       int num_vms)

    void schedule_free(schedule_t* schedule)

    void schedule_complete_1(problem_t* problem,
                             schedule_t* schedule,
                             int* scheduling_order,
                             machine_t* vms)

cdef class Resources:
    cdef resources_t c_res

cdef class Problem:
    cdef problem_t c_problem
    cdef list real_ids
    cdef list real_vm_types
    cpdef num_tasks(self)
    cpdef num_types(self)

cdef class Machine:
    cdef machine_t c_vm

cdef class Schedule:
    cdef schedule_t c_schedule

