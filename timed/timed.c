#include "includes.h"
#include <signal.h>

/* extern unsigned _stklen = 6500; */

// extern unsigned _ovrbuffer;

extern int clockthreadid;

int  pause=0;
char timdir[100] = "";

void readparms (int argc, char *argv[]);
void getstats(AREA *curarea);
AREA *nextnewmail(AREA *thisarea, int direction);
void donothing(int sig);


#ifdef __FLAT__

void donothing(int sig)         //define the handler
{
  signal(sig, donothing);
}

#else

int c_break(void)
{
   return (1);
}

#endif


void main(int argc, char *argv[])
{
   struct _minf   minf;
   AREA *thisarea=NULL, *lastarea;
   BOX *intro, *copyright, *configstatus;
   int erlvl, readret=ESC;
   int keyread=0;
   char temp[100];


   // We need a bit more than the usual stuff.

   _grow_handles(40);

#ifdef __OS2__
   atexit(killclock);
   signal(SIGBREAK, donothing);
   signal(SIGINT, donothing);
#else
   #ifndef __386__
   ins09();
   atexit(undo09);
   #else
   signal(SIGBREAK, donothing);
   signal(SIGINT, donothing);
   #endif
#endif

   memset(&cfg, '\0', sizeof(CFG));
   memset(&AreaSelectKeys, '\0', 256 * sizeof(sword));
   memset(&ReadMessageKeys, '\0', 256 * sizeof(sword));
   memset(&EditorKeys, '\0', 256 * sizeof(sword));
   memset(&GlobalKeys, '\0', 256 * sizeof(sword));
   memset(&ListKeys, '\0', 256 * sizeof(sword));

   cfg.usr.registered = mem_calloc(1, sizeof(char));
   cfg.col[Cpopframe] = 112;
   cfg.col[Cpoptitle] = 112;
   cfg.col[Cpoptext] = 112;

   readparms(argc, argv);

   video_init();

#if !defined(__OS2__) && !defined(__NT__)
   check_mtask();
#endif

   #ifndef __WATCOMC__
   dv_conio();
   #endif

   tzset();

   #ifdef __WATCOMC__
   _settextcursor(0x2000);
   #else
   _setcursortype(_NOCURSOR);
   #endif

   cls();

   copyright = initbox(0,0,4,maxx-1,3,7,SINGLE,NO,' ');
   intro = initbox(4,0,maxy-1,maxx-1,3,7,SINGLE,NO,'�');
   drawbox(copyright);
   drawbox(intro);
   delbox(copyright);
   delbox(intro);

   print(1,maxx-28,4,  "���������������������������");
   print(2,maxx-28,112,"  Made in The Netherlands  ");
   print(3,maxx-28,1 , "���������������������������");
   print(0,maxx-29,3,"�");
   print(1,maxx-29,3,"�");
   print(2,maxx-29,3,"�");
   print(3,maxx-29,3,"�");
   print(4,maxx-29,3,"�");

   print(1,19,7,myname);
   print(2,5,7,"(c) 1992-'96  Gerard van Essen (2:281/527)");
   print(3,3,7,"Message editor for Squish, *.MSG, JAM & Hudson");

   print(4,0,3,"�"); print(4,maxx-1,3,"�");

   configstatus = initbox(8,4,20,75,3,7,SINGLE,NO,' ');
   drawbox(configstatus);
   delbox(configstatus);

   print(10,6,7,"[ ] timEd configuration file");
   print(12,6,7,"[ ] timEd key & macro configuration");
   print(14,6,7,"[ ] Tosser configuration file");
   print(16,6,7,"[ ] Areas.bbs file");
   print(18,6,7,"[ ] Include config file");

   if(cfg.homedir[0] == '\0')
           getcwd(cfg.homedir, 80);
   if(cfg.homedir[strlen(cfg.homedir)-1] == '\\')
      cfg.homedir[strlen(cfg.homedir)-1] = '\0';

   #ifndef __SENTER__
      keyread = read_key();
   #else
      keyread = 1;
   #endif

//   if( (keyread = read_key()) == 0)
//     {
//     cls();
//     _settextcursor(0x0607);
//     MoveXY(1,6);
//     print(3,0,7,"* This is a gamma version, only for registered users!");
//     exit(254);
//     }

   readconfig();

   if(ReadKeyFile() != 0)
     Message("Can't read keyboard definition file!", -1, 254, YES);

   if(keyread) check_registration();

   if(cfg.usr.status & LOWLEVELKB)  // Only needed for low level routines.
      check_enhanced();

   memset(&minf, '\0', sizeof(struct _minf));
   minf.def_zone=cfg.usr.address[0].zone;    /* set default zone to user's primary address */
   if(cfg.usr.status & NOSPACEPASSWORD)
      minf.nospace = 1;
   MsgOpenApi(&minf, cfg.homedir, (cfg.usr.status & ARCMAIL) ? 1 : 0, cfg.usr.hudsonpath);
   lastarea = cfg.first;

   ReadTagFile(0);      // Read in default tag set


   if(TIMREGISTERED)
      {
      sprintf(msg, " Registered to %s. ", cfg.usr.name[0].name);
      print(6, 40 - (strlen(msg)/2), 113, msg);
      }
   else
      {
//      cls();
//      print(3,0,7,"This gamma version is only for registered users!");
//      _settextcursor(0x0607);
//      MoveXY(1,6);
//      exit(254);
      // See also exit errorlevel !!!!!!!!!!! ++++ !!!!!
      print(6, 23, 113, " Unregistered evaluation version. ");
      }

   if ((pause==1) || (!(TIMREGISTERED)) )
       {
       print(22,32,113," Press a key.. ");
       kbflush();
       get_idle_key(0, GLOBALSCOPE);  // 0, we don't want to loose stuffed startup_scan key
       }

   if(cfg.usr.lines != maxy)
     setlines(cfg.usr.lines);

//   ShowKeyboardInfo();

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

   clockoff();

//   cls();
   clsw(7);

   #ifdef __OS2__
   killclock();
   #endif

   cls();

   if(cfg.usr.lines != 0)
      {
      video_deinit();
      cls();
      }

   sprintf(temp, "%s  (c) 1992-'96  Gerard van Essen (2:281/527)", myname);
   print(1,0,7,temp);

   if(TIMREGISTERED)
      {
      #ifdef __SENTER__
      vprint(3,0,3,"This copy of timEd is licensed to Senter.");
      #else
      vprint(3,0,3,"This copy of timEd is registered to %s.", cfg.usr.name[0].name);
      #endif
      MoveXY(1,6);
      }
   else    // Unregistered folk..
     {
     print(3,0,3,"This is an ");
     print(3,11,131,"UNREGISTERED");
     print(3,23,3," evaluation version of timEd.");
     print(5,0,7,"If you like this program, please show your support and register.");
     print(6,0,7,"See the file TIMED.REG in the timEd package for more information.");
     sleep(2);
     MoveXY(1,9);
     }

   #ifdef __WATCOMC__
   _settextcursor(0x0607);
   #else
   _setcursortype(_NORMALCURSOR);
   #endif

   erlvl = write_echolog();

   MsgCloseApi();

   kbflush();

   exit(erlvl);
}


int write_echolog(void)
{
   AREA *curarea;
   int erlvl=0;

   /* some defines to generate the exit errorlevel */

   #define ER_NET  0x01
   #define ER_ECHO 0x02
   #define ER_LOC  0x04
   #define ER_MAIL 0x08
   #define ER_NEWS 0x10


   curarea = cfg.first;

   while(curarea)
      {
      if(curarea->newmail)
         {
         switch(curarea->type)
           {
           case ECHOMAIL:
             erlvl |= ER_ECHO;
             break;

           case NEWS:

             erlvl |= ER_NEWS;
             break;

           case NETMAIL:

             erlvl |= ER_NET;
             break;

           case LOCAL:

             erlvl |= ER_LOC;
             break;

           case MAIL:

             erlvl |= ER_MAIL;
             break;
           }
         }

      curarea = curarea->next;
      }

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

  ScanArea(curarea, areahandle, 1);

  MsgCloseArea(areahandle);

}



AREA *nextnewmail(AREA *thisarea, int direction)
{
   char temp[80];

   // First we go to the next / previous area
   thisarea = (direction == NEXTAREA) ? thisarea->next : thisarea->prev;

   // Then we find the first one that is visible
   while(thisarea && !AreaVisible(thisarea))
      thisarea = (direction == NEXTAREA) ? thisarea->next : thisarea->prev;

   while(thisarea)
      {
      if( kbhit() && (getch()==27) )
         return NULL;

      sprintf(temp, "Scanning area: %-0.50s", thisarea->tag);
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

      // Then we find the first one that is visible
      while(thisarea && !AreaVisible(thisarea))
         thisarea = (direction == NEXTAREA) ? thisarea->next : thisarea->prev;
      }

   return NULL;

}
