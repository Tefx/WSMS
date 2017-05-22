#ifndef WSMS_PLATFORM_H
#define WSMS_PLATFORM_H

#include "bin.h"
#include "common.h"
#include "problem.h"

typedef item_t *task_t;
#define task_item(task) (*(task))
void task_init(task_t *task);
void task_destory(task_t *task);
void task_set(task_t *task, int length, res_t demands);
#define task_start_time(task) (task_item(task)->start_time)
#define task_finish_time(task) (task_start_time(task) + task_item(task)->length)

typedef bin_t *machine_t;
#define machine_bin(machine) (*(machine))
#define machine_item(machine) ((item_t *)((bin_t *)(*(machine)) + 1))
void machine_init(machine_t *machine);
void machine_destory(machine_t *machine);
void machine_set(machine_t *machine, int demands);

typedef bin_t *platform_t;
#define platform_bin(platform) (*(platform))
void platform_init(platform_t *platform);
void platform_destory(platform_t *platform);

// Machine related
#define machine_open_time(machine) bin_open_time(machine_bin(machine))
#define machine_close_time(machine) bin_close_time(machine_bin(machine))
#define machine_runtime(machine) bin_span(machine_bin(machine))

int machine_earliest_position(machine_t *machine, task_t *task, int est,
                              res_t capacities);
int machine_place_task(machine_t *machine, task_t *task);
void machine_shift_task(machine_t *machine, task_t *task, int delta);
int machine_extendable_interval_start(machine_t *machine, task_t *task,
                                      res_t capacities);
int machine_extendable_interval_finish(machine_t *machine, task_t *task,
                                       res_t capacities);

int platform_earliest_position(platform_t *platform, machine_t *machine,
                               int est, int total_limit);
int platform_place_machine(platform_t *platform, machine_t *machine);
void platform_extend_machine(platform_t *platform, machine_t *machine);
void platform_shift_machine(platform_t *platform, machine_t *machine,
                            int delta);
int platform_extendable_interval_start(platform_t *platform, machine_t *machine,
                                       int total_limit);
int platform_extendable_interval_finish(platform_t *platform,
                                        machine_t *machine, int total_limit);

#endif
