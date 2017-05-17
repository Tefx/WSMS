from cpython cimport array
import array

cdef class Machine:

    cdef _setc(Machine self, machine_t* c):
        self.c = c[0]

    def __dealloc__(self):
        machine_free(&self.c)

    def open_time(Machine self):
        return machine_open_time(&self.c)

    def close_time(self):
        return machine_close_time(&self.c)

    def runtime(self):
        return machine_runtime(&self.c)

    def alloc_earliest(self, int est, int rt,
                       Resources demands, Resources capacities):
        return machine_alloc_earliest(&self.c, est, rt, &demands.c, &capacities.c)

    def earliest_slot(self, int est, int rt,
                      Resources demands, Resources capacities):
        return machine_earliest_slot(&self.c, est, rt, &demands.c, &capacities.c)

    def place_task(self, int st, int rt, Resources demands):
        machine_place_task(&self.c, st, rt, &demands.c)


cdef class Platform:
    cdef platform_t c

    def __cinit__(self):
        platform_init(&self.c)

    def __dealloc__(self):
        platform_free(&self.c)

    def earliest_slot(self, int est, int rt, int total_limit):
        return platform_earliest_slot(&self.c, est, rt, total_limit)

    def alloc_machine(self, int st, int rt):
        machine = Machine()
        platform_alloc_machine(&self.c, &machine.c, st, rt)
        return machine

    def extend_machine(self, Machine machine, int st, int ft):
        platform_extend_machine(&self.c, &machine.c, st, ft)

    def shift_machine(self, Machine machine, int delta):
        platform_shift_machine(&self.c, &machine.c, delta)

    def free_machine(self, Machine machine):
        platform_free_machine(&self.c, &machine.c)

    def extendable_interval(self, Machine machine, int total_limit):
        cdef int st, ft
        platform_extendable_interval(&self.c, &machine.c, &st, &ft, total_limit)
        return st, ft
