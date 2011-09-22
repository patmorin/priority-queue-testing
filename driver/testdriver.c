#include<stdio.h>
#include<stdlib.h> 
#include<sys/time.h>
#include<stdint.h>
#ifdef DUMMY
    // This measures the overhead of processing the input files, which should be 
    // subtracted from all heap time measurements.
    #define construct_pq(...)   NULL
    #define destruct_pq(...)    
    #define clear(...)          
    #define prioval(...)        0.0
    char dummy_val[4] = "xxx";
    #define infoval(...)        (&dummy_val)
    #define size(...)           0
    #define insert(...)         NULL
    #define find_min(...)       NULL
    #define del_min(...)        0.0
    #define decrease_p(...)
    #define PRINT_STATS(...)
    #include <string.h>
    typedef double pr_type;
    typedef char str16[16];
    typedef str16 in_type;
    typedef int* pq_ptr;
    typedef int* it_type;
#else
    #ifdef USE_BINARY_ARRAY
        #include "../queues/binary_array_heap.h"
    #elif defined USE_BINARY_POINTER
        #include "../queues/binary_pointer_heap.h"
    #elif defined USE_FIBONACCI
        #include "../queues/fibonacci_heap.h"
    #elif defined USE_PAIRING
        #include "../queues/pairing_heap.h"
    #elif defined USE_QUAKE
        #include "../queues/quake_heap.h"
    #elif defined USE_RANK_PAIRING
        #include "../queues/rank_pairing_heap.h"
    #elif defined USE_VIOLATION
        #include "../queues/violation_heap.h"
    #endif
    #define construct_pq(M)     create_heap(M)
    #define destruct_pq(a)      destroy_heap(a)
    #define clear(a)            clear_heap(a)
    #define prioval(a,b)        get_key(a,b)
    #define infoval(a,b)        get_item(a,b)
    #define size(a)             get_size(a)
    #define insert(a,b,c)	    insert(a,c,b)
    #define find_min(a)		    find_min(a)
    #define del_min(a) 	        delete_min(a)
    #define decrease_p(a,b,c)   decrease_key(a,b,c)
#endif

/*------------------------------------------------------------*/
/* This is a priority queue driver for the DIMACS tests.  It  */
/* reads command lines and performs procedure calls, and      */
/* records statistics about the run.  This implementation     */ 
/* assumes that in_type is a string and pr_type is a double.  */
/* (But the assumption only shows up in printfs.)  ccm 5/96   */
/*------------------------------------------------------------*/

#define MAXNAMES 10000000        /* Max user names                       */ 
#define LINES       10000        /* Number of lines to process at a time */

it_type *itemindex[MAXNAMES+1];  /* Maps user names (integers) to items  */ 
int         inx;                 /* index to itemindex                   */
int         N;                   /* number of user names                 */
uint32_t    M;                   /* max size of priority queue           */
int         i; 

pq_ptr  Q;                       /* The priority queue                   */
pr_type pri, priority;           /* priorities                           */
it_type *item;                   /* item locator                         */
in_type dummy;                   /* string for info part                 */ 

#define CMD_SIZE 10              /* User command interpreter             */
typedef char cmdtype[4];         
cmdtype cmdtable[CMD_SIZE+1];    /* table of user commands               */
cmdtype cmd;                     /* user command                         */
int count;                       /* num fields in ins input              */

/*-------------tabinit------------------------------------------*/
/* Initializes the table of user commands, and sets defaults    */
void tabinit()
{
    strcpy(cmdtable[0], "sentinal.spot");
    strcpy(cmdtable[1], "pqh");      /* header line*/
    strcpy(cmdtable[2], "com");      /* comment */
    strcpy(cmdtable[3], "ins");      /* insert  */ 
    strcpy(cmdtable[4], "fmn");      /* find min */
    strcpy(cmdtable[5], "dmn");      /* delete min */ 
    strcpy(cmdtable[6], "dcr");      /* decrease priority */ 
    strcpy(cmdtable[7], "prv");      /* prioval */
    strcpy(cmdtable[8], "inv");      /* infoval */
    strcpy(cmdtable[9], "siz");      /* size */
    strcpy(cmdtable[10], "clr");     /* clear */
}/*tabinit*/

/*-------------lookup-------------------------------------------*/
/* Look up user command in command table, return integer        */
int lookup (cmdtype cmd) 
{
    int i;
    int stop;
    strncpy(cmdtable[0], cmd,3);  /*sentinel*/ 
    stop = 1;
    for (i = CMD_SIZE ; stop != 0; i--) {
        stop = strncmp(cmdtable[i], cmd, 3);
    } 
    return(i+1);
}/*lookup*/ 

/*---------main-------------------------------------------------*/
/* Driver reads command lines, calls routines, reports          */
int main()
{
    char buf[LINES][100];  /*input line buffer */
    int index[LINES];
    int lines = LINES - 1;
    int current_line; 

    tabinit();     

    struct timeval t0, t1;
    long totalt = 0;
    char* check;

    while ( lines == LINES - 1 ) {
        lines = 0;
        while ( ( lines < LINES ) && ( scanf("%s", cmd) != EOF ) ) {
            check = fgets (buf[lines], sizeof(buf[lines]), stdin);     
            index[lines] = lookup (cmd);
            lines++;
        }

        gettimeofday(&t0, NULL);
        for ( current_line = 0; current_line < lines; current_line++ ) {
            switch(index[current_line]) {
                //case 0: { printf("%s: Unknown Command\n", cmd);
                //            break;
                //        }
                case 1: { /* pqh */
                            count = sscanf ( buf[current_line], "%d %d", &N, &M);
                            if (N > MAXNAMES) {
                                printf("Too many names, please recompile qdriver.c\n");
                                exit(1);
                            } 

                            /* M is max number of queue elements. Use of M is  */
                            /* a variation from DIMACS specifications.         */ 
                            if (count < 2) {
                                M = 1000; 
                                printf("Default max queue size %d\n", M); 
                            }
                            Q = construct_pq(M);   
                                                  
                            break;
                        }
                case 2: { /*com*/ 
                            break;
                        }
                case 3: { /*ins*/
                            count = sscanf( buf[current_line],  "%lf  %d\n", &pri, &inx);
                            if (count == 0) { printf("Bad ins line\n"); exit(1);} 
                            else if (count == 1) { 
                                inx = 0; 
                                strcpy(dummy, "xxx"); 
                            } 
                            else if (count == 2) {
                                 sprintf(dummy, "xxx%d", inx); 
                            } 
                            if (inx >= MAXNAMES) {
                                printf ("Name too big, please recompile\n");
                                exit(1); 
                            } 
                            item = insert ( Q, pri, dummy); 
                            if (inx != 0) itemindex[inx] = item;
                            break;
                        } 
                case 4: { /*fmn*/
                            item = find_min(Q);
                            break; 
                        }
                case 5: { /*dmn*/
                            pri = (pr_type) del_min(Q);
                            break;
                        }
                case 6: { /* dcr*/
                            sscanf ( buf[current_line] , "%lf  %d", &pri, &inx);
                            item = itemindex[inx];
                            decrease_p (Q, item, pri);
                            break; 
                        }
                case 7:  { /*prv*/
                            sscanf (buf[current_line], "%d", &inx);
                            item = itemindex[inx];
                            pri = (pr_type) prioval (Q, item);
                            break; 
                        }
                case 8:  { /*inv*/
                            sscanf (buf[current_line] ,"%d", &inx);
                            item = itemindex[inx];
                            strcpy(dummy, *infoval(Q, item)); 
                            break;
                        }
                case 9: { /* siz */
                            inx = size(Q);
                            break;
                        } 
                case 10:{ /* clr */
                            clear (Q);
                            break; 
                        }
            }/*end switch*/


        }/*while scanf*/
        gettimeofday(&t1, NULL);
        totalt += (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec);
    }
    
    PRINT_STATS(Q->stats)

	//check_heap(Q);
    destruct_pq(Q); 

	printf("Time: %f\n", totalt / 1000000.0);

}/*main*/ 
