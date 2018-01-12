#include <stdio.h>
#include <string.h>
#include <msgapi.h>
#include <alloc.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>

#include "wrap.h"
#include "tstruct.h"
#include "copymsg.h"

#include <video.h>
#include <scrnutil.h>
#include "message.h"

#include "getmsg.h"
#include "readarea.h"
#include "global.h"
#include "idlekey.h"
#include "select.h"
#include "header.h"
#include "reply.h"
#include "repedit.h"
#include "xmalloc.h"
#include "showhdr.h"
#include "tosslog.h"

/* -------------------------------------------------------------- */


void CopyMsg(MSG *areahandle, MSG *toareahandle, AREA *toarea, dword msgno, char kill)
{
   MSGH *msghandle;
   MMSG *curmsg;
   char temp[80];
   dword txtlen, clen;


   if ( (curmsg = GetMsg(msgno, areahandle, 0, NULL)) == NULL)
      {
      sprintf(temp, "Error reading message %d!", msgno);
      Message(temp, -1, 0, YES);
      return;
      }


   /* clean it up (SEEN-BY strip etc) */

//   if(curmsg->msgtext)
//      clean_end_message(curmsg->msgtext);


   /* Message read now, goto write */

   // Recalc lengths (esp. for Hudson, len is inaccurate for those..

   txtlen = curmsg->msgtext ? strlen(curmsg->msgtext)+1 : 0L;
   clen   = curmsg->ctxt ? strlen(curmsg->ctxt   )+1 : 0L;

//   curmsg->hdr.attr = toarea->stdattr;      /* Reset msgbits */

   memset(&curmsg->hdr.replyto, '\0', sizeof(curmsg->hdr.replyto));
   memset(&curmsg->hdr.replies, '\0', sizeof(curmsg->hdr.replies));

   /* Write it; no locking here, done in list.c! */

   if((msghandle=MsgOpenMsg(toareahandle, MOPEN_CREATE, 0)) == NULL)
      {
      Message("Error writing message!", -1, 0, YES);
      }
   else
      {
      if(MsgWriteMsg(msghandle, 0, &curmsg->hdr, curmsg->msgtext, txtlen, txtlen, clen, curmsg->ctxt) == -1)
         {
         Message("Error writing message!", -1, 0, YES);
         }

      if(MsgCloseMsg(msghandle) == -1)
         Message("Error closing message!", -1, 0, YES);

      add_tosslog(toarea, toareahandle);

      if (kill)
         {
         if(MsgKillMsg(areahandle, msgno) == -1)
            Message("Error deleting message!", -1, 0, YES);
         }
      }


   ReleaseMsg(curmsg, 1);

}


/* ----------------------------------------------- */


void MoveMessage(MSG *areahandle, AREA *area, MMSG *curmsg)

{
   BOX   *border;
   char  infotext[320];
   int   c=32;
   dword no;
   MMSG  *message = NULL;
   AREA  *toarea = NULL;
   MSG   *toareahandle = NULL;
   MSGH  *msghandle = NULL;
   dword textlen=0, totlen=0, clen=0, i, reporiglen=0;
   UMSGID absolute;
   XMSG *replace_header = NULL;
   char *replace_kludges = NULL, *replace_origin = NULL;
   char usenet_address[80] = "";


   no = MsgGetCurMsg(areahandle);
   absolute = MsgMsgnToUid(areahandle, no);

   border=initbox(10,19,14,60,cfg.col.popframe,cfg.col.poptext,SINGLE,YES,' ');
   drawbox(border);
   boxwrite(border,1,1,"    [M]ove, [C]opy or [F]orward?");
   MoveXY(1,13);

   while(
         (c != 'm') &&
         (c != 'M') &&
         (c != 'c') &&
         (c != 'C') &&
         (c != 'f') &&
         (c != 'F') &&
         (c != 27)
        )
      c = get_idle_key(1);

   delbox(border);

   if (c == 27) return;

   if((message = GetMsg(no, areahandle, 0, area)) == NULL)
      {
      Message("Error reading Message!", -1, 0, YES);
      return;
      }

   if(message->msgtext)
      clean_end_message(message->msgtext);

   memset(&message->hdr.replyto, '\0', sizeof(message->hdr.replyto));
   memset(&message->hdr.replies, '\0', sizeof(message->hdr.replies));

   savescreen();
//   clsw(cfg.usr..);

   if( (toarea = SelectArea(cfg.first, 1, cfg.first)) == NULL)
       goto close_up;

   message->hdr.attr |= toarea->stdattr;      /* Reset msgbits */
   message->hdr.attr &= ~(MSGSENT|MSGSCANNED);

   if(toarea == area) toarea = NULL;

   putscreen();

   if (toarea != NULL)
      {
      if(!(toareahandle=MsgOpenArea(toarea->dir, MSGAREA_CRIFNEC, toarea->base)))
         {
         Message("Error opening area!", -1, 0, YES);
         goto close_up;
         }
      get_custom_info(toarea);
      }
   else
      toareahandle = areahandle;  /* Forward / Move / Copy to same area */


   memset(infotext, '\0', sizeof(infotext));

   switch(c)
     {
     case 'M':
     case 'm':

        if(cfg.usr.status & SHOWNOTES)
           sprintf(infotext, "-=> Note:\rMoved (from: %s) by %s using timEd.\r\r", area->tag, cfg.usr.name[custom.name].name);

        break;

     case 'C':
     case 'c':

        if(cfg.usr.status & SHOWNOTES)
           sprintf(infotext, "-=> Note:\rCopied (from: %s) by %s using timEd.\r\r", area->tag, cfg.usr.name[custom.name].name);

        break;

     case 'F':
     case 'f':


        /* Make a 'fresh header, and new kludges */

        clsw(cfg.col.msgtext);
        if ( (replace_header = MakeHeader(toareahandle, NULL, 0, toarea ? toarea : area, 0, message->hdr.subj, usenet_address)) == NULL)
           goto close_up;

        if(usenet_address[0] == '\0')
           sprintf(infotext, "-=> Note:\rForwarded (from: %s) by %s using timEd.\rOriginally from %s (%d:%d/%d.%d) to %s.\rOriginal dated: %s\r\r", area->tag, cfg.usr.name[custom.name].name, message->hdr.from, curmsg->hdr.orig.zone, curmsg->hdr.orig.net, curmsg->hdr.orig.node, curmsg->hdr.orig.point, message->hdr.to, MakeT(&curmsg->hdr.date_written));
        else
           sprintf(infotext, "TO: %s\r\r-=> Note:\rForwarded (from: %s) by %s using timEd.\rOriginally from %s (%d:%d/%d.%d) to %s.\rOriginal dated: %s\r\r", usenet_address, area->tag, cfg.usr.name[custom.name].name, message->hdr.from, curmsg->hdr.orig.zone, curmsg->hdr.orig.net, curmsg->hdr.orig.node, curmsg->hdr.orig.point, message->hdr.to, MakeT(&curmsg->hdr.date_written));

        replace_kludges = MakeKludge(NULL, replace_header,
             ((toarea && (toarea->type == NETMAIL)) ||
              (!toarea && (area->type == NETMAIL))) );

        if( (toarea && (toarea->type == ECHOMAIL)) ||
            (!toarea && (area->type && ECHOMAIL)) )
          {
          if(toarea) get_custom_info(toarea);
          if(message->msgtext)
             invalidate_origin(message->msgtext);
          replace_origin = make_origin(custom.aka);
          for(i=0; i < strlen(replace_origin); i++) /* Replace \n with \r.. */
            {
            if(replace_origin[i] == '\n')
               replace_origin[i] = '\r';
            }

          if(toarea) get_custom_info(area);
          }


        break;

     default:
        goto close_up;
     }


   while(1)
      {
      if( MsgLock(toareahandle) == -1 )
         {
         Message("Can't lock area! Wait for retry, <ESC> to abort", 10, 0, YES);
         if( xkbhit() && (get_idle_key(1) == 27) )
            goto close_up;
         }
      else break;
      }

   if((msghandle=MsgOpenMsg(toareahandle, MOPEN_CREATE, 0)) == NULL)
      {
      Message("Error creating message!", -1, 0, YES);
      goto close_up;
      }

   textlen=message->msgtext ? strlen(message->msgtext)+1 : 0;
   if(infotext)
       totlen=textlen+strlen(infotext);
   else totlen = textlen;

   if(replace_origin)
     {
     reporiglen = strlen(replace_origin);
     totlen += reporiglen;
     textlen--;
     }

   if(replace_kludges)
      clen = strlen(replace_kludges) + 1;
   else
      {
      if(message->ctxt)
          clen = strlen(message->ctxt)+1;
      else clen = 0;
      }

   if(
       (MsgWriteMsg(msghandle, 0, replace_header ? replace_header : &message->hdr, infotext, infotext ? strlen(infotext) : 0, totlen, clen, replace_kludges ? replace_kludges : message->ctxt)) ||
       (MsgWriteMsg(msghandle, 1, NULL, message->msgtext, textlen, totlen, 0L, NULL)) ||
       (MsgWriteMsg(msghandle, 1, NULL, replace_origin, reporiglen+1, totlen, 0L, NULL))
     )
      {
      Message("Error writing message!", -1, 0, YES);
      goto close_up;
      }
   else
      {
      if(toarea)
         {
         add_tosslog(toarea, toareahandle);
         }
      else
         {
         add_tosslog(area, areahandle);
         }
      }


   if ((c == 'M') || (c == 'm'))
      MsgKillMsg(areahandle, MsgUidToMsgn(areahandle, absolute, UID_EXACT));

   /* Here we close & release all */

   close_up:

   if (msghandle && MsgCloseMsg(msghandle))
      Message("Error closing message!", -1, 0, YES);


   if(toareahandle != NULL) /* Only if we actually have opened/locked/written */
       MsgUnlock(toareahandle);

   if ((toarea != NULL) && (toareahandle != NULL))
      {
      ScanArea(toarea, toareahandle);
      MsgCloseArea(toareahandle);
      }


   if (message) ReleaseMsg(message, 1);

   if (replace_header)  free(replace_header );
   if (replace_kludges) free(replace_kludges);

/*   Do *not* do this, it's a static local var for make_origin!! */
/*   -=- if (replace_origin)  free(replace_origin ); -=-         */


   ScanArea(area, areahandle);

}


/* -------------------------------------------------------------- */
/* Strip CR's, PATHlines, SEEN-BY's, and VIA lines from the end   */
/* of the messages. This makes them ready for re-export (when     */
/* moving, copying etc)                                           */
/* -------------------------------------------------------------- */


dword clean_end_message(char *msgbuf)
{

   #define TRUE  1
   #define FALSE 0

   char *cptr, *end, *tmpptr, *curend=NULL;
   int chopit = FALSE, stophere = FALSE;


   if(msgbuf == NULL) return 0;

   end = cptr = strchr(msgbuf, '\0');  /* goto end */

   /* Do we have anything to work on? */

   if( (end == NULL) || (end == msgbuf) ) return 0;

   cptr--;  /* go back one char from the trailing '\0' */

   while(                      /* Strip all junk at the end */
             ( (*cptr == ' ' ) ||
               (*cptr == '\r') ||
               (*cptr == '\t') ||
               (*cptr == '\n') )   && (cptr > msgbuf)
          )
          *cptr-- = '\0';

   curend = cptr+1;

   while((--cptr >= msgbuf) && (stophere != TRUE) ) /* Go back no further than start of message! */
     {
     if(*cptr == '\r')      /* end of line, what's on beg. of next line? */
        {
        /* Skip LF's and Soft Returns */
        tmpptr = cptr+1;
        while( ((*tmpptr == 0x0A) || (*tmpptr == 0x8D)) && (tmpptr < end) )
               tmpptr++;

        if(*tmpptr == '\0')   /* We're still at the end */
          chopit = TRUE;

        else if(*tmpptr == '\01')   /* Kludge detected */
           {
           if(strncmpi(tmpptr+1, "via", 3) == 0)  /* 'via' kludge, strip */
               chopit = TRUE;
           else if (strncmp(tmpptr+1, "PATH", 4) == 0) /* path, strip */
               chopit = TRUE;
           else stophere = TRUE;    /* unknown, stop chopping! */
           }

        else if(*tmpptr == 'S')  /* Seen-by maybe? */
              {
              if (strncmp(tmpptr, "SEEN-BY:", 7) == 0) /* SB, strip */
                  chopit = TRUE;
              else stophere = TRUE;
              }

        /* else if(*tmpptr == ' ')  /* Origin maybe? */
              {
              if (strncmp(tmpptr, " * Origin: ", 11) == 0) /* Origin! */
                  stophere = TRUE;

              } */
        else stophere = TRUE;

        if(chopit == TRUE)
           {
           *cptr = '\0';
           curend = cptr;
           chopit = FALSE;
           }

        }  /* if */

     } /* while */

   *curend++ = '\r';
   *curend = '\0';

   return (dword) (curend - msgbuf);
}



/* Clean the end of message and check for origin */

int clean_origin(char *msgbuf)
{
   char *cptr, *end, *open, *close;
   char *timname;


   if(msgbuf == NULL) return 1;

   end = cptr = strchr(msgbuf, '\0');  /* goto end */

   if( (end == NULL) || (end == msgbuf) ) return 1;

//   if( (*(cptr-1) == '\r') && (*(cptr-2) == ')') )
//      return 0;    /* All fine and dandy.. */

   cptr--;

   /* strip junk at end of message */

   while(
          ( (*cptr == ' ' ) ||
            (*cptr == '\r') ||
            (*cptr == '\t') ||
            (*cptr == '\n') )   && (cptr > msgbuf)
        )
           *cptr-- = '\0';


   while(--cptr >= msgbuf)
     {
     if(*cptr == '\r')
       {
       cptr++;
       break;
       }
     }

   if(strncmp(cptr, " * Origin: ", 11) != 0)
      return 1;

   if (
        ((open  = strrchr(cptr, '(')) == NULL) ||
        ((close = strrchr(open, ')')) == NULL) ||
        (open > close)
      )
      return 1;

    *(close+1) = '\r';
    *(close+2) = '\0';

    if(strlen(cptr) > 80)
       Message("Warning, Origin line too long!!", -1, 0, 1);

    return 0;

}



void invalidate_origin(char *msg)
{
   char *cptr;


   if(msg == NULL) return;

   cptr = strchr(msg, '\0');  /* goto end */

   if( (cptr == NULL) || (cptr == msg) ) return;

   cptr--;

   /* skip junk at end of message */

   while(
          ( (*cptr == ' ' ) ||
            (*cptr == '\r') ||
            (*cptr == '\t') ||
            (*cptr == '\n') )   && (cptr > msg)
        )
           cptr--;

   /* find first '\r' == last line */

   while( (cptr >= msg) && (*cptr != '\r'))
      cptr--;

   cptr++;

   /* Skip \n and LF */

   while( (*cptr == 0x0A) || (*cptr == 0x8D) )
       cptr++;

   if(strncmp(cptr, " * Origin: ", 11) == 0)  /* Is last line origin? */
      {
      *(cptr+1) = '-';  /* invalidate origin */

      if(cptr>msg) cptr--;

      while( (*cptr == 0x0A) || (*cptr == 0x8D) )
       cptr--;

      cptr--;

      /* goto line before last one.. */

      while( (cptr >= msg) && (*cptr != '\r') )
         cptr--;

      cptr++;
      }

   while( (*cptr == 0x0A) || (*cptr == 0x8D) )
       cptr++;

   if(strncmp(cptr, "---", 3) == 0)     /* Is last line tearline? */
      {
      *(cptr) = *(cptr+1) = *(cptr+2) = '_';
      }


}
