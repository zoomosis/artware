#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <video.h>
#include <scrnutil.h>
#include "message.h"
#include <conio.h>



void *xmalloc(unsigned size)
{
	char *tmp;

   if(size == 0)
           {
           size = 2; /* Strange, lets alloc some, to not upset a free? */
           }

	if (!(tmp = malloc (size)))
		{
            cls(); clrscr();
            printf("\nError in malloc(%u)!\n", size);
            puts("\nOut of memory! Press a key twice to abort..\n");
            getch(); getch();
            exit(254);
		}

	return(tmp);
}

/* --------------------- */


void *xcalloc(unsigned n, unsigned size)
{
	char *tmp;


   if(size == 0)
           { 
           size = 2; /* Strange, lets alloc some, to not upset a free? */
           }

	if (!(tmp = calloc (n, size)))
		{
            cls(); clrscr();
            printf("Error in xcalloc(%u, %u)\n", n, size);
            puts("\nOut of memory! Press a key twice to abort..\n");
            getch(); getch();
            exit(254);
		}



	return(tmp);
}

/* --------------- */

char *xstrdup(char *s)

{
	char *tmp;

	if (!(tmp = strdup(s)))
		{
            cls(); clrscr();
            puts("\nOut of memory in xstrdup()! Press a key twice to abort..");
            getch(); getch();
            exit(254);
		}

	return(tmp);
}

