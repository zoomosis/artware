/*
**  SHEL2DOS.C - Shell to DOS from a running program
**
**  Original Copyright 1989-1991 by Bob Stout as part of
**  the MicroFirm Function Library (MFL)
**
**  This subset version is hereby donated to the public domain.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <conio.h>
#include <dir.h>

#include <video.h>
#include <scrnutil.h>
#ifndef __OS2__
#include <exec.h>
#endif

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "global.h"

int shell_to_DOS(void)
{
      static char *comspec, prompt[256], oldprompt[256];
      char *ptr, *curdir;
      int retval=0, drive;
      static char didswap=0;

      curdir = getcwd(NULL, MAXPATH);
      drive = getdisk();

      savescreen();
      cls();

	   _setcursortype(_NORMALCURSOR);
      window(1,1,80,25);
      textattr(7);
      clrscr();

      print(1,0,14,"timEd");
      print(1,5,7,"  (c) 1992-'94  Gerard van Essen (2:281/527)");

      print(3,0,7,"OS shell - type 'exit' to return to timEd..");

      MoveXY(1,5);

      comspec = getenv("COMSPEC");

      if(comspec == NULL)
      #ifndef __OS2__
           comspec = "COMMAND.COM";     /* Better than nothing... */
      #else
           comspec = "CMD.EXE";
      #endif


      if(!didswap)
         {
         memset(oldprompt, '\0', sizeof(oldprompt));
         memset(prompt, '\0', sizeof(prompt));

         if((ptr=getenv("PROMPT")) != NULL)
            {
            sprintf(oldprompt, "%0.200s", ptr);
            }

         sprintf(prompt, "PROMPT=[Type EXIT to return to timEd]\r\n%s",
                                                                   oldprompt);
         putenv(prompt);
         didswap = 1;
         }

      kbflush();

      #ifndef __OS2__
      retval = do_exec("", "", USE_ALL|HIDE_FILE|CHECK_NET, cfg.usr.swap_shell ? 0xFFFF : 0x0000, environ);
      #else
      retval = spawnlpe(P_WAIT, comspec, comspec, NULL, environ);
      #endif

//      putenv("PROMPT=");
//      memset(prompt, '\0', sizeof(prompt));
//      sprintf(prompt, "PROMPT=%s", oldprompt);
//      putenv(prompt);

      _setcursortype(_NOCURSOR);

     putscreen();

     setdisk(drive);
     chdir(curdir);
     if(curdir) free(curdir);

     return retval;
}
#ifdef TEST

#include <stdio.h>

void main(void)
{
      int retval = shell_to_DOS();

      printf("shell_to_DOS() returned %d\n", retval);
}

#endif
