#include "schedule.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static inline int _earliest_start_time(task_info_t* task, int* finish_times) {
    register int est = 0;
    register int ft;
    int num_prevs = task->num_prevs;
    int* prevs = task->prevs;
    while (num_prevs) {
        ft = finish_times[prevs[--num_prevs]];
        iMAX(est, ft);
    }
    return est;
}

static inline int _compare_int(const void* a, const void* b) {
    const double* da = (const double*)a;
    const double* db = (const double*)b;

    return (*da > *db) - (*da < *db);
}

static inline void _fill_finish_times(schedule_t* schedule,
                                      problem_t* problem) {
    int num_tasks = schedule->num_tasks;
    schedule->_finish_times = schedule->start_times + num_tasks;
    for (int i = 0; i < num_tasks; ++i)
        schedule->_finish_times[i] =
            schedule->start_times[i] +
            problem_task_runtime(problem, i, TYPL(schedule, i));
}

static inline void _fill_vm_times(schedule_t* schedule) {
    int num_vms = schedule->num_vms;
    int num_tasks = schedule->num_tasks;
    int* start_times = schedule->start_times;
    int* placements = schedule->placements;

    schedule->_finish_times = schedule->start_times + num_tasks;
    schedule->_vm_open_times = (int*)malloc(sizeof(int) * num_vms * 2);
    schedule->_vm_close_times = schedule->_vm_open_times + num_vms;
    int* finish_times = schedule->_finish_times;
    int* open_times = schedule->_vm_open_times;
    int* close_times = schedule->_vm_close_times;

    for (int vm_id = 0; vm_id < num_vms; ++vm_id) {
        open_times[vm_id] = INT_MAX;
        close_times[vm_id] = -INT_MAX;
    }

    for (int task_id = 0; task_id < num_tasks; ++task_id) {
        int vm_id = placements[task_id];
        iMIN(open_times[vm_id], start_times[task_id]);
        iMAX(close_times[vm_id], finish_times[task_id]);
    }
}

void schedule_init(schedule_t* schedule, int num_tasks) {
    schedule->num_tasks = num_tasks;
    schedule->placements = (int*)malloc(sizeof(int) * num_tasks * 3);
    schedule->start_times = schedule->placements + num_tasks;
    schedule->vm_types = NULL;
    schedule->_finish_times = NULL;
    schedule->_vm_open_times = schedule->_vm_close_times = NULL;
    schedule->pnvm = 0;
}

void schedule_free(schedule_t* schedule) {
    free(schedule->placements);
    if (schedule->vm_types) free(schedule->vm_types);
    if (schedule->_vm_open_times) free(schedule->_vm_open_times);
}

void schedule_set_placements(schedule_t* schedule, int* placements) {
    memcpy(schedule->placements, placements, sizeof(int) * schedule->num_tasks);
}

void schedule_set_vm_types(schedule_t* schedule, int* vm_types, int num_vms) {
    schedule->num_vms = num_vms;
    schedule->vm_types = (int*)malloc(sizeof(int) * num_vms);
    memcpy(schedule->vm_types, vm_types, sizeof(int) * num_vms);
}

void schedule_set_start_times(schedule_t* schedule, int* start_times) {
    memcpy(schedule->start_times, start_times,
           sizeof(int) * schedule->num_tasks);
}

void schedule_simulate(schedule_t* schedule, problem_t* problem, int* order,
                       bool forward, mempool_t* mpool) {
    int num_tasks = schedule->num_tasks;
    int num_vms = schedule->num_vms;

    schedule->_finish_times = schedule->start_times + num_tasks;
    schedule->_vm_open_times = (int*)malloc(sizeof(int) * num_vms * 2);
    schedule->_vm_close_times = schedule->_vm_open_times + num_vms;

    int* placements = schedule->placements;
    int* vm_types = schedule->vm_types;
    int* start_times = schedule->start_times;
    int* finish_times = schedule->_finish_times;
    task_info_t* tasks = problem->tasks;
    type_info_t* types = problem->types;
    bool new_mpool = mpool == NULL;
    if (new_mpool) mpool = machine_create_mpool(num_tasks);

    machine_t* _vms = (machine_t*)malloc(sizeof(machine_t) * num_vms);
    task_t task;
    machine_t* vm;
    int task_id, vm_id, type_id;
    volume_t capacities;
    int est;

    for (int i = 0; i < num_vms; ++i) {
        vm = _vms + i;
        machine_init(vm, mpool);
        machine_set_demands(vm, types[vm_types[i]].demands);
    }

    for (int i = 0; i < num_tasks; ++i) {
        task_id = order[i];
        vm_id = placements[task_id];
        type_id = vm_types[vm_id];
        vm = _vms + vm_id;

        task_prepare(&task, problem, task_id, type_id);
        est = _earliest_start_time(tasks + task_id, finish_times);
        capacities = types[type_id].capacities;
        start_times[task_id] = task.start_time =
            forward
                ? machine_earliest_position_forward(vm, &task, est, capacities)
                : machine_earliest_position(vm, &task, est, capacities);
        finish_times[task_id] = machine_place_task(vm, &task);
    }

    for (int i = 0; i < num_vms; ++i) {
        schedule->_vm_open_times[i] = machine_open_time(_vms + i);
        schedule->_vm_close_times[i] = machine_close_time(_vms + i);
        machine_destory(_vms + i);
    }
    free(_vms);
    if (new_mpool) mp_free_pool(mpool);
}

void schedule_calculate_objectives(schedule_t* schedule, problem_t* problem) {
    if (!schedule->_finish_times) _fill_finish_times(schedule, problem);
    if (!schedule->_vm_open_times) _fill_vm_times(schedule);

    int* vm_types = schedule->vm_types;
    int* open_times = schedule->_vm_open_times;
    int* close_times = schedule->_vm_close_times;
    double cost = 0;
    int span_start = INT_MAX;
    int span_finish = -INT_MAX;
    int open_time, close_time;

    for (int vm_id = 0; vm_id < schedule->num_vms; ++vm_id) {
        open_time = open_times[vm_id];
        close_time = close_times[vm_id];
        cost +=
            problem_charge(problem, vm_types[vm_id], close_time - open_time);
        iMIN(span_start, open_time);
        iMAX(span_finish, close_time);
    }
    schedule->objectives.cost = cost;
    schedule->objectives.makespan = span_finish - span_start;
}

int schedule_calculate_pnvm(schedule_t* schedule, problem_t* problem) {
    if (!schedule->_finish_times) _fill_finish_times(schedule, problem);
    if (!schedule->_vm_open_times) _fill_vm_times(schedule);

    int num_vms = schedule->num_vms;
    int* open_times = (int*)malloc(sizeof(int) * num_vms * 2);
    int* close_times = open_times + num_vms;

    memcpy(open_times, schedule->_vm_open_times, sizeof(int) * num_vms * 2);
    qsort(open_times, num_vms, sizeof(int), _compare_int);
    qsort(close_times, num_vms, sizeof(int), _compare_int);

    int peak_num = 0, cur_num = 0;
    int j = 0;
    for (int i = 0; i < num_vms; ++i) {
        int cur_time = open_times[i];
        while (close_times[j++] <= cur_time) cur_num--;
        cur_num++;
        iMAX(peak_num, cur_num);
    }
    free(open_times);
    schedule->pnvm = peak_num;

    return peak_num;
}
