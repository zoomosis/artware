#include <fcntl.h>
#include <sys\stat.h>
#include <share.h>
#include <string.h>
#include <io.h>

int canlock(char *path)
{
   int testfile, i;
   char temp[120];


   strcpy(temp, path);
   if(path[strlen(temp)-1] != '\\')
      strcat(temp, "\\");

   strcat(temp, "locktest.~~~");
   if( (testfile = open(temp, O_BINARY | O_RDWR | SH_DENYWR | O_CREAT, S_IREAD | S_IWRITE)) == -1)
      return 0;

   i = lock(testfile, 0L, 1L);

   close(testfile);
   unlink(temp);

   return (i == -1) ? 0 : 1;   /* Return 1 if locking worked, 0 otherwise */

}
