#include "includes.h"

/*



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
            dumpmem();
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
            dumpmem();
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
            dumpmem();
            getch(); getch();
            exit(254);
		}

	return(tmp);
}

*/
void dumpmem(void)
{
      FILE *out;
      struct heapinfo hi;
      int i;


      if( (out=fopen("\\chain.txt", "w+")) == NULL )
           return;

      fprintf(out, "Memory available: %lu\n\n", (unsigned long) coreleft());

      hi.ptr = NULL;
      fprintf(out, "Pointer       Size   Status\n" );
      fprintf(out, "-------       ----   ------\n\n" );

      while( heapwalk( &hi ) == _HEAPOK )
         {
         fprintf(out, "%p  %7u    ", hi.ptr, hi.size);
         if(hi.in_use)
            fprintf(out, "Used.\n");
         else
            fprintf(out, "Free. <--\n");
         }

      fclose(out);
}