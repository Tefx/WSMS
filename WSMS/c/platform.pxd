from WSMS.c.common cimport vlen_t, Resources
from WSMS.c.problem cimport *
from WSMS.c.mpool cimport MemPool, mempool_t, mp_free_pool


cdef extern from "platform.h":
    struct item_t
    struct bin_t
    ctypedef vlen_t* volume_t

    ctypedef item_t* task_t
    void task_prepare(task_t* task, problem_t* problem, int task_id, int type_id);
    void task_set_start_time(task_t* task, int st)
    int task_start_time(task_t* task)
    int task_finish_time(task_t* task)

    ctypedef bin_t* machine_t
    mempool_t* machine_create_mpool(int buffer_size)
    void machine_init(machine_t *machine, mempool_t* mpool)
    void machine_destory(machine_t *machine)
    void machine_set_demands(machine_t* machine, vlen_t* demands)
    void machine_set_runtime(machine_t* machine, int runtime)

    void machine_print(machine_t* machine)

    int machine_open_time(machine_t *machine)
    int machine_close_time(machine_t *machine)
    int machine_runtime(machine_t *machine)

    int machine_earliest_position(machine_t *machine, task_t *task, int est,
                                  vlen_t* capacities)
    int machine_place_task(machine_t *machine, task_t *task)
    void machine_shift_task(machine_t *machine, task_t *task, int delta);
    int machine_extendable_interval_start(machine_t *machine, task_t *task,
                                          vlen_t* capacities)
    int machine_extendable_interval_finish(machine_t *machine, task_t *task,
                                           vlen_t* capacities)

    ctypedef bin_t* platform_t
    mempool_t* platform_create_mpool(int buffer_size)

    void platform_init(platform_t *platform, mempool_t* mpool)
    void platform_destory(platform_t *platform)

    void platform_print(platform_t* platform)

    int platform_earliest_position(platform_t *platform, machine_t *machine,
                                   int est, vlen_t* plim)
    int platform_place_machine(platform_t *platform, machine_t *machine)
    void platform_extend_machine(platform_t *platform, machine_t *machine)
    void platform_shift_machine(platform_t *platform, machine_t *machine,
                                int delta)
    int platform_extendable_interval_start(platform_t *platform,
                                           machine_t *machine, vlen_t* plim)
    int platform_extendable_interval_finish(platform_t *platform,
                                            machine_t *machine, vlen_t* plim)

    int machine_pnumber(machine_t *machines, int num_machines)


cdef class Task:
    cdef task_t c
    cdef int _task_id


cdef class Machine:
    cdef machine_t c
    cdef int _type_id
    cdef problem_t* _problem
    cdef set _tasks
    cdef mempool_t* _mpool
    cdef bool _own_mpool


cdef class Platform:
    cdef platform_t c
    cdef vlen_t* _limits
    cdef set _machines
    cdef mempool_t* _mpool
    cdef bool _own_mpool
