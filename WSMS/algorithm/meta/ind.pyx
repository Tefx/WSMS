from WSMS.c.mpool cimport MemPool, mempool_t
from WSMS.c.problem cimport Problem, problem_t
from WSMS.c.schedule cimport Schedule, schedule_t, schedule_simulate

from random import randint

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


cdef class FastInd:
    cdef fast_ind_t* c_ptr
    cdef MemPool pool

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
    def from_schedule(Schedule schedule, MemPool pool):
        cdef FastInd ind = FastInd.__new__(FastInd)
        ind.pool = pool
        ind.c_ptr = fi_from_schedule(&schedule.c, pool.c_ptr)
        return ind

    def to_schedule(self, Problem problem):
        cdef Schedule schedule = Schedule(problem, self.c_ptr.num_vms)
        fi_to_schedule(self.c_ptr, &schedule.c)
        schedule_simulate(&schedule.c, &problem.c, self.c_ptr.order, True)
        return schedule

    cdef rand_vm_index(self):
        return randint(0, self.c_ptr.num_vms-1)

    cdef rand_task_index(self):
        return randint(0, self.c_ptr.num_tasks-1)

    cdef split(self):
        cdef int vm_index = self.rand_vm_index()
        cdef int split_point = self.rand_task_index()
        cdef _vm_t* orig_vm = self.c_ptr.vms[vm_index]
        cdef _vm_t* new_vm = fi_add_vm(self.c_ptr, orig_vm.type, self.pool.c_ptr)
        for i in range(split_point, self.c_ptr.num_tasks):
            if self.c_ptr.pls[i] == orig_vm:
                self.c_ptr.pls[i] = new_vm

    cdef merge(self):
        cdef int vm_index = self.rand_vm_index()
        cdef int target_index = self.rand_vm_index()
        while target_index == vm_index:
            target_index = self.rand_vm_index()
        fi_move_tasks(self.c_ptr, self.c_ptr.vms[vm_index], 
                      self.c_ptr.vms[target_index])
        fi_del_vm(self.c_ptr, vm_index, self.pool.c_ptr)

    def mutate(self):
        if self.c_ptr.num_vms == 1:
            self.split()
        elif self.c_ptr.num_vms == self.c_ptr.num_tasks:
            self.merge()
        else:
            if randint(0,1) == 1:
                self.split()
            else:
                self.merge()
