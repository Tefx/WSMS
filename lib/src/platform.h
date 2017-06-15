#ifndef WSMS_PLATFORM_H
#define WSMS_PLATFORM_H

#include "bin.h"
#include "common.h"
#include "problem.h"

typedef item_t task_t;
#define task_item(task) (task)

inline void task_prepare(task_t *task, problem_t *problem, int task_id,
                         int type_id) {
    task_t *item = task_item(task);
    item->start_node = item->finish_node = NULL;
    item->length = problem_task_runtime(problem, task_id, type_id);
    item->demands = problem_task_demands(problem, task_id);
}

#define task_set_start_time(task, st) task_item(task)->start_time = st
#define task_start_time(task) (task_item(task)->start_time)
#define task_finish_time(task) (task_start_time(task) + task_item(task)->length)

typedef struct machine_t {
    bin_t bin;
    item_t item;
} machine_t;

/*#define machine_create_mpool(buffer_size) bnode_create_mpool(RES_DIM,
 * buffer_size)*/

#define machine_bin(machine) (&(machine)->bin)
#define machine_item(machine) (&(machine)->item)
void machine_init_external_pool(machine_t* machine, mempool_t* pool);
void machine_init(machine_t *machine, int num_tasks);
void machine_destory(machine_t *machine);
#define machine_set_demands(machine, dmnd) \
    machine_item(machine)->demands = (dmnd)
#define machine_set_runtime(machine, rt) machine_item(machine)->length = (rt)
#define machine_print(machine) bin_print(machine_bin(machine))

typedef bin_t platform_t;
#define platform_bin(platform) (platform)
void platform_init(platform_t *platform, int limit);
void platform_destory(platform_t *platform);
#define platform_print(platform) bin_print(platform_bin(platform))

#define machine_open_time(machine) bin_open_time(machine_bin(machine))
#define machine_close_time(machine) bin_close_time(machine_bin(machine))
#define machine_runtime(machine) bin_span(machine_bin(machine))

int machine_earliest_position(machine_t *machine, task_t *task, int est,
                              res_t capacities);
int machine_earliest_position_forward(machine_t *machine, task_t *task, int est,
                                      res_t capacities);
int machine_place_task(machine_t *machine, task_t *task);
void machine_shift_task(machine_t *machine, task_t *task, int delta);
int machine_extendable_interval_start(machine_t *machine, task_t *task,
                                      res_t capacities);
int machine_extendable_interval_finish(machine_t *machine, task_t *task,
                                       res_t capacities);

int platform_earliest_position(platform_t *platform, machine_t *machine,
                               int est, plim_t plim);
int platform_place_machine(platform_t *platform, machine_t *machine);
void platform_extend_machine(platform_t *platform, machine_t *machine);
void platform_shift_machine(platform_t *platform, machine_t *machine,
                            int delta);
int platform_extendable_interval_start(platform_t *platform, machine_t *machine,
                                       plim_t plim);
int platform_extendable_interval_finish(platform_t *platform,
                                        machine_t *machine, plim_t plim);

#endif
