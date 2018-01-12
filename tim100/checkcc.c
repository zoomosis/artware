#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <alloc.h>
#include <time.h>
#include <conio.h>
#include <stdlib.h>

#include <video.h>
#include <scrnutil.h>

#include <msgapi.h>
#include <progprot.h>
#include "input.h"
#include "xmalloc.h"
#include "wrap.h"
#include "tstruct.h"
#include "global.h"
/* #include <mstruct.h> */
#include "reply.h"
#include "message.h"
#include "nodeglue.h"
#include "tosslog.h"
#include "header.h"

/* prototypes */

void check_cc(AREA *area, MSG *areahandle, XMSG *header, char *msgbuf);

XMSG *check_element(char *charptr, XMSG *header, AREA *area, char *usenet);
void write_one(MSG *areahandle, AREA *area, XMSG *curheader, char *msgbuf, char *usenet);
void get_bomb_list(char *charptr, XMSG *header, char *msgbuf, AREA *area, MSG *areahandle);
XMSG *get_copy(XMSG *header);

char note[200];     /* Used to store: original was to.. */


void check_cc(AREA *area, MSG *areahandle, XMSG *header, char *msgbuf)
{

   char *start = msgbuf, *charptr, *nextnl, *nextel;
   char temp[256];
   XMSG *curheader;
   char *end;
   char didlock = 0;
   size_t howmuch;
   char usenet[80] = "";

   if(msgbuf == NULL)
      return;

   while( (nextnl = strchr(start, '\r')) != NULL)  /* analyse line by line.. */
      {
      memset(temp, '\0', sizeof(temp));
      howmuch = min(255, (size_t)(nextnl - start));
      strncpy(temp, start, howmuch);    /* copy line (strtok messes it up) */
      end = temp + sizeof(temp);
      charptr = temp;
      start = nextnl+1;

      while(*charptr == ' ' || *charptr ==  '\r')  /* skip leading space */
         charptr++;

      if(strncmpi(charptr, "CC:", 3) != 0)   /* No CC:, return */
         return;

      if(!didlock) /* not locked yet, do it now */
        {
        while(1) /* We lock, don't unlock: we need to write/lock original message, unlock after that */
           {
           if( MsgLock(areahandle) == -1 )
              {
              Message("Can't lock area! Wait for retry, <ESC> to abort", 10, 0, YES);
              if( xkbhit() && (get_idle_key(1) == 27) )
                 return;
              }
           else break;
           }
        }  /* if !didlock */

      charptr = charptr + 3;  /* Skip CC: chars */

      while(*charptr == ' ')  /* skip space again */
         charptr++;

      /* We do this for every line w/ a CC:, but that's better than   */
      /* doing it earlier, before we even know the msg *has* a CC: .. */

      sprintf(note, "-=> Note: Original was to: %s\r\r", header->to);

      charptr = strtok(charptr, ",");     /* get elements, separated by comma's */
      if(charptr != NULL)
         nextel  = charptr + strlen(charptr) + 1;


      while(charptr)
        {
        while(*charptr == ' ') charptr++;
        if(*charptr == '<')
            get_bomb_list(charptr+1, header, msgbuf, area, areahandle);
        else if( (curheader = check_element(charptr, header, area, usenet)) != NULL)
            write_one(areahandle, area, curheader, msgbuf, usenet);
        if(nextel <= end)
           {
           charptr = strtok(nextel, ",");
           if(charptr)
              nextel  = charptr + strlen(charptr) + 1;
           }
        else charptr=NULL;
        }

      }

}


/* -------------------------------------------------------------- */
/* Here we look at the element, try to expand it to a full header */
/* modified from the (passed) original header                     */
/* If we can't make it, we return NULL, else the created header   */
/* -------------------------------------------------------------- */


XMSG *check_element(char *el, XMSG *header, AREA *area, char *usenet)

{
   XMSG *thisheader;
   char *sep;
   char elcopy[80], *element;

   strcpy(elcopy, el);
   element = elcopy;

   memset(usenet, '\0', sizeof(usenet));

   thisheader = get_copy(header);

   while(*element == ' ')
      element++;

   while(element[strlen(element)-1] == ' ')
      element[strlen(element)-1] = '\0';


   /* we check if there is an explicit address, using a '#' */

   if((sep = strchr(element, '#')) != NULL)
      {
      strncpy(thisheader->to, element, (size_t) (sep - element));
      address_expand(sep+1, &thisheader->dest, area->aka);
      return thisheader;
      }

   strcpy(thisheader->to, element);

   /* we start to check if the element is a macro.. */

   if(check_alias(thisheader, usenet))   /* return 0 if no luck, # of elements else */
         return thisheader;

   if(check_node(thisheader, area->aka, NOPROMPT) == 0)  /* 0 == not found */
      {
      sprintf(msg, "%s not found in nodelist!", thisheader->to);
      Message(msg, -1, 0, YES);
      free(thisheader);
      return NULL;
      }

   return thisheader;
}


/* Write a CC: message, with modified header passed to it */


void write_one(MSG *areahandle, AREA *area, XMSG *curheader, char *msgbuf, char *usenet)
{
   char *kludges;
   dword textlen, totlen, clen;
   MSGH *msghandle;
   char oldzone;
   char finalnote[200];

   if(usenet[0] != '\0') /* Add usenet TO: field from macro? */
      sprintf(finalnote, "TO: %s\r\r%s", usenet, note);
   else
      strcpy(finalnote, note);

   /* First we try to match AKA's... */

   matchaka(curheader);

   kludges = MakeKludge(NULL, curheader, 1);  /* Last parm == 1, always netmail */

   oldzone = cfg.usr.zonegate;                     /* save value */
   if(cfg.usr.zonegate == 2) cfg.usr.zonegate = 1; /* Don't ask confirm */
   zonegate(curheader, kludges);
   cfg.usr.zonegate = oldzone;                     /* reset value */

   if((msghandle=MsgOpenMsg(areahandle, MOPEN_CREATE, 0)) == NULL)
      {
      Message("Error creating message!", -1, 0, YES);
      goto close_it_up;
      }

   textlen = (dword)strlen(msgbuf) + (dword) 1;
   totlen = (dword) textlen + (dword) strlen(finalnote);
   clen = (dword) strlen(kludges) + (dword) 1;

   if (MsgWriteMsg(msghandle,         /* msghandle        */
                   0,                 /* append?          */
                   curheader,         /* the header       */
                   finalnote,         /* msg text         */
           (dword) strlen(finalnote), /* length this pass */
                   totlen,            /* len total msg    */
                   clen,              /* len of kludges   */
                   kludges)           /* ptr to kludges   */
                   == -1)
      {
      Message("Error writing message!", -1, 0, YES);
      MsgCloseMsg(msghandle);
      goto close_it_up;
      }

   if (MsgWriteMsg(msghandle,       /* msghandle        */
                   1,               /* append?          */
                   NULL,            /* the header       */
                   msgbuf,          /* msg text         */
                   textlen,         /* length this pass */
                   totlen,          /* len total msg    */
                   0,               /* len of kludges   */
                   NULL)            /* ptr to kludges   */
                   == -1)
      {
      Message("Error writing message!", -1, 0, YES);
      MsgCloseMsg(msghandle);
      goto close_it_up;
      }
   else
      {
      add_tosslog(area, areahandle);
      }

   MsgCloseMsg(msghandle);

   close_it_up:

   if (curheader) free(curheader);
   if (kludges) free(kludges);
}


/* get a list from disk, with names to CC: the message to.. */

void get_bomb_list(char *charptr, XMSG *header, char *msgbuf, AREA *area, MSG *areahandle)
{
   FILE *ccfile;
   char temp[256];
   XMSG *curheader;
   char usenet[80] = "";


   while(*charptr == ' ')
      charptr++;

   while(charptr[strlen(charptr)-1] == ' ')
      charptr[strlen(charptr)-1] = '\0';


   if( (ccfile = fopen(charptr, "rt")) == NULL)
      {
      sprintf(msg, "Can't open %s!", charptr);
      Message(msg, -1, 0, YES);
      return;
      }

   while(fgets(temp, 256, ccfile))
     {
     if(temp[0] == '\0' || temp[0] == '\n' || temp[0] == ';')
        continue;

     charptr = temp;

     while(*charptr == ' ') charptr++;

     if(charptr[strlen(charptr)-1] == '\n')
        charptr[strlen(charptr)-1] = '\0';

     while(charptr[strlen(charptr)-1] == ' ')
      charptr[strlen(charptr)-1] = '\0';


     if( (curheader = check_element(charptr, header, area, usenet)) != NULL)
            write_one(areahandle, area, curheader, msgbuf, usenet);

     }

   fclose(ccfile);

}


XMSG *get_copy(XMSG *header)
{
   XMSG *newheader;
   union stamp_combo combo;
   struct tm *tmdate;
   time_t time_now;

   newheader = xcalloc(1, sizeof(XMSG));

   strcpy(newheader->from, header->from);

   newheader->orig.zone  = header->orig.zone;
   newheader->orig.net   = header->orig.net;
   newheader->orig.node  = header->orig.node;
   newheader->orig.point = header->orig.point;

   strcpy(newheader->subj, header->subj);

   newheader->attr = header->attr;
   newheader->attr |= MSGKILL;

   time(&time_now);
   tmdate=localtime(&time_now);
   newheader->date_written = newheader->date_arrived =
                           (TmDate_to_DosDate(tmdate,&combo))->msg_st;

   return newheader;
}
