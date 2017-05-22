from WSMS.c.problem import Problem
from WSMS.c.platform import Platform, Machine
from WSMS.c.schedule import Schedule
from WSMS.c.common import Resources
from array import array

class Heuristic(objective):
    def __init__(self, problem):
        self.problem = problem
        self.platform = Platform()
        self.finish_times = array("i", [0] * problem->num_tasks())
        self.placements = array("i", [0] * problem->num_tasks())

    def earliest_start_time(self, task_id):
        return max(self.finish_time[t] for t in self.problem.task_prevs(task_id))

    def __call__(self):
        pass

