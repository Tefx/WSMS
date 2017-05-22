#!/usr/bin/env python

from WSMS.c.problem import Problem
from WSMS.c.schedule import Schedule
from WSMS.c.platform import Platform, Machine, Task
from WSMS.c.extra import sort_tasks_by_upward_ranks
from array import array

workflows = [
    "CyberShake_30", "Epigenomics_24", "Inspiral_30", "Montage_25", "Sipht_30",
    "CyberShake_50", "Epigenomics_46", "Inspiral_50", "Montage_50", "Sipht_60",
    "CyberShake_100", "Epigenomics_100", "Inspiral_100", "Montage_100", "Sipht_100",
    "CyberShake_1000", "Epigenomics_997", "Inspiral_1000", "Montage_1000", "Sipht_1000",
    ]

if __name__ == '__main__':
    for wrk in workflows:
        problem = Problem.load(wrk, "EC2", 20, 3600)

        for _ in range(10 * 10000):
            schedule = Schedule(problem)
            schedule.placements = array("i", [0]*problem.num_tasks)
            schedule.vm_types = array("i", [4])
            schedule.scheduling_order = sort_tasks_by_upward_ranks(problem)

        schedule.plot_utilization("core", wrk)
        print("{:16} {:36} Verified {}".format(wrk, str(schedule.objectives), schedule.verify()))

        machine = Machine(problem, 4)
        finish_times = [0] * problem.num_tasks
        for task_id in sort_tasks_by_upward_ranks(problem):
            est = max([finish_times[t] for t in problem.task_prevs(task_id)], default=0)
            task = Task(task_id)
            machine.earliest_position(task, est)
            finish_times[task_id] = machine.place_task(task)
        print(machine.open_time, machine.close_time, machine.cost)
