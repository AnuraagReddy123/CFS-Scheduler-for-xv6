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

// Replaces one subtree with another
static void rb_transplant(struct rb_tree *rb, struct rb_node *u, struct rb_node *v) {
  if (u->p == rb->NIL)
    rb->root = v;
  else if (u == u->p->l)
    u->p->l = v;
  else
    u->p->r = v;
  v->p = u->p;
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

// Fixes violations caused by deletion in rb tree
static void fixup_delete(struct rb_tree *rb, struct rb_node *node) {
  while (node != rb->root && node->col == BLACK) {
    if (node == node->p->l) {
      struct rb_node *sib = node->p->r;
      if (sib->col == RED) {
        sib->col = BLACK;
        node->p->col = RED;
        left_rotate(rb, node->p);
        sib = node->p->r;
      }
      if (sib->l->col == BLACK && sib->r->col == BLACK) {
        sib->col = RED;
        node = node->p;
      } else {
        if (sib->r->col == BLACK) {
          sib->l->col = BLACK;
          sib->col = RED;
          right_rotate(rb, sib);
          node = rb->root;
        }
      }
    } else {
      struct rb_node *sib = node->p->l;
      if (sib->col == RED) {
        sib->col = BLACK;
        node->p->col = RED;
        right_rotate(rb, node->p);
        sib = node->p->l;
      }
      if (sib->r->col == BLACK && sib->l->col == BLACK) {
        sib->col = RED;
        node = node->p;
      } else {
        if (sib->l->col == BLACK) {
          sib->r->col = BLACK;
          sib->col = RED;
          left_rotate(rb, sib);
          sib = node->p->l;
        }
        sib->col = node->p->col;
        node->p->col = BLACK;
        sib->l->col = BLACK;
        right_rotate(rb, node->p);
        node = rb->root;
      }
    }
  }
  node->col = BLACK;
  cfs_tree.min_vruntime = leftmost(&cfs_tree, cfs_tree.root)->vruntime;
}

// Finds the leftmost node starting from given node
struct rb_node *leftmost(struct rb_tree *rb, struct rb_node *node) {
  while (node != 0 && node != rb->NIL && node->l != rb->NIL)
    node = node->l;
  return node;
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

// Deletes node for a process in the rb tree
void delete_proc(struct rb_tree *rb, struct rb_node *node) {
  struct rb_node *temp = node;
  struct rb_node *child;
  enum color temp_orig_col = temp->col;

  if (node->l == rb->NIL) {
    child = node->r;
    rb_transplant(rb, node, node->r);
  } else if (node->r == rb->NIL){
    child = node->l;
    rb_transplant(rb, node, node->l);
  } else {
    temp = leftmost(rb, node->r);
    temp_orig_col = temp->col;
    child = temp->r;
    if (temp->p == node)
      child->p = node;
    else {
      rb_transplant(rb, temp, temp->r);
      temp->r = node->r;
      temp->r->p = temp;
    }
    rb_transplant(rb, node, temp);
    temp->l = node->l;
    temp->l->p = temp;
    temp->col = node->col;
  }
  if (temp_orig_col == BLACK)
    fixup_delete(rb, child);
  rb->nproc--;
}
