#include <stdio.h>
#include "prog.h"

#define NUM sizeof(array)/sizeof(array[0])
#define SWAP(a,b,s) s=a; a=b; b=s;

static void _fast iqksort(int *p_lo,int *p_hi);

void _fast qksort(int a[],size_t n)
{
  if (n > 1)
    iqksort(a,&a[n-1]);
}



static void _fast iqksort(int *p_lo,int *p_hi)
{
  int  *p_mid=p_lo+(((int)(p_hi-p_lo))/2),
       *p_i,
       *p_lastlo,
       tmp;

  SWAP(*p_lo,*p_mid,tmp);

  p_lastlo=p_lo;

  for (p_i=p_lo+1;p_i <= p_hi;++p_i)
  {
    if (*p_lo > *p_i)
    {
      ++p_lastlo;
      SWAP(*p_lastlo,*p_i,tmp);
    }
  }

  SWAP(*p_lo,*p_lastlo,tmp);

  if (p_lo < p_lastlo && p_lo < p_lastlo-1)
    iqksort(p_lo,p_lastlo-1);

  if (p_lastlo+1 < p_hi)
    iqksort(p_lastlo+1,p_hi);
}

