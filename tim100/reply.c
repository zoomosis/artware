#ifndef __OS2__
#pragma inline
#endif

#include <process.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <alloc.h>
#include <msgapi.h>
#include <conio.h>
#include <errno.h>
#include <sys\stat.h>
#include <video.h>
#include <scrnutil.h>
#include <stdlib.h>
#include <progprot.h>

#include "input.h"
#include "xmalloc.h"
#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "showmail.h"
#include "getmsg.h"
#include "select.h"
#include "readarea.h"
#include "attach.h"
#include "message.h"
#include "showhdr.h"
#include "idlekey.h"
#include "checkcc.h"
#include "working.h"
#include "copymsg.h"
#include "header.h"
#include "getbody.h"
#include "repedit.h"
#include "dow.h"
#include "tosslog.h"

#define NORMALCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'. !?,*&^#$%()-_=+[]{}\\\"|';:,.<>/?`~1234567890"
#define ADDRESSCHARS "0123456789:/.-"

static char *mtext[]={	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static char *days[]={ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};


/* ------ Prototypes -------- */


char *MakeKludge    (MMSG *curmsg, XMSG *header, int netmail);
void MakeMessage    (MMSG *curmsg, AREA *area, MSG *areahandle, word reply, UMSGID reply_to_id, char *add_to_top);
void ChangeMessage  (MMSG *curmsg, AREA *area, MSG *areahandle, int bodytoo);
void MakeQuote      (MMSG *curmsg, AREA *area, char *add_to_top, word replytype);
int  pascal IsQuote        (char *line);
void MakeText       (MMSG *curmsg);
void ReplyOther     (MMSG *curmsg, AREA *area);
char *getinitials   (char *name);
void MakeTemplate   (XMSG *header, AREA *area, char *usenet_address);
char *make_origin   (int aka);
char *make_rephello (XMSG *header, word replytype);
char *make_hello    (XMSG *header);
char *expand        (char *line, XMSG *header);
int  check_alias    (XMSG *hdr, char *usenet_address);
void address_expand (char *line, NETADDR *fa, int aka);
void zonegate       (XMSG *header, char *kludges);
int  check_gated    (MMSG *msg);


LINE *firstmemline = NULL;        /* For use with the internal editor */
LINE *lastmemline  = NULL;
void add_text(char *line);  /* Add a line to text in mem        */
int WriteReplyLink(AREA *area, MSG *areahandle, dword orig, dword new);

/* ----------------------------------------------- */
/* reply == 1 : normal reply                       */
/* reply == 2 : reply (also) to original addressee */
/* reply == 3 : turbo reply                        */
/* ----------------------------------------------- */

void MakeMessage(MMSG *curmsg, AREA *area, MSG *areahandle, word reply, UMSGID reply_to_id, char *add_to_top)

{
	MSGH	*msghandle;
	XMSG	*header=NULL;
	dword textlen, totlen, clen, origmsg;
	char	*msgbuf=NULL, *kludges=NULL, msgfile[100];
   UMSGID new_msg_id;
   char usenet_address[80] = "", temp[145];

   firstmemline = lastmemline = NULL;

   ClsRectWith(5,0,maxy-2,maxx-1,cfg.col.msgtext,' ');

	header = MakeHeader(areahandle, curmsg, reply, area, reply_to_id, NULL, usenet_address);

   if (!header)
       return;

   sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
   unlink(msgfile);

	if (reply)
       {
       if(usenet_address[0] != '\0')
         {
         sprintf(temp, "TO: %s", usenet_address);
         MakeQuote(curmsg, area, temp, reply);
         }
       else
		   MakeQuote(curmsg, area, add_to_top, reply);
       }
   else
       MakeTemplate(header, area, usenet_address);


   /* Now we will get the body (spawn editor), so memory could be */
   /* an issue. That's why we de-allocate the mem for the message */
   /* body of the 'curmsg'. If it's a (very) large message this   */
   /* could save us from a 'out of mem' or save swapping time     */

   if(curmsg)
      ReleaseMsg(curmsg, 0);


   if(!(header->attr & MSGFRQ) || confirm("Do you want to add a message body? (y/N)"))
   {
      sprintf(temp, "Area: %-15.15s To: %s (%s)", area->tag, header->to, header->subj);
      if(strlen(temp) > 80) /* String too long, trunc */
         {
         temp[78] = ')' ;
         temp[79] = '\0';
         }

	   if ((msgbuf = GetBody(area->type == ECHOMAIL, make_origin(custom.aka), firstmemline, temp)) == NULL)  /* Empty msg allowed for freq and attach */
         {
         if ( (header->attr & MSGFILE) || (header->attr & MSGFRQ) )
            {
            msgbuf = xmalloc(1);
            *msgbuf = '\0';
            }
         else
            {
            Message("Message aborted", 10, 0, YES);
            goto close_up;
            }
         }
   }
   else
      {
      msgbuf = xmalloc(1);
      *msgbuf = '\0';
      }

	kludges = MakeKludge(curmsg, header, area->type == NETMAIL);

   /* Check for any CC:'s in the message, write other   */
   /* msgs in that function, and the original over here */

   if(area->type == NETMAIL)
       {
       /* Check CC: first! Don't mess up zonegate etc!! */
       check_cc(area, areahandle, header, msgbuf);
       zonegate(header, kludges);
       }

   while(1)
      {
      if( MsgLock(areahandle) == -1 )
         {
         Message("Can't lock area! Wait for retry, <ESC> to abort", 10, 0, YES);
         if( xkbhit() && (get_idle_key(1) == 27) )
            goto close_up;
         }
      else break;
      }

   if((msghandle=MsgOpenMsg(areahandle, MOPEN_CREATE, 0)) == NULL)
		{
      Message("Error creating message!", -1, 0, YES);
      goto close_up;
		}

	textlen = totlen = (dword)strlen(msgbuf) + (dword) 1;
	clen = strlen(kludges) + (dword) 1;

	if (MsgWriteMsg(msghandle, 0, header, msgbuf, textlen, totlen, clen, kludges))
		{
      Message("Error writing message!", -1, 0, YES);
      MsgCloseMsg(msghandle);
      goto close_up;
		}


	if(MsgCloseMsg(msghandle) == -1)
      {
      Message("Error closing message!", -1, 0, YES);
      goto close_up;
      }

   add_tosslog(area, areahandle);

   if (reply && reply_to_id)     /* Write reply-info in msg we replied to */
      {
      if( (area->type==NETMAIL) && (cfg.usr.delreply != 0) )
         {
         if(                                        /* 1 == no confirm */
             (cfg.usr.delreply == 1) ||
             (confirm("Delete original message? (y/N)") != 0)
            )
            {
            if(MsgKillMsg(areahandle, MsgUidToMsgn(areahandle, reply_to_id, UID_EXACT)) == -1)
                Message("Can't kill message!", -1, 0, YES);
            goto close_up;
            }
         }

      new_msg_id = MsgMsgnToUid(areahandle, MsgGetHighMsg(areahandle));
      origmsg    = MsgUidToMsgn(areahandle, reply_to_id, UID_EXACT);
      WriteReplyLink(area, areahandle, origmsg, new_msg_id);
      }

   close_up:

   MsgUnlock(areahandle);

	if (msgbuf)  free(msgbuf);
	if (header)  free(header);
	if (kludges) free(kludges);

	ScanArea(area, areahandle);

}

/* ----------------------------------------------- */

void ReplyOther(MMSG *curmsg, AREA *area)

{
   AREA *toarea;
   MSG  *toareahandle;
   char temp[100];

	savescreen();
	cls();

	toarea = SelectArea(cfg.first, 1, cfg.first);

	putscreen();

   if(toarea == NULL) return;

   sprintf(temp, "-=> Note:\nReply to a message in %s.", area->tag);

   if (toarea != area)
      {
    	if(!(toareahandle=MsgOpenArea(toarea->dir, MSGAREA_CRIFNEC, toarea->base)))
         {
         Message("Error opening area", -1, 0, YES);
         return;
		   }
      get_custom_info(toarea);
      }
   else return;

   MakeMessage(curmsg, toarea, toareahandle, 1, 0, temp);

   MsgCloseArea(toareahandle);

   get_custom_info(area);

}

/* ----------------------------------------------- */

// bodytoo == -1 means attributes only

void ChangeMessage(MMSG *curmsg, AREA *area, MSG *areahandle, int bodytoo)
{
	MSGH  *msghandle;
	dword totlen, textlen;
	char  *msgbuf=NULL;   /* stays NULL when !bodytoo */
   int   editret=0, zonegated=check_gated(curmsg);
   char *newkludges, temp[145];
   MMSG *tempmsg;
   dword thisone;
   union stamp_combo combo;
   struct tm *tmdate;
   time_t time_now;


   thisone = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));

   // Only in JAM areas, re-install replynext for later write.
   if(area->base & MSGTYPE_JAM)
      {
      if(curmsg->replynext)
         curmsg->hdr.replies[1] = curmsg->replynext;
      else
         curmsg->hdr.replies[1] = 0L;
      }


   firstmemline = lastmemline = NULL;

   /* Check if the message is sent already.. */

   if( (curmsg->hdr.attr & MSGSENT) || (curmsg->hdr.attr & MSGSCANNED) )
       {
       if(confirm("Message already sent/scanned! Edit anyway? (y/N)") == 0) return;
       }

   ClsRectWith(5,0,maxy-2,maxx-1,cfg.col.msgtext,' ');

   if(bodytoo != -1)
     {
     time(&time_now);
     tmdate=localtime(&time_now);
     curmsg->hdr.date_written=curmsg->hdr.date_arrived=(TmDate_to_DosDate(tmdate,&combo))->msg_st;

     if (area->type == NETMAIL)
	     editret = EditHeader(areahandle, &curmsg->hdr, 1, custom.aka, 0, NULL, area);
     else
        editret = EditHeader(areahandle, &curmsg->hdr, 0, 0, 0, NULL, area);

     if (editret == ABORT)          /* ESC pressed */
        return;
     }
   else  /* bodytoo == -1, set attributes only */
     {
     /* Edit attribs until <ESC> or <ENTER> */
     while(((editret=SetAttributes (&curmsg->hdr)) != ESC) && (editret!=0))
         ;

     if(editret == ESC)
        return;
     }

   if(bodytoo == 1)
      {
	   MakeText(curmsg);             /* Literally write out text */

      sprintf(temp, "Area: %-15.15s To: %s (%s)", area->tag, (&curmsg->hdr)->to, (&curmsg->hdr)->subj);
      if(strlen(temp) > 80) /* String too long, trunc */
         {
         temp[78] = ')' ;
         temp[79] = '\0';
         }

      /* We can't free up mem here, might need original text if no body change.. */

	   msgbuf = GetBody(area->type == ECHOMAIL, make_origin(custom.aka), firstmemline, temp);    /* could be NULL, then no TEXT write in MsgWrite Msg below.. */

      if(msgbuf == NULL)
        {
        Message("Change aborted.", 10, 0, YES);
        return;
        }
      }

   /* We may have no body, but need to rewrite the entire message, */
   /* to allow the kludges to grow (for longer msgid addresses, or */
   /* zonegating, DIR flags etc.)                                  */
   /* If we have no raw body, get it..                             */

   if(msgbuf == NULL)
      {
      tempmsg = GetMsg(MsgUidToMsgn(areahandle, thisone, UID_EXACT), areahandle, 0, area);
      msgbuf = tempmsg->msgtext; /* Get raw text */
      tempmsg->msgtext = NULL;  /* So this won't be released.. */
      ReleaseMsg(tempmsg, 1);
      }

   if(msgbuf != NULL)
      textlen = totlen = (dword) strlen(msgbuf) + (dword) 1;
   else
      textlen = totlen = 0L;

   /* Clean out the old kludges.. */

   if(curmsg->ctxt)
     {
     RemoveFromCtrl(curmsg->ctxt, "FLAGS");
     RemoveFromCtrl(curmsg->ctxt, "MSGID");
     if(!zonegated)
        RemoveFromCtrl(curmsg->ctxt, "INTL");
//     RemoveFromCtrl(curmsg->ctxt, "PID");
     }

   /* Make us some new ones (with correct address, might have changed) */

   newkludges = MakeKludge(NULL, &curmsg->hdr, area->type == NETMAIL);

   /* Add the old one (might be REPLY kludge left), free old ones */

   if(zonegated)
      RemoveFromCtrl(newkludges, "INTL");

   RemoveFromCtrl(newkludges, "PID");

   if(curmsg->ctxt)
      {
      // Realloc, with extra space for expansion (INTL for example)
      if((newkludges=realloc(newkludges, strlen(newkludges) + strlen(curmsg->ctxt)+256)) == NULL)
          Message("Out of memory!", -1, 254, NO);
      strcat(newkludges, curmsg->ctxt);
      if(curmsg->ctxt) free(curmsg->ctxt);
      }

   curmsg->ctxt = newkludges;    /* Put them in place */

   /* Add possible FLAGS DIR and/or INTL kludges.. */

   if(area->type==NETMAIL)
     {
     /* Check CC: first! Don't mess up zonegate etc!! */
     if(curmsg->hdr.attr & MSGLOCAL)  // Only for local messages checking..
        check_cc(area, areahandle, &curmsg->hdr, msgbuf);
     zonegate(&curmsg->hdr, curmsg->ctxt);
     }

   while(1)
      {
      if( MsgLock(areahandle) == -1 )
         {
         Message("Can't lock area! Wait for retry, <ESC> to abort", 10, 0, YES);
         if( xkbhit() && (get_idle_key(1) == 27) )
            {
            if(msgbuf) free(msgbuf);
            return;
            }
         }
      else break;
      }


	if((msghandle=MsgOpenMsg(areahandle, MOPEN_CREATE, MsgUidToMsgn(areahandle, thisone, UID_EXACT))) == NULL)
	   {
      Message("Error creating message", -1, 0, YES);
      if (msgbuf) free(msgbuf);
      MsgUnlock(areahandle);
      return;
		}

   /* msgbuf could be NULL, then write original text */

	if (MsgWriteMsg(msghandle, 0, &curmsg->hdr, msgbuf, textlen, totlen, strlen(curmsg->ctxt)+1, curmsg->ctxt))
		{
      Message("Error writing Message", -1, 0, YES);
		}
   else
      {
      add_tosslog(area, areahandle);
      }

	if (msgbuf) free(msgbuf);

	MsgCloseMsg(msghandle);

   MsgUnlock(areahandle);

   ScanArea(area, areahandle);

}


/* ----------------------------------------------- */

void MakeQuote(MMSG *curmsg, AREA *area, char *add_to_top, word replytype)

{
   FILE *repfile;
   LINE *curline, *nextline, *quote, *lineptr;
   char temp[256], *para, *fbuf=NULL, inits[10], msgfile[100];
   int len;
   unsigned totlen, cursize, no_of_lines=0;

   if(!cfg.usr.internal_edit)
     {
     sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
     if (!(repfile = fopen(msgfile, "wt")))
           {
           Message("Error opening output file", -1, 0, YES);
           return;
           }

     fbuf = xmalloc(4096);
     setvbuf(repfile, fbuf, _IOFBF, 4096);
     }

   if( (curmsg->status & REPLYTO) && (area->type == NETMAIL) )
      {
      sprintf(temp, "TO: %s\n\n", curmsg->rep_name);
      if(!cfg.usr.internal_edit)
         fputs(temp, repfile);
      else
         add_text(temp);
      }

   if(add_to_top != NULL)
      {
      sprintf(temp, "%s\n\n", add_to_top);
      if(!cfg.usr.internal_edit)
         fputs(temp, repfile);
      else
         add_text(temp);
      }

   sprintf(temp, "%s\n\n", make_rephello(&curmsg->hdr, replytype));

   if(!cfg.usr.internal_edit)
      fputs(temp, repfile);
   else
      add_text(temp);

   strcpy(inits, getinitials(curmsg->hdr.from));

   curline = curmsg->txt;

   while(curline)
     {
     if(!(no_of_lines++ % 25))
       {
       if(no_of_lines > 100)
          working(maxy-1,79,7);
       }
     if (curline->status & KLUDGE)
        {
        if (cfg.usr.status & SHOWKLUDGES)
           {
           sprintf(temp, "%s> @%s\n", inits, curline->ls+1);
           if(!cfg.usr.internal_edit)
              fputs(temp, repfile);
           else
              add_text(temp);
           }
        }
     else if (curline->status & QUOTE)
        {
        sprintf(temp, "%s\n", curline->ls);
        if(!cfg.usr.internal_edit)
           fputs(temp, repfile);
        else
           add_text(temp);
        }

     else if ( (len = strlen(curline->ls)) < (maxx-20) )
        {
        if (len)
           sprintf(temp, "%s> %s\n", inits, curline->ls);
        else
           sprintf(temp, "\n");

        if(!cfg.usr.internal_edit)
           fputs(temp, repfile);
        else
           add_text(temp);
        }

     else       /* Build a paragraph of text, so we can wrap it for quoting */
        {
        cursize=4000;
        para = xmalloc(cursize);

        totlen=strlen(curline->ls);             /* start with first line.. */
        memcpy(para, curline->ls, totlen);
        *(para+totlen) = ' ';
        totlen++;

        while(1)                                /* and append until end of paragraph */
           {
           nextline=curline->next;

           if ((nextline == NULL) ||
               (nextline->status & QUOTE) ||
               (nextline->status & KLUDGE) ||
               (nextline->ls[0] == '\0'))
               {
               *(para+totlen) = '\0';
               break;
               }
           else         /* We have to add it.. */
              {
              len = strlen(nextline->ls);

              if ( (totlen + len + 100) > cursize)    /* need more mem? */
                 {
                 cursize += 1000;
                 if ( (para = realloc(para, cursize)) == NULL)
                    {
                    Message("Out of memory", -1, 254, NO);
                    }
                 }

              memcpy(para+totlen, nextline->ls, len);    /* append it */
              curline = nextline;                        /* got next line, nextline == NULL was catched in beginning */

              if ( len < (maxx-20) )      /* short line, end of paragraph */
                 {
                 *(para+totlen+len) = '\0';
                 break;                      /* End of adding to paragraph text */
                 }
              else                           /* set up for more adding */
                 {
                 *(para+totlen+len) = ' ';
                 totlen = totlen+len+1;
                 }
              }
           }

           lineptr = quote = FormatText(para, 60);

           while(lineptr)
              {
              if (strlen(lineptr->ls))
                 sprintf(temp, "%s> %s\n", inits, lineptr->ls);
              else
                 sprintf(temp, "\n");

              if(!cfg.usr.internal_edit)
                 fputs(temp, repfile);
              else
                 add_text(temp);
              lineptr = lineptr->next;
              }

              free(para);

              while(quote)
                 {
                 lineptr = quote;
                 quote = quote->next;
                 free(lineptr);
                 }
        }

        if (curline)
           curline = curline->next;
     }

   sprintf(temp, "\n%s",  expand(custom.signoff, &curmsg->hdr));
   if(!cfg.usr.internal_edit)
      fputs(temp, repfile);
   else
      add_text(temp);

   if (area->type == ECHOMAIL)
      {
      if(!cfg.usr.internal_edit)
         fputs(make_origin(custom.aka), repfile);
      else
         add_text(make_origin(custom.aka));
      }
   else if(area->type == LOCAL)
      {
      /* Nothing */
      }
   else if(cfg.usr.status & NETTEAR)
      {
      if(cfg.usr.emptytear == 0)
         {
          #ifndef __OS2__
              if(!cfg.usr.internal_edit)
                 fprintf(repfile, "\n--- timEd " VERSION "%s", (REGISTERED) ? "+" : "");
              else
                 {
                 sprintf(temp, "\n--- timEd " VERSION "%s", (REGISTERED) ? "+" : "");
                 add_text(temp);
                 }
          #else
              if(!cfg.usr.internal_edit)
                 fprintf(repfile, "\n--- timEd/2 " VERSION "%s", (REGISTERED) ? "+" : "");
              else
                 {
                 sprintf(temp, "\n--- timEd/2 " VERSION "%s", (REGISTERED) ? "+" : "");
                 add_text(temp);
                 }
          #endif
         }
      else
        {
        if(!cfg.usr.internal_edit)
           fputs("\n---", repfile);
        else
           add_text("\n---");
        }
      }

   if(!cfg.usr.internal_edit)
      {
      fclose(repfile);
      free(fbuf);
      }

}


/* ----------------------------------------------- */


#ifdef __NEVER__
int IsQuote(char *line)

{
	char *ptr1, *ptr2;
   int len = min(7, strlen(line));

	if ( (ptr1 = memchr(line, '>', len)) == NULL)
      return 0;

	ptr2 = memchr(line, '<', len-1);

	if ( ptr1 != NULL && (ptr2 == NULL || ptr2 > ptr1) )
		return 1;

	else return 0;

}

#endif

#ifndef  __OS2__

int pascal IsQuote(char *line)
{
   asm push ds

   asm lds si,line    /* ds:si points to line      */
   asm mov cx,7       /* No more than 7 characters */
   asm cld            /* We move forward           */

again:
   asm lodsb         /* Get a char in al           */
   asm or al,al      /* Is it a 0? End of string?  */
   asm jz nogo       /* Yes, return 0              */
   asm cmp al,60     /* Is it a '<' ?              */
   asm je nogo       /* Yes, return 0              */
   asm cmp al,62     /* Is it '>' ?                */
   asm loopne again  /* no, Load next char         */
   asm jne nogo      /* 7 chars scanned, return 0  */
   asm mov ax,1      /* Lets return 1              */
   asm jmp getout    /* And get out                */

nogo:
   asm xor ax,ax     /* Return 0                   */

getout:
   asm pop ds
}

#endif

/* ----------------------------------------------- */


/* char *MakeKludge(MMSG *curmsg, NETADDR *addr) */

char *MakeKludge(MMSG *curmsg, XMSG *header, int netmail)
{
/*	static char counter=0; */
	static time_t	t;
	char *buffer, temp[80];

   if(!t)
      {
	   (void)time(&t);
	   t = (t<<4);
      }

	buffer = xcalloc(1, 256);
	sprintf(buffer, "\01MSGID: %i:%i/%i.%i %lx", header->orig.zone, header->orig.net, header->orig.node, header->orig.point, t++);

   if ( curmsg && (strcmpi(curmsg->id, "") != 0) )
      {
      sprintf(temp, "\01REPLY: %s", curmsg->id);
      strcat(buffer, temp);
      }

   if(cfg.usr.emptytear)
      {
      #ifndef __OS2__
      strcat(buffer, "\01PID: timEd " VERSION "");
      #else
      strcat(buffer, "\01PID: timEd/2 " VERSION "");
      #endif
      if(REGISTERED) strcat(buffer, "+");
      }

   if(netmail && (cfg.usr.status & INTLFORCE))
      {
      sprintf(temp, "\01INTL %d:%d/%d %d:%d/%d",
                    header->dest.zone,
                    header->dest.net,
                    header->dest.node,
                    header->orig.zone,
                    header->orig.net,
                    header->orig.node);
      strcat(buffer, temp);
      }

	return buffer;

}


/* ----------------------------------------------- */


void MakeText(MMSG *curmsg)

{

	FILE *repfile;
	LINE *curline;
	char temp[256], *fbuf=NULL, msgfile[100];


   if(!cfg.usr.internal_edit)
     {
     sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
     if (!(repfile = fopen(msgfile, "wt")))
			{
         Message("Error opening output file!", -1, 0, YES);
			return;
			}

      fbuf = xmalloc(4096);
      setvbuf(repfile, fbuf, _IOFBF, 4096);
      }

	for(curline = curmsg->txt; curline; curline = curline->next)
		{
      if(curline->status & KLUDGE) continue;   /* Don't put your own kludges in! */

		sprintf(temp, "%s\n", curline->ls);
      if(!cfg.usr.internal_edit)
		   fputs(temp, repfile);
      else
         {
         if(!(curline->status & HCR))
            temp[strlen(temp)-1] = '\0';
         add_text(temp);
         }
		}

   if(!cfg.usr.internal_edit)
     {
     fclose(repfile);
     free(fbuf);
     }

}


/* ------------------------------------------------------------------------- */
/* -- Expand address string using 'defaults' for your AKA, 0 if aka == -1 -- */
/* ------------------------------------------------------------------------- */

void address_expand(char *line, NETADDR *fa, int aka)

{
   char *c;

   if ((c = strchr(line, ':')) != NULL) /* Do we have a zone number? */
      {
      fa->zone = atoi(line);
      line = c+1;
      }
   else if(aka != -1)               /* No, default to our own zone.. */
      {
      fa->zone = cfg.usr.address[aka].zone;
      }

   if ((c = strchr(line, '/')) != NULL) /* Do we have a net number? */
      {
      fa->net = atoi(line);
      line = c+1;
      }
   else if(aka != -1)
      {
      fa->net = cfg.usr.address[aka].net;
      }

   if ((c = strchr(line, '.')) != NULL) /* Do we have a pointnumber? */
      {
      if (c == line)          /* String is like:  ".8" */
         {
         if (aka != -1) fa->node = cfg.usr.address[aka].node;
         fa->point = atoi(++c);
         }
      else                    /* String is like:  "527.8"  */
         {
         fa->node  = atoi(line);
         fa->point = atoi(++c);
         }
      }
   else                       /* There's no point(number) */
     {
     fa->node = atoi(line);
     fa->point = 0;
     }

}

/* ------------------------------------------------- */
/* -- Get someone's initials for quoting ( GvE> ) -- */
/* ------------------------------------------------- */


char *getinitials(char *name)

{
   static char inits[20];
   char *charptr, *result, *tempptr;

   memset(inits, '\0', sizeof(inits));
   charptr = name;
   result = inits;

   if(cfg.usr.internal_edit)  // We can add a leading space for int. edit
     *result++ = ' ';         // Not ext. edit, editors may align to it :-(

   *result++ = *name;

   if( (tempptr=strchr(charptr, '@')) == NULL)  // Usenet name?
     {
     while(*charptr)
        {
        if (*charptr == ' ' || *charptr == '.' || *charptr == '_')
           {
           if(! ( (*charptr=='.') && (*(charptr+1)==' ') ) )
              *result++ = *++charptr;
           }
        if(charptr)
           charptr++;
        }
     }
   else
     {
     *result++ = *(++tempptr);
     }

   *result = '\0';

   return inits;

}


/* ------------------------------------------------- */

void MakeTemplate(XMSG *header, AREA *area, char *usenet_address)
{
 	FILE *repfile;
	char temp[256], msgfile[100];

   if(!cfg.usr.internal_edit)
     {
     sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
	  if (!(repfile = fopen(msgfile, "wt")))
			  {
           Message("Error opening output file!", -1, 0, YES);
			  return;
			  }
     }

   /* Do we need to write a Usenet address at the top? */

   if(usenet_address[0] != '\0')
     {
     sprintf(temp, "TO: %s\n\n", usenet_address);
     if(!cfg.usr.internal_edit)
        fputs(temp, repfile);
     else
        add_text(temp);
     }

   sprintf(temp, "%s\n\n", make_hello(header));
   if(!cfg.usr.internal_edit)
      fputs(temp, repfile);
   else
      add_text(temp);

   sprintf(temp, "\n\n%s", expand(custom.signoff, header));
   if(!cfg.usr.internal_edit)
      fputs(temp, repfile);
   else
      add_text(temp);

   if (area->type == ECHOMAIL)
      {
      if(!cfg.usr.internal_edit)
         fputs(make_origin(custom.aka), repfile);
      else
         add_text(make_origin(custom.aka));
      }
   else if(area->type == LOCAL)
     {
     /* Nothing */
     }
   else if(cfg.usr.status & NETTEAR)
      {
      if(cfg.usr.emptytear == 0)
         {
          #ifndef __OS2__
          if(!cfg.usr.internal_edit)
             fprintf(repfile, "\n--- timEd " VERSION "%s\n", (REGISTERED) ? "+" : "");
          else
            {
            sprintf(temp, "\n--- timEd " VERSION "%s\n", (REGISTERED) ? "+" : "");
            add_text(temp);
            }
          #else
          if(!cfg.usr.internal_edit)
             fprintf(repfile, "\n--- timEd/2 " VERSION "%s\n", (REGISTERED) ? "+" : "");
          else
             {
             sprintf(temp, "\n--- timEd/2 " VERSION "%s\n", (REGISTERED) ? "+" : "");
             add_text(temp);
             }
          #endif
          }
      else
          {
          if(!cfg.usr.internal_edit)
             fputs("\n---\n", repfile);
          else
             add_text("\n---\n");
          }
      }

   if(!cfg.usr.internal_edit)
      fclose(repfile);

}


/* ---------------------------------------------------------- */


char * make_origin(int aka)
{
   static char temp[256];
   char address[40];

   if (cfg.usr.address[aka].point != 0)
      sprintf(address, "(%d:%d/%d.%d)", (int) cfg.usr.address[aka].zone,
                                        (int) cfg.usr.address[aka].net,
                                        (int) cfg.usr.address[aka].node,
                                        (int) cfg.usr.address[aka].point);
   else
      sprintf(address, "(%d:%d/%d)", cfg.usr.address[aka].zone,
                                     cfg.usr.address[aka].net,
                                     cfg.usr.address[aka].node);

   custom.origin[67 - strlen(address)] = '\0';

   if(cfg.usr.emptytear == 0)
      {
       #ifndef __OS2__
       sprintf(temp, "\n--- timEd " VERSION "%s\n * Origin: %s %s\n",
                                                (REGISTERED) ? "+" : "",
                                                custom.origin, address);
       #else
       sprintf(temp, "\n--- timEd/2 " VERSION "%s\n * Origin: %s %s\n",
                                                (REGISTERED) ? "+" : "",
                                                custom.origin, address);
       #endif
       }
   else
       sprintf(temp, "\n---\n * Origin: %s %s\n",
                                                custom.origin, address);

   return temp;
}

/* ------------------------------------------------------- */

char * make_rephello(XMSG *header, word replytype)
{
   static char hello[300];


   if(replytype == 2)
       strcpy(hello, expand(custom.followhello, header));
   else
       strcpy(hello, expand(custom.rephello, header));

   return hello;

}


/* ----------------------------------------------------- */

char * make_hello(XMSG *header)
{
   static char hello[300];


   strcpy(hello, expand(custom.hello, header));

   return hello;

}


/* ----------------------------------------------------------- */
/* - Expand an 'hellostring', so fill in %to and %from names - */
/* ----------------------------------------------------------- */


char *expand(char *line, XMSG *hdr)
{
   static char temp[300];
   char *lineptr=line, *tempptr=temp;
   char tempaddress[50];
   struct _stamp t = hdr->date_written;

   memset(temp, '\0', sizeof(temp));

   if( (t.date.mo < 1) || (t.date.mo > 12))
        t.date.mo = 1;

   while(*lineptr)
     {
     if(*lineptr == '%')
        {
        lineptr++;
        if(strncmpi(lineptr, "to", 2)==0)    /* %to */
           {
           memcpy(tempptr, hdr->to, strlen(hdr->to));
           lineptr += 2;
           tempptr += strlen(hdr->to);
           }

        else if(strncmpi(lineptr, "fto", 3)==0)   /* %fto */
           {
           /* Only copy till first space */
           if(strchr(hdr->to, '@') == NULL)
             {
             memcpy(tempptr, hdr->to, strcspn(hdr->to, " "));
             tempptr += strcspn(hdr->to, " ");
             }
           else
             {
             memcpy(tempptr, hdr->to, strcspn(hdr->to, "@"));
             tempptr += strcspn(hdr->to, "@");
             }
           lineptr += 3;
           }

        else if(strncmpi(lineptr, "from", 4)==0)      /* %from */
           {
           memcpy(tempptr, hdr->from, strlen(hdr->from));
           lineptr += 4;
           tempptr += strlen(hdr->from);
           }

        else if(strncmpi(lineptr, "ffrom", 5)==0)      /* %ffrom */
           {
           /* Only copy till first space.. */
           if(strchr(hdr->from, '@') == NULL)
             {
             memcpy(tempptr, hdr->from, strcspn(hdr->from, " "));
             tempptr += strcspn(hdr->from, " ");
             }
           else
             {
             memcpy(tempptr, hdr->from, strcspn(hdr->from, "@"));
             tempptr += strcspn(hdr->from, "@");
             }
           lineptr += 5;
           }

        else if(strncmpi(lineptr, "subj", 4)==0)    /* %subj */
           {
           memcpy(tempptr, hdr->subj, strlen(hdr->subj));
           lineptr += 4;
           tempptr += strlen(hdr->subj);
           }

        else if(strncmpi(lineptr, "orig", 4)==0)    /* %orig */
           {
           sprintf(tempaddress, "%d:%d/%d.%d", hdr->orig.zone,
                                               hdr->orig.net,
                                               hdr->orig.node,
                                               hdr->orig.point);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "dest", 4)==0)    /* %dest */
           {
           sprintf(tempaddress, "%d:%d/%d.%d", hdr->dest.zone,
                                               hdr->dest.net,
                                               hdr->dest.node,
                                               hdr->dest.point);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "time", 4)==0)    /* %time */
           {
           sprintf(tempaddress, "%2.2i:%2.2i", t.time.hh, t.time.mm);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "year", 4)==0)    /* %year */
           {
           sprintf(tempaddress, "19%2.2i", t.date.yr+80);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "mon", 3)==0)    /* %mon */
           {
           sprintf(tempaddress, "%s", mtext[t.date.mo-1]);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 3;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "day", 3)==0)    /* %day */
           {
           sprintf(tempaddress, "%2.2i", t.date.da);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 3;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "dow", 3)==0)    /* %dow */
           {
           sprintf(tempaddress, "%s", days[dow(t.date.yr+1980, t.date.mo, t.date.da)]);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 3;
           tempptr += strlen(tempaddress);
           }

        }
     else if(*lineptr == '\\' && *(lineptr+1) == 'n')
        {
        *tempptr++ = '\n';
        lineptr += 2;
        }
     else *tempptr++ = *lineptr++;
     }

   return temp;
}


/* Walk through the list of aliases (macro's...) and check if    */
/* there is a match. If there is, fill header, and return number */
/* of items filled in (that can be skipped in editheader..)      */


int check_alias(XMSG *hdr, char *usenet_address)
{
  MACROLIST *thismacro=cfg.firstmacro;

  while(thismacro)
    {
    if(!strcmpi(thismacro->macro, hdr->to))
       {
       if( (thismacro->usenet[0] != '\0') && (usenet_address!=NULL) )
          strcpy(usenet_address, thismacro->usenet);

       strcpy(hdr->to, thismacro->toname);
       if(thismacro->toaddress.zone == 0) return 0;
       hdr->dest = thismacro->toaddress;
       if(thismacro->subject[0] == '\0') return 1;
       strcpy(hdr->subj, thismacro->subject);
       return 2;
       }
    thismacro = thismacro->next;
    }

  return 0;    /* Not found.. */
}


/*
 * -------------------------------------------------------------------
 * Check if a message needs to be zonegated (only if user set it up).
 * Add INTL kludge & rewrite header, don't gate freqs etc
 * -------------------------------------------------------------------
*/

void zonegate(XMSG *header, char *kludges)
{
   char temp[100];

   if((header == NULL) || (kludges == NULL))
      return;

   if(cfg.usr.zonegate == 0) return;   /* User doesn't want zonegating */

   if(
       (header->attr & MSGCRASH) ||
       (header->attr & MSGHOLD)  ||
       (header->attr & MSGFRQ)   ||
       (header->attr & MSGFILE)  ||
       (header->attr & ADD_DIR)  ||
       (header->attr & ADD_IMM)
     ) return;                 /* Don't gate crash or hold or frq/attach */

   if(header->orig.zone != header->dest.zone)  /* Inter zone msg! */
      {
      if(
          (cfg.usr.zonegate == 1) ||
          (confirm("Send message through zonegate? (y/N)") != 0)
        )
         {
         RemoveFromCtrl(kludges, "INTL");
         sprintf(temp, "\01INTL %d:%d/%d %d:%d/%d",
                    header->dest.zone,
                    header->dest.net,
                    header->dest.node,
                    header->orig.zone,
                    header->orig.net,
                    header->orig.node);

         header->dest.node = header->dest.zone;
         header->dest.zone = header->orig.zone;
         header->dest.net  = header->orig.zone;
         strcat(kludges, temp);
         }
      }

}


/* Check a message to see if it is a zonegated message             */
/* Now used to see if we can remove INTL kludge for change message */
/* function                                                        */

int check_gated(MMSG *msg)
{
   char *intl;
   int destzone = msg->hdr.orig.zone;


   if(msg->hdr.orig.zone != msg->hdr.dest.zone)
      return 0;

   if( (intl=stristr(msg->ctxt, "INTL")) == NULL)
      return 0;

   sscanf(intl, "INTL %hd:", &destzone);

   if(destzone != msg->hdr.orig.zone)
      return 1;

   return 0;

}



/* Add text to the lines in memory, for the internal editor */


void add_text(char *line)
{
   LINE *thisline, *lineptr, *lastline, *oldlast;
   char *dupline,
        *charptr;
   int quoted=0;
   char quote[20], temp[140];

   charptr = dupline = xstrdup(line);

   while(*charptr != '\0')
      {
      if(*charptr == '\n')
         *charptr = '\r';
      charptr++;
      }

   /* Now, if the previous line did NOT end in a HCR, we need to
      append this text to it, and rewrap the whole thing.. */

   if(firstmemline)   /* Only if we actually have previous lines */
     {
     if(!(lastmemline->status & HCR))  /* No HCR? Then append */
       {
       charptr = xcalloc(1, strlen(dupline) + lastmemline->len + 2);  // + 2 for trailing zero and possible added space */
       strcpy(charptr, lastmemline->ls); // start with previous line..

       /* Check if we need to add a space */
       if( (charptr[strlen(charptr)-1] != ' ') &&
           (dupline[0] != ' ') )
           strcat(charptr, " ");

       strcat(charptr, dupline); // add new part..
       free(dupline);
       dupline = charptr;

       if(firstmemline == lastmemline)  /* Are we changing/rewrapping the first line? */
          firstmemline = NULL;
       oldlast = lastmemline; // Save value, we need to free() it later
       if(lastmemline->prev)
          {
          lastmemline->prev->next = NULL;   // Destroy forward link
          lastmemline = lastmemline->prev;  // Set back last line to existing line
          }

       if(oldlast->ls)
          free(oldlast->ls);
       free(oldlast);
       }
     }

   lastline = lineptr = thisline = FormatText(dupline, maxx-2);  /* Destroys 'line' !!!! */

   if( (lineptr->status & QUOTE) && (lineptr->next) )
     {
     quoted=1;
     lineptr->status |= HCR;
     get_quote_string(lineptr->ls, quote);
     }

   /* So we copy the lines... */
   while(lineptr)
     {
     if(quoted && (!(lineptr->status & QUOTE)) && (lineptr->ls[0] != '\0') )
        {
        sprintf(temp, "%s%s", quote, lineptr->ls);
        lineptr->ls = xstrdup(temp);
        lineptr->len = strlen(temp);
        lineptr->status |= (HCR|QUOTE);
        }
     else
        {
        lineptr->ls = xstrdup(lineptr->ls);
        lineptr->len = strlen(lineptr->ls);
        }
     lastline = lineptr;
     lineptr = lineptr->next;
     }


   /* Remove empty last line.. */
   if(lastline && (lastline->ls[0] == '\0') && (!(lastline->status & HCR)))
     {
     if(lastline->prev)
       {
       lastline->prev->next=NULL;
       if(lastline->ls)
          free(lastline->ls);
       free(lastline);
       }
     }

   if(!firstmemline)
      firstmemline = thisline;
   else
      {
      lastmemline->next = thisline;
      thisline->prev = lastmemline;
      }

   /* Find new end of textlines */

   lastmemline = thisline;
   while(lastmemline->next != NULL)
     lastmemline = lastmemline->next;

   free(dupline);
}



/* -------------------------------------------------------- */
/* - Write a (forward) reply link to the original message - */
/* -       Returns 0 if all is well, -1 on error          - */
/* -------------------------------------------------------- */


int WriteReplyLink(AREA *area, MSG *areahandle, dword orig, dword new)
{
   XMSG replyheader;
   int n;
   MSGH *msghandle;


  if(!(area->base & MSGTYPE_JAM))
    {
    if((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, orig)) == NULL)
       return -1;

    if((MsgReadMsg(msghandle, &replyheader, 0, 0, NULL, 0, NULL)) != -1)
       {
       for (n=0; n<10; n++)
         {
         if(replyheader.replies[n] == 0)    /* Look for a free 'slot' */
           {
           replyheader.replies[n] = new;
           break;
           }
         }
       MsgWriteMsg(msghandle, 0, &replyheader, NULL, 0, 0, 0, NULL);
       }

    MsgCloseMsg(msghandle);

    return 0;
    }

  // JAM, fix up strange reply stuff now..

  if((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, orig)) == NULL)
     return -1;

  if((MsgReadMsg(msghandle, &replyheader, 0, 0, NULL, 0, NULL)) == -1)
     {
     MsgCloseMsg(msghandle);
     return -1;
     }

  if(replyheader.replies[0] == 0)      // Replyfirst?
     {
     replyheader.replies[0] = new;
     MsgWriteMsg(msghandle, 0, &replyheader, NULL, 0, 0, 0, NULL);
     MsgCloseMsg(msghandle);
     return 0;
     }

  MsgCloseMsg(msghandle);

  // If we get here, if we have to search replynext fields for an empty space

  /* First init with start value: the first reply: */
  replyheader.replies[1] = replyheader.replies[0];

  while(replyheader.replies[1] != 0) // Loop until space found..
    {
    if((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, replyheader.replies[1])) == NULL)
       return -1;

    if((MsgReadMsg(msghandle, &replyheader, 0, 0, NULL, 0, NULL)) == -1)
       {
       MsgCloseMsg(msghandle);
       return -1;
       }

    if(replyheader.replies[1] != 0)    // Replynext?
       MsgCloseMsg(msghandle);
    }

  replyheader.replies[1] = new;
  MsgWriteMsg(msghandle, 0, &replyheader, NULL, 0, 0, 0, NULL);
  MsgCloseMsg(msghandle);

  return 0;

}
