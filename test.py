#!/usr/bin/env python
from WSMS.c.problem import Problem
from WSMS.c.schedule import Schedule

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


def print_and_plot(schedule, wrk, alg):
    print(
        "[TEST {:<10}]:\t Makespan: {makespan:<8} Cost: {cost:<8.3f} PNVM: {:<8} Verified {}".
        format(alg, schedule.pnvm, schedule.verify(), **schedule.objectives))
    schedule.plot_utilization("core", "{}_by_{}".format(wrk, alg))


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


def test_heft(problem, wrk):
    from WSMS.algorithm.heuristic.heft import HEFT_uni_rt
    schedule = HEFT_uni_rt(problem)
    print_and_plot(schedule, wrk, "HEFT")


def test_heft_om(problem, wrk):
    from WSMS.algorithm.heuristic.MSC import HEFT_on_max
    schedule = HEFT_on_max(problem).schedule
    print_and_plot(schedule, wrk, "HEFT_om")


def test_msc_eft(problem, wrk):
    from WSMS.algorithm.heuristic.MSC import MSC_EFTC
    schedule = MSC_EFTC(problem).schedule
    print_and_plot(schedule, wrk, "MSC_EFT")


def test_msc_dl(problem, wrk, k_dl):
    from WSMS.algorithm.heuristic.MSC import MSC_DL
    deadline = MSC_DL.generate_deadline(problem, k_dl)
    schedule = MSC_DL(problem, deadline).schedule
    print_and_plot(schedule, wrk, "MSC_DL")


def test_ea(problem, wrk):
    from WSMS.algorithm.meta.evolutionary import ea_fastind_simple
    schedule = ea_fastind_simple(problem, problem.num_tasks * 20, 10)
    print_and_plot(schedule, wrk, "EA_SIMPLE")


if __name__ == '__main__':
    for wrk in workflows:
        print("Running on {}...".format(wrk))
        problem = Problem.load(wrk, "EC2", 20, 3600)
        test_heft_om(problem, wrk)
        # test_msc_eft(problem, wrk)
        test_msc_dl(problem, wrk, 0)
        test_ea(problem, wrk)
