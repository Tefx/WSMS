# cython: profile=False, cdivision=True, boundscheck=False, wraparound=False, initializedcheck=False
from libc.stdlib cimport malloc, free
from cpython cimport array
import array

cdef class Schedule:
    cdef schedule_t c

    def __cinit__(self, array.array placements, array.array vm_types):
        schedule_init(&self.c, placements.data.as_ints, vm_types.data.as_ints,
                      len(placements), len(vm_types))

    def __dealloc__(self):
        schedule_free(&self.c)

    def complete_1(self, Problem problem, array.array order):
        cdef machine_t* cms = <machine_t*>malloc(sizeof(machine_t) * self.c.num_vms)

        schedule_complete_1(&problem.c, &self.c, order.data.as_ints, cms)
        ms = []
        for i in range(self.c.num_vms):
            m = Machine()
            m._setc(&cms[i])
            ms.append(ms)
        free(cms)
        return ms

    def objectives(self):
        return (self.c.objectives.core, self.c.objectives.memory)

    def PL(self, int task_id):
        return self.c.placements[task_id]

    def TYP(self, int vm_id):
        return self.c.vm_types[vm_id]

    def TYP_Task(self, int task_id):
        return self.c.vm_types[self.c.placements[task_id]]

    def ST(self, int task_id):
        return self.c.start_times[task_id]

    def finish_time(self, Problem problem, int task_id):
        return self.ST(task_id) + problem.task_runtime(task_id, self.TYP_Task(task_id)) 

    def vms(self):
        return range(self.c.num_vms)
