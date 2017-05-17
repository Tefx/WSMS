//
//
// Created by zhaomengz on 27/4/17.
//

#ifndef WMSM_BIN_H
#define WMSM_BIN_H

#include "common.h"
#include "linkedlist.h"
#include "mpool.h"

#ifndef BIN_DEMINSION
#define BIN_DEMINSION 3
#endif

typedef double volume_t[BIN_DEMINSION];

typedef struct bin_node_t {
  list_node_t list;
  int time;
  volume_t allocated;
} bin_node_t;

typedef struct bin_t {
  bin_node_t* head;
  bin_node_t* finger;
  mempool_t pool;
} bin_t;

mempool_t prepare_mempool(void);

void __attribute__((visibility("internal"))) bin_init(bin_t* bin);
void __attribute__((visibility("internal"))) bin_destroy(bin_t* bin);
void __attribute__((visibility("internal")))
bin_alloc_space(bin_t* bin, int st, int ft, volume_t demands);
int __attribute__((visibility("internal")))
bin_earliest_slot(bin_t* bin,
                  int st,
                  int length,
                  volume_t demands,
                  volume_t capacities);
int __attribute__((visibility("internal")))
bin_alloc_earliest(bin_t* bin,
                   int st,
                   int length,
                   volume_t demands,
                   volume_t capacities);
int __attribute__((visibility("internal"))) bin_open_time(bin_t* bin);
int __attribute__((visibility("internal"))) bin_close_time(bin_t* bin);

#define bin_is_empty(bin) ((bin)->head->list.next->next == &(bin)->head->list)
#define bin_runtime(bin) \
  (bin_is_empty(bin) ? 0 : bin_close_time(bin) - bin_open_time(bin))

#define _next(node) list_entry((node)->list.next, bin_node_t, list)
#define _prev(node) list_entry((node)->list.prev, bin_node_t, list)

#endif  // WMSM_BIN_H
