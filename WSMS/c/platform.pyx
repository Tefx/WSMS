from WSMS.c.common cimport wrap_c_resources

from cpython cimport array
import array
from operator import attrgetter


cdef class Task:
    def __cinit__(self, int task_id):
        self._task_id = task_id

    # def __dealloc__(self):
        # self.c.demands = NULL

    @property
    def start_time(self):
        return task_start_time(&self.c)

    @start_time.setter
    def start_time(self, st):
        task_set_start_time(&self.c, st)

    @property
    def finish_time(self):
        return task_finish_time(&self.c)

    @property
    def task_id(self):
        return self._task_id


cdef class Machine:
    def __cinit__(self, Problem problem):
        machine_init(&self.c, problem.num_tasks)
        self._problem = &problem.c
        self._tasks = set()

    def __dealloc__(self):
        machine_destory(&self.c)

    def prepare(self, Task task, int type_id):
        cdef rt = problem_task_runtime(self._problem, task._task_id, self._type_id)
        task_prepare(&task.c, self._problem, task._task_id, self._type_id)
        self.type_id = type_id
        machine_set_runtime(&self.c, rt)

    @classmethod
    def new_for_task(cls, Problem problem, Task task, int type_id):
        machine = cls(problem)
        machine.prepare(task, type_id)
        return machine

    def earliest_position(self, Task task, int est, int type_id=-1):
        if type_id < 0: type_id = self._type_id
        cdef vlen_t* capacities = problem_type_capacities(self._problem, type_id)
        task_prepare(&task.c, self._problem, task._task_id, type_id)
        return machine_earliest_position(&self.c, &task.c, est, capacities)

    def extendable_interval(self, Task task):
        cdef vlen_t* capacities = problem_type_capacities(self._problem, self._type_id)
        cdef int st = machine_extendable_interval_start(&self.c, &task.c, capacities)
        cdef int ft = machine_extendable_interval_finish(&self.c, &task.c, capacities)
        return max(st, self.open_time), min(ft, self.close_time)

    def place(self, Task task):
        self._tasks.add(task)
        return machine_place_task(&self.c, &task.c)

    def shift(self, Task task, int delta):
        # print()
        # print("Shifting", task.task_id, delta)
        machine_shift_task(&self.c, &task.c, delta)

    @property
    def type_id(self):
        return self._type_id

    @type_id.setter
    def type_id(self, type_id):
        self._type_id = type_id
        machine_set_demands(&self.c, problem_type_demands(self._problem, type_id))

    @property
    def open_time(self):
        return machine_open_time(&self.c)

    @property
    def close_time(self):
        return machine_close_time(&self.c)

    @property
    def peak_usage(self):
        return wrap_c_resources(machine_peak_usage(&self.c))

    def tight_type(self):
        self.type_id = problem_cheapest_type_for_demands(self._problem, 
                                                         machine_peak_usage(&self.c));

    def tight_span(self, list tasks):
        cdef int* deps;
        for task in sorted(self._tasks, key=attrgetter("finish_time"), reverse=True):
            deps = problem_task_nexts(self._problem, task.task_id)
            lft = self.extendable_interval(task)[1]
            for i in range(problem_task_num_nexts(self._problem, task.task_id)):
                lft = min(lft, tasks[deps[i]].start_time)
            self.shift(task, lft - task.finish_time)
        for task in sorted(self._tasks, key=attrgetter("start_time")):
            deps = problem_task_prevs(self._problem, task.task_id)
            est = self.extendable_interval(task)[0]
            for i in range(problem_task_num_prevs(self._problem, task.task_id)):
                est = max(est, tasks[deps[i]].finish_time)
            self.shift(task, est - task.start_time)

    @property
    def runtime(self):
        return machine_runtime(&self.c)

    @runtime.setter
    def runtime(self, runtime):
        machine_set_runtime(&self.c, runtime)

    @property
    def cost(self):
        return problem_charge(self._problem, self._type_id, machine_runtime(&self.c))

    def cost_increase(self, Task task, int type_id):
        cdef int st = min(machine_open_time(&self.c), task.start_time)
        cdef int rt = max(machine_close_time(&self.c), task.finish_time) - st
        return problem_charge(self._problem, type_id, rt) - self.cost

    @property
    def tasks(self):
        return self._tasks

    def print_bin(self):
        machine_print(&self.c)


cdef class Platform:
    def __cinit__(self, Problem problem):
        self._limits = &problem.c.limits[0]
        self._machines = set()
        platform_init(&self.c, min(problem.c.limits[0], problem.c.num_tasks))

    def __dealloc__(self):
        platform_destory(&self.c)

    def earliest_position(self, Machine machine, int est):
        return platform_earliest_position(&self.c, &machine.c, est, self._limits)

    def update(self, Machine machine):
        if machine not in self._machines:
            self._machines.add(machine)
            platform_place_machine(&self.c, &machine.c)
        else:
            platform_extend_machine(&self.c, &machine.c)

    def shift(self, Machine machine, int delta):
        platform_shift_machine(&self.c, &machine.c, delta)

    def extendable_interval(self, Machine machine):
        cdef int st = platform_extendable_interval_start(&self.c, &machine.c, self._limits)
        cdef int ft = platform_extendable_interval_finish(&self.c, &machine.c, self._limits)
        return st, ft

    @property
    def machines(self):
        return self._machines

    @property
    def peak_usage(self):
        return platform_peak_usage(&self.c)[0]

    def print_bin(self):
        platform_print(&self.c)
