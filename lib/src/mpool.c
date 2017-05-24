//
// Created by zhaomengz on 26/4/17.
//

#include "mpool.h"
#include <stdlib.h>

#define NEXT(ptr) (*ptr)

void mp_init(mempool_t* pool, size_t block_size, size_t buffer_size) {
    *pool = (mempool_t){NULL, NULL, block_size, buffer_size};
}

void mp_destroy(mempool_t* pool) {
    enode_t* buffer = pool->bl_node;
    enode_t* tmp;
    while (buffer) {
        tmp = buffer;
        buffer = NEXT(buffer);
        free(tmp);
    }
}

void _enlarge(mempool_t* pool) {
    enode_t* buffer;
    enode_t* node;
    size_t buffer_size = pool->buffer_size;
    buffer = (enode_t*)malloc(
        buffer_size * (pool->block_size + sizeof(enode_t)) + sizeof(enode_t));
    NEXT(buffer) = pool->bl_node;
    pool->bl_node = buffer;
    for (size_t i = 0; i < buffer_size; i++) {
        node = (enode_t*)((void*)(buffer + 1) +
                          i * (pool->block_size + sizeof(enode_t)));
        NEXT(node) = pool->fl_head;
        pool->fl_head = node;
    }
}

void* mp_alloc(mempool_t* pool) {
    enode_t* node;
    if (!pool->fl_head) _enlarge(pool);
    node = pool->fl_head;
    pool->fl_head = NEXT(node);
    return node + 1;
}

void mp_free(mempool_t* pool, void* ptr) {
    enode_t* node;
    node = (enode_t*)ptr - 1;
    NEXT(node) = pool->fl_head;
    pool->fl_head = node;
}
