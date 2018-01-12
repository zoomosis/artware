#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

/* #include "wstruct.h" */

void *xmalloc(unsigned size)

{
	char *tmp;

	if (!(tmp = malloc (size)))
		{
			getout("Not enough memory!!");
		}

	return(tmp);
}


void *xcalloc(unsigned n, unsigned size)

{
	char *tmp;

	if (!(tmp = calloc (n, size)))
		{
			getout("Not enough memory!");
		}

	return(tmp);
}

