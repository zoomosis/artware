#include "includes.h"
#include "idlekey.h"
#include <jam.h>

//#define __NOHMB__

char *views[] = {

" ~A~ll areas                  ",
" ~T~agged areas               ",
" Areas with ~n~ew mail        ",
" Tagged areas with new ~m~ail ",
""
};


typedef struct {                       /* MSGIDX.BBS structure definiton   */
  sword msg_num;                       /* Message #                        */
  unsigned char board;                 /* Board # where msg is stored      */
} HMB_MSGIDX;


HMB_MSGIDX * HMBReadEntireIndex(void);

typedef char HMB_MSGTOIDX[36];     /* MSGTOIDX.BBS structure def.      */


int debugtest;

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
void ScanAreas    (AREA *first, int personal, int unscanned);
int  PersMsg      (MSG *areahandle, AREA *curarea);
int  PersHMB      (AREA *curarea);
int  speedsearch  (char c, AREA *first, int teller, int pickonly);
int  fastscan     (AREA *curarea, int personal);
int  show_thismsg (MSG *areahandle, AREA *area, UMSGID uid);
int  JAMpersonal  (MSG *areahandle, AREA *curarea);
int  quickscan    (AREA *area);
void analyse_toidx(void);
void check_toidx(HMB_MSGTOIDX *thisentry, unsigned recno, int idx);
void Analyse_HMB_Index(void);
int  extracheck(void);
int  SetView(void);


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


//IDXDATA idxmem;
static int scannedall=0;


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
   short maxlines=maxy;
   int rows, l, result;
	static int curline=0, start=0;
   int oldstart, oldcur, doredraw;
   int numlines;
   int restore_curline=curline, restore_start=start;
	char	temp[81];
   int c, movedown;
	BOX	*border;

   start:

   movedown =  1;
   numlines =  0;
   oldstart = -1;
   oldcur   = -1;

   if(!pickonly && (cfg.mode == SHOWNEW || cfg.mode == SHOWNEWTAGGED))
      ScanAreas(cfg.first, 0, 1);

   for(curptr=first; curptr != NULL; curptr=curptr->next)
     {
     if(area_to_start && curptr == area_to_start)
        curline = numlines;

     if(pickonly || AreaVisible(curptr))
       numlines++;
     }

   area_to_start = NULL;

   if(numlines == 0)    // No matching areas found!
     {
     area_to_start=NULL;
     curline=0;
     scannedall=0;

     switch(cfg.mode)
       {
       case SHOWNEW:
         Message("No more areas with unread messages..", -1, 0, YES);
         cfg.mode = SHOWALL;
         break;

       case SHOWTAGGED:
         Message("No tagged areas found..", -1, 0, YES);
         cfg.mode = SHOWALL;
         break;

       case SHOWNEWTAGGED:
         Message("No more new messages in tagged areas..", -1, 0, YES);
         cfg.mode = SHOWTAGGED;
         break;
       }
     goto start;  // Wrong, try again! :-)
     }

	rows = numlines >= maxlines-3 ? maxlines-3 : numlines;

   if(curline>numlines-1)
      curline=numlines-1;

   if ( (curline>=start+rows) || (curline < start) )
      {
      start = curline - (rows/2);
      if(start < 0)
         start = 0;
      }


   if (start > numlines-rows)
      {
      start = numlines-rows;
      if(start<0) start=0;
      }

	clsw(cfg.col[Castext]);
	border=initbox(1,0,rows+2,79,cfg.col[Casframe],cfg.col[Castext],SINGLE,NO,-1);
	printn(0,0,cfg.col[Casbar],"  Area description:                                                New    Last  ", 80);
	drawbox(border);
	delbox(border);
   speedsearch(0, NULL, 0, pickonly);

	while(1)		/* Exit is w/ return statement that gives command back */
		{
		curptr=first;

		/* search first line to display */

		for(l=0; l<start; /* nothing */)
        {
        if(pickonly || AreaVisible(curptr)) l++;
        curptr=curptr->next;
        }

      doredraw =  (start == oldstart) ? 0 : 1; /* Need to repaint whole screen? */
      oldstart = start;

		/* display them.. */

		for(l=start; l<start+rows; curptr=curptr->next)
			{
         if(!pickonly && AreaVisible(curptr) == 0)
           continue;

         if (curptr->scanned)
			   sprintf(temp, " %-40.40s %-20.20s  %5ld  %5ld  ", curptr->desc, curptr->tag, (dword) (curptr->highest-curptr->lr), curptr->lr);
         else
            sprintf(temp, " %-40.40s %-20.20s      -      -  ", curptr->desc, curptr->tag);
         if(curptr->tagged) temp[0] = 'þ';
         if (curptr->highest > curptr->lr) temp[63] = '*';
         if (!doredraw  && l== oldcur) /* remove inverse bar */
				printn(l-start+2, 1, cfg.col[Castext], temp, 78);
			if (l==curline)
				{
				printn(l-start+2, 1, cfg.col[Cashigh], temp, 78);    /* Color : 31 */
				highlighted=curptr;
            MoveXY(2, l-start+3); // Position cursor for blind users
				}
			else
				if(doredraw)
               printn(l-start+2, 1, cfg.col[Castext], temp, 78);

         l++;         // Only count now we now it is visible..
			}

      oldcur = curline;

		/* Now check for commands... */

      switch (c = get_idle_key(1, AREASCOPE))
			{
         case cAStag:
            highlighted->tagged =  highlighted->tagged ? 0 : 1;
            if(cfg.mode == SHOWTAGGED || cfg.mode == SHOWNEWTAGGED)
              {
              area_to_start=NULL;
              curline=0;
              goto start;
              }

            if(movedown == 1)
              stuffkey(cASdown);
            else
              stuffkey(cASup);
            break;

         case cASsetview:      // ALT-M
            if(!pickonly && SetView() == 0)
              {
              area_to_start = NULL;
              curline       = 0;
              scannedall    = 0;
              goto start;
              }
            break;

         case cASup:      /* up */
            speedsearch(0, NULL, 0, pickonly);
            movedown=0;
				if (curline)
					{
					curline--;
					if (curline<start)
						start=curline;
					}
				else
					{
					curline=numlines-1;
					start=numlines-rows;
					}
				break;

         case cASdown:      /* down */
            speedsearch(0, NULL, 0, pickonly);
            movedown=1;
				if (curline<numlines-1)
					{
					curline++;
					if (curline>=start+rows)
							start++;
					}
				else
					start=curline=0;
				break;

         case cAShome:      /* home */
            speedsearch(0, NULL, 0, pickonly);
            movedown=1;
				curline=start=0;
				break;

         case cASend:      /* end */
            speedsearch(0, NULL, 0, pickonly);
            movedown=0;
				curline=numlines-1;
				start=numlines-rows;
				break;

         case cASpageup:      /* page up */
            speedsearch(0, NULL, 0, pickonly);
            movedown=0;
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

         case cASpagedown:      /* page down */
            speedsearch(0, NULL, 0, pickonly);
            movedown=1;
            if (start == numlines-rows)
               {
               curline = numlines-1;
               break;
               }

            start = start + rows;

            if (start > numlines-rows)
               start = numlines-rows;

            if (curline < start)
               curline = start;

				break;

         case cASshell:
            shell_to_DOS();
            break;

         case cASscanthorough:   // Thorough scan
            oldstart = -1;
            ScanAreas(first, 0, 0);
            if(cfg.mode == SHOWNEW || cfg.mode == SHOWNEWTAGGED)
              {
              area_to_start=NULL;
              curline=0;
              goto start;
              }
            break;

         case cASscan:      /* ALT-S */
            oldstart = -1; /* Force entire redraw of screen to update * signs */
				ScanAreas(first, 0, 1);
				break;

         case cASscanpersonal:    /* ALT - P */
            if(TIMREGISTERED)
              {
              oldstart = -1; /* Force entire redraw of screen to update * signs */
	           if(!pickonly) ScanAreas(first, 1, 0);
              }
            else
              Message("Personal mailscan only available in registered version.", -1, 0, YES);

            goto start;

         case cAStagsetwrite:         // ALT - W - write tagset
            SaveTags();
            break;

         case cAStagsetread:         // ALT - R  - read tagset
            LoadTags();
            scannedall=0;
            if(cfg.mode == SHOWTAGGED || cfg.mode == SHOWNEWTAGGED)
              {
              area_to_start=NULL;
              curline=0;
              goto start;
              }
            else
              oldstart = -1;
            break;

         case cASexit:    /* ALT - X */
            if (!pickonly)
               {
               if(
                 (!(cfg.usr.status & CONFIRMEXIT)) ||
                 (confirm("Sure you want to leave timEd? (y/N)"))
                 )
                   return NULL;
               }
            break;

         case cASlist:    /* ALT - L */
            stuffkey(cREADlist);
            return highlighted;

         case cASenter:    /* <CR> */
            if(pickonly)
              {
              curline = restore_curline;
              start   = restore_start;
              }
				return highlighted;

         case cASquit:              /* <ESC> */
            if(pickonly)
               {
               curline = restore_curline;
               start   = restore_start;
				   return NULL;
               }
            break;

         case cAShelp:            /* F1 */

            show_help(0);
            break;


         default:
            if ( ((c>31) || (c==8) || (c==12)) && (c<256) )
               {
               if ( (result = speedsearch(c,
                                          c==12 ? highlighted->next  : first,
                                          c==12 ? curline+1 : 0,
                                          pickonly)) != -1 )
                  {
                  curline = start = result;

                  if (start > numlines-rows)
                     start = numlines-rows;
                  }
               }
            break;

			}				/* switch */

	}			/* while(1) */

}


/* ------------------------------------------------ */


void ScanAreas(AREA *first, int personal, int unscanned)
{

	MSG 	*areahandle;
	AREA	*curarea;
	BOX	*msgbox;
	char  temp[81];
   int   stop = 0;
   HMB_PERSINDEX *this;
   char what;

   if(scannedall == 1 && unscanned)
     return;

   if(!unscanned && !personal)
     {
     scannedall=0;
     for (curarea = first; curarea; curarea=curarea->next)
        {
        curarea->scanned=0;
        curarea->lr=curarea->highest=-1;
        }
     }

	msgbox=initbox(11,9,15,69,cfg.col[Cpopframe],cfg.col[Cpoptext],S_HOR,YES,' ');
	drawbox(msgbox);
   titlewin(msgbox, TLEFT, " Scanning.. ", 0);

   #ifndef __NOHMB__
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
   #endif  // NOHMB

   if(personal && (extracheck() != 0) )      // Hackers!!
     {
     cfg.usr.name[0].hash = cfg.usr.name[0].crc = time(NULL);
     }


	for (curarea = first; curarea; curarea=curarea->next)
		{
      if(unscanned && curarea->scanned)
         continue;

      if( (!(curarea->tagged)) &&
          ((cfg.mode == SHOWTAGGED) ||
          (cfg.mode == SHOWNEWTAGGED)) )
          continue;

      if( xkbhit() && (get_idle_key(1, GLOBALSCOPE) == 27) )    /* User hit ESC */
         break;

      if( (curarea->base & MSGTYPE_HMB) && !personal )
         continue;

		sprintf(temp, " Scanning area: %-43.43s", curarea->desc);
		boxwrite(msgbox,1,0,temp);

      if ( !(curarea->base & (MSGTYPE_SQUISH | MSGTYPE_HMB)) )
         {
         if( (curarea->base & MSGTYPE_JAM) && !personal && !BePrivate(curarea))
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

         ScanArea(curarea, areahandle, 1);

		   if (personal)
            {
            if(curarea->base & MSGTYPE_SDM)
               stop = PersMsg(areahandle, curarea);
            else
               stop = JAMpersonal(areahandle, curarea);
            }

         MsgCloseArea(areahandle);

         if((dword) curarea->lr == (dword) -1L) curarea->lr = 0;
         if((dword) curarea->lr == (dword) -2L) curarea->lr = 0;
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

   if(curarea == NULL)              // We've scanned them ALL.
      scannedall = 1;

   #ifndef __NOHMB__
   if(cfg.usr.hudsonpath[0] != '\0')
      {
      if(personal)
         {
         this = firstHMB;
         while(this)
            {
            firstHMB = this->next;
            mem_free(this);
            this = firstHMB;
            }
         firstHMB = NULL;   // I think it's redundant, but what the heck...
         }
      }
    #endif

}



int PersMsg(MSG *areahandle, AREA *curarea)
{
	long  curno = curarea->lr-1;
   int   command = NEXT, l, readresult;
	char	temp[81];
	MSGH	*msghandle;
   MIS   *mis;
	BOX	*persbox;

	persbox=initbox(17,5,19,73,112,112,SINGLE,YES,' ');
	drawbox(persbox);

   if(curno < 1) curno = 1;   // Get a good start.

	while(curno++ < MsgGetHighMsg(areahandle))
		{
      if( xkbhit() && (get_idle_key(1, GLOBALSCOPE) == 27) )    /* User hit ESC */
         {
         command = ESC;
         break;
         }

		if ((msghandle=MsgOpenMsg(areahandle, MOPEN_READ, curno))==NULL)
			continue;	/* open failed, try next message */

      mis = mem_calloc(1, sizeof(MIS));

      readresult = MsgReadMsg(msghandle, mis, 0L, 0L, NULL, 0L, NULL);
      MsgCloseMsg(msghandle);

		if ( readresult != -1 )
			{
         sprintf(temp, "%li %-36.36s ", curno, mis->to);
			boxwrite(persbox,0,0,temp);

         for(l=0; ((cfg.usr.name[l].name[0] != '\0') && (l<tMAXNAMES)); l++)
           {
           if(strcmpi(mis->to, cfg.usr.name[l].name)==0)
				  {
              if(!((cfg.usr.status & SKIPRECEIVED) && (mis->attr1 & aREAD)))
                   command = show_thismsg(areahandle, curarea, curno);
              break;
              }
           }
         }

      FreeMIS(mis);
      mem_free(mis);

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


int speedsearch(char c, AREA *first, int teller, int pickonly)

{
   static char speedtag[20];
   AREA *current = first;
   int oldteller = teller;

   if (c == 0)   /* clear the string */
      {
      memset(speedtag, '\0', 20);
      print(1,42,cfg.col[Casframe],"ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");
      return -1;
      }

   if(c == 8)     /* Backspace */
      {
      if(strlen(speedtag))
         speedtag[strlen(speedtag)-1] = '\0';
      }
   else if( (c != 12) && (strlen(speedtag) < 19) )   /* 20 = maxlen */
      {
      speedtag[strlen(speedtag)+1] = '\0';
      speedtag[strlen(speedtag)] = c;
      }

   vprint(1,42,cfg.col[Casframe], "[%-19.19s]", speedtag);

   for( ; current; current = current->next)
        {
        if(pickonly || AreaVisible(current))
          {
          if(strncmpi(current->tag, speedtag, strlen(speedtag)) == 0 )
            return teller;
          teller++;
          }
        }

   for(teller=oldteller,current=first; current; current = current->next)
        {
        if(pickonly || AreaVisible(current))
          {
          if ( stristr(current->tag, speedtag) != 0 )
             return teller;
          teller++;
          }
        }

   return -1;

}



int fastscan(AREA *curarea, int personal)
{
   int ret = NEXT;
   struct my_idx *l;
   long current=0, lastuid=0;
   PERSMSG *firstmsg=NULL, *lastmsg=NULL, *curmsg;
   MSG *areahandle;
   int loop;
   int index;
   dword last;
   char idxname[100], lastname[100];
   char *idxbuf;
   #define CHUNKSIZE 100
   unsigned chunksize = CHUNKSIZE * sizeof(struct my_idx);
   int got;
   int iterations;

   curarea->highest = 0L;
	curarea->lowest  = 0L;
	curarea->lr      = 0L;
	curarea->nomsgs  = 0L;
	curarea->scanned = 1;

   last = (dword) get_last_squish(curarea); // Get lastread UID

   if( (idxbuf = malloc(chunksize)) == NULL )
     return 0;

   sprintf(idxname,  "%s.SQI", curarea->dir);
   sprintf(lastname, "%s.SQL", curarea->dir);

   if ( (index = sopen(idxname, O_RDONLY | O_BINARY, SH_DENYNO)) == -1)
      {
      mem_free(idxbuf);
      return 0;
      }

   while( (got=read(index, idxbuf, chunksize)) > 0 )
     {
     iterations = got / sizeof(struct my_idx);
     l = (struct my_idx *)idxbuf;

     while(iterations)
       {
       if(l->umsgid == -1L || l->umsgid <= lastuid)
         break;

       current++;

       lastuid = l->umsgid;

       if(l->umsgid <= last)
         curarea->lr = current;
       else if(personal)
         {
         for(loop=0; cfg.usr.name[loop].name[0] != '\0' && loop < tMAXNAMES; loop++)
           {
           if(
               (l->hash == cfg.usr.name[loop].hash) ||
               (l->hash == (cfg.usr.name[loop].hash | 0x80000000Lu) )
             )
              {
              curmsg = mem_calloc(1, sizeof(PERSMSG));
              curmsg->uid = l->umsgid;
              if(!firstmsg)
                 firstmsg = curmsg;
              else lastmsg->next = curmsg;
              lastmsg = curmsg;
              }
           }
         }
       iterations--;
       l++;
	    }
     }

   curarea->highest = current;
   curarea->lowest = 1;
   curarea->nomsgs = current;

   mem_free(idxbuf);
   close(index);

   /* Now let's check if any msgs were found, display them, and mem_free */
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

      while(firstmsg)      /* mem_free mem again */
        {
        curmsg = firstmsg;
        firstmsg = firstmsg->next;
        mem_free(curmsg);
        }

      MsgCloseArea(areahandle);

      }

   return (ret == ESC) ? 1 : 0;   /* Ret == 1 means stop scan */

}


int show_thismsg(MSG *areahandle, AREA *area, UMSGID uid)
{
   dword curno, command;
   MMSG *persmsg;


   if((curno = MsgUidToMsgn(areahandle, uid, UID_EXACT)) == 0)
      return 0;

   get_custom_info(area);

	savescreen();

   command=0;

   while( (command != ESC)  &&
          (command != NEXT) &&
          (command <= 0) )
          {
          if( (persmsg = GetFmtMsg(curno, areahandle, area)) == NULL)
             {
             sprintf(msg, "Error reading msg #%d in %s", (int) curno, area->tag);
             Message(msg, -1, 0, YES);
             putscreen();
             return NEXT;
             }
          kbflush();

          if(
              (persmsg->status & PERSONAL) &&
              (!((cfg.usr.status & SKIPRECEIVED) && (persmsg->mis.attr1 & aREAD)))
            )
             {
             clsw(cfg.col[Cmsgtext]);
             if(area->mlist == NULL)
                area->mlist = InitMarkList();

			    command = ShowMsg(persmsg, area, areahandle, SCAN_DISPLAY);

             if(area->mlist->active == 0)
               {
               DumpMarkList(area->mlist);
               DumpMarkList(area->mayseelist);
               area->mlist = NULL;
               area->mayseelist = NULL;
               }
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
   long len=0;


   if(curarea->lr >= curarea->highest)
      {
      curarea->lr = curarea->highest;
      return 0;
      }

   if(curarea->lr < 0)
      curarea->lr = 0;


   indexhandle = ((JAMDATA *)areahandle->apidata)->IdxHandle;

   if(indexhandle < 1)
     {
     sprintf(msg, "Indexhandle: %d", indexhandle);
     Message(msg, -1, 0, YES);
     }

   if( (len=filelength(indexhandle)) == 0 )
      return 0;

   current = (long) curarea->lr - (long) ((JAMDATA *)areahandle->apidata)->HdrInfo.BaseMsgNum;
   if(current < 0)
     current = 0;

   base = current;

   idxmem = mem_malloc(2000L * sizeof(JAMIDXREC));
   l = (JAMIDXREC *)idxmem;

   do
      {
      if(((long) current * (long) sizeof(JAMIDXREC)) >= len)
         {
         sprintf(msg, "Seek too high - go away! (seek: %ld - len: %ld)", (long) current * (long) sizeof(JAMIDXREC), len);
         Message(msg, -1, 0, YES);

         sprintf(msg, "Indexhandle: %d", indexhandle);
         Message(msg, -1, 0, YES);
         bytesread = 0;
         break;
         }

      if (lseek(indexhandle,
               (long) ((long) current * (long) sizeof(JAMIDXREC)),
               SEEK_SET) == -1)
          {
          sprintf(msg, "Error seeking JAM index (%s, errno %d)!", curarea->tag, errno);
          break;
          }

      if((bytesread=read(indexhandle, idxmem, 2000L * sizeof(JAMIDXREC))) == -1)
        {
        sprintf(msg, "Error reading JAM index (%s, errno: %d)!", curarea->tag, errno);
        Message(msg, -1, 0, YES);
        break;
        }

      total = (long) (bytesread/ sizeof(JAMIDXREC));

      l = (JAMIDXREC *)idxmem;

      // Checked: check elements this read; current: keeps increasing (realno)
      for(checked=0; checked < total; checked++, current++, l++)
        {
        for(loop=0; cfg.usr.name[loop].name[0] != '\0' && loop<tMAXNAMES; loop++)
          {
          if(l->UserCRC == cfg.usr.name[loop].crc)
             {
             if(MsgUidToMsgn(areahandle, current + ((JAMDATA *)areahandle->apidata)->HdrInfo.BaseMsgNum, UID_EXACT) != 0)
                {
                 curmsg = mem_calloc(1, sizeof(PERSMSG));
                 curmsg->uid = current + ((JAMDATA *)areahandle->apidata)->HdrInfo.BaseMsgNum;
                 if(!firstmsg)
                     firstmsg = curmsg;
                 else lastmsg->next = curmsg;
                 lastmsg = curmsg;
                 }
             }
           }
        }
      } while ( bytesread == (2000 * sizeof(JAMIDXREC)) );   // Do loop..

   mem_free(idxmem);

   /* Now let's check if any msgs were found, display them, and mem_free */
   /* the memory taken up by the list op personal msgs               */


   if(firstmsg)                  /* personal msgs found */
      {
      for(curmsg = firstmsg; curmsg && (ret != ESC); curmsg = curmsg->next)
         {
         if( (ret = show_thismsg(areahandle, curarea, curmsg->uid)) == ESC)
             break;
         }

      while(firstmsg)      /* mem_free mem again */
        {
        curmsg = firstmsg;
        firstmsg = firstmsg->next;
        mem_free(curmsg);
        }
      }

   return (ret == ESC) ? 1 : 0;   /* Ret == 1 means stop scan */

}


/* Try to do a quick scan on a JAM base. Return 0 if we failed, 1 = success */


int quickscan(AREA *area)
{
   int jhr, jlr;
   char jlrname[120], jhrname[120], jdxname[120];
   JAMINFO jaminfo;
   struct stat statinfo;
   JAMLREAD lread;

   area->highest = 0L;
	area->lowest  = 0L;
	area->lr      = 0L;
	area->nomsgs  = 0L;
	area->scanned = 1;

   sprintf(jhrname, "%s.jhr", area->dir);
   if( (jhr=sopen(jhrname, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
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
   if( (jlr=sopen(jlrname, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
     return 1;

   while(read(jlr, &lread, sizeof(JAMLREAD)) == sizeof(JAMLREAD))
     {
     if(lread.UserCRC == cfg.usr.name[0].crc)
        {
        area->lr = (cfg.usr.status & JAMGETLAST) ? lread.LastReadMsg : lread.HighReadMsg;
        if(area->lr > area->highest)
           area->lr = area->highest;
        if(area->lr < area->lowest)
           area->lr = area->lowest-1;
        break;
        }
     }

   close(jlr);

   return 1;
}

#ifndef __NOHMB__

void analyse_toidx(void)
{
   int toidx, idx, l, bytes, howmuch;
   char temp[120];
   HMB_MSGTOIDX *block;
   unsigned int total = 0;
   #define BATCHSIZE 300

   sprintf(temp, "%s\\msgidx.bbs", cfg.usr.hudsonpath);

   if( (idx = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
      return;

   sprintf(temp, "%s\\msgtoidx.bbs", cfg.usr.hudsonpath);

   if( (toidx = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
      {
      close(idx);
      return;
      }

   block = mem_calloc(BATCHSIZE, sizeof(HMB_MSGTOIDX));

   while((bytes=read(toidx, block, BATCHSIZE * sizeof(HMB_MSGTOIDX))) == (BATCHSIZE * sizeof(HMB_MSGTOIDX)))
      {
      for(l=0; l<BATCHSIZE; l++)
          check_toidx(&block[l], total+l, idx);
      total += BATCHSIZE;
      }

   howmuch = bytes / sizeof(HMB_MSGTOIDX);
   for(l=0; l<howmuch; l++)
          check_toidx(&block[l], total+l, idx);

   mem_free(block);
   close(toidx);
   close(idx);
}


void check_toidx(HMB_MSGTOIDX *thisentry, unsigned recno, int idx)
{
   int l;
   HMB_PERSINDEX *thisone;
   static HMB_PERSINDEX *last = NULL;


   for(l=0; ((cfg.usr.name[l].name[0] != '\0') && (l<tMAXNAMES)); l++)
     {
     if(strncmpi((char *)thisentry+1, cfg.usr.name[l].name, strlen(cfg.usr.name[l].name))==0)
		  {
        thisone = mem_calloc(1, sizeof(HMB_PERSINDEX));
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
     Message("HMB index not found!", -1, 0, YES);
     return;
     }

   total = (unsigned)  (mystat.st_size / (long) sizeof(HMB_MSGIDX));

   if(total == 0)
      return;

   HMBreadlast();  // If error, all lr's on 0.. (fuck them!)

   HMBareas = mem_calloc(sizeof(HMBAREAS), 200);

   if( (thismsg = HMBReadEntireIndex()) == NULL )
     {
     Message("Error reading HMB index!", -1, 0, YES);
     mem_free(HMBareas);
     HMBareas=NULL;
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
        sprintf(msg, "Illegal Hudson board number %d (msgnum: %d, recno: %d)!", board+1, thismsg->msg_num, n+1);
        Message(msg, -1, 0, YES);
        continue;
        }

     if(board == 13)
       {
       debugtest++;
       }

     HMBareas[board].high += 1;   // Update no of msgs in this area

     if(HMBareas[board].last == 0)  // Looking for lastread info?
       {
       if(HMBlr.lastread[board] == 0)
          ;       /* Nothing, leave it at 0! */

       else if(thismsg->msg_num == HMBlr.lastread[board])
          HMBareas[board].last = HMBareas[board].high;

       else if(thismsg->msg_num < HMBlr.lastread[board])
          HMBareas[board].lower = HMBareas[board].high;

       else // Higher
          {
          if(HMBareas[board].lower != 0)
             HMBareas[board].last = HMBareas[board].lower;
          else
             {
             HMBareas[board].last = HMBareas[board].high-1;
             if((HMBareas[board].last < 0   ) ||
                (HMBareas[board].last == -1L) )
                  HMBareas[board].last = 0;
             }
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
     thisarea->lr      = HMBareas[board].last ? HMBareas[board].last : HMBareas[board].lower;
     // Safe?
     if(thisarea->lr < 0) thisarea->lr = 0;
     }

     mem_free(HMBareas);
     if(base) mem_free(base);
}

#endif   // NOHMB

// Extra check on regkey. Returns 0 if all is well.

int extracheck(void)
{
   dword ascvals;
   char *charptr;
   char name[40];

   #ifndef __SENTER__

     strcpy(name, cfg.usr.name[0].name);
     strlwr(name);

     ascvals=0L;

     for(charptr=name; *charptr; charptr++)
       ascvals += (unsigned char) (*charptr + 27);

     if(cfg.key.strkey.ascvals != ascvals)
       return 1;

  #endif

  return 0;

}


int AreaVisible(AREA *curptr)
{
   int visible=0;

   switch(cfg.mode)
     {
     case SHOWALL:
        visible=1;
        break;

     case SHOWNEW:
        if(curptr->highest > curptr->lr)
          visible=1;
        break;

     case SHOWTAGGED:
        if(curptr->tagged == 1)
          visible=1;
        break;

     case SHOWNEWTAGGED:
        if( (curptr->highest > curptr->lr) && (curptr->tagged == 1) )
          visible=1;
        break;
     }

   return visible;

}

// Pick the view on the area database

int SetView(void)
{
   int choice;

   choice = picklist(views, NULL, maxy/2-2, 26, maxy, maxx);

   if(choice == -1)
     return -1;

   cfg.mode = choice;
   return 0;
}


