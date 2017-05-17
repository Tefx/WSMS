from WSMS.c.common cimport Resources
from WSMS.c.problem cimport problem_t, Problem
from WSMS.c.machine cimport machine_t, Machine

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

    void schedule_init(schedule_t* schedule, int* placements, int* vm_types,
                       int num_tasks, int num_vms)
    void schedule_free(schedule_t* schedule)
    void schedule_complete_1(problem_t* problem, schedule_t* schedule,
                             int* scheduling_order, machine_t* vms)
