#include <stdio.h>
#include <dir.h>
#include <string.h>
#include <conio.h>
#include <alloc.h>
#include "xmalloc.h"
#include <video.h>
#include <scrnutil.h>
#include <msgapi.h>

#include "wrap.h"
#include "tstruct.h"
#include "attach.h"
#include "message.h"
#include "global.h"
#include "idlekey.h"
#include "help.h"
#include "header.h"
#include "reply.h"
#include "tosslog.h"

FILELIST *getfiles(char *filespec);
void     showselect(FILELIST *first);
void     writefa(XMSG *hdr, MSG *areahandle, char *subject, AREA *area);
void     printstats(FILELIST *first);

int ok = 1;

/* -------------------------------------------------- */


int check_attach(XMSG *hdr, MSG *areahandle, AREA *area)

{
   FILELIST *ff=NULL, *cf;
   char     *file, list[72], subject[72], fl[80], *lastpath=NULL;

   ok = 1;

   strcpy(list, hdr->subj);

   file = strtok(list, " ");

   while(file && ok)         /* Do all listed filespecs on subj line */
     {
     if (!ff)                          /* first 'list w/ files'? */
        cf = ff = getfiles(file);
     else                              /* link w/ list made earlier */
        cf->next = getfiles(file);

     if(cf != NULL) while(cf->next)    /* goto end of list, for later link */
        cf = cf->next;

     file = strtok(NULL, " ");
     }

     if (ok)
        {
        /* We do *not* UNlock, the 'real' message has to be written too, */
        /* the MsgLock before THAT (reply.c) will always succeed (when   */
        /* locked already), and the base will be unlocked after that     */

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

        cf = ff;                       /* Goto start of list */
        memset(subject, '\0', sizeof(subject));

        while(cf)
           {
           if (cf->tagged)
              {
              sprintf(fl, "%s%s ", cf->path, cf->name);
              if ( (strlen(subject) + strlen(fl) ) > 71 )
                 {
                 writefa(hdr, areahandle, subject, area);
                 memset(subject, '\0', sizeof(subject));
                 }
              strcat(subject, fl);
              }
           cf = cf->next;
           }

        strcpy(hdr->subj, subject);    /* Rest is put in original msgheader */
        }


     close_up:

     cf = ff;
     while(cf)
       {
       ff = cf->next;
       if (lastpath != cf->path)
          {
          lastpath = cf->path;
          free(cf->path);
          }
       free(cf);
       cf = ff;
       }

     return ok;
}


/* ----------------------------------------------------- */


FILELIST *getfiles(char *filespec)

{
   BOX *specbox;
   struct ffblk ffblk;
   FILELIST *ff=NULL, *cf;
   int done;
   char temp[60], *location, drive[MAXDRIVE], dir[MAXDIR];


   specbox = initbox(0,12,2,68,cfg.col.popframe,cfg.col.poptext,SINGLE,YES,' ');
   drawbox(specbox);
   sprintf(temp, "Current filespec: %-35.35s", filespec);
   boxwrite(specbox, 0, 1, temp);
   fnsplit(filespec, drive, dir, NULL, NULL);
   location = xmalloc(MAXPATH);
   sprintf(location, "%s%s", drive, dir);

   done = findfirst(filespec, &ffblk, 0);
   while (!done)
     {
     if(!ff)
        cf = ff = xmalloc(sizeof(FILELIST));
     else
        {
        cf->next = xmalloc(sizeof(FILELIST));
        cf = cf->next;
        }

     strncpy(cf->name, ffblk.ff_name,29);
     cf->size = ffblk.ff_fsize;
     cf->tagged = 0;
     cf->path = location;
     cf->next = NULL;

     done = findnext(&ffblk);
     }

   showselect(ff);

   delbox(specbox);

   return ff;

}


/* ------------------------------------------------------ */


void showselect(FILELIST *first)

{

   BOX *filebox;

   FILELIST    *curptr, *highlighted;
   int   maxlines=25, numlines=0, rows=0, l=0;
   int curline=0, start=0;
   char  temp[81];

   if(!first)
      {
      Message("No files found!", -1, 0, YES);
      ok = 0;
      return;
      }

   if(first->next == NULL)
      {
      first->tagged=1;
      sprintf(temp, "%s selected!", first->name);
      Message(temp, -1, 0, YES);
      return;
      }

   for(curptr=first; curptr->next != NULL; curptr=curptr->next)
         numlines++;

   rows = numlines>maxlines-8 ? maxlines-8 : numlines+1;

   filebox = initbox(5,0,rows+6,79,cfg.col.asframe,cfg.col.astext,SINGLE,YES,' ');
   drawbox(filebox);
   statusbar("<ENTER> or <SPACE> to select, CTRL-ENTER to accept");

   while(1)    /* Exit is w/ return statement that gives command back */
      {
      curptr=first;

      /* search first line to display */

      for(l=0; l<start; l++)
         curptr=curptr->next;

      /* display them.. */

      for(l=start; l<start+rows; l++, curptr=curptr->next)
         {
         sprintf(temp, "* %-20.20s   %4d K ", curptr->name, curptr->size/1024);

         if (!curptr->tagged) temp[0] = ' ';

         if (l==curline)
            {
            print(l-start+6, 1, cfg.col.ashigh, temp);
            highlighted = curptr;
            MoveXY(2, l-start+7);
            }
         else
            print(l-start+6, 1, cfg.col.astext, temp);
         }

      /* Now check for commands... */

      switch (get_idle_key(1))
         {
         case 328:      /* up */
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

         case 336:      /* down */
            if (curline<numlines)
               {
               curline++;
               if (curline>=start+rows)
                     start++;
               }
            else
               start=curline=0;
            break;

         case 327:      /* home */
            curline=start=0;
            break;

         case 335:      /* end */
            curline=numlines;
            start=numlines-rows+1;
            break;

         case 329:      /* page up */

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

         case 337:      /* page down */

            if (start == numlines-rows+1)
               {
               curline = numlines;
               break;
               }

            start = curline+rows;

            if (start > numlines-rows+1)
               start = numlines-rows+1;

            if (curline < start)
               curline = start;

            break;

         case 315:
            show_help(4);
            break;

         case 10:
            delbox(filebox);
            return;

         case 27:
            delbox(filebox);
            ok = 0;
            return;

         case 13:    /* <CR> */
         case 32:    /* Space */

            highlighted->tagged = highlighted->tagged ? 0 : 1;
            printstats(first);
            stuffkey(336);
            break;

         case 43:    /* + */

              curptr = first;
              while(curptr)
                 {
                 curptr->tagged = 1;
                 curptr = curptr->next;
                 }
              printstats(first);
              break;

         case 45:    /* - */

              curptr = first;
              while(curptr)
                 {
                 curptr->tagged = 0;
                 curptr = curptr->next;
                 }
              printstats(first);
              break;

         }           /* switch */

   }        /* while(1) */

}


/* ---------------------------------------------------- */


void writefa(XMSG *hdr, MSG *areahandle, char *subject, AREA *area)

{
   MSGH *msghandle;
   char *kludges;
   dword oldattr = hdr->attr; /* ave them, in check_direct they might change */

   strcpy(hdr->subj, subject);

   kludges = MakeKludge(NULL, hdr, 1); /* Last==1, always netmail */

   if ((msghandle = MsgOpenMsg(areahandle, MOPEN_CREATE, 0)) == NULL)
      return;

   if ((MsgWriteMsg(msghandle,
                    0,
                    hdr,
                    NULL,
                    0L,
                    0L,
                    strlen(kludges)+1,
                    kludges)) == -1)
        {
        Message("Error writing message!", -1, 0, YES);
        }

   MsgCloseMsg(msghandle);

   add_tosslog(area, areahandle);

   hdr->attr = oldattr;

}


void printstats(FILELIST *first)

{
   long tsize=0L;
   int n=0;
   FILELIST *countptr = first;
   char temp[80];


   while(countptr)
      {
      if(countptr->tagged)
         {
         n++;
         tsize += countptr->size;
         }
      countptr = countptr->next;
      }

      sprintf(temp, "A total of %d files selected (%dK).  ", n, tsize/1024);
      print(6,40,cfg.col.astext,temp);

}
