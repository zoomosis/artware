#include <stdlib.h>
#include "video.h"

void main(void)
{
   int i, l;

   video_init();

   cls();

   biprint(1,0,7,14,"Press ~a key~ to continue..", ' ');
   getch(); cls();

   for(l=0; l<20; l++)
     {
     biprinteol(l+2,l,112,126,"Dit is een ~mooie~ regel! ", ' ');
     }

   biprinteol(18,0,113,126,"Wat heeft die kerel een ~grote~ zeg!", ' ');

   getch();
   cls();
   biprint(10,10,11,30,"��~ test ~���������������������͸", ' ');
   print(11,10,11,"�                             �");
   print(12,10,11,"�                             �");
   print(13,10,11,"�                             �");
   print(14,10,11,"�                             �");
   print(15,10,11,"�                             �");
   print(16,10,11,"�                             �");
   print(17,10,11,"�����������������������������;");

   getch();


}

