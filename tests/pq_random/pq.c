/****************************************************

  pq.c - a basic heap for use as a priority queue
  pq.h - type definitions and prototypes

  Benjamin Chang (bcchang@unix.amherst.edu) 10/95

*****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "pq.h"

#define parent(i) i>>1          /* parent, left, and right functions */
#define left(i) i<<1            /* for heap, defined as bitwise ops. */
#define right(i) (i<<1) +1

void HeapConstruct (heap_type *H)
{
  H->data = malloc(sizeof(cell)*MAXITEMS);
  H->size=0;
  H->data[0].prio=0;
  H->data[0].name=0xFFFFFFFF;
}
void HeapNodeExchange (heap_type *A,int x,int y)
{
  cell dummy;
  dummy=A->data[x];
  A->data[x]=A->data[y];
  A->data[y]=dummy;
}

void Heapify (heap_type *A,int i)
{
 int l,r,smallest;

 l=left(i);
 r=right(i);
 if ( l<=A->size && A->data[l].prio<A->data[i].prio )
   smallest=l;
 else smallest=i;

 if ( r<=A->size && A->data[r].prio<A->data[smallest].prio)
   smallest=r;
 if (smallest != i)
   {
     HeapNodeExchange (A,i,smallest);
     Heapify (A,smallest);
   }
}

void printheap (heap_type *A)
{
  int i;
  for (i=1;i<=A->size;++i)
    printf ("%d:%lu %llu\n",i,(long unsigned int)A->data[i].name,(long long unsigned int)A->data[i].prio);
}

pr_type prioval (heap_type *H,it_type x)
{
  return H->data[x].prio;
}
in_type infoval (heap_type *H,it_type x)
{
  return H->data[x].name;
}

pr_type HeapExtractMin (heap_type *A)
{
  pr_type min;
  if (A->size<1)
    printf ("heap underflow.\n");
  min=A->data[1].prio;
  A->data[1]=A->data[A->size];
  --A->size;
  Heapify (A,1);
  return min;
}

it_type HeapFindMin (heap_type *A)
{
  return 1;
}

it_type HeapInsert (heap_type *A,in_type name,pr_type key)
{
  it_type i;
  ++A->size;
  i=A->size;
  while (i>1 && A->data[parent(i)].prio>key)
    {
      A->data[i]=A->data[parent(i)];
      i=parent(i);
    }
  A->data[i].prio=key;
  A->data[i].name=name;
  return i;
}
void HeapDecreaseKey (heap_type *A, it_type node, pr_type key)
{
  int x;

  if (A->data[node].prio<key)
    return;

  x=node;
  A->data[x].prio=key;
  while (A->data[x].prio<A->data[parent(x)].prio)
    {
      HeapNodeExchange (A,x,parent(x));
      x=parent(x);
    }
}












