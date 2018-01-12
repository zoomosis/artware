#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include <alloc.h>

#include <msgapi.h>
#include <progprot.h>
#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "xmalloc.h"
#include "idlekey.h"
#include "header.h"
#include "select.h"
#include "attach.h"
#include "print.h"

#include <video.h>
#include <scrnutil.h>
#include "message.h"
#include "showhdr.h"
#include "input.h"
#include "readarea.h"


#define VFNWW "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:\\._|=+-)(&^%$#@![]{}<>~`0123456789?*"

char seps[] = " \t)]'`,\":.";


typedef struct _reqlist
{

   char              name[13];   /* Name of the file */
   int               tagged;     /* 1 = tagged */
   struct _reqlist   *next;      /* Link to next file-rec */

} REQLIST;

static REQLIST *firstfile=NULL;

char *exts[] = {"arj",
                "zip",
                "arc",
                "pak",
                "lzh",
                "sqz",
                "com",
                "exe",
                "lha",
                "zoo",
                "txt",
                "sdn",
                "sda",
                "ans",
                "jpg",
                "gif",
                "tar",
                "ico",
                "bat",
                "cmd",
                "uc2",
                "" };

int  file_ext(char *s);
void addfilename(char *p, char *first);
int  showlist(REQLIST *first);
void writerequest(REQLIST *first, AREA *area, MSG *areahandle, MMSG *origmsg);



int get_request(MMSG *curmsg, AREA *area, MSG *areahandle)
{
   LINE *curptr;      /* Point to first line of message */
   char *p;
   int retval;
   REQLIST *thisfile;

   if(firstfile==NULL)  /* A 're-entry', don't scan if so? */
      {
      for(curptr=curmsg->txt; curptr; curptr=curptr->next)
         {
         if( (curptr->status & KLUDGE) ||
             (curptr->status & TEAR)   ||
             (curptr->ls[0] == '\0')    )
            continue;          /* No files in kludges or tears! (Origs maybe!) */


         for( p=strchr(curptr->ls+1, '.'); p; p=strchr(p, '.') )
            {
            if(
                 (strchr(seps, *(p+1))==NULL) &&
                 (strchr(seps, *(p-1))==NULL) &&
                 (*(p+1) != '\0')
              )
                {
                if(file_ext(p+1) != 0)   /* Yep, we got a file here! */
                   {
                   addfilename(p-1, curptr->ls);
                   p += 3;
                   }
                else
                   {
                   p++;              /* Skip dot..                         */
                   if(*p == '.')     /* Yet another dot! Maybe ......... ? */
                      while(*p == '.') p++;  /* Skip it fast */
                   }
                }
            else
                p++;         /* Move beyond dot..                  */
            }
         }
      }

   savescreen();

   retval = showlist(firstfile);

   if(retval == ACCEPT)
      writerequest(firstfile, area, areahandle, curmsg);

   /* Check if we need to dump the list from mem, or keep it */
   /* in order to move the msg one page up or down and       */
   /* re-disp[lay the list and continue..                    */

   if( (retval == ACCEPT) || (retval == ESC) )
      {
      while(firstfile)     /* Free mem taken by list w/ files */
         {
         thisfile=firstfile->next;
         free(firstfile);
         firstfile=thisfile;
         }

      firstfile = NULL;
      }

   putscreen();

   return retval;

}

/* Check if the string starting with *s is a file-extension, */
/* return 1 if this is the case, 0 otherwise.                */


int file_ext(char *s)
{
   int i;

   if(isdigit(*s))   /* Like timEd 1.00 etc. */
     return 0;


   for(i=0; exts[i][0] != '\0'; i++)
     {
     if(
          (strncmpi(s, exts[i], 3) == 0) &&     /* extension matched */
          (strchr(seps, *(s+3))!=NULL) /* fname ends here too! */
       )
       return 1;
     }

   if(
        ((*s == 'a') || (*s == 'A') || (*s == 'z') || (*s == 'Z') ) &&
        isdigit(*(s+1)) &&
        isdigit(*(s+2)) &&
        (strchr(seps, *(s+3))!=NULL) /* fname ends here too! */
     )
     return 1;  /* Catch nodediff.a22 etc */

   return 0;
}

/* Extract filename from line, and add it to the list */

void addfilename(char *p, char *first)
{
   char temp[100];
   char *charptr=p;
   REQLIST *thisone;
   static REQLIST *last=NULL;

   while( (strchr(seps, *charptr)==NULL) && (charptr > first ) )
        charptr--;

   if(strchr(seps, *charptr) != NULL)
       charptr++; /* Skip the found space */

   if( (p-charptr) > 8 )   /* Check length of filename */
      return;

   memset(temp, '\0', sizeof(temp));
   memmove(temp, charptr, p-charptr+5);

   thisone = xcalloc(1, sizeof(REQLIST));
   strcpy(thisone->name, temp);

   if(firstfile == NULL)
      firstfile=thisone;
   else last->next=thisone;

   last=thisone;

}


/* Show list of files found, let user select some of them, and manually add */


int showlist(REQLIST *first)
{

   BOX *filebox, *extrabox;

   REQLIST *curptr, *highlighted;
	int numlines, rows, l;
	int curline, start, editret;
	char	temp[81], extrafile[13];

   if(!first)
      {
      firstfile = xcalloc(1, sizeof(REQLIST));
      strcpy(firstfile->name, "FILES");
      first = firstfile;
      }

   startup:

   start = curline = 0;

	for(curptr=first, numlines=0; curptr->next != NULL; curptr=curptr->next)
			numlines++;

	rows = numlines>=maxy-8 ? maxy-8 : numlines+1;

	filebox = initbox(5,maxx-17,rows+6,maxx-1,cfg.col.asframe,cfg.col.astext,SINGLE,YES,' ');
	drawbox(filebox);
   statusbar("<enter> or <space> to select, ctrl-enter to accept, <ins> to add filenames");

	while(1)		/* Exit is w/ return statement that gives command back */
		{
		curptr=first;

		/* search first line to display */

		for(l=0; l<start; l++)
			curptr=curptr->next;

		/* display them.. */

		for(l=start; l<start+rows; l++, curptr=curptr->next)
			{
         sprintf(temp, "* %-12.12s ", curptr->name);

         if (!curptr->tagged) temp[0] = ' ';

			if (l==curline)
            {
				print(l-start+6, maxx-16, cfg.col.ashigh, temp);
            highlighted = curptr;
            MoveXY(maxx-14, l-start+7);
            }
			else
				print(l-start+6, maxx-16, cfg.col.astext, temp);
			}

		/* Now check for commands... */

		switch (get_idle_key(1))
			{
			case 328:		/* up */
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
				curline=start=0;
				break;

			case 335:		/* end */
				curline=numlines;
				start=numlines-rows+1;
				break;

         case 338:      /* <INS> */
            memset(extrafile, '\0', sizeof(extrafile));
            extrabox=initbox(13,24,17,56,cfg.col.popframe,cfg.col.poptext,SINGLE,YES,' ');
            drawbox(extrabox);
            print(15,26,cfg.col.poptext,"Filename to add:");
            editret = getstring(15,43,extrafile,12,VFNWW,cfg.col.entry);
            delbox(extrabox);
            if( (editret == 0) && (extrafile[0] != '\0') )
                 {
                 for(curptr=first; curptr->next; curptr=curptr->next)
                  /* nop, goto last entry */ ;
                 curptr->next = xcalloc(1, sizeof(REQLIST));
                 strcpy(curptr->next->name, extrafile);
                 curptr->next->tagged=1;
                 delbox(filebox);               /* New one will be made */
                 goto startup;
                 }
            
            break;

			case 329:		/* page up */

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
            /* show_help(4); */
            break;

         case 388:  /* ctrl pgup */

            delbox(filebox);
            return UP;

         case 374:  /* ctrl pgdwn */

            delbox(filebox);
            return DOWN;

         case 10:
            delbox(filebox);
            return ACCEPT;

         case 27:
            delbox(filebox);
            return ESC;

			case 13:		/* <CR> */
         case 32:    /* Space */

            highlighted->tagged = highlighted->tagged ? 0 : 1;
            stuffkey(336);
            break;

         case 43:    /* + */

              curptr = first;
              while(curptr)
                 {
                 curptr->tagged = 1;
                 curptr = curptr->next;
                 }
              break;

         case 45:    /* - */

              curptr = first;
              while(curptr)
                 {
                 curptr->tagged = 0;
                 curptr = curptr->next;
                 }
              break;

			}				/* switch */

	}			/* while(1) */

}


/* ---------------------------------------------------- */

void writerequest(REQLIST *first, AREA *area, MSG *areahandle, MMSG *origmsg)
{
   AREA *toarea;
   REQLIST *curfile=first;
   XMSG *header;
   union stamp_combo combo;
   struct tm *tmdate;
   time_t time_now;
   MSG *toareahandle;
   int retval;

   if( (toarea = SelectArea(cfg.first, 1, cfg.first)) == NULL)   /* Select a netmail area to use */
      return;

   header = (XMSG *)xcalloc(1, sizeof(XMSG));      /* Message header */

   header->attr= toarea->stdattr | MSGFRQ;
   
   strcpy(header->from, cfg.usr.name[custom.name].name);
   strcpy(header->to, "SysOp");

   header->orig.zone  = cfg.usr.address[toarea->aka].zone;          /* Fill this all */
   header->orig.net   = cfg.usr.address[toarea->aka].net;
   header->orig.node  = cfg.usr.address[toarea->aka].node;
   header->orig.point = cfg.usr.address[toarea->aka].point;

   header->dest.zone = origmsg->hdr.orig.zone;
   header->dest.net  = origmsg->hdr.orig.net;
   header->dest.node = origmsg->hdr.orig.node;
   header->dest.point= origmsg->hdr.orig.point;

   time(&time_now);
   tmdate=localtime(&time_now);
   header->date_written=header->date_arrived=(TmDate_to_DosDate(tmdate,&combo))->msg_st;

   if (toarea != area)
      {
    	if(!(toareahandle=MsgOpenArea(toarea->dir, MSGAREA_CRIFNEC, toarea->base)))
         {
         Message("Error opening area", -1, 0, YES);
         return;
		   }
      }
   else toarea=NULL;

   /* Now let the user edit the attributes */

   cls();
   paint_header(header, 1, 0);

   retval=UP;
   while( (retval!=ESC) && (retval!=0) )
      retval=SetAttributes(header);

   if(retval == ESC)
      goto close_up;

   while(1)
      {
      if( MsgLock(toarea ? toareahandle : areahandle) == -1 )
         {
         Message("Can't lock area! Wait for retry, <ESC> to abort", 10, 0, YES);
         if( xkbhit() && (get_idle_key(1) == 27) )
            goto close_up;
         }
      else break;
      }
  

   while(curfile)
     {
     if(curfile->tagged)
        {
        if( (strlen(header->subj) + strlen(curfile->name)) > 70)
           {
           writefa(header, toarea ? toareahandle : areahandle, header->subj, toarea ? toarea : area);
           memset(header->subj, '\0', 72);
           }
        strcat(header->subj, curfile->name);
        strcat(header->subj, " ");
        }
     curfile = curfile->next;
     }

   if(header->subj[0] != '\0')   /* Only if files *were* tagged! */
       writefa(header, toarea ? toareahandle : areahandle, header->subj, toarea ? toarea : area);

/*   if(toarea)
      toarea->newmail = 1;
   else
      area->newmail = 1; */

   close_up:

   free(header);

   MsgUnlock(toarea ? toareahandle : areahandle);

   ScanArea(toarea? toarea : area, toarea ? toareahandle : areahandle);

   if(toarea) 
       MsgCloseArea(toareahandle);

}

