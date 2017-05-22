from WSMS.c.common cimport res_t, plim_t, vlen_t, Resources
from WSMS.c.problem cimport *


cdef extern from "platform.h":
    struct item_t
    struct bin_t

    ctypedef item_t* task_t
    void task_set(task_t* task, int length, res_t demands)
    int task_start_time(task_t* task)
    int task_finish_time(task_t* task)

    ctypedef bin_t* machine_t
    void machine_init(machine_t *machine)
    void machine_destory(machine_t *machine)
    void machine_set(machine_t* machine, plim_t demands)

    int machine_open_time(machine_t *machine)
    int machine_close_time(machine_t *machine)
    int machine_runtime(machine_t *machine)

    int machine_earliest_position(machine_t *machine, task_t *task, int est,
                                  res_t capacities)
    int machine_place_task(machine_t *machine, task_t *task)
    void machine_shift_task(machine_t *machine, task_t *task, int delta);
    int machine_extendable_interval_start(machine_t *machine, task_t *task,
                                          res_t capacities)
    int machine_extendable_interval_finish(machine_t *machine, task_t *task,
                                           res_t capacities)

    ctypedef bin_t* platform_t
    void platform_init(platform_t *platform)
    void platform_destory(platform_t *platform)

    int platform_earliest_position(platform_t *platform, machine_t *machine,
                                   int est, plim_t plim)
    int platform_place_machine(platform_t *platform, machine_t *machine)
    void platform_extend_machine(platform_t *platform, machine_t *machine)
    void platform_shift_machine(platform_t *platform, machine_t *machine,
                                int delta)
    int platform_extendable_interval_start(platform_t *platform,
                                           machine_t *machine, plim_t plim)
    int platform_extendable_interval_finish(platform_t *platform,
                                            machine_t *machine, plim_t plim)


    int machine_pnumber(machine_t *machines, int num_machines)
