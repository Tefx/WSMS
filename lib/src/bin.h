#ifndef WSMS_BIN_H
#define WSMS_BIN_H

#include "common.h"
#include "linkedlist.h"
#include "mpool.h"

typedef struct bin_node_t {
    int time;
    list_node_t list;
} bin_node_t;

#define bnode_real_size(dim) (sizeof(bin_node_t) + sizeof(vlen_t) * (dim))
#define bnode_usage(x) ((volume_t)((x) + 1))
/*#define bnode_create_mpool(dim, size) mp_create_pool(bnode_real_size(dim),
 * size)*/

typedef struct item_t {
    bin_node_t* start_node;
    bin_node_t* finish_node;
    volume_t demands;
    int start_time;
    int length;
} item_t;

typedef struct bin_t {
    bin_node_t* head;
    mempool_t* pool;
    int volume_dim;
    bin_node_t* last_start_node;
} bin_t;

void bin_print(bin_t* bin);
#define bin_is_empty(bin) ((bin)->head->list.next->next == &(bin)->head->list)

inline mempool_t* bin_prepare_pool(int dim, size_t buffer_size) {
    return mp_create_pool(bnode_real_size(dim), buffer_size);
}

void bin_init(bin_t* bin, int dim, mempool_t* pool);
void bin_empty(bin_t* bin);
void bin_shift(bin_t* bin, int delta);

int bin_open_time(bin_t* bin);
int bin_close_time(bin_t* bin);
int bin_span(bin_t* bin);

int bin_earliest_position(bin_t* bin, item_t* item, int est,
                          volume_t capacities);
int bin_earliest_position_res(bin_t* bin, item_t* item, int est,
                              volume_t capacities);
int bin_earliest_position_forward(bin_t* bin, item_t* item, int est,
                                  volume_t cap);
int bin_earliest_position_forward_res(bin_t* bin, item_t* item, int est,
                                      volume_t cap);
int bin_place_item(bin_t* bin, item_t* item);
int bin_place_item_res(bin_t* bin, item_t* item);
void bin_extend_item(bin_t* bin, item_t* item, int st, int ft);
void bin_shift_item(bin_t* bin, item_t* item, int delta);
int bin_extendable_interval_start(bin_t*, item_t* item, volume_t capacities);
int bin_extendable_interval_finish(bin_t* bin, item_t* item,
                                   volume_t capacities);

#endif  // ifndef WSMS_BIN_H
