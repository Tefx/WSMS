from cpython cimport array
import array


cdef class Task:
    cdef task_t c
    cdef int _task_id

    def __cinit__(self, int task_id):
        self._task_id = task_id
        task_init(&self.c)

    def __dealloc(self):
        task_destory(&self.c)

    @property
    def start_time(self):
        return task_start_time(&self.c)

    @property
    def finish_time(self):
        return task_finish_time(&self.c)

    @property
    def task_id(self):
        return self._task_id


cdef class Machine:
    cdef machine_t c
    cdef int _type_id
    cdef res_t _capacities
    cdef problem_t* _problem
    cdef set _tasks

    def __cinit__(self, Problem problem, int type_id):
        machine_init(&self.c)
        self._type_id = type_id
        self._capacities = problem_type_capacities(&problem.c, type_id)
        self._problem = &problem.c
        self._tasks = set()

    def __dealloc__(self):
        machine_destory(&self.c)

    def earliest_position(self, Task task, int est):
        cdef int task_id = task._task_id
        task_set(&task.c,
                 problem_task_runtime(self._problem, task_id, self._type_id),
                 problem_task_demands(self._problem, task_id))
        return machine_earliest_position(&self.c, &task.c, est, self._capacities)

    def place_task(self, Task task):
        self._tasks.add(task)
        return machine_place_task(&self.c, &task.c)

    def shift_task(self, Task task, int delta):
        machine_shift_task(&self.c, &task.c, delta)

    @property
    def type_id(self):
        return self._type_id

    @type_id.setter
    def type_id(self, type_id):
        self._type_id = type_id
        self._capacities = problem_type_capacities(self._problem, type_id)

    @property
    def open_time(self):
        return machine_open_time(&self.c)

    @property
    def close_time(self):
        return machine_close_time(&self.c)

    @property
    def runtime(self):
        return machine_runtime(&self.c)

    @property
    def cost(self):
        cdef int runtime = machine_runtime(&self.c)
        return problem_charge(self._problem, self._type_id, runtime)

    @property
    def tasks(self):
        return self._tasks


cdef class Platform:
    cdef platform_t c
    cdef int _total_limit
    cdef set _machines

    def __cinit__(self, Problem problem):
        platform_init(&self.c)
        self._total_limit =  problem.c.total_limit
        self._machines = set()

    def __dealloc__(self):
        platform_destory(&self.c)

    def earliest_position(self, Machine machine, int est):
        machine_set(&machine.c, 1)
        return platform_earliest_position(&self.c, &machine.c, est,
                                         self._total_limit)

    def provision_machine(self, Machine machine):
        self._machines.add(machine)
        return platform_place_machine(&self.c, &machine.c)

    def extend_machine(self, Machine machine):
        platform_extend_machine(&self.c, &machine.c)

    def shift_machine(self, Machine machine, int delta):
        platform_shift_machine(&self.c, &machine.c, delta)

    def extendable_interval(self, Machine machine):
        cdef int st = platform_extendable_interval_start(&self.c, &machine.c, self._total_limit)
        cdef int ft = platform_extendable_interval_finish(&self.c, &machine.c, self._total_limit)
        return st, ft

    @property
    def machines(self):
        return self._machines
