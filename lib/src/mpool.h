//
// Created by zhaomengz on 26/4/17.
//

#ifndef WSMS_MPOOL_H
#define WSMS_MPOOL_H

#include <stddef.h>
#include <stdlib.h>

/*typedef void* enode_t;*/
typedef struct enode_t {
    void* next;
    int ref;
} enode_t;

#define NEXT(ptr) ((ptr)->next)
#define ENODE(ptr) ((enode_t*)(ptr)-1)

typedef struct mempool_t {
    enode_t* fl_head;
    void* bl_node;
    size_t block_size;
    size_t buffer_size;
    size_t bl_ptr;
} mempool_t;

inline void _enlarge(mempool_t* pool) {
    enode_t* buffer;
    size_t buffer_size = pool->buffer_size;
    buffer = malloc(buffer_size * pool->block_size + sizeof(enode_t));
    NEXT(buffer) = pool->bl_node;
    pool->bl_node = buffer;
    pool->bl_ptr = buffer_size;
}

inline mempool_t* mp_create_pool(size_t block_size, size_t buffer_size) {
    mempool_t* pool = (mempool_t*)malloc(sizeof(mempool_t));
    *pool =
        (mempool_t){NULL, NULL, block_size + sizeof(enode_t), buffer_size, 0};
    return pool;
}

inline void mp_free_pool(mempool_t* pool) {
    enode_t* buffer = pool->bl_node;
    enode_t* tmp;
    while (buffer) {
        tmp = buffer;
        buffer = NEXT(buffer);
        free(tmp);
    }
    free(pool);
}

inline void* mp_alloc(mempool_t* pool) {
    enode_t* node;
    if (pool->fl_head) {
        node = pool->fl_head;
        pool->fl_head = NEXT(node);
    } else {
        if (!pool->bl_ptr--) _enlarge(pool);
        node = (enode_t*)(pool->bl_node + sizeof(enode_t) +
                          pool->bl_ptr * pool->block_size);
    }
    node->ref = 1;
    return ++node;
}


inline void mp_free(mempool_t* pool, void* ptr) {
    enode_t* node;
    node = ENODE(ptr);
    if (!--node->ref) {
        NEXT(node) = pool->fl_head;
        pool->fl_head = node;
    }
}

inline void mp_inc_ref(void* ptr) { ENODE(ptr)->ref--; }

#endif  // BIN_MPOOL_H
