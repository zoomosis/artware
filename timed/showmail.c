#include "includes.h"

dword getnumber (char c, MSG *areahandle, AREA *area);
dword PickReply (MMSG *curmsg, MSG *areahandle);
int   HighlightLines (MMSG *curmsg);


void pagedown (void);
void pageup   (void);

/* These two used for msg display. Global to be used by freq */
/* function, together with the two functions above           */

static int numlines,
             curline;


typedef struct
{
   AREA   *area;
   UMSGID number;

} MARK;

MARK mark;


dword ShowMsg(MMSG *msg, AREA *area, MSG *areahandle, int displaytype)
{
	char	temp[MAX_SCREEN_WIDTH];
	dword command;


   paint_header(msg, area->type);

   sprintf(temp, "%-0.50s", MakeRep(msg, areahandle, area));
   #ifdef __SENTER__
   print(1,26,cfg.col[Cmsglinks],temp);
   #else
   print(0,26,cfg.col[Cmsglinks],temp);
   #endif

   if(IsMarked(area->mlist, msg->uid))
   #ifdef __SENTER__
      printn(2, maxx-8, cfg.col[Cmsgattribs], "[Marked]", 8);
   #else
      printn(1, maxx-8, cfg.col[Cmsgattribs], "[Marked]", 8);
   #endif

#if !defined(__OS2__) && !defined(__NT__)
   if(!(cfg.usr.status & CLOCK))
      sprintf(temp, "Area: %-31.31s 100%%  Current: %5ld, high: %5ld ³  %3dK ", area->desc, MsgGetCurMsg(areahandle), area->highest, (int)(((dword)coreleft())/1024));
   else
      {
      sprintf(temp, "Area: %-31.31s 100%%  Current: %5ld, high: %5ld ", area->desc, MsgGetCurMsg(areahandle), area->highest);
      update_clock(1);
      }

#else
   sprintf(temp, "Area: %-31.31s 100%%  Current: %5ld, high: %5ld ³", area->desc, MsgGetCurMsg(areahandle), area->highest);
#endif

//   if(maxx > 80)
//           {
//           memset(temp+80, ' ',maxx-80);
//           temp[maxx]='\0';
//           }

   if(!(cfg.usr.status & CLOCK))
      printeol(maxy-1,0,cfg.col[Cmsgbar],temp);
   else
     {
     memset(temp+strlen(temp), ' ', sizeof(temp)-strlen(temp));
     temp[maxx-8] = '\0';
     print(maxy-1, 0, cfg.col[Cmsgbar],temp);
     }

   clockon();
	command=showbody(msg, areahandle, area, displaytype);
   clockoff();

	return command;
}


// ==============================================================

dword showbody(MMSG *curmsg, MSG *areahandle, AREA *area, int displaytype)

{
   int l;
	LINE *top, *last, *lptr;
	char	temp[MAX_SCREEN_WIDTH];
   int c, linecolor, retval, key;
   LINE *first;
   MSGH *msghandle, *tempmsg;
   long realno;
   unsigned long a, b;
   unsigned int progress;
   dword curno, ret=0;
   BOX *endarea;
   UMSGID this;
   MIS tmpmis;


#define NOK !(cfg.usr.status & SHOWKLUDGES)

   start:

   numlines = 0;
   curline = 0;
   first = curmsg->txt;
	top=first;
	lptr=top;

	/* Count the number of lines the message exists of */

	for(last=first; last != NULL; last=last->next)
         {
         if(!(NOK && (last->status & KLUDGE)))
			   numlines++;
         }

   #ifdef __SENTER__
   if (numlines < maxy-7)  /* if msg is shorter than screen, clear lower part of screen */
      ClsRectWith(6+numlines,0,maxy-2,maxx-1,cfg.col[Cmsgtext],' ');
   #else
   if (numlines < maxy-6)  /* if msg is shorter than screen, clear lower part of screen */
      ClsRectWith(5+numlines,0,maxy-2,maxx-1,cfg.col[Cmsgtext],' ');
   #endif

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
	   print(maxy-1,38,cfg.col[Cmsgbar],temp);

      #ifdef __SENTER_
      MoveXY(1,7);
      #else
      MoveXY(1,6);
      #endif

      #ifdef __SENTER__
      for(l=0; (l<maxy-7) && lptr; l++, lptr=lptr->next)
      #else
      for(l=0; (l<maxy-6) && lptr; l++, lptr=lptr->next)
      #endif
         {
         while(NOK && (lptr->status & KLUDGE))
            {
            lptr = lptr->next;
            if(!lptr) break;     /* end of msg reached */
            }
         if(!lptr) break;     /* end of msg reached */

         if(lptr->status & HIGHLIGHT)
            linecolor = cfg.col[Cmsgspecial];
         else if (lptr->status & NORMTXT)
            linecolor = cfg.col[Cmsgtext];
         else if(lptr->status & QUOTE)
            linecolor = cfg.col[Cmsgquote];
         else if(lptr->status & KLUDGE)
            linecolor = cfg.col[Cmsgkludge];
         else if(lptr->status & ORIGIN || lptr->status & TEAR)
            linecolor = cfg.col[Cmsgorigin];
         if(lptr->status & HIGHLIGHT)
            linecolor = cfg.col[Cmsgspecial];

         #ifdef __SENTER__
         printeoln(6+l,0,linecolor,lptr->ls, lptr->len);
         #else
         printeoln(5+l,0,linecolor,lptr->ls, lptr->len);
         #endif
			}

		/* Now check for commands... */

      switch (c = get_idle_key(1, READERSCOPE))
			{
         case cREADreturnbookmark:  /* TAB */
           if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
                break;

           if(mark.area == area)
             {
             if((realno=MsgUidToMsgn(areahandle, mark.number, UID_EXACT)) != 0)
                return realno;
             }
           break;

         case cREADsetbookmark:     /* CTRL <ENTER> */

           mark.area = area;
           mark.number = curmsg->uid;
           break;

         case cREADedithello:    /* ALT - H */

           if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
                break;

           return EDITHELLO;

         case cREADexit:    /* ALT - X */

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

         case cREADlookuporigaddress:         // F10
            memset(&tmpmis, '\0', sizeof(MIS));
            tmpmis.destfido = curmsg->mis.origfido;
            tmpmis.destfido.point = 0;              // zero out point number.
            sprintf(tmpmis.to, "%s", FormAddress(&tmpmis.destfido));
            savescreen();
            check_node(&tmpmis, 0, 1);
            putscreen();
            break;

         case cREADlookuptoname:          // Shift-F10
            memset(&tmpmis, '\0', sizeof(MIS));
            strcpy(tmpmis.to, curmsg->mis.to);
            savescreen();
            check_node(&tmpmis, 0, 1);
            putscreen();
            break;

         case cREADlookupfromname:          // F9
            memset(&tmpmis, '\0', sizeof(MIS));
            strcpy(tmpmis.to, curmsg->mis.from);
            savescreen();
            check_node(&tmpmis, 0, 1);
            putscreen();
            break;

         case cREADnext:      /* right   */

            if(displaytype == SCAN_DISPLAY)  /* No check for scanning functions */
               return NEXT;

            if(CurrentIsNotLast(area, areahandle))
                   return NEXT;
				else
               {
               if(cfg.usr.status & ENDAREAMENU)
                  {
                  endarea = initbox(11, 12, 17, 68, cfg.col[Cpopframe], cfg.col[Cpoptext], SINGLE, YES, ' ');
                  drawbox(endarea);
                  boxwrite(endarea,0,1,"           End of messages in this area!");
                  boxwrite(endarea,2,1,"<right>, <enter> or '+' : next area with new mail");
                  boxwrite(endarea,3,1,"<ESC>                   : exit area");
                  boxwrite(endarea,4,1,"Other keys              : remove this text");

                  kbflush();

                  key = get_idle_key(1, GLOBALSCOPE);

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

         case cREADmark:       /* <enter> */

            if(IsMarked(area->mlist, curmsg->uid)==0)
              {
              AddMarkList(area->mlist, curmsg->uid);
              #ifdef __SENTER__
              printn(2, maxx-8, cfg.col[Cmsgattribs], "[Marked]", 8);
              #else
              printn(1, maxx-8, cfg.col[Cmsgattribs], "[Marked]", 8);
              #endif
              }
            else
              {
              RemoveMarkList(area->mlist, curmsg->uid);
              #ifdef __SENTER__
              printn(2, maxx-8, cfg.col[Cmsgattribs], "        ", 8);
              #else
              printn(1, maxx-8, cfg.col[Cmsgattribs], "        ", 8);
              #endif
              }
            break;

         case cREADprevious:      /* left */
               return PREV;

         case cREADgotooriginal:   /* ctrl - left */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            realno = MsgUidToMsgn(areahandle, curmsg->mis.replyto, UID_EXACT);

            if ( (realno != 0) &&
                 ((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL) )
               {
               MsgCloseMsg(msghandle);
               return realno;
               }
            else break;

         case cREADnextreply:   /* ALT - right              */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            realno = MsgUidToMsgn(areahandle, curmsg->mis.nextreply, UID_EXACT);

            if ( (realno != 0) &&
                 ((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL) )
               {
               MsgCloseMsg(msghandle);
               return realno;
               }
            else break;

         case cREADgotoreply:   /* ctrl - right */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            ret = 0;
            if(curmsg->mis.replies[0] != 0)
               ret = PickReply(curmsg, areahandle);
            if (ret)
               return ret;
            else
               break;

         case cREADup:      /* up */
				if (curline)
						curline--;
				break;

         case cREADdown:      /* down */
            #ifdef __SENTER__
            if ( (curline < numlines-(maxy-7)) && (numlines > (maxy-7)) )
            #else
            if ( (curline < numlines-(maxy-6)) && (numlines > (maxy-6)) )
            #endif
						curline++;
				break;

         case cREADbegtext:      /* home */
				curline=0;
				break;

         case cREADendtext:      /* end */
            #ifdef __SENTER__
            if (numlines > (maxy-7))
               curline=numlines-(maxy-7);
            #else
            if (numlines > (maxy-6))
					curline=numlines-(maxy-6);
            #endif

				break;

         case cREADpageup:      /* page up */

            pageup();
				break;

         case cREADpagedown:      /* page down */

            pagedown();
            break;

         case cREADnextmsgorpage:      /* Space */

            #ifdef __SENTER__
            if ( (numlines > (maxy-7)) && (curline < numlines-(maxy-7)) )
               stuffkey(cREADpagedown);
            #else
            if ( (numlines > (maxy-6)) && (curline < numlines-(maxy-6)) )
               stuffkey(cREADpagedown);
            #endif
             else
               {
               if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
                  return HELP;
               else
                  stuffkey(cREADnext);
               }
            break;

         case cREADnextmarked:    /* ctrl-up */
            if( ((curno=NextMarked(area->mlist, curmsg->uid)) == 0) ||
                ((curno=MsgUidToMsgn(areahandle, curno, UID_EXACT)) == 0) )
              beep();
            else
              return curno;
            break;

         case cREADprevmarked:    /* ctrl-dn */
            if( ((curno=PrevMarked(area->mlist, curmsg->uid)) == 0) ||
                ((curno=MsgUidToMsgn(areahandle, curno, UID_EXACT)) == 0) )
              beep();
            else
              return curno;
            break;

         case cREADwrite:    /* ALT - W */

            PrintMessage(curmsg, area, areahandle, 0, 0, PRINTALL);  /* Last element: no hardcopy */
            break;

         case cREADwritebody:

            PrintMessage(curmsg, area, areahandle, 0, 0, PRINTBODY);  /* Last element: no hardcopy */
            break;

         case cREADwriterealbody:

            PrintMessage(curmsg, area, areahandle, 0, 0, PRINTREALBODY);  /* Last element: no hardcopy */
            break;

         case cREADwriteheader:

            PrintMessage(curmsg, area, areahandle, 0, 0, PRINTHDR);  /* Last element: no hardcopy */
            break;

         case cREADprint:  /* ALT-P */

            PrintMessage(curmsg, area, areahandle, 1, 0, PRINTALL);  /* Last element: a hardcopy */
            break;

         case cREADenter:      /* ALT - E */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

				return ENTER;

         case cREADreply:      /* ALT - R */

            curno = anchor(DOWN, areahandle);
            clockoff();
            MakeMessage(curmsg, area, areahandle, 1, curno, NULL);
            clockon();

				return(anchor(UP, areahandle));

         case cREADmaintenance:     /* ALT-U */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return MSGMAINT;

         case cREADturboreply:     /* ALT - T */

            curno = anchor(DOWN, areahandle);
            clockoff();
            MakeMessage(curmsg, area, areahandle, 3, curno, NULL);
            clockon();

				return (anchor(UP, areahandle));

         case cREADbouncereply:               // CTRL-B - Bounce reply

            if(area->type != NETMAIL)
               break;
            curno = anchor(DOWN, areahandle);
            clockoff();
            MakeMessage(curmsg, area, areahandle, 4, curno, NULL);
            clockon();

				return (anchor(UP, areahandle));

         case cREADinfo:   /* ALT - I */

            clockoff();
            showinfo(curmsg, area, areahandle);
            clockon();
            break;

         case cREADfollowup:    /* ALT - O */

            curno = anchor(DOWN, areahandle);
            clockoff();
            MakeMessage(curmsg, area, areahandle, 2, curno, NULL);
            clockon();

				return (anchor(UP, areahandle));

         case cREADgoback:    /* ALT - A */

//            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
//               break;

            return ESC;

         case 27:        // ESC, might be that ESC is not bound to a command..

            if(displaytype != SCAN_DISPLAY)  // If not scanning, this is only
               break;                        // used to jump to AreaSel, people
                                             // should defined cREADgoback for that..

            return ESC;

         case cREADshell:    /* ALT - J */

            shell_to_DOS();
            break;

         case cREADdelete:    /* ALT D */

           anchor(DOWN, areahandle);

           if( (area->mlist->active==0) ||
               (confirm("Process marked messages? [y/N]")==0) )
             {
             if(
                (!(cfg.usr.status & CONFIRMDELETE)) ||
                (confirm("Delete current message. Are you sure? (y/N)"))
               )
               {
				   MsgKillMsg(areahandle, MsgGetCurMsg(areahandle));
               RemoveMarkList(area->mlist, curmsg->uid);
               }
             }
           else
             {
             if(
                (!(cfg.usr.status & CONFIRMDELETE)) ||
                (confirm("Delete marked messages. Are you sure? (y/N)"))
               )

             DeleteMarked(area, areahandle);
             }

			  curno = anchor(UP, areahandle);

           ScanArea(area, areahandle, 0);

           if( (area->type == NETMAIL) && (cfg.usr.netsema[0] != '\0'))
              creat_touch(cfg.usr.netsema);

           if(area->nomsgs == 0)
             {
             if(displaytype != SCAN_DISPLAY) // Don't return ESC in mailscan! It will abort it!
                return ESC;
             else
                return NEXT;
             }

           else return curno;

         case cREADfind:    /* ALT - F */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return FIND;

         case cREADtogglekludges:      /* ALT - K */

            cfg.usr.status ^= SHOWKLUDGES;
            goto start;

         case cREADlist:    /* ALT - L */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return LIST;

         case cREADbroadlist:       /* B */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

            return BROADLIST;

         case cREADchange:      /* ALT - C */

            clockoff();
            ChangeMessage(curmsg, area, areahandle, 1);
				return 0L;   /* Reread, may have dumped from mem */

         case cREADchangeheader:        /* CTRL - H */

            clockoff();
            ChangeMessage(curmsg, area, areahandle, 0);
            return 0L;

         case cREADchangeattributes:     /* ALT - S */

             clockoff();
             ChangeMessage(curmsg, area, areahandle, -1);
             return 0L;

         case cREADunreceive:     /* CTRL - U */

            unreceive(curmsg, areahandle, area);
            break;

         case cREADreplyother:    /* ALT - N */

            clockoff();
            ReplyOther(curmsg, area);
            return 0L;

         case cREADmove:      /* ALT - M */

            anchor(DOWN, areahandle);
            clockoff();
				MoveMessage(areahandle, area, curmsg);
            clockon();
				curno = anchor(UP, areahandle);

            ScanArea(area, areahandle, 0);
				if (area->nomsgs == 0)
                {
                if(displaytype == SCAN_DISPLAY)  // Don't return ESC in a mailscan!
                    return NEXT;
                else
						  return ESC;
                }
            else return curno;

         case cREADhelp:    /* F1 */

            show_help(1);
            break;

         case cREADlastmsg:   /* CTRL - END */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

              return END;

         case cREADfirstmsg:   /* CTRL - HOME */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

              return HOME;

         case cREADchangeaddress:     /* CTRL - A */

              if ((retval=choose_address()) != -1)
                 custom.aka = retval;
              break;

         case cREADfreqfiles:     /* CTRL - F */

              this = MsgGetCurMsg(areahandle);
              clockoff();
              retval = get_request(curmsg, area, areahandle);
              clockon();
              ScanArea(area, areahandle, 0);
              if((tempmsg = MsgOpenMsg(areahandle, MOPEN_READ, (int) MsgUidToMsgn(areahandle, this, UID_EXACT))) != NULL)
                 MsgCloseMsg(tempmsg);

              if(retval == UP)
                 {
                 pageup();
                 stuffkey(cREADfreqfiles);
                 }
              else if(retval == DOWN)
                 {
                 pagedown();
                 stuffkey(cREADfreqfiles);
                 }
              else
                 ScanArea(area, areahandle, 0);
              break;

         case cREADchangename:    /* CTRL - N */

              if ((retval=choose_name()) != -1)
                 custom.name = retval;
              break;

         case cREADchangecharset: /* CTRL - N */

              return CHANGECHARSET;
              break;

         case cREADmarkchain:    /* '*' */

            this = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));
            MarkReplyChain(area, areahandle, curmsg->uid);
            #ifdef __SENTER__
            printn(2, maxx-8, cfg.col[Cmsgattribs], "[Marked]", 8);
            #else
            printn(1, maxx-8, cfg.col[Cmsgattribs], "[Marked]", 8);
            #endif
            if((tempmsg = MsgOpenMsg(areahandle, MOPEN_READ, (int) MsgUidToMsgn(areahandle, this, UID_EXACT))) != NULL)
               MsgCloseMsg(tempmsg);  // reposition MSGAPI to current msg..
            break;

         case cREADnextareanewmail:    /*  +  */

				return NEXTAREA;

         case cREADprevareanewmail:    /*  -  */

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;

             return PREVAREA;

         case cREADrunexternal:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;
            return EXTERNAL;

         case cREADfiltermsg:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;
            return FILTERMSG;

         case cREADfiltermemory:

            FilterMemory(curmsg, 1);
            goto start;

         case cREADfilterrealbody:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;
            return FILTERREALBODY;

         case cREADsqundelete:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;
            return SQUNDELETE;

         case cREADsdmrenumber:

            if(displaytype == SCAN_DISPLAY)  /* Not for scanners :) */
               break;
            return SDMRENUMBER;

         case cREADfiledelete:

            FileDelete();
            break;

         case cREADsearchcurmsg:
            retval = HighlightLines(curmsg);
            if(retval != -1) // something was found, try to center first found
              {
              curline = retval;
              #ifdef __SENTER__
              if(curline > ((maxy-7)/2))
                 curline -= (maxy-7)/2;
              #else
              if(curline > ((maxy-6)/2))
                 curline -= (maxy-6)/2;
              #endif

              else
                 curline = 0;
              }

            if (curline>numlines-(maxy-6)) /* Not entire screen filled */
              #ifdef __SENTER__
              ClsRectWith(6+(numlines-curline-1),0,maxy-2,maxx-1,cfg.col[Cmsgtext],' ');
              #else
              ClsRectWith(5+(numlines-curline-1),0,maxy-2,maxx-1,cfg.col[Cmsgtext],' ');
              #endif

            break;

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

              ret = getnumber(c, areahandle, area);
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

dword getnumber(char c, MSG *areahandle, AREA *area)
{
   dword n, curno;
   int ret=1;
   char number[7];
   BOX *numbox;
   MSGH *msghandle;
   char jumpy=0;

   if(cfg.usr.status & JUMPYEDIT)   /* Save jumpy edit setting */
      jumpy = 1;

   cfg.usr.status &= ~JUMPYEDIT;    /* Turn off for now */

   curno = MsgGetCurMsg(areahandle);   // Save position, to later reset MSGAPI.

   memset(number, '\0', sizeof(number));
   n = (dword) c - 48;
   ltoa(n, number, 10);

   numbox = initbox(12,18,14,41,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   drawbox(numbox);
   boxwrite(numbox, 0, 1, "Goto message #");

   while( (ret != ESC) && (ret != 0) )
          ret = getstring(13,35,number,6,6,"0123456789",cfg.col[Centry], cfg.col[Cpoptext]);

   delbox(numbox);

   if(jumpy)
      cfg.usr.status |= JUMPYEDIT;

   if (ret == ESC) return 0;

   n = atol(number);

   if ( (msghandle = MsgOpenMsg(areahandle, MOPEN_READ, n)) == NULL )
      return 0;

   MsgCloseMsg(msghandle);

   if( (area->type != ECHOMAIL && area->type != NEWS) &&
       (cfg.usr.status & RESPECTPRIVATE) &&
        !MaySee(areahandle, n) )
      {
      sprintf(msg, "That is a private message not from or to yourself");
      Message(msg, -1, 0, YES);
      // Reset MSGAPI to point again to current message.
      if((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, curno)) != NULL)
           MsgCloseMsg(msghandle);
      return 0;
      }

   return n;

}

/* ------------------------------------------------------------- */

char *MakeRep(MMSG *msg, MSG *areahandle, AREA *area)
{
   static char repstring[90];
   char temp[80];
   int n=0, gotone=0;
   long realno;
   dword wherenow = MsgGetCurMsg(areahandle);
   MSGH *msghandle;

   memset(repstring, '\0', 90);

   realno = MsgUidToMsgn(areahandle, msg->mis.replyto, UID_EXACT);
   if( (realno != 0) &&
       (!(BePrivate(area) && !MaySee(areahandle, realno)))
     )
      {
      sprintf(repstring, "%4ld%c ", realno, 17);
      }
   else sprintf(repstring, "      ");

   while( (n < 9) && (msg->mis.replies[n] != 0) )
     {
     realno = MsgUidToMsgn(areahandle, msg->mis.replies[n], UID_EXACT);
     if(realno &&
        (!(BePrivate(area) && !MaySee(areahandle, realno)))
       )
        {
        gotone=1;
        sprintf(temp, "%ld ", realno);
        strcat(repstring, temp);
        }
     n++;
     }

   if(msg->mis.nextreply != 0)
     {
     realno = MsgUidToMsgn(areahandle, msg->mis.nextreply, UID_EXACT);
     if(realno &&
        (!(BePrivate(area) && !MaySee(areahandle, realno)))
       )
       {
       gotone=1;
       sprintf(temp, "(%ld)", realno);
       strcat(repstring, temp);
       }
     }

   if (gotone) repstring[5] = 16;  /* This is a  */

   if(MsgGetCurMsg(areahandle) != wherenow)
     {
     if((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, wherenow)) != NULL)
         MsgCloseMsg(msghandle);  /* 'reposition' MSGAPI where we were.. */
     }

   return repstring;
}

/* --------------------------------------------------------------- */


dword PickReply(MMSG *curmsg, MSG *areahandle)
{
   MSGH *msghandle;
   MIS mis;
   int n=0, tot=0, chosen;
   char **names;
   long realno;
   UMSGID this = MsgGetCurMsg(areahandle);
   dword reps[10];


   if (curmsg->mis.replies[1] == 0)  /* only 1 reply */
      {
      realno = MsgUidToMsgn(areahandle, curmsg->mis.replies[0], UID_EXACT);

      /* Check if it exists.. */

      if ( (msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL)
         {
         MsgCloseMsg(msghandle);
         return (int) realno;
         }
      else return 0;
      }

   /* Check replies for existance, get names of senders */

   names = mem_calloc(11, sizeof(char *));  /* Alloc space for 11 names, 10 is max for squish */
   while(n < 9)
     {
     if( (realno = MsgUidToMsgn(areahandle, curmsg->mis.replies[n], UID_EXACT)) != 0)
        {
        if ( (msghandle = MsgOpenMsg(areahandle, MOPEN_READ, realno)) != NULL)
           {
           if (MsgReadMsg(msghandle, &mis, 0, 0, NULL, 0, NULL) != -1)
              {
              names[tot] = mem_calloc(1, 42);
              reps[tot]  = realno;
              sprintf(names[tot++], " %-4.4lu %-34s ", realno, mis.from);
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

   return (chosen == -1) ? this : reps[chosen];

}

// =============================================================

void pageup(void)
{
   curline=curline-(maxy-7);

	if(curline<0)
		curline=0;
}


// =============================================================

void pagedown(void)
{
   #ifdef __SENTER__
   if ( (numlines > (maxy-7)) && (curline < numlines-(maxy-7)) )
      {
      curline+=(maxy-8);

      if (curline>numlines-(maxy-7)) /* Not entire screen filled */
        {
        ClsRectWith(6+(numlines-curline-1),0,maxy-2,maxx-1,cfg.col[Cmsgtext],' ');
        }
      }
   #else
   if ( (numlines > (maxy-6)) && (curline < numlines-(maxy-6)) )
      {
	   curline+=(maxy-7);

      if (curline>numlines-(maxy-6)) /* Not entire screen filled */
        {
        ClsRectWith(5+(numlines-curline-1),0,maxy-2,maxx-1,cfg.col[Cmsgtext],' ');
        }
      }
   #endif
}

// =====================================================================

int HighlightLines(MMSG *curmsg)
{
  BOX  *inputbox;
  int ret;
  static char keyword[80] = "";
  LINE *curline;
  int firstfound=-1;
  int curlineno = 0;

  inputbox = initbox(10,10,15,70,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
  drawbox(inputbox);
  boxwrite(inputbox,0,1,"Give the keyword to look for:");
  ret = getstring(13, 12, keyword, 56, 79, "",cfg.col[Centry], cfg.col[Cpoptext]);
  delbox(inputbox);

  if (ret == ESC) return -1;

  for(curline=curmsg->txt; curline; curline=curline->next)
    {
    if(curline->status & KLUDGE)
      {
      if(cfg.usr.status & SHOWKLUDGES)
         curlineno++;
      continue;
      }

    curlineno++;
    if(curline->ls && (stristr(curline->ls, keyword) != NULL))
       {
       curline->status |= HIGHLIGHT;
       if(firstfound == -1)
          firstfound = curlineno;
       }
    }

  return firstfound;

}

// =====================================================================

