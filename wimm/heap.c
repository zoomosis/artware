#include <conio.h>
#include <stdio.h>
#include <alloc.h>
#include <video.h>
#include <scrnutil.h>

void check_my_heap(void)

{
   struct heapinfo hi;
   int i=0;


   printf("\n\nHere we go........: \n");
   printf("Heapcheck: %d\n", heapcheck());
   hi.ptr = NULL;
   printf("Free mem: %u\n", coreleft()); getch();
   printf( "   Size   Status\n" );
   printf( "   ----   ------\n" );
   while( heapwalk( &hi ) == _HEAPOK ) {

          printf( "%7u    ", hi.size);
          if (hi.in_use) printf("Used\n");
          else printf("Free\n");

          if (i++ > 22) {

             printf(" --- more --- ");
             getch();
             clrscr();
             i=1;
          }
   }

   printf(" --- end ---");
   getch();

}
