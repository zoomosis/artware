#ifndef __CBTREE_H__
#define __CBTREE_H__

#ifdef CBTREE

/* include the Mix C Database Toolchest */

#include <cbtree.h>

#else

/* fake it */

#define ERROR -1
#define FOUND -2
#define NOTFOUND -3
#define BOI -4
#define EOI  -5
#define OK    0

#define LESS     -1
#define GREATER   1
#define EQUAL     0

typedef struct
{
  int dummy;
}
Cbtree;

typedef long Item;

/* dummy prototypes */

#define cbinit(bufcnt, bufsize) ERROR
#define cbopen(x, compare) NULL
#define cbexit()
#define cbclose(cbt)
#define cbfind(cbt, searchitem, len, item) ERROR
#define cbcurr(cbt, item) ERROR
#define cbnext(cbt, item) ERROR
#define cbprev(cbt, item) ERROR
#define cbhead(cbt, item) ERROR
#define cbtail(cbt, item) ERROR
#define cbmark(cbt) ERROR
#define cbfindmark(cbt, item) ERROR

#endif

#endif  /* __CBTREE_H__ */
