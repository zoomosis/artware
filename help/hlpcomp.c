#include <string.h>
#include <alloc.h>
#include <stdio.h>
#include <conio.h>
#include <video.h>
#include <scrnutil.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>


#include "xmalloc.h"
#include "dvaware.h"
#include "message.h"
#include "help.h"

HELPIDX help[20];

void show_help(int cat);
void compile_help(void);


main(void)
{

   clrscr();
   compile_help();

}


void compile_help(void)

{
   FILE *in;
   int out;
   char temp[256];
   char *curptr, *begin=NULL;
   int helpcat = 0, skip=0;

   if( (in = fopen("help.txt", "r")) == NULL)
      {
      printf("\n\nCan't open help.txt!\n");
      exit(1);
      }

   if( (out = open("timed.hlp", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) == -1)
      {
      printf("\n\nCan't open timed.hlp!");
      exit(1);
      }

   if(write(out, &help, sizeof(help)) != sizeof(help))
      {
      printf("\nError writing to timed.help!\n");
      exit(1);
      }

   while(fgets(temp, 256, in))
     {

     if(temp[strlen(temp)-1] == '\n')      /* Strip \n from text */
        temp[strlen(temp)-1] = '\0';

     if(strncmpi(temp, "@@@", 3) == 0)    /* @@@ signals start of new helpscreen */
        {
        if(skip)   /* first @@@ encountered is begin of file, nothing to write then */
           {
           help[helpcat].offset = tell(out);       /* Where are we in the help file? */
           help[helpcat].bytes  = strlen(begin)+1;   /* How long was this helptext?    */
           helpcat++;

           if(write(out, begin, strlen(begin)+1) != strlen(begin)+1)
              {
              printf("\nError writing timed.hlp!\n");
              exit(1);
              }
           }

        if (begin) free(begin);
        begin = NULL;
        skip=1;
        continue;
        }
     if(!begin)
       {
       begin = xmalloc(strlen(temp)+2); /* + 2 for trailing 0 and \r */
       strcpy(begin, temp);
       strcat(begin, "\r");
       }
     else
       {
       begin = realloc(begin, strlen(begin) + strlen(temp) + 2); /* + 2 for trailing 0 and \r */
       strcat(begin, temp);
       strcat(begin, "\r");
       }
     }

   help[helpcat].offset = tell(out);       /* Where are we in the help file? */
   help[helpcat].bytes  = strlen(begin)+1;   /* How long was this helptext?    */
   helpcat++;

   if(write(out, begin, strlen(begin)+1) != strlen(begin)+1)
      {
      printf("\nError writing timed.hlp!\n");
      exit(1);
      }

   if (begin) free(begin);

   lseek(out, 0L, SEEK_SET);
   if(write(out, &help, sizeof(help)) != sizeof(help))
      {
      printf("\nError writing to helpfile!\n");
      exit(1);
      }

   close(out);
   fclose(in);

   printf("Done! %d help categories found.\n\n", helpcat);

}
