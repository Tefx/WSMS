#include "bin.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define _next(node) list_entry((node)->list.next, bin_node_t, list)
#define _prev(node) list_entry((node)->list.prev, bin_node_t, list)

void print_node(bin_node_t* node, int dim) {
    printf("[%d, <", node->time);
    for (int i = 0; i < dim; ++i) printf("%.2f,", bnode_usage(node)[i] / 100.0);
    printf("\b>]");
}

void print_bin(bin_t* bin) {
    bin_node_t* node = bin->head;
    int dim = bin->volume_dim;
    print_node(node, dim);
    printf("<=>");
    while (node != _prev(bin->head)) {
        print_node(node, dim);
        printf("<=>");
        node = _next(node);
    }
    print_node(node, dim);
    printf("\n");
}

static inline bool volume_le(volume_t a, volume_t b, int dim) {
    for (int i = 0; i < dim; ++i)
        if (fgt(a[i], b[i])) return false;
    return true;
}

static inline bool volume_le_precise(volume_t a, volume_t b, int dim) {
    for (int i = 0; i < dim; ++i)
        if (a[i] > b[i]) return false;
    return true;
}

static inline bool volume_eq(volume_t a, volume_t b, int dim) {
    for (int i = 0; i < dim; ++i)
        if (!feq(a[i], b[i])) return false;
    return true;
}

#define volume_ineg(a, dim) \
    for (int i = 0; i < (dim); ++i) (a)[i] = -(a)[i]
#define volume_iadd(a, b, dim) \
    for (int i = 0; i < (dim); ++i) (a)[i] += (b)[i]
#define volume_isub(a, b, dim) \
    for (int i = 0; i < (dim); ++i) (a)[i] -= (b)[i]
#define volume_sub(c, a, b, dim) \
    for (int i = 0; i < dim; ++i) (c)[i] = (a)[i] - (b)[i]
#define volume_iadd_v(a, v, dim) \
    for (int i = 0; i < (dim); ++i) (a)[i] += (v)

#if RES_DIM == 2

#define volume_le_small(a, b) (fle((a)[0], (b)[0]) && fle((a)[1], (b)[1]))
#define volume_le_precise_small(a, b) ((a)[0] < (b)[0] && (a)[1] < (b)[1])
#define volume_eq_small(a, b) (feq((a)[0], (b)[0]) && feq((a)[1], (b)[1]))

#define volume_ineg_small(a, b) \
    (a)[0] = -(a)[0];           \
    (a)[1] = -(a)[1]
#define volume_iadd_small(a, b) \
    (a)[0] += (b)[0];           \
    (a)[1] += (b)[1]
#define volume_isub_small(a, b) \
    (a)[0] -= (b)[0];           \
    (a)[1] -= (b)[1]
#define volume_sub_small(c, a, b) \
    (c)[0] = (a)[0] - (b)[0];     \
    (c)[1] = (a)[1] - (b)[1]
#define volume_iadd_v_small(a, v) \
    (a)[0] += v;                  \
    (a)[1] += v
#else
#define volume_le_small(a, b) volume_le(a, b, RES_DIM)
#define volume_le_precise_small(a, b) volume_le_precise(a, b, RES_DIM)
#define volume_eq_small(a, b) volume_eq(a, b, RES_DIM)
#define volume_ineg_small(a) \
    for (int i = 0; i < RES_DIM; ++i) (a)[i] = -(a)[i]
#define volume_iadd_small(a, b) \
    for (int i = 0; i < RES_DIM; ++i) (a)[i] += (b)[i]
#define volume_isub_small(a, b) \
    for (int i = 0; i < RES_DIM; ++i) (a)[i] -= (b)[i]
#define volume_sub_small(c, a, b) \
    for (int i = 0; i < RES_DIM; ++i) (c)[i] = (a)[i] - (b)[i]
#define volume_iadd_v_small(a, v) \
    for (int i = 0; i < RES_DIM; ++i) (a)[i] += v
#endif

static inline void _set_volume(bin_node_t* node, double x, int dim) {
    for (int i = 0; i < dim; ++i) bnode_usage(node)[i] = x;
}

#define _alloc_node(bin) ((bin_node_t*)mp_alloc(&(bin)->pool))

static inline void _delete_node(bin_t* bin, bin_node_t* node) {
    list_delete(&(node)->list);
    mp_free(&(bin)->pool, node);
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

void bin_init(bin_t* bin, int dim, int node_buffer_size) {
    bin->volume_dim = dim;
    mp_init(&bin->pool, bnode_real_size(dim), node_buffer_size);
    bin->head = _alloc_node(bin);
    bin->head->time = -INT_MAX;
    _set_volume(bin->head, 0, dim);
    list_init_head(&bin->head->list);
    bin_node_t* tail = _alloc_node(bin);
    tail->time = INT_MAX;
    _set_volume(tail, 0, dim);
    list_insert_after(&bin->head->list, &tail->list);
    bin->vol_tmp = (vlen_t*)malloc(sizeof(vlen_t) * dim);
    bin->last_start_node = bin->head;
}

void bin_destory(bin_t* bin) { mp_destroy(&bin->pool); }

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
    while (node->time > time) node = _prev(node);
    return node;
}

static inline bin_node_t* _earliest_slot(bin_node_t* node, int est, int length,
                                         volume_t vol, int dim) {
    register int ft = est + length;
    bin_node_t* start_node = node;
    volume_iadd_v(vol, EPSILON, dim);
    for (int i = 0; i < dim; ++i) vol[i] += EPSILON;
    while (node->time < ft) {
        if (volume_le_precise(bnode_usage(node), vol, dim)) {
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
                                               int length, volume_t vol) {
    register int ft = est + length;
    bin_node_t* start_node = node;
    volume_iadd_v_small(vol, EPSILON);
    while (node->time < ft) {
        if (volume_le_precise_small(bnode_usage(node), vol)) {
            node = _next(node);
        } else {
            node = _next(node);
            start_node = node;
            ft = node->time + length;
        }
    }
    return start_node;
}

int bin_earliest_position(bin_t* bin, item_t* item, int est, volume_t cap) {
    int dim = bin->volume_dim;
    volume_sub(bin->vol_tmp, cap, item->demands, dim);
    item->start_node = _earliest_slot(_search_node(bin, est), est, item->length,
                                      bin->vol_tmp, dim);
    return item->start_time = MAX(est, item->start_node->time);
}

int bin_earliest_position_small(bin_t* bin, item_t* item, int est,
                                volume_t cap) {
    volume_sub_small(bin->vol_tmp, cap, item->demands);
    item->start_node = _earliest_slot_small(_search_node(bin, est), est,
                                            item->length, bin->vol_tmp);
    return item->start_time = MAX(est, item->start_node->time);
}

int bin_earliest_position_forward(bin_t* bin, item_t* item, int est,
                                  volume_t cap) {
    int dim = bin->volume_dim;
    bin_node_t* node = bin->last_start_node;
    if (node->time < est) {
        do
            node = _next(node);
        while (node->time < est);
        if (node->time > est) node = _prev(node);
    }
    volume_t vol = bin->vol_tmp;
    volume_sub(vol, cap, item->demands, dim);
    volume_iadd_v(vol, EPSILON, dim);
    while (!volume_le_precise(bnode_usage(node), vol, dim)) node = _next(node);
    item->start_node = node;
    return item->start_time = MAX(est, node->time);
}

int bin_earliest_position_forward_small(bin_t* bin, item_t* item, int est,
                                        volume_t cap) {
    bin_node_t* node = bin->last_start_node;
    if (node->time < est) {
        do
            node = _next(node);
        while (node->time < est);
        if (node->time > est) node = _prev(node);
    }
    volume_t vol = bin->vol_tmp;
    volume_sub_small(vol, cap, item->demands);
    volume_iadd_v_small(vol, EPSILON);
    while (!volume_le_precise_small(bnode_usage(node), vol)) node = _next(node);
    item->start_node = node;
    return item->start_time = MAX(est, node->time);
}

static inline bin_node_t* _bin_alloc(bin_t* bin, int st, int ft,
                                     volume_t demands, bin_node_t* node) {
    int dim = bin->volume_dim;
    if (node->time != st) node = _clone_node(bin, node, st);
    volume_t usage = bnode_usage(node);
    volume_iadd(usage, demands, dim);
    if (volume_eq(bnode_usage(_prev(node)), usage, dim)) {
        _delete_node(bin, node);
        node = _prev(node);
    }
    bin->last_start_node = node;
    while (node->time < ft) {
        usage = bnode_usage(node);
        volume_iadd(usage, demands, dim);
        node = _next(node);
    }
    if (node->time > ft) {
        node = _clone_node(bin, _prev(node), ft);
        volume_isub(bnode_usage(node), demands, dim);
    } else if (volume_eq(bnode_usage(_prev(node)), bnode_usage(node), dim)) {
        _delete_node(bin, node);
        node = _prev(node);
    }
    return node;
}

static inline bin_node_t* _bin_alloc_small(bin_t* bin, int st, int ft,
                                           volume_t demands, bin_node_t* node) {
    if (node->time != st) node = _clone_node(bin, node, st);
    volume_t usage = bnode_usage(node);
    volume_iadd_small(usage, demands);
    if (volume_eq_small(bnode_usage(_prev(node)), usage)) {
        _delete_node(bin, node);
        node = _prev(node);
    }
    bin->last_start_node = node;
    while (node->time < ft) {
        usage = bnode_usage(node);
        volume_iadd_small(usage, demands);
        node = _next(node);
    }
    if (node->time > ft) {
        node = _clone_node(bin, _prev(node), ft);
        volume_isub_small(bnode_usage(node), demands);
    } else if (volume_eq_small(bnode_usage(_prev(node)), bnode_usage(node))) {
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

int bin_place_item_small(bin_t* bin, item_t* item) {
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
    item->length = ft - st;
}

int bin_extendable_interval_start(bin_t* bin, item_t* item, volume_t cap) {
    int dim = bin->volume_dim;
    volume_sub(bin->vol_tmp, cap, item->demands, dim);
    bin_node_t* node = item->start_node;
    if (node->time == item->start_time) node = _prev(node);
    while (volume_le(bnode_usage(node), bin->vol_tmp, dim)) node = _prev(node);
    node = _next(node);
    return MIN(node->time, item->start_time);
}

int bin_extendable_interval_finish(bin_t* bin, item_t* item, volume_t cap) {
    int dim = bin->volume_dim;
    volume_sub(bin->vol_tmp, cap, item->demands, dim);
    bin_node_t* node = item->finish_node;
    while (volume_le(bnode_usage(node), bin->vol_tmp, dim)) node = _next(node);
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
    volume_ineg(item->demands, bin->volume_dim);
    bin_place_item(bin, item);
    volume_ineg(item->demands, bin->volume_dim);
    item->start_time += delta;
    item->finish_node += delta;
    item->start_node = item->finish_node = NULL;
    bin_place_item(bin, item);
}
