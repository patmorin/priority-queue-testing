typedef struct heap_st
{
   long              size;          /* the number of the last heap element */
   node            **node;         /* heap of the pointers to nodes       */ 
} 
   heap;

long h_current_pos,
     h_new_pos,
     h_pos,
     h_last_pos;

node *node_j,
     *node_k;

double key_k,
       key_min;

