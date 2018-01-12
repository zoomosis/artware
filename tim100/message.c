#include <string.h>
#include <conio.h>
#include <dos.h>
#include <video.h>
#include <scrnutil.h>
#include <stdlib.h>
#include <ctype.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "idlekey.h"

void Message(char *text, int pause, int stop, YesNo save);
int  confirm(char *text);

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
   MoveXY(x1+3,14);

   if (pause == -1)
      get_idle_key(1);
   else
     #ifndef __OS2__
        delay(pause*100);
     #else
        sleep(pause/6);
     #endif

   delbox(popup);

   if (stop != 0)
      {
      cls();
	   _setcursortype(_NORMALCURSOR);
      window(1,1,80,25);
      textattr(7);
      clrscr();

      exit(stop);
      }
}


int confirm(char *text)
{
   BOX *popup;
   int x1,x2;
   int c;
   char ret=0;

   x1 = (maxx - (strlen(text) + 8))/2;
   x2 = x1 + strlen(text) + 4;

   popup = initbox(12,x1,16,x2,cfg.col.popframe,cfg.col.poptext,SINGLE,YES,' ');
   drawbox(popup);
   boxwrite(popup,1,1,text);
   MoveXY(x1+3,15);

   c = get_idle_key(1);

   delbox(popup);

   ret = ((c == 'Y') || (c == 'y')) ? 1 : 0;

   return ret;
}

