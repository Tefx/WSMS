cdef extern from "mpool.h":
    struct mempool_t
    mempool_t* mp_create_pool(size_t block_size, size_t buffer_size)
    void mp_free_pool(mempool_t* pool)

cdef class MemPool:
    cdef mempool_t* c_ptr
