#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>

void *xmalloc(unsigned size)

{
	char *tmp;

	if (!(tmp = malloc (size)))
		{
            exit(1);
		}

	return(tmp);
}


void *xcalloc(unsigned n, unsigned size)

{
	char *tmp;

	if (!(tmp = calloc (n, size)))
		{
            exit(1);
		}

	return(tmp);
}

