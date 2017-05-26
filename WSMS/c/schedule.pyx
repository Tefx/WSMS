from libc.stdlib cimport malloc, free
from cpython cimport array
import array
import matplotlib as mpl; mpl.use("Agg")
import matplotlib.pyplot as plt
import os.path


cdef class Schedule:
    cdef schedule_t c
    cdef Problem problem
    cdef bool _calculated
    cdef mempool_t* _mpool

    def __cinit__(self, Problem problem):
        self.problem = problem
        schedule_init(&self.c, problem.num_tasks)
        self._calculated = False
        self._mpool = NULL

    def __dealloc__(self):
        schedule_free(&self.c)

    @classmethod
    def from_platform(cls, Problem problem, Platform platform):
        placements = array.array("i", [0] * problem.num_tasks)
        start_times = array.array("i", [0] * problem.num_tasks)
        vm_types = array.array("i")
        for m_id, machine in enumerate(platform.machines):
            vm_types.append(machine.type_id)
            for task in machine.tasks:
                placements[task.task_id] = m_id
                start_times[task.task_id] = task.start_time
        schedule = cls(problem)
        schedule.placements = placements
        schedule.vm_types = vm_types
        schedule.start_times = start_times
        return schedule

    def __setattr__(self, attr, value):
        getattr(self, "set_"+attr)(value)

    def set_placements(self, array.array placements):
        schedule_set_placements(&self.c, placements.data.as_ints)

    def set_vm_types(self, array.array vm_types):
        schedule_set_vm_types(&self.c, vm_types.data.as_ints, len(vm_types))

    def set_start_times(self, array.array start_times):
        schedule_set_start_times(&self.c, start_times.data.as_ints)

    def set_mempool(self, MemPool mpool):
        self._mpool = mpool.c_ptr

    def set_scheduling_order(self, array.array order):
        schedule_simulate(&self.c, &self.problem.c, order.data.as_ints, False, self._mpool)

    def set_start_order(self, array.array order):
        schedule_simulate(&self.c, &self.problem.c, order.data.as_ints, True, self._mpool)

    def PL(self, int task_id):
        return PL(&self.c, task_id)

    def TYP(self, int vm_id):
        return TYP(&self.c, vm_id)

    def TYPL(self, int task_id):
        return TYPL(&self.c, task_id)

    def ST(self, int task_id):
        return ST(&self.c, task_id)

    def FT(self, int task_id):
        return FT(&self.c, task_id)

    @property
    def objectives(self):
        if not self._calculated:
            schedule_calculate_objectives(&self.c, &self.problem.c)
            schedule_calculate_pnvm(&self.c, &self.problem.c)
            self._calculated = True
        return schedule_objectives(&self.c)

    @property
    def pnvm(self):
        if not self._calculated:
            schedule_calculate_objectives(&self.c, &self.problem.c)
            schedule_calculate_pnvm(&self.c, &self.problem.c)
            self._calculated = True
        return schedule_pnvm(&self.c)

    @property
    def vms(self):
        return range(self.c.num_vms)

    @property
    def num_vms(self):
        return self.c.num_vms

    def utilization(self, int vm_id):
        if not self._calculated:
            schedule_calculate_objectives(&self.c, &self.problem.c)
            schedule_calculate_pnvm(&self.c, &self.problem.c)
            self._calculated = True
        _tasks = [t for t in self.problem.tasks if self.PL(t) == vm_id]
        tid_by_st = sorted(_tasks, key=self.ST)
        tid_by_ft = sorted(_tasks, key=self.FT)
        cur_usage = Resources.zero()
        f_i = 0
        usages = [[0, Resources.zero()]]

        for tid in tid_by_st:
            time = self.FT(tid_by_ft[f_i])
            while time <= self.ST(tid):
                cur_usage -= self.problem.task_demands(tid_by_ft[f_i])
                _append_usage(usages, time, cur_usage)
                f_i += 1
                time = self.FT(tid_by_ft[f_i])
            cur_usage += self.problem.task_demands(tid)
            time = self.ST(tid)
            _append_usage(usages, time, cur_usage)

        while f_i < len(tid_by_ft):
            time = self.FT(tid_by_ft[f_i])
            cur_usage -= self.problem.task_demands(tid_by_ft[f_i])
            _append_usage(usages, time, cur_usage)
            f_i += 1

        return usages

    def verify(self):
        for vm in self.vms:
            capacities = self.problem.type_capacities(self.TYP(vm))
            for usage in self.utilization(vm):
                if not usage[1] <= capacities:
                    return False
        return True

    def plot_utilization(self, res, name, path="results"):
        num_vms = self.c.num_vms
        fig, axes = plt.subplots(num_vms, sharex=True, figsize=(20, 3 * num_vms))

        if num_vms == 1:
            axes = [axes]

        for ax, vm in zip(axes, self.vms):
            usages = self.utilization(vm)
            capacities = self.problem.type_capacities(self.TYP(vm))
            times = []
            points = []
            for time, usage in usages:
                points.extend([usage[res]]*2)
                times.extend([time]*2)
            times = times[1:-1]
            points = points[:-2]
            ax.grid(True)
            ax.fill_between(times, points, 0, facecolor="green")
            ax.set_ylim(0, capacities[res])
            ax.set_ylabel(res)

        axes[-1].set_xlabel("Times (s)")
        fig.tight_layout()
        plt.savefig(os.path.join(path, "{}.{}.png".format(name, res)))


def _append_usage(usages, time, usage):
    if usages[-1][0] == time:
        usages[-1][1] = usage.copy()
    elif usages[-1][1] != usage:
        usages.append([time, usage.copy()])
    if len(usages) > 1 and usages[-1][1] == usages[-2][1]:
        del usages[-1]

