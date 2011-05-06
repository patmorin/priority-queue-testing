#define MAXITEMS 100002
#define MAXNAMES 100002

typedef int it_type;
typedef double pr_type;
typedef long in_type;

typedef struct {
  in_type name;
  pr_type prio;
} cell;

typedef struct {
  long size;
  cell data[MAXITEMS];
} heap_type;

typedef heap_type *pq_ptr;


pr_type HeapExtractMin (heap_type *);
pr_type prioval (heap_type *,it_type);
in_type infoval (heap_type *,it_type);
void HeapConstruct (heap_type *H);
void HeapNodeExchange (heap_type *A,int x,int y);
void Heapify (heap_type *A,int i);
void printheap (heap_type *A);
it_type HeapInsert (heap_type *A,in_type name,pr_type key);
void HeapDecreaseKey (heap_type *A, it_type node, pr_type key);


