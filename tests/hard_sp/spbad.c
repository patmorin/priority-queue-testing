/* A generator of hard graph instances for Dijkstra's SSP algorithm */
/* (lots of decrease-keys).  The graph format is compatible with   */
/* SPLIB.  Contributed by A. V. Goldberg to DIMACS Challenge 5.    */
/*  ccm 7/96                                                       */

/* 
    The source is connected to all other nodes to fill up the heap.
    There is a cycle of unit weight arcs.
    Additional arcs are added at random to make each node degree d.
    Length of these arcs is computed using the ARCLEN macro;
    it is a function of the number of arcs from i to j along the cycle.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <math.h>

#include "random.c"

#define DASH '-'
#define VERY_FAR 100000000

double sqrt(double);  /* you can't define math fn. prototypes too many times */
double ceil(double);  /* I've found this out through hard experience */

#define ARCLEN(k)   /* arclen between nodes i and j if (i-j) = k */ \
      ( (long)(ceil(2.0 * k * sqrt((double)(k)))) )

#define PRINT_ARC( i, j, length )\
{\
   printf ("a %8ld %8ld %12ld\n", i, j, length );\
}

/* generator of shortest path problems 
   generats problems requiring many decrease-key operations in Dijkstra's algo.
   extended DIMACS format for output */

main ( argc, argv )

int argc;
char* argv[];


{

  char   args[30];

  long   n,
         source,
         i,
         j,
         np,
         dij;

  long   m,
         d,
         k;

  long   lx, h, ch;

  long   *head;

  long   seed;


  /* parsing  parameters */

  if ( argc < 2 ) goto usage;

  strcpy ( args, argv[1] );

  if (( args[0] == DASH ) && ( args[1] == 'h'))
      goto help;

  if (argc > 4) goto usage;

  /* first parameter - number of nodes */
  np = 1;
  if (( n = atoi ( argv[1] ))  <  2)  goto usage;

  /* second parameter - degree */
  np = 2;
  if (( d = atoi ( argv[2] ) )  <  2  )  goto usage;

  /* other parameters */

  lx = 1; /* default */
  seed = 31415; /* default */
 
 for (np = 3; np < argc; np++) {
    strcpy ( args, argv[np] );
    if (args[0] != DASH) goto usage;
    if (args[1] == 'l')
      lx  = (long ) atof ( &args[2] );
    else if (args[1] == 's') {
      seed  = (long ) atof ( &args[2] );
     }
    else
      goto usage;
  }

  /* ----- ajusting parameters ----- */

  if (d >= n) goto usage;
  m = n*d;

  /* allocate space */
  head = (long *) calloc(n+1, sizeof(long));
  

  /* start output */
  printf ("c bad problem for Dijkstra's algorithm\n");
  printf ("c extended DIMACS format\nc\n" );

  printf ("p sp %8ld %8ld\nc\n", n, m );

  source = 1;
  printf ("n %8ld\nc\n", source );

  /* generate cycle */
  for (i = 1; i < n; i++)
    PRINT_ARC(i, i+1, lx);
  PRINT_ARC(n, 1, lx);

  /* generate other arcs */
  for (i = 1; i <= n; i++) {
    /* initialize */
    init_rand(seed);

    /* generate arc heads */
    /* cases when d <= n/2 and d > n/2 are different for efficiency reasons */
    if (d >= n/2) {

      for (k = 1; k <= n; k++)
	 head[k] = 0;

      /* cycle arc */
      if (i < n) 
	 ch = i+1;
      else 
	 ch = 1;

      for (k = 2; k <= d; k++) {
	 do {
	      j = 1 + nrand(n);
	      } while ((j == i) || (j == ch) || (head[j] == 1));
	 head[j] = 1;
      }
    }
    else {
      for (k = 1; k <= n; k++)
	 head[k] = 1;

      /* cycle arc */
      if (i < n) 
	 ch = i+1;
      else 
	 ch = 1;

      /* delete n-d arcs */
      head[i] = 0;
      head[ch] = 0; /* already exists */
      for (k = 2; k <= n-d; k++) {
	 do {
	      j = 1 + nrand(n);
	      } while (head[j] == 0);
	 head[j] = 0;
      }
    }

    for  (k = 1; k <= n; k++) 
      if (head[k] == 1) {
	 if (i < k)
	      dij = k - i;
	 else
	      dij = n - (i-k);
	 PRINT_ARC(i, k, lx*ARCLEN(dij));
      }
  }

  /* all is done */
  exit (0);

  /* ----- wrong usage ----- */

 usage:
  fprintf ( stderr,
	      "\nusage: %s  n  d [-l#i] [-s#i]\n help:  %s -h\n must have n > 1, 1 <= d < n\n\n", argv[0], argv[0]);

  exit (4);

  /* ---- help ---- */

 help:

  fprintf ( stderr, 
	      "\n'%s' - Dijkstra-worst-case shortest path problem generator.\n Generates problems in extended DIMACS format.\n \n %s  n d [ -l#i] \n #i - integer number   #f - real number\n \n n     - number of nodes, must be 2 or more\n, d     - out-degree, must be 1 or more\n, -l#i  - arc lengths multiplier         (default 1)\n -s#i  - seed \n",
argv[0], argv[0], argv[0] );

exit (0);

}

