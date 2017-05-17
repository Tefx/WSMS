from WSMS.c.common import Resources

def vm_usages(problem, tasks, start_times, finish_times):
    tid_by_st = sorted(tasks, key=start_times.__getitem__)
    tid_by_ft = sorted(tasks, key=finish_times.__getitem__)
    cur_time = 0
    cur_usage = Resources.zero()

    j = 0
    for tid in tid_by_st:
        while finish_times[tid_by_ft[j]] <= start_times[tid]:
            cur_usage -= problem.task_demands(tid_by_ft[j])
            j += 1
        cur_usage += problem.task_demands(tid)
        yield cur_usage


def verify_schedule(problem, schedule):
    tasks_on_vms = [[] for _ in schedule.vms()]
    start_times = [schedule.ST(tid) for tid in problem.tasks()]
    finish_times = [
        schedule.finish_time(problem, tid) for tid in problem.tasks()
    ]
    for tid in problem.tasks():
        tasks_on_vms[schedule.PL(tid)].append(tid)

    for vid in schedule.vms():
        capacities = problem.type_capacities(schedule.TYP(vid))
        usages = vm_usages(problem, tasks_on_vms[vid], start_times,
                                  finish_times)
        peak_usage = Resources.zero()
        for usage in usages:
            peak_usage.scale(usage)
        print("Peak usage of {}: {} [{}]".format(vid, peak_usage, peak_usage <=
                                                 capacities))
