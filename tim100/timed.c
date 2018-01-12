#include <msgapi.h>
#include <string.h>
#include <alloc.h>
#include <stdio.h>
#include <conio.h>
#include <video.h>
#include <scrnutil.h>
#include <stdlib.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <sys\stat.h>
#include <time.h>

#include "xmalloc.h"
#include "wrap.h"
#include "tstruct.h"
#include "select.h"
#include "readarea.h"
#include "config.h"
#include "global.h"
#ifndef __OS2__
#include "dvaware.h"
#endif
#include "message.h"
#include "idlekey.h"
#include "header.h"
#include "register.h"


#ifdef DEBUG
       #include "debug.h"
#endif

extern unsigned _stklen = 6500;

//extern unsigned _ovrbuffer;

int  pause=0;
char timdir[100] = "";

void readparms (int argc, char *argv[]);
int  close_it_up(void);
void getstats(AREA *curarea);
AREA *nextnewmail(AREA *thisarea, int direction);


void cdecl main(int argc, char *argv[])

{
	struct _minf	minf;
   AREA *thisarea=NULL, *lastarea;
   BOX *intro, *copyright, *configstatus;
   int erlvl, readret=ESC;
   int keyread=0;

   memset(&cfg, '\0', sizeof(CFG));
   cfg.usr.registered = xcalloc(1, sizeof(char));

   readparms(argc, argv);

	video_init();

   #ifndef __OS2__
   dv_conio();
   #endif

   tzset();

	_setcursortype(_NOCURSOR);

   copyright = initbox(0,0,4,maxx-1,3,7,SINGLE,NO,' ');
   intro = initbox(4,0,maxy-1,maxx-1,3,7,SINGLE,NO,'Б');
   drawbox(copyright);
   drawbox(intro);
   delbox(copyright);
   delbox(intro);

   print(1,maxx-28,4,  "ллллллллллллллллллллллллллл");
   print(2,maxx-28,112,"  Made in The Netherlands  ");
   print(3,maxx-28,1 , "ллллллллллллллллллллллллллл");
   print(0,maxx-29,3,"Т");
   print(1,maxx-29,3,"Г");
   print(2,maxx-29,3,"Г");
   print(3,maxx-29,3,"Г");
   print(4,maxx-29,3,"С");

   print(1,20,7,"timEd 1.00");
   print(2,5,7,"(c) 1992-'94  Gerard van Essen (2:281/527)");
   print(3,3,7,"Message editor for Squish, *.MSG, JAM & Hudson");

   print(4,0,3,"У"); print(4,maxx-1,3,"Д");

   configstatus = initbox(9,4,19,75,3,7,SINGLE,NO,' ');
   drawbox(configstatus);
   delbox(configstatus);

   print(11,6,7,"[ ] timEd configuration file");
   print(13,6,7,"[ ] Tosser configuration file");
   print(15,6,7,"[ ] Areas.bbs file");
   print(17,6,7,"[ ] Include config file");

   keyread = read_key();

   if(cfg.homedir[0] == '\0')
           getcwd(cfg.homedir, 80);
   if(cfg.homedir[strlen(cfg.homedir)-1] == '\\')
      cfg.homedir[strlen(cfg.homedir)-1] = '\0';

   readconfig();

   if(heapcheck() == -1)
         Message("Heap corrupt (in timed.c)! Report to the author!", -1, 254, NO);

   if(keyread) check_registration();

   if(cfg.usr.status & LOWLEVELKB)  // Only needed for low level routines.
      check_enhanced();

   memset(&minf, '\0', sizeof(struct _minf));
	minf.def_zone=cfg.usr.address[0].zone;    /* set default zone to user's primary address */
   MsgOpenApi(&minf, cfg.homedir, cfg.usr.arcmail, cfg.usr.hudsonpath);
   lastarea = cfg.first;

   print(21,32,113," Press a key.. ");

   if(REGISTERED)
      {
      sprintf(msg, " Registered to: \"%s\". ", cfg.usr.name[0].name);
      print(7, 40 - (strlen(msg)/2), 113, msg);
      }
   else
      {
      print(7, 23, 113, " Unregistered evaluation version. ");
      }

   if ((pause==1) || (!(REGISTERED)) )
       {
       kbflush();
       get_idle_key(0);  // 0, we don't want to loose stuffed startup_scan key
       }

	while(1)
		{
      if( (readret == NEXTAREA) || (readret == PREVAREA) ) /* goto next area with mail */
         {
         thisarea = nextnewmail(thisarea, readret);
         if(thisarea == NULL) thisarea=SelectArea(cfg.first, 0, lastarea);
         }

		else thisarea=SelectArea(cfg.first, 0, lastarea);

		if(thisarea==NULL)                  /* Selectarea ALT-X */
			break;

		if((readret=ReadArea(thisarea)) == EXIT)  /* ALT-X pressed */
         break;

      lastarea = thisarea;
		}

	erlvl = close_it_up();

	MsgCloseApi();

	cls();

   window(1,1,80,25);
   textattr(7);
   clrscr();

   print(1,0,14,"timEd");
   print(1,5,7,"  (c) 1992-'94  Gerard van Essen (2:281/527)");

   if(REGISTERED)
      {
      sprintf(msg, "This copy of timEd is registered to: \"%s\".", cfg.usr.name[0].name);
      print(3,0,3,msg);
      MoveXY(1,6);
      }
   else    // Unregistered folk..
     {
     print(3,0,3,"This is an ");
     print(3,11,131,"UNREGISTERED");
     print(3,23,3," evaluation version of timEd.");
     print(5,0,7,"If you like this program, please show your support and register.");
     print(6,0,7,"See the file REGFORM.TXT in the timEd package for more information.");
     sleep(2);
     MoveXY(1,9);
     }

	_setcursortype(_NORMALCURSOR);

   kbflush();

   exit(erlvl);

}


int close_it_up(void)

{
	AREA *curarea;
   char temp[80];
   FILE *echolog;
   int erlvl=0, logisopen=0;

   /* some defines to generate the exit errorlevel */

   #define ER_NET  0x01
   #define ER_ECHO 0x02
   #define ER_LOC  0x04


	curarea = cfg.first;

   while(curarea)
      {
      if(curarea->newmail)
         {
         switch(curarea->type)
           {
           case ECHOMAIL:

              if( (cfg.usr.echolog[0] != '\0') &&
                   (
                     (curarea->base & MSGTYPE_SQUISH) ||
                     (curarea->base & MSGTYPE_SDM) ||
                     ((curarea->base & MSGTYPE_JAM) && (cfg.usr.jamlogpath[0]=='\0'))
                   )

                )  /* Write echotoss? */
                {
                if(!logisopen)
                   {
                   if( (echolog = fopen(cfg.usr.echolog, "at")) == NULL)
                     {
                     sprintf(temp, "Can't open %s!", cfg.usr.echolog);
                     Message(temp , -1, 0, NO);
                     }
                   else logisopen = 1;
                   }
                }

                if(logisopen)
                   fprintf(echolog, "%s\n", curarea->tag);

              erlvl |= ER_ECHO;
              break;

           case NETMAIL:

              erlvl |= ER_NET;
              break;

           case LOCAL:

              erlvl |= ER_LOC;
              break;
           }
         }

      curarea = curarea->next;
      }


   if(logisopen)
      fclose(echolog);

   return erlvl;
}


/* Analyse command line */


void readparms (int argc, char *argv[])
{
    int i;
    char *p;


    for(i=1; i<argc; i++)
        {
        p=argv[i];
        if(*p == '-' || *p == '/')
            {
            switch(tolower(*(++p)))
                {
                case 'c':                 /* Configfile */
                     strcpy(cfg.homedir,++p);
                     break;

                case 'p':                 /* Pause at startup */
                     pause=1;
                     break;
                }
            }
        }

}

/* Do a 'slow' scan of an area, using MSGAPI calls  */

void getstats(AREA *curarea)
{
  MSG *areahandle;

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
     return;
	  }

  /* Get the statistics.... */

  ScanArea(curarea, areahandle);

  MsgCloseArea(areahandle);

}



AREA *nextnewmail(AREA *thisarea, int direction)
{
   char temp[80];

   thisarea = (direction == NEXTAREA) ? thisarea->next : thisarea->prev;

   while(thisarea)
      {
      if( kbhit() && (getch()==27) )
         return NULL;

      sprintf(temp, "Scanning area: %s", thisarea->desc);
      statusbar(temp);

      if(!thisarea->scanned)
         {
         if(thisarea->base & MSGTYPE_SQUISH)
            {
            fastscan(thisarea, 0);
            if(thisarea->highest > thisarea->lr)   /* Fast scan might be wrong */
               getstats(thisarea);
            }
         else
            getstats(thisarea);
         }

      if(thisarea->highest > thisarea->lr)
                 return thisarea;

      thisarea = (direction == NEXTAREA) ? thisarea->next : thisarea->prev;
      }

   return NULL;

}
