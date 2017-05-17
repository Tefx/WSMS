#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpool.h"
#include "linkedlist.h"
#include "problem.h"

typedef struct foo { int bar; } foo;

void test_mempool() {
  mempool_t pool;
  foo *fp, *fp2;
  int times = 10000;
  clock_t start, diff;

  pool = mp_init(sizeof(foo));
  start = clock();
  for (int i = 0; i < times; ++i) {
    for (int j = 0; j < 10000; ++j) {
      fp = mp_alloc(&pool);
      fp->bar = 1;
      mp_free(&pool, fp);
    }
  }
  mp_destroy(&pool);
  diff = clock() - start;
  printf("Using %f seconds\n", (float)diff / CLOCKS_PER_SEC);

  fp = malloc(sizeof(fp));
  start = clock();
  for (int i = 0; i < times; ++i) {
    for (int j = 0; j < 10000; ++j) {
      fp2 = fp;
      fp = malloc(sizeof(fp));
      fp->bar = 1;
      free(fp2);
    }
  }
  diff = clock() - start;
  printf("Using %f seconds\n", (float)diff / CLOCKS_PER_SEC);
}

typedef struct foo_node {
  int no;
  list_node_t list;
} foo_node;

#define foo_entry(pos) list_entry(pos, foo_node, list)

void test_linkedlist() {
  foo_node foo_head;
  foo_head.no = -1;
  list_init_head(&foo_head.list);

  mempool_t pool = mp_init(sizeof(foo_node));

  foo_node* node = NULL;
  for (int i = 0; i < 10; ++i) {
    //        node = (foo_node*)malloc(sizeof(struct foo_node));
    node = (foo_node*)mp_alloc(&pool);
    node->no = i;
    list_insert_after(&foo_head.list, &node->list);
  }

  //    for (list_node_t* pos = foo_head.head.next;
  //         pos != &foo_head.head;
  //         pos = pos->next) {
  //        node = list_entry(pos, foo_node, head);
  //        node->no++;
  ////        printf("%d:", node->no);
  //    }

  list_node_t* pos;
  list_for_each(pos, &foo_head.list) {
    node = foo_entry(pos);
    node->no++;
    printf("%d:", node->no);
  }
  printf("\b\n");

  pos = foo_head.list.next;
  while (!list_is_end(pos, &foo_head.list)) {
    node = foo_entry(pos);
    printf("%d:", node->no);
    pos = pos->next;
  }
  printf("\b\n");

  node = foo_entry(foo_head.list.next);
  list_while(node, &foo_head.list, foo_node, list, node->no >= 5) {
    printf("%d:", node->no);
  }
  printf("\b\n");
  mp_destroy(&pool);
}

int main() {
  clock_t start, diff;
  start = clock();
  test_linkedlist();
  diff = clock() - start;
  printf("Using %f seconds\n", (float)diff / CLOCKS_PER_SEC);
}
