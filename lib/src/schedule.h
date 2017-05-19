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

void schedule_init(schedule_t* schedule, int num_tasks);
void schedule_free(schedule_t* schedule);

void schedule_set_placements(schedule_t* schedule, int* placements);
void schedule_set_vm_types(schedule_t* schedule, int* vm_types, int num_vms);
void schedule_set_start_times(schedule_t* schedule, int* start_times);
void schedule_set_finish_times(schedule_t* schedule, int* finish_times);
void schedule_set_objectives(schedule_t* schedule, int makespan, double cost);

void schedule_autofill_start_times(schedule_t* schedule, problem_t* problem);
void schedule_autofill_finish_times(schedule_t* schedule, problem_t* problem);

void schedule_autofill_1(schedule_t* schedule, problem_t* problem, int* order,
                         machine_t* vms);

#define PL(schedule, task_id) ((schedule)->placements[task_id])
#define TYP(schedule, type_id) ((schedule)->vm_types[type_id])
#define ST(scheduke, task_id) ((scheduke)->start_times[task_id])
#define FT(scheduke, task_id) ((scheduke)->finish_times[task_id])
#define TYPL(schedule, task_id) TYP(schedule, PL(schedule, task_id))

#endif  // ifndef WSMS_SCHEDULE_H
