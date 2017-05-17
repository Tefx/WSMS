#!/usr/bin/env python

import array

from WSMS.workflow import read_workflow, read_VM_types, trans_dax_workflow
from WSMS.c.problem import Problem
from WSMS.c.schedule import Schedule
from WSMS.c.extra import sort_by_upward_ranks
from WSMS.utils import verify_schedule

workflows = [
    "CyberShake_30", "Epigenomics_24", "Inspiral_30", "Montage_25", "Sipht_30",
    "CyberShake_50", "Epigenomics_46", "Inspiral_50", "Montage_50", "Sipht_60",
    "CyberShake_100", "Epigenomics_100", "Inspiral_100", "Montage_100", "Sipht_100",
    "CyberShake_1000", "Epigenomics_997", "Inspiral_1000", "Montage_1000", "Sipht_1000",
    ]

if __name__ == '__main__':
    types = read_VM_types("ec2.typ")

    for wrk in workflows:
        tasks = read_workflow(wrk)
        problem = Problem(tasks, types, 20, 3600)

        placements = array.array("i", [0 for _ in range(problem.num_tasks())])
        vm_types = array.array("i", [4])
        order = sort_by_upward_ranks(problem)
        for _ in range(10 * 10000):
            schedule = Schedule(placements, vm_types)
            schedule.complete_1(problem, order)
        verify_schedule(problem, schedule)
