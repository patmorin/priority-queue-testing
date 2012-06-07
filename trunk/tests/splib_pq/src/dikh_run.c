/***********************************************************/
/*                                                         */
/*               Executor of SP codes                      */
/*               (for usual Dijkstra)                      */
/*                                                         */
/***********************************************************/
/*****THIS CODE HAS BEEN MODIFIED TO PRODUCE PRORITY QUEUE **/
/*****TRACES FOR DIMACS CHALLENGE 5. LOOK FOR challenge5   **/
/*****COMMENTS IN THE CODE.  C. McGeoch 7/96                */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/* statistical variables */
long n_scans = 0;
long n_impr = 0;

/* definitions of types: node & arc */

#include "types_dh.h"

/* parser for getting extended DIMACS format input and transforming the
   data to the internal representation */

#include "parser_dh.c"

/* function 'timer()' for mesuring processor time */

#include "timer.c"

/* function for constructing shortest path tree */

#include "dikh.c"


int main (int argc, char** argv)

{

float t;
arc *arp;
node *ndp, *source, *k;
long n, m, nmin; 
char name[21];
uint64_t sum_d = 0;

int trace_file = open( argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU );

 parse( &n, &m, &ndp, &arp, &source, &nmin, name );
/*
printf ( "%s\nn= %ld, m= %ld, nmin= %ld, source = %ld\n",
        name,
        n,m,nmin, (source-ndp)+nmin );

printf ("\nordered arcs:\n");
for ( k = ndp; k< ndp + n; k++ )
  { i = (k-ndp)+nmin;
    for ( ta=k -> first; ta != (k+1)-> first; ta++ )
      printf ( " %2ld %2ld %4ld\n",
               i, ((ta->head)-ndp)+nmin, ta->len
             );

  }
*/
t = timer();

dikh ( trace_file, n, ndp, source );

t = timer() - t;

for ( k= ndp; k< ndp + n; k++ )
  if ( k -> parent != (node*) NULL )
   sum_d += (uint64_t) (k -> dist);

 
#define nd(ptr) (int)(ptr-ndp+nmin)

close( trace_file );

return 0;

/*
for ( k=ndp; k< ndp+n; k++ )
printf (" %d %d %d\n", nd(k), nd(k->parent), k->dist);
*/
}

