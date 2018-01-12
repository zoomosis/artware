#include <stdio.h>
#include <io.h>
#include <msgapi.h>
#include <progprot.h>
#include <alloc.h>
#include <string.h>
#include <conio.h>
#include <fcntl.h>
#include <video.h>
#include <time.h>
#include <scrnutil.h>
#include <stdlib.h>
#include <share.h>
#include <sys\stat.h>

#include <jam.h>

#include "wrap.h"
#include "xmalloc.h"
#include "tstruct.h"
#include "readarea.h"
#include "idx.h"
#include "getmsg.h"
#include "showmail.h"
#include "reply.h"
#include "global.h"
#include "idlekey.h"
#include "shel2dos.h"
#include "help.h"
#include "repedit.h"
#include "message.h"


typedef struct {                       /* MSGIDX.BBS structure definiton   */
  sword msg_num;                         /* Message #                        */
  unsigned char board;                 /* Board # where msg is stored      */
} HMB_MSGIDX;

HMB_MSGIDX *HMBReadEntireIndex(void);

typedef char HMB_MSGTOIDX[36];     /* MSGTOIDX.BBS structure def.      */

typedef struct _hmb_persindex
{

  HMB_MSGIDX            idx;
  struct _hmb_persindex *next;

} HMB_PERSINDEX;

HMB_PERSINDEX *firstHMB;

typedef struct
{
   word   lastread[200];
   time_t read;
} HMBLASTREAD;

extern HMBLASTREAD HMBlr;

/* Guest prototype */
int   HMBreadlast(void);


AREA *SelectArea  (AREA *first, int pickonly, AREA *area_to_start);
void ScanAreas    (AREA *first, int personal);
/* void PersonalScan (MSG *areahandle, AREA *curarea); */
/* void PersSquish   (MSG *areahandle, AREA *curarea); */
int PersMsg      (MSG *areahandle, AREA *curarea);
int PersHMB      (AREA *curarea);
int speedsearch  (char c, AREA *first, int teller);
int fastscan     (AREA *curarea, int personal);
int SQReadIndex  (char *path);
int show_thismsg (MSG *areahandle, AREA *area, UMSGID uid);
int JAMpersonal  (MSG *areahandle, AREA *curarea);
int quickscan    (AREA *area);
void analyse_toidx(void);
void check_toidx(HMB_MSGTOIDX *entry, unsigned recno, int idx);
void Analyse_HMB_Index(void);
int  extracheck(void);


typedef struct
  {

  dword lower;
  dword last;
  dword high;

  } HMBAREAS;

HMBAREAS *HMBareas = NULL;


typedef struct idxdata
   {
   struct my_idx *idxptr;
   long n;
   long last;
   } IDXDATA;

typedef struct persmsg
   {
   UMSGID uid;
   struct persmsg *next;
   } PERSMSG;

typedef struct
   {
   char  fill[12];
   dword activemsgs;
   dword fill2;
   dword BaseMsgNum;
   } JAMINFO;


IDXDATA idxmem;

/* Some HMB specific vars */


typedef struct
{

    unsigned char board;
    char          *msgs;
    word          array_size;

} HMBdata;


/* ---------------------------------------------- */

AREA *SelectArea(AREA *first, int pickonly, AREA *area_to_start)

{
	AREA 	*curptr, *highlighted;
	int 	maxlines=maxy, rows, l, result;
	static int curline=0, start=0;
   int oldstart = -1, oldcur = -1, doredraw;
   int numlines=0;
   int restore_curline=curline, restore_start=start;
	char	temp[81];
   int c;
	BOX	*border;



   for(curptr=first; curptr->next != NULL; curptr=curptr->next)
     {
     if(curptr == area_to_start)
        curline = numlines;
     numlines++;
     }


	rows = numlines >= maxlines-3 ? maxlines-3 : numlines+1;


   if ( (curline>=start+rows) || (curline < start) )
      {
      start = curline - (rows/2);
      if(start < 0)
         start = 0;
      }


   if (start > numlines-rows+1)
      {
      start = numlines-rows+1;
      if(start<0) start=0;
      }

	clsw(cfg.col.astext);

	border=initbox(1,0,rows+2,79,cfg.col.asframe,cfg.col.astext,SINGLE,NO,-1);
	printn(0,1,cfg.col.asbar," Area description:                                                New    Last ", 78);
	drawbox(border);
	delbox(border);
   speedsearch(0, NULL, 0);

	while(1)		/* Exit is w/ return statement that gives command back */
		{
		curptr=first;

		/* search first line to display */

		for(l=0; l<start; l++)
			curptr=curptr->next;

      doredraw =  (start == oldstart) ? 0 : 1; /* Need to repaint whole screen? */
      oldstart = start;

		/* display them.. */

		for(l=start; l<start+rows; l++, curptr=curptr->next)
			{
         if (curptr->scanned)
			   sprintf(temp, " %-40.40s %-20.20s  %5ld  %5ld  ", curptr->desc, curptr->tag, (dword) (curptr->highest-curptr->lr), curptr->lr);
         else
            sprintf(temp, " %-40.40s %-20.20s      -      -  ", curptr->desc, curptr->tag);
         if (curptr->highest > curptr->lr) temp[63] = '*';
         if (!doredraw  && l== oldcur) /* remove inverse bar */
				printn(l-start+2, 1, cfg.col.astext, temp, 78);
			if (l==curline)
				{
				printn(l-start+2, 1, cfg.col.ashigh, temp, 78);    /* Color : 31 */
				highlighted=curptr;
            MoveXY(2, l-start+3); // Position cursor for blind users
				}
			else
				if(doredraw)
               printn(l-start+2, 1, cfg.col.astext, temp, 78);
			}

      oldcur = curline;

		/* Now check for commands... */

		switch (c = get_idle_key(1))
			{
			case 328:		/* up */
            speedsearch(0, NULL, 0);
				if (curline)
					{
					curline--;
					if (curline<start)
						start=curline;
					}
				else
					{
					curline=numlines;
					start=numlines-rows+1;
					}
				break;

			case 336:		/* down */
            speedsearch(0, NULL, 0);
				if (curline<numlines)
					{
					curline++;
					if (curline>=start+rows)
							start++;
					}
				else
					start=curline=0;
				break;

			case 327:		/* home */
            speedsearch(0, NULL, 0);
				curline=start=0;
				break;

			case 335:		/* end */
            speedsearch(0, NULL, 0);
				curline=numlines;
				start=numlines-rows+1;
				break;

			case 329:		/* page up */
            speedsearch(0, NULL, 0);
            if (start == 0)
               {
               curline = 0;
               break;
               }

            start = curline-rows;

            if (start < 0)
               start = 0;

            if (curline > start + rows - 1)
               curline = start;

				break;

			case 337:		/* page down */
            speedsearch(0, NULL, 0);
            if (start == numlines-rows+1)
               {
               curline = numlines;
               break;
               }

            /* start = curline+rows; */
            start = start + rows;

            if (start > numlines-rows+1)
               start = numlines-rows+1;

            if (curline < start)
               curline = start;

				break;

         case 292:
            shell_to_DOS();
            break;

			case 287:		/* ALT-S */
            oldstart = -1; /* Force entire redraw of screen to update * signs */
				ScanAreas(first, 0);
				break;

			case 281:    /* ALT - P */
            if(REGISTERED)
              {
              oldstart = -1; /* Force entire redraw of screen to update * signs */
	           if(!pickonly) ScanAreas(first, 1);
              }
            else
              Message("Personal mailscan only available in registered version.", -1, 0, YES);

            break;

         case 301:    /* ALT - X */
            if (!pickonly)
               {
               if(
                 (!(cfg.usr.status & CONFIRMEXIT)) ||
                 (confirm("Sure you want to leave timEd? (y/N)"))
                 )
                   return NULL;
               }
            break;

         case 294:    /* ALT - L */
            custom.mode = DIRECTLIST;
            return highlighted;

         case 333:   /* Right arrow */
            /* intentional fallthru */

			case 13:		/* <CR> */
            custom.mode = NORMAL_MODE;
            if(pickonly)
              {
              curline = restore_curline;
              start   = restore_start;
              }
				return highlighted;

         case 27:              /* <ESC> */
            if(pickonly)
               {
               curline = restore_curline;
               start   = restore_start;
				   return NULL;
               }
            break;

         case 315:            /* F1 */

            show_help(0);
            break;


         default:
            if ( (c>31 && c<123) || (c==8) || (c==12) )
               {
               if ( (result = speedsearch(c,
                                          c==12 ? highlighted->next  : first,
                                          c==12 ? curline+1 : 0)) != -1 )
                  {
                  curline = start = result;

                  if (start > numlines-rows+1)
                     start = numlines-rows+1;
                  }
               }
            break;

			}				/* switch */

	}			/* while(1) */

}


/* ------------------------------------------------ */


void ScanAreas(AREA *first, int personal)

{

	MSG 	*areahandle;
	AREA	*curarea;
	BOX	*msgbox;
	char  temp[81];
   int   stop = 0;
   HMB_PERSINDEX *this;
   char what;

	msgbox=initbox(11,9,15,69,cfg.col.popframe,cfg.col.poptext,DOUBLE,YES,' ');
	drawbox(msgbox);

   if(cfg.usr.hudsonpath[0] != '\0')
      {
		sprintf(temp, " Analyse Hudson base..");
		boxwrite(msgbox,1,0,temp);
      // sneaky_cache_HMB_index();
      Analyse_HMB_Index();

      if(personal)
         {
         firstHMB = NULL;
         sprintf(temp, " Check personal messages..");
         boxwrite(msgbox,1,0,temp);
         if(extracheck() != 0)  // Hackers at work!!
            {
            what = cfg.usr.name[0].name[0];
            cfg.usr.name[0].name[0] = '\255';
            }
         analyse_toidx();
         if(cfg.usr.name[0].name[0] == '\255')
            cfg.usr.name[0].name[0] = what;
         }
      }

   if(personal && (extracheck() != 0) )      // Hackers!!
     {
     cfg.usr.name[0].hash = cfg.usr.name[0].crc = time(NULL);
     }

	for (curarea = first; curarea; curarea=curarea->next)
		{
      if( xkbhit() && (get_idle_key(1) == 27) )    /* User hit ESC */
         break;

      if( (curarea->base & MSGTYPE_HMB) && !personal )
         continue;

		sprintf(temp, " Scanning area: %-43.43s", curarea->desc);
		boxwrite(msgbox,1,0,temp);

      if ( !(curarea->base & (MSGTYPE_SQUISH | MSGTYPE_HMB)) )
         {
         if( (curarea->base & MSGTYPE_JAM) && (!personal) )
           {
           /* try a quick scan, but it might fail... */
           /* it returns non-zero if it worked OK */
           if(quickscan(curarea) != 0)
              continue;
           }

		   if(!(areahandle=MsgOpenArea(curarea->dir, MSGAREA_NORMAL, curarea->base)))
			   {
			   if (msgapierr == MERR_NOENT)
				   {
				   curarea->highest = 0L;
				   curarea->lowest  = 0L;
				   curarea->lr      = 0L;
				   curarea->nomsgs  = 0L;
				   curarea->scanned = 1;
				   }
    			continue;
			   }

		   /* Get the statistics.... */

         ScanArea(curarea, areahandle);

		   if (personal)
            {
            if(curarea->base & MSGTYPE_SDM)
               stop = PersMsg(areahandle, curarea);
            else
               stop = JAMpersonal(areahandle, curarea);
            }

         MsgCloseArea(areahandle);
		   }
      else
         {
         if(curarea->base & MSGTYPE_SQUISH)
           stop = fastscan(curarea, personal);  /* Only Squish areas */
         else
           stop = PersHMB(curarea);
         }


      if(stop) break;
      }

	delbox(msgbox);

   if(cfg.usr.hudsonpath[0] != '\0')
      {
      // sneaky_cache_HMB_index_dump();

      if(personal)
         {
         this = firstHMB;
         while(this)
            {
            firstHMB = this->next;
            free(this);
            this = firstHMB;
            }
         firstHMB = NULL;   // I think it's redundant, but what the heck...
         }
      }

}



int PersMsg(MSG *areahandle, AREA *curarea)

{
	long  curno = curarea->lr-1;
   int   command = NEXT, l, readresult;
	char	temp[81];
	MSGH	*msghandle;
	XMSG  *header;
	BOX	*persbox;

	persbox=initbox(17,5,19,73,112,112,SINGLE,YES,' ');
	drawbox(persbox);

	while(curno++ < MsgGetHighMsg(areahandle))
		{
      if( xkbhit() && (get_idle_key(1) == 27) )    /* User hit ESC */
         {
         command = ESC;
         break;
         }

		if ((msghandle=MsgOpenMsg(areahandle, MOPEN_READ, curno))==NULL)
			continue;	/* open failed, try next message */

		header = xcalloc(1, sizeof(XMSG));

      readresult = MsgReadMsg(msghandle, header, 0L, 0L, NULL, 0L, NULL);
      MsgCloseMsg(msghandle);

		if ( readresult != -1 )
			{
			sprintf(temp, "%li %-36.36s ", curno, header->to);
			boxwrite(persbox,0,0,temp);

         for(l=0; ((cfg.usr.name[l].name[0] != '\0') && (l<10)); l++)
           {
           if(strcmpi(header->to, cfg.usr.name[l].name)==0)
				  {
              if(!((cfg.usr.status & SKIPRECEIVED) && (header->attr & MSGREAD)))
                   command = show_thismsg(areahandle, curarea, curno);
              break;
              }
           }
         }

		free(header);

      if(command == ESC) break;
		}

		delbox(persbox);

   return (command == ESC) ? 1 : 0;  /* Ret == 1 means stop scan */
}


int PersHMB(AREA *curarea)
{
   HMB_PERSINDEX *this;
   int command = NEXT;
   MSG *areahandle = NULL;
   unsigned char board;

   board = (unsigned char) atoi(curarea->dir);

   for(this=firstHMB; this; this = this->next)
     {
     if( this->idx.board == board )
         {
         if(!areahandle) // Don't open a second time!
           {
           if(!(areahandle=MsgOpenArea(curarea->dir, MSGAREA_CRIFNEC, curarea->base)))
	  		    return 0;
           }

         command = show_thismsg(areahandle, curarea, this->idx.msg_num);
         if(command == ESC)
            break;
         }

     }

   if(areahandle)
      MsgCloseArea(areahandle);

   return (command == ESC) ? 1 : 0;
}


int speedsearch(char c, AREA *first, int teller)

{
   static char speedtag[20];
   AREA *current = first;
   char temp[23];
   int oldteller = teller;

   if (c == 0)   /* clear the string */
      {
      memset(speedtag, '\0', 20);
      sprintf(temp, "컴컴컴컴컴컴컴컴컴컴컴", speedtag);
      print(1,42,cfg.col.asframe,temp);
      return -1;
      }

   if(c == 8)     /* Backspace */
      {
      if(strlen(speedtag))
         speedtag[strlen(speedtag)-1] = '\0';
      }
   else if( (c != 12) && (strlen(speedtag) < 20) )   /* 20 = maxlen */
      {
      speedtag[strlen(speedtag)+1] = '\0';
      speedtag[strlen(speedtag)] = c;
      }

   sprintf(temp, "[%-20.20s]", speedtag);
   print(1,42,cfg.col.asframe,temp);

   for( ; current; current = current->next, teller++)
        {
        if ( strncmpi(current->tag, speedtag, strlen(speedtag)) == 0 )
           return teller;
        }

   for(teller=oldteller,current=first; current; current = current->next, teller++)
        {
        if ( stristr(current->tag, speedtag) != 0 )
           return teller;
        }

   return -1;

}



int fastscan(AREA *curarea, int personal)
{
   int ret = NEXT;
   struct my_idx *l;
   long total=0, current=0, lastuid;
   PERSMSG *firstmsg=NULL, *lastmsg=NULL, *curmsg;
   MSG *areahandle;
   int loop;


   curarea->highest = 0L;
	curarea->lowest  = 0L;
	curarea->lr      = 0L;
	curarea->nomsgs  = 0L;
	curarea->scanned = 1;

   if ( (ret = SQReadIndex(curarea->dir)) != 1)
      return 0;

   if (idxmem.n % sizeof(struct my_idx))
      {
      free(idxmem.idxptr);
      return 0;
      }

   total = (long)idxmem.n/sizeof(struct my_idx);

   if (total > 0)
      {
      l=idxmem.idxptr;
      lastuid=l->umsgid-1;
      while (
              (l->umsgid) &&
              (l->umsgid <= idxmem.last) &&
              (current < total)
            )
         {
         if(l->umsgid <= lastuid)
            {
            curarea->lr = curarea->highest = current;
            free(idxmem.idxptr);
            return 0;
            }
         lastuid=l->umsgid;
         current++;
         l++;
         }
      curarea->lr = current;
      }
   else
      {
      free(idxmem.idxptr);
      return 0;
      }

   while( (current < total) && (l->umsgid > idxmem.last) )
      {
      idxmem.last = l->umsgid;
      for(loop=0; cfg.usr.name[loop].name[0] != '\0'; loop++)
        {
        if(personal &&
              (
              (l->hash == cfg.usr.name[loop].hash) ||
              (l->hash == (cfg.usr.name[loop].hash | 0x80000000Lu) )
              )
           )
           {
           curmsg = xcalloc(1, sizeof(PERSMSG));
           curmsg->uid = l->umsgid;
           if(!firstmsg)
              firstmsg = curmsg;
           else lastmsg->next = curmsg;
           lastmsg = curmsg;
           }
        }
      current++;
      l++;
	   }

   curarea->highest = current;
   curarea->lowest = 1;
   curarea->nomsgs = current;

   free(idxmem.idxptr);


   /* Now let's check if any msgs were found, display them, and free */
   /* the memory taken up by the list op personal msgs               */


   if(firstmsg)                  /* personal msgs found */
      {
      if(!(areahandle=MsgOpenArea(curarea->dir, MSGAREA_CRIFNEC, curarea->base)))
				return 0;

      for(curmsg = firstmsg; curmsg && (ret != ESC); curmsg = curmsg->next)
         {
         if( (ret = show_thismsg(areahandle, curarea, curmsg->uid)) == ESC)
             break;
         }

      while(firstmsg)      /* Free mem again */
        {
        curmsg = firstmsg;
        firstmsg = firstmsg->next;
        free(curmsg);
        }

      MsgCloseArea(areahandle);

      }

   return (ret == ESC) ? 1 : 0;   /* Ret == 1 means stop scan */

}



/* Read a Squish index from disk, and obtain lastread info if needed */

int SQReadIndex(char *path)

{
   int index, last;
   char idxname[100], lastname[100];
   unsigned bytes;

   sprintf(idxname,  "%s.SQI", path);
   sprintf(lastname, "%s.SQL", path);

   memset(&idxmem, '\0', sizeof(idxmem));

   if ( (index = open(idxname, O_RDONLY | O_BINARY | SH_DENYNO)) == -1)
      return 0;

   if ( (lseek(index, 0L, SEEK_END) == -1) ||
        ((idxmem.n = tell(index)) == -1) )
      {
      close(index);
      return -1;
      }

   if (idxmem.n <= 0)
      {
      close(index);
      return 0;
      }

   idxmem.idxptr = xmalloc((int)idxmem.n);

   lseek(index, 0L, SEEK_SET);

   bytes = read(index, idxmem.idxptr, (unsigned) idxmem.n);

   if ( bytes != idxmem.n )
      {
      free(idxmem.idxptr);
      close(index);
      return -1;
      }

   close(index);

   if ( ((last = open(lastname, O_RDONLY | O_BINARY | SH_DENYNO)) == -1))
       idxmem.last = 0;
   else
       {
       if (
            ( (cfg.usr.sqoff != 0) &&
              (lseek(last, cfg.usr.sqoff * sizeof(long), SEEK_SET) != (cfg.usr.sqoff * sizeof(long)))
            ) ||
            (read(last, &idxmem.last, sizeof(long)) != sizeof(long))
          )
         idxmem.last = 0;

       close(last);
       }

   return 1;

}



int show_thismsg(MSG *areahandle, AREA *area, UMSGID uid)
{
   dword curno;
   MMSG *persmsg;
   int command;


   if((curno = MsgUidToMsgn(areahandle, uid, UID_EXACT)) == 0)
      return 0;

   get_custom_info(area);

	savescreen();

   command=0;

   while( (command != ESC)  &&
          (command != NEXT) &&
          (command <= 0) )
          {
          if( (persmsg = GetMsg(curno, areahandle, 1, area)) == NULL)
             {
             sprintf(msg, "Error reading msg #%d in %s", (int) curno, area->tag);
             Message(msg, -1, 0, YES);
             putscreen();
             return NEXT;
             }
          kbflush();

          if(!((cfg.usr.status & SKIPRECEIVED) && (persmsg->hdr.attr & MSGREAD)))
             {
             clsw(cfg.col.msgtext);
			    command = ShowMsg(persmsg, area, areahandle, SCAN_DISPLAY);
             }
          else command = NEXT;
          ReleaseMsg(persmsg, 1);
          }


	putscreen();

   return command;

}



int JAMpersonal(MSG *areahandle, AREA *curarea)
{
   int ret = NEXT;
   JAMIDXREC  *l;
   char *idxmem;
   long total=0, current=0, base;
   PERSMSG *firstmsg=NULL, *lastmsg=NULL, *curmsg;
   int loop, checked;
   int indexhandle;
   int bytesread;


   if(curarea->lr >= curarea->highest)
      {
      curarea->lr = curarea->highest;
      return 0;
      }

   indexhandle = ((JAMDATA *)areahandle->apidata)->IdxHandle;

   if( filelength(indexhandle) == 0 )
      return 0;

   current = curarea->lr - ((JAMDATA *)areahandle->apidata)->HdrInfo.BaseMsgNum;

   base = current;

   idxmem = xmalloc(2000 * sizeof(JAMIDXREC));
   l = (JAMIDXREC *)idxmem;

   do
      {
      if(
         (lseek(indexhandle,
                (long) ((long) current * (long) sizeof(JAMIDXREC)),
                SEEK_SET) == -1) ||

        ((bytesread=read(indexhandle, idxmem, 2000 * sizeof(JAMIDXREC))) == -1)
        )
        {
        Message("Error reading JAM index!", -1, 0, YES);
        break;
        }

      total = (long) (bytesread/ sizeof(JAMIDXREC));

      l = (JAMIDXREC *)idxmem;

      // Checked: check elements this read; current: keeps increasing (realno)
      for(checked=0; checked < total; checked++, current++, l++)
        {
        for(loop=0; cfg.usr.name[loop].name[0] != '\0'; loop++)
          {
          if(l->UserCRC == cfg.usr.name[loop].crc)
             {
             if(MsgUidToMsgn(areahandle, current + ((JAMDATA *)areahandle->apidata)->HdrInfo.BaseMsgNum, UID_EXACT) != 0)
                {
                 curmsg = xcalloc(1, sizeof(PERSMSG));
                 curmsg->uid = current + ((JAMDATA *)areahandle->apidata)->HdrInfo.BaseMsgNum;
                 if(!firstmsg)
                     firstmsg = curmsg;
                 else lastmsg->next = curmsg;
                 lastmsg = curmsg;
                 }
             }
           }
        }
//      current = base + 2000;
//      base = current;
      } while ( bytesread == (2000 * sizeof(JAMIDXREC)) );   // Do loop..

   free(idxmem);

   /* Now let's check if any msgs were found, display them, and free */
   /* the memory taken up by the list op personal msgs               */


   if(firstmsg)                  /* personal msgs found */
      {
      for(curmsg = firstmsg; curmsg && (ret != ESC); curmsg = curmsg->next)
         {
         if( (ret = show_thismsg(areahandle, curarea, curmsg->uid)) == ESC)
             break;
         }

      while(firstmsg)      /* Free mem again */
        {
        curmsg = firstmsg;
        firstmsg = firstmsg->next;
        free(curmsg);
        }
      }

   return (ret == ESC) ? 1 : 0;   /* Ret == 1 means stop scan */

}


/* Try to do a quick scan on a JAM base. Return 0 if we failed, 1 = success */


int quickscan(AREA *area)
{
   int jhr, jlr;
   char jhrname[120], jdxname[120], jlrname[120];
   JAMINFO jaminfo;
   struct stat statinfo;
   JAMLREAD lread;

   area->highest = 0L;
	area->lowest  = 0L;
	area->lr      = 0L;
	area->nomsgs  = 0L;
	area->scanned = 1;

   sprintf(jhrname, "%s.jhr", area->dir);
   if( (jhr=open(jhrname, O_BINARY | O_RDONLY | O_DENYNONE)) == -1)
     {
     return 1; /* Cannot open - all set to zero */
     }

   if(read(jhr, &jaminfo, sizeof(JAMINFO)) != sizeof(JAMINFO)) /* Corrupt? Stop.. */
     {
     close(jhr);
     return 1;
     }

   close(jhr);

   if(jaminfo.activemsgs == 0)
      return 1;

   sprintf(jdxname, "%s.jdx", area->dir);
   if (stat(jdxname, &statinfo) == -1)
      return 1;

   /* Check if the area is 'contiguous', return 0 to do 'slow' scan if not */

   if( (statinfo.st_size / sizeof(JAMIDXREC)) != jaminfo.activemsgs)
      return 0;

   area->highest = jaminfo.BaseMsgNum + jaminfo.activemsgs - 1;
   area->lowest  = jaminfo.BaseMsgNum;
   area->nomsgs  = jaminfo.activemsgs;

   sprintf(jlrname, "%s.jlr", area->dir);
   if( (jlr=open(jlrname, O_BINARY | O_RDONLY | O_DENYNONE)) == -1)
     return 1;

   while(read(jlr, &lread, sizeof(JAMLREAD)) == sizeof(JAMLREAD))
     {
     if(lread.UserCRC == cfg.usr.name[0].crc)
        {
        area->lr = lread.HighReadMsg;
        if(area->lr > area->highest)
           area->lr = area->highest;
        if(area->lr < area->lowest)
           area->lr = area->lowest;
        break;
        }
     }

   close(jlr);

   return 1;
}


void analyse_toidx(void)
{
   int toidx, idx, l, bytes, howmuch;
   char temp[120];
   HMB_MSGTOIDX *block;
   unsigned int total = 0;
   #define BATCHSIZE 300

   sprintf(temp, "%s\\msgidx.bbs", cfg.usr.hudsonpath);

   if( (idx = open(temp, O_BINARY | O_RDWR | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
      return;

   sprintf(temp, "%s\\msgtoidx.bbs", cfg.usr.hudsonpath);

   if( (toidx = open(temp, O_BINARY | O_RDWR | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
      {
      close(idx);
      return;
      }

   block = xcalloc(BATCHSIZE, sizeof(HMB_MSGTOIDX));

   while((bytes=read(toidx, block, BATCHSIZE * sizeof(HMB_MSGTOIDX))) == (BATCHSIZE * sizeof(HMB_MSGTOIDX)))
      {
      for(l=0; l<BATCHSIZE; l++)
          check_toidx(&block[l], total+l, idx);
      total += BATCHSIZE;
      }

   howmuch = bytes / sizeof(HMB_MSGTOIDX);
   for(l=0; l<howmuch; l++)
          check_toidx(&block[l], total+l, idx);

   free(block);
   close(toidx);
   close(idx);
}


void check_toidx(HMB_MSGTOIDX *entry, unsigned recno, int idx)
{
   int l;
   HMB_PERSINDEX *thisone;
   static HMB_PERSINDEX *last = NULL;


   for(l=0; ((cfg.usr.name[l].name[0] != '\0') && (l<10)); l++)
     {
     if(strncmpi((char *)entry+1, cfg.usr.name[l].name, strlen(cfg.usr.name[l].name))==0)
		  {
        thisone = xcalloc(1, sizeof(HMB_PERSINDEX));
        if(firstHMB == NULL)
           firstHMB = thisone;
        else
           last->next  = thisone;
        last = thisone;

        if(lseek(idx, (long) ((long)recno * sizeof(HMB_MSGIDX)), SEEK_SET) !=
                              (long) ((long)recno * sizeof(HMB_MSGIDX)))
           {
           Message("Error seeking Hudson index!", -1, 0, YES);
           continue;
           }

        if(read(idx, &thisone->idx, sizeof(HMB_MSGIDX)) != sizeof(HMB_MSGIDX))
           {
           Message("Error reading Hudson index!", -1, 0, YES);
           continue;
           }

        }
     }

}


/* ------------------------------------------------------ */

void Analyse_HMB_Index(void)
{
   HMB_MSGIDX *thismsg, *base=NULL;
   struct stat mystat;
   char idxname[120];
   unsigned total, n;
   register int board;
   AREA *thisarea = cfg.first;

   sprintf(idxname, "%s\\msgidx.bbs", cfg.usr.hudsonpath);

   if(stat(idxname, &mystat) != 0)
     {
     Message("Error reading HMB index!", -1, 0, YES);
     return;
     }

   total = (unsigned)  (mystat.st_size / (long) sizeof(HMB_MSGIDX));

   if(total == 0)
      return;

   HMBreadlast();  // If error, all lr's on 0.. (fuck them!)

   HMBareas = xcalloc(sizeof(HMBAREAS), 200);

   if( (thismsg = HMBReadEntireIndex()) == NULL )
     {
     Message("Error reading HMB index!", -1, 0, YES);
     return;
     }

   base = thismsg;
   for(n=0; n<total; n++, thismsg++)
     {
     if(thismsg->msg_num == -1)     // Deleted message!
        continue;

     board = (int) thismsg->board - 1;

     if( (board < 0) || (board > 199) )
        {
        sprintf(msg, "Illegal board number %d (msgnum: %d, recno: %d)!", board, thismsg->msg_num, n+1);
        Message(msg, -1, 0, YES);
        continue;
        }

     HMBareas[board].high += 1;   // Update no of msgs in this area

     if(HMBareas[board].last == 0)  // Looking for lastread info?
       {
       if(thismsg->msg_num == HMBlr.lastread[board])
          HMBareas[board].last = HMBareas[board].high;

       else if(thismsg->msg_num < HMBlr.lastread[board])
          HMBareas[board].lower = HMBareas[board].high;

       else // Higher
          {
          if(HMBareas[board].lower != 0)
             HMBareas[board].last = HMBareas[board].lower;
          else
             HMBareas[board].last = HMBareas[board].high;
          }
       }
     }

   for( ; thisarea; thisarea = thisarea->next)
     {
     if(!(thisarea->base & MSGTYPE_HMB))
        continue;

     board = atoi(thisarea->dir);
     if( (board < 1) || (board > 200) )
        continue;

     board--;    // Make 0 based for index
     thisarea->scanned = 1;
     thisarea->nomsgs  = HMBareas[board].high;
     thisarea->highest = HMBareas[board].high;
     thisarea->lowest  = HMBareas[board].high ? 1 : 0;
     if( (thisarea->lr = HMBareas[board].last) == 0)
          thisarea->lr = thisarea->highest;
     }

     free(HMBareas);
     if(base) free(base);
}


// Extra check on regkey. Returns 0 if all is well.

int extracheck(void)
{
   dword ascvals3=0L;
   char *charptr;
   char name[36];

   strcpy(name, cfg.usr.name[0].name);
   strlwr(name);

   for(charptr=name; *charptr; charptr++)
     ascvals3 += ((unsigned char) ( ((*charptr/3)*5) + 11) );

  if(cfg.key.strkey.ascvals3 != ascvals3)
    return 1;

  return 0;

}