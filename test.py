#!/usr/bin/env python

import array

from WSMS.workflow import read_dax, read_VM_types
from WSMS.c.problem import Problem
from WSMS.c.schedule import Schedule
from WSMS.c.extra import sort_by_upward_ranks
from WSMS.utils import verify_schedule

if __name__ == '__main__':
    tasks = read_dax("Montage_1000")
    vmtypes = read_VM_types("ec2_types.json")
    problem = Problem(tasks, vmtypes, 20, 3600)
    # for i in range(problem.num_types()):
        # print(i, problem.type_capacities(i))
    placements = array.array("i", [0 for _ in range(problem.num_tasks())])
    vm_types = array.array("i", [4])
    order = sort_by_upward_ranks(problem)
    # print("Order:", order)
    # print("Placements:", placements)
    # print("VM Types:", vm_types)
    for _ in range(20 * 10000):
        schedule = Schedule(placements, vm_types)
        schedule.complete_1(problem, order)

    schedule = Schedule(placements, vm_types)
    schedule.complete_1(problem, order)
    verify_schedule(problem, schedule)
