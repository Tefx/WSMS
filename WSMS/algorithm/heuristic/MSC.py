from WSMS.c.platform import Platform, Machine, Task
from WSMS.c.schedule import Schedule
from WSMS.c.common import Resources
from WSMS.c.extra import sort_tasks_by_upward_ranks
from math import inf


class MSC(object):
    def __init__(self, problem):
        self.problem = problem
        self.platform = None

    def fitness_default(self):
        return inf

    def fitness(self, task, machine, type_id):
        raise NotImplementedError

    def fitness_better_than(self, fitness_0, fitness_1):
        return fitness_0 < fitness_1

    def sort_tasks(self):
        raise NotImplementedError

    def candidate_types(self, min_demands, max_demands):
        upper_types = []
        for type_id in self.problem.types:
            capacities = self.problem.type_capacities(type_id)
            if min_demands <= capacities:
                if max_demands <= capacities:
                    upper_types.append((type_id, capacities))
                else:
                    yield type_id
        for type_id, capacities in upper_types:
            if not any(tmp_cap < capacities for _, tmp_cap in upper_types):
                yield type_id

    def candidate_types_for_existing_vms(self, machine, demands):
        peak_usage = machine.peak_usage
        yield from self.candidate_types(
            Resources.max([peak_usage, demands]), peak_usage + demands)

    def candidate_types_for_new_vm(self, demands):
        yield from self.candidate_types(demands, demands)

    def best_placement_on_existing_vms(self, task_id, est):
        demands = self.problem.task_demands(task_id)
        placement_bst, fitness_bst = None, self.fitness_default()
        for machine in self.platform.machines:
            ei = self.platform.extendable_interval(machine)
            for type_id in self.candidate_types_for_existing_vms(
                    machine, demands):
                task = Task(task_id)
                task.start_time = machine.earliest_position(
                    task, max(ei[0], est), type_id)
                if task.finish_time <= ei[1]:
                    fitness = self.fitness(task, machine, type_id)
                    if self.fitness_better_than(fitness, fitness_bst):
                        placement_bst, fitness_bst = (task, machine,
                                                      type_id), fitness
        return placement_bst, fitness_bst

    def best_placement_on_new_vm(self, task_id, est):
        demands = self.problem.task_demands(task_id)
        placement_bst, fitness_bst = None, self.fitness_default()
        for type_id in self.candidate_types_for_new_vm(demands):
            task = Task(task_id)
            machine = Machine.new_for_task(self.problem, task, type_id)
            task.start_time = self.platform.earliest_position(machine, est)
            fitness = self.fitness(task, machine, type_id)
            if self.fitness_better_than(fitness, fitness_bst):
                placement_bst, fitness_bst = (task, machine, type_id), fitness
        return placement_bst, fitness_bst

    def place(self, placement):
        task_bst, machine_bst, type_bst = placement
        machine_bst.type_id = type_bst
        machine_bst.place(task_bst)
        self.platform.update(machine_bst)
        self.tasks[task_bst.task_id] = task_bst

    def framework(self):
        self.platform = Platform(self.problem)
        self.tasks = [None for _ in self.problem.tasks]
        for task_id in self.sort_tasks():
            est = max(
                [
                    self.tasks[prev_id].finish_time
                    for prev_id in self.problem.task_prevs(task_id)
                ],
                default=0)
            placement_exg, fitness_exg = self.best_placement_on_existing_vms(
                task_id, est)
            placement_new, fitness_new = self.best_placement_on_new_vm(
                task_id, est)
            if self.fitness_better_than(fitness_new, fitness_exg):
                self.place(placement_new)
            else:
                self.place(placement_exg)

    def solve(self):
        self.framework()

    @property
    def schedule(self):
        if not self.platform:
            self.solve()
        return Schedule.from_platform(self.problem, self.platform)


class HEFT_on_max(MSC):
    def __init__(self, problem):
        super(HEFT_on_max, self).__init__(problem)
        self.find_max_types()

    def find_max_types(self):
        types = [(type_id, self.problem.type_capacities(type_id))
                 for type_id in self.problem.types]
        self.max_types = []
        for type_id, capacities in types:
            if not any(capacities < other_capacities
                       for _, other_capacities in types):
                self.max_types.append(type_id)

    def candidate_types(self, min_demands, max_demands):
        return [
            type_id for type_id in self.max_types
            if min_demands <= self.problem.type_capacities(type_id)
        ]

    def fitness(self, task, machine, type_id):
        return task.finish_time

    def sort_tasks(self):
        return sort_tasks_by_upward_ranks(self.problem)

    def solve(self):
        super().solve()
        for machine in self.platform.machines:
            machine.tight_type()


class MSC_EFTC(MSC):
    def fitness_default(self):
        return inf, inf

    def fitness(self, task, machine, type_id):
        return task.finish_time, machine.cost_increase(task, type_id)

    def sort_tasks(self):
        return sort_tasks_by_upward_ranks(self.problem)


class MSC_DL(MSC):
    @staticmethod
    def generate_deadline(problem, k_dl):
        cheapest_type = problem.cheapest_type()
        min_span = HEFT_on_max(problem).schedule.makespan
        max_span = sum(
            problem.task_runtime(task_id, cheapest_type)
            for task_id in problem.tasks)
        return min_span + (max_span - min_span) * k_dl

    def __init__(self, problem, deadline):
        super().__init__(problem)
        self.deadline = deadline
        self.calculate_latest_start_times()

    def calculate_latest_start_times(self):
        self.problem.reverse_dag()
        schedule = HEFT_on_max(self.problem).schedule
        self.latest_start_times = [
            self.deadline - schedule.FT(t) for t in self.problem.tasks
        ]
        self.problem.reverse_dag()

    def fitness_default(self):
        return inf, inf

    def fitness(self, task, machine, type_id):
        return task.start_time - self.latest_start_times[task.task_id], \
                machine.cost_increase(task, type_id)

    def fitness_better_than(self, fitness_0, fitness_1):
        dt_0, dc_0 = fitness_0
        dt_1, dc_1 = fitness_1
        if dt_0 <= 0 and dt_1 <= 0:
            return (dc_0, dt_0) <= (dc_1, dt_1)
        return (dt_0, dc_0) <= (dt_1, dc_1)

    def sort_tasks(self):
        return sorted(self.problem.tasks, key=self.latest_start_times.__getitem__)

    def solve(self):
        super().solve()
        for machine in self.platform.machines:
            machine.tight_type()
            machine.tight_span(self.tasks)
