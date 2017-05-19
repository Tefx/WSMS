from WSMS.c.common cimport resources_t, Resources
from WSMS.c.problem cimport *


cdef extern from "platform.h":
    struct bin_t
    struct bin_node_t
    struct bin_item_t:
        bin_node_t* start_node
        bin_node_t* finish_node

    struct machine_t:
        bin_t* bin;
        bin_node_t *finger;
        bin_item_t item_in_platform;

    void machine_init(machine_t *machine)
    void machine_free(machine_t *machine)

    int machine_open_time(machine_t *machine)
    int machine_close_time(machine_t *machine)
    int machine_runtime(machine_t *machine)

    void machine_shift(machine_t *machine, int delta)

    int machine_alloc_earliest(machine_t *machine, int est, int rt, 
                               resources_t *demands, resources_t *capacities)
    int machine_earliest_slot(machine_t *machine, int est, int rt,
                              resources_t *demands, resources_t *capacities)
    void machine_place_task(machine_t *machine, int st, int rt,
                            resources_t *demands)

    struct platform_t:
        bin_t* bin;
        bin_node_t *finger;

    void platform_init(platform_t *platform)
    void platform_free(platform_t *platform)

    int platform_earliest_slot(platform_t *platform, int est, int rt,
                               int total_limit)

    machine_t *platform_alloc_machine(platform_t *platform, machine_t* machine,
                                      int st, int rt)
    void platform_free_machine(platform_t *platform, machine_t *machine)

    void platform_extend_machine(platform_t *platform, machine_t *machine,
                                 int st, int ft)
    void platform_shift_machine(platform_t *platform, machine_t *machine,
                                int delta)
    void platform_extendable_interval(platform_t *platform, machine_t *machine,
                                      int *start, int *finish, int total_limit)

    int machine_pnumber(machine_t *machines, int num_machines)

cdef class Machine:
    cdef machine_t c
    cdef int _type_id
    cdef resources_t _capacities
    cdef problem_t* _problem
