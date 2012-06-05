#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dimacs_input.h"

cmdtype cmdtable[7]={"NULL","init","reps","with","seed","comm","prio"};
cmd2type cmd2table[5]={"NUL","ins","dcr","fmn","dmn"};

/*************** cmd_lookup() *****************
* look up init,reps,with,seed,comm commands  */  

int cmd_lookup (cmdtype cmd) {
  int i;
  int go;
  strncpy (cmdtable[0],cmd,4); /*sentinel*/
  go=1;
  for (i=6;go;i--) 
    go=strncmp(cmdtable[i],cmd,3);
  return (i+1);
}
/*************** cmd_lookup2 () ****************/
/* look up 'with' commands                  */
int cmd_lookup2 (cmd2type cmd) {
  int i;
  int go;
  strncpy (cmd2table[0],cmd,3); /*sentinel*/
  go=1;
  for (i=4;go;i--) 
    go=strncmp(cmd2table[i],cmd,3);
  return (i+1);
}


