#include "includes.h"


void Message(char *text, int pause, int stop, YesNo save);
int  confirm(char *text);



void Message(char *text, int pause, int stop, YesNo save)
{
   BOX *popup;
   int x1,x2;

   x1 = (maxx - (strlen(text) + 8))/2;
   x2 = x1 + strlen(text) + 4;

   popup = initbox(11,x1,15,x2,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,save,' ');
   drawbox(popup);
   boxwrite(popup,1,1,text);
   if(pause == -1)
      titlewin(popup, TCENTER, " Press a key.. ", cfg.col[Cpoptitle]);
   MoveXY(x1+3,14);

   if (pause == -1)
      get_idle_key(0, GLOBALSCOPE);
   else
     #ifndef __OS2__
//        delay(pause*100);
        sleep(pause/6);
     #else
        sleep(pause/6);
     #endif

   delbox(popup);

   if (stop != 0)
      {
      cls();
      #ifdef __WATCOMC__
      _settextcursor(0x0607);
      #else
	   _setcursortype(_NORMALCURSOR);
      #endif
//      window(1,1,80,25);
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

   popup = initbox(12,x1,16,x2,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   drawbox(popup);
   titlewin(popup, TLEFT, " Confirm.. ", cfg.col[Cpoptitle]);
   boxwrite(popup,1,1,text);
   MoveXY(x1+3,15);

   c = get_idle_key(0, GLOBALSCOPE);

   delbox(popup);

   ret = ((c == 'Y') || (c == 'y')) ? 1 : 0;

   return ret;
}

