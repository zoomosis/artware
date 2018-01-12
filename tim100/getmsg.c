#include <stdio.h>
#include <alloc.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>

#include <msgapi.h>
#include "wrap.h"
#include "xmalloc.h"
#include "tstruct.h"
#include <msgapi.h>
#include <video.h>
#include <scrnutil.h>
#include "message.h"
#include "global.h"
#include "timer.h"

MMSG *GetMsg(long curno, MSG *areahandle, int doformat, AREA *area);
void FixReplies(MMSG *curmsg, MSG *areahandle);
void checkaddress(MMSG *curmsg, AREA *area);
void cleanup(char *s);
void get_JAM_thread(MMSG *curmsg, MSG *areahandle);

void pascal normalize(char *s);
int MarkReceived(MSG *areahandle, dword no, int not);


MMSG *GetMsg(long curno, MSG *areahandle, int doformat, AREA *area)
{
    MMSG  *curmsg;
    MSGH  *msghandle;

    LINE *templptr = NULL, *last;

    char temp[256];

    int l, p;

    dword   result  = 0,
            txtlen  = 0,
            ctrllen = 0;


    if (!(msghandle=MsgOpenMsg(areahandle, MOPEN_READ, curno)))
       {
       sprintf(temp, "Can't open message (%d)!", curno);
       Message(temp, 2, 0, YES);
       return NULL;
       }

    /* Get space for a msg */

    curmsg = xcalloc(1, sizeof(MMSG));

    /* Read the length of this message */

    txtlen  = MsgGetTextLen(msghandle);


    /* Sanity checks */

    if( (dword) txtlen == (dword) -1L )
       {
       txtlen = (dword) 0L;
       }

    if( (dword) txtlen < (dword) 0L )
       txtlen = (dword) 0L;

    if( (dword) txtlen > (dword) 64000L )
       {
       Message("Extra large msg detected!!!", -1, 0, YES);
       txtlen = (dword) 0L;
       }

    curmsg->txtsize = txtlen;


    ctrllen = MsgGetCtrlLen(msghandle);

    if (ctrllen == (dword) -1L)
       ctrllen = (dword) 0L;

    curmsg->ctrlsize = (dword) ctrllen;

    /* Get the required memory to store (unformatted) text and ctrlinfo */

    if (txtlen)
       curmsg->msgtext = xmalloc( (unsigned) txtlen + 1);
    else
       curmsg->msgtext = NULL;  /*(xmalloc(80);*/

    if (ctrllen)
       curmsg->ctxt = xcalloc(1, (unsigned) ctrllen + 1);
    else curmsg->ctxt=NULL;

    /* Get it from disk */


    result = MsgReadMsg(msghandle, &curmsg->hdr, 0L, txtlen,
                                   curmsg->msgtext, ctrllen, curmsg->ctxt);


    if (
         (MsgCloseMsg(msghandle) == -1) ||
         (result == (dword) -1)
       )          /* Oh, oh, error! OLD: check != txtlen */
       {
       sprintf(temp, "Error reading msg no %d!", curno);
       Message(temp, 2, 0, YES);
       if(curmsg->msgtext) free(curmsg->msgtext);
       if(curmsg->ctxt)   free(curmsg->ctxt);
       free(curmsg);
       return NULL;
       }

    /* Make sure it's NULL terminated */

    if (txtlen) *(curmsg->msgtext + txtlen) = '\0';

    curmsg->uid = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));

    /* Check if it is a personal message, if so set received bit */

   if(area != NULL) /* Only if we have area info (not fast copy etc) */
         {
         for(l=0; ((cfg.usr.name[l].name[0] != '\0') && (l<10)); l++)
             {
             if(strcmpi(curmsg->hdr.to, cfg.usr.name[l].name)==0)
                {
                for(p=0; ((cfg.usr.address[p].zone != 0) && (p<25)); p++)
                   {
                   if(
                       (
                       (cfg.usr.address[p].zone  == curmsg->hdr.dest.zone) &&
                       (cfg.usr.address[p].net   == curmsg->hdr.dest.net)  &&
                       (cfg.usr.address[p].node  == curmsg->hdr.dest.node) &&
                       (cfg.usr.address[p].point == curmsg->hdr.dest.point)
                       )
                     || (area->type != NETMAIL)
                     )
                       {
                       curmsg->status |= PERSONAL;
                       if (!(curmsg->hdr.attr & MSGREAD))
                          {
                          if(MarkReceived(areahandle,
                                          MsgGetCurMsg(areahandle), 0) == -1)
                             Message("Error updating message to 'Received' status!", -1, 0, YES);
                          }
                       break;
                       }
                   }
                }
             }
         }

    /*  Wrap the text, strip strange chars etc., but only if there */
    /* _is_ text, and if we really have to wrap (doformat != 0)    */


    if (txtlen && doformat && (curmsg->msgtext[0] != '\0'))
       {
       normalize(curmsg->msgtext);
       curmsg->txt=FormatText(curmsg->msgtext, maxx);
       }

    else if (doformat)      /* so it was an empty message */
       {
       if(curmsg->msgtext)
           curmsg->msgtext = realloc(curmsg->msgtext, 80);
       else curmsg->msgtext = xcalloc(1, 80);
       if(!curmsg->msgtext) Message("Out of memory!", -1, 254, NO);
       strcpy(curmsg->msgtext, "\r");
       curmsg->txt = FormatText(curmsg->msgtext, maxx);
       }

    if (doformat)    /* then we format & analyse the kludges as well */
         {
         if(ctrllen && (curmsg->ctxt[0] != '\0'))
            {
            curmsg->ctext = CvtCtrlToKludge(curmsg->ctxt);      /* Gives a formatted copy, original kept for C)hange msg for example */
            templptr = last = FormatText(curmsg->ctext, maxx);  /* Format kludges */
            while(last->next)
                 last = last->next; /* Get last line */

            if((last->ls[0] == '\0') && (last->prev) )    /* last line empty (but is not only line), strip it */
              {
              last->prev->next = curmsg->txt;  /* link prev line w/ msgtext */
              free(last);                      /* get rid of this last one  */
              }
            else last->next = curmsg->txt;
            curmsg->txt = templptr;   /* let it point to beginning of kludges, which is now message start */
            }

         if(area)
            {
            checkaddress(curmsg, area);
            if(area->base & MSGTYPE_JAM)
               get_JAM_thread(curmsg, areahandle);
            }

         }  /* if (ctrllen) */


    return curmsg;
}


/* This routine will first check for msgid, and also get the addres from there */
/* If there is no MSGID, it will try to find the origin and get it from there  */
/* It now also scans for replyto/replyaddr kludges, and Flags DIR stuff */

void checkaddress(MMSG *curmsg, AREA *area)

{
   NETADDR afz;
   LINE *lptr = curmsg->txt;
   char *begin, *end, temp[80], *tmpptr;
   int count=0, have_address=0;

   memset(&afz, '\0', sizeof(NETADDR));
   memset(temp, '\0', sizeof(temp));

   if (area->type != NETMAIL)
      memset(&curmsg->hdr.orig, '\0', sizeof(NETADDR));
   else have_address = 1; /* for netmail, the address is OK already */

   for(; lptr; lptr = lptr->next)
      {
      if(lptr->status & KLUDGE)
         {
         if(strncmp(lptr->ls, "\01MSGID:", 7) == 0)
            {
            if(curmsg->id[0] == '\0') /* Only if we don't have one - next might be MSGID in msgtext.. ?! */
               strcpy(curmsg->id, lptr->ls+8);
            if (area->type != NETMAIL)
               {
               tmpptr=curmsg->id;
               while( (!(isdigit(*tmpptr))) && (*tmpptr!=0) ) tmpptr++;
               sscanf(tmpptr, "%hd:%hd/%hd.%hd", &afz.zone, &afz.net, &afz.node, &afz.point);
               if( (afz.zone != 0) && (afz.net != 0) )
                 {
                 curmsg->hdr.orig = afz;
                 have_address = 1; /* We have address, could stop */
                 }
               }
            }
         else if(strncmpi(lptr->ls, "\01REPLYTO", 8) == 0)
            {
            strcpy(temp, lptr->ls+9);
            if((tmpptr=strtok(temp, " \t\n\r")) != NULL)
               {
               sscanf(tmpptr, "%hd:%hd/%hd.%hd", &curmsg->rep_addr.zone,
                                          &curmsg->rep_addr.net,
                                          &curmsg->rep_addr.node,
                                          &curmsg->rep_addr.point);

               if((tmpptr=strtok(NULL, "\n\r")) != NULL)
                  {
                  while(*tmpptr==' ') tmpptr++;
                  strcpy(curmsg->rep_gate, tmpptr);
                  }
               }

            } /* ^AREPLYTO */

         else if(strncmpi(lptr->ls, "\01REPLYADDR", 10) == 0)
            {
            strcpy(temp, lptr->ls+11);
            tmpptr=temp;
            while(*tmpptr == ' ') tmpptr++;
            strcpy(curmsg->rep_name, tmpptr);
            }

         if(                 /* FSC35 'replyto' kludges are both there */
              (curmsg->rep_addr.zone != 0)  &&
              (curmsg->rep_gate[0] != '\0') &&
              (curmsg->rep_name[0] != '\0')
           )
           curmsg->status |= REPLYTO; /* So remember this (and use in ALT-N) */
         }

      else if((lptr->status & ORIGIN) && (!have_address))
        {
        end   = strrchr(lptr->ls, ')');       /* Closing bracket for address */
        begin = strrchr(lptr->ls, '(');       /* Opening bracket for address */
        if ( begin && end && (end > begin) )
           {
           while( (!isdigit(*begin)) && (begin<end))
              begin++;                                /* find first digit */
           while( (!isdigit(*end)) && (begin<end) )
              end--;
           memset(temp, '\0', sizeof(temp));
           strncpy(temp, begin, end-begin+1);
           address_expand(temp, &afz, -1);
           if (area->type != NETMAIL)
              curmsg->hdr.orig = afz;
           }
        }
      /* If we have address, and 20 lines already scanned, stop scanning
         for any more kludge-info.. */

      if((count++ > 20) && have_address)
         return;
      }

}



#ifdef __NEVER__
void pascal normalize(char *s)
{
    char   *tmp = s;

    while (*s)
        {
        if ((unsigned) (0xff & *s) == (unsigned) 0x8d)
            s++;
        else if (*s == '\n')
            s++;
        else
            *tmp++ = *s++;
        }
    *tmp = '\0';
}
#endif


void get_JAM_thread(MMSG *curmsg, MSG *areahandle)
{
   long current, nextread;
   int pos = 1;
   MSGH *msghandle;
   XMSG header;

   curmsg->replynext = curmsg->hdr.replies[1];
   curmsg->hdr.replies[1] = 0;

   if(curmsg->hdr.replies[0] == 0)  /* No replynext field */
      return;

   current = MsgGetCurMsg(areahandle);
   nextread  = curmsg->hdr.replies[0];

   while(nextread && (pos < 10))
     {
     if( (msghandle=MsgOpenMsg(areahandle, MOPEN_READ, nextread)) == NULL)
        break;

     if(MsgReadMsg(msghandle, &header, 0L, 0L, NULL, 0L, NULL) == -1)
        {
        Message("Error reading reply-chain message!", -1, 0, YES);
        MsgCloseMsg(msghandle);
        break;
        }

     MsgCloseMsg(msghandle);

     nextread = curmsg->hdr.replies[pos++] = header.replies[1];
     }

   if((msghandle=MsgOpenMsg(areahandle, MOPEN_READ, current)) != NULL) /* Reset current MsgApi msg */
      MsgCloseMsg(msghandle);

}



/* ----------------------------------------------- */
/* -  Update a message to the 'received' status  - */
/* ----------------------------------------------- */


int MarkReceived(MSG *areahandle, dword no, int not)
{
  XMSG *temphdr   = NULL;
  MSGH *msghandle = NULL;
  int didlock = 0;
  int retval = -1;

  if(!areahandle->locked)
    {
    MsgLock(areahandle);
    didlock=1;
    }

  temphdr = xcalloc(1, sizeof(XMSG));

  if((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, no)) == NULL)
      goto Exit;

  if(MsgReadMsg(msghandle, temphdr, 0L, 0L, NULL, 0L, NULL) == -1)
      goto Exit;

  if(not)
     temphdr->attr &= ~MSGREAD;
  else
     temphdr->attr |= MSGREAD;

  if(MsgWriteMsg(msghandle, 0, temphdr, NULL, 0L, 0L, 0L, NULL) == -1)
      goto Exit;

  retval = 0;

  Exit:

  if(msghandle)
     MsgCloseMsg(msghandle);

  if(temphdr)
     free(temphdr);

  if(didlock)
     MsgUnlock(areahandle);

  return retval;

}
