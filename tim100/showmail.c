#include <video.h>
#include <scrnutil.h>
#include <msgapi.h>
#include <conio.h>
#include <stdio.h>
#include <alloc.h>
#include <video.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#include "wrap.h"
#include "xmalloc.h"
#include "tstruct.h"
#include "input.h"
#include "showmail.h"
#include "reply.h"
#include "global.h"
#include "showhdr.h"
#include "idlekey.h"
#include "shel2dos.h"
#include "choose.h"
#include "pickone.h"
#include "message.h"
#include "help.h"
#include "request.h"
#include "readarea.h"
#include "showinfo.h"
#include "print.h"
#include "copymsg.h"
#include "unrec.h"

int  getnumber (char c, MSG *areahandle);
int  PickReply (MMSG *curmsg, MSG *areahandle);


void pagedown (void);
void pageup   (void);

/* These two used for msg display. Global to be used by freq */
/* function, together with the two functions above           */

static numlines,
        curline;


typedef struct
{
   AREA   *area;
   UMSGID number;

} MARK;

MARK mark;


int ShowMsg(MMSG *msg, AREA *area, MSG *areahandle, int displaytype)
{
	char	temp[133];
	int	command;


   paint_header(&msg->hdr, (area->type == NETMAIL), (msg->status & PERSONAL));

   sprintf(temp, "%-0.50s", MakeRep(msg, areahandle));
   print(0,26,cfg.col.msglinks,temp);

#ifndef __OS2__
	sprintf(temp, "Area: %-27.27s      100%%  Current: %5ld, high: %5ld ³ %3dK ", area->desc, MsgGetCurMsg(areahandle), area->highest, ((unsigned long)coreleft())/1024);
#else
	sprintf(temp, "Area: %-27.27s      100%%         Current: %5ld, high: %5ld ", area->desc, MsgGetCurMsg(areahandle), area->highest);
#endif

   if(maxx > 80)
           {
           memset(temp+80, ' ',maxx-80);
           temp[maxx]='\0';
           }

	print(maxy-1,0,cfg.col.msgbar,temp);

	command=showbody(msg, areahandle, area, displaytype);

	return command;
}



int showbody(MMSG *curmsg, MSG *areahandle, AREA *area, int displaytype)

{
	int l, ret=0;
	LINE *top, *last, *lptr;
	char	temp[133];
   int c, linecolor, retval, key;
   LINE *first = curmsg->txt;
   MSGH *msghandle, *tempmsg;
   long realno;
   unsigned long a, b;
   unsigned int progress;
   dword curno;
   BOX *endarea;
   UMSGID this;


#define NOK !(cfg.usr.status & SHOWKLUDGES)

   start:

   numlines = 0;
   curline = 0;
	top=first;
	lptr=top;

	/* Count the number of lines the message exists of */

	for(last=first; last != NULL; last=last->next)
         {
         if(!(NOK && (last->status & KLUDGE)))
			   numlines++;
         }


	if (numlines < maxy-6)  /* if msg is shorter than screen, clear lower part of screen */
		{
      ClsRectWith(5+numlines,0,maxy-2,maxx-1,cfg.col.msgtext,' ');
		}


	while(1)		/* Exit is w/ return statements that gives command back */
		{
		lptr=first;

		/* search first line to display */

      if (NOK)
         while(lptr && (lptr->status & KLUDGE))
            lptr = lptr->next;

		for(l=0; l<curline; l++)
         {
         do
            {
			   lptr=lptr->next;
            }
         while(NOK && (lptr->status & KLUDGE) );
         }

		/* display them.. */

      if(numlines==0)
         progress=100;
      else
        {
        a = 100L*((unsigned long)(curline+maxy-6));
        b = (unsigned long) numlines;
        progress = (int) (a / b);
        if(progress > 100)
           progress = 100;
        }

      sprintf(temp, "%3u%%", progress, a, b, curline+maxy-6);
	   print(maxy-1,39,cfg.col.msgbar,temp);

      MoveXY(1,6);

		for(l=0; (l<maxy-6) && lptr; l++, lptr=lptr->next)
			{
         while(NOK && (lptr->status & KLUDGE))
            {
            lptr = lptr->next;
            if(!lptr) break;     /* end of msg reached */
            }
         if(!lptr) break;     /* end of msg reached */


         if (lptr->status & NORMTXT)
            linecolor = cfg.col.msgtext;
         else if(lptr->status & QUOTE)
            linecolor = cfg.col.msgquote;
         else if(lptr->status & KLUDGE)
            linecolor = cfg.col.msgkludge;
         else if(lptr->status & ORIGIN || lptr->status & TEAR)
            linecolor = cfg.col.msgorigin;
         else if(lptr->status & HIGHLIGHT)
            linecolor = cfg.col.msgspecial;

         if(!(lptr->status & WITHTAB))
           printeol(5+l,0,linecolor,lptr->ls);
         else tabprint(5+l,0,linecolor,lptr->ls,3);
			}

		/* Now check for commands... */

		switch (c = get_idle_key(1))
			{
         case 9:      /* TAB */
           if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
                break;

           if(mark.area == area)
             {
             if((realno=MsgUidToMsgn(areahandle, mark.number, UID_EXACT)) != 0)
                return realno;
             }
           break;

         case 10:     /* CTRL <ENTER> */

           mark.area = area;
           mark.number = curmsg->uid;
           break;

         case 291:    /* ALT - H */
         case 104:    /* h */
         case  72:    /* H */

           if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
                break;

           return EDITHELLO;

         case 301:    /* ALT - X */

            if(displaytype == NORMAL_DISPLAY)
               {
               if(cfg.usr.status & CONFIRMEXIT)
                  {
                  if(confirm("Do you really want to exit timEd? (y/N)"))
                      return EXIT;
                  }
               else return EXIT;
               }
            break;

			case 333:		/* right   */
         case 13:       /* <enter> */

            if(displaytype == SCAN_DISPLAY)  /* No check for scanning functions */
               return NEXT;

            if (MsgGetCurMsg(areahandle) < MsgGetHighMsg(areahandle) )
						   return NEXT;
				else
               {
               if(cfg.usr.status & ENDAREAMENU)
                  {
                  endarea = initbox(11, 12, 17, 68, cfg.col.popframe, cfg.col.poptext, SINGLE, YES, ' ');
                  drawbox(endarea);
                  boxwrite(endarea,0,1,"           End of messages in this area!");
                  boxwrite(endarea,2,1,"<right>, <enter> or '+' : next area with new mail");
                  boxwrite(endarea,3,1,"<ESC>                   : exit area");
                  boxwrite(endarea,4,1,"Other keys              : remove this text");

                  kbflush();

                  key = get_idle_key(1);

                  delbox(endarea);

                  switch(key)
                     {
                     case 333:     /* right cursor */
                     case 13 :     /* <enter>      */
                     case 43 :     /* '+'          */
                        return NEXTAREA;

                     case 27:       /* <ESC> */
                        return ESC;

                     default:
                        break;
                     }
                  }
               else
                  {
                  beep();
                  kbflush();
                  }
               }

            break;

			case 331:		/* left */
				return PREV;

         case 371:   /* ctrl - left */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            realno = MsgUidToMsgn(areahandle, curmsg->hdr.replyto, UID_EXACT);

            if ( (realno != 0) &&
                 ((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL) )
               {
               MsgCloseMsg(msghandle);
               return (int) realno;
               }
            else break;

         case 413:   /* ALT - right              */
         case 334:   /* ALT - grey plus          */
         case 387:   /* ALT - +, or better ALT-= */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            realno = MsgUidToMsgn(areahandle, curmsg->replynext, UID_EXACT);

            if ( (realno != 0) &&
                 ((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL) )
               {
               MsgCloseMsg(msghandle);
               return (int) realno;
               }
            else break;

         case 372:   /* ctrl - right */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            ret = 0;
            if(curmsg->hdr.replies[0] != 0)
               ret = PickReply(curmsg, areahandle);
            if (ret)
               return ret;
            else
               break;

			case 328:		/* up */
				if (curline)
						curline--;
				break;

			case 336:		/* down */
				if ( (curline < numlines-(maxy-6)) && (numlines > (maxy-6)) )
						curline++;
				break;

			case 327:		/* home */
				curline=0;
				break;

			case 335:		/* end */
            if (numlines > (maxy-6))
					curline=numlines-(maxy-6);
				break;

			case 329:		/* page up */

            pageup();
				break;

			case 337:		/* page down */


            pagedown();
            break;

         case 32:      /* Space */
            if ( (numlines > (maxy-6)) && (curline < numlines-(maxy-6)) )
               stuffkey(337);
            else
               stuffkey(333);

            break;

         case 273:    /* ALT - W */
         case 119:    /* w */
         case  87:

            PrintMessage(curmsg, area, 0);  /* Last element: no hardcopy */
            break;

         case 281:  /* ALT-P */
         case 112:
         case 80:

            PrintMessage(curmsg, area, 1);  /* Last element: a hardcopy */
            break;

			case 274:		/* ALT - E */
         case 101:      /* E */
         case 69:
         case 338:      /* <ins> */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

				return ENTER;

			case 275:		/* ALT - R */
         case 114:      /* R       */
         case 82:
         case 272:      /* ALT-Q   */
         case 113:      /* Q       */
         case 81:

            curno = anchor(DOWN, areahandle);
				MakeMessage(curmsg, area, areahandle, 1, curno, NULL);

				return((int) anchor(UP, areahandle));

         case 278:     /* ALT-U */
         case 117:     /* U     */
         case 85:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return MSGMAINT;

         case 276:     /* ALT - T */
         case 116:     /* T */
         case 84:

            curno = anchor(DOWN, areahandle);
				MakeMessage(curmsg, area, areahandle, 3, curno, NULL);

				return ((int) anchor(UP, areahandle));


         case 279:   /* ALT - I */
         case 105:   /* I */
         case 73:

            showinfo(curmsg, area, areahandle);
            break;

         case 280:    /* ALT - O */
         case 111:    /* O */
         case 79:

            curno = anchor(DOWN, areahandle);
				MakeMessage(curmsg, area, areahandle, 2, curno, NULL);

				return ((int) anchor(UP, areahandle));

         case 286:    /* ALT - A */
         case 97:     /* A       */
         case 65:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return ESC;

         case 292:    /* ALT - J */
         case 106:    /* J */
         case 74:

            shell_to_DOS();
            break;

         case 288:    /* ALT D */
         case 100:    /* D */
         case 68:
         case 339:    /* <del> */

           anchor(DOWN, areahandle);

           if(
              (!(cfg.usr.status & CONFIRMDELETE)) ||
              (confirm("Really delete this work of art? (y/N)"))
             )
				  MsgKillMsg(areahandle, MsgGetCurMsg(areahandle));

			  curno = anchor(UP, areahandle);

           ScanArea(area, areahandle);

           if(area->nomsgs == 0)
             {
             if(displaytype != SCAN_DISPLAY) // Don't return ESC in mailscan! It will abort it!
                return ESC;
             else
                return NEXT;
             }

           else return (int) curno;

         case 289:    /* ALT - F */
         case 102:    /* F */
         case 70:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return FIND;

			case 293:		/* ALT - K */
         case 107:      /* K */
         case 75:
         case 303:      /* ALT - V */
         case 118:      /* V */
         case 86:

            cfg.usr.status ^= SHOWKLUDGES;
            goto start;

         case 294:    /* ALT - L */
         case 108:    /* L */
         case 76:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return LIST;

         case 98:       /* B */
         case 304:      /* ALT-B */
         case 66:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return BROADLIST;

			case 302:		/* ALT - C */
         case 99:       /* C */
         case 67:

				ChangeMessage(curmsg, area, areahandle, 1);
				return 0;   /* Reread, may have dumped from mem */

         case 8:        /* CTRL - H */

				ChangeMessage(curmsg, area, areahandle, 0);
            return 0;

         case 287:     /* ALT - S */
         case 115:
         case 83:

             ChangeMessage(curmsg, area, areahandle, -1);
             return 0;

         case 21:     /* CTRL - U */

            unreceive(curmsg, areahandle, area);
            break;

         case 305:    /* ALT - N */
         case 110:    /* N */
         case 78:

            ReplyOther(curmsg, area);
            return 0;

			case 306:		/* ALT - M */
         case 109:      /* M */
         case 77:

            anchor(DOWN, areahandle);
				MoveMessage(areahandle, area, curmsg);
				curno = anchor(UP, areahandle);

            ScanArea(area, areahandle);
				if (area->nomsgs == 0)
                {
                if(displaytype == SCAN_DISPLAY)  // Don't return ESC in a mailscan!
                    return NEXT;
                else
						  return ESC;
                }
            else return (int) curno;

         case 315:    /* F1 */

            show_help(1);
            break;

         case 373:   /* CTRL - END */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

              return END;

         case 375:   /* CTRL - HOME */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

              return HOME;

         case 1:     /* CTRL - A */

              if ((retval=choose_address()) != -1)
                 custom.aka = retval;
              break;

         case 6:     /* CTRL - F */

              this = MsgGetCurMsg(areahandle);
              retval = get_request(curmsg, area, areahandle);
              if((tempmsg = MsgOpenMsg(areahandle, MOPEN_READ, (int) MsgUidToMsgn(areahandle, this, UID_EXACT))) != NULL)
                 MsgCloseMsg(tempmsg);
              if(retval == UP)
                 {
                 pageup();
                 stuffkey(6);
                 }
              else if(retval == DOWN)
                 {
                 pagedown();
                 stuffkey(6);
                 }
              break;

         case 14:    /* CTRL - N */

              if ((retval=choose_name()) != -1)
                 custom.name = retval;
              break;

			case 27:		/* esc */

            return ESC;

         case 43:    /*  +  */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

				return NEXTAREA;

         case 45:    /*  -  */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

             return PREVAREA;

         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':

             if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
                break;

              ret = getnumber(c, areahandle);
              if (ret > 0)
                 return ret;
              break;

         default:
              beep();
              break;

			}				/* switch */

	}			/* while(1) */

#undef NOK
}  /* showbody */


/* ---------------------------------------------------------- */


int getnumber(char c, MSG *areahandle)

{
   dword n;
   int ret=1;
   char number[6];
   BOX *numbox;
   MSGH *msghandle;
   char jumpy=0;

   if(cfg.usr.status & JUMPYEDIT)   /* Save jumpy edit setting */
      jumpy = 1;

   cfg.usr.status &= ~JUMPYEDIT;    /* Turn off for now */

   memset(number, '\0', sizeof(number));
   n = (dword) c - 48;
   ltoa(n, number, 10);

   numbox = initbox(12,18,14,41,cfg.col.popframe,cfg.col.poptext,SINGLE,YES,' ');
   drawbox(numbox);
   boxwrite(numbox, 0, 1, "Goto message #");

   while( (ret != ESC) && (ret != 0) )
          ret = getstring(13,35,number,5,"0123456789",cfg.col.entry);

   delbox(numbox);

   if(jumpy)
      cfg.usr.status |= JUMPYEDIT;

   if (ret == ESC) return 0;

   n = atol(number);

   if ( (msghandle = MsgOpenMsg(areahandle, MOPEN_READ, n)) == NULL )
      return 0;

   MsgCloseMsg(msghandle);

   return (int) n;

}

/* ------------------------------------------------------------- */

char *MakeRep(MMSG *msg, MSG *areahandle)

{
   static char repstring[90];
   char temp[80];
   int n=0, gotone=0;
   long realno;

   memset(repstring, '\0', 90);

   realno = MsgUidToMsgn(areahandle, msg->hdr.replyto, UID_EXACT);
   if (realno != 0)
      {
      sprintf(repstring, "%4ld%c ", realno, 17);
      }
   else sprintf(repstring, "      ");

   while( (n < 10) && (msg->hdr.replies[n] != 0) )
     {
     realno = MsgUidToMsgn(areahandle, msg->hdr.replies[n], UID_EXACT);
     if(realno)
        {
        gotone=1;
        sprintf(temp, "%ld ", realno);
        strcat(repstring, temp);
        }
     n++;
     }

   if(msg->replynext != 0)
     {
     realno = MsgUidToMsgn(areahandle, msg->replynext, UID_EXACT);
     if(realno)
       {
       gotone=1;
       sprintf(temp, "(%ld)", realno);
       strcat(repstring, temp);
       }
     }

   if (gotone) repstring[5] = 16;  /* This is a  */

   return repstring;
}

/* --------------------------------------------------------------- */


int PickReply(MMSG *curmsg, MSG *areahandle)
{
   MSGH *msghandle;
   XMSG header;
   int n=0, tot=0, chosen;
   char **names;
   long realno;
   UMSGID this = MsgGetCurMsg(areahandle);
   dword reps[10];


   if (curmsg->hdr.replies[1] == 0)  /* only 1 reply */
      {
      realno = MsgUidToMsgn(areahandle, curmsg->hdr.replies[0], UID_EXACT);

      /* Check if it exists.. */

      if ( (msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL)
         {
         MsgCloseMsg(msghandle);
         return (int) realno;
         }
      else return 0;
      }

   /* Check replies for existance, get names of senders */

   names = xcalloc(11, sizeof(char *));  /* Alloc space for 11 names, 10 is max for squish */
   while(n < 10)
     {
     if( (realno = MsgUidToMsgn(areahandle, curmsg->hdr.replies[n], UID_EXACT)) != 0)
        {
        if ( (msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL)
           {
           if (MsgReadMsg(msghandle, &header, 0, 0, NULL, 0, NULL) != -1)
              {
              names[tot] = xcalloc(1, 42);
              reps[tot]  = realno;
              sprintf(names[tot++], " %-4.4lu %-34s ", realno, header.from);
              }
           MsgCloseMsg(msghandle);
           }
        }
        n++;
        }

   /* Draw a box, listing replies (names of senders of replies) */

   if(tot != 1)
      chosen = pickone(names, 5, 19, 23, 79);
   else
      chosen = 0;

   free_picklist(names);

   return (chosen == -1) ? this : (int) reps[chosen];

}



void pageup(void)
{
	curline=curline-(maxy-6);

	if(curline<0)
		curline=0;
}



void pagedown(void)
{
   if ( (numlines > (maxy-6)) && (curline < numlines-(maxy-6)) )
      {
	   curline+=(maxy-7);

      if (curline>numlines-(maxy-6)) /* Not entire screen filled */
        {
        ClsRectWith(5+(numlines-curline-1),0,maxy-2,maxx-1,cfg.col.msgtext,' ');
        }
      }
}
