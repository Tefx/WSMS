from libcpp cimport bool

DEF RES_DIM = 2
DEF LIM_DIM = 1

cdef extern from "common.h":
    ctypedef float vlen_t
    ctypedef vlen_t res_t[RES_DIM]
    ctypedef vlen_t plim_t[LIM_DIM];

    bool res_richcmp(res_t r0, res_t r1, int op);
    void res_scale(res_t r0, res_t r1);

    void res_iadd(res_t res0, res_t res1)
    void res_isub(res_t res0, res_t res1)


cdef class Resources:
    cdef res_t c
    cdef _setc(self, res_t c)
