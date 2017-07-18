from WSMS.algorithm.meta.ind import FastInd
from WSMS.algorithm.heuristic.heft import HEFT_uni_rt
from WSMS.c.extra import sort_tasks_by_upward_ranks
from WSMS.c.schedule import Schedule
from WSMS.c.common import Resources

from array import array
from random import randint


def get_random_schedule(problem):
    min_demands = Resources.zero()
    for task_id in problem.tasks:
        min_demands.imax(problem.task_demands(task_id))
    min_type = problem.cheapest_type_for_demands(min_demands)
    schedule = Schedule(problem, 1)
    schedule.placements = array("i", [0] * problem.num_tasks)
    schedule.vm_types = array("i", [randint(min_type, problem.num_types - 1)])
    schedule.scheduling_order = sort_tasks_by_upward_ranks(problem)
    return schedule


def ea_fastind_simple(problem, num_gen, num_pop):
    mpool = FastInd.create_mpool(problem.num_tasks * num_pop * 2)
    pop = [
        FastInd.from_schedule(problem, get_random_schedule(problem), mpool)
        for _ in range(num_pop)
    ]
    for gen in range(num_gen):
        offsprings = [ind.clone() for ind in pop]
        for ind in offsprings:
            ind.mutate(problem)
            ind.evaluate(problem)
        pop += offsprings
        valid = [ind for ind in pop if ind.pnvm <= problem.total_limit]
        valid.sort(key=lambda x: (x.makespan, x.cost, x.pnvm))
        non_valid = [ind for ind in pop if ind.pnvm > problem.total_limit]
        non_valid.sort(key=lambda x: (x.pnvm, x.makespan, x.cost))
        pop = (valid + non_valid)[:num_pop]
    return pop[0].to_schedule(problem)
