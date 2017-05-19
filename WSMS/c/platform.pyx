from cpython cimport array
import array


cdef class Machine:
    def __cinit__(self, Problem problem):
        machine_init(&self.c)
        self._problem = &problem.c

    def __dealloc__(self):
        machine_free(&self.c)

    def __getattr__(self, attr):
        return getattr(self, "get_"+attr)()

    def __setattr__(self, attr, value):
        getattr(self, "set_"+attr)(value)

    def set_type_id(self, int type_id):
        self._type_id = type_id
        self._capacities = problem_type_capacities(self._problem, type_id)

    def get_open_time(Machine self):
        return machine_open_time(&self.c)

    def get_close_time(self):
        return machine_close_time(&self.c)

    def get_runtime(self):
        return machine_runtime(&self.c)

    def get_cost(self):
        cdef int runtime = machine_runtime(&self.c)
        return problem_charge(self._problem, self._type_id, runtime) 

    def get_capacities(self):
        return self._capacities

    def place(self, int task_id, int st):
        cdef int rt = problem_task_runtime(self._problem, task_id, self._type_id)
        cdef resources_t demands = problem_task_demands(self._problem, task_id)
        machine_place_task(&self.c, st, rt, &demands)

    def place_earliest(self, int task_id, int est):
        cdef int rt = problem_task_runtime(self._problem, task_id, self._type_id)
        cdef resources_t demands = problem_task_demands(self._problem, task_id)
        return machine_alloc_earliest(&self.c, est, rt, &demands, &self._capacities) + rt

    def earliest_slot(self, int task_id, int est):
        cdef int rt = problem_task_runtime(self._problem, task_id, self._type_id)
        cdef resources_t demands = problem_task_demands(self._problem, task_id)
        return machine_earliest_slot(&self.c, est, rt, &demands, &self._capacities)

    def shift(self, int delta):
        machine_shift(&self.c, delta)


cdef class Platform:
    cdef platform_t c
    cdef int total_limit

    def __cinit__(self, Problem problem):
        platform_init(&self.c)
        self._total_limit =  problem.c.total_limit

    def __dealloc__(self):
        platform_free(&self.c)

    def earliest_slot(self, int est, int rt, int total_limit):
        return platform_earliest_slot(&self.c, est, rt, total_limit)

    def extendable_interval(self, Machine machine):
        cdef int st, ft
        platform_extendable_interval(&self.c, &machine.c, &st, &ft, self._total_limit)
        return st, ft

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
