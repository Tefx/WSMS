#!/usr/bin/env python

import array

from WSMS.c.problem import Problem
from WSMS.c.schedule import Schedule
from WSMS.c.extra import sort_by_upward_ranks

workflows = [
    "CyberShake_30", "Epigenomics_24", "Inspiral_30", "Montage_25", "Sipht_30",
    "CyberShake_50", "Epigenomics_46", "Inspiral_50", "Montage_50", "Sipht_60",
    "CyberShake_100", "Epigenomics_100", "Inspiral_100", "Montage_100", "Sipht_100",
    "CyberShake_1000", "Epigenomics_997", "Inspiral_1000", "Montage_1000", "Sipht_1000",
    ]

if __name__ == '__main__':
    for wrk in workflows:
        problem = Problem.load(wrk, "EC2", 20, 3600)
        placements = array.array("i", [0]*problem.num_tasks())
        vm_types = array.array("i", [2])
        order = sort_by_upward_ranks(problem)
        schedule = Schedule(placements, vm_types)
        schedule.complete_1(problem, order)
        print("Verifying schedule on {}: {}".format(wrk, schedule.verify(problem)))
        schedule.plot_utilization(problem, "core", wrk)
