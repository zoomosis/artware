#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <video.h>

/* #include "wstruct.h" */

void *xmalloc(unsigned size)

{
	char *tmp;

	if (!(tmp = malloc (size)))
        {
        cls(); MoveXY(3,1);
        print(0,0,7,"Out of memory..");
		}

	return(tmp);
}


void *xcalloc(unsigned n, unsigned size)

{
	char *tmp;

	if (!(tmp = calloc (n, size)))
		{
        cls(); MoveXY(3,1);
        print(0,0,7,"Out of memory..");
		}

	return(tmp);
}

