#ifndef __GNUC__
#include <mem.h>
#include <io.h>
#include <dos.h>
#include <share.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include "lincomp.h"

int canlock(char *path)
{
   int testfile, i;
   char temp[120];


   strcpy(temp, path);
   if(path[strlen(temp)-1] != '\\')
      strcat(temp, "\\");

   strcat(temp, "locktest.~~~");

   #ifdef __KASPER__
   printf("Opening %s\n", temp);
   #endif

   if( (testfile = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYWR, S_IREAD | S_IWRITE)) == -1)
      return 0;

   #ifdef __KASPER__
   printf("rc = %d\n", testfile);
   #endif

   #ifdef __KASPER__
   printf("attempting lock\n");
   #endif

   i = lock(testfile, 0L, 1L);

   #ifdef __KASPER__
   printf("rc = %d\n", i);
   #endif

   #ifdef __KASPER__
   printf("Close file\n");
   #endif

   close(testfile);

   #ifdef __KASPER__
   printf("Unlink file\n");
   #endif

   unlink(temp);

   #ifdef __KASPER__
   printf("Retval canlock(): %d\n", (i == -1) ? 0 : 1);
   #endif

   return (i == -1) ? 0 : 1;   /* Return 1 if locking worked, 0 otherwise */

}
