#ifndef __RBTREE_H
#define __RBTREE_H

#include <stddef.h>
#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr) - offsetof(type, member)))

extern struct rb_tree cfs_tree;

void rb_tree_init();


#endif
