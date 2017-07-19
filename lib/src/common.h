#ifndef WSMS_COMMON_H
#define WSMS_COMMON_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <math.h>
#include <stdbool.h>

#define RES_DIM 2
#define LIM_DIM 1
#define DIM_MAX 2

typedef int vlen_t;
#define VLEN_IS_INT

#ifdef VLEN_IS_INT
#define EPSILON 0
#define feq(x, y) ((x) == (y))
#define fne(x, y) ((x) != (y))
#define fle(x, y) ((x) <= (y))
#define flt(x, y) ((x) < (y))
#define fge(x, y) ((x) >= (y))
#define fgt(x, y) ((x) > (y))
#else
#define EPSILON 0.001
#define feq(x, y) (fabs((x) - (y)) <= EPSILON)
#define fne(x, y) (fabs((x) - (y)) > EPSILON)
#define fle(x, y) ((x) <= ((y) + EPSILON))
#define flt(x, y) ((x) < ((y)-EPSILON))
#define fge(x, y) ((x) >= ((y)-EPSILON))
#define fgt(x, y) ((x) > ((y) + EPSILON))
#endif

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define iMAX(x, y) (x) = MAX(x, y)
#define iMIN(x, y) (x) = MIN(x, y)

bool res_richcmp(vlen_t* r0, vlen_t* r1, int op);

static inline bool vol_le(vlen_t* a, vlen_t* b, int dim) {
    for (int i = 0; i < dim; ++i)
        if (fgt(a[i], b[i])) return false;
    return true;
}

static inline bool vol_le_precise(vlen_t* a, vlen_t* b, int dim) {
    for (int i = 0; i < dim; ++i)
        if (a[i] > b[i]) return false;
    return true;
}

static inline bool vol_lt(vlen_t* a, vlen_t* b, int dim) {
    for (int i = 0; i < dim; ++i)
        if (fge(a[i], b[i])) return false;
    return true;
}

static inline bool vol_eq(vlen_t* a, vlen_t* b, int dim) {
    for (int i = 0; i < dim; ++i)
        if (!feq(a[i], b[i])) return false;
    return true;
}

#define vol_ineg(a, dim) \
    for (int _i = 0; _i < (dim); ++_i) (a)[_i] = -(a)[_i]
#define vol_iadd(a, b, dim) \
    for (int _i = 0; _i < (dim); ++_i) (a)[_i] += (b)[_i]
#define vol_isub(a, b, dim) \
    for (int _i = 0; _i < (dim); ++_i) (a)[_i] -= (b)[_i]
#define vol_sub(c, a, b, dim) \
    for (int _i = 0; _i < (dim); ++_i) (c)[_i] = (a)[_i] - (b)[_i]
#define vol_iadd_v(a, v, dim) \
    for (int _i = 0; _i < (dim); ++_i) (a)[_i] += (v)
#define vol_imax(a, b, dim) \
    for (int _i = 0; _i < (dim); ++_i) iMAX((a)[_i], (b)[_i])

#define vol_set(a, v, dim) \
    for (int i = 0; i < (dim); ++i) (a)[i] = (v)

#if RES_DIM == 2

#define res_le(a, b) (fle((a)[0], (b)[0]) && fle((a)[1], (b)[1]))
#define res_le_precise(a, b) ((a)[0] < (b)[0] && (a)[1] < (b)[1])
#define res_lt(a, b) (flt((a)[0], (b)[0]) && flt((a)[1], (b)[1]))
#define res_eq(a, b) (feq((a)[0], (b)[0]) && feq((a)[1], (b)[1]))
#define res_ineg(a, b) (a)[0] = -(a)[0], (a)[1] = -(a)[1]
#define res_iadd(a, b) (a)[0] += (b)[0], (a)[1] += (b)[1]
#define res_isub(a, b) (a)[0] -= (b)[0], (a)[1] -= (b)[1]
#define res_sub(c, a, b) (c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1]
#define res_iadd_v(a, v) (a)[0] += v, (a)[1] += v
#define res_imax(a, b)    \
    iMAX((a)[0], (b)[0]); \
    iMAX((a)[1], (b)[1])

#define res_set(a, v) (a)[0] = (a)[1] = v

#else

#define res_le(a, b) vol_le(a, b, RES_DIM)
#define res_le_precise(a, b) vol_le_precise(a, b, RES_DIM)
#define res_le(a, b) vol_lt(a, b, RES_DIM)
#define res_eq(a, b) vol_eq(a, b, RES_DIM)
#define res_ineg(a) vol_ineg(a, b, RES_DIM)
#define res_iadd(a, b) vol_iadd(a, , b, RES_DIM)
#define res_isub(a, b) vol_isub(a, b, RES_DIM)
#define res_sub(c, a, b) vol_sub(c, a, b, RES_DIM)
#define res_iadd_v(a, v) vol_iadd_v(a, v, RES_DIM)
#define res_imax(a, b) vol_imax(a, v, RES_DIM)

#define res_set(a, v) vol_set(a, v, RES_DIM)
#endif

#define _div_and_ceil(x, y) (((x) + (y)-1) / (y))

#endif /* ifndef WSMS_COMMON_H_ */
