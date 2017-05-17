# cython: profile=False, cdivision=True, boundscheck=False, wraparound=False, initializedcheck=False
from libc.stdlib cimport malloc, free
from cpython cimport array
import array

cdef class Resources:
    @classmethod
    def zero(cls):
        res = cls()
        res._set_value(0, 0)
        return res

    def _set_value(self, double core, int memory):
        self.c_res.core = core
        self.c_res.memory = memory

    def __iadd__(Resources self, Resources other):
        self.c_res.core += other.c_res.core
        self.c_res.memory += other.c_res.memory
        return self

    def __isub__(Resources self, Resources other):
        self.c_res.core -= other.c_res.core
        self.c_res.memory -= other.c_res.memory
        return self

    def __richcmp__(Resources self, Resources other, int op):
        return res_richcmp(&self.c_res,&other.c_res, op)

    def scale(Resources self, Resources other):
        res_scale(&self.c_res, &other.c_res)

    def __str__(self):
        return "({:.2%}, {})".format(self.c_res.core, int(self.c_res.memory))

cdef class Problem:
    def __cinit__(self, dict tasks, dict vm_types,
                  int total_limit, int charge_unit):
        cdef resources_t res
        cdef array.array prev_ids, next_ids

        self.real_ids = list(tasks.keys())
        self.real_vm_types = list(vm_types.keys())
        problem_init(&self.c_problem,
                     len(tasks), len(vm_types),
                     total_limit, charge_unit)
        for i, real_id in enumerate(self.real_ids):
            res.core, res.memory = tasks[real_id]["demands"]
            prev_ids = array.array("i", 
                [self.real_ids.index(t) for t in tasks[real_id]["prevs"]])
            next_ids = array.array("i",
                [self.real_ids.index(t) for t in tasks[real_id]["nexts"]])
            problem_add_task(&self.c_problem,
                             i, res, tasks[real_id]["runtime"],
                             prev_ids.data.as_ints, len(prev_ids),
                             next_ids.data.as_ints, len(next_ids))

        for i, real_vt in enumerate(self.real_vm_types):
            res.core, res.memory = vm_types[real_vt]["capacities"]
            problem_add_type(&self.c_problem, i, res,
                             vm_types[real_vt]["speed"],
                             vm_types[real_vt]["price"],
                             vm_types[real_vt]["limit"])

    def __dealloc__(self):
        problem_free(&self.c_problem)

    def task_demands(self, int task_id):
        res = Resources()
        res.c_res = task_demands(&self.c_problem, task_id)
        return res

    def type_capacities(self, int type_id):
        res = Resources()
        res.c_res = type_capacities(&self.c_problem, type_id)
        return res

    def runtime_on(self, int task_id, int type_id):
        return runtime_on(&self.c_problem, task_id, type_id)

    cpdef num_tasks(self):
        return self.c_problem.num_tasks

    cpdef num_types(self):
        return self.c_problem.num_types

    def tasks(self):
        return range(self.c_problem.num_tasks)

    def types(self):
        return range(self.c_problem.num_types)


cdef class Machine:
    def __cinit__(self, int type_id = -1):
        if type_id >= 0:
            machine_init(&self.c_vm, type_id)

    def __dealloc__(self):
        machine_free(&self.c_vm)

    def cost(self, Problem problem):
        return vm_cost(&problem.c_problem, &self.c_vm)


cdef class Schedule:
    def __cinit__(self, array.array placements, array.array vm_types):
        schedule_init(&self.c_schedule, placements.data.as_ints,
                      vm_types.data.as_ints, len(placements), len(vm_types))

    def __dealloc__(self):
        schedule_free(&self.c_schedule)

    def complete_1(self, Problem problem, array.array order):
        schedule_complete_1(&problem.c_problem, &self.c_schedule,
                            order.data.as_ints, NULL)

    def object(self):
        return (self.c_schedule.object.core, self.c_schedule.object.memory)

    def PL(self, int task_id):
        return self.c_schedule.placements[task_id]

    def TYP(self, int vm_id):
        return self.c_schedule.vm_types[vm_id]

    def TYPL(self, int task_id):
        return self.c_schedule.vm_types[self.c_schedule.placements[task_id]]

    def ST(self, int task_id):
        return self.c_schedule.start_times[task_id]

    def finish_time(self, Problem problem, int task_id):
        return self.ST(task_id) + self.runtime(problem, task_id)

    def runtime(self, Problem problem, int task_id):
        return runtime_on(&problem.c_problem, task_id,
                          self.c_schedule.vm_types[self.c_schedule.placements[task_id]])

    def num_vms(self):
        return self.c_schedule.num_vms;

    def vm_ids(self):
        return range(self.c_schedule.num_vms)
