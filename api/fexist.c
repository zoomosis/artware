#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifndef __GNUC__
#include <io.h>
#include <dos.h>
#else
#include <unistd.h>
#endif
#include "ffind.h"
#include "prog.h"

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #include <os2.h>
#endif

/*
main()
{
  printf("asdfe=%d\n",direxist("c:\\asdfe"));
  printf("blank=%d\n",direxist("c:\\blank"));
  printf("tc=%d\n",direxist("c:\\tc"));
  printf("c:\\=%d\n",direxist("c:\\"));
  printf("d:\\=%d\n",direxist("d:\\"));
  printf("e:\\=%d\n",direxist("e:\\"));
  printf("f:\\=%d\n",direxist("f:\\"));
}
*/


int _fast fexist(char *filename)
{
  #ifndef __OS2__

   if(access(filename, 0) == -1)
      return FALSE;

   return TRUE;

  #else

  byte temp[PATHLEN];
  HDIR          FindHandle;
  FILEFINDBUF3  FindBuffer;
  ULONG         FindCount;
  APIRET        rc;          /* Return code */

  FindHandle = 0x0001;
  FindCount = 1;

  strcpy(temp, filename);

  rc = DosFindFirst(temp,                  /* File pattern */
                    &FindHandle,           /* Directory search handle */
                    0,                     /* Search attribute */
                    (PVOID) &FindBuffer,   /* Result buffer */
                    sizeof(FindBuffer),    /* Result buffer length */
                    &FindCount,            /* Number of entries to find */
                    FIL_STANDARD);         /* Return level 1 file info */


  DosFindClose(FindHandle);

  return (rc==0) ? TRUE : FALSE;

  #endif

}


int _fast direxist(char *directory)
{

  #ifndef __OS2__
    char temp[128];
    strcpy(temp, directory);
    #ifndef __GNUC__
    Strip_Trailing(temp, '\\');
    #else
    Strip_Trailing(temp, '/');
    #endif

    if(access(temp, 0) == -1)
       return FALSE;

    return TRUE;

  #else

  byte temp[PATHLEN];
  HDIR          FindHandle;
  FILEFINDBUF3  FindBuffer;
  ULONG         FindCount;
  APIRET        rc;          /* Return code */

  FindHandle = 0x0001;
  FindCount = 1;

  strcpy(temp, directory);

  Add_Trailing(temp, '\\');

  if( (isalpha(temp[0]) && temp[1]==':' &&
      (temp[2]=='\\' || temp[2]=='/') && !temp[3]) ||
      eqstr(temp, "\\"))
    return TRUE;


  Strip_Trailing(temp, '\\');

  rc = DosFindFirst(temp,                  /* File pattern */
                    &FindHandle,           /* Directory search handle */
                    16,                    /* Search attribute */
                    (PVOID) &FindBuffer,   /* Result buffer */
                    sizeof(FindBuffer),    /* Result buffer length */
                    &FindCount,            /* Number of entries to find */
                    FIL_STANDARD);         /* Return level 1 file info */

  if(FindBuffer.attrFile != 16) rc=1;  // Nasty, but works :-)

  DosFindClose(FindHandle);

  return (rc==0) ? TRUE : FALSE;

  #endif


}

