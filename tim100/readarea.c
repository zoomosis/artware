#include <alloc.h>
#include <msgapi.h>
#include <conio.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <video.h>
#include <dos.h>
#include <string.h>
#include <share.h>
#include <sys\stat.h>
#include <scrnutil.h>

#include "xmalloc.h"
#include "wrap.h"
#include "tstruct.h"
#include "showmail.h"
#include "getmsg.h"
#include "reply.h"
#include "print.h"
#include "list.h"
#include "message.h"
#include "find.h"
#include "global.h"
#include "repedit.h"
#include "cleanidx.h"
#include "idlekey.h"
#include "showinfo.h"
#include "readarea.h"
#include "help.h"
#include "maint.h"


#ifdef __OS2__

  #define INCL_DOSPROCESS
  #include <os2.h>

#endif

long  GetLast(AREA *area, MSG *areahandle);
void  ReleaseMsg(MMSG *thismsg, int allofit);
void  UpdateLastread(AREA *area, long last, MSG *areahandle);
long  MsgGetLowMsg(MSG *areahandle);
void  beep (void);
void  ScanArea(AREA *area, MSG *areahandle);
void  get_custom_info(AREA *area);
long  get_last_squish(AREA *area);
long  get_last_sdm(AREA *area);
long  get_last_JAM(AREA *area, MSG *areahandle);
void  put_last_squish(AREA *area, long last);
void  put_last_sdm(AREA *area, long last);
void  put_last_JAM(AREA *area, long last, MSG *areahandle);
long  get_last_HMB(AREA *area, MSG *areahandle);
void  put_last_HMB(AREA *area, long last, MSG *areahandle);

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
         last=0,
         lastok=0,
         timhigh,
         lastorhigh;

	int	command=NEXT,
         errors=0;


	/* Open message area */

	if(!(areahandle=MsgOpenArea(area->dir, MSGAREA_CRIFNEC, area->base)))
      {
      Message("Error opening area.", -1, 0, YES);
		return 0;
      }

	/* Read area statistics */

   timhigh = area->highest > 0 ? area->highest : 0;   /* Save 'fast scan' result */

   ScanArea(area, areahandle);                   /* Do a 'real' MSGAPI scan */

   lastorhigh = MsgMsgnToUid(areahandle, area->lr);

   if((timhigh > area->highest) && (area->base & MSGTYPE_SQUISH)) /* Check for 'dirty' index */
     {
     MsgCloseArea(areahandle);
     clean_index(area);
     if(!(areahandle=MsgOpenArea(area->dir, MSGAREA_CRIFNEC, area->base)))
        return 0;
     }

   get_custom_info(area);        /* Get hello, rephello, etc in 'custom' */

	if (area->nomsgs == 0)        /* Empty area! */
		{
      clsw(cfg.col.msgtext);
		print(12, 20, cfg.col.msgtext, "No Messages!");
      print(14, 20, cfg.col.msgtext, "Press ALT-E, E or <INS> to enter a message.");

      switch(get_idle_key(1))
         {
         case 274:
         case 338:
         case 101:
         case  69:
              MakeMessage(NULL, area, areahandle, 0, 0, NULL);
              break;
         case 27:
		        MsgCloseArea(areahandle);
		        return 0;
         default:
              break;
         }

      ScanArea(area, areahandle);

      if (area->nomsgs == 0)
         {
         MsgCloseArea(areahandle);
         return 0;
         }

		}        /* Empty area */


	curno = lastok = area->lr;
	clsw(cfg.col.msgtext);
   stuffkey(10);

	/* Message reading loop */

	do
		{

      readit:

		curmsg = GetMsg(curno, areahandle, 1, area);			/* Read msg from disk */

      if(heapcheck() == -1)
         Message("Heap corrupt (in readarea.c)! Report to the author!", -1, 254, NO);

      if (!curmsg)           /* Error reading msg! */
         {
         if (errors++ > 4)
            {
            Message("Errors, exiting area", 4, 0, NO);
            MsgCloseArea(areahandle);
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
      lastorhigh = curmsg->uid;


      if(custom.mode == DIRECTLIST)  /* ALT-L entry of the area */
         {
         command = LIST;
         custom.mode = NORMAL_MODE; /* Jump to list only once, on entry of area */
         }

      else command = ShowMsg(curmsg, area, areahandle, NORMAL_DISPLAY);			/* Show it! */

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

				   curno = MSGNUM_NEXT;
				   break;

			   case PREV:

				   if (MsgGetCurMsg(areahandle) > area->lowest)
						   curno = MSGNUM_PREV;
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

               /* Free up mem first, could get tight */

               ReleaseMsg(curmsg, 0);

               if ((curno = MsgList(areahandle, area, curno, (command==BROADLIST))) == 0)
                  command = ESC;

               break;

            case FIND:

               /* Free up mem first, could get tight */

               ReleaseMsg(curmsg, 0);

               anchor(DOWN, areahandle);
               FindMessage(areahandle, area, MsgGetCurMsg(areahandle));
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

            case MSGMAINT:

               ReleaseMsg(curmsg, 0);

               anchor(DOWN, areahandle);

               if(maint_menu(area, &areahandle, lastorhigh) == -1)  /* Can't re-open the area!! */
                  {
                  ReleaseMsg(curmsg, 1);
                  return ESC; /* Short way out (what can I do?) Normal routines try to close area! */
                  }

				   curno = anchor(UP, areahandle);
               break;


            default:          /* Catch all other junk */
               break;
			   }
         }

		ReleaseMsg(curmsg, 1);			/* Release mem */
      curmsg=NULL;

	} 	 while( (command != ESC) && (command != EXIT) && (command != NEXTAREA) && (command != PREVAREA) );		/* Until the user hits <ESC> */

	UpdateLastread(area, lastorhigh, areahandle);

	ScanArea(area, areahandle);

	if(MsgCloseArea(areahandle))
		Message("Error closing area!", 2, 0, NO);

   return command;

}



long GetLast(AREA *area, MSG *areahandle)
{
	long last, lr=0;

   if(area->base & MSGTYPE_SQUISH)
      last = get_last_squish(area);
   else if(area->base & MSGTYPE_SDM)
      last = get_last_sdm(area);
   else if(area->base & MSGTYPE_JAM)
      last = get_last_JAM(area, areahandle);
   else
      last = get_last_HMB(area, areahandle);


	lr = MsgUidToMsgn(areahandle, last, UID_NEXT);	/* lastread or first new msg */

	if (lr == 0)										/* didn't exist! */
					lr = MsgUidToMsgn(areahandle, last, UID_PREV);	/* OK, then prev. msg! */

	if (lr == 0)										/* Well then the first */
					lr = MsgUidToMsgn(areahandle, 1L, UID_NEXT);


   return (lr > 0) ? lr : 1;

}



void ReleaseMsg(MMSG *thismsg, int allofit)

{
	LINE *lptr1, *lptr2;

   if (!thismsg) return;

   if (thismsg->ctxt)    free(thismsg->ctxt);
   thismsg->ctxt=NULL;
   if (thismsg->ctext)   free(thismsg->ctext);
   thismsg->ctext=NULL;
	if (thismsg->msgtext) free(thismsg->msgtext);
   thismsg->msgtext=NULL;

	lptr1=thismsg->txt;
	while(lptr1)
		{
		lptr2=lptr1;
		lptr1=lptr1->next;
		free(lptr2);
		}
   thismsg->txt = NULL;

	if(allofit)
      free(thismsg);
}



void UpdateLastread(AREA *area, long last, MSG *areahandle)
{

	if (area->base & MSGTYPE_SQUISH)
			put_last_squish(area, last);
	else if(area->base & MSGTYPE_JAM)
         put_last_JAM(area, last, areahandle);
   else if(area->base & MSGTYPE_SDM)
			put_last_sdm(area, last);
   else if(area->base & MSGTYPE_HMB)
			put_last_HMB(area, last, areahandle);
   else return;

}



long MsgGetLowMsg(MSG *areahandle)

{

	return (MsgUidToMsgn(areahandle, 1L, UID_NEXT));

}

void beep (void)

{
   #ifndef __OS2__
	sound(50);
	delay(10);
	nosound();
   #else
   DosBeep(50, 100);
   #endif
}



void ScanArea(AREA *area, MSG *areahandle)

{

	area->scanned = 1;
	if ( (area->nomsgs  = MsgGetNumMsg (areahandle)) == 0)
      {
      area->highest = 0L;
      area->lowest  = 0L;
      area->lr      = 0L;
      UpdateLastread(area, 0L, areahandle);
      return;
      };

	area->highest = MsgGetHighMsg(areahandle);
	area->lowest  = MsgGetLowMsg (areahandle);
	area->lr      = GetLast(area, areahandle);

}

dword anchor(int direction, MSG *areahandle)
{
   static dword last=0;
   dword retval;

   if (direction == DOWN)
      {
      last = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));
      return last;
      }

   /* else direction == UP */

	if( (retval = MsgUidToMsgn(areahandle, last, UID_NEXT)) == 0)
	   retval = MsgUidToMsgn(areahandle, last, UID_PREV);

   return retval;
}



long get_last_squish(AREA *area)
{
	int lrfile;
	long last;
	char temp[80];


   sprintf(temp, "%s.SQL", area->dir);

	if ((lrfile = open(temp, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      return 1L;

   if(lseek(lrfile, cfg.usr.sqoff * sizeof(long), SEEK_SET) != (cfg.usr.sqoff * sizeof(long)))
     {
     last = -1L;
     }
   else if(read(lrfile, &last, sizeof(long)) != sizeof(long))
     {
     last = 1L;
     }

   close(lrfile);

   return last;

}


long get_last_sdm(AREA *area)
{
	int lrfile;
	long retval;
   word last;
	char temp[80];


   sprintf(temp, "%s\\%s", area->dir, cfg.usr.lr);

	if ((lrfile = open(temp, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      return 1L;

	if(read(lrfile, &last, sizeof(word)) != sizeof(word))
      last = 1L;

   close(lrfile);

   retval = (long) last;

   return retval;

}


long get_last_JAM(AREA *area, MSG *areahandle)
{
   char temp[40];
   int didlock = 0;
   long last;


   if( (last=JAMmbFetchLastRead(areahandle, cfg.usr.name[0].crc)) == -1)
      Message("Error reading lastread!", -1, 0, YES);

   return last;

}



long get_last_HMB(AREA *area, MSG *areahandle)
{
   char temp[40];
   int didlock = 0;

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
   char *lrmem;
   unsigned lastpos = cfg.usr.sqoff * sizeof(long);
   long len, dummy=0;

   sprintf(temp, "%s.SQL", area->dir);

	if ((lrfile = open(temp, O_CREAT|O_RDWR|O_BINARY|SH_DENYWR, S_IREAD|S_IWRITE)) == -1)
		{
      Message("Can't open lastread file!", -1, 0, YES);
      return;
		}

   if( (len = filelength(lrfile)) < lastpos )
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


void put_last_HMB(AREA *area, long last, MSG *areahandle)
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

	if ((lrfile = open(temp, O_CREAT|O_WRONLY|O_TRUNC|O_BINARY|SH_DENYWR, S_IREAD|S_IWRITE)) == -1)
		{
      Message("Can't open lastread file!", -1, 0, YES);
      return;
		}

	if (write(lrfile, &lastint, sizeof(word)) != sizeof(word) )
         Message("Can't write! Disk full?", -1, 0, YES);

	close(lrfile);

}


void put_last_JAM(AREA *area, long last, MSG *areahandle)
{
   int didlock = 0;

   if(!areahandle->locked)
     {
     MsgLock(areahandle);
     didlock = 1;
     }

   if(JAMmbStoreLastRead(areahandle, last, cfg.usr.name[0].crc) == -1)
      Message("Error updating lastread!", -1, 0, YES);

   if(didlock)
      MsgUnlock(areahandle);

}



/* Read all 200 ints of lastreads for entire Hudson base */

int   HMBreadlast(void)
{
       int lrfile;
       char lrfilename[120];

       memset(&HMBlr, '\0', sizeof(HMBlr));

       sprintf(lrfilename, "%s\\lastread.bbs", cfg.usr.hudsonpath);
       if( (lrfile=open(lrfilename, O_CREAT | O_BINARY | O_RDONLY | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
          return -1;

       if(filelength(lrfile) > 0L)
          {
          if(read(lrfile, &HMBlr.lastread, sizeof(word) * 200) != (sizeof(word) * 200))
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

       sprintf(lrfilename, "%s\\lastread.bbs", cfg.usr.hudsonpath);
       if( (lrfile=open(lrfilename, O_BINARY | O_WRONLY | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
          return -1;

       if(write(lrfile, &HMBlr.lastread, sizeof(word) * 200) != (sizeof(word) * 200))
          {
          close(lrfile);
          return -1;
          }

       close(lrfile);

       return 0;

}