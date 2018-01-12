#include "prog.h"

#ifdef __GNUC__
#include <unistd.h>

void pascal far flush_handle2( int fh )
{
    fsync(fh);
}

void _fast flush_handle (FILE *fp )
{
    fflush(fp);
}

#else
#include <io.h>
#include <dos.h>




#ifdef __OS2__
#define INCL_NOPM
#include <os2.h>


void pascal far flush_handle2( int fh )
{
    DosBufReset((HFILE)fh);
}

#elif defined(__FLAT__)

void pascal far flush_handle2( int fh )
{
   int f2;

   if( (f2=dup(fh)) != -1)
     close(f2);
}

#endif


/* This makes sure a file gets flushed to disk.  Thanks to Ray Duncan      *
 * for this tip in his _Advanced MS-DOS_ book.                             */

void _fast flush_handle(FILE *fp)
{

  fflush(fp);

#ifdef __OS2__
  DosBufReset(fileno(fp));
#endif

#ifndef __OS2__
  flush_handle2(fileno(fp));
#endif
}


#endif