//
// Created by zhaomengz on 11/5/17.
//

#include "common.h"

bool res_richcmp(resources_t* r0, resources_t* r1, int op) {
    bool res;
    switch (op) {
        case 0:
            res = fle(r0->core, r1->core) && fle(r0->memory, r1->memory) &&
                  (fne(r0->core, r1->core) || fne(r1->memory, r1->memory));
            break;
        case 1:
            res = fle(r0->core, r1->core) && fle(r0->memory, r1->memory);
            break;
        case 2:
            res = feq(r0->core, r1->core) && feq(r0->memory, r1->memory);
            break;
        case 3:
            res = fne(r0->core, r1->core) || fne(r0->memory, r1->memory);
            break;
        case 4:
            res = fge(r0->core, r1->core) && fge(r0->memory, r1->memory) &&
                  (fne(r0->core, r1->core) || fne(r0->memory, r1->memory));
            break;
        case 5:
            res = fge(r0->core, r1->core) && fge(r0->memory, r1->memory);
            break;
        default:
            res = false;
    }
    return res;
}

void res_scale(resources_t* r0, resources_t* r1) {
    iMAX(r0->core, r1->core);
    iMAX(r0->memory, r1->memory);
}
