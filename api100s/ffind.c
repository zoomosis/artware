/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Portable file-searching hooks                                          *
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
 *  Thanks go to Peter Fitzsimmons for these routines.                     *
 *                                                                         *
 ***************************************************************************/

/* $Id: ffind.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include "compiler.h"

#ifndef __OS2__
#include <dir.h>
#endif

#include "ffind.h"

#ifdef __OS2__
#define INCL_NOPM
#define INCL_DOS
#include <os2.h>
#endif

/* FindOpen;  Use like MSDOS "find first" function,  except be sure to
 * release allocated system resources by caling FindClose() with the
 * handle returned by this function.
 *
 * returns:  NULL  -- file not found.
 *
 */

FFIND * _fast FindOpen(char *filespec, unsigned short attribute)
{
  FFIND *ff;

  ff=malloc(sizeof(FFIND));

  if (ff)
  {

#ifndef __OS2__

    if (findfirst(filespec,(struct ffblk *)ff,attribute))
    {
      free(ff);
      ff=NULL;
    }

#elif defined(__OS2__)

    USHORT usSearchCount = 1;
    FILEFINDBUF findbuf;

    ff->hdir=HDIR_CREATE;

    if (!DosFindFirst(filespec,&ff->hdir,attribute,&findbuf,
                      sizeof(FILEFINDBUF),&usSearchCount,0L))
    {
      ff->ff_attrib=(char)findbuf.attrFile;
      ff->ff_fsize=findbuf.cbFile;

      ff->ff_ftime=*((USHORT *)&findbuf.ftimeLastWrite);
      ff->ff_fdate=*((USHORT *)&findbuf.fdateLastWrite);

      strncpy(ff->ff_name,findbuf.achName,sizeof(ff->ff_name));
    }
    else
    {
      free(ff);
      ff = NULL;
    }

#elif defined(__MSC__) || defined(__WATCOMC__)

    if (_dos_findfirst(filespec,attribute,(struct find_t *)ff))
    {
      free(ff);
      ff=NULL;
    }
#else
#error "I don't know which compiler we're using!"
#endif
  }

  return ff;
}

/* FindNext:   returns 0 if next file was found, non-zero if it was not.
 *
 */

int _fast FindNext(FFIND *ff)
{
  int rc=-1;

  if (ff)
  {

#ifndef __OS2__

    rc=findnext((struct ffblk *)ff);

#elif defined(__OS2__)

    USHORT usSearchCount = 1;
    FILEFINDBUF findbuf;

    if(ff->hdir && !DosFindNext(ff->hdir,&findbuf,sizeof(findbuf),&usSearchCount))
    {
      ff->ff_attrib=(char)findbuf.attrFile;
      ff->ff_ftime=*((USHORT *)&findbuf.ftimeLastWrite);
      ff->ff_fdate=*((USHORT *)&findbuf.fdateLastWrite);
      ff->ff_fsize=findbuf.cbFile;
      strncpy(ff->ff_name, findbuf.achName, sizeof(ff->ff_name));
      rc=0;
    }
#elif defined(__MSC__) || defined(__WATCOMC__)
    rc=_dos_findnext((struct find_t *)ff);
#else
#error "I don't know which compiler we're using!"
#endif
  }

  return rc;
}

/* FindClose: End a directory search.  Failure to call this function will
 * result in unclosed file handles (os/2),  and un-free()'d memory (dos/os2).
 *
 */
void _fast FindClose(FFIND *ff)
{
  if (ff)
  {
#ifdef __OS2__
    if(ff->hdir)
        DosFindClose(ff->hdir);
#endif
    free(ff);
  }
}


