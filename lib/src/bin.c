#include "bin.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

static vlen_t vol_tmp[DIM_MAX];

#define _next(node) list_entry((node)->list.next, bin_node_t, list)
#define _prev(node) list_entry((node)->list.prev, bin_node_t, list)

void print_node(bin_node_t* node, int dim) {
    printf("[%d, <", node->time);
    for (int i = 0; i < dim; ++i) printf("%d,", bnode_usage(node)[i]);
    printf("\b>]");
}

void bin_print(bin_t* bin) {
    bin_node_t* node = bin->head;
    int dim = bin->volume_dim;
    print_node(node, dim);
    node = _next(node);
    while (node != bin->head) {
        printf("<=>");
        print_node(node, dim);
        node = _next(node);
    }
    printf("\n");
}

static inline void _set_volume(bin_node_t* node, double x, int dim) {
    for (int i = 0; i < dim; ++i) bnode_usage(node)[i] = x;
}

#define _alloc_node(bin) ((bin_node_t*)mp_alloc(bin->pool))

static inline void _delete_node(bin_t* bin, bin_node_t* node) {
    list_delete(&(node)->list);
    mp_free(bin->pool, node);
}

static inline bin_node_t* _clone_node(bin_t* bin, bin_node_t* prev_node,
                                      int time) {
    bin_node_t* node = _alloc_node(bin);
    list_insert_after(&prev_node->list, &node->list);
    node->time = time;
    memcpy(bnode_usage(node), bnode_usage(prev_node),
           bin->volume_dim * sizeof(vlen_t));
    return node;
}

void bin_init(bin_t* bin, int dim, mempool_t* pool) {
    bin->volume_dim = dim;
    bin->pool = pool;
    bin->head = _alloc_node(bin);
    bin->head->time = -INT_MAX;
    _set_volume(bin->head, 0, dim);
    list_init_head(&bin->head->list);
    bin_node_t* tail = _alloc_node(bin);
    tail->time = INT_MAX;
    _set_volume(tail, 0, dim);
    list_insert_after(&bin->head->list, &tail->list);
    bin->last_start_node = bin->head;
}

void bin_empty(bin_t* bin) {
    bin_node_t* node = bin->head;
    bin_node_t* tmp = node;
    node = _next(node);
    mp_free(bin->pool, tmp);
    while (node != bin->head) {
        tmp = node;
        node = _next(node);
        mp_free(bin->pool, tmp);
    }
}

int bin_open_time(bin_t* bin) {
    bin_node_t* tmp = _next(bin->head);
    return bin_is_empty(bin) ? 0 : tmp->time;
}

int bin_close_time(bin_t* bin) {
    bin_node_t* tmp = _prev(_prev(bin->head));
    return bin_is_empty(bin) ? 0 : tmp->time;
}

int bin_span(bin_t* bin) {
    return bin_is_empty(bin) ? 0 : bin_close_time(bin) - bin_open_time(bin);
}

static inline bin_node_t* _search_node(bin_t* bin, int time) {
    bin_node_t* node = _prev(bin->head);
    while (node->time > time) {
        node = _prev(node);
    }
    return node;
}

static inline bin_node_t* _earliest_slot(bin_node_t* node, int est, int length,
                                         vlen_t* vol, int dim) {
    register int ft = est + length;
    bin_node_t* start_node = node;
    vol_iadd_v(vol, EPSILON, dim);
    for (int i = 0; i < dim; ++i) vol[i] += EPSILON;
    while (node->time < ft) {
        if (vol_le_precise(bnode_usage(node), vol, dim)) {
            node = _next(node);
        } else {
            node = _next(node);
            start_node = node;
            ft = node->time + length;
        }
    }
    return start_node;
}

static inline bin_node_t* _earliest_slot_small(bin_node_t* node, int est,
                                               int length, vlen_t* vol) {
    register int ft = est + length;
    bin_node_t* start_node = node;
    res_iadd_v(vol, EPSILON);
    while (node->time < ft) {
        if (res_le_precise(bnode_usage(node), vol)) {
            node = _next(node);
        } else {
            node = _next(node);
            start_node = node;
            ft = node->time + length;
        }
    }
    return start_node;
}

int bin_earliest_position(bin_t* bin, item_t* item, int est, vlen_t* cap) {
    int dim = bin->volume_dim;
    vol_sub(vol_tmp, cap, item->demands, dim);
    item->start_node = _earliest_slot(_search_node(bin, est), est, item->length,
                                      vol_tmp, dim);
    return MAX(est, item->start_node->time);
}

int bin_earliest_position_res(bin_t* bin, item_t* item, int est, vlen_t* cap) {
    res_sub(vol_tmp, cap, item->demands);
    item->start_node = _earliest_slot_small(_search_node(bin, est), est,
                                            item->length, vol_tmp);
    return MAX(est, item->start_node->time);
}

int bin_earliest_position_forward(bin_t* bin, item_t* item, int est,
                                  vlen_t* cap) {
    int dim = bin->volume_dim;
    bin_node_t* node = bin->last_start_node;
    if (node->time < est) {
        do
            node = _next(node);
        while (node->time < est);
        if (node->time > est) node = _prev(node);
    }
    vol_sub(vol_tmp, cap, item->demands, dim);
    vol_iadd_v(vol_tmp, EPSILON, dim);
    while (!vol_le_precise(bnode_usage(node), vol_tmp, dim)) node = _next(node);
    item->start_node = node;
    return item->start_time = MAX(est, node->time);
}

int bin_earliest_position_forward_res(bin_t* bin, item_t* item, int est,
                                      vlen_t* cap) {
    bin_node_t* node = bin->last_start_node;
    if (node->time < est) {
        do
            node = _next(node);
        while (node->time < est);
        if (node->time > est) node = _prev(node);
    }
    res_sub(vol_tmp, cap, item->demands);
    res_iadd_v(vol_tmp, EPSILON);
    while (!res_le_precise(bnode_usage(node), vol_tmp)) node = _next(node);
    item->start_node = node;
    return item->start_time = MAX(est, node->time);
}

static inline bin_node_t* _bin_alloc(bin_t* bin, int st, int ft,
                                     vlen_t* demands, bin_node_t* node) {
    int dim = bin->volume_dim;
    if (!node) node = _search_node(bin, st);
    if (node->time != st) node = _clone_node(bin, node, st);
    vlen_t* usage = bnode_usage(node);
    vol_iadd(usage, demands, dim);
    if (vol_eq(bnode_usage(_prev(node)), usage, dim)) {
        _delete_node(bin, node);
        node = _prev(node);
    }
    bin->last_start_node = node;
    node = _next(node);
    while (node->time < ft) {
        usage = bnode_usage(node);
        vol_iadd(usage, demands, dim);
        node = _next(node);
    }
    if (node->time > ft) {
        node = _clone_node(bin, _prev(node), ft);
        vol_isub(bnode_usage(node), demands, dim);
    } else if (vol_eq(bnode_usage(_prev(node)), bnode_usage(node), dim)) {
        _delete_node(bin, node);
        node = _prev(node);
    }
    return node;
}

static inline bin_node_t* _bin_alloc_small(bin_t* bin, int st, int ft,
                                           vlen_t* demands, bin_node_t* node) {
    if (!node) node = _search_node(bin, st);
    if (node->time != st) node = _clone_node(bin, node, st);
    vlen_t* usage = bnode_usage(node);
    res_iadd(usage, demands);
    if (res_eq(bnode_usage(_prev(node)), usage)) {
        _delete_node(bin, node);
        node = _prev(node);
    }
    bin->last_start_node = node;
    node = _next(node);
    while (node->time < ft) {
        usage = bnode_usage(node);
        res_iadd(usage, demands);
        node = _next(node);
    }
    if (node->time > ft) {
        node = _clone_node(bin, _prev(node), ft);
        res_isub(bnode_usage(node), demands);
    } else if (res_eq(bnode_usage(_prev(node)), bnode_usage(node))) {
        _delete_node(bin, node);
        node = _prev(node);
    }
    return node;
}

int bin_place_item(bin_t* bin, item_t* item) {
    int st = item->start_time;
    int ft = st + item->length;
    item->finish_node =
        _bin_alloc(bin, st, ft, item->demands, item->start_node);
    return ft;
}

int bin_place_item_res(bin_t* bin, item_t* item) {
    int st = item->start_time;
    int ft = st + item->length;
    item->finish_node =
        _bin_alloc_small(bin, st, ft, item->demands, item->start_node);
    return ft;
}

void bin_extend_item(bin_t* bin, item_t* item, int st, int ft) {
    int st_0 = item->start_time;
    int ft_0 = st_0 + item->length;
    if (st < st_0) {
        item->start_node = _search_node(bin, st);
        _bin_alloc(bin, st, st_0, item->demands, item->start_node);
        item->start_node = bin->last_start_node;
        item->start_time = st;
    }
    if (ft > ft_0) {
        item->finish_node =
            _bin_alloc(bin, ft_0, ft, item->demands, item->finish_node);
    }
    item->length = MAX(ft, ft_0) - MIN(st, st_0);
}

int bin_extendable_interval_start(bin_t* bin, item_t* item, vlen_t* cap) {
    int dim = bin->volume_dim;
    vol_sub(vol_tmp, cap, item->demands, dim);
    bin_node_t* node = item->start_node;
    if (node->time == item->start_time) node = _prev(node);
    while (vol_le(bnode_usage(node), vol_tmp, dim)) node = _prev(node);
    node = _next(node);
    return MIN(node->time, item->start_time);
}

int bin_extendable_interval_finish(bin_t* bin, item_t* item, vlen_t* cap) {
    int dim = bin->volume_dim;
    vol_sub(vol_tmp, cap, item->demands, dim);
    bin_node_t* node = item->finish_node;
    while (vol_le(bnode_usage(node), vol_tmp, dim)) node = _next(node);
    return MAX(node->time, item->start_time + item->length);
}

void bin_shift(bin_t* bin, int delta) {
    bin_node_t* node = _next(bin->head);
    while (node->time != INT_MAX) {
        node->time += delta;
        node = _next(node);
    }
}

void bin_shift_item(bin_t* bin, item_t* item, int delta) {
    vol_ineg(item->demands, bin->volume_dim);
    bin_place_item(bin, item);
    vol_ineg(item->demands, bin->volume_dim);
    item->start_time += delta;
    item->finish_node += delta;
    item->start_node = item->finish_node = NULL;
    bin_place_item(bin, item);
}
