#include <stdio.h>
#include <conio.h>


void main(void)
{
   int ch;

   printf("By all means, start typing:\n\n");

   do {

     ch = getch();
     if(ch == 0)
        ch = 256 + getch();
     printf("ResultCode: %d\n", ch);

   } while(ch != 27);

}

