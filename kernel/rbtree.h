#ifndef __RBTREE_H
#define __RBTREE_H

#include <stddef.h>
#include "types.h"

#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr) - offsetof(type, member)))

enum color { RED, BLACK };

struct rb_node {
  struct rb_node *p; // Parent node
  struct rb_node *l; // Left node
  struct rb_node *r; // Right node
  enum color col;    // Color

  // Fields for scheduling
  uint64 vruntime;      // Virtual runtime
  uint64 prev_vruntime; // Previous virtual runtime
  uint64 starttime;     // Time when process starts
  uint64 timeslice;     // Size of timeslice process gets to run
  int nice; // Nice values (-20=>highest priority, 19=>lowest priority)
};

struct rb_tree {
  struct spinlock rb_lock; // Use this before accessing the root
  struct rb_node *root;    // Root of the tree
  struct rb_node *NIL;     // Null node for comparison
  int nproc;               // Number of processes in the tree
  int min_vruntime;        // Smallest virtual runtime
};

extern struct rb_tree cfs_tree;
extern struct rb_node nil_node;

void rb_tree_init();
struct rb_node *leftmost(struct rb_tree *rb, struct rb_node *node);
void insert_proc(struct rb_tree *rb, struct rb_node *node);
void delete_proc(struct rb_tree *rb, struct rb_node *node);

#endif
