/* Nagomoci et. al. version of the Nagomoci-Ibaraki min-cut algorithm */
/* see Math. Prog. A VOL. 67, 1994, 325--342 */
/* 
  Chandra Checkri, Stanford U.
  Andrew V. Goldberg NEC Research Inst.
  David Karger, MIT
  Matt Levine,  MIT
  Cliff Stein, Dartmouth U. 
*/

/*  
  This is a preliminary version of the code, intended only to 
  generate inputs for the 5th DIMACS Challenge. 
  Permission to use this code is given only as a generator of 
  priority queue operation sequences.
*/

/* 3/4  deleted all things related to outputing a cut  CS */

/*-------------------------------------------------------------*/
/* This program has been modified from the original to produce */
/* (max) priority queue traces for DIMACS Challenge 5.  Changes*/
/* are marked below with **CH5.  ccm 8/96                      */
/*-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <values.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#define NNULL (node*) NULL
#define InTree 0xFFFFFFFF00000000
#define MAX_INT64 0x7FFFFFFF00000000
#define MASK_PRIO 0xFFFFFFFF00000000

/* If HYBRID is on, disable our PR tests */
#ifdef HYBRID
#define NO_PR
#endif

#include "../../trace_tools.h"
#include "types_ni.h"
#include "heap.h"
#include "heap.c"
/**CH5 comented out #include "timer.c"*/ 

/************************************************ shared macros *******/

#define MAX( x, y ) ( ( (x) > (y) ) ?  x : y )
#define MIN( x, y ) ( ( (x) < (y) ) ? x : y )
#define Reverse( a ) (last_r_arc - ((a) - arcs))
#define ABS( x ) ( (x) >= 0 ) ? (x) : -(x)
#define nNode( i ) ( ( (i) == NULL ) ? -1 : ( (i) - nodes + 1) )
#define N_ARC( a ) ( ( (a) == NULL )? -1 : (a) - arcs )

#define ForAllNodes(v) for ( v = nodes; v != sentinelNode; v++ )
#define ForAllArcs(v,a) for ( a = v -> first; a != NULL; a = a -> next )

#define DeleteArc(v,a) \
{\
  if ( a == v -> first ) v -> first = a -> next;\
  if ( a == v -> last ) v -> last = a -> prev;\
  if ( a -> next != NULL ) a -> next -> prev = a -> prev;\
  if ( a -> prev != NULL ) a -> prev -> next = a -> next;\
}

#define EmptyAStack ( aStack == aTOS )

#define APush( a ) \
{\
   *aTOS = a;\
   aTOS++;\
}

#define APop( a )\
{\
   aTOS--;\
   a = *aTOS;\
}


/************************************* global variables **************/
/* operation counts */

/* int timeStamp; */
int newMinCut;

long numPhases, numScans;
long numPRC;

arc  **aStack;             /* set of arcs to contract */
arc  **aTOS;               /* top of stack pointer */

int64_t minCap;            /* minimum cut capacity seen */
heap   h;

long   input_n,            /* original number of nodes */
       currentN,          /* current number of nodes */
       input_m;            /* number of arcs */

node   *nodes,               /* array of nodes */
       *sentinelNode;       /* next after last */
node   *minV;               /* candidate for mincut */

arc    *arcs,                /* array of arcs */
       *r_arcs,              /* array of Reverse arcs */
       *sentinel_arc,        /* next after last */
       *last_r_arc;

float  t, dt;                /* time, detection time */

arc *lastA;

arc   d_arc;                 /* dummy arc - for technical reasons */

int trace_file;
pq_trace_header header;
pq_op_create op_create;
pq_op_destroy op_destroy;
pq_op_insert op_insert;
pq_op_decrease_key op_decrease_key;
pq_op_find_min op_find_min;
pq_op_delete_min op_delete_min;
pq_op_delete_min op_get_size;

#ifdef HYBRID
node *last_node;		
#endif


#include "parser_ni.c"

void printGraph ()
/* used for debuggeing */
{
  node *i;
  arc *a;

  printf("GRAPH:\n");
  ForAllNodes ( i ) {
    printf("node %d leader %d\n", nNode ( i ), nNode ( i -> leader ));
    ForAllArcs ( i, a ) {
      printf("arc (%d %d) cap %f\n", nNode ( i ), nNode ( a -> head ),
	     a -> cap );
    }
  }
}

node *findLeader ( node *v )
/* do path compression */
{
  if ( v != v -> leader )
    v -> leader = findLeader ( v -> leader );
  return ( v -> leader );
}

void contract ( arc *e )

{
  node *v, *w;


  currentN--;
/*
printf("contracting %d %d\n", nNode(e->head), nNode(Reverse(e)->head));
*/
  v = findLeader ( Reverse ( e ) -> head );
  w = findLeader ( e -> head );

  w -> leader = v;

  v -> last -> next = w -> first;
  w -> first -> prev = v -> last;
  v -> last = w -> last;

}



int64_t computeCap ( node *v)

{
  int64_t ans;
  arc *a;

  ans = 0;

  ForAllArcs ( v, a )
    if ( findLeader ( a -> head ) != v )
      ans += a -> cap;

  return ( ans );
}

void updateMinCut (node *v)
{
  node *w;
  minCap = v->cap;
/**CH5 comented out  dt = timer () -t ;*/

/*  printf("Min cut value = %f\n",minCap); */
  newMinCut++;

#ifdef SAVECUT
  ForAllNodes(w) {
    if (w -> leader == v)
      w->status = 1;
  else
    w-> status = 0;
  }
#endif

}


/* useful for cuts updated by alpha test */
void saveTCut ( )

{
  node *w;

  ForAllNodes ( w )
    if ( (int64_t) (findLeader ( w ) -> key & MASK_PRIO) == (int64_t) InTree )
      w -> status = 1;
    else
      w -> status = 0;
}


void printCut ( )

{
  node *w;

  printf("c one side of the mincut\n");
  ForAllNodes ( w )
    if ( w -> status == 0 )
      printf("n %d\n", nNode ( w ));
}


void deleteExtras ( node *v )
/* delete parallel edges adjacent to v */
/* delete self-loops */

{
  node *w;
  arc *a, *b, *ar;

  ForAllArcs ( v, a ) {
    a -> head = a -> head -> leader;
    a -> head -> auxArc = NULL;
  }

  v -> cap = 0;
  ForAllArcs ( v, a ) {
    w = a -> head;
    if ( w == v ) {
      /* delete self-loop */
     DeleteArc ( v, a );
    }
    else {
      v -> cap += a -> cap;
      if ( w -> auxArc == NULL ) {
	w -> auxArc = a;
      }
      else {
	b = w -> auxArc;
	if ( a == lastA )
	  lastA = b;
	b -> cap += a -> cap;
	Reverse ( b ) -> cap = b -> cap;
	DeleteArc ( v, a );
	DeleteArc ( a -> head, Reverse ( a ) );
      }
    }
    assert(w->leader == w);
  }

  if ( v -> cap < minCap ) updateMinCut(v);
}


void componentSizes ()

{
  node *v;
  long bigSize;

  bigSize = 2 * input_n / currentN;

  ForAllNodes ( v )
    v -> leader -> key += (int64_t) 1 << 32;

  ForAllNodes ( v )
    if ( v -> key >> 32 >= bigSize )
      printf("Big supernode %10d size %10.0f\n", nNode ( v ), v -> key >> 32 );

  ForAllNodes ( v )
    v -> key = v - nodes;
}

void compact ()

{

  node *w;
  arc *a;

  ForAllNodes ( w )
    findLeader ( w );

  /* now there are no leader chains until contract is called */

  ForAllNodes ( w )
    if ( w -> leader == w ) {
      deleteExtras ( w );
      w -> key = w - nodes;
      if ( w -> cap < minCap ) updateMinCut(w);

    }


#ifdef LONG_OUTPUT
  componentSizes ();
#endif
}

void *phase ( node *v )

{
  arc *a;
  node *w, *v1;
  int64_t newKey;
  int64_t alphaP = 0;
  long phaseScans = 0;
  long phaseContracts = 0;

  numPhases++;

  hInsert(h, v);
/***CH5***/
    op_insert.node_id = v - nodes;
    op_insert.item = v - nodes;
    op_insert.key = MAX_INT64 - v->key;
    pq_trace_write_op( trace_file, &op_insert );
    header.op_count++;
    header.node_ids++;
    
  do {

    extractMax ( h, v1 );
/***CH5**/
    pq_trace_write_op( trace_file, &op_delete_min );
    header.op_count++;

    /* scan v */
    phaseScans++;

    /* NOI alpha heuristic */

    alphaP += v1 -> cap -  2 * (v1 -> key>>32);

/***CH5***/
    pq_trace_write_op( trace_file, &op_get_size );
    header.op_count++;

    if ( alphaP < minCap && nonEmptyH( h )) {
      newMinCut++;
      minCap = alphaP;
#ifdef SAVECUT
      saveTCut();
#endif
/**CH5 commented out       dt = timer () - t;*/
    }

    v1 -> key = (int64_t) InTree | (int64_t) (v1 - nodes);
    ForAllArcs ( v1, a ) {
      w = a -> head;
      if ( (int64_t) (w -> key & MASK_PRIO) > (int64_t) InTree ) {
	lastA = a;
	newKey = (w -> key>>32) + a -> cap;
	if (( newKey >= minCap ) && ( minCap > (w -> key>>32) )) {
	  APush ( a );
	}
	if ( (int64_t)(w -> key & MASK_PRIO) == 0 ) {
	  w -> key = (int64_t)(newKey << 32) | (int64_t)(w - nodes);
	  hInsert ( h, w );
/***CH5***/
    op_insert.node_id = w - nodes;
    op_insert.item = w - nodes;
    op_insert.key = MAX_INT64 - w->key;
    pq_trace_write_op( trace_file, &op_insert );
    header.op_count++;
    header.node_ids++;
	  
	}
	else {
	  w -> key = (int64_t)(newKey<<32) | (int64_t)(w-nodes);
	  increaseKey ( h, w, w->key );
/***CH5***/
    op_decrease_key.node_id = w - nodes;
    op_decrease_key.key = MAX_INT64 - w->key;
    pq_trace_write_op( trace_file, &op_decrease_key );
    header.op_count++;
	}
      }
    }

  } while ( nonEmptyH ( h ) );


  if ( phaseScans == currentN ) 
    numScans += phaseScans;
  else {
    fprintf( stderr, ">>> Input graph not connected! (%d,%d)\n",phaseScans,
	currentN);
    exit ( 5 );
  }
    
  if (v1 -> cap < minCap) updateMinCut ( v1 );

  while ( !EmptyAStack ) {
    APop ( a );
#ifdef HYBRID
    last_node = findLeader( Reverse(a) -> head );
#endif
    contract ( a );
    phaseContracts++;
  }

  /* might as well contract the last arc scanned if not contracted */

  if ( findLeader ( lastA -> head ) != 
       findLeader ( Reverse ( lastA ) -> head )) {
#ifdef HYBRID
    last_node = findLeader( Reverse(lastA) -> head );
#endif
    contract ( lastA );
    phaseContracts++;
  }

#ifdef LONG_OUTPUT
  printf("Phase: %d nodes contracted (%d left)\n", 
	 phaseContracts, currentN );
#endif

}

void mainInit ()

{
  int64_t cutCap;
  node *v;

  newMinCut = 0;
/*  timeStamp = 0; */
  makeHeap ( h, input_n );

  aStack = (arc **) calloc ( 2 * input_m + 1, sizeof (arc*));
  if ( aStack == NULL ) {
    fprintf( stderr, "can't obtain enough memory to solve this problem\n");
    exit ( 4 );
  }
  aTOS = aStack;

  currentN = input_n;
  numPhases = 0;
  numScans = 0;

  minCap = MAX_INT64;

  ForAllNodes ( v ) {
    v -> leader = v;
  }

#ifdef HYBRID
  last_node = nodes;
#endif

}

void cutCapInit ()

{
  node *v;
  int64_t bestTrivial;
  node *bestv;

  bestTrivial = minCap;
  ForAllNodes ( v ) {
   v->cap = computeCap ( v );
   if ( v->cap < bestTrivial){
     bestTrivial = v->cap;
     bestv = v;
   }
   
  }
  updateMinCut(bestv);
}


void PrContract ( node * v, node * w )


{
  arc *a, *to_delete;
  arc *ra;
  arc *self_loop;
  node *x;

/*  assumes that v and w are leaders.  */

/*
printf("contracting %d %d\n", nNode(v), nNode(w));
printGraph();
*/

	    
  currentN--;


  /*  scan w's arcs looking for parallel edges and self loops */

  for (a = w->first; a != NULL ; a = a->next) {
    ra = Reverse(a);
    x = a-> head;
    if (x == v){ /* self loop */
      self_loop = a;  /* pointing w to v */

    }
    else if (x->auxArc != NULL) {  /* catch for a vertex that 
				      never had auxArc */ 
      if ((x->auxArc->head == v) /* && (x->status == timeStamp)*/ ){ /* parallel edge, merge earlier edge 
				      into this one */
	to_delete = x->auxArc;

	a->cap += to_delete->cap;
	ra -> cap = a-> cap;
	ra -> head = v;

	x->auxArc = ra;  /* reset auxArc to new one */
      
	DeleteArc(x,to_delete);
	DeleteArc(v,Reverse(to_delete));
	
      }
      else {  /* not a parallel arc */
	x->auxArc = ra;
/*	x->status = timeStamp; */
	ra -> head = v;
      }

    }
    else {  /* not a parallel arc */
      x->auxArc = ra;
/*      x->status = timeStamp; */
      ra -> head = v;
    }
    
  }

/* delete self_loop */

  DeleteArc(v,Reverse(self_loop));

  if (w->first == w->last) { /* handle case when w's only edge is to v */
    w->first = w->last = NULL;
    w->leader = v;
    return;
  }

  DeleteArc(w,self_loop);

/* done deleting self loop */

  if ( v->first == NULL ) { /* catch case when v's neighbors are all w's 
				neighbors too */
    v->first = w->first;
    v->last = w->last;
    w-> leader = v;
    return;
  }

  w -> leader = v;
  v -> last -> next = w -> first;
  w -> first -> prev = v -> last;
  v -> last = w -> last;
}

int PRTest12 ()

{
  node *v, *w;
  arc *a;
  int64_t vCap;
  long PRContracts=0;

  ForAllNodes ( v ) {
/*    timeStamp ++; */


    if ( currentN <= 2 ) break;

    if ( v -> leader == v ) { 

      while (v->first == v->last) {  /* contract singlton chains  */
	w = v->first->head;
	/* printf("singleton contracting %d %d\n", nNode(v), nNode(w)); */
	v->cap += w->cap - 2 * v->first->cap;
	if (v->cap < minCap) updateMinCut(v);
	PrContract(v,w);
	if (currentN <= 2) break;
	PRContracts++;
      }
	if (currentN <= 2) break;

      vCap = v -> cap;

      /* store pointers back to v for merging */
      ForAllArcs(v,a) {
	a->head->auxArc = Reverse(a);
/* 	a->head->status = timeStamp; */
      }

      ForAllArcs ( v, a ) {
	if ( currentN <= 2 ) break;
	w = a->head;  
	
 	assert(w == findLeader ( w ));  /* correctness check,
					      remember to remove */
	if ( v < w ) {  /* to insure linear time */
	  if (( 2 * a -> cap >= MIN(vCap, w -> cap )) || 
	      ( a -> cap >= minCap )) {
	    

	    v -> cap  += w-> cap - 2 * a-> cap;
	    if (v->cap < minCap) updateMinCut(v);  

	    PrContract ( v, w );
	    vCap = v->cap;
	    PRContracts++;

	    
	  }
	}
      }
    }
  }
#ifdef LONG_OUTPUT
    printf("PRT2: %d nodes contracted (%d left)\n", 
	   PRContracts, currentN );    
#endif
    numPRC += PRContracts;
    if ( PRContracts > 0 )
      return ( 1 );
    else
      return ( 0 );
}


#ifdef HYBRID

/* PR tests as implemented by Nagamochi and Ibaraki in their Hybrid algorithm */

long PR_contracts12 = 0;
long PR_contracts34 = 0;

void PR_Hybrid(node *x)
{
  arc *a, *b;
  int flag = 1;
  node *v;
  node *y, *z;
  int64_t bigCap;

v  if ( currentN <= 2 )
    return;

  while ( flag ) 
    {
      flag = 0;
      bigCap = 0.7 * x -> cap / currentN;
      ForAllArcs ( x, a )
	{
	  if (a -> cap < bigCap)
	    continue;
      
	  z = findLeader(a -> head);
	  y = findLeader(Reverse(a) -> head);

	  /* tests 1 and 2 */
	  if ( a->cap >= minCap || ( 2 * a -> cap >= y->cap ) ||
	      (2 * a -> cap >= z -> cap)) 
	    {
	      PR_contracts12++;
	      contract(a);
	      ForAllArcs ( z, b ) 
		Reverse(b) -> head = y;
	    
	      deleteExtras( y ); 
	      x = y;
	      if ( currentN > 2 ) 
		flag = 1;
	      break;
	    }
	  
#ifdef PR_34
	  if ( PR34(a) ) 
	    {
	      PR_contracts34++;
	      contract(a);
	      ForAllArcs ( z, b ) 
		Reverse(b) -> head = y;
	      deleteExtras( y );
	      x = y;
	      if ( currentN > 2 ) 
		flag = 1;
	      break;
	    }
#endif	  
	}
    }
  return;
} 

#ifdef PR_34
/* to test if an arc satisfies the PR tests 3 & 4 */

int PR34(arc *a)
{
  node *x, *y, *z;
  int64_t cap = 0;
  int64_t cap_xy, cap_xz, cap_yz;
  arc *b;
  int flag = 0;

  y = a -> head;
  x = Reverse(a) -> head;
  cap_xy = a -> cap;

  ForAllArcs (y , b)
    b -> head -> auxArc = NULL;

  ForAllArcs ( x , b ) {
    if ( b -> head != y )
      b -> head -> auxArc = b;
  }

  ForAllArcs ( y , b ) {
    z = b -> head;
    if ( z -> auxArc != NULL ) {
      cap_xz = z -> auxArc -> cap;
      cap_yz = b -> cap;
      if ( ( x -> cap <= 2*(cap_xy + cap_xz) ) && ( y -> cap <= 2*(cap_xy + cap_yz) ) ) {
	flag = 1;
	break;
      }

      cap += MIN( cap_xz, cap_yz );
    }
  }
  
/*
  ForAllArcs ( x , b ) {	
    b -> head -> auxArc = NULL; 
  }
*/
  
  if ( flag )
    return 1;
  
  if ( cap_xy + cap >= minCap )
    return 1;
  
  return 0;

}

#endif
#endif

int trace_file;

main (argc, argv )

int   argc;
char *argv[];

{

  arc *a;
  node  *v;
  int beforen;
  int first_iteration = 1;

  header.op_count = 0;
  header.pq_ids = 1;
  header.node_ids = 0;
  op_create.pq_id = 0;
  op_destroy.pq_id = 0;
  op_insert.pq_id = 0;
  op_find_min.pq_id = 0;
  op_delete_min.pq_id = 0;
  op_decrease_key.pq_id = 0;
  op_get_size.pq_id = 0;
  op_create.code = PQ_OP_CREATE;
  op_destroy.code = PQ_OP_DESTROY;
  op_insert.code = PQ_OP_INSERT;
  op_find_min.code = PQ_OP_FIND_MIN;
  op_delete_min.code = PQ_OP_DELETE_MIN;
  op_decrease_key.code = PQ_OP_DECREASE_KEY;
  op_get_size.code = PQ_OP_GET_SIZE;

    trace_file = open( argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU );
    if( trace_file < 0 )
    {
        printf("Failed to open trace file.\n");
        return -1;
    }

    pq_trace_write_header( trace_file, header );
    pq_trace_write_op( trace_file, &op_create );
    header.op_count++;
    
  parse ();
/***CH5**/


  /* initialization */
  mainInit ();
  v = nodes;

/**CH5 printf("c nodes:   %12d    arcs:     %15d\n", input_n, input_m);*/

/**CH5 commented out:  t = timer ();*/

  cutCapInit ();
  /* main loop */
  do {
    compact ();

#ifdef HYBRID
    PR_Hybrid(last_node);
    if ( currentN <= 2 )
      break;
#else
#ifndef NO_PR

    if (first_iteration)
      {
	beforen = currentN * 10;
	while ((currentN <= beforen *.5) && (currentN >= 3)) {
	  beforen = currentN;
	  PRTest12();
	}
/**CH5  printf("c pnodes:  %12d\n",currentN);*/
/**CH5  printf("c ptime: %14.2f\n", timer() - t); */ 

	first_iteration = 0; 
      }
    else
      PRTest12();

    if ( currentN <= 1 )
      break;
#endif
#endif

    v = findLeader ( nodes );
    phase ( v );
    if ( currentN <= 1 )
      break;

  } while ( 1 );

    pq_trace_write_op( trace_file, &op_destroy );
    header.op_count++;
    pq_trace_write_header( trace_file, header );
    pq_trace_flush_buffer( trace_file );
    close( trace_file );


}

