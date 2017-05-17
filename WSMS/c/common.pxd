from libcpp cimport bool

cdef extern from "common.h":
    struct resources_t:
        double core
        double memory

    bool res_richcmp(resources_t* r0, resources_t* r1, int op);
    void res_scale(resources_t* r0, resources_t* r1);


cdef class Resources:
    cdef resources_t c
