from WSMS.c.mpool cimport MemPool, mempool_t
from WSMS.c.problem cimport Problem, problem_t
from WSMS.c.schedule cimport Schedule, schedule_t, objectives_t, schedule_init, schedule_destory, schedule_simulate, schedule_calculate_pnvm, schedule_calculate_objectives

from random import randint, random, getrandbits, choice

cdef extern from "ind_fast.c":
    struct _vm_t:
        int id
        int type

    struct fast_ind_t:
        _vm_t** pls
        _vm_t** vms
        int* order
        int num_vms
        int num_tasks

    mempool_t* fi_create_mpool(int buffer_size)
    void fi_free_mpool(mempool_t* pool)

    fast_ind_t* fi_malloc(int num_tasks)
    fast_ind_t* fi_clone(fast_ind_t* orig)
    void fi_free(fast_ind_t* ind, mempool_t* mpool)
    fast_ind_t* fi_from_schedule(schedule_t* schedule, mempool_t* mpool)
    void fi_to_schedule(fast_ind_t* ind, schedule_t* schedule)

    _vm_t* fi_add_vm(fast_ind_t* ind, int vm_type, mempool_t* pool)
    void fi_del_vm(fast_ind_t* ind, int index, mempool_t* pool)
    void fi_move_tasks(fast_ind_t* ind, _vm_t* src, _vm_t* dest)
    void fi_change_type(fast_ind_t* ind, int index, int new_type, mempool_t* pool)
    void fi_swap_order(fast_ind_t* ind, problem_t* problem, int index)
    int fi_minimum_type_for_mutation(fast_ind_t* ind, problem_t* problem, int index)


cdef class FastInd:
    cdef fast_ind_t* c_ptr
    cdef MemPool pool
    cdef public int makespan
    cdef public double cost
    cdef public int pnvm

    def __dealloc__(self):
        fi_free(self.c_ptr, self.pool.c_ptr)

    def clone(self):
        cdef FastInd ind = FastInd.__new__(FastInd)
        ind.pool = self.pool
        ind.c_ptr = fi_clone(self.c_ptr)
        return ind

    @staticmethod
    def create_mpool(int buffer_size):
        cdef MemPool pool = MemPool()
        pool.c_ptr = fi_create_mpool(buffer_size)
        return pool

    @staticmethod
    def from_schedule(Problem problem, Schedule schedule, MemPool pool):
        cdef FastInd ind = FastInd.__new__(FastInd)
        ind.pool = pool
        ind.c_ptr = fi_from_schedule(&schedule.c, pool.c_ptr)
        ind.evaluate(problem)
        return ind

    def to_schedule(self, Problem problem):
        cdef Schedule schedule = Schedule(problem, self.c_ptr.num_vms)
        fi_to_schedule(self.c_ptr, &schedule.c)
        schedule_simulate(&schedule.c, &problem.c, self.c_ptr.order, True)
        schedule_calculate_objectives(&schedule.c, &problem.c)
        schedule_calculate_pnvm(&schedule.c, &problem.c)
        return schedule

    def evaluate(self, Problem problem):
        cdef schedule_t schedule;
        schedule_init(&schedule, self.c_ptr.num_tasks, self.c_ptr.num_vms)
        fi_to_schedule(self.c_ptr, &schedule)
        schedule_simulate(&schedule, &problem.c, self.c_ptr.order, True)
        schedule_calculate_objectives(&schedule, &problem.c)
        schedule_calculate_pnvm(&schedule, &problem.c)
        self.makespan = schedule.objectives.makespan
        self.cost = schedule.objectives.cost
        self.pnvm = schedule.pnvm
        schedule_destory(&schedule)

    cdef int rand_vm_index(self):
        return randint(0, self.c_ptr.num_vms-1)

    cdef int rand_task_index(self):
        return randint(0, self.c_ptr.num_tasks-1)

    cdef void split(self):
        cdef int vm_index = self.rand_vm_index()
        cdef _vm_t* orig_vm = self.c_ptr.vms[vm_index]

        cdef int split_point = self.rand_task_index()
        cdef int num_remain = 0;
        cdef int num_move = 0;
        for i in range(self.c_ptr.num_tasks):
            if self.c_ptr.pls[self.c_ptr.order[i]] == orig_vm:
                if i <= split_point:
                    num_remain += 1
                else:
                    num_move += 1
        if not num_remain or not num_move:
            return

        cdef _vm_t* new_vm = fi_add_vm(self.c_ptr, orig_vm.type, self.pool.c_ptr)
        cdef int task_id
        for i in range(split_point, self.c_ptr.num_tasks):
            task_id = self.c_ptr.order[i]
            if self.c_ptr.pls[task_id] == orig_vm:
                self.c_ptr.pls[task_id] = new_vm

    cdef void merge(self):
        cdef int vm_index = self.rand_vm_index()
        cdef int target_index = self.rand_vm_index()
        while target_index == vm_index:
            target_index = self.rand_vm_index()
        fi_move_tasks(self.c_ptr, self.c_ptr.vms[vm_index], 
                      self.c_ptr.vms[target_index])
        fi_del_vm(self.c_ptr, vm_index, self.pool.c_ptr)

    cdef void mutate_order(self, Problem problem):
        cdef double prob = 1.0 / self.c_ptr.num_tasks
        for i in range(self.c_ptr.num_tasks-1):
            if random() <= prob:
                fi_swap_order(self.c_ptr, &problem.c, i)

    cdef void mutate_type(self, Problem problem):
        # cdef int index = self.rand_vm_index()
        cdef double prob = 1.0 / self.c_ptr.num_tasks
        cdef int min_type, new_type
        for index in range(self.c_ptr.num_vms):
            if random() <= prob:
                min_type = fi_minimum_type_for_mutation(self.c_ptr, &problem.c, index)
                new_type = randint(min_type, problem.c.num_types-1)
                fi_change_type(self.c_ptr, index, new_type, self.pool.c_ptr)

    cdef void mutate_type_neighbor(self, Problem problem):
        cdef double prob = 1.0 / self.c_ptr.num_tasks
        for index in range(self.c_ptr.num_vms):
            if random() <= prob:
                vm_type = self.c_ptr.vms[index].type
                if vm_type == problem.num_types-1:
                    vm_type -= 1
                elif vm_type == fi_minimum_type_for_mutation(self.c_ptr, &problem.c, index):
                    vm_type += 1
                elif getrandbits(1):
                    vm_type -= 1
                else:
                    vm_type += 1
                fi_change_type(self.c_ptr, index, vm_type, self.pool.c_ptr)

    def mutate(self, Problem problem):
        if self.c_ptr.num_vms == 1:
            self.split()
        elif self.c_ptr.num_vms == self.c_ptr.num_tasks:
            self.merge()
        else:
            if getrandbits(1):
                self.split()
            else:
                self.merge()
        self.mutate_order(problem)
        self.mutate_type(problem)
