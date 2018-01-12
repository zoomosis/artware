#include "video.h"
#include "scrnutil.h"

void Message(char *text, int pause, int stop, int save);


void Message(char *text, int pause, int stop, int save)
{
   BOX *popup;
   int x1,x2;

   x1 = (maxx - (strlen(text) + 8))/2;
   x2 = x1 + strlen(text) + 4;

   popup = initbox(11,x1,15,x2,113,113,SINGLE,save,' ');
   drawbox(popup);
   boxwrite(popup,1,1,text);
   if(pause == -1)
      titlewin(popup, TCENTER, " Press a key.. ");
   MoveXY(x1+3,14);

   getch();

   delbox(popup);

   if (stop != 0)
      {
      cls();
      exit(stop);
      }
}

