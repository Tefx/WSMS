#ifndef WSMS_COMMON_H
#define WSMS_COMMON_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <math.h>
#include <stdbool.h>

#define RES_DIM 2
#define LIM_DIM 1

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
#define iMAX(x, y) \
    if ((x) < (y)) (x) = (y)
#define iMIN(x, y) \
    if ((x) > (y)) (x) = (y)

typedef vlen_t res_t[RES_DIM];
typedef vlen_t plim_t[LIM_DIM];

bool res_richcmp(res_t r0, res_t r1, int op);
#define res_scale(res0, res1) ((res0)[0] += (res1)[0], (res0)[1] += (res1)[1])
#define res_iadd(x, y) ((x)[0] += (y)[0], (x)[1] += (y)[1])
#define res_isub(x, y) ((x)[0] -= (y)[0], (x)[1] -= (y)[1])

#define _div_and_ceil(x, y) (((x) + (y)-1) / (y))

#endif /* ifndef WSMS_COMMON_H_ */
