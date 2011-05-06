/* A little function to return times supposedly accurate to microseconds.
 * Subtracting the value of two calls to this function should give an accurate
 * measure of the time between the calls. 
 * Note that only processor time spent on THIS process is counted. This is 
 * not a wall clock. 
 *
 * Author: Matthew Levine (mslevine@theory.lcs.mit.edu)
 * $Id: timer.h,v 1.2 1996/06/19 17:16:39 mincut Exp $ */

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <sys/resource.h>

float timer();

extern inline float timer()
{
  struct rusage r;

  getrusage(0, &r);
  return (float)(r.ru_utime.tv_sec+r.ru_utime.tv_usec/(float)1000000);
}

#endif
