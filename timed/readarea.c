#include "includes.h"

#ifdef __OS2__

  #define INCL_DOSPROCESS
  #include <os2.h>

#endif

#ifdef __NT__

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODE
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS

#include <windows.h>

#endif

long  GetLast(AREA *area, MSG *areahandle, int raw);
void  ReleaseMsg(MMSG *thismsg, int allofit);
void  UpdateLastread(AREA *area, long last, dword highest, MSG *areahandle);
long  MsgGetLowMsg(MSG *areahandle);
void  beep (void);
void  ScanArea(AREA *area, MSG *areahandle, int raw);
void  get_custom_info(AREA *area);
long  get_last_squish(AREA *area);
long  get_last_sdm(AREA *area);
long  get_last_JAM(MSG *areahandle);
void  put_last_squish(AREA *area, long last);
void  put_last_sdm(AREA *area, long last);
void  put_last_JAM(long last, MSG *areahandle, dword highest);
long  get_last_HMB(MSG *areahandle);
void  put_last_HMB(long last, MSG *areahandle);
int   ReScanArea(MSG **areahandle, AREA *area, dword lastorhigh, dword highest);
dword GetMaySeeLastRead(MSG *areahandle, AREA *area, dword lr);


typedef struct
{

    word msgno;
    word recno;

} API_HMBIDX;

typedef struct
{

    unsigned char board;
    API_HMBIDX    *msgs;
    word          array_size;

} HMBdata;


int   HMBreadlast(void);
int   HMBwritelast(void);


typedef struct
{
   word   lastread[200];
   time_t read;
} HMBLASTREAD;

HMBLASTREAD HMBlr;



int ReadArea(AREA *area)
{

	MSG 	*areahandle = NULL;
	MMSG	*curmsg     = NULL;
   MSGH  *msghandle  = NULL;


	dword	curno=0,
         lastok=0,
         timhigh,
         lastorhigh,
         highest=0,
         howmuch;

	int	command=NEXT,
         errors=0,
         retval;


   #ifdef __WATCOMC__
   _heapmin();
   #endif

	/* Open message area */

	if(!(areahandle=MsgOpenArea(area->dir, MSGAREA_CRIFNEC, area->base)))
      {
      Message("Error opening area.", -1, 0, YES);
      showerror();
		return 0;
      }


	/* Read area statistics */

   timhigh = area->highest > 0 ? area->highest : 0;   /* Save 'fast scan' result */

   ScanArea(area, areahandle, 0);                   /* Do a 'real' MSGAPI scan */

   lastorhigh = MsgMsgnToUid(areahandle, area->lr);


   if((timhigh > area->highest) &&
      (area->base & MSGTYPE_SQUISH) &&
      !BePrivate(area) ) /* Check for 'dirty' index */
     {
     MsgCloseArea(areahandle);
     clean_index(area);
     if(!(areahandle=MsgOpenArea(area->dir, MSGAREA_CRIFNEC, area->base)))
        return 0;
     }

   get_custom_info(area);        /* Get hello, rephello, etc in 'custom' */

   while(area->nomsgs == 0)        /* Empty area! */
		{
      clsw(cfg.col[Cmsgtext]);
		print(12, 20, cfg.col[Cmsgtext], "No Messages!");
      print(14, 20, cfg.col[Cmsgtext], "Press ALT-E, E or <INS> to enter a message.");

      switch(get_idle_key(1, READERSCOPE))
         {
         case cREADenter:
            MakeMessage(NULL, area, areahandle, 0, 0, NULL);
            break;

         case cREADgoback:
            MsgCloseArea(areahandle);
            return 0;

         case cREADmaintenance:
            // retval: -2 == error, -1 == no messages in area.
            if(maint_menu(area) == -2)  /* Can't re-open the area!! */
              return ESC; /* Short way out (what can I do?) Normal routines try to close area! */
            break;

         case cREADinfo:
            showinfo(NULL, area, areahandle);
            break;

         case cREADchangeaddress:     /* CTRL - A */
            if ((retval=choose_address()) != -1)
               custom.aka = retval;
            break;

         case cREADchangename:    /* CTRL - N */
            if ((retval=choose_name()) != -1)
               custom.name = retval;
            break;

         case cREADrunexternal:

            curmsg = mem_calloc(1, sizeof(MMSG));
            if(runexternal(area, &areahandle, curmsg, 0, 0) == -1)
              {
              mem_free(curmsg);
              // Argh! We can't reopen the area. Go back soon!
              area->mlist = NULL;
              return ESC; /* Short way out (what can I do?) Normal routines try to close area! */
              }
            mem_free(curmsg);
            break;

         case cREADsqundelete:
            if(!(area->base & MSGTYPE_SQUISH))
              {
              Message("This is not a Squish area!", -1, 0, YES);
              break;
              }
            anchor(DOWN, areahandle);
            savescreen();
            AttemptLock(areahandle);
            Message("Please wait, undeleting..", 0, 0, NO);
            howmuch=SquishUndelete(areahandle);
            MsgUnlock(areahandle);
            ScanArea(area, areahandle, 0);     /* Do a 'real' MSGAPI scan */
            if(howmuch == (dword) -1)
               Message("Error undeleting messages!", -1, 0, YES);
            else
               {
               sprintf(msg, "%lu message were undeleted.", howmuch);
               Message(msg, -1, 0, NO);
               }
            putscreen();
            curno = anchor(UP, areahandle);
            break;


         default:
            break;
         }

      ScanArea(area, areahandle, 0);

		}        /* Empty area */


	curno = lastok = area->lr;
	clsw(cfg.col[Cmsgtext]);
   stuffkey(cREADsetbookmark);

   // Set up marklist space

   if(area->mlist == NULL)
     area->mlist = InitMarkList();

	/* Message reading loop */

	do
		{
      readit:

		curmsg = GetFmtMsg(curno, areahandle, area);			/* Read msg from disk */

      if(heapcheck() == -1)
         Message("Heap corrupt (in readarea.c)! Report to the author!", -1, 254, NO);

      if (!curmsg)           /* Error reading msg! */
         {
         if(((retval = ReScanArea(&areahandle, area, lastorhigh, highest)) != 0) ||
            (errors++ > 4) )
            {
            Message("Errors, exiting area", 4, 0, NO);
            if(retval != -2) MsgCloseArea(areahandle);
            return 0;
            }

         if (lastok < area->highest)
            curno = ++lastok;             /* Goto next.. */

         else if (lastok > area->lowest)
            curno = --lastok;              /* Goto prev.. */

         else
              {
              MsgCloseArea(areahandle);
              return 0;
              }

         goto readit;

         }

      lastok = curno = MsgGetCurMsg(areahandle);         /* Where are we? */
      if( (lastorhigh = curmsg->uid) > highest)
        highest = lastorhigh;

      command = ShowMsg(curmsg, area, areahandle, NORMAL_DISPLAY);			/* Show it! */

      if(MsgGetCurMsg(areahandle) != curno)
        {
        if((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, curno)) != NULL)
            MsgCloseMsg(msghandle);                      /* 'reposition' MSGAPI where we were.. */
        }

      errors=0;

      if (command>0)
         curno = command;
      else
         {
		   switch(command)
			   {
			   case NEXT:

               if(BePrivate(area))
                 curno = GetNextPrivate(area, areahandle);
               else
                 curno = MSGNUM_NEXT;
				   break;

			   case PREV:

				   if (MsgGetCurMsg(areahandle) > area->lowest)
                 {
                 if(BePrivate(area))
                     curno = GetPrevPrivate(area, areahandle);
                 else
						   curno = MSGNUM_PREV;
                 }
				   else { curno = area->lowest; beep(); }
				   break;

            case HOME:

                 curno = MsgGetLowMsg(areahandle);
                 break;

            case END:

                 curno = MsgGetHighMsg(areahandle);
                 break;


            case LIST :
            case BROADLIST:

               /* mem_free up mem first, could get tight */

               ReleaseMsg(curmsg, 0);

               if ((curno = MsgList(areahandle, area, curno, (command==BROADLIST))) == 0)
                  command = ESC;

               break;

            case FIND:

               /* mem_free up mem first, could get tight */

               ReleaseMsg(curmsg, 0);

               anchor(DOWN, areahandle);
               FindMessage(areahandle, area, MsgGetCurMsg(areahandle));
               if(area->nomsgs == 0)
                  command = ESC;
               curno = anchor(UP, areahandle);
               /* This might have been reset */

               get_custom_info(area);
               break;

			   case ENTER:

               anchor(DOWN, areahandle);
               ReleaseMsg(curmsg, 0);
               MakeMessage(NULL, area, areahandle, 0, 0, NULL);
               curno = anchor(UP, areahandle);

				   break;

            case EDITHELLO:

               edit_hello_strings(area);
               break;

            case FILTERMSG:

               FilterMessage(curmsg, area, areahandle, 0);
               break;

            case CHANGECHARSET:

               PickCharMap();
               break;

            case FILTERREALBODY:

               FilterMessage(curmsg, area, areahandle, 1);
               break;

            case SQUNDELETE:

               if(!(area->base & MSGTYPE_SQUISH))
                 {
                 Message("This is not a Squish area!", -1, 0, YES);
                 break;
                 }
               anchor(DOWN, areahandle);
               savescreen();
               AttemptLock(areahandle);
               Message("Please wait, undeleting..", 0, 0, NO);
               howmuch=SquishUndelete(areahandle);
               MsgUnlock(areahandle);
               ScanArea(area, areahandle, 0);     /* Do a 'real' MSGAPI scan */
               if(howmuch == (dword) -1)
                  Message("Error undeleting messages!", -1, 0, YES);
               else
                  {
                  sprintf(msg, "%lu message were undeleted.", howmuch);
                  Message(msg, -1, 0, NO);
                  }
               putscreen();
               curno = anchor(UP, areahandle);
               break;

            case SDMRENUMBER:

               if(!(area->base & MSGTYPE_SDM))
                 {
                 Message("This is not a *.MSG area!", -1, 0, YES);
                 break;
                 }
               savescreen();
               Message("Please wait, renumbering..", 0, 0, NO);
               if((curno = (dword) SDMRenumber(areahandle)) == -1)
                  Message("There was an error while renumbering", -1, 0, YES);
               ScanArea(area, areahandle, 0);     /* Do a 'real' MSGAPI scan */
               if((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, curno)) != NULL)
                   MsgCloseMsg(msghandle);                      /* 'reposition' MSGAPI where we were.. */
               putscreen();
               break;

            case EXTERNAL:

               anchor(DOWN, areahandle);

               if(runexternal(area, &areahandle, curmsg, lastorhigh, highest) == -1)
                 {
                 // Argh! We can't reopen the area. Go back soon!
                 ReleaseMsg(curmsg, 1);
                 DumpMarkList(area->mlist);
                 DumpMarkList(area->mayseelist);
                 area->mlist = NULL;
                 area->mayseelist = NULL;
                 return ESC; /* Short way out (what can I do?) Normal routines try to close area! */
                 }

               curno = anchor(UP, areahandle);

               break;

            case MSGMAINT:

               ReleaseMsg(curmsg, 0);

               anchor(DOWN, areahandle);

               // return : -2 == error, -1 == no msgs in area anymore
               curno = maint_menu(area);
               if(curno== -1 || curno == -2)  /* Can't re-open the area!! */
                  {
                  ReleaseMsg(curmsg, 1);
                  DumpMarkList(area->mlist);
                  DumpMarkList(area->mayseelist);
                  area->mlist = NULL;
                  area->mayseelist = NULL;
                  return ESC; /* Short way out (what can I do?) Normal routines try to close area! */
                  }

               if(curno == 0)
                  curno = anchor(UP, areahandle);
               else   // renumbered area: reposition
                 {
                 if((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, curno)) != NULL)
                     MsgCloseMsg(msghandle);                      /* 'reposition' MSGAPI where we were.. */
                 }
               break;

            default:          /* Catch all other junk */
               break;
			   }
         }

		ReleaseMsg(curmsg, 1);			/* Release mem */
      curmsg=NULL;

	} 	 while( (command != ESC) && (command != EXIT) && (command != NEXTAREA) && (command != PREVAREA) );		/* Until the user hits <ESC> */

	UpdateLastread(area, lastorhigh, highest, areahandle);

   ScanArea(area, areahandle, 0);

	if(MsgCloseArea(areahandle))
		Message("Error closing area!", 2, 0, NO);

   // Are there any message marks left?
   if( (command != EXIT) &&
       ( (area->mlist->active == 0) ||
       (confirm("There are marked messages! Erase marks? [y/N]") != 0)) )
     {
     DumpMarkList(area->mlist);
     area->mlist = NULL;
     }

   DumpMarkList(area->mayseelist);
   area->mayseelist = NULL;

   return command;

}



long GetLast(AREA *area, MSG *areahandle, int raw)
{
	long last, lr=0;

   if(area->base & MSGTYPE_SQUISH)
      last = get_last_squish(area);
   else if(area->base & MSGTYPE_SDM)
      last = get_last_sdm(area);
   else if(area->base & MSGTYPE_JAM)
      last = get_last_JAM(areahandle);
   else
      last = get_last_HMB(areahandle);

   if(last == 0L)
     {
     if(raw)    // The return lowest in area, minus 1
       {
       lr = MsgUidToMsgn(areahandle, 1L, UID_NEXT);
       return (lr ? lr-1 : 0);
       }
     else
       last=1L;    // UIDToMsgn won't work with 0.
     }

   if(raw)
     {
     lr = MsgUidToMsgn(areahandle, last, UID_EXACT);  // Does is still exist?
     if(!lr)
       {
       lr = MsgUidToMsgn(areahandle, last, UID_NEXT);
       if(!lr)
         {
         lr = MsgUidToMsgn(areahandle, last, UID_PREV);  // First previous one
         }
       else
         lr--;   // Sit one below first new message..
       }
     return lr;
     }

   // not raw if we get here

   lr = MsgUidToMsgn(areahandle, last, UID_NEXT);  /* lastread or first new msg */

	if(lr == 0)										/* didn't exist! */
     lr = MsgUidToMsgn(areahandle, last, UID_PREV);	/* OK, then prev. msg! */

	if(lr == 0)										/* Well then the first */
     lr = MsgUidToMsgn(areahandle, 1L, UID_NEXT);


   return (lr > 0) ? lr : 1;

}



void ReleaseMsg(MMSG *thismsg, int allofit)
{

   if (!thismsg) return;

   FreeMIS(&thismsg->mis);

   if (thismsg->ctxt)    mem_free(thismsg->ctxt);
   thismsg->ctxt=NULL;
   if (thismsg->ctext)   mem_free(thismsg->ctext);
   thismsg->ctext=NULL;
   if (thismsg->msgtext) mem_free(thismsg->msgtext);
   thismsg->msgtext=NULL;

   FreeLines(thismsg->txt);
   thismsg->txt = NULL;

   if(thismsg->firstblock)
      FreeBlocks(thismsg->firstblock);
   thismsg->firstblock = NULL;

	if(allofit)
      mem_free(thismsg);
}



void UpdateLastread(AREA *area, long last, dword highest, MSG *areahandle)
{

	if (area->base & MSGTYPE_SQUISH)
			put_last_squish(area, last);
	else if(area->base & MSGTYPE_JAM)
         put_last_JAM(last, areahandle, highest);
   else if(area->base & MSGTYPE_SDM)
			put_last_sdm(area, last);
   else if(area->base & MSGTYPE_HMB)
         put_last_HMB(last, areahandle);
   else return;

}



long MsgGetLowMsg(MSG *areahandle)
{

	return (MsgUidToMsgn(areahandle, 1L, UID_NEXT));

}

void beep (void)
{
   #if !defined(__OS2__) && !defined(__NT__)
   int i;
	sound(50);
//   delay(10);
//   sleep(1);
   for(i=0; i<1000; i+=2)
     i--;
	nosound();
   #elif defined(__NT__)
   Beep(50, 100);
   #else
   DosBeep(50, 100);
   #endif
}



void ScanArea(AREA *area, MSG *areahandle, int raw)
{
   dword noprivate, highprivate, lowprivate, lastprivate;

	area->scanned = 1;
	if ( (area->nomsgs  = MsgGetNumMsg (areahandle)) == 0)
      {
      area->highest = 0L;
      area->lowest  = 0L;
      area->lr      = 0L;
      UpdateLastread(area, 0L, 0L, areahandle);
      return;
      };

	area->highest = MsgGetHighMsg(areahandle);
	area->lowest  = MsgGetLowMsg (areahandle);
   area->lr      = GetLast(area, areahandle, raw);

   // These are the basic stats, but what if we only have to
   // show private messages!

   if( (cfg.usr.status & RESPECTPRIVATE) &&
       (area->type != ECHOMAIL && area->type != NEWS) )
     {
     noprivate = highprivate = lowprivate = lastprivate = 0L;
     ScanMaySee(area, areahandle);
     if( (area->nomsgs = area->mayseelist->active) != 0)
       {
       area->lowest = MsgUidToMsgn(areahandle, area->mayseelist->list[0], UID_EXACT);
       area->highest = MsgUidToMsgn(areahandle,
               area->mayseelist->list[area->mayseelist->active-1], UID_EXACT);
       area->lr = GetMaySeeLastRead(areahandle, area, area->lr);
       }
     }


}

// ==============================================================

dword GetMaySeeLastRead(MSG *areahandle, AREA *area, dword lr)
{
   dword uid;
   int i;

   uid = MsgMsgnToUid(areahandle, lr);
   if(IsMarked(area->mayseelist, uid)) // The lastread is a message we may see
     return lr;

   // Otherwise we traverse the list of marked messages to find a higher
   // one. If we don't, we'll take a lower one..

   for(i=0; i<area->mayseelist->active; i++)
     {
     if(area->mayseelist->list[i] > uid)
       return MsgUidToMsgn(areahandle, area->mayseelist->list[i], UID_EXACT);
     }

   // If we got here, there wasn't a higher one. Take the highest (last)

   return MsgUidToMsgn(areahandle, area->mayseelist->list[i-1], UID_EXACT);

}

// ==============================================================

dword anchor(int direction, MSG *areahandle)
{
   #define MAXANCHORS 10
   static dword last[MAXANCHORS];
   static int now=0;
   dword retval;

   if (direction == DOWN)
      {
      if(now < MAXANCHORS)
         last[now++] = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));
      return last[now-1];
      }

   /* else direction == UP */

   if(now > 0)
     {
     now--;
	  if( (retval = MsgUidToMsgn(areahandle, last[now], UID_NEXT)) == 0)
	     retval = MsgUidToMsgn(areahandle, last[now], UID_PREV);
     }
   else Message("Anchor error!", -1, 0, YES);

   return retval;
}



long get_last_squish(AREA *area)
{
	int lrfile;
	long last;
	char temp[80];


   sprintf(temp, "%s.SQL", area->dir);

   if ((lrfile = sopen(temp, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
      return 0L;

   if(lseek(lrfile, cfg.usr.sqoff * sizeof(long), SEEK_SET) != (cfg.usr.sqoff * sizeof(long)))
     {
     last = 0L;
     }
   else if(read(lrfile, &last, sizeof(long)) != sizeof(long))
     {
     last = 0L;
     }

   close(lrfile);

   return last;

}


long get_last_sdm(AREA *area)
{
	int lrfile;
	long retval;
   dword last= 0L;
	char temp[80];


   sprintf(temp, "%s\\%s", area->dir, cfg.usr.lr);

   if ((lrfile = sopen(temp, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
      return 0L;

	if(read(lrfile, &last, sizeof(word)) != sizeof(word))
      last = 0L;

   close(lrfile);

   retval = (long) last;

   return retval;

}


long get_last_JAM(MSG *areahandle)
{
   long last;

   if( (last=JAMmbFetchLastRead(areahandle, cfg.usr.name[0].crc, (cfg.usr.status & JAMGETLAST) ? 1 : 0)) == -1)
      Message("Error reading lastread!", -1, 0, YES);

   return last;
}



long get_last_HMB(MSG *areahandle)
{
   if(HMBlr.read < (time(NULL) - 10) )
      {
      if(HMBreadlast() == -1)
         return 1L;
      }

   return(HMBlr.lastread[(((HMBdata *)areahandle->apidata)->board - 1)]);

}




void put_last_squish(AREA *area, long last)
{
	char temp[80];
	int lrfile;
   unsigned lastpos = cfg.usr.sqoff * sizeof(long);
   long dummy=0;

   sprintf(temp, "%s.SQL", area->dir);

   if ((lrfile = sopen(temp, O_CREAT|O_RDWR|O_BINARY, SH_DENYWR, S_IREAD|S_IWRITE)) == -1)
		{
      Message("Can't open lastread file!", -1, 0, YES);
      return;
		}

   if(filelength(lrfile) < lastpos )
      {
      lseek(lrfile, 0L, SEEK_END);
      while(tell(lrfile) < lastpos)
        {
        if(write(lrfile, &dummy, sizeof(long)) != sizeof(long))
           {
           Message("Can't write! Disk full?", -1, 0, YES);
           close(lrfile);
           return;
           }
        }
      }

   if(lseek(lrfile, lastpos, SEEK_SET) != lastpos)
      {
      Message("Can't seek lastread file!", -1, 0, YES);
      close(lrfile);
      return;
      }

	if ( (write(lrfile, &last, sizeof(long))) != sizeof(long) )
         Message("Can't write! Disk full?", -1, 0, YES);

	close(lrfile);

}


void put_last_HMB(long last, MSG *areahandle)
{

   if(HMBreadlast() == -1)
     {
     Message("Error updating lastread!", -1, 0, YES);
     return;
     }

   HMBlr.lastread[(((HMBdata *)areahandle->apidata)->board - 1)] = (word) last;

   if(HMBwritelast() == -1)
     {
     Message("Error updating lastread!", -1, 0, YES);
     return;
     }

}


void put_last_sdm(AREA *area, long last)
{
	char temp[80];
	int lrfile;
   word lastint;


   sprintf(temp, "%s\\%s", area->dir, cfg.usr.lr);

   lastint = (word) last;

   if ((lrfile = sopen(temp, O_CREAT|O_WRONLY|O_TRUNC|O_BINARY, SH_DENYWR, S_IREAD|S_IWRITE)) == -1)
		{
      Message("Can't open lastread file!", -1, 0, YES);
      return;
		}

	if (write(lrfile, &lastint, sizeof(word)) != sizeof(word) )
         Message("Can't write! Disk full?", -1, 0, YES);

	close(lrfile);

}


void put_last_JAM(long last, MSG *areahandle, dword highest)
{
   int didlock = 0;

   if(!areahandle->locked)
     {
     AttemptLock(areahandle);
     didlock = 1;
     }

   if(JAMmbStoreLastRead(areahandle, last, highest, cfg.usr.name[0].crc) == -1)
      Message("Error updating lastread!", -1, 0, YES);

   if(didlock)
      MsgUnlock(areahandle);

}



/* Read all 200 ints of lastreads for entire Hudson base */

int   HMBreadlast(void)
{
  int lrfile;
  char lrfilename[120];
  long offset;

  memset(&HMBlr, '\0', sizeof(HMBlr));

  sprintf(lrfilename, "%s\\lastread.bbs", cfg.usr.hudsonpath);
  if( (lrfile=sopen(lrfilename, O_CREAT | O_BINARY | O_RDWR, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
     return -1;

  offset = (long) (cfg.usr.hudsonoff * (sizeof(word) * 200));

  if(filelength(lrfile) < (offset+(sizeof(word)*200)))
     {
     if(chsize(lrfile, (long) offset + ((long)sizeof(word)*200L)) == -1)
       Message("Error resizing lastread file!", -1, 0, YES);
     else
       {          // All's well, no read necessary though..
       close(lrfile);
       return 0;
       }
     }

  if(filelength(lrfile) > 0L)
     {
     if(
         (lseek(lrfile, offset, SEEK_SET) != offset) ||
         (read(lrfile, &HMBlr.lastread, (unsigned) (sizeof(word) * 200)) != (int) (sizeof(word) * 200))
       )
        {
        close(lrfile);
        return -1;
        }
     }

  close(lrfile);

  HMBlr.read = time(NULL);

  return 0;

}

int   HMBwritelast(void)
{
  int lrfile;
  char lrfilename[120];
  long offset;


  sprintf(lrfilename, "%s\\lastread.bbs", cfg.usr.hudsonpath);
  if( (lrfile=sopen(lrfilename, O_BINARY | O_WRONLY | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
     return -1;

  offset = (long) (cfg.usr.hudsonoff * (sizeof(word) * 200));

  if(
     (lseek(lrfile, offset, SEEK_SET) != offset) ||
     (write(lrfile, &HMBlr.lastread, sizeof(word) * 200) != (sizeof(word) * 200))
    )
     {
     close(lrfile);
     return -1;
     }

  close(lrfile);

  return 0;

}

// ===============================================================

int ReScanArea(MSG **areahandle, AREA *area, dword lastorhigh, dword highest)
{

   UpdateLastread(area, lastorhigh, highest, *areahandle);

   if(MsgCloseArea(*areahandle))
      {
      Message("Error closing area!", 2, 0, NO);
      return -1;
      }

   if(!(*areahandle=MsgOpenArea(area->dir, MSGAREA_CRIFNEC, area->base)))
      {
      Message("Error re-opening area!", -1, 0, YES);
      showerror();
      return -2;
      }

   /* Read area statistics */

   ScanArea(area, *areahandle, 0);     /* Do a 'real' MSGAPI scan */

   return (area->nomsgs > 0) ? 0 : -1;  /* Ret -1, exit area if no msgs */

}

// ================================================================


