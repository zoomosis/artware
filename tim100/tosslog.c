#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <share.h>
#include <string.h>
#include <time.h>

#include <msgapi.h>
#include <progprot.h>

#include "wrap.h"
#include "tstruct.h"

#include "global.h"

#include <video.h>
#include <scrnutil.h>
#include "message.h"


void creat_touch(char *filename);


void add_tosslog(AREA *area, MSG *areahandle)
{
   int tosslog;
   char temp[120];
   dword newnumber;
   dword current;


   area->newmail = 1;

   if(area->type == LOCAL)
        return;

   if(area->type == NETMAIL)
        {
        if(cfg.usr.netsema[0] != '\0')
          {
          creat_touch(cfg.usr.netsema);
          }
        }

   if(!(area->base & MSGTYPE_JAM))
      return;

   if(cfg.usr.jamlogpath[0] == '\0') // No JAMLOG defined!
      return;

   current = MsgGetCurMsg(areahandle);
   newnumber = MsgMsgnToUid(areahandle, current);

   if(area->base & MSGTYPE_JAM)
     {
     if(area->type == ECHOMAIL)
        sprintf(temp, "%s\\echomail.jam", cfg.usr.jamlogpath);
     else
        sprintf(temp, "%s\\netmail.jam", cfg.usr.jamlogpath);

     if( (tosslog = open(temp, O_TEXT | O_CREAT | O_WRONLY | SH_DENYWR, S_IWRITE | S_IREAD)) == -1)
         {
         Message("Error opening JAM echomail.jam file!", -1, 0, YES);
         return;
         }

     lseek(tosslog, 0L, SEEK_END);
     sprintf(temp, "%s %lu\n", Strip_Trailing(area->dir, '\\'), newnumber);
     write(tosslog, temp, strlen(temp));
     close(tosslog);
     }
}


void creat_touch(char *filename)
{
   time_t    tnow;
   struct tm tmnow;
   struct ftime ft;
   FILE *f;

   tnow  = time(NULL);
   tmnow = *localtime(&tnow);

   ft.ft_year  = tmnow.tm_year - 80;
   ft.ft_month = tmnow.tm_mon + 1;
   ft.ft_day   = tmnow.tm_mday;
   ft.ft_hour  = tmnow.tm_hour;
   ft.ft_min   = tmnow.tm_min;
   ft.ft_tsec  = tmnow.tm_sec/2;

   if ((f = fopen(filename, "r+b")) != NULL)
         setftime(fileno(f), &ft);
   else if ((f = fopen(filename, "w")) != NULL)
         setftime(fileno(f), &ft);
   else  Message("Can't create semaphore file!", -1, 0, YES);

   if (f)
         fclose(f);

}



