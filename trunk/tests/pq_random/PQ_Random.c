/**********************************************************
   
  PQRandom.c - generates a random set of priority
   queue function call traces for a DIMACS priority
   queue driver
 
  pq.c - a  basic heap implementation 
  dimacs_input.c - functions for reading in commands

  queue items:
  name - long : unique,persistent name for each item. 
  prio - double value: integer part is random priority in [1,MAXPRIO]
                       decimal extension is given to ensure unique priorities
		       extension = name / MAXNAMES
     
  Benjamin Chang (bcchang@unix.amherst.edu) 8/96

*********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "dimacs_input.h"
#include "pq.h"

#define true 1
#define false 0


/* Q is the priority queue */
pq_ptr Q;

/* newname is an increasing value - each new item gets a unique name */
int newname=1;

long seed=0;  /* seed value */
int Maxprio;  /* Max priority */ 
int init;    /* number of initial inserts */
int reps;    /* number of repetitions of main loop */

/* with[]: flags to determine whether to perform each op. in main loop */
int with[5]={false,false,false,false,false};
cmd2type cmdstable[5]={"NUL","ins","dcr","fmn","dmn"}; 

/****************** my_rand () ***************************************/
/* return integers in [1,range] */
int my_rand(int range)
{
  double foo;
  foo=((double) drand48() * range)+1.0;
  return (int) foo;
}

/***************** dec_ext () ****************************************/
/* create a decimal extension based on the item's name so it will have 
   a unique priority */
double dec_ext (int x)
{
  double v2;
  v2=(double) x / MAXNAMES ;
  return v2;
}

/**************** dcr_amnt () *********************************************/
/* return a new random priority in [min,prio]
   where min is the current minimum priority and prio is the current prio */
double dcr_amnt (pr_type prio)
{
 it_type minitem=HeapFindMin(Q);
 int minprio=(int)prioval(Q,minitem);
 int realprio=(int) prio;
 double extension=prio-realprio;
 
 int new=my_rand(realprio-minprio)+minprio;

 return (double) new+extension;
}

/****************************** DoInsert ***********************************/
void DoInsert ()
{
  in_type info;
  pr_type prio;
  it_type it;

  if (Q->size>MAXITEMS) {
          printf ("Too many items.  increase MAXITEMS.\n");
          exit(1);
	} 
  else {
   info=newname; 

   prio=(pr_type)my_rand(Maxprio)+dec_ext(newname);
   it=HeapInsert (Q,info,prio);
   /*printf ("insert: [%d] %d with priority %f. \n",it,newname,prio);*/
   printf ("ins %.9lf %ld\n",prio,info);
   ++newname;
 }
}
/**************************** DoDecrease ***********************************/
void DoDecrease ()
{
  it_type item;
  pr_type newprio;
  pr_type oldprio;
  in_type info;

  if (Q->size) {
    item=my_rand(Q->size-1)+1;

    oldprio=Q->data[item].prio;
    newprio=dcr_amnt (oldprio);
    info=infoval (Q,item);
    HeapDecreaseKey (Q,item,newprio);
/*    printf ("decrease: index:%d name:%ld from %f to priority %f.\n",item,info,oldprio,newprio); */
    printf ("dcr %f %ld\n",newprio,info);
  }
}

/***************************** DoFindMin ********************************/
void DoFindMin ()
{
  it_type item=HeapFindMin(Q);
  pr_type prio=Q->data[item].prio;
  in_type name=Q->data[item].name;
  /*printf ("findmin: %d  %ld %f\n",item,name,prio);*/
  printf ("fmn\n");
}

/*************************** DoDeleteMin () **********************************/
void DoDeleteMin ()
{
  it_type item;
  in_type info;
  in_type newinfo;
  pr_type prio;
  int ix,name;

  item=HeapFindMin(Q);
  info=infoval (Q,item);
  prio=HeapExtractMin (Q);
  /*printf ("delete min: [%d] name=%d with priority %f\n",item,info,prio);*/
  printf ("dmn\n");
}

/***************** ReadInput () ***************************/
/* read and parse commands from stdin                     */
void ReadInput () {
  int i;
  char buf[100];
  int index;
  cmdtype cmd;  /* 4 letter command */
  cmd2type cmd2; /* 3 letter 'with' commands */
  int w;
 
 
  Maxprio = 100; /* default */ 
 
  while (scanf("%s",cmd) !=EOF) {
    fgets (buf,sizeof(buf),stdin);
    index=cmd_lookup (cmd);


    switch (index) {
    case 0: printf ("unknown command %s\n",cmd);
      break;
    case init_cmd:
      sscanf (buf,"%d",&init);
      break;
    case reps_cmd:
      sscanf (buf,"%d",&reps);
      break;
    case with_cmd:
      sscanf (buf,"%s",cmd2);
      w=cmd_lookup2 (cmd2);
      with[w]=true;
      break;
    case seed_cmd:
      sscanf (buf,"%ld",&seed);
      break;
    case comm_cmd:
      printf ("com %s",buf);
      break;
    case prio_cmd: 
      sscanf (buf, "%d", &Maxprio);
      break; 
    }
  }
} /* ReadInput () */

main ()
{
  int i,cmd=0;

  int totins, totsize; 
  pr_type pr;
  heap_type heap;
  Q=&heap;
  HeapConstruct (Q);

  ReadInput ();
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
  printf ("pqh %d %d\n", totins, totsize); 
  printf ("com ## Init=%d, Reps=%d \n",init, reps);
  printf ("com ## With= "); 
  for (i=1;i<=4;++i) if (with[i]) printf ("%.3s ",cmdstable[i]);
  printf("\n"); 

  printf ("com ## Max Priority= %d (+epsilon)\n", Maxprio); 

  if (!seed) seed = (int) time(0); 
  srand48(seed); 
  printf ("com ## Seed: %ld\n",seed);

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


}/* main */
