#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys\stat.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "readarea.h"
#include "header.h"

#include <video.h>
#include <scrnutil.h>
#ifndef __OS2__
#include <exec.h>
#endif
#include "message.h"
#include "pickone.h"
#include "xmalloc.h"
#include "idlekey.h"

int maint_menu(AREA *area, MSG **areahandle, long lastorhigh);
void write_echolog(void);


int maint_menu(AREA *area, MSG **areahandle, long lastorhigh)
{
   char **picklist, tmpmsg[100];
   char command[100], parms[100];
   int action, retval;
   #ifdef __OS2__
   char commandline[130];
   #endif

   memset(command, '\0', sizeof(command));
   memset(parms, '\0', sizeof(parms));

   if(area->base & MSGTYPE_SDM)
      {
      picklist = xcalloc(2+1, sizeof(char *));
      picklist[0] = (char *) xstrdup(" Renumber message area  ");
      picklist[1] = (char *) xstrdup(" Execute mail processor ");

      action = pickone(picklist, 5, 21, 23, 56);

      free_picklist(picklist);

      if(action == -1) return 0;

      }
   else if(area->base & MSGTYPE_JAM)
      {
      picklist = xcalloc(2+1, sizeof(char *));
      picklist[0] = (char *) xstrdup(" Execute mail processor ");
      picklist[1] = (char *) xstrdup(" Inspect message area   ");

      action = pickone(picklist, 5, 21, 23, 56);

      free_picklist(picklist);

      if(action == -1) return 0;
      }

   else if(area->base & MSGTYPE_HMB)
      {
      picklist = xcalloc(1+1, sizeof(char *));
      picklist[0] = (char *) xstrdup(" Execute mail processor ");

      action = pickone(picklist, 5, 21, 23, 56);

      free_picklist(picklist);

      if(action == -1) return 0;
      }

   else /* Squish, let user choose between Pack, Reindex etc. */
      {
      picklist = xcalloc(5+1, sizeof(char *));
      picklist[0] = (char *) xstrdup(" Pack message area      ");
      picklist[1] = (char *) xstrdup(" Re-index message area  ");
      picklist[2] = (char *) xstrdup(" Fix message area       ");
      picklist[3] = (char *) xstrdup(" Inspect message area   ");
      picklist[4] = (char *) xstrdup(" Execute mail processor ");

      action = pickone(picklist, 5, 21, 23, 56);

      free_picklist(picklist);

      if(action == -1) return 0;
      }

   /* User said 'yes' (*.MSG) or picked one (not <ESC>) so close area.. */

   UpdateLastread(area, lastorhigh, *areahandle);

	if(MsgCloseArea(*areahandle))
      {
		Message("Error closing area!", 2, 0, NO);
      return 0;
      }

   /* Now build the commandline.. */

   strcpy(parms, area->dir);

   if(area->base & MSGTYPE_SDM)
     {
     switch(action)
        {
        case 0:
          #ifndef __OS2__
          strcpy(command, "renum.bat");
          #else
          strcpy(command, "renum.cmd");
          #endif
          break;
        case 1:
          write_echolog();
          #ifndef __OS2__
          strcpy(command, "mail.bat");
          #else
          strcpy(command, "mail.cmd");
          #endif
          break;
        }
     }
   else if(area->base & MSGTYPE_JAM)
     {
     switch(action)
        {
        case 0:
          write_echolog();
          #ifndef __OS2__
          strcpy(command, "mail.bat");
          #else
          strcpy(command, "mail.cmd");
          #endif
          break;
        case 1:
          #ifndef __OS2__
          strcpy(command, "JamInfo");
          #else
          strcpy(command, "JamInfop");
          #endif
          break;
        }
     }
   else if(area->base & MSGTYPE_HMB)
     {
     write_echolog();
     #ifndef __OS2__
     strcpy(command, "mail.bat");
     #else
     strcpy(command, "mail.cmd");
     #endif
     }
   else
     {
     switch(action)
        {
        case 0:
           #ifndef __OS2__
           strcpy(command, "SQpack");
           #else
           strcpy(command, "SQpackp");
           #endif
           break;
        case 1:
           #ifndef __OS2__
           strcpy(command, "SQreidx");
           #else
           strcpy(command, "SQreidxp");
           #endif
           break;
        case 2:
           #ifndef __OS2__
           strcpy(command, "SQfix");
           #else
           strcpy(command, "SQfixp");
           #endif
           break;
        case 3:
           #ifndef __OS2__
           strcpy(command, "SQinfo");
           #else
           strcpy(command, "SQinfop");
           #endif
           sprintf(parms, "%s -q", area->dir);
           break;
        case 4:
          write_echolog();
          #ifndef __OS2__
          strcpy(command, "mail.bat");
          #else
          strcpy(command, "mail.cmd");
          #endif
          break;

        }
     }

   savescreen();

	_setcursortype(_NORMALCURSOR);
   textattr(7);
   clrscr();
   cls();

   sprintf(tmpmsg, "Executing: \"%s %s\"", command, parms);
   print(0,0,7,tmpmsg);

   MoveXY(1,3);

/*   __spawn_resident = (cfg.usr.swap_shell == 0) ? 0xFFFF : 0; */

   #ifndef __OS2__
   retval = do_exec(command, parms, USE_ALL|HIDE_FILE|CHECK_NET,cfg.usr.swap_shell ? 0xFFFF : 0x0000, environ);
   #else
   sprintf(commandline, "%s %s", command, parms);
   system(commandline);
   #endif

   printf("\n");
   _setcursortype(_NOCURSOR);
   statusbar("Press any key to return to timEd");
   get_idle_key(1);

   putscreen();

   /* Here we re-open the stuff, update stats etc. */

   if(!(*areahandle=MsgOpenArea(area->dir, MSGAREA_CRIFNEC, area->base)))
      {
      Message("Error re-opening area!", -1, 0, YES);
		return -1;
      }

	/* Read area statistics */

   ScanArea(area, *areahandle);     /* Do a 'real' MSGAPI scan */

   return (area->nomsgs > 0) ? 0 : -1;  /* Ret -1, exit area if no msgs */

}



void write_echolog(void)
{
	AREA *curarea;
   FILE *echolog;
   int logisopen=0;

	curarea = cfg.first;

   if(cfg.usr.echolog[0] == '\0')
       return;

   while(curarea)
      {
      if(curarea->newmail)
         {
         curarea->newmail = 0;

         if( (curarea->type == ECHOMAIL) &&
             (
               (curarea->base & MSGTYPE_SQUISH) ||
               (curarea->base & MSGTYPE_SDM) ||
               ((curarea->base & MSGTYPE_JAM) && (cfg.usr.jamlogpath[0]=='\0'))
             )
           )
           {
           if(!logisopen)
             {
             if( (echolog = fopen(cfg.usr.echolog, "at")) == NULL)
                {
                sprintf(msg, "Can't open %s!", cfg.usr.echolog);
                Message(msg , -1, 0, NO);
                return;
                }
             else
                logisopen=1;
             }

           if(logisopen)
              fprintf(echolog, "%s\n", curarea->tag);
           }
         }
      curarea = curarea->next;
      }

   if(logisopen)
      fclose(echolog);

}


