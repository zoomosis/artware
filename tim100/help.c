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

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "xmalloc.h"
#ifndef __OS2__
#include "dvaware.h"
#endif
#include "message.h"
#include "global.h"
#include "idlekey.h"


typedef struct
{

   long offset;
   long bytes;

}  HELPIDX;


HELPIDX help[20];

void show_help(int cat);


void show_help(int cat)

{
   int helpfile;
   char *helptext, filename[100];
   LINE *firstline, *curline, *last;
   int nolines=0, maxlen=0, line;
   BOX *helpbox;

   sprintf(filename, "%s\\timed.hlp", cfg.homedir);

   if( (helpfile=open(filename, O_RDONLY | O_BINARY)) == -1)
     {
     sprintf(msg, "Error opening helpfile (%s)!", filename);
     Message(msg, -1, 0, YES);
     return;
     }

   if( read(helpfile, &help, sizeof(help)) != sizeof(help))
      {
      Message("Error reading helpfile!", -1, 0, YES);
      close(helpfile);
      return;
      }

   helptext = xcalloc(1, help[cat].bytes);

   lseek(helpfile, help[cat].offset, SEEK_SET);
   if (read(helpfile, helptext, help[cat].bytes) != help[cat].bytes)
      {
      Message("Error reading helpfile!", -1, 0, YES);
      close(helpfile);
      return;
      }

   close(helpfile);

   firstline = FormatText(helptext, maxx-4);

   for(curline=firstline; curline; curline=curline->next)
      {
      nolines++;
      maxlen = max(maxlen, strlen(curline->ls));
      last = curline;
      }

   if( (last->ls[0] == '\0') &&
       (last != firstline) )        /* Last line is empty */
      {
      last->prev->next = NULL;
      nolines--;
      free(last);
      }

   helpbox = initbox( (maxy-nolines-3)/2,
                      (maxx-maxlen-4)/2,
                      (maxy-nolines-3)/2+nolines+3,
                      (maxx-maxlen-4)/2+maxlen+3,
                      cfg.col.popframe, cfg.col.poptext, SINGLE, YES, ' ');
   drawbox(helpbox);

   for(curline=firstline, line=0; curline; curline=curline->next, line++)
      boxwrite(helpbox, line+1, 1, curline->ls);

   get_idle_key(1);

   free(helptext);

   curline=firstline;
   while(curline)
      {
      firstline = curline;
      curline = curline->next;
      free(firstline);
      }

   delbox(helpbox);

}

