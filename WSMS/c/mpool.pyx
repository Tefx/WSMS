from libc.stdio cimport printf


cdef class MemPool:
    def __cinit__(self):
        self.c_ptr = NULL

    def __dealloc__(self):
        if self.c_ptr:
            mp_free_pool(self.c_ptr)
