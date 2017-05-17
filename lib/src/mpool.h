//
// Created by zhaomengz on 26/4/17.
//

#ifndef WSMS_MPOOL_H
#define WSMS_MPOOL_H

#include <stddef.h>

typedef void* enode_t;

typedef struct mempool_t {
  enode_t* fl_head;
  enode_t* bl_node;
  size_t block_size;
} mempool_t;

void mp_init(mempool_t* pool, size_t block_size);

void mp_destroy(mempool_t* pool);

void* mp_alloc(mempool_t* pool);

void mp_free(mempool_t* pool, void* ptr);

#endif  // BIN_MPOOL_H
