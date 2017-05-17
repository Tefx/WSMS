cdef class Resources:
    @classmethod
    def zero(cls):
        res = cls()
        res._set_value(0, 0)
        return res

    def _set_value(self, core, memory):
        self.c.core = core
        self.c.memory = memory

    cdef _setc(self, resources_t c):
        self.c = c

    def __iadd__(Resources self, Resources other):
        self.c.core += other.c.core
        self.c.memory += other.c.memory
        return self

    def __isub__(Resources self, Resources other):
        self.c.core -= other.c.core
        self.c.memory -= other.c.memory
        return self

    def __richcmp__(Resources self, Resources other, int op):
        return res_richcmp(&self.c,&other.c, op)

    def scale(Resources self, Resources other):
        res_scale(&self.c, &other.c)

    def __repr__(self):
        return "({:.2%}, {})".format(self.c.core, int(self.c.memory))

    def __getitem__(self, res):
        if res == "core":
            return self.c.core
        elif res == "memory":
            return self.c.memory

    def copy(self):
        res = Resources()
        res._set_value(self.c.core, self.c.memory)
        return res
