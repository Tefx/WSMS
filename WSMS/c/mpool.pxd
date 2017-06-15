cdef extern from "mpool.h":
    struct enode_t
    struct mempool_t

    mempool_t* mp_create_pool(size_t block_size, size_t block_num)
    void mp_free_pool(mempool_t* pool)
    void* mp_alloc(mempool_t* pool)
    void mp_free(mempool_t* pool, void* ptr)
    void mp_inc_ref(void* ptr)

cdef class MemPool:
    cdef mempool_t* c_ptr
