#define HEAP_DEGREE  3
#define NILL        -1

/* internal definition */
#define PUT_TO_POS_IN_HEAP( h, node_i, pos )\
{\
h.node[pos]        = node_i;\
node_i -> heap_pos = pos;\
}

#define makeHeap( h, n )\
{\
h.size = 0;\
h.node = (node **) calloc (( n+1 ), sizeof(node*));\
}

#define nonEmptyH( h )  ( h.size > 0 )

#define NODE_IN_HEAP( node_i ) ( node_i -> heap_pos != NILL )

#define increaseKey( h, node_i, key_i ) \
{\
for ( h_current_pos =  node_i -> heap_pos;\
      h_current_pos > 0;\
      h_current_pos = h_new_pos\
    )\
      {\
        h_new_pos = ( h_current_pos - 1 ) / HEAP_DEGREE;\
\
        node_j = h.node[h_new_pos];\
        if ( key_i  <=  node_j -> key ) break;\
\
        PUT_TO_POS_IN_HEAP ( h, node_j, h_current_pos )\
      }\
\
PUT_TO_POS_IN_HEAP ( h, node_i, h_current_pos )\
}

#define hInsert( h, node_i )\
{\
PUT_TO_POS_IN_HEAP ( h, node_i, h.size )\
h.size ++;\
increaseKey ( h, node_i, node_i -> key );\
}

#define extractMax( h, node_0 ) \
{\
node_0             = h.node[0];\
node_0 -> heap_pos = NILL;\
\
h.size -- ;\
\
if ( h.size > 0 )\
  {\
     node_k =  h.node [ h.size ];\
     key_k =  node_k -> key;\
\
     h_current_pos = 0;\
\
     while ( 1 )\
       {\
         h_new_pos = h_current_pos * HEAP_DEGREE  +  1;\
         if ( h_new_pos >= h.size ) break;\
\
         key_min  = h.node[h_new_pos] -> key;\
\
         h_last_pos  = h_new_pos + HEAP_DEGREE;\
	 if ( h_last_pos > h.size ) h_last_pos = h.size;\
\
         for ( h_pos = h_new_pos + 1; h_pos < h_last_pos; h_pos ++ )\
            {\
 	      if ( h.node[h_pos] -> key > key_min )\
		{\
		  h_new_pos = h_pos;\
		  key_min  = h.node[h_pos] -> key;\
		}\
	    }\
\
         if ( key_k >= key_min ) break;\
\
         PUT_TO_POS_IN_HEAP ( h, h.node[h_new_pos], h_current_pos )\
\
         h_current_pos = h_new_pos;\
       }\
\
    PUT_TO_POS_IN_HEAP ( h, node_k, h_current_pos )\
  }\
}

