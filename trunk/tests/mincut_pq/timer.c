/* timing function
 * Author: Matthew Levine (mslevine@theory.lcs.mit.edu)
 * $Id: timer.c,v 1.3 1996/06/19 17:17:14 mincut Exp $ */

#include "timer.h"

float timer ()
{
  struct rusage r;

  getrusage(0, &r);
  return (float)(r.ru_utime.tv_sec+r.ru_utime.tv_usec/(float)1000000);
}
