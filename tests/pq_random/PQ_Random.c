/**********************************************************
 *   
 * PQRandom.c - generates a random set of priority
 * queue function call traces for a DIMACS priority
 * queue driver
 *
 * pq.c - a  basic heap implementation 
 * dimacs_input.c - functions for reading in commands
 *
 * queue items:
 * name - uint32_t : unique,persistent name for each item. 
 * prio - uint64_t: high 32 bits is random priority in [1,MAXPRIO]
 *                  low 32 bits is a copy of the name to ensure unique priorities
 *    
 * Benjamin Chang (bcchang@unix.amherst.edu) 8/96
 * Modified by Dan Larkin (dhlarkin@cs.princeton.edu) 6/12
 *********************************************************/

#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "dimacs_input.h"
#include "pq.h"
#include "../../trace_tools.h"

#define true 1
#define false 0

/* Q is the priority queue */
pq_ptr Q;

int trace_file;

/* newname is an increasing value - each new item gets a unique name */
uint64_t newname=0;

long int seed=0;  /* seed value */
uint64_t Maxprio = 0xFFFFFFFF;  /* Max priority */ 
uint64_t init;    /* number of initial inserts */
uint64_t reps;    /* number of repetitions of main loop */

pq_trace_header header;
pq_op_create op_create;
pq_op_destroy op_destroy;
pq_op_insert op_insert;
pq_op_decrease_key op_decrease_key;
pq_op_find_min op_find_min;
pq_op_delete_min op_delete_min;

/* with[]: flags to determine whether to perform each op. in main loop */
int with[5]={false,false,false,false,false};
cmd2type cmdstable[5]={"NUL","ins","dcr","fmn","dmn"}; 

/****************** my_rand () ***************************************/
/* return integers in [0,range-1] */
uint64_t my_rand(uint64_t range)
{
  double foo;
  foo=((double) drand48() * range);
  return (uint64_t) foo;
}

/**************** dcr_amnt () *********************************************/
/* return a new random priority in [min,prio]
   where min is the current minimum priority and prio is the current prio */
uint64_t dcr_amnt (pr_type prio)
{
 it_type minitem=HeapFindMin(Q);
 uint64_t minprio = ( prioval(Q,minitem) & MASK_PRIO ) >> 32;
 uint64_t realprio = ( prio & MASK_PRIO ) >> 32;
 uint64_t name = prio & MASK_NAME;
 
 uint64_t new=my_rand(realprio-minprio)+minprio;

 return ( new << 32 ) | name;
}

/****************************** DoInsert ***********************************/
void DoInsert ()
{
  in_type info;
  pr_type prio;

  if (Q->size>MAXITEMS) {
          printf ("Too many items.  increase MAXITEMS.\n");
          exit(1);
	} 
  else {
   info=newname; 

   prio=(my_rand(Maxprio)<<32) | newname;
   HeapInsert (Q,info,prio);

    op_insert.node_id = newname;
    op_insert.item = (uint32_t) info;
    op_insert.key = prio;
    pq_trace_write_op( trace_file, &op_insert );

   ++newname;
   header.op_count++;
   header.node_ids++;
 }
}
/**************************** DoDecrease ***********************************/
void DoDecrease ()
{
  it_type item;
  pr_type newprio;
  pr_type oldprio;

  if (Q->size) {
    item=my_rand(Q->size);

    oldprio=Q->data[item].prio;
    newprio=dcr_amnt (oldprio);
    HeapDecreaseKey (Q,item,newprio);
    
    op_decrease_key.node_id = item;
    op_decrease_key.key = newprio;
    pq_trace_write_op( trace_file, &op_decrease_key );

    header.op_count++;
  }
}

/***************************** DoFindMin ********************************/
void DoFindMin ()
{
  HeapFindMin(Q);

    pq_trace_write_op( trace_file, &op_find_min );

    header.op_count++;
}

/*************************** DoDeleteMin () **********************************/
void DoDeleteMin ()
{
  HeapExtractMin(Q);

    pq_trace_write_op( trace_file, &op_delete_min );

    header.op_count++;
}

int main ( int argc, char** argv )
{
  header.op_count = 0;
  header.pq_ids = 1;
  header.node_ids = 0;
  op_create.pq_id = 0;
  op_destroy.pq_id = 0;
  op_insert.pq_id = 0;
  op_find_min.pq_id = 0;
  op_delete_min.pq_id = 0;
  op_decrease_key.pq_id = 0;
  op_create.code = PQ_OP_CREATE;
  op_destroy.code = PQ_OP_DESTROY;
  op_insert.code = PQ_OP_INSERT;
  op_find_min.code = PQ_OP_FIND_MIN;
  op_delete_min.code = PQ_OP_DELETE_MIN;
  op_decrease_key.code = PQ_OP_DECREASE_KEY;
  

  int i;

  int totins, totsize; 
  heap_type heap;
  Q=&heap;
  HeapConstruct (Q);

    // parse cli
    if( argc != 9 )
    {
        printf("Invalid usage.");
        return -1;
    }

    trace_file = open( argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU );
    if( trace_file < 0 )
    {
        printf("Failed to open trace file.\n");
        return -1;
    }

    // spaceholder
    pq_trace_write_header( trace_file, header );


    init = (uint64_t) atoi( argv[2] );
    reps = (uint64_t) atoi( argv[3] );
    with[1] = atoi( argv[4] );
    with[2] = atoi( argv[5] );
    with[3] = atoi( argv[6] );
    with[4] = atoi( argv[7] );
    Maxprio = PQ_MIN( Maxprio, atoi( argv[8] ) );


  /* find total number inserts */
  totins = init;
  if (with[1]) totins += reps;
  /* find max heap size */
  totsize = init;
  if (with[1] && (!with[4])) totsize += reps;
  if ( totsize > MAXITEMS-1 ) {
     printf("Too big. Please recompile with bigger MAXITEMS\n");
     exit(1);
   }

    pq_trace_write_op( trace_file, &op_create );
    header.op_count++;

  seed = (int) time(0); 
  srand48(seed); 
  
  for (i=0;i<init;++i)
    DoInsert ();


  for (i=0;i<reps;++i) {
      if (with[ins_cmd]) {
	DoInsert ();
      }
      if (with[dcr_cmd]) {
	DoDecrease ();
      }
      if (with[fmn_cmd]) {
	DoFindMin ();
      }
      if (with[dmn_cmd]) {
	DoDeleteMin ();
      }
  }/*for */

    pq_trace_write_op( trace_file, &op_destroy );
    header.op_count++;
    pq_trace_write_header( trace_file, header );
    pq_trace_flush_buffer( trace_file );
    close(trace_file);
    free( Q->data );
        
    return 0;
}/* main */
