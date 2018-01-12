#include <string.h>
#include <alloc.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>

#include "xmalloc.h"
#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "list.h"
#include "readarea.h"
#include "global.h"
#include <video.h>
#include <scrnutil.h>
#include "message.h"
#include "copymsg.h"
#include "select.h"
#include "print.h"
#include "getmsg.h"
#include "input.h"
#include "idlekey.h"
#include "shel2dos.h"
#include "help.h"

#define TAGGED    0x01
#define PERSMSG   0x02

#define TAG    0x01
#define UNTAG  0x02


#ifdef __OS2__
unsigned long coreleft(void);
#endif

/* Internal structure for API_SDM, abused here to get info... */

struct _sdmdata
{
  byte base[80];

  unsigned *msgnum;   /* has to be of type 'int' for qksort() fn */
  word msgnum_len;

  dword hwm;
  word hwm_chgd;

  word msgs_open;
};

/* Info is stored in a "index" of the structures below */

dword *uidlist;
char  *statuslist;
char  **linelist;

static char wide_display = 0;

int oldstart = -1, oldcur = -1;

int   fillstruct  (MSG *areahandle, AREA *area, dword current);
void  readheader  (unsigned int num, MSG *areahandle, AREA *area);
void  checkfilled (int start, int rows, int maxpos, MSG *areahandle, AREA *area);
void  showthem    (int start, int rows, int curpos);
void  free_all    (int howmany, char basetoo);
void  makefree    (int start, int rows, int maxpos);
void  print_msg   (MSG *areahandle, AREA *area, long msgno, FILE *outfile);
dword process_msgs(MSG *areahandle, AREA *area, int curpos, int maxpos, int operation);
void  get_range   (char dowhat, int maxpos, AREA *area);
void tagall       (char dowhat, int maxpos);

dword MsgList(MSG *areahandle, AREA *area, dword current, char wide)
{
   dword   lastmsg;
   BOX     *listbox;
   int     maxlines=maxy, rows=maxlines-2;
   int     start = 0, curpos = 0, maxpos;

   wide_display = wide;   /* 0 == normal, 1 == wide subject */

   start:

   oldstart = oldcur = -1;

   lastmsg = MsgMsgnToUid(areahandle, current); /* Set mark to return to.. */
   maxpos  = (int) MsgGetNumMsg(areahandle)-1;

   curpos = fillstruct(areahandle, area, lastmsg);     /* Make "index" to use */

   if(maxpos < rows-1)
      {
      rows = maxpos+1;
      start = 0;
      }
   else
      {
      start = curpos - (rows/2);
      if(start < 0) start = 0;
      if(start+rows > maxpos) start = maxpos - rows + 1;
      }

   listbox = initbox(0,0,rows+1,79,cfg.col.asframe,cfg.col.astext,SINGLE,NO,-1);
   clsw(cfg.col.astext);

   drawbox(listbox);
   delbox(listbox);

   while(curpos != -1)
      {
      checkfilled(start, rows, maxpos, areahandle, area);
      showthem(start, rows, curpos);

      switch(get_idle_key(1))
         {
         case 287: /* ALT - S */
         case 115: /* S       */
         case 83:

              oldstart = -1;
              current = MsgUidToMsgn(areahandle, uidlist[curpos], UID_EXACT);
              free_all(maxpos, 1);
              wide_display = (wide_display == 0) ? 1 : 0;

              goto start;

         case 273:    /* ALT - W */
         case 119:    /* W       */
         case 87:

              oldstart = -1;
              if ( (current = process_msgs(areahandle, area, curpos, maxpos, PRINT)) == 0)
                 return 0;
              else if (current == -1) break;

              goto start;

         case 281:    /* ALT - P */
         case 112:    /* P       */
         case 80:

              oldstart = -1;
              if ( (current = process_msgs(areahandle, area, curpos, maxpos, HARDCOPY)) == 0)
                 return 0;
              else if (current == -1) break;

              goto start;


         case 288:    /* ALT - D */
         case 100:    /* D       */
         case 68:
         case 339:    /* <Del> */

              oldstart = -1;
              if ( (current = process_msgs(areahandle, area, curpos, maxpos, KILL)) == 0)
                 return 0;
              else if (current == -1) break;

              goto start;

         case 302:    /* ALT - C */
         case 99:     /* C       */
         case 67:

              oldstart = -1;
              if ( (current = process_msgs(areahandle, area, curpos, maxpos, COPY)) == 0)
                 return 0;
              else if (current == -1) break;

              goto start;

         case 306:    /* ALT - M */
         case 109:    /* M       */
         case 77:

              oldstart = -1;
              if ( (current = process_msgs(areahandle, area, curpos, maxpos, MOVE)) == 0)
                 return 0;
              else if (current == -1) break;

              goto start;

         case 315:       /* F1 */

              show_help(2);
              break;

         case 328:      /* up */

            if (curpos) curpos--;
            if (curpos < start) start--;

            break;

         case 336:      /* down */

            if (curpos < maxpos) curpos++;
            if (curpos > start + rows-1)
               start++;
            if(start+rows > maxpos) start = maxpos-rows+1;

            break;

         case 327:      /* home */

            curpos=0;
            start=0;

            break;

         case 335:      /* end */

            curpos = maxpos;
            start = maxpos - rows + 1;

            break;

         case 329:      /* page up */

            curpos = start = start - rows;
            if (curpos < 0)
               curpos = start = 0;

            break;

         case 337:      /* page down */

            curpos = start = start + rows;

            if (curpos > maxpos)
               curpos = maxpos;
            if(start+rows > maxpos) start = maxpos-rows+1;

            break;


         case 27:

            curpos = -1;
            break;

         case 292:   /* ALT - J */
         case 106:   /* J       */
         case 74:

            shell_to_DOS();
            break;

         case 13:    /* <CR>        */
         case 333:   /* Right arrow */

            lastmsg = uidlist[curpos];
            curpos = -1;
            break;

         case 32:    /* Space */

            statuslist[curpos] ^= TAGGED;
            if (curpos < maxpos) curpos++;
            if (curpos > start + rows-1)
               start++;
            if(start+rows > maxpos) start = maxpos-rows+1;

            break;

         case 43:    /* + */

              oldstart = -1;
              get_range(TAG, maxpos, area);
              break;

         case 334:
         case 387:

              oldstart = -1;
              tagall(TAG, maxpos);
              break;

         case 330:
         case 386:

              oldstart = -1;
              tagall(UNTAG, maxpos);
              break;

         case 45:    /* - */

              oldstart = -1;
              get_range(UNTAG, maxpos, area);
              break;

         }           /* switch */

      }



   free_all(maxpos, 1);


   if ((current = MsgUidToMsgn(areahandle, lastmsg, UID_NEXT)) == 0)
               current = MsgUidToMsgn(areahandle, lastmsg, UID_PREV);

   return current;

}


/* ------------------------------------------------- */


void readheader(unsigned num, MSG *areahandle, AREA *area)

{
   MSGH *msghandle;
   XMSG header;
   dword no;
   int l;

   no = (!((area->base & MSGTYPE_HMB) || (area->base & MSGTYPE_SQUISH))) ? uidlist[num] : num + 1;
   linelist[num] = xmalloc(81);

   if ((msghandle = MsgOpenMsg(areahandle, MOPEN_RDHDR, no)) == NULL)
      {
      strcpy (linelist[num], "           ** Error reading message! **                                      ");
      }
   else
      {
      if(MsgReadMsg(msghandle, &header, 0L, 0L, NULL, 0L, NULL) == -1)
         Message("Error reading message!", -1, 0, 1);

      if(MsgCloseMsg(msghandle) == -1)
         Message("Error closing message!", -1, 0, 1);

      if(wide_display)
         sprintf(linelist[num], "   %4ld %-20.20s %-48.48s ", no, header.from, header.subj);
      else
         sprintf(linelist[num], "   %4ld %-20.20s %-20.20s %-27.27s ", no, header.from, header.to, header.subj);


      for(l=0; (cfg.usr.name[l].name[0] != '\0') && (l<10); l++)
        {
        if(strcmpi(header.to, cfg.usr.name[l].name)==0)
           {
           statuslist[num] |= PERSMSG;
           break;
           }
        }
      }

}


/* Free memory taken by headers and "index" */


void free_all(int howmany, char basetoo)
{
   int l;

   for(l=0; l <= howmany; l++)
     {
     if(linelist[l])
          {
          free(linelist[l]);
          linelist[l] = NULL;
          }
     }

   if (basetoo)
      {
      if (uidlist) free(uidlist);
      if (statuslist) free(statuslist);
      if (linelist) free(linelist);
      }
}



/* Fill the "index" of msgs, w/ pointers to 'header string' to be displayed */



int fillstruct(MSG *areahandle, AREA *area, dword curmsgno)
{
   struct _sdmdata *dataptr  = (struct _sdmdata *)areahandle->apidata;
   unsigned        *msglijst = (unsigned *)dataptr->msgnum;
   int l, curpos=0;
   unsigned realno;
   BOX *wait;

   uidlist    = xcalloc((size_t) MsgGetNumMsg(areahandle)+1, sizeof(UMSGID));
   statuslist = xcalloc((size_t) MsgGetNumMsg(areahandle)+1, sizeof(char));
   linelist   = xcalloc((size_t) MsgGetNumMsg(areahandle)+1, sizeof(char *));

   if(area->base & MSGTYPE_SQUISH) MsgLock(areahandle);

   if(!(area->base & MSGTYPE_JAM))
     {
     for(l=0; l < MsgGetNumMsg(areahandle); l++)
        {
        uidlist[l] = (area->base & MSGTYPE_SDM) ? *msglijst++ : MsgMsgnToUid(areahandle, l+1);
        if (uidlist[l] == curmsgno)
           curpos = l;
        }
     }
   else if(area->base & MSGTYPE_JAM)
     {
     wait = initbox(13, 35, 15, 45, cfg.col.popframe, cfg.col.poptext, SINGLE, YES, ' ');
     drawbox(wait);
     boxwrite(wait,0,1,"Wait...");

     realno = (unsigned) MsgGetLowMsg(areahandle);  // Why cast???????

     /* 'contigious' msg numbers? So no deleted msgs in area? */
     /* Cause then it's very easy. Otherwise we have to check */
     /* which message number actually exist :-(               */

     if(MsgGetNumMsg(areahandle) == (MsgGetHighMsg(areahandle) - realno + 1))
       {
       for(l=0; l < MsgGetNumMsg(areahandle); l++, realno++)
          {
          uidlist[l] = realno;
          if (uidlist[l] == curmsgno)
             curpos = l;
          }
       }
     else
       {
       for(l=0; l < MsgGetNumMsg(areahandle), realno <= MsgGetHighMsg(areahandle); l++)
          {
          while( (MsgUidToMsgn(areahandle, realno, UID_EXACT) == 0) &&
               realno <= MsgGetHighMsg(areahandle) )
             realno++;
          uidlist[l] = realno;
          if (uidlist[l] == curmsgno)
             curpos = l;
          realno++;
          }
       }

     delbox(wait);
     }

   if(area->base & MSGTYPE_SQUISH) MsgUnlock(areahandle);

   return curpos;
}



/* Walk through msgs to be displayed, if header not read yet, do it now! */


void checkfilled(int start, int rows, int maxpos, MSG *areahandle, AREA *area)
{

   unsigned int l, n;
   char didlock = 0;
   int howmany=0;

   #ifndef __OS2__

   if(coreleft() < (unsigned long) 30000)        /* Check for enough free mem */
           free_all(maxpos, 0);

   #endif

   for(n = start, l=0; l < rows; l++, n++)
      {
      if(!(linelist[n]))
        {
        if(++howmany > 2)
           break;
        }
      }


   for(n = start, l=0; l < rows; l++, n++)
      {
      if(!(linelist[n]))
        {
        if( (!didlock) && (howmany>2)  )   /* Lock base for extra speed, we probably have to read multiple headers */
           {
           if(area->base & MSGTYPE_SQUISH)
              MsgLock(areahandle);
           didlock = 1;
           print(0,29,cfg.col.asframe,"[Scanning..]");
           }
        readheader(n, areahandle, area);
        }
      }

   if (didlock)
      {
      if(area->base & MSGTYPE_SQUISH)
         MsgUnlock(areahandle);
      print(0,29,cfg.col.asframe,"컴컴컴컴컴컴");
      }
}



/* Display the headers on the screen */


void showthem(int start, int rows, int curpos)
{
   unsigned int l, colour;
   char temp[40];
   int doredraw;


   doredraw = (start == oldstart) ? 0 : 1;
   oldstart = start;

   for(l=start; l < start+rows; l++)
      {
      if(curpos == l)
        {
        colour = cfg.col.ashigh;
        MoveXY(2, l-start+2);
        }
      else colour = (statuslist[l] & PERSMSG) ? cfg.col.asspecial : cfg.col.astext;

      if((doredraw) || (l==oldcur) || (curpos==l) )
         printn(l-start+1, 1, colour, linelist[l], 78);

      if (statuslist[l] & TAGGED)
         printc(l-start+1,2,colour,'*');
      }

   oldcur = curpos;

}


/* Kill, Move, Copy, Print all tagged messages */

dword process_msgs(MSG *areahandle, AREA *area, int curpos, int maxpos, int operation)

{
   int loop, counter=0;
   dword marker, current;
   AREA *toarea;
   MSG *toareahandle;
   BOX *progress;
   char temp[80], *buf;

   FILE *outfile = NULL;                            /* For PRINT */

   if ((operation == KILL) && (cfg.usr.status & CONFIRMDELETE))
        {
        if( !confirm("Really delete all tagged messages? (y/N)") )
            return -1L;
        }

   marker = uidlist[curpos];

   free_all(maxpos, 0);       /* Free up some mem first */

   if ( (operation == MOVE) || (operation == COPY) )
      {
      savescreen();
      toarea = SelectArea(cfg.first, 1, cfg.first);
      putscreen();
      if(toarea == NULL)
         {
         return -1L;
         }

      if(toarea == area)
          toareahandle = areahandle;   /* same area? */
      else
         {
         if((toareahandle=MsgOpenArea(toarea->dir, MSGAREA_CRIFNEC, toarea->base))==NULL)
            {
            sprintf(temp, "Can't open area (%30s)!", toarea->dir);
            Message(temp, -1, 0, YES);
            return -1L;
            }

         while(1)
            {
            if( MsgLock(toareahandle) == -1 )
               {
               Message("Can't lock area! Wait for retry, <ESC> to abort", 10, 0, YES);
               if( xkbhit() && (get_idle_key(1) == 27) )
                  {
                  MsgCloseArea(toareahandle);
                  return -1L;
                  }
               }
            else break;
            }

         }
      }
   else if ((operation == PRINT) || (operation == HARDCOPY))
      {
      inst_newcrit();
      if(operation == PRINT)
         {
         if ( (outfile = OpenExist(cfg.usr.writename)) == NULL)
            {
            inst_oldcrit();
            return 0;
            }
         }
      else
         {
         if( (outfile = fopen(cfg.usr.printer, "at")) == NULL)
            {
            inst_oldcrit();
            return 0;
            }
         }

      buf = xmalloc(4096);
      setvbuf(outfile, buf, _IOFBF, 4096);
      }


   progress = initbox(12,25,14,55,cfg.col.popframe,cfg.col.poptext,SINGLE,NO,' ');
   drawbox(progress);
   delbox(progress);

   MsgLock(areahandle);

   for(loop=0; loop <= maxpos; loop++)
      {
      if(statuslist[loop] & TAGGED)
         {
         sprintf(temp, "Processing msg# %d", ++counter);
         print(13,27,cfg.col.poptext,temp);

         current = MsgUidToMsgn(areahandle, uidlist[loop], UID_EXACT);

         if(xkbhit())
           {
           if(get_idle_key(1) == 27)
              break;
           }

         if (operation == KILL)
            {
            if(MsgKillMsg(areahandle, current) == -1)
               Message("Error deleting message!", -1, 0, YES);
            }
         else if ( (operation == COPY) || (operation == MOVE) )
            CopyMsg(areahandle, toareahandle, toarea, current, (operation == MOVE));
         else if ((operation == PRINT) || (operation==HARDCOPY))
              print_msg(areahandle, area, current, outfile);


         }  /* if */

      }  /* for */

   MsgUnlock(areahandle);

   if ( (operation == MOVE) || (operation == COPY) )
      {
      ScanArea(toarea, toareahandle);
      if (area != toarea)
         {
         MsgUnlock(toareahandle);
         MsgCloseArea(toareahandle);
         }
      }
   else if ((operation == PRINT) || (operation == HARDCOPY))
      {
      if (
            (operation == HARDCOPY) ||
            (strcmpi (cfg.usr.writename, "PRN")    == 0) ||
            (strncmpi(cfg.usr.writename, "LPT", 3) == 0) ||
            (strncmpi(cfg.usr.writename, "COM", 3) == 0)
         )
         fputc(12, outfile);

      fclose(outfile);
      free(buf);
      inst_oldcrit();
      }

   free(uidlist);    uidlist    = NULL;
   free(statuslist); statuslist = NULL;
   free(linelist);   linelist   = NULL;

   ScanArea(area, areahandle);
   if (area->nomsgs == 0)
      return 0;

   if ( (current = MsgUidToMsgn(areahandle, marker, UID_NEXT)) == 0)
      current = MsgUidToMsgn(areahandle, marker, UID_PREV);

   return current;
}




void print_msg(MSG *areahandle, AREA *area, long msgno, FILE *outfile)
{

   MMSG *curmsg;

   curmsg = GetMsg(msgno, areahandle, 1, area);       /* Read msg from disk, formatted */

   if(curmsg)
      {
      do_print(outfile, area, curmsg);
      ReleaseMsg(curmsg, 1);
      }

}


void get_range(char dowhat, int maxpos, AREA *area)
{
   dword n;
   int ret=1, action=0;
   long begin, end;
   char nbegin[6], nend[6];
   BOX *numbox;
   unsigned curmsg;


   memset(nbegin, '\0', sizeof(nbegin));
   memset(nend ,  '\0', sizeof(nend  ));


   numbox = initbox(8,21,15,47,cfg.col.popframe,cfg.col.poptext,DOUBLE,YES,' ');
   drawbox(numbox);
   boxwrite(numbox, 1, 2, "Tag/Untag:");
   boxwrite(numbox, 3, 2, "Starting msg #");
   boxwrite(numbox, 4, 2, "Ending   msg #");

   while(action < 2)
      {
      switch(action)
         {
         case 0:

            while( (ret != ESC) && (ret != 0) )
               ret = getstring(12,40,nbegin,5,"0123456789",cfg.col.entry);
            action = (ret == 0) ? 1 : 2;
            ret=1;
            break;

         case 1:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
               ret = getstring(13,40,nend,5,"0123456789",cfg.col.entry);
            action = ( (ret == UP) || (ret == BTAB) ) ? 0 : 2;
            ret=1;
            break;

         default:
            break;    /* nothing */
         }
      }

   delbox(numbox);

   if (ret == ESC) return;

   begin = atol(nbegin);
   end   = atol(nend  );

   for(curmsg=0, n=1; n <= maxpos+1; curmsg++, n++)
      {
      if( (area->base & MSGTYPE_SDM) || (area->base & MSGTYPE_JAM) )
         {
         if (uidlist[curmsg] > end) break;

         if( (uidlist[curmsg] >= begin) && (uidlist[curmsg] <= end) )
            {
            statuslist[curmsg] = (dowhat == TAG) ? statuslist[curmsg] | TAGGED : (statuslist[curmsg] & 254);
            }
         }
      else
         {
         if (n > end) break;

         if( (n >= begin) && (n <= end) )
            {
            statuslist[curmsg] = (dowhat == TAG) ? statuslist[curmsg] | TAGGED : (statuslist[curmsg] & 254);
            }
         }
      }

}


#ifdef __OS2__
unsigned long coreleft(void)
{
 return 1;
}
#endif


void tagall(char dowhat, int maxpos)
{
   dword n;
   unsigned curmsg;


   for(curmsg=0, n=1; n <= maxpos+1; curmsg++, n++)
      statuslist[curmsg] = (dowhat == TAG) ? statuslist[curmsg] | TAGGED : (statuslist[curmsg] & 254);

}
