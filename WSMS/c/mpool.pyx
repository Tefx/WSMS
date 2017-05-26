from libc.stdio cimport printf


cdef class MemPool:
    def __cinit__(self):
        self.c_ptr = NULL

    def free_pool(self):
        print("free pool...")
        mp_free_pool(self.c_ptr)

    def __dealloc__(self):
        if self.c_ptr:
            mp_free_pool(self.c_ptr)
