#ifndef WSMS_BIN_H
#define WSMS_BIN_H

#include "common.h"
#include "linkedlist.h"
#include "mpool.h"

typedef vlen_t* volume_t;

typedef struct bin_node_t {
    list_node_t list;
    int time;
} bin_node_t;

#define bnode_real_size(dim) (sizeof(bin_node_t) + sizeof(vlen_t) * (dim))
#define bnode_new(dim) ((bin_node_t*)malloc(bnode_real_size(dim)))
#define bnode_free(node) free(node)
#define bnode_usage(x) ((volume_t)((x) + 1))

typedef struct item_t {
    int start_time;
    int length;
    bin_node_t* start_node;
    bin_node_t* finish_node;
    volume_t demands;
} item_t;

/*typedef struct item_t {*/
    /*int start_time;*/
    /*int length;*/
    /*bin_node_t* start_node;*/
    /*bin_node_t* finish_node;*/
/*} item_t;*/

/*#define item_real_size(dim) (sizeof(item_t) + sizeof(vlen_t) * (dim))*/
/*#define item_new(dim) ((item_t*)malloc(item_real_size(dim)))*/
/*#define item_free(item) free(item)*/
/*#define item_demands(item) ((volume_t)((item_t*)(item) + 1))*/
/*#define item_start_time(item) ((item)->start_node->time)*/
/*#define item_finish_time(item) ((item)->start_node->time + (item)->length)*/

typedef struct bin_t {
    bin_node_t* head;
    mempool_t pool;
    int volume_dim;
    vlen_t* vol_tmp;
} bin_t;

void print_bin(bin_t* bin);
#define bin_is_empty(bin) ((bin)->head->list.next->next == &(bin)->head->list)

void bin_init(bin_t* bin, int dim);
void bin_destory(bin_t* bin);
void bin_shift(bin_t* bin, int delta);

int bin_open_time(bin_t* bin);
int bin_close_time(bin_t* bin);
int bin_span(bin_t* bin);

int bin_earliest_position(bin_t* bin, item_t* item, int est,
                          volume_t capacities);
int bin_place_item(bin_t* bin, item_t* item);
void bin_extend_item(bin_t* bin, item_t* item, int st, int ft);
void bin_shift_item(bin_t* bin, item_t* item, int delta);
int bin_extendable_interval_start(bin_t*, item_t* item, volume_t capacities);
int bin_extendable_interval_finish(bin_t* bin, item_t* item,
                                   volume_t capacities);

#endif  // ifndef WSMS_BIN_H
