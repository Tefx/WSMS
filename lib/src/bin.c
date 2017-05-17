#include "bin.h"
#include <limits.h>
#include <string.h>
#include "common.h"

#define _next(node) list_entry((node)->list.next, bin_node_t, list)
#define _prev(node) list_entry((node)->list.prev, bin_node_t, list)
#define allocated(x) ((volume_t)((x) + 1))

static inline bool volume_le(volume_t a, volume_t b, int dim) {
    while (--dim >= 0 && fle(a[dim], b[dim]))
        ;
    return dim < 0;
}

static inline bool volume_eq(volume_t a, volume_t b, int dim) {
    while (--dim >= 0 && feq(a[dim], b[dim]))
        ;
    return dim < 0;
}

/*static inline bool volume_ne(volume_t a, volume_t b, int dim) {*/
/*while (--dim >= 0 && fne(a[dim], b[dim]))*/
/*;*/
/*return dim < 0;*/
/*}*/

static inline void volume_iadd(volume_t a, volume_t b, int dim) {
    while (--dim >= 0) a[dim] += b[dim];
}

static inline void volume_isub(volume_t a, volume_t b, int dim) {
    while (--dim >= 0) a[dim] -= b[dim];
}

/*static inline void volume_add(volume_t op1, volume_t op2, volume_t res,*/
/*int dim) {*/
/*while (--dim >= 0) res[dim] = op1[dim] + op2[dim];*/
/*}*/

/*static inline void volume_sub(volume_t op1, volume_t op2, volume_t res,*/
/*int dim) {*/
/*while (--dim >= 0) res[dim] = op1[dim] - op2[dim];*/
/*}*/

static inline void _set_volume(bin_node_t* node, double x, int dim) {
    for (int i = 0; i < dim; ++i) allocated(node)[i] = x;
}

static inline bin_node_t* _alloc_node(bin_t* bin) {
    bin_node_t* node = (bin_node_t*)mp_alloc(&bin->pool);
    node->allocated = (volume_t*)(node + 1);
    return node;
}

static inline bin_node_t* _clone_node(bin_t* bin, bin_node_t* prev_node,
                                      int time) {
    bin_node_t* node = _alloc_node(bin);
    list_insert_after(&prev_node->list, &node->list);
    node->time = time;
    memcpy(node->allocated, prev_node->allocated,
           bin->volume_dim * sizeof(vlen_t));
    return node;
}

static inline void _delete_node(bin_t* bin, bin_node_t* node) {
    list_delete(&node->list);
    mp_free(&bin->pool, node);
}

void bin_init(bin_t* bin, int dim) {
    bin->_actual_node_size = sizeof(bin_node_t) + sizeof(double) * dim;
    bin->volume_dim = dim;
    mp_init(&bin->pool, bin->_actual_node_size);
    bin->head = _alloc_node(bin);
    bin->head->time = -INT_MAX;
    _set_volume(bin->head, 0, dim);
    list_init_head(&bin->head->list);
    bin_node_t* tail = _alloc_node(bin);
    tail->time = INT_MAX;
    _set_volume(tail, 0, dim);
    list_insert_after(&bin->head->list, &tail->list);
}

bin_node_t* bin_search(bin_t* bin, int time) {
    bin_node_t* node = _prev(bin->head);
    while (node->time > time) node = _prev(node);
    return node;
}

void bin_free(bin_t* bin) { mp_destroy(&bin->pool); }

int bin_open_time(bin_t* bin) {
    bin_node_t* tmp = _next(bin->head);
    return bin_is_empty(bin) ? 0 : tmp->time;
}

int bin_close_time(bin_t* bin) {
    bin_node_t* tmp = _prev(_prev(bin->head));
    return bin_is_empty(bin) ? 0 : tmp->time;
}

bin_node_t* earliest_available_node(bin_t* bin, int est, int length,
                                    volume_t demands, volume_t capacities) {
    int ft = est + length;
    int dim = bin->volume_dim;
    bin_node_t* node = bin_search(bin, est);
    bin_node_t* start_node = node;

    volume_isub(capacities, demands, dim);

    while (node->time < ft) {
        if (volume_le(allocated(node), capacities, dim)) {
            node = _next(node);
        } else {
            node = _next(node);
            start_node = node;
            ft = node->time + length;
        }
    }

    return start_node;
}

bin_item_t bin_alloc_after_node(bin_t* bin, bin_node_t* node, int st, int ft,
                                volume_t demands) {
    bin_node_t* tail = _next(bin->head);
    int dim = bin->volume_dim;
    if (node->time != st) node = _clone_node(bin, node, st);
    bin_node_t* start_node;
    volume_iadd(allocated(node), demands, dim);
    if (volume_eq(allocated(_prev(node)), allocated(node), dim)) {
        start_node = _prev(node);
        _delete_node(bin, node);
    } else {
        start_node = node;
    }
    node = _next(node);
    while (node->time < ft) {
        volume_iadd(allocated(node), demands, dim);
        node = _next(node);
    }
    if (node->time > ft) {
        node = _clone_node(bin, _prev(node), ft);
        volume_isub(allocated(node), demands, dim);
    } else if (volume_eq(allocated(_prev(node)), allocated(node), dim)) {
        _delete_node(bin, node);
        return (bin_item_t){start_node, _prev(node)};
    }
    return (bin_item_t){start_node, node};
}

int bin_extendable_interval_start(bin_t* bin, int st, volume_t demands,
                                  volume_t capacities, bin_node_t* start_node) {
    int dim = bin->volume_dim;
    volume_isub(capacities, demands, dim);

    if (!start_node) start_node = bin_search(bin, st);

    if (start_node->time == st) start_node = _prev(start_node);
    while (volume_le(allocated(start_node), capacities, dim))
        start_node = _prev(start_node);
    start_node = _next(start_node);
    return fmin(start_node->time, st);
}

int bin_extendable_interval_finish(bin_t* bin, int ft, volume_t demands,
                                   volume_t capacities,
                                   bin_node_t* finish_node) {
    int dim = bin->volume_dim;
    volume_isub(capacities, demands, dim);

    if (!finish_node) finish_node = bin_search(bin, ft);

    while (volume_le(allocated(finish_node), capacities, dim))
        finish_node = _next(finish_node);
    return MAX(finish_node->time, ft);
}
