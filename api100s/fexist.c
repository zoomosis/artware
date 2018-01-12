/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  File-exist and directory-searching routines                            *
 *                                                                         *
 *  For complete details of the licensing restrictions, please refer to    *
 *  the licence agreement, which is published in its entirety in           *
 *  README.1ST.                                                            *
 *                                                                         *
 *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
 *  MSGAPI LICENSING AGREEMENT.  IF YOU DO NOT FIND THE TEXT OF THIS       *
 *  AGREEMENT IN ANY OF THE AFOREMENTIONED FILES, OR IF YOU DO NOT HAVE    *
 *  THESE FILES, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT ONE OF THE   *
 *  ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO USE THIS    *
 *  FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE MSGAPI LICENSING         *
 *  AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE ABLE TO REACH WITH THE   *
 *  AUTHOR.                                                                *
 *                                                                         *
 *  You can contact the author at one of the address listed below:         *
 *                                                                         *
 *  Scott Dudley           FidoNet  1:249/106                              *
 *  777 Downing St.        Internet f106.n249.z1.fidonet.org               *
 *  Kingston, Ont.         BBS      (613) 389-8315   HST/14.4k, 24hrs      *
 *  Canada - K7M 5N3                                                       *
 *                                                                         *
 ***************************************************************************/

/* $Id: fexist.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
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
    Strip_Trailing(temp, '\\');

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

