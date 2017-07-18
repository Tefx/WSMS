from libcpp cimport bool

DEF RES_DIM = 2
DEF LIM_DIM = 1

cdef extern from "common.h":
    ctypedef int vlen_t

    bool res_richcmp(vlen_t* r0, vlen_t* r1, int op);
    void res_iadd(vlen_t* res0, vlen_t* res1)
    void res_isub(vlen_t* res0, vlen_t* res1)
    void res_imax(vlen_t* res0, vlen_t* res1)
    bool res_le(vlen_t* res0, vlen_t* res1)


ctypedef vlen_t res_t[RES_DIM]
ctypedef vlen_t plim_t[LIM_DIM]

cdef class Resources:
    cdef res_t c
    cdef _setc(self, vlen_t* c)
