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

    int* placements;
    int* start_times;
    int* vm_types;

    objectives_t objectives;
    int pnvm;

    int* _finish_times;
    int* _vm_open_times;
    int* _vm_close_times;
} schedule_t;

void schedule_init(schedule_t* schedule, int num_tasks);
void schedule_free(schedule_t* schedule);
void schedule_set_placements(schedule_t* schedule, int* placements);
void schedule_set_vm_types(schedule_t* schedule, int* vm_types, int num_vms);
void schedule_set_start_times(schedule_t* schedule, int* start_times);

void schedule_simulate(schedule_t* schedule, problem_t* problem, int* order,
                       bool forward);

void schedule_calculate_objectives(schedule_t* schedule, problem_t* problem);
int schedule_calculate_pnvm(schedule_t* schedule, problem_t* problem);

#define PL(schedule, task_id) ((schedule)->placements[task_id])
#define TYP(schedule, type_id) ((schedule)->vm_types[type_id])
#define ST(scheduke, task_id) ((scheduke)->start_times[task_id])
#define FT(scheduke, task_id) ((scheduke)->_finish_times[task_id])
#define TYPL(schedule, task_id) TYP(schedule, PL(schedule, task_id))

#define schedule_objectives(schedule) ((schedule)->objectives)
#define schedule_pnvm(schedule) ((schedule)->pnvm)

#endif  // ifndef WSMS_SCHEDULE_H
