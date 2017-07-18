from libc.string cimport memcpy

DEF RES_DIM = 2

cdef class Resources:
    @classmethod
    def zero(cls):
        res = cls()
        res._set_value(0, 0)
        return res

    def _set_value(self, core, memory):
        self.c[0] = core
        self.c[1] = memory

    cdef _setc(self, res_t c):
        memcpy(self.c, c, sizeof(vlen_t) * RES_DIM)

    def __iadd__(Resources self, Resources other):
        res_iadd(self.c, other.c)
        return self

    def __isub__(Resources self, Resources other):
        res_isub(self.c, other.c)
        return self

    def imax(Resources self, Resources other):
        res_imax(self.c, other.c)

    def __richcmp__(Resources self, Resources other, int op):
        return res_richcmp(self.c, other.c, op)

    def __repr__(self):
        return str(self.c)

    def __getitem__(self, res):
        if res == "core":
            return self.c[0]
        elif res == "memory":
            return self.c[1]

    def copy(self):
        res = Resources()
        memcpy(res.c, self.c, sizeof(vlen_t)*RES_DIM)
        return res
