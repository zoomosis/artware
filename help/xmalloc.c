#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <video.h>
#include <scrnutil.h>
#include "message.h"

void *xmalloc(unsigned size)

{
	char *tmp;

	if (!(tmp = malloc (size)))
		{
			printf("\nNot enough memory!");
		}

	return(tmp);
}


void *xcalloc(unsigned n, unsigned size)

{
	char *tmp;

	if (!(tmp = calloc (n, size)))
		{
			puts("\n\nNot enough memory!!\n");
			exit(1);
		}

	return(tmp);
}

