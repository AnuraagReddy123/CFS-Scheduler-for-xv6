#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

enum color { RED, BLACK };

struct rb_node {
  struct rb_node *pn; // Parent node
  struct rb_node *ln; // Left node
  struct rb_node *rn; // Right node
  enum color col;     // Color

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

struct rb_tree cfs_tree;
struct rb_node nil_node;

void rb_tree_init() {
  nil_node.ln = 0;
  nil_node.rn = 0;
  nil_node.pn = 0;
  nil_node.col = BLACK;
  cfs_tree.NIL = &nil_node;
  cfs_tree.root = cfs_tree.NIL;
  cfs_tree.nproc = 0;
  cfs_tree.min_vruntime = 0;
  initlock(&cfs_tree.rb_lock, "rb_lock");
}
