from WSMS.c.common cimport Resources
from WSMS.c.problem cimport problem_t, Problem
from WSMS.c.platform cimport machine_t, Machine

cdef extern from "schedule.h":
    struct objectives_t:
        int makespan
        double cost

    struct schedule_t:
        int num_tasks
        int num_vms
        int* start_times
        int* finish_times
        int* placements
        int* vm_types
        objectives_t objectives
    void schedule_init(schedule_t* schedule, int num_tasks)
    void schedule_free(schedule_t* schedule)

    void schedule_set_placements(schedule_t* schedule, int* placements)
    void schedule_set_vm_types(schedule_t* schedule, int* vm_types, int num_vms)
    void schedule_set_start_times(schedule_t* schedule, int* start_times)
    void schedule_set_finish_times(schedule_t* schedule, int* finish_times)
    void schedule_set_objectives(schedule_t* schedule, int makespan, double cost)

    void schedule_autofill_start_times(schedule_t* schedule, problem_t* problem)
    void schedule_autofill_finish_times(schedule_t* schedule, problem_t* problem)
    void schedule_autofill_1(schedule_t* schedule, problem_t* problem, 
                             int* order, machine_t* vms)

    int PL(schedule_t* schedule, int task_id)
    int TYP(schedule_t* schedule, int vm_id)
    int ST(schedule_t* schedule, int task_id)
    int FT(schedule_t* schedule, int task_id)
    int TYPL(schedule_t* schedule, int task_id)
