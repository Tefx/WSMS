//
// Created by zhaomengz on 11/5/17.
//

#include "common.h"

bool res_richcmp(res_t r0, res_t r1, int op) {
    bool res;
    switch (op) {
        case 0:
            res = fle(r0[0], r1[0]) && fle(r0[1], r1[1]) &&
                  (fne(r0[0], r1[0]) || fne(r1[1], r1[1]));
            break;
        case 1:
            res = fle(r0[0], r1[0]) && fle(r0[1], r1[1]);
            break;
        case 2:
            res = feq(r0[0], r1[0]) && feq(r0[1], r1[1]);
            break;
        case 3:
            res = fne(r0[0], r1[0]) || fne(r0[1], r1[1]);
            break;
        case 4:
            res = fge(r0[0], r1[0]) && fge(r0[1], r1[1]) &&
                  (fne(r0[0], r1[0]) || fne(r0[1], r1[1]));
            break;
        case 5:
            res = fge(r0[0], r1[0]) && fge(r0[1], r1[1]);
            break;
        default:
            res = false;
    }
    return res;
}
