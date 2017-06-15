from WSMS.c.common cimport Resources
from WSMS.c.problem cimport problem_t, Problem
from WSMS.c.platform cimport Platform

from libcpp cimport bool

cdef extern from "schedule.h":
    struct machine_t

    struct objectives_t:
        int makespan
        double cost

    struct schedule_t:
        int num_tasks
        int num_vms

        int* placements
        int* start_times
        int* vm_types

        objectives_t objectives
        int pnvm

        int* _finish_times
        int* _vm_open_times
        int* _vm_close_times

    void schedule_init(schedule_t* schedule, int num_tasks, int num_vms)
    void schedule_destory(schedule_t* schedule)
    void schedule_set_placements(schedule_t* schedule, int* placements)
    void schedule_set_vm_types(schedule_t* schedule, int* vm_types)
    void schedule_set_start_times(schedule_t* schedule, int* start_times)

    void schedule_simulate(schedule_t* schedule, problem_t* problem,
                           int* order, bool forward)
    void schedule_calculate_objectives(schedule_t* schedule, problem_t* problem)
    void schedule_calculate_pnvm(schedule_t* schedule, problem_t* problem)

    int PL(schedule_t* schedule, int task_id)
    int TYP(schedule_t* schedule, int vm_id)
    int ST(schedule_t* schedule, int task_id)
    int FT(schedule_t* schedule, int task_id)
    int TYPL(schedule_t* schedule, int task_id)

    objectives_t schedule_objectives(schedule_t* schedule)
    int schedule_pnvm(schedule_t* schedule)


cdef class Schedule:
    cdef schedule_t c
    cdef Problem problem
    cdef bool _calculated
