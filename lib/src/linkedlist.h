//
// Created by zhaomengz on 26/4/17.
//

#ifndef BIN_LINKEDLIST_H
#define BIN_LINKEDLIST_H

#include <stddef.h>

#define container_of(ptr, type, member) \
  (type*)((char*)(ptr)-offsetof(type, member))

typedef struct list_node_t {
  struct list_node_t* prev;
  struct list_node_t* next;
} list_node_t;

static inline void list_conn(list_node_t* prev, list_node_t* next) {
  prev->next = next;
  next->prev = prev;
}

static inline void list_insert_after(list_node_t* prev, list_node_t* node) {
  list_conn(node, prev->next);
  list_conn(prev, node);
}

static inline void list_delete(list_node_t* node) {
  list_conn(node->prev, node->next);
}

static inline void list_init_head(list_node_t* node) {
  node->prev = node->next = node;
}

#define list_entry(node, type, member) container_of((node), type, member)

#endif  // BIN_LINKEDLIST_H
