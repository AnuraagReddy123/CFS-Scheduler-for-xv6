#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "rbtree.h"

struct rb_tree cfs_tree;
struct rb_node nil_node;

// Initializes the CFS Red-Black tree
void rb_tree_init() {
  nil_node.l = 0;
  nil_node.r = 0;
  nil_node.p = 0;
  nil_node.col = BLACK;
  cfs_tree.NIL = &nil_node;
  cfs_tree.root = cfs_tree.NIL;
  cfs_tree.nproc = 0;
  cfs_tree.min_vruntime = 0;
  initlock(&cfs_tree.rb_lock, "rb_lock");
}

// Left rotation of the passed node in given rb tree
void left_rotate(struct rb_tree *rb, struct rb_node *node) {
  struct rb_node *right = node->r;
  node->r = right->l;
  if (node->r != rb->NIL)
    node->r->p = node;
  right->p = node->p;
  if (node->p == rb->NIL)
    rb->root = right;
  else if (node == node->p->l)
    node->p->l = right;
  else
    node->p->r = right;
  right->l = node;
  node->p = right;
}

// Right rotation of the passed node in given rb tree
void right_rotate(struct rb_tree *rb, struct rb_node *node) {
  struct rb_node *left = node->l;
  node->l = left->r;
  if (node->l != rb->NIL)
    node->l->p = node;
  left->p = node->p;
  if (node->p == rb->NIL)
    rb->root = left;
  else if (node == node->p->l)
    node->p->l = left;
  else
    node->p->r = left;
  left->r = node;
  node->p = left;
}


