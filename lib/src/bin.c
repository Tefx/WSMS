#include "bin.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _next(node) list_entry((node)->list.next, bin_node_t, list)
#define _prev(node) list_entry((node)->list.prev, bin_node_t, list)

void print_node(bin_node_t* node, int dim) {
    printf("[%d, <", node->time);
    for (int i = 0; i < dim; ++i) printf("%.2f,", bnode_usage(node)[i]);
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
        if (!fle(a[i], b[i])) return false;
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

static inline bin_node_t* _search_node(bin_t* bin, int time) {
    bin_node_t* node = _prev(bin->head);
    while (node->time > time) node = _prev(node);
    return node;
}

void bin_init(bin_t* bin, int dim) {
    bin->volume_dim = dim;
    mp_init(&bin->pool, bnode_real_size(dim));
    bin->head = _alloc_node(bin);
    bin->head->time = -INT_MAX;
    _set_volume(bin->head, 0, dim);
    list_init_head(&bin->head->list);
    bin_node_t* tail = _alloc_node(bin);
    tail->time = INT_MAX;
    _set_volume(tail, 0, dim);
    list_insert_after(&bin->head->list, &tail->list);
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

int bin_earliest_position(bin_t* bin, item_t* item, int est,
                          volume_t capacities) {
    int length = item->length;
    int ft = est + length;
    int dim = bin->volume_dim;
    bin_node_t* node = _search_node(bin, est);
    item->start_node = node;
    volume_isub(capacities, item_demands(item), dim);
    while (node->time < ft) {
        if (volume_le(bnode_usage(node), capacities, dim)) {
            node = _next(node);
        } else {
            node = _next(node);
            item->start_node = node;
            ft = node->time + length;
        }
    }
    item->start_time = MAX(est, item->start_node->time);
    return item->start_time;
}

bin_node_t* bin_alloc(bin_t* bin, int st, int ft, volume_t demands,
                      bin_node_t** start_node) {
    int dim = bin->volume_dim;
    bin_node_t* node = *start_node ? *start_node : _search_node(bin, st);
    if (node->time != st) node = _clone_node(bin, node, st);
    volume_iadd(bnode_usage(node), demands, dim);
    if (volume_eq(bnode_usage(_prev(node)), bnode_usage(node), dim)) {
        node = _prev(node);
        _delete_node(bin, _next(node));
    }
    *start_node = node;
    while (node->time < ft) {
        volume_iadd(bnode_usage(node), demands, dim);
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

int bin_place_item(bin_t* bin, item_t* item) {
    int st = item->start_time;
    int ft = st + item->length;
    item->finish_node =
        bin_alloc(bin, st, ft, item_demands(item), &item->start_node);
    return ft;
}

void bin_extend_item(bin_t* bin, item_t* item, int st, int ft) {
    int st_0 = item->start_time;
    int ft_0 = st_0 + item->length;
    if (st < st_0) {
        item->start_node = NULL;
        bin_alloc(bin, st, st_0, item_demands(item), &item->start_node);
        item->start_time = st;
    }
    if (ft > ft_0) {
        item->finish_node =
            bin_alloc(bin, ft_0, ft, item_demands(item), &item->finish_node);
    }
    item->length = ft - st;
}

int bin_extendable_interval_start(bin_t* bin, item_t* item,
                                  volume_t capacities) {
    int dim = bin->volume_dim;
    volume_isub(capacities, item_demands(item), dim);
    bin_node_t* node = item->start_node;
    if (node->time == item->start_time) node = _prev(node);
    while (volume_le(bnode_usage(node), capacities, dim)) node = _prev(node);
    node = _next(node);
    return MIN(node->time, item->start_time);
}

int bin_extendable_interval_finish(bin_t* bin, item_t* item,
                                   volume_t capacities) {
    int dim = bin->volume_dim;
    volume_isub(capacities, item_demands(item), dim);
    bin_node_t* node = item->finish_node;
    while (volume_le(bnode_usage(node), capacities, dim)) node = _next(node);
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
    volume_ineg(item_demands(item), bin->volume_dim);
    bin_place_item(bin, item);
    volume_ineg(item_demands(item), bin->volume_dim);
    item->start_time += delta;
    item->finish_node += delta;
    item->start_node = item->finish_node = NULL;
    bin_place_item(bin, item);
}
