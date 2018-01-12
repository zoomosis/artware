/***************************************************************************
 *                                                                         *
 *  Squish Developers Kit Source, Version 2.00                             *
 *  Copyright 1989-1994 by SCI Communications.  All rights reserved.       *
 *                                                                         *
 *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
 *  SQUISH DEVELOPERS KIT LICENSING AGREEMENT IN SQDEV.PRN.  IF YOU DO NOT *
 *  FIND THE TEXT OF THIS AGREEMENT IN THE AFOREMENTIONED FILE, OR IF YOU  *
 *  DO NOT HAVE THIS FILE, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT    *
 *  ONE OF THE ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO  *
 *  USE THIS FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE SQUISH          *
 *  DEVELOPERS KIT LICENSING AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE *
 *  ABLE TO REACH WITH THE AUTHOR.                                         *
 *                                                                         *
 *  You can contact the author at one of the address listed below:         *
 *                                                                         *
 *  Scott Dudley       FidoNet     1:249/106                               *
 *  777 Downing St.    Internet    sjd@f106.n249.z1.fidonet.org            *
 *  Kingston, Ont.     CompuServe  >INTERNET:sjd@f106.n249.z1.fidonet.org  *
 *  Canada  K7M 5N3    BBS         1-613-634-3058, V.32bis                 *
 *                                                                         *
 ***************************************************************************/

/*# name=Portable file-searching hooks
    credit=Thanks go to Peter Fitzsimmons for these routines.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#ifndef __IBMC__
#include <dos.h>
#endif

#ifdef __TURBOC__
#include <dir.h>
#endif

#include "ffind.h"

static void near CopyDTA2FF(FFIND *ff)
{
  ff->usAttr=(word)ff->__dta.bAttr;

  ff->scCdate.dos_st.time=ff->__dta.usTime;
  ff->scCdate.dos_st.date=ff->__dta.usDate;

  /* Copy dates into other structure too */

  ff->scAdate=ff->scWdate=ff->scCdate;
  ff->ulSize=ff->__dta.ulSize;

  memset(ff->szName, '\0', sizeof(ff->szName));
  memmove(ff->szName, ff->__dta.achName, sizeof(ff->szName));
  strupr(ff->szName);
}

FFIND * _fast FindOpen(char *filespec, unsigned short attribute)
{
  FFIND *ff;

  ff=malloc(sizeof(FFIND));

  if (ff)
  {
    if (__dfindfirst(filespec, attribute, &ff->__dta)==0)
      CopyDTA2FF(ff);
    else
    {
      free(ff);
      ff=NULL;
    }
  }

  return ff;
}


int _fast FindNext(FFIND *ff)
{
  int rc=-1;

  if (ff)
  {
    if ((rc=__dfindnext(&ff->__dta))==0)
      CopyDTA2FF(ff);
  }

  return rc;
}

void _fast FindClose(FFIND *ff)
{
  if (ff)
    free(ff);
}

FFIND * _fast FindInfo(char *filespec)
{
  return FindOpen(filespec, 0);
}


