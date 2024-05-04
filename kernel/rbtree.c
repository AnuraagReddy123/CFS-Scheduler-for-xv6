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
static void left_rotate(struct rb_tree *rb, struct rb_node *node) {
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
static void right_rotate(struct rb_tree *rb, struct rb_node *node) {
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

// Fixes violations caused by insertion in rb tree
static void fixup_insert(struct rb_tree *rb, struct rb_node *node) {
  while (node->p->col == RED) {
    if (node->p == node->p->p->l) {
      struct rb_node *temp = node->p->p->r;

      if (temp->col == RED) {
        node->p->col = BLACK;
        temp->col = BLACK;
        node->p->p->col = RED;
        node = node->p->p;
      } else {
        if (node == node->p->r) {
          node = node->p;
          left_rotate(rb, node);
        }
        node->p->col = BLACK;
        node->p->p->col = RED;
        right_rotate(rb, node->p->p);
      }
    } else {
      struct rb_node *temp = node->p->p->l;

      if (temp->col == RED) {
        node->p->col = BLACK;
        temp->col = BLACK;
        node->p->p->col = RED;
        node = node->p->p;
      } else {
        if (node == node->p->l) {
          node = node->p;
          right_rotate(rb, node);
        }
        node->p->col = BLACK;
        node->p->p->col = RED;
        left_rotate(rb, node->p->p);
      }
    }
  }
  rb->root->col = BLACK;
}

// Inserts node for a process into the rb tree
void insert_proc(struct rb_tree *rb, struct rb_node *node) {
  struct rb_node *par = rb->NIL;
  struct rb_node *temp = rb->root;

  if (rb->min_vruntime > node->vruntime)
    rb->min_vruntime = node->vruntime;

  while (temp != rb->NIL) {
    par = temp;
    if (node->vruntime <= temp->vruntime)
      temp = temp->l;
    else
      temp = temp->r;
  }

  node->p = par;

  if (par == rb->NIL)
    rb->root = node;
  else if (node->vruntime <= par->vruntime)
    par->l = node;
  else
    par->r = node;

  node->r = rb->NIL;
  node->l = rb->NIL;

  fixup_insert(rb, node);
  rb->nproc++;
}
