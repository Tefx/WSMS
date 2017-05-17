#ifndef WSMS_BIN_H
#define WSMS_BIN_H

#include "linkedlist.h"
#include "mpool.h"

typedef double vlen_t;
typedef double* volume_t;

typedef struct bin_node_t {
    list_node_t list;
    int time;
    volume_t* allocated;
} bin_node_t;

typedef struct bin_item_t {
    bin_node_t* start_node;
    bin_node_t* finish_node;
} bin_item_t;

typedef struct bin_t {
    bin_node_t* head;
    mempool_t pool;
    int volume_dim;
    size_t _actual_node_size;
} bin_t;

#define item_start_time(x) ((x)->start_node->time)
#define item_finish_time(x) ((x)->finish_node->time)

#define bin_is_empty(bin) ((bin)->head->list.next->next == &(bin)->head->list)

void bin_init(bin_t* bin, int dim);
void bin_free(bin_t* bin);

bin_node_t* bin_search(bin_t* bin, int time);

int bin_open_time(bin_t* bin);
int bin_close_time(bin_t* bin);

bin_node_t* earliest_available_node(bin_t* bin, int est, int length,
                                    volume_t demands, volume_t capacities);
bin_item_t bin_alloc_after_node(bin_t* bin, bin_node_t* node, int st, int ft,
                                volume_t demands);

int bin_extendable_interval_start(bin_t* bin, int st, volume_t demands,
                                  volume_t capacities, bin_node_t* start_node);
int bin_extendable_interval_finish(bin_t* bin, int ft, volume_t demands,
                                   volume_t capacities,
                                   bin_node_t* finish_node);

#endif  // ifndef WSMS_BIN_H
