#ifndef WSMS_COMMON_H
#define WSMS_COMMON_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <math.h>
#include <stdbool.h>

#define EPSILON 0.001
#define feq(x, y) (fabs((x) - (y)) <= EPSILON)
#define fne(x, y) (fabs((x) - (y)) > EPSILON)
#define fle(x, y) ((x) <= ((y) + EPSILON))
#define flt(x, y) ((x) < ((y)-EPSILON))
#define fge(x, y) ((x) >= ((y)-EPSILON))
#define fgt(x, y) ((x) > ((y) + EPSILON))

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define iMAX(x, y) \
    if ((x) < (y)) (x) = (y)
#define iMIN(x, y) \
    if ((x) > (y)) (x) = (y)

typedef struct resources_t {
    double core;
    double memory;
} resources_t;

bool res_richcmp(resources_t* r0, resources_t* r1, int op);
void res_scale(resources_t* r0, resources_t* r1);

#define res_iadd(x, y) ((x).core += (y).core, (x).memory += (y).memory)
#define res_isub(x, y) ((x).core -= (y).core, (x).memory -= (y).memory)

#define _div_and_ceil(x, y) (((x) + (y)-1) / (y))

#endif /* ifndef WSMS_COMMON_H_ */
