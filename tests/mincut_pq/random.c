#include "random.h"
#include <stdio.h>

/*** This is a portable random number generator whose origins are
 *** unknown.  As far as can be told, this is public domain software.

/*** portable random number generator */

/*** Note that every variable used here must have at least 31 bits
 *** of precision, exclusive of sign.  Long integers should be enough.
 *** The generator is the congruential:  i = 7**5 * i mod (2^31-1).
 ***/

#define MULTIPLIER 16807
#define MODULUS    2147483647

#ifdef NDEBUG
static long saved_seed = 123456; /*default seed*/
static int no_seed=0;
#else
static long saved_seed;
static int no_seed=1;
#endif


/*** set_random - initialize constants and seed */

void SetRandom(seed)
long seed;
{
  saved_seed = seed;
  no_seed=0;
}


/*** random - generate a random integer in the interval [a,b] (b >= a >= 0) */

long Random(a, b)
long a, b;
{
  register long hi, lo;

#ifndef NDEBUG
  if (no_seed)
    {
      fprintf(stderr,"error:no random seed set\n");
      exit(17);
    }
#endif

  hi = MULTIPLIER * (saved_seed >> 16);
  lo = MULTIPLIER * (saved_seed & 0xffff);
  hi += (lo>>16);
  lo &= 0xffff;
  lo += (hi>>15);
  hi &= 0x7fff;
  lo -= MODULUS;
  if ((saved_seed = (hi<<16) + lo) < 0)
    saved_seed += MODULUS;

  if (b <= a)
    return b;
  return a + saved_seed % (b - a + 1);
}

double DblUnitRandom()
{
  return Random((long) 0,(long)MODULUS-1)/((double)MODULUS-1);
}

void RandomPermute(rgi,iMac)
     int iMac;
     int rgi[/*iMac*/];
{
  int j,t;
  
  while(--iMac)
    {
      j = Random((long)0,(long)iMac);
      t = rgi[j];
      rgi[j] = rgi[iMac];
      rgi[iMac] = t;
    }

}
