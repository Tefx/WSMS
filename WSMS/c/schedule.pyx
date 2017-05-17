from libc.stdlib cimport malloc, free
from cpython cimport array
import array

import matplotlib as mpl; mpl.use("Agg")
import matplotlib.pyplot as plt
import os.path

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

    def finish_time(self, int task_id):
        return self.c.finish_times[task_id]

    def vms(self):
        return range(self.c.num_vms)

    def utilization(self, Problem problem, int vm_id):
        _tasks = [t for t in problem.tasks() if self.PL(t) == vm_id]
        tid_by_st = sorted(_tasks, key=self.ST)
        tid_by_ft = sorted(_tasks, key=self.finish_time)
        cur_usage = Resources.zero()
        f_i = 0
        usages = [[0, Resources.zero()]]

        for tid in tid_by_st:
            time = self.finish_time(tid_by_ft[f_i])
            while time <= self.ST(tid):
                cur_usage -= problem.task_demands(tid_by_ft[f_i])
                _append_usage(usages, time, cur_usage)
                f_i += 1
                time = self.finish_time(tid_by_ft[f_i])
            cur_usage += problem.task_demands(tid)
            time = self.ST(tid)
            _append_usage(usages, time, cur_usage)

        while f_i < len(tid_by_ft):
            time = self.finish_time(tid_by_ft[f_i])
            cur_usage -= problem.task_demands(tid_by_ft[f_i])
            _append_usage(usages, time, cur_usage)
            f_i += 1

        return usages

    def verify_vm(self, Problem problem, int vm_id):
        capacities = problem.type_capacities(self.TYP(vm_id))
        for usage in self.utilization(problem, vm_id):
            if not usage[1] <= capacities:
                return False
        return True

    def verify(self, Problem problem):
        return all(self.verify_vm(problem, x) for x in self.vms())

    def plot_utilization(self, problem, res, name, path="results"):
        num_vms = len(self.vms())
        fig, axes = plt.subplots(num_vms, sharex=True, figsize=(20, 3 * num_vms))

        if num_vms == 1:
            axes = [axes]

        for ax, vm in zip(axes, self.vms()):
            usages = self.utilization(problem, vm)
            capacities = problem.type_capacities(self.TYP(vm))
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
