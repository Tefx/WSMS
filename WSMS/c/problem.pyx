from cpython cimport array
import array
from math import ceil
import os.path
import json


cdef class Problem:
    def __cinit__(self, dict tasks, dict mtypes,
                   int total_limit, int charge_unit):
        cdef res_t res
        cdef array.array prevs, nexts
        cdef int rt

        self.r_task_ids = list(tasks.keys())
        self.r_type_ids = sorted(mtypes.keys(), key=lambda x:mtypes[x]["capacities"])
        problem_init(&self.c, len(tasks), len(mtypes), total_limit, charge_unit)

        for i, rtid in enumerate(self.r_task_ids):
            res[0] = ceil(tasks[rtid]["demands"][0] * 1000)
            res[1] = ceil(tasks[rtid]["demands"][1])
            prevs = array.array("i", [self.r_task_ids.index(t) for t in tasks[rtid]["prevs"]])
            nexts = array.array("i", [self.r_task_ids.index(t) for t in tasks[rtid]["nexts"]])
            problem_add_task(&self.c, i, res,
                             prevs.data.as_ints, len(prevs),
                             nexts.data.as_ints, len(nexts))

        for i, rmid in enumerate(self.r_type_ids):
            res[0] = ceil(mtypes[rmid]["capacities"][0] * 1000)
            res[1] = ceil(mtypes[rmid]["capacities"][1])
            problem_add_type(&self.c, i, res,
                             mtypes[rmid]["price"], mtypes[rmid]["limit"])

        for i, rtid in enumerate(self.r_task_ids):
            for j, rmid in enumerate(self.r_type_ids):
                rt = ceil(tasks[rtid]["runtime"]/mtypes[rmid]["speed"])
                problem_set_runtime(&self.c, i, j, rt)

    def __dealloc__(self):
        problem_destory(&self.c)

    def task_demands(self, int task_id):
        res = Resources()
        res._setc(problem_task_demands(&self.c, task_id))
        return res

    def task_runtime(self, int task_id, int type_id):
        return problem_task_runtime(&self.c, task_id, type_id)

    def task_is_entry(self, int task_id):
        return problem_task_is_entry(&self.c, task_id)

    def task_is_exit(self, int task_id):
        return problem_task_is_exit(&self.c, task_id)

    def task_prevs(self, int task_id):
        cdef task_info_t* task = problem_task(&self.c, task_id)
        return [task.prevs[i] for i in range(task.num_prevs)]

    def task_nexts(self, int task_id):
        cdef task_info_t* task = problem_task(&self.c, task_id)
        return [task.nexts[i] for i in range(task.num_prevs)]

    def task_is_adjacent(self, int task_id_0, int task_id_1):
        return problem_task_is_adjacent(&self.c, task_id_0, task_id_1)

    def valid_types_for_demands(self, Resources demands):
        for type_id in range(self.c.num_types):
            if res_le(demands.c, problem_type_capacities(&self.c, type_id)):
                yield type_id

    def cheapest_type_for_demands(self, Resources demands):
        return problem_cheapest_type_for_demands(&self.c, demands.c)

    def type_capacities(self, int type_id):
        res = Resources()
        res._setc(problem_type_capacities(&self.c, type_id))
        return res

    def type_price(self, int type_id):
        return problem_type_price(&self.c, type_id)

    def charge(self, int type_id, int runtime):
        return problem_charge(&self.c, type_id, runtime)

    def type_le(self, int tid_0, int tid_1):
        cdef vlen_t* cap0 = problem_type_capacities(&self.c, tid_0)
        cdef vlen_t* cap1 = problem_type_capacities(&self.c, tid_1)
        return res_le(cap0, cap1)

    @property
    def num_tasks(self):
        return self.c.num_tasks

    @property
    def num_types(self):
        return self.c.num_types

    @property
    def tasks(self):
        return range(self.c.num_tasks)

    @property
    def types(self):
        return range(self.c.num_types)

    @property
    def total_limit(self):
        return self.c.limits[0]

    @classmethod
    def load(cls, dax_name, type_file, total_limit, charge_unit,
             families=["c4"], path="resources"):
        with open(os.path.join(path, "platforms/{}.plt".format(type_file))) as f:
            types = {k:v for k,v in json.load(f).items() \
                     if k.split(".")[0] in families}
        with open(os.path.join(path, "workflows/{}.wrk".format(dax_name))) as f:
            tasks = json.load(f)
        return cls(tasks, types, total_limit, charge_unit)

