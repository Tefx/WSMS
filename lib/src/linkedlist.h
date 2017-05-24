//
// Created by zhaomengz on 26/4/17.
//

#ifndef BIN_LINKEDLIST_H
#define BIN_LINKEDLIST_H

#include <stddef.h>

typedef struct list_node_t {
    struct list_node_t* prev;
    struct list_node_t* next;
} list_node_t;

#define container_of(ptr, type, member) \
    (type*)((char*)(ptr)-offsetof(type, member))

#define list_conn(p, n) \
    (p)->next = (n);    \
    (n)->prev = (p)

#define list_insert_after(p, node) \
    list_conn(node, (p)->next);    \
    list_conn(p, node)

#define list_delete(node) list_conn((node)->prev, (node)->next)
#define list_init_head(node) (node)->prev = (node)->next = node
#define list_entry(node, type, member) container_of((node), type, member)

#endif  // BIN_LINKEDLIST_H
