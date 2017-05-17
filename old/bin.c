//
// Created by zhaomengz on 27/4/17.
//

#include "bin.h"
#include <float.h>
#include <limits.h>
#include <memory.h>


#if BIN_DEMINSION == 1

#define volume_le(tn, res) fle((tn)->allocated[0], (res)[0])
#define volume_eq(xn, yn) feq((xn)->allocated[0], (yn)->allocated[0])
#define volume_iadd(node, demand) ((node)->allocated[0] += (demand)[0])
#define volume_isub(node, demand) ((node)->allocated[0] -= (demand)[0])
#define volume_set(node, x) ((node)->allocated[0] = 0)
#define volume_sub(r0, r1, res) ((res)[0] = (r0)[0] - (r1)[0])

#elif BIN_DEMINSION == 2

#define volume_le(tn, res) \
  (fle((tn)->allocated[0], (res)[0]) && fle((tn)->allocated[1], (res)[1]))

#define volume_eq(xn, yn)                         \
  (feq((xn)->allocated[0], (yn)->allocated[0]) && \
   feq((xn)->allocated[1], (yn)->allocated[1]))

#define volume_iadd(node, demand) \
  ((node)->allocated[0] += (demand)[0], (node)->allocated[1] += (demand)[1])

#define volume_isub(node, demand) \
  ((node)->allocated[0] -= (demand)[0], (node)->allocated[1] -= (demand)[1])

#define volume_set(node, x) ((node)->allocated[0] = (node)->allocated[0] = x)

#define volume_sub(r0, r1, res) \
  ((res)[0] = (r0)[0] - (r1)[0], (res)[1] = (r0)[1] - (r1)[1])

#else

static inline bool volume_le(bin_node_t* tn, volume_t res) {
  for (int i = 0; i < BIN_DEMINSION; ++i)
    if (!fle(tn->allocated[i], res[i]))
      return false;
  return true;
}

static inline bool volume_eq(bin_node_t* xn, bin_node_t* yn) {
  for (int i = 0; i < BIN_DEMINSION; ++i)
    if (fne(xn->allocated[i], yn->allocated[i]))
      return false;
  return true;
}

static inline void volume_iadd(bin_node_t* node, volume_t demand) {
  for (int i = 0; i < BIN_DEMINSION; ++i)
    node->allocated[i] += demand[i];
}

static inline void volume_isub(bin_node_t* node, volume_t demand) {
  for (int i = 0; i < BIN_DEMINSION; ++i)
    node->allocated[i] -= demand[i];
}

static inline void volume_set(bin_node_t* node, double x) {
  for (int i = 0; i < BIN_DEMINSION; ++i)
    node->allocated[i] = x;
}

static inline void volume_sub(volume_t r0, volume_t r1, volume_t res) {
  for (int i = 0; i < BIN_DEMINSION; ++i)
    res[i] = r0[i] - r1[i];
}

#endif  // BIN_DEMINSION == 2

// void bin_print(bin_t* bin, char* prefix) {
//  bin_node_t* tmp = bin->head;
//  printf("%s: ", prefix);
//  do {
//    printf("[%d|%f,%f<%p, %p>]->", tmp->time, tmp->allocated[0],
//           tmp->allocated[1], tmp->list.prev, tmp->list.next);
//    tmp = _next(tmp);
//  } while (tmp != bin->head);
//  printf("\n");
//}
//
// void node_print(bin_node_t* node) {
//  printf("[%d|%f,%f<%p, %p>]->", node->time, node->allocated[0],
//         node->allocated[1], node->list.prev, node->list.next);
//}

bin_node_t* _clone_node(bin_t* bin, bin_node_t* prev_node, int time) {
  bin_node_t* node = (bin_node_t*)mp_alloc(&bin->pool);
  memcpy(node, prev_node, sizeof(bin_node_t));
  node->time = time;
  list_insert_after(&prev_node->list, &node->list);
  return node;
}

void _delete_node(bin_t* bin, bin_node_t* node) {
  list_delete(&node->list);
  mp_free(&bin->pool, node);
}

bin_node_t* _search(bin_t* bin, int time) {
  bin_node_t* node = _prev(bin->head);
  while (node->time > time)
    node = _prev(node);
  return node;
}

bin_node_t* _finger_search(bin_t* bin, int time) {
  bin_node_t* node = bin->finger;
  if (node->time > time) {
    while (node->time > time)
      node = _prev(node);
  } else {
    while (node->time <= time)
      node = _next(node);
    node = _prev(node);
  }
  bin->finger = node;
  return node;
}

void bin_init(bin_t* bin) {
  mp_init(&bin->pool, sizeof(bin_node_t));
  bin->head = (bin_node_t*)mp_alloc(&bin->pool);
  bin_node_t* tail = (bin_node_t*)mp_alloc(&bin->pool);
  bin->head->time = -INT_MAX;
  tail->time = INT_MAX;
  volume_set(bin->head, 0);
  volume_set(tail, DBL_MAX);
  list_init_head(&bin->head->list);
  list_insert_after(&bin->head->list, &tail->list);
  bin->finger = tail;
}

void bin_destroy(bin_t* bin) {
  mp_destroy(&bin->pool);
}

void bin_alloc_space(bin_t* bin, int st, int ft, volume_t demands) {
  //    bin_print(bin, "Before Allocation");
  bin_node_t* node = _finger_search(bin, st);
  if (node->time != st)
    node = _clone_node(bin, node, st);
  volume_iadd(node, demands);
  if (volume_eq(_prev(node), node))
    _delete_node(bin, node);
  node = _next(node);
  while (node->time < ft) {
    volume_iadd(node, demands);
    node = _next(node);
  }
  if (node->time > ft) {
    node = _clone_node(bin, _prev(node), ft);
    volume_isub(node, demands);
  }
  if (volume_eq(node, _next(node)))
    _delete_node(bin, _next(node));
  //    bin_print(bin, "After Allocation");
}

int bin_earliest_slot(bin_t* bin,
                      int st,
                      int length,
                      volume_t demands,
                      volume_t capacities) {
  volume_t tmp_res;
  int ft = st + length;
  bin_node_t* node = _finger_search(bin, st);

  volume_sub(capacities, demands, tmp_res);

  while (node->time < ft) {
    if (volume_le(node, tmp_res)) {
      node = _next(node);
    } else {
      node = _next(node);
      ft = node->time + length;
    }
  }
  return ft - length;
}

int bin_alloc_earliest(bin_t* bin,
                       int st,
                       int length,
                       volume_t demands,
                       volume_t capacity) {
  volume_t tmp_res;
  bin_node_t* tmp = _finger_search(bin, st);
  bin_node_t* node = tmp;
  int ft = st + length;

  volume_sub(capacity, demands, tmp_res);

  while (tmp->time < ft) {
    if (volume_le(tmp, tmp_res)) {
      tmp = _next(tmp);
    } else {
      tmp = _next(tmp);
      ft = tmp->time + length;
      node = tmp;
    }
  }

  st = ft - length;
  if (node->time < st)
    node = _clone_node(bin, node, st);
  volume_iadd(node, demands);
  if (volume_eq(_prev(node), node))
    _delete_node(bin, node);
  node = _next(node);
  while (node->time < ft) {
    volume_iadd(node, demands);
    node = _next(node);
  }
  if (node->time > ft) {
    node = _clone_node(bin, _prev(node), ft);
    volume_isub(node, demands);
  }
  if (volume_eq(node, _next(node)))
    _delete_node(bin, _next(node));

  return st;
}

int bin_open_time(bin_t* bin) {
  bin_node_t* tmp = _next(bin->head);
  return bin_is_empty(bin) ? 0 : tmp->time;
}

int bin_close_time(bin_t* bin) {
  bin_node_t* tmp = _prev(_prev(bin->head));
  return bin_is_empty(bin) ? 0 : tmp->time;
}
