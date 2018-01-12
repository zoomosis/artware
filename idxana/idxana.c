#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <share.h>

#include <msgapi.h>


struct my_idx
{

  long offset;
  UMSGID umsgid;
  dword hash;

};



void main(int argc, char *argv[])
{
   int idxhandle;
   struct my_idx thisidx;
   int number=0;


   if(argc != 2)
       {
       printf("\nWrong number of arguments!\n");
       printf("Usage  : idxana <indexname>\n");
       printf("Example: idxana c:\msgs\sysop.idx\n\n");
       exit(254);
       }

   if( (idxhandle = open(argv[1], O_RDONLY | O_BINARY | SH_DENYNO)) == -1)
       {
       printf("\nError opening %s!\n", argv[1]);
       exit(254);
       }

   while(read(idxhandle, &thisidx, sizeof(struct my_idx)) == sizeof(struct my_idx))
      {
      printf("\nNumber : %d  \n", ++number);
      printf("Offset : %lu \n", thisidx.offset);
      printf("UMSGID : %lu \n", thisidx.umsgid);
      printf("Hash   : %lu \n", thisidx.hash);
      }

   close(idxhandle);

}







