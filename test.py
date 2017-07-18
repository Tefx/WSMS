#!/usr/bin/env python
from WSMS.c.problem import Problem
from WSMS.c.schedule import Schedule
from WSMS.c.platform import Platform, Machine, Task
from WSMS.c.extra import sort_tasks_by_upward_ranks
from WSMS.algorithm.heuristic.heft import HEFT_uni_rt
from array import array

workflows = [
    "CyberShake_30",
    "Epigenomics_24",
    "Inspiral_30",
    "Montage_25",
    "Sipht_30",
    "CyberShake_50",
    "Epigenomics_46",
    "Inspiral_50",
    "Montage_50",
    "Sipht_60",
    "CyberShake_100",
    "Epigenomics_100",
    "Inspiral_100",
    "Montage_100",
    "Sipht_100",
    "CyberShake_1000",
    "Epigenomics_997",
    "Inspiral_1000",
    "Montage_1000",
    "Sipht_1000",
]


def test_performance(problem, wrk):
    order = sort_tasks_by_upward_ranks(problem)
    for _ in range(10 * 10000):
        schedule = Schedule(problem, 1)
        schedule.placements = array("i", [0] * problem.num_tasks)
        schedule.vm_types = array("i", [4])
        schedule.start_order = order
        # schedule.scheduling_order = sort_tasks_by_upward_ranks(problem)

    print("OBJS: {:<56} PNVM: {:<8} Verified {}".format(
        str(schedule.objectives), schedule.pnvm, schedule.verify()))


def test_heft(problem, wrk):
    schedule = HEFT_uni_rt(problem)
    print("[TEST_HEFT]:\t OBJS: {:<56} PNVM: {:<8} Verified {}".format(
        str(schedule.objectives), schedule.pnvm, schedule.verify()))
    schedule.plot_utilization("core", "{}_by_HEFT".format(wrk))


def test_ind(problem, wrk):
    from WSMS.algorithm.meta.ind import FastInd

    mpool = FastInd.create_mpool(2000)
    schedule = HEFT_uni_rt(problem)
    for _ in range(10):
        ind = FastInd.from_schedule(problem, schedule, mpool)
        for _ in range(10000):
            ind = ind.clone()
            ind.mutate(problem)
            ind.evaluate(problem)
    print("[TEST IND]:\t", ind.makespan, ind.cost, ind.pnvm, schedule.verify())


def test_ea(problem, wrk):
    from WSMS.algorithm.meta.evolutionary import ea_fastind_simple
    schedule = ea_fastind_simple(problem, problem.num_tasks * 20, 10)
    print("[TEST EA]:\t OBJS: {:<56} PNVM: {:<8} Verified {}".format(
        str(schedule.objectives), schedule.pnvm, schedule.verify()))
    schedule.plot_utilization("core", "{}_by_EA".format(wrk))


if __name__ == '__main__':
    for wrk in workflows:
        print("Running on {}...".format(wrk))
        problem = Problem.load(wrk, "EC2", 20, 3600)
        # test_performance(problem, wrk)
        test_heft(problem, wrk)
        # test_ind(problem, wrk)
        test_ea(problem, wrk)
