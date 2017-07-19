from WSMS.c.extra import sort_tasks_by_upward_ranks, earliest_start_time
from WSMS.c.platform import Platform, Machine, Task
from WSMS.c.schedule import Schedule
from array import array


def HEFT_uni_rt(problem):
    def best_placements(best_pl, task, machine, type_id):
        best_task, best_machine, best_type, best_ci = best_pl
        ci = machine.cost_increase(task, type_id)
        if (not best_task) \
           or (task.finish_time < best_task.finish_time) :
            return task, machine, type_id, ci
        else:
            return best_pl

    platform = Platform(problem)
    finish_times = array("i", [0 for _ in problem.tasks])

    for task_id in sort_tasks_by_upward_ranks(problem):
        best_pl = None, None, None, float("inf")
        est = earliest_start_time(problem, task_id, finish_times)
        avail_types = list(
            problem.valid_types_for_demands(problem.task_demands(task_id)))

        for machine in platform.machines:
            for type_id in [t for t in avail_types if t >= machine.type_id]:
                task = Task(task_id)
                task.start_time = machine.earliest_position(task, est, type_id)
                best_pl = best_placements(best_pl, task, machine, type_id)

        task = Task(task_id)
        type_id = problem.cheapest_type_for_demands(problem.task_demands(task_id))
        machine = Machine.new_for_task(problem, task, type_id)
        task.start_time = platform.earliest_position(machine, est)
        best_pl = best_placements(best_pl, task, machine, machine.type_id)

        best_task, best_machine, best_type, _ = best_pl
        best_machine.type_id = best_type
        finish_times[task_id] = best_machine.place(best_task)
        platform.update(best_machine)

    return Schedule.from_platform(problem, platform)
