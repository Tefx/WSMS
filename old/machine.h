//
// Created by zhaomengz on 11/5/17.
//

#ifndef WMSM_MACHINE_H
#define WMSM_MACHINE_H

#include "common.h"

#ifndef BIN_DEMINSION
#define BIN_DEMINSION 2
#include "bin.h"
#undef BIN_DEMINSION
#else
#include "bin.h"
#endif

typedef struct machine_t {
  int type_id;
  bin_t* bin;
} machine_t;

void machine_init(machine_t* machine, int type_id);
void machine_free(machine_t* machine);
int machine_open_time(machine_t* machine);
int machine_close_time(machine_t* machine);
int machine_runtime(machine_t* machine);
int machine_alloc_earliest(machine_t* machine,
                           int est,
                           int rt,
                           resources_t* demands,
                           resources_t* capacities);

#endif  // WMSM_LIB_MACHINE_H
