#include <string.h>
#include <conio.h>
#include <dos.h>
#include <video.h>
#include <scrnutil.h>
#include <stdlib.h>
#include <ctype.h>

#include "tstruct.h"
#include "global.h"
#include "idlekey.h"

void Message(char *text, int pause, int stop, YesNo save);
char confirm(char *text);

char msg[81];


void Message(char *text, int pause, int stop, YesNo save)
{
   BOX *popup;
   int x1,x2;

   x1 = (maxx - (strlen(text) + 8))/2;
   x2 = x1 + strlen(text) + 4;

   popup = initbox(11,x1,15,x2,cfg.col.popframe,cfg.col.poptext,SINGLE,save,' ');
   drawbox(popup);
   boxwrite(popup,1,1,text);

   if (pause == -1)
      get_idle_key();
   else delay(pause*100);

   delbox(popup);

   if (stop != 0)
      exit(stop);
}


char confirm(char *text)
{
   BOX *popup;
   int x1,x2;
   char c;

   x1 = (maxx - (strlen(text) + 8))/2;
   x2 = x1 + strlen(text) + 4;

   popup = initbox(11,x1,17,x2,cfg.col.popframe,cfg.col.poptext,SINGLE,YES,' ');
   drawbox(popup);
   boxwrite(popup,1,1,text);
   boxwrite(popup,3,(x2-x1)/2-10,"Are you sure? (y/N)");

   c = toupper(get_idle_key());

   delbox(popup);

   return (c == 'Y') ? 1 : 0;
}

