#include "includes.h"

void UpdateEchotosslog(char *tag);


void add_tosslog(AREA *area, MSG *areahandle)
{
   int tosslog;
   char temp[1024], tmpmsg[1024];
   dword newnumber;
   dword current;


   area->newmail = 1;

   if(area->type == LOCAL)
        return;

   if(area->type == NETMAIL)
     {
     if(cfg.usr.netsema[0] != '\0')
       creat_touch(cfg.usr.netsema);
     }

   if( (area->base & MSGTYPE_JAM) && cfg.usr.jamlogpath[0] != '\0')
     {
     current = MsgGetCurMsg(areahandle);
     newnumber = MsgMsgnToUid(areahandle, current);

     if(area->type == ECHOMAIL || area->type == NEWS)
        sprintf(temp, "%s\\echomail.jam", cfg.usr.jamlogpath);
     else
        sprintf(temp, "%s\\netmail.jam", cfg.usr.jamlogpath);

     if( (tosslog = sopen(temp, O_TEXT | O_CREAT | O_WRONLY, SH_DENYWR, S_IWRITE | S_IREAD)) == -1)
         {
         sprintf(tmpmsg, "Error opening %s (errno: %d)!", temp, errno);
         Message(tmpmsg, -1, 0, YES);
         return;
         }

     lseek(tosslog, 0L, SEEK_END);
     sprintf(temp, "%s %lu\n", Strip_Trailing(area->dir, '\\'), newnumber);
     write(tosslog, temp, strlen(temp));
     close(tosslog);
     }
   else if((area->type == ECHOMAIL || area->type == NEWS) && !(area->base & MSGTYPE_HMB))
     {
     UpdateEchotosslog(area->tag);
     }


}


// ==============================================================

void UpdateEchotosslog(char *tag)
{
   FILE *tl;
   char temp[120];
   int found = 0;
   char *cr;

   if(cfg.usr.echolog[0] == '\0')
     return;

   tl = _fsopen(cfg.usr.echolog, "a+t", SH_DENYRW);
   if(tl == NULL)
     {
     sprintf(msg, "Error opening %s!", cfg.usr.echolog);
     Message(msg, -1, 0, YES);
     return;
     }

   fseek(tl, 0, SEEK_SET);
   while(fgets(temp, 119, tl) != NULL)
     {
     if( (cr=strchr(temp, '\n')) != NULL)
       *cr = '\0';
     if(strcmpi(temp, tag) == 0)
       {
       found = 1;
       break;
       }
     }

   if(!found)
     {
     fseek(tl, 0, SEEK_END);
     fprintf(tl, "%s\n", tag);
     }

   fclose(tl);

}

// ==============================================================

