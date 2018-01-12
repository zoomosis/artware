#include <msgapi.h>
#include <conio.h>
#include <video.h>
#include <scrnutil.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <alloc.h>
#include <time.h>
#include <progprot.h>
#include <stdlib.h>

#include "xmalloc.h"
#include "wrap.h"
#include "tstruct.h"
#include "find.h"
#include "input.h"
#include "global.h"
#include "idlekey.h"
#include "message.h"
#include "getmsg.h"
#include "bmg.h"
#include "help.h"
#include "showmail.h"
#include "readarea.h"
#include "repedit.h"
#include "header.h"

#define FROM    0x01
#define TO      0x02
#define SUBJECT 0x04
#define BODY    0x08
#define DO_IT   99

#define LEGAL " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-_=+\\|][{}'\";:/?.>,<"

static  int allareas=0, allmsgs=0, header, body;

int select;

typedef struct _findwhat
{
   char    string[21];
   char    where;
   bmgARG  pattern;

} FindWhat;


static FindWhat  ARGS[10];   // Was 10

int  getargs     (AREA *area);
int  checkbox    (char pos, int whatnow, AREA *area);
char *make_where (char where);
int  getss       (AREA *area);
int  searchloc   (char line, AREA *area);
void do_search   (MSG *areahandle, AREA *area, long last);
int  search_area (MSG *areahandle, AREA *curarea, long start);
int  check_msg   (XMSG *hdr, char *msgtext, dword txtlen);
int  mark_lines  (MMSG *curmsg);

void load_search_data(void);
void save_search_data(AREA *area);


void FindMessage(MSG *areahandle, AREA *area, long last)

{
   int fake;
   BOX *master;


   load_search_data();

   _setcursortype(_NORMALCURSOR);

   master = initbox(5,6,22,67,112,7,DOUBLE,NO,' ');
   drawbox(master);
   print(20,26,113," Press F1 for help ");
   if(!(REGISTERED))
      statusbar("    Note: highlighting of lines with matches only in registered versions!");

   while(1)
      {
      select = 3;
      header = body = 0;

      if (getargs(area) != 0)
         break;

      for (fake=0; (fake<10) && (ARGS[fake].string[0] != '\0'); fake++)
         {
         if ( (ARGS[fake].where & FROM   ) ||
              (ARGS[fake].where & TO     ) ||
              (ARGS[fake].where & SUBJECT)   )
              header=1;
         if (ARGS[fake].where & BODY) body=1;
         bmgCompile(ARGS[fake].string, &ARGS[fake].pattern, 1);
         }

      if (header || body)
         do_search(areahandle, area, last);
      else Message("I don't know where to search!",-1,0,YES);
      }

   delbox(master);

}


int getargs(AREA *area)
{
   BOX *scanwhat, *scanfrom, *args;
   int loop=1;

   scanwhat = initbox(7,9,10,34,3,7,SINGLE,NO,' ');

   drawbox(scanwhat);
   print(7,11,7," Areas: ");
   boxwrite(scanwhat, 0, 1, "( ) Current Area only");
   boxwrite(scanwhat, 1, 1, "( ) All Areas");
   if (!allareas)
      printc(8, 12, 7, 7);
   else printc(9,12,7,7);

   delbox(scanwhat);

   scanfrom = initbox(11,9,14,34,3,7,SINGLE,NO,' ');

   drawbox(scanfrom);
   print(11,11,7," Messages: ");
   boxwrite(scanfrom, 0, 1, "( ) From Lastread");
   boxwrite(scanfrom, 1, 1, "( ) All Messages");
   if (!allmsgs)
      printc(12,12,7,7);
   else printc(13,12,7,7);
   delbox(scanfrom);

   args = initbox(7,35,18,64,3,7,SINGLE,NO,' ');

   drawbox(args);
   print(7,37,7," Search for: ");
   print(7,58,7," in: ");
   delbox(args);

   while(loop == 1)
      {
      switch(select)
         {
         case 1:
            if ( (allareas = checkbox ( 8,allareas,area)) == ESC)
               {
               allareas=0;
               return ESC;
               }
            break;
         case 2:
            if ( (allmsgs = checkbox (12,allmsgs,area)) == ESC)
               {
               allmsgs=0;
               return ESC;
               }
            break;
         case 3:
            if (getss(area) != 0)
               return ESC;
            break;

         default:
             loop=0;
             break;
         }
      }

   return 0;
}


int checkbox(char pos, int whatnow, AREA *area)
{
   char loop=1;

   while(loop)
      {
      printc(pos+whatnow,12,112,7);
      MoveXY(13, pos+whatnow+1);
      printc(pos+1-whatnow,12,7,' ');
      switch(get_idle_key(1))
         {
/*         case 13:*/  /* ENTER */
            /* intentional fallthrough */
         case 9:  /* TAB */
            loop=0;
            select++;
            break;
         case 10:
            loop=0;
            select=0;
            break;
         case 27:
            loop=0;
            return ESC;

         case 271:  /* SHIFT-TAB */
                  loop=0;
                  if (--select == 0)
                     select = 3;
                  break;

         case 13:
                  /* Intentional fallthrough */
         case 328:  /* UP */
                  /* intentional fallthrough */

         case 336:  /* DOWN */
                  whatnow = whatnow ? 0 : 1;
                  break;
         case 315:   /* F1 */
                  show_help(3);
                  break;
         case 287:  /* ALT-S */
              save_search_data(area);
              break;
         }
      }

   printc(pos+whatnow,12,7,7);    /* reset highlight */
   return whatnow;
}


int getss(AREA *area)
{
   char line=0, item=0, loop=1;
   int ret;
   char fake;

   for(fake=0; (ARGS[fake].string[0] != '\0') && (fake<10); fake++)
      {
      print(8+fake,37,7,ARGS[fake].string);
      print(8+fake,59,7,make_where(ARGS[fake].where));
      }

   while(loop)
      {
      if(!item)
         ret = getstring(8+line, 37, ARGS[line].string, 20, "",cfg.col.entry);
      else
         {
         MoveXY(60, 8+line+1);
         ret = searchloc(line, area);
         }

      switch(ret)
         {
         case UP:
            /* intentional fallthrough */
/*       case BTAB: */
            if(item)
               item--;
            else if(line)
                  {
                  line--;
                  item=1;
                  }
            else
                  {
                  line=9;
                  item=1;
/*                  select--;
                  loop=0; */
                  }
            break;

         case BTAB:
           loop=0;
           select=2;
           break;

         case TAB:
           loop=0;
           select=1;
           break;

         case ESC:

              return ESC;

         case RESET:

              for(fake=0; (ARGS[fake].string[0] != '\0') && (fake<10); fake++)
                 {
                 memset(ARGS[fake].string, '\0', 20);
                 ARGS[fake].where = 0;
                 print(8+fake,37,7,"                     ");
                 print(8+fake,59,7,make_where(ARGS[fake].where));
                 }
              line=item=0;
              break;

         case ACCEPT:

              loop=0;
              select=0;
              break;

         case HELP:

              show_help(3);
              break;

         case SAVE:

              save_search_data(area);
              break;

         default:
            if(!item)
               item=1;
            else
               {
               if(++line > 9)
                  {
                  /*loop=0;
                  select=1;*/
                  line=0;
                  }
               item = 0;
               }
            break;
         }
      }

   return 0;
}


char *make_where(char where)
{
   static char  ws[5];

   memset(ws, ' ', 4);
   ws[4]='\0';

   if(where & FROM   ) ws[0]= 'F';
   if(where & TO     ) ws[1]= 'T';
   if(where & SUBJECT) ws[2]= 'S';
   if(where & BODY   ) ws[3]= 'B';

   return ws;
}


int searchloc(char line, AREA *area)
{
   BOX *wtsbox;
   char loop=1;
   int  ret=0;


   print(8+line,59,7,make_where(ARGS[line].where));

   wtsbox = initbox(7+line,65,12+line,77,3,7,SINGLE,YES,' ');
   drawbox(wtsbox);
   boxwrite(wtsbox,0,1,"[F]rom");
   boxwrite(wtsbox,1,1,"[T]o");
   boxwrite(wtsbox,2,1,"[S]ubject");
   boxwrite(wtsbox,3,1,"[B]ody");

   while (loop)
      {
      switch( get_idle_key(1) )
         {
         case 'f':
         case 'F':
            ARGS[line].where ^= FROM;
            break;
         case 't':
         case 'T':
            ARGS[line].where ^= TO;
            break;
         case 's':
         case 'S':
            ARGS[line].where ^= SUBJECT;
            break;
         case 'b':
         case 'B':
            ARGS[line].where ^= BODY;
            break;
         case 13:    /* Enter */
            loop=0;
            ret = ENTER;
            break;
         case 9:     /* TAB */
            loop=0;
            ret = TAB;
            break;
         case 27:    /* ESC */
            ret = ESC;
            loop = 0;
            break;
         case 10:      /* CTRL - ENTER */
            loop=0;
            ret=ACCEPT;
            break;
         case 271:  /* SHIFT-TAB */
                  loop = 0;
                  ret = BTAB;
                  break;
         case 275:   /* ALT-R */
                  loop=0;
                  ret=RESET;
                  break;
         case 287:   /* ALT-S */
                  save_search_data(area);
                  break;
         case 328:  /* UP */
                  ret = UP;
                  loop = 0;
                  break;
         case 336:  /* DOWN */
                  ret = ENTER; /* Don't tell anyone, same reaction as ENTER */
                  loop = 0;
                  break;
         case 315:    /* F1 */
                  show_help(3);
                  break;
         }

      print(8+line,59,7,make_where(ARGS[line].where));
      MoveXY(60, 8+line+1);
      }

   delbox(wtsbox);
   return ret;
}


void do_search(MSG *areahandle, AREA *area, long last)

{
   AREA *thisarea;
   BOX *whatarea;
   char temp[80];
   int stop=0;
   MSG *curhandle;

   whatarea = initbox(15,9,18,34,3,7,SINGLE,NO,' ');
   drawbox(whatarea);
   boxwrite(whatarea,0,1,"Area:");
   boxwrite(whatarea,1,1,"Msg#:");
   delbox(whatarea);


   if(allareas)
     {
     for(thisarea = cfg.first; thisarea && !stop; thisarea=thisarea->next)
        {
        if(thisarea == area)  /* Scanning the area we are in */
          curhandle = areahandle;
        else
          if((curhandle=MsgOpenArea(thisarea->dir, MSGAREA_CRIFNEC, thisarea->base)) == NULL)
              {
              sprintf(msg, "Can't open %s (%d)!", thisarea->tag, heapcheck());
              Message(msg,-1,0,YES);
              continue; /* Can't open, skip to next */
              }

        sprintf(temp, "%-15.15s ", thisarea->desc);
        print(16,17,7,temp);
        if(!thisarea->scanned)
              ScanArea(thisarea, curhandle);

		  stop = search_area(curhandle, thisarea, allmsgs ? 0L : thisarea->lr);

        if(thisarea != area) /* Don't close it if it's our 'current' area */
                    MsgCloseArea(curhandle);

        } /* for */
     }
   else
       search_area(areahandle, area, allmsgs ? 0L : last);

}


int search_area(MSG *areahandle, AREA *curarea, long start)
{
   MSGH *msghandle=NULL;
   MMSG *foundmsg=NULL;
   long msgno;
   XMSG hdr;
   dword txtlen, result, anchor;
   char *msgtext=NULL, temp[80];
   int command=0;

   sprintf(temp, "%4.4ld", start);
   print(17,17,7,temp);

   for( msgno=start+1; (msgno <= MsgGetHighMsg(areahandle)) && (command != ESC); msgno++)
      {
      if( xkbhit() && (get_idle_key(1) == 27) )
                 return 1;

      if ((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, msgno)) == NULL)
         continue;

      if( (msgno%10) == 0)
          {
          sprintf(temp, "%5.5ld  ", msgno);
          print(17,17,7,temp);
          }

      if( (!body) && header)
         result = MsgReadMsg(msghandle, &hdr, 0L, 0L, NULL, 0L, NULL);
      else
         {
         memset(&hdr, '\0', sizeof(XMSG));

         txtlen  = MsgGetTextLen(msghandle);

         /* Sanity checks */

         if( (dword) txtlen < (dword) 0L )
            txtlen = (dword) 0L;

         if( (dword) txtlen > (dword) 64000L )
            txtlen = (dword) 64000L;


		   if (txtlen)
				msgtext = xcalloc(1, (unsigned) txtlen + 1);
         else
				msgtext = NULL;

         if(header)
		      result = MsgReadMsg(msghandle, &hdr, 0L, txtlen, msgtext, 0L, NULL);
         else
            result = MsgReadMsg(msghandle, NULL, 0L, txtlen, msgtext, 0L, NULL);

         }

		if (result == (dword) -1)			/* Oh, oh, error! */
			{
			sprintf(temp, "Error reading msg no %d!", msgno);
         Message(temp, 2, 0, YES);
         free(msgtext);
         msgtext=NULL;
         continue;
			}

      if (body && txtlen) *(msgtext + txtlen - 1) = '\0';

		if(MsgCloseMsg(msghandle))
		   Message("Error closing message!", 2, 0, YES);

      msghandle = NULL;

      if (check_msg(&hdr, msgtext, txtlen))
         {
         if(msgtext) /* Free some mem, we may need it! */
            {
            free(msgtext);
            msgtext = NULL;
            }

         get_custom_info(curarea);

         /* Anchor, user might delete, reply -> sliding msgs */
         anchor = MsgMsgnToUid(areahandle, msgno);

			savescreen();

         command=0;

         while( (command != ESC)  &&
                (command != NEXT) &&
                (command <= 0) )
                {
                foundmsg = GetMsg(msgno, areahandle, 1, curarea);
                mark_lines(foundmsg);
			       clsw(cfg.col.msgtext);
                kbflush();
			       command = ShowMsg(foundmsg, curarea, areahandle, SCAN_DISPLAY);
                ReleaseMsg(foundmsg, 1);
                }

         /* Pull up anchor, so we always restart at the right msg */
         msgno = MsgUidToMsgn(areahandle, anchor, UID_PREV);

			putscreen();

         if(command == ESC) return 1;
         }


      if(msgtext)
         {
         free(msgtext);
         msgtext = NULL;
         }
      }

   return 0;
}


int check_msg(XMSG *hdr, char *msgtext, dword txtlen)
{
   int loop;


   for(loop=0; ARGS[loop].string[0] != '\0' ; loop++)
      {

      if(ARGS[loop].where & FROM)
         {
         if( (bmgSearch(hdr->from, strlen(hdr->from), &ARGS[loop].pattern)) != NULL)
             return 1;
         }

      if(ARGS[loop].where & TO)
         {
         if( (bmgSearch(hdr->to, strlen(hdr->to), &ARGS[loop].pattern)) != NULL)
             return 1;
         }

      if(ARGS[loop].where & SUBJECT)
         {
         if( (bmgSearch(hdr->subj, strlen(hdr->subj), &ARGS[loop].pattern)) != NULL)
             return 1;
         }

      if(ARGS[loop].where & BODY)
         {
         if (txtlen && (bmgSearch(msgtext, (int) txtlen, &ARGS[loop].pattern) != NULL) )
             return 1;
         }

      }


   return 0;
}


int mark_lines(MMSG *curmsg)
{
   int loop;
   LINE *thisline;

   if(!(REGISTERED))
     return 0;

   for(thisline=curmsg->txt; thisline; thisline=thisline->next)
      {
      for(loop=0; ARGS[loop].string[0] != '\0' ; loop++)
         {
         if(ARGS[loop].where & BODY)
            {
            if (stristr(thisline->ls, ARGS[loop].string) != NULL)
                {
                thisline->status &= 0x60; /* clear all except tab/hcr */
                thisline->status |= HIGHLIGHT;
                }
            }

         }
      }


   return 0;
}



void load_search_data(void)
{
   int i;
   char temp[30];

   if(strlen(custom.sargs[0].string) < 3)
       return;

   memset(&ARGS, '\0', sizeof(ARGS));

   allareas = custom.allareas;
   allmsgs  = custom.allmsgs;

   for(i=0; i<10; i++)
      {
      if(strlen(custom.sargs[i].string) < 3)
         break;

      if(custom.sargs[i].string[strlen(custom.sargs[i].string)-1] == '\n')
         custom.sargs[i].string[strlen(custom.sargs[i].string)-1] = '\0';

      memset(temp, '\0', sizeof(temp));
      strncpy(temp, custom.sargs[i].string, 20);
      while(temp[strlen(temp)-1] == ' ')
            temp[strlen(temp)-1] = '\0';

      strcpy(ARGS[i].string, temp);

      memset(temp, '\0', sizeof(temp));
      strncpy(temp, custom.sargs[i].string + 21, 4);

      if(strchr(temp, 'F') != NULL)
          ARGS[i].where |= FROM;
      if(strchr(temp, 'T') != NULL)
          ARGS[i].where |= TO;
      if(strchr(temp, 'S') != NULL)
          ARGS[i].where |= SUBJECT;
      if(strchr(temp, 'B') != NULL)
          ARGS[i].where |= BODY;
      }

}


void save_search_data(AREA *area)
{

   char datafile[120], temp[201];
   FILE *data;
   char hello=0, rephello=0, signoff=0, followhello=0, origin=0;
   int i;

   if(!(REGISTERED))
     {
     Message("This function is only available in registered versions", -1, 0, YES);
     return;
     }


   if(area->base & MSGTYPE_SQUISH)
     {
     sprintf(datafile, "%s.SQT", area->dir);
     }
   else if(area->base & MSGTYPE_SDM)
     {
     sprintf(datafile, "%s\\timed.dat", area->dir);
     }
   else if(area->base & MSGTYPE_JAM)
     {
     sprintf(datafile, "%s.JTI", area->dir);
     }
   else /* Hudson */
     {
     sprintf(datafile, "%s\\MSG%d.TIM", cfg.usr.hudsonpath, atoi(area->dir));
     }


   if( (data=fopen(datafile, "r")) != NULL)
      {
      if(area->base & MSGTYPE_HMB)
         {
         if(fgets(temp, 200, data) != NULL)
             {
             if(strncmpi(temp, "<default>", 9) != 0)
                 origin = 1;
             }
         }

      if(fgets(temp, 200, data) != NULL)
          {
          if(strncmpi(temp, "<default>", 9) != 0)
              hello = 1;
          }

      if( (!feof(data)) && (fgets(temp, 200, data) != NULL) )
          {
          if(strncmpi(temp, "<default>", 9) != 0)
              rephello = 1;
          }

      if( (!feof(data)) && (fgets(temp, 200, data) != NULL) )
          {
          if(strncmpi(temp, "<default>", 9) != 0)
              signoff = 1;
          }

      if( (!feof(data)) && (fgets(temp, 200, data) != NULL) )
          {
          if(strncmpi(temp, "<default>", 9) != 0)
              followhello = 1;
          }

      fclose(data);
      }


   if( (data = fopen(datafile, "w+")) == NULL)
      {
      sprintf(temp, "Can't open %s for writing!", datafile);
      Message(temp, -1, 0, YES);
      return;
      }

   /* First we write hellostrings and such */

   if(area->base & MSGTYPE_HMB)
     {
     if(origin)
         fprintf(data, "%s\n", custom.origin);
     else
         fprintf(data, "<default>\n");
     }

   if(hello)
       fprintf(data, "%s\n", custom.hello);
   else
       fprintf(data, "<default>\n");

   if(rephello)
       fprintf(data, "%s\n", custom.rephello);
   else
       fprintf(data, "<default>\n");

   if(signoff)
       fprintf(data, "%s\n", custom.signoff);
   else
       fprintf(data, "<default>\n");

   if(followhello)
       fprintf(data, "%s\n", custom.followhello);
   else
       fprintf(data, "<default>\n");


   /* Then we write the search strings.... */

   fprintf(data, "%s\n", allareas ? "all" : "current" );
   fprintf(data, "%s\n", allmsgs  ? "all" : "lastread");

   custom.allareas = allareas;
   custom.allmsgs  = allmsgs;

   for(i=0; i<10; i++)
      {
      if(ARGS[i].string[0] == '\0')
         break;
      fprintf(data, "%-20.20s %-4.4s\n", ARGS[i].string, make_where(ARGS[i].where));
      sprintf(custom.sargs[i].string, "%-20.20s %-4.4s", ARGS[i].string, make_where(ARGS[i].where));
      }

   fclose(data);

}
