/* defs.h */

#ifndef DIKH_TYPES_DH
#define DIKH_TYPES_DH

#include <stdint.h>

typedef  /* arc */
   struct arc_st
{
   uint32_t              len;            /* length of the arc */
   struct node_st   *head;           /* head node */
}
  arc;

typedef  /* node */
   struct node_st
{
   arc              *first;           /* first outgoing arc */
   uint64_t          dist;	      /* tentative shortest path length */
   struct node_st   *parent;          /* parent pointer */
   long              heap_pos;        /* number of position in the heap */
   uint64_t          temp;            /* for temporary labels */
} node;

#endif
