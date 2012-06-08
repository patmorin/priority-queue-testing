#include <stdint.h>

typedef  /* arc */
   struct arc_st
{
   int64_t             cap;             /* capacity */
   struct node_st   *head;           /* head node */
   struct arc_st    *next;           /* next in the arc list */
   struct arc_st    *prev;           /* next in the arc list */
/*   int passPR2;    field to check if node is marked for prtest2 */

}
  arc;

typedef  /* node */
   struct node_st
{
   arc              *first;           /* first outgoing arc */
   arc              *last;           /* last outgoing arc */
   int64_t           key;              /* priority queue key */
   long             heap_pos;         /* heap position */
   struct node_st   *leader;
   arc              *auxArc;          /* used to delete parallel edges */
   int              status;           /* in or out of cut */
   int64_t           cap;	      /* capacity of cut {v} */
} node;

