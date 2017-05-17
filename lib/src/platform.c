#include "platform.h"
#include <stdlib.h>

#define RES_DIM 2

#define res2vol(res, vol) ((vol)[0] = (res)->core, (vol)[1] = (res)->memory)
#define lim2vol(lim, vol) ((vol)[0] = lim)

void machine_init(machine_t *machine) {
    machine->bin = (bin_t *)malloc(sizeof(bin_t));
    bin_init(machine->bin, RES_DIM);
    machine->finger = NULL;
}

void machine_free(machine_t *machine) {
    bin_free(machine->bin);
    free(machine->bin);
}

int machine_open_time(machine_t *machine) {
    return bin_open_time(machine->bin);
}

int machine_close_time(machine_t *machine) {
    return bin_close_time(machine->bin);
}

int machine_runtime(machine_t *machine) {
    return bin_is_empty(machine->bin)
               ? bin_close_time(machine->bin) - bin_open_time(machine->bin)
               : 0;
}

int machine_alloc_earliest(machine_t *machine, int est, int rt,
                           resources_t *demands, resources_t *capacities) {
    vlen_t demands_v[RES_DIM];
    vlen_t capacities_v[RES_DIM];
    res2vol(demands, demands_v);
    res2vol(capacities, capacities_v);
    bin_node_t *node =
        earliest_available_node(machine->bin, est, rt, demands_v, capacities_v);
    iMAX(est, node->time);
    bin_alloc_after_node(machine->bin, node, est, est + rt, demands_v);
    return est;
}

int machine_earliest_slot(machine_t *machine, int est, int rt,
                          resources_t *demands, resources_t *capacities) {
    vlen_t demands_v[RES_DIM];
    vlen_t capacities_v[RES_DIM];
    res2vol(demands, demands_v);
    res2vol(capacities, capacities_v);
    machine->finger =
        earliest_available_node(machine->bin, est, rt, demands_v, capacities_v);
    return MAX(machine->finger->time, est);
}

void machine_place_task(machine_t *machine, int st, int rt,
                        resources_t *demands) {
    vlen_t demands_v[RES_DIM];
    res2vol(demands, demands_v);
    if (!machine->finger) machine->finger = bin_search(machine->bin, st);
    bin_alloc_after_node(machine->bin, machine->finger, st, st + rt, demands_v);
}

void platform_init(platform_t *platform) {
    platform->bin = (bin_t *)malloc(sizeof(bin_t));
    bin_init(platform->bin, 1);
    platform->finger = NULL;
}

void platform_free(platform_t *platform) {
    bin_free(platform->bin);
    free(platform->bin);
}

int platform_earliest_slot(platform_t *platform, int est, int rt,
                           int total_limit) {
    static vlen_t demands_v[1] = {1};
    bin_node_t *node = earliest_available_node(
        platform->bin, est, rt, demands_v, (vlen_t *)&total_limit);
    return MAX(node->time, est);
}

void platform_alloc_machine(platform_t *platform, machine_t *machine, int st,
                            int rt) {
    static vlen_t demands_v[1] = {1};
    machine_init(machine);
    if (!platform->finger) platform->finger = bin_search(platform->bin, st);
    machine->item_in_platform = bin_alloc_after_node(
        platform->bin, platform->finger, st, st + rt, demands_v);
}

void platform_extend_machine(platform_t *platform, machine_t *machine, int st,
                             int ft) {
    static vlen_t demands_v[1] = {1};
    int st_0 = machine_open_time(machine);
    int ft_0 = machine_close_time(machine);

    if (st < st_0) {
        bin_node_t *node = bin_search(platform->bin, st);
        bin_item_t item =
            bin_alloc_after_node(platform->bin, node, st, st_0, demands_v);
        machine->item_in_platform.start_node = item.start_node;
    }

    if (ft > ft_0) {
        bin_item_t item = bin_alloc_after_node(
            platform->bin, machine->item_in_platform.finish_node, ft_0, ft,
            demands_v);
        machine->item_in_platform.finish_node = item.finish_node;
    }
}

void platform_shift_machine(platform_t *platform, machine_t *machine,
                            int delta) {
    static vlen_t demands_v[1];
    demands_v[0] = -1;
    int st_0 = machine_open_time(machine);
    int ft_0 = machine_close_time(machine);
    bin_alloc_after_node(platform->bin, machine->item_in_platform.start_node,
                         st_0, ft_0, demands_v);
    bin_node_t *node = bin_search(platform->bin, st_0 + delta);
    demands_v[0] = 1;
    machine->item_in_platform = bin_alloc_after_node(
        platform->bin, node, st_0 + delta, ft_0 + delta, demands_v);
}

void platform_free_machine(platform_t *platform, machine_t *machine) {
    static vlen_t demands_v[1] = {-1};
    bin_alloc_after_node(platform->bin, machine->item_in_platform.start_node,
                         machine_open_time(machine),
                         machine_close_time(machine), demands_v);
}

void platform_extendable_interval(platform_t *platform, machine_t *machine,
                                  int *start, int *finish, int total_limit) {
    static vlen_t demands_v[1] = {-1};
    *start = bin_extendable_interval_start(
        platform->bin, machine_open_time(machine), demands_v,
        (vlen_t *)&(total_limit), machine->item_in_platform.start_node);
    *finish = bin_extendable_interval_finish(
        platform->bin, machine_close_time(machine), demands_v,
        (vlen_t *)&(total_limit), machine->item_in_platform.finish_node);
}

static int _compare_int(const void *a, const void *b) {
    const double *da = (const double *)a;
    const double *db = (const double *)b;

    return (*da > *db) - (*da < *db);
}

int machine_pnumber(machine_t *machines, int num_machines) {
    int *open_times = (int *)malloc(sizeof(int) * num_machines);
    int *close_times = (int *)malloc(sizeof(int) * num_machines);
    for (int i = 0; i < num_machines; ++i) {
        open_times[i] = machine_open_time(machines + i);
        close_times[i] = machine_close_time(machines + i);
    }
    qsort(open_times, num_machines, sizeof(int), _compare_int);
    qsort(close_times, num_machines, sizeof(int), _compare_int);
    int peak_num = 0, cur_num = 0;
    int cur_time = 0;
    int j = 0;
    for (int i = 0; i < num_machines; ++i) {
        cur_time = open_times[i];
        while (close_times[j++] <= cur_time) cur_num--;
        cur_num++;
        peak_num = MAX(peak_num, cur_num);
    }
    free(open_times);
    free(close_times);
    return peak_num;
}
