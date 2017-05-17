# cython: profile=False, cdivision=True, boundscheck=False, wraparound=False, initializedcheck=False
from cpython cimport array
import array
from math import ceil

cdef class Problem:
    def __cinit__(self, dict tasks, dict mtypes,
                   int total_limit, int charge_unit):
        cdef resources_t res
        cdef array.array prevs, nexts
        cdef int rt

        self.r_task_ids = list(tasks.keys())
        self.r_type_ids = list(mtypes.keys())
        problem_init(&self.c, len(tasks), len(mtypes), total_limit, charge_unit)

        for i, rtid in enumerate(self.r_task_ids):
            res.core, res.memory = tasks[rtid]["demands"]
            prevs = array.array("i", [self.r_task_ids.index(t) for t in tasks[rtid]["prevs"]])
            nexts = array.array("i", [self.r_task_ids.index(t) for t in tasks[rtid]["nexts"]])
            problem_add_task(&self.c, i, res,
                             prevs.data.as_ints, len(prevs),
                             nexts.data.as_ints, len(nexts))

        for i, rmid in enumerate(self.r_type_ids):
            res.core, res.memory = mtypes[rmid]["capacities"]
            problem_add_type(&self.c, i, res,
                             mtypes[rmid]["price"], mtypes[rmid]["limit"])

        for i, rtid in enumerate(self.r_task_ids):
            for j, rmid in enumerate(self.r_type_ids):
                rt = ceil(tasks[rtid]["runtime"]/mtypes[rmid]["speed"])
                problem_set_runtime(&self.c, i, j, rt)

    def __dealloc__(self):
        problem_free(&self.c)

    def task_demands(self, int task_id):
        res = Resources()
        res._setc(problem_task_demands(&self.c, task_id))
        return res

    def task_runtime(self, int task_id, int type_id):
        return problem_task_runtime(&self.c, task_id, type_id)

    def task_prevs(self, int task_id):
        cdef task_t* task = problem_task(&self.c, task_id)
        return [task.prevs[i] for i in range(task.num_prevs)]

    def task_nexts(self, int task_id):
        cdef task_t* task = problem_task(&self.c, task_id)
        return [task.nexts[i] for i in range(task.num_prevs)]

    def type_capacities(self, int type_id):
        res = Resources()
        res._setc(problem_type_capacities(&self.c, type_id))
        return res

    def type_price(self, int type_id):
        return problem_type_price(&self.c, type_id)

    def charge(self, int type_id, int runtime):
        return problem_charge(&self.c, type_id, runtime)

    def num_tasks(self):
        return self.c.num_tasks

    def tasks(self):
        return range(self.c.num_tasks)
