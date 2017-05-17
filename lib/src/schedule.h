#ifndef WSMS_SCHEDULE_H
#define WSMS_SCHEDULE_H

#include "platform.h"
#include "problem.h"

typedef struct objectives_t {
    int makespan;
    double cost;
} objectives_t;

typedef struct schedule_t {
    int num_tasks;
    int num_vms;
    int* start_times;
    int* finish_times;
    int* placements;
    int* vm_types;
    objectives_t objectives;
} schedule_t;

void schedule_init(schedule_t* schedule, int* placements, int* vm_types,
                   int num_tasks, int num_vms);
void schedule_free(schedule_t* schedule);
void schedule_complete_1(problem_t* problem, schedule_t* schedule,
                         int* order, machine_t* vms);

#endif  // ifndef WSMS_SCHEDULE_H
