#include <conio.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys\stat.h>
#include <time.h>
#include <direct.h>
#include <dos.h>
#include <errno.h>
#include <stdarg.h>
#include <malloc.h>

#include <msgapi.h>
#include <progprot.h>

#include "nstruct.h"
#include "wrap.h"
#include "txtbuild.h"
#include "lastread.h"
#include "akamatch.h"
#include "external.h"
#include "netfuncs.h"
#include "binkpack.h"
#include "command.h"
#include "config.h"

//#define __KASPER__

#ifdef __WATCOMC__

   #define MAXDIR   _MAX_DIR
   #define MAXDRIVE _MAX_DRIVE
   #define MAXFILE  _MAX_NAME
   #define MAXEXT   _MAX_EXT
//   #define coreleft _memavl
   #define fnsplit  _splitpath
   #define strncmpi strnicmp

   dword coreleft(void);

#endif

CFG  cfg;
XMASKLIST *firstxmask;
char cfgname[100] = "netmgr.cfg";
int  log;
int  dolog = 0;
int  debug = 0;

int  quiet = 0;
int  didsomething=0;

MESSAGE    msg;
AREALIST * thisarea;

CACHE    * cacheptr    = NULL;   // Points to first element in nodelist cache.
int        buffilled   = 0;      // How many elements used in cache?
int        lookups     = 0,
           cachehits   = 0;

static char *mtext[]={	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


typedef struct
{
   long bit;
   char txt[5];

} BITTXT;

#define NUMCONVS1 31

BITTXT Attr2Text1[] = {

{ aPRIVATE, "pvt "},
{ aCRASH  , "cra "},
{ aREAD   , "rcv "},
{ aSENT   , "snt "},
{ aFILE   , "f/a "},
{ aFWD    , "fwd "},
{ aORPHAN , "orp "},
{ aKILL   , "kil "},
{ aLOCAL  , "loc "},
{ aHOLD   , "hld "},
{ aXX2    , "xx2 "},
{ aFRQ    , "frq "},
{ aRRQ    , "rrq "},
{ aCPT    , "cpt "},
{ aARQ    , "arq "},
{ aURQ    , "urq "},
{ aSCANNED, "scn "},
{ aDIR    , "dir "},
{ aAS     , "a/s "},
{ aIMM    , "imm "},
{ aKFS    , "kfs "},
{ aTFS    , "tfs "},
{ aCFM    , "cfm "},
{ aLOK    , "lok "},
{ aZGT    , "zgt "},
{ aENC    , "enc "},
{ aCOM    , "com "},
{ aESC    , "esc "},
{ aFPU    , "fpu "},
{ aNODISP , "nod "},
{ aDEL    , "del "}

};


#define NUMCONVS2 7

BITTXT Attr2Text2[] = {

{ aHUB, "hub "},
{ aXMA, "xma "},
{ aHIR, "hir "},
{ aCOV, "cov "},
{ aSIG, "sig "},
{ aLET, "let "},
{ aFAX, "fax "}

};

// A variable for storing info for command line operations (like POST).

COMMANDINFO CommandInfo;

// Lastread pointer in memory:

long MemLastRead;



void GetConfig(char *cfgname);      /* proto from f'ion in config.c */

void ScanArea      (void);
void readparms     (int argc, char *argv[]);
void showparms     (void);
void logit         (char *line);
int  theymatch     (MASKLIST *curmask);
int  XmaskMatch    (XMASK *xmask);

int CheckAttribs(ATTRLIST *attrlist, dword attr1, dword attr2);
int MatchORAttr(dword attr1, dword attr2, ATTRLIST *current);

int  SimpleCheckName (NAMELIST *root, char *findwhere);
int  MatchOR         (NAMELIST *root, char *findwhere);
int  MatchElement    (char *what, char *where);

int  SimpleCheckAddress  (ADDRLIST *root, NETADDR *findwhere);
int  MatchORaddress      (ADDRLIST *root, NETADDR *findwhere);
int  MatchAddressElement (NETADDR *what, int not, NETADDR *where);

int  CheckBody        (XMASK *xmask);
int  MatchBodyElement (char *what, char *where);
int  MatchORbody      (NAMELIST *root, RAWBLOCK *fblk);
void ChopBody         (RAWBLOCK *blk, dword lines);

void mainprocess   (MASKLIST *curmask, dword current);
int  process       (MASKLIST *curmask, dword current);
int  matchaddress  (NETADDR *msg, NETADDR *mask);

int  copymsg       (char * todir);
int  echocopymsg   (char * todir, char * seenby, NETADDR * address);
void rewrite       (MASK * writemask, dword current, int uucp);
void changepath    (dword current, char * newpath, int movefiles);

int clean_origin(char *msgbuf, char **startorig);

void   dumpconfig(void);
char * getattribs(dword attr1, dword attr2);
void   verboze(MASKLIST *curmask);
void   ShowMask(MASK *mask);
void   ShowXMask(XMASK *xmask);
void   ShowNameList(NAMELIST *first);
void   ShowAddressList(ADDRLIST *first);

void pascal normalize(char *s);
void do_print(FILE *outfile, MIS *hdr, LINE *firstline, int writewhat);

char * attr_to_txt(dword attr1, dword attr2);
void   MakeSem(char *filename, dword current);

char * CheckInternet(char *kludges, MIS *hdr);

void beginendlog(int what);
void jamlog(char *areadir, MSG *areahandle, int echo);

int checknode     (NETADDR *addr);
int checknodelist (NETADDR *tmpaddr);

// Nodelist cache handling.

void OpenCache  (void);
void CloseCache (void);
void AddCache   (NETADDR *address, int result);
int  CheckCache (NETADDR *address);

char * ExpandMacros(char *s);
void   KillAttaches(void);
char   GetFlavour(char *s);
MSG *  OpenArea(char *todir, int isecho);
void   StripPercent(char *s);
char * ExpandAttachesRequests(void);


#define LOGBEGIN 0   // Defines for beginendlog() above..
#define LOGEND   1

// =============== Guest prototypes from Nodelist stuff ===================

int ver7find     (NETADDR *opus_addr);
int getFDnode    (NETADDR *address  );
int validaddress (word czone,word cnet,word cnode);

// ========================================================================

void main(int argc, char *argv[])
{
   time_t begin, end;
   struct _minf   minf;
   char drive[MAXDRIVE], dir[MAXDIR];
   AREALIST *maskarea;

   setvbuf(stdout, NULL, _IONBF, 0);

   time(&begin);
   memset(&cfg, '\0', sizeof(CFG));
   memset(&CommandInfo, '\0', sizeof(COMMANDINFO));

   fnsplit(argv[0], drive, dir, NULL, NULL);
   sprintf(cfg.homedir, "%s%s", drive, dir);
   Strip_Trailing(cfg.homedir, '\\');

   #ifdef __OS2__
    printf("\nNetmgr/2 " VERSION "  (c) 1992-'96  Gerard van Essen (2:280/408)\n\n");
   #endif
   #if defined(__386__) && !defined(__NT__) && !defined(__OS2__)
    printf("\nNetmgr/386 " VERSION "  (c) 1992-'96  Gerard van Essen (2:280/408)\n\n");
   #endif
   #ifdef __NT__
    printf("\nNetmgr/NT " VERSION "  (c) 1992-'96  Gerard van Essen (2:280/408)\n\n");
   #endif
   #ifdef __DOS__
    printf("\nNetmgr " VERSION "  (c) 1992-'96  Gerard van Essen (2:280/408)\n\n");
   #endif

   readparms(argc, argv);

   GetConfig(cfgname);

   /* Always registered */

   cfg.registered = 1;

   if (*cfg.registername)
       printf("\nRegistered to: \"%s\"\n\n", cfg.registername);

   if(debug) dumpconfig();

   memset(&minf, '\0', sizeof(struct _minf));
   minf.def_zone    = cfg.homeaddress[0].zone;
   minf.req_version = 0;

   MsgOpenApi(&minf, "", cfg.useflags, cfg.hudsonpath);

   OpenCache();

   if ( cfg.logfile[0] != '\0')
      {
      if ( (log = sopen(cfg.logfile, O_CREAT | O_APPEND | O_RDWR | O_TEXT, SH_DENYWR, S_IWRITE | S_IREAD)) == -1)
         {
         printf("Can't open logfile! (%s)\n", cfg.logfile);
         dolog = 0;
         }
      else
          {
          dolog = 1;
          beginendlog(LOGBEGIN);
         }
      }

   /* Now process all areas... */

   if(CommandInfo.CommandMode == 0)
     {
     for(thisarea=cfg.firstarea; thisarea; thisarea=thisarea->next)
         {
         if(thisarea->firstmask == NULL) // Get pointer to masks for this dir.
           {
           maskarea = thisarea->next;

           while(maskarea && (maskarea->firstmask == NULL))
                 maskarea = maskarea->next;

           if(maskarea && (maskarea->firstmask != NULL))
              thisarea->firstmask = maskarea->firstmask;
           else
             {
             print_and_log("ScanDir without Mask/Action!\n");
             exit(254);
             }
           }
         ScanArea();
         }
     }
   else
     {
     ProcessCommand();
     }

   CloseCache();

   time(&end);

   if((lookups + cachehits) > 0)
     print_and_log("Total nodelist lookups: %d (%d diskreads, %d cachehits).\n", lookups+cachehits, lookups, cachehits);

   printf("\nDone! Total runtime: %d seconds.\n", end - begin);

   beginendlog(LOGEND);

   if (dolog)
      close(log);

   MsgCloseApi();

   exit(didsomething);

}

// ============================================================

void ScanArea(void)
{

   char *areadir;
   word type;
   MASKLIST *curmask;
   int readkludges = 0;
   char *tmpkludge = NULL;
   dword current, next=0, inithigh=0, uid;
   char dir[120];
   struct tm tmbuf;


   strcpy(dir, thisarea->dir);

   MemLastRead = -1;

   /* Get area type (Squish or *.MSG) */

   switch(dir[0])
     {
     case '$':
        type = MSGTYPE_SQUISH | MSGTYPE_NET;
        areadir = dir + 1; /* Skip the $ */
        readkludges = 1;   /* We need to read kludges, for ^AFLAGS ! */
        break;

     case '!':
        type = MSGTYPE_JAM | MSGTYPE_NET;
        areadir = dir + 1; /* Skip the ! */
        break;

     case '#':
        type = MSGTYPE_HMB | MSGTYPE_NET;
        areadir = dir + 1; /* Skip the # */
        readkludges = 1;
        break;

     default:
        type = MSGTYPE_SDM | MSGTYPE_NET;
        areadir = dir;
        readkludges = 1;
        MemLastRead = get_last_sdm(areadir);  // So it's not -1 anymore and we adapt it.
        break;
     }

   /* Open the area */

   if( (msg.areahandle=MsgOpenArea(areadir, MSGAREA_CRIFNEC, type)) == NULL)
     {
     print_and_log("Can't open area! (%s)\n", areadir);
     return;
     }

   if(MsgLock(msg.areahandle) == -1)
      {
      print_and_log("Can't lock area! (%s)\n", areadir);
      MsgCloseArea(msg.areahandle);
      return;
      }

   if (MsgGetNumMsg(msg.areahandle) == 0)
     {
     print_and_log("Area (%s) is empty\n", areadir);
     MsgCloseArea(msg.areahandle);
     return;
     }

   print_and_log("Scanning %s (%d msgs)\n", areadir, MsgGetNumMsg(msg.areahandle));

   /* Now get the lowest message number */

   if( (current = MsgUidToMsgn(msg.areahandle, 1, UID_NEXT)) == 0)
     {
     print_and_log("Can't find lowest msg?! Corrupt? Exit..\n");
     return;
     }

   if( (inithigh = MsgMsgnToUid(msg.areahandle, MsgGetHighMsg(msg.areahandle))) == 0)
     {
     print_and_log("Can't find highest msg?! Corrupt? Exit..\n");
     return;
     }

   for( ; current && (next <= inithigh); current = MsgUidToMsgn(msg.areahandle, next, UID_NEXT))
     {
     msg.msghandle=MsgOpenMsg(msg.areahandle, MOPEN_READ, current);
     msg.body    = NULL;
     msg.kludges = NULL;
     msg.fblk    = NULL;
     msg.bodyread   = 0;
     msg.body_len   = 0;
     msg.kludge_len = 0;
     msg.fromlisted = -1;
     msg.tolisted   = -1;
     tmpkludge      = NULL;
     FreeMIS(&msg.mis);

     next = MsgMsgnToUid(msg.areahandle, current) + 1; /* save this for later */

     if(msg.msghandle == NULL)
       {
       print_and_log("Error opening msg #%d!\n", (int) current);
       if((int) current == (int) -1)
          exit(254);
       continue;
       }

     msg.kludge_len = MsgGetCtrlLen(msg.msghandle);

     if (msg.kludge_len == (dword) -1L)
         msg.kludge_len  = (dword) 0L;

     /* Get the required memory to store ctrlinfo */

     if (msg.kludge_len)
        msg.kludges = mem_calloc(1, (unsigned) msg.kludge_len + 1);
     else msg.kludges = NULL;

     if(MsgReadMsg(msg.msghandle, &msg.mis, 0, 0, NULL, msg.kludge_len, msg.kludges) == -1)
       {
       print_and_log("Error reading header of msg #%d!\n", (int) current);
       MsgCloseMsg(msg.msghandle);
       continue;
       }

     if(!debug)
        {
        if(!quiet && !(current % 5))
           {
             printf("Message number: %d\r", (int) current);
             fflush(stdout);
           }
        }
     else
        {
        uid = MsgMsgnToUid(msg.areahandle, current);
        printf("Message number: %d (UID: %lu) \n", (int) current, uid);

        printf("From: %s (%d:%d/%d.%d)\n", msg.mis.from,
                                        msg.mis.origfido.zone,
                                        msg.mis.origfido.net,
                                        msg.mis.origfido.node,
                                        msg.mis.origfido.point);

        printf("To  : %s (%d:%d/%d.%d)\n", msg.mis.to,
                                        msg.mis.destfido.zone,
                                        msg.mis.destfido.net,
                                        msg.mis.destfido.node,
                                        msg.mis.destfido.point);

        if(msg.mis.attached || msg.mis.requested)
          {
          printf("File: %s\n", ExpandAttachesRequests());
          if(msg.mis.subj[0] != '\0')
            printf("Subj: %s\n", msg.mis.subj);
          }
        else
          printf("Subj: %s\n", msg.mis.subj);

        printf("Attr: %s\n", getattribs(msg.mis.attr1, msg.mis.attr2));

        _localtime((time_t *)&msg.mis.msgwritten, &tmbuf);
        printf("Date written  : %s", msg.mis.msgwritten ? asctime(&tmbuf) : "-\n");

        _localtime((time_t *)&msg.mis.msgprocessed, &tmbuf);
        printf("Date processed: %s", msg.mis.msgprocessed ? asctime(&tmbuf) : "-\n");

         _localtime((time_t *)&msg.mis.msgreceived, &tmbuf);
        printf("Date received : %s\n", msg.mis.msgreceived ? asctime(&tmbuf) : "-\n");
        }

     for(curmask = thisarea->firstmask; curmask; curmask = curmask->next)
       {
       if(theymatch(curmask))       /* cfg.header is global */
          {
          if(debug) printf("Found a match on mask #%d!\n", curmask->no);

          didsomething=1;

          mainprocess(curmask, current);

          if(debug) printf("\n");

          break;  // stop scanning masks for this message.
          }
       }

     if(msg.msghandle) MsgCloseMsg(msg.msghandle); /* May be closed already for kill */
     if(msg.kludges) mem_free(msg.kludges);
     if(msg.body)    mem_free(msg.body);
     if(msg.fblk)    FreeBlocks(msg.fblk);
     FreeMIS(&msg.mis);
     }

   if(thisarea->renumber == 1 && strchr("!$#", thisarea->dir[0]) == NULL)
     MemLastRead = RenumArea(msg.areahandle, MemLastRead);

   MsgCloseArea(msg.areahandle);

   // Update the thing if we have to (*.MSG only).
   if(MemLastRead != -1)
      put_last_sdm(areadir, MemLastRead);

}

// ============================================================

void readparms (int argc, char *argv[])
{
    int i;
    char *p;
    char drive[MAXDRIVE], dir[MAXDIR];

    for(i=1;i<argc;i++) {

        p=argv[i];
        if(*p == '-' || *p == '/')
          {
          switch(tolower(*(++p))) {

                case 'c':                 /* Configfile */
                     strncpy(cfgname,++p,99);
                     fnsplit(cfgname, drive, dir, NULL, NULL);
                     sprintf(cfg.cfgdir, "%s%s", drive, dir);
                     Strip_Trailing(cfg.cfgdir, '\\');
                     break;

                case 'x':                 // XMASK for POST
                     strncpy(CommandInfo.xmask, ++p, 39);
                     break;

                case 'p':                 // Password
                     strncpy(CommandInfo.password, ++p, 39);
                     break;

                case 'f':                 // Filename for body
                     strncpy(CommandInfo.file, ++p, 119);
                     break;

                case 'a':                 // Area to post in
                     strncpy(CommandInfo.area, ++p, 119);
                     break;

                case '#':
                     if(sscanf(++p, "%hu:%hu/%hu.%hu",
                                     &CommandInfo.address.zone,
                                     &CommandInfo.address.net,
                                     &CommandInfo.address.node,
                                     &CommandInfo.address.point) < 3)
                        {
                        print_and_log("Incorrect address given!\n");
                        exit(254);
                        }
                     break;

                case 's':
                     CommandInfo.flavour = GetFlavour(++p);
                     break;

                case 'n':
                     CommandInfo.newflavour = GetFlavour(++p);
                     break;

                case 'e':                 // e == is Echomail area
                     CommandInfo.isecho = 1;
                     break;

                case 'd':
                     debug=1;
                     break;

                case 'q':
                     quiet=1;
                     break;

                default : showparms();
            }
          }
        else if(!IsCommand(p))
          showparms();
    }
}

// ============================================================
/* Show command line parms if I can't make sense out of them */

void showparms(void)
{
   #ifndef __OS2__
   printf("\nUsage:   NETMGR [-Cconfigfile] [-D]\n\n");
   #else
   printf("\nUsage:   NETMGRP [-Cconfigfile] [-D]\n\n");
   #endif
   printf("Use -D to specify the 'debug' mode with extra info while running\n");
   printf("Example: NETMGR -Cc:\\Squish\\mycfg.cfg\n");
   exit(254);

}

// ============================================================


void print_and_log(char *line, ...)
{
   va_list argptr;
   char temp[200];
   int howmuch;

   va_start(argptr, line);
   howmuch = vsprintf(temp, line, argptr);
   va_end(argptr);

   if(!quiet)
     {
     printf("                                                 \r");
     printf(temp);
     fflush(stdout);
     }

   logit(temp);
}


// ============================================================
/* Write a line to the logfile (if logging is on..) */

void logit(char *line)
{
  time_t ltime;
  struct tm *tp;
  char temp[160];

  if (!dolog) return;

  (void)time (&ltime);
  tp = localtime (&ltime);

  if(cfg.frodolog)
    sprintf(temp, "+ %2d:%02d:%02d  %s", tp->tm_hour, tp->tm_min, tp->tm_sec, line);
  else
    sprintf (temp, "  %02i %03s %02i:%02i:%02i NMGR %s",
        tp->tm_mday, mtext[tp->tm_mon], tp->tm_hour, tp->tm_min, tp->tm_sec,
        line);

  write(log, temp, strlen(temp));

//  sprintf(temp, "Mem: %ld Stack: %d Heap: %d", coreleft(), (int) stackavail(), _heapchk());
//  write(log, temp, strlen(temp));

}

// ==============================================

void beginendlog(int what)
{
  time_t ltime;
  struct tm *tp;
  char temp[160];
  static char wdays[][4] =
  {
   "Sun",
   "Mon",
   "Tue",
   "Wed",
   "Thu",
   "Fri",
   "Sat"
  };

  if (!dolog) return;

  (void)time (&ltime);
  tp = localtime (&ltime);


  if(what == LOGEND)
    {
    #if !defined(__OS2__) && !defined(__NT__)
     #ifdef __386__
      sprintf(temp, "End, NMGR/386 " VERSION " %s(%u K) \n", REGISTERED ? "" : "[UNREGISTERED] ", coreleft()/1024);
     #else
      sprintf(temp, "End, NMGR " VERSION " %s(%u K) \n", REGISTERED ? "" : "[UNREGISTERED] ", coreleft()/1024);
     #endif
    #else
     #ifndef __NT__
      sprintf(temp, "End, NMGR/2 " VERSION " %s\n", REGISTERED ? "" : "[UNREGISTERED]");
     #else
      sprintf(temp, "End, NMGR/NT" VERSION " %s\n", REGISTERED ? "" : "[UNREGISTERED]");
     #endif
    #endif
    }
  else
    {
    write(log, "\n", 1);
    if(cfg.frodolog)
      {
      #if !defined(__OS2__) && !defined(__NT__)
       #ifdef __386__
        sprintf(temp, "----------  %s %02d %s %2d, NMGR/386 " VERSION " %s(%u K) \n", wdays[tp->tm_wday], tp->tm_mday, mtext[tp->tm_mon], tp->tm_year, REGISTERED ? "" : "[UNREGISTERED] ", coreleft()/1024);
       #else
        sprintf(temp, "----------  %s %02d %s %2d, NMGR " VERSION " %s(%u K) \n", wdays[tp->tm_wday], tp->tm_mday, mtext[tp->tm_mon], tp->tm_year, REGISTERED ? "" : "[UNREGISTERED] ", coreleft()/1024);
       #endif
      #else
       #ifndef __NT__
        sprintf(temp, "----------  %s %02d %s %2d, NMGR/2 " VERSION " %s\n", wdays[tp->tm_wday], tp->tm_mday, mtext[tp->tm_mon], tp->tm_year, REGISTERED ? "" : "[UNREGISTERED]");
       #else
        sprintf(temp, "----------  %s %02d %s %2d, NMGR/NT " VERSION " %s\n", wdays[tp->tm_wday], tp->tm_mday, mtext[tp->tm_mon], tp->tm_year, REGISTERED ? "" : "[UNREGISTERED]");
       #endif
      #endif
      write(log, temp, strlen(temp));
      return;
      }
    else
      {
      #if !defined(__OS2__) && !defined(__NT__)
       #ifdef __386__
        sprintf(temp, "Begin, NMGR/386 " VERSION " %s(%u K) \n", REGISTERED ? "" : "[UNREGISTERED] ", coreleft()/1024);
       #else
        sprintf(temp, "Begin, NMGR " VERSION " %s(%u K) \n", REGISTERED ? "" : "[UNREGISTERED] ", coreleft()/1024);
       #endif
      #else
       #ifndef __NT__
        sprintf(temp, "Begin, NMGR/2 " VERSION " %s\n", REGISTERED ? "" : "[UNREGISTERED]");
       #else
        sprintf(temp, "Begin, NMGR/NT " VERSION " %s\n", REGISTERED ? "" : "[UNREGISTERED]");
       #endif
      #endif
      }
    }

   logit(temp);
}

/* ------------------------------------------------------- */

int theymatch(MASKLIST *curmask)
{
   int not = 0, attreqmatch;
   char *string, *attreq;

   if(curmask->xmask)
      return XmaskMatch(curmask->xmask);

   /* Check for attribs that *must* be there */

   if( (msg.mis.attr1 & curmask->mask.yes_attribs1) != curmask->mask.yes_attribs1)
      return 0;

   if( (msg.mis.attr2 & curmask->mask.yes_attribs2) != curmask->mask.yes_attribs2)
      return 0;

   /* Check for attribs that must *not* be there */

   if(msg.mis.attr1 & curmask->mask.no_attribs1)
      return 0;

   if(msg.mis.attr2 & curmask->mask.no_attribs2)
      return 0;

   if(curmask->mask.fromname[0] != '*')
      {
      not = 0;
      string = curmask->mask.fromname;
      if(*string == '!')
         {
         not = 1;
         string++;
         }

      if(*string != '~')  // Looking for exact match!
        {
        if(strcmpi(msg.mis.from, string) != 0)  // Not found!
           {
           if(not) // No match found, which is what we wanted
              /* Nothing */ ;
           else   // No match found, and we wanted one.
              return 0;
           }
        else  // We did find an exact match!
           {
           if(not)
              return 0;
           else
             /* Nothing */ ;
           }
        }
      else // Substring, use stristr...
        {
        string++;  // Skip over ~ token..

        if(stristr(msg.mis.from, string) == NULL)  // Not found!
           {
           if(not) // No match found, which is what we wanted
              /* Nothing */ ;
           else   // No match found, and we wanted one.
              return 0;
           }
        else  // We did find a substring match!
           {
           if(not)         // We didn't want one
              return 0;
           else            // We wanted one!
             /* Nothing */ ;
           }

        }
      }


   if(curmask->mask.toname[0] != '*')
      {
      not = 0;
      string = curmask->mask.toname;
      if(*string == '!')
         {
         not = 1;
         string++;
         }

      if(*string != '~')  // Looking for exact match!
        {
        if(strcmpi(msg.mis.to, string) != 0)  // Not found!
           {
           if(not) // No match found, which is what we wanted
              /* Nothing */ ;
           else   // No match found, and we wanted one.
              return 0;
           }
        else  // We did find an exact match!
           {
           if(not)
              return 0;
           else
             /* Nothing */ ;
           }
        }
      else // Substring, use stristr...
        {
        string++;  // Skip over ~ token..

        if(stristr(msg.mis.to, string) == NULL)  // Not found!
           {
           if(not) // No match found, which is what we wanted
              /* Nothing */ ;
           else   // No match found, and we wanted one.
              return 0;
           }
        else  // We found a substring match!
           {
           if(not)         // We didn't want one
              return 0;
           else            // We wanted one!
             /* Nothing */ ;
           }

        }
      }


   if(curmask->mask.subject[0] != '*')
      {
      not = 0;
      string = curmask->mask.subject;
      if(*string == '!')
         {
         not = 1;
         string++;
         }

      attreq = ExpandAttachesRequests();

      if(*string != '~')  // Looking for exact match!
        {
        attreqmatch = strcmpi(attreq, string);
        mem_free(attreq);

        if(strcmpi(msg.mis.subj, string) != 0 &&
           attreqmatch != 0)                                  // Not found!
           {
           if(not) // No match found, which is what we wanted
              /* Nothing */ ;
           else   // No match found, and we wanted one.
              return 0;
           }
        else  // We did find an exact match!
           {
           if(not)
              return 0;
           else
             /* Nothing */ ;
           }
        }
      else // Substring, use stristr...
        {
        string++;  // Skip over ~ token..

        attreqmatch = (stristr(attreq, string) == NULL) ? 0 : 1;
        mem_free(attreq);

        if(stristr(msg.mis.subj, string) == NULL &&
           attreqmatch == 0)                                    // Not found!
           {
           if(not) // No match found, which is what we wanted
              /* Nothing */ ;
           else   // No match found, and we wanted one.
              return 0;
           }
        else  // We did find a substring match!
           {
           if(not)         // We didn't want one
              return 0;
           else            // We wanted one!
             /* Nothing */ ;
           }
        }
      }


   if(!matchaddress(&msg.mis.origfido, &curmask->mask.fromaddress))
      {
      // No match found:
      if(curmask->mask.fromnot)  // We dont' want one!
         /* Nothing */ ;
      else        // We wanted a match!
         return 0;
      }
   else     // Addresses match!
     {
      // Match found:
      if(curmask->mask.fromnot)  // We dont' want one!
         return 0;
      else        // We wanted a match!
         /* Nothing */ ;

     }

   if(!matchaddress(&msg.mis.destfido, &curmask->mask.toaddress))
      {
      // No match found:
      if(curmask->mask.tonot)  // We dont' want one!
         /* Nothing */ ;
      else        // We wanted a match!
         return 0;
      }
   else     // Addresses match!
     {
      // Match found:
      if(curmask->mask.tonot)  // We dont' want one!
         return 0;
      else        // We wanted a match!
         /* Nothing */ ;

     }

   // Now we check for nodelist stuff...
   if(curmask->mask.tolisted != 0)     // So we want to check..
     {
     if(msg.tolisted == -1)            // No lookup done yet.
        msg.tolisted = checknode(&msg.mis.destfido);

     if(curmask->mask.tolisted == 1 && msg.tolisted == 0)
        return 0;

     if(curmask->mask.tolisted == -1 && msg.tolisted == 1)
        return 0;
      }

   // Now we check for nodelist stuff...
   if(curmask->mask.fromlisted != 0)     // So we want to check..
     {
     if(msg.fromlisted == -1)            // No lookup done yet.
        msg.fromlisted = checknode(&msg.mis.origfido);

     if(curmask->mask.fromlisted == 1 && msg.fromlisted == 0)
        return 0;

     if(curmask->mask.fromlisted == -1 && msg.fromlisted == 1)
        return 0;
      }


   return 1;
}


/* ------------------------------------------------------- */

int XmaskMatch(XMASK *xmask)
{
   char *string, *attreq;
   int retval;

   /* Check for attribs that *must* be there */

   if(xmask->attribs)
     {
     if(CheckAttribs(xmask->attribs, msg.mis.attr1, msg.mis.attr2) == 0)
       return 0;
     }

   if(xmask->fromname)
     {
     if(SimpleCheckName(xmask->fromname, msg.mis.from) == 0)
       return 0;
     }

   if(xmask->toname)
     {
     if(SimpleCheckName(xmask->toname, msg.mis.to) == 0)
       return 0;
     }

   if(xmask->subj)  // Take care of subject line, *and* names of attaches/request
     {
     attreq = ExpandAttachesRequests();

     // Make space for strings, plus an attaching space and a trailing '\0'
     string = mem_calloc(1, strlen(attreq) + strlen(msg.mis.subj) + 2);
     sprintf(string, "%s%s", attreq, msg.mis.subj);
     mem_free(attreq);

     retval = SimpleCheckName(xmask->subj, string);
     mem_free(string);

     if(retval == 0)
       return 0;
     }

   if(xmask->kludge)
     {
     if(SimpleCheckName(xmask->kludge, msg.kludges) == 0)
       return 0;
     }

   if(xmask->orig)
     {
     if(SimpleCheckAddress(xmask->orig, &msg.mis.origfido) == 0)
       return 0;
     }

   if(xmask->dest)
     {
     if(SimpleCheckAddress(xmask->dest, &msg.mis.destfido) == 0)
       return 0;
     }

   if(xmask->olderwritten != 0 && msg.mis.msgwritten >= xmask->olderwritten)
     return 0;

   if(xmask->olderprocessed != 0 && msg.mis.msgprocessed >= xmask->olderprocessed)
     return 0;

   if(xmask->olderread != 0 && msg.mis.msgreceived >= xmask->olderread)
     return 0;

   // Now we check for nodelist stuff...

   if(xmask->tolisted != 0)     // So we want to check..
     {
     if(msg.tolisted == -1)            // No lookup done yet.
        msg.tolisted = checknode(&msg.mis.destfido);

     if(xmask->tolisted == 1 && msg.tolisted == 0)
        return 0;

     if(xmask->tolisted == -1 && msg.tolisted == 1)
        return 0;
      }

   if(xmask->fromlisted != 0)     // So we want to check..
     {
     if(msg.fromlisted == -1)            // No lookup done yet.
        msg.fromlisted = checknode(&msg.mis.origfido);

     if(xmask->fromlisted == 1 && msg.fromlisted == 0)
        return 0;

     if(xmask->fromlisted == -1 && msg.fromlisted == 1)
        return 0;
      }

   // Finally, we have to check the body.

   if(xmask->body)
     {
     if(CheckBody(xmask) == 0)
       return 0;
     }

   return 1;
}

// ==============================================================

int CheckAttribs(ATTRLIST *attrlist, dword attr1, dword attr2)
{
  ATTRLIST *current;

  for(current=attrlist; current; current = current->or)
    {
    if(MatchORAttr(attr1, attr2, current) == 1)
      return 1;
    }

  return 0;
}

// ==============================================================

int MatchORAttr(dword attr1, dword attr2, ATTRLIST *current)
{

  if((attr1 & current->yes_attribs1) != current->yes_attribs1)
    return 0;

  if((attr2 & current->yes_attribs2) != current->yes_attribs2)
    return 0;

  /* Check for attribs that must *not* be there */

  if(attr1 & current->no_attribs1)
    return 0;

  if(attr2 & current->no_attribs2)
    return 0;

  return 1;

}

// ==============================================================

int SimpleCheckName(NAMELIST *root, char *findwhere)
{
   NAMELIST *current;

   for(current = root; current; current=current->and)
     {
     if(MatchOR(current, findwhere) == 0)  // No match here!
       return 0;
     }

   // If we get here, all if it seemed to match.

   return 1;

}

// ==============================================================

int MatchOR(NAMELIST *root, char *findwhere)
{
   NAMELIST *current;

   for(current = root; current; current=current->or)
     {
     if(MatchElement(current->name, findwhere) == 1)  // Match here!
       return 1;
     }

   // If we get here, no match was found.

   return 0;

}

// ==============================================================

int SimpleCheckAddress(ADDRLIST *root, NETADDR *findwhere)
{
   ADDRLIST *current;

   for(current = root; current; current=current->and)
     {
     if(MatchORaddress(current, findwhere) == 0)  // No match here!
       return 0;
     }

   // If we get here, all if it seemed to match.

   return 1;

}

// ==============================================================

int MatchORaddress(ADDRLIST *root, NETADDR *findwhere)
{
   ADDRLIST *current;

   for(current = root; current; current=current->or)
     {
     if(MatchAddressElement(&current->addr, current->not, findwhere) == 1)  // Match here!
       return 1;
     }

   // If we get here, no match was found.

   return 0;

}

// ==============================================================

int MatchAddressElement(NETADDR *what, int not, NETADDR *where)
{

  if(!matchaddress(where, what))
     {
     // No match found:
     if(not)         // We dont' want one!
        return 1;
     else            // We wanted a match!
        return 0;
     }
  else     // Addresses match!
    {
     // Match found:
     if(not)         // We dont' want one!
        return 0;
     else            // We wanted a match!
        return 1;

    }

}

// ==============================================================


int MatchElement(char *what, char *where)
{
  int not = 0;

  if(*what == '!')
     {
     not = 1;
     what++;
     }

  if(!where)      // NULL pointer given, so element non-existant or empty
    {
    if(not)
      return 1;
    else
      return 0;
    }


  if(*what != '~')  // Looking for exact match!
    {
    if(strcmpi(where, what) != 0)  // Not found!
       {
       if(not)             // No match found, which is what we wanted
          return 1;
       else   // No match found, and we wanted one.
          return 0;
       }
    else  // We did find an exact match!
       {
       if(not)
          return 0;
       else
          return 1;
       }
    }
  else // Substring, use stristr...
    {
    what++;  // Skip over ~ token..

    if(stristr(where, what) == NULL)  // Not found!
       {
       if(not)          // No match found, which is what we wanted
         return 1;
       else             // No match found, and we wanted one.
         return 0;
       }
    else  // We did find a substring match!
       {
       if(not)         // We didn't want one
         return 0;
       else            // We wanted one!
         return 1;
       }

    }

}

// ==============================================================

int matchaddress(NETADDR *msg, NETADDR *mask)
{

   if(
       (mask->zone != (word) -99) &&
       (mask->zone != msg->zone)
     )
     return 0;

   if(
       (mask->net != (word) -99) &&
       (mask->net != msg->net)
     )
     return 0;

   if(
       (mask->node != (word) -99) &&
       (mask->node != msg->node)
     )
     return 0;

   if(
       (mask->point != (word) -99) &&
       (mask->point != msg->point)
     )
     return 0;

   return 1;
}


// ================================================================

void mainprocess(MASKLIST *curmask, dword current)
{
   int retval = 0;
   dword curuid;

   while(curmask->action == GETNEXT)   // Find where the action belonging to
     {                                 // mask is defined.
     if(curmask->next)
       curmask = curmask->next;
     else
       return;
     }

   while(curmask)       // Now perform all actions defined for this mask.
      {
      // Between each Action, we drop an 'anchor' and re-orientate. This
      // is needed in Squish areas, where Bouncing and stuff adds messages
      // to an area and therefore messes with our 'current message' pointer
      // when there is a 'max msgs' limit set..

      curuid = MsgMsgnToUid(msg.areahandle, current);

      if( (retval=process(curmask, current)) > 0)
        break;

      current = MsgUidToMsgn(msg.areahandle, curuid, UID_EXACT);
      curmask = curmask->nextaction;
      }

   return;

}


/* ---------------------------------------------------------- */
/* ret > 0 means stop scanning list of masks for this message */
/* ---------------------------------------------------------- */


int process(MASKLIST *curmask, dword current)
{
   char *expanded;

   if(curmask->action == REWRITE)
      {
      rewrite((MASK *)curmask->value, current, 0); /* 0 == no uucp */
      return 0;
      }

   if(curmask->action == CHANGEPATH)
      {
      changepath(current, (char *)curmask->value, 0);
      return 0;
      }

   if(curmask->action == CHANGEPATHMOVE)
      {
      changepath(current, (char *)curmask->value, 1);
      return 0;
      }

   if(curmask->action == DISPLAY)
      {
      expanded = ExpandMacros((char *)curmask->value);
      StripPercent(expanded);
      print_and_log(expanded);
      mem_free(expanded);
      return 0;
      }

   if(curmask->action == RUNEXTERNAL)
      {
      runexternal(curmask->bodyfile, (char *)curmask->value, thisarea->dir);
      return 0;
      }

   if(curmask->action == UUCP)
      {
      rewrite((MASK *)curmask->value, current, 1); /* 1 == uucp rewrite */
      return 0;
      }

   if(curmask->action == WRITE)
      {
      writemsg(current, (char *)curmask->value, WRITE_ALL);
      return 0;
      }

   if(curmask->action == HDRFILE)
      {
      writemsg(current, (char *)curmask->value, WRITE_HEADER);
      return 0;
      }

   if(curmask->action == ADDNOTE)
      {
      AddNote((char *)curmask->value, current, 0);
      return 0;
      }

   if(curmask->action == BOUNCE      ||
      curmask->action == HDRBOUNCE   ||
      curmask->action == EMPTYBOUNCE ||
      curmask->action == FORWARD     )
      {
      bounce(curmask->bodyfile, current, &curmask->origaddr, curmask->action, (MASK *)curmask->value, 0, curmask->destarea);
      return 0;
      }

   if(curmask->action == XBOUNCE      ||
      curmask->action == XHDRBOUNCE   ||
      curmask->action == XEMPTYBOUNCE )
      {
      if(REGISTERED)
         bounce(curmask->bodyfile, current, &curmask->origaddr, curmask->action, (MASK *)curmask->value, 1, curmask->destarea);
      else
         print_and_log("eXtended bounce actions only for registered users!\n");
      return 0;
      }

   if(curmask->action == PACKMAIL)
      {
      // Packmail returs -1 on error, 1 if we want to stop (msg killed). 0 = OK.
      if(PackMail(&curmask->origaddr, &curmask->destaddr, current, curmask->password) != 0)
         return 1;
      return 0;
      }

   if(curmask->action == MOVEMAIL)
      {
      // Movemail returs -1 on error, 1 if we want to stop (msg killed). 0 = OK.
      if(MoveMail(&curmask->origaddr, &curmask->destaddr, (char *)curmask->value, current, curmask->password) != 0)
         return 1;
      return 0;
      }

   if(curmask->action == MAKEMSG)
      {
      bounce((char *)curmask->bodyfile, current, NULL, curmask->action, (MASK *)curmask->value, 0, curmask->destarea);
      return 0;
      }

   if(curmask->action == SEMA)
      {
      MakeSem((char *)curmask->value, current);
      return 0;
      }

   if( (curmask->action == COPY) || (curmask->action == MOVE) )
     {
     if(copymsg((char *)curmask->value) != 0)
       {
       print_and_log("Error copying msg#%d to %s!\n", (int) current, (char *)curmask->value);
       return 1;
       }
     else
       {
       print_and_log("%s msg#%d to %s.\n", curmask->action == COPY ? "Copied" : "Moved", (int) current, (char *)curmask->value);
       }

     if(curmask->action == COPY)
        return 0;

     /* else it's move and we drop to DELETE below.. */
     }

   if( (curmask->action == ECHOCOPY) || (curmask->action == ECHOMOVE) )
     {
     if(echocopymsg((char *)curmask->value, curmask->seenby, &curmask->origaddr) != 0)
       {
       print_and_log("Error copying msg#%d to %s!\n", (int) current, (char *)curmask->value);
       return 1;
       }
     else
       {
       print_and_log("%s msg#%d to %s.\n", curmask->action == ECHOCOPY ? "Copied" : "Moved", (int) current, (char *)curmask->value);
       }

     if(curmask->action == ECHOCOPY)
        return 0;

     /* else it's move and we drop to DELETE below.. */
     }


   if(
       (curmask->action == DELETE) ||
       (curmask->action == DELETEATTACH) ||
       (curmask->action == MOVE)   ||
       (curmask->action == ECHOMOVE)
     )
      {
      MsgCloseMsg(msg.msghandle);
      msg.msghandle = NULL;

      // First, we'll see if we need to adjust the lastread pointer.

      if(MemLastRead == current)
         MemLastRead = AdjustLastread(MemLastRead, msg.areahandle);

      if(MsgKillMsg(msg.areahandle, current) == -1)
         {
         print_and_log("Can't kill msg#%d!\n", (int) current);
         return 1;
         }

      if(curmask->action == DELETE || curmask->action == DELETEATTACH)
         print_and_log("Killed msg#%d.\n", (int) current);

      if(curmask->action == DELETEATTACH)
         KillAttaches();

      return 1;   /* stop scanning for this message */
      }


   return 0;
}


/* -------------------------------------------------- */
/* Return 0 on success                                */


int copymsg(char *todir)
{
   MSG *toareahandle;
   int  retval;

   if( (toareahandle=OpenArea(todir, 0)) == NULL)
     return 1;

   GetRawMsg(0L);

   msg.mis.replyto = 0;
   memset(msg.mis.replies, '\0', sizeof(msg.mis.replies));

   if(SaveMessage(toareahandle, &msg.mis, msg.fblk, msg.kludges, 0, cfg.intlforce) != -1)
      retval = 0;
   else
      retval = 1;

   jamlog(todir, toareahandle, 0);
   MsgCloseArea(toareahandle);

   return retval;

}

/* ---------------------------------------------------------- */

int echocopymsg(char *todir, char *seenby, NETADDR *address)
{
   MSG *toareahandle;
   MIS newmis;
   char *startorigin;
   char temp[80], addtomsg[300], *newkludges=NULL;
   RAWBLOCK *last=NULL;
   int result;

   if( (toareahandle = OpenArea(todir, 1)) == NULL)
     return 1;

   GetRawMsg(0L);

   if(msg.fblk)
     {
     AddToBlock(msg.fblk, "\r", 1);  // We add a '\r' here, because clean_end later NEEDS an extra character at the end!!!!
     last = JoinLastBlocks(msg.fblk, 2048);
     }

   if(last)
     {
     clean_end_message(last->txt);

     if(clean_origin(last->txt, &startorigin) == 0)   // There is an origin/tear
       *startorigin = '\0';

     last->curlen = strlen(last->txt);
     last->curend = last->txt + last->curlen;
     }
   else  // There was no body, make one 'cause we add origin and stuff!
     {
     msg.fblk = InitRawblock(2048, 2048, 64000);
     }

   CopyMIS(&msg.mis, &newmis);

   newmis.attr1 = aLOCAL;
   newmis.replyto = 0;
   memset(&newmis.replies, '\0', sizeof(newmis.replies));
   FreeStringList(newmis.seenby);
   FreeStringList(newmis.path);
   FreeStringList(newmis.via);
   newmis.seenby = newmis.path = newmis.via = NULL;

   /* prepare origin line */

   if(address->point != 0) sprintf(temp, "%d:%d/%d.%d",
                                         address->zone,
                                         address->net,
                                         address->node,
                                         address->point);

   else sprintf(temp, "%d:%d/%d", address->zone,
                                  address->net,
                                  address->node);


   #ifndef __OS2__
   sprintf(addtomsg, "\r--- NetMgr " VERSION "%s\r * Origin: %s (%s)\r", REGISTERED ? "+" : " [unreg]", cfg.origin, temp);
   #else
   sprintf(addtomsg, "\r--- NetMgr/2 " VERSION "%s\r * Origin: %s (%s)\r", REGISTERED ? "+" : " [unreg]", cfg.origin, temp);
   #endif


   if( (seenby != NULL) && (seenby[0] != '\0') )
      {
      while(*seenby == ' ') seenby++;

      while(seenby[strlen(seenby)-1] == ' ')
            seenby[strlen(seenby)-1] = '\0';

      newmis.seenby = AddToStringList(newmis.seenby, seenby, NULL, 0);
      }

   newkludges = MakeKludge(NULL, address);

   AddToBlock(msg.fblk, addtomsg, -1);

   if(SaveMessage(toareahandle, &newmis, msg.fblk, newkludges, 0, 0) == -1)
     result = 1;
   else
     result = 0;

   jamlog(todir, toareahandle, 1);
   MsgCloseArea(toareahandle);

   if(newkludges) mem_free(newkludges);

   if(msg.fblk) FreeBlocks(msg.fblk);
   msg.fblk = NULL;

   if(msg.kludges) mem_free(msg.kludges);
   msg.kludges = NULL;

   FreeMIS(&newmis);

   msg.bodyread = 0;

   return result;

}


/* ------------------------------------------------------ */
/* Send the message back, with explaining text at the top */
/* Add your own kludges, including MSGID and REPLY, if    */
/* that is applicable                                     */
/* ------------------------------------------------------ */

void bounce(char *bouncefile, dword current, NETADDR *address, int action, MASK *forwardmask, int extended, char *destarea)
{
   FILE *infile;
   char templine[256], *msgid = NULL;
   MIS  *mis = NULL;
   RAWBLOCK *add, *thisone;
   RAWBLOCK *internadd = NULL;
   int howmuch;
   char *s, *toline = NULL;
   char *expanded;
   int freemask = 0;
   MSG *destareahandle;
   char reportarea[120] = "";

   /* first read the bounce text to a buffer.. */

   add = InitRawblock(2048, 2048, 64000);

   if(action != FORWARD)
     {
     if( (infile = _fsopen(bouncefile, "r", SH_DENYNO)) == NULL)
        {
        print_and_log("Can't open bouncefile (%s)!\n", bouncefile);
        FreeBlocks(add);
        return;
        }

     if(action == XEMPTYBOUNCE && forwardmask == NULL)
       {
       if(fgets(templine, 256, infile) == NULL)
         {
         print_and_log("First line of %s does not contain a mask!\n", bouncefile);
         fclose(infile);
         FreeBlocks(add);
         return;
         }

       forwardmask = mem_calloc(1, sizeof(MASK));  // This one needs to be free'd later!
       freemask=1;
       if(fillmask(forwardmask, templine, 1) == -1)
         {
         mem_free(forwardmask);
         fclose(infile);
         FreeBlocks(add);
         return;
         }
       }

     while(fgets(templine, 256, infile))
        {
        if(templine[strlen(templine)-1] == '\n')
          {
          Strip_Trailing(templine, '\n');
          Add_Trailing(templine, '\r');
          }

        if(templine[0] != '\0')
           {
           expanded = ExpandMacros(templine);
           if(expanded)
             {
             AddToBlock(add, expanded, -1);
             mem_free(expanded);
             }
           }
        else
           AddToBlock(add, "\r", -1);
        }

     fclose(infile);
     }

   /* We have the bounce text, now lets make the header of the bounce msg */

   mis = mem_calloc(1, sizeof(MIS));

   if(action != FORWARD && action != MAKEMSG)
     {
     if(extended)
       {
       strcpy(mis->to, msg.mis.from);
       strcpy(mis->from, msg.mis.to);
       strcpy(mis->subj, msg.mis.subj);
       if(mis->subj[0] == '\0' && (msg.mis.attached || msg.mis.requested))
         strncpy(mis->subj, ExpandAttachesRequests(), 99);

       mis->destfido  = msg.mis.origfido;
       mis->origfido  = msg.mis.destfido;
       mis->attr1 = msg.mis.attr1;
       mis->attr2 = msg.mis.attr2;

       // Now plug in all the stuff that was passed through the mask..

       if(forwardmask->fromname[0] != '*') strcpy(mis->from, forwardmask->fromname);
       if(forwardmask->toname[0] != '*') strcpy(mis->to, forwardmask->toname);
       if(forwardmask->subject[0] != '*') strcpy(mis->subj, forwardmask->subject);

       if(forwardmask->fromaddress.zone != (word) -99)
          mis->origfido.zone = forwardmask->fromaddress.zone;

       if(forwardmask->fromaddress.net != (word) -99)
          mis->origfido.net = forwardmask->fromaddress.net;

       if(forwardmask->fromaddress.node != (word) -99)
          mis->origfido.node = forwardmask->fromaddress.node;

       if(forwardmask->fromaddress.node != (word) -99)
          mis->origfido.point = forwardmask->fromaddress.point;

       if(forwardmask->toaddress.zone != (word) -99)
          mis->destfido.zone = forwardmask->toaddress.zone;

       if(forwardmask->toaddress.net != (word) -99)
          mis->destfido.net = forwardmask->toaddress.net;

       if(forwardmask->toaddress.node != (word) -99)
          mis->destfido.node = forwardmask->toaddress.node;

       if(forwardmask->toaddress.node != (word) -99)
          mis->destfido.point = forwardmask->toaddress.point;

       if(forwardmask->yes_attribs1 != 0)
          mis->attr1 = forwardmask->yes_attribs1;

       if(forwardmask->yes_attribs2 != 0)
          mis->attr2 = forwardmask->yes_attribs2;
       }
     else    // it's not 'extended'
       {
       strcpy(mis->from, "NetMgr");

       if(REGISTERED)
          strcat(mis->from, "+");
       else
          strcat(mis->from, " [unreg]");

       strcpy(mis->to, msg.mis.from);
       strcpy(mis->subj, "Bounced message");

       mis->origfido = *address;

       mis->destfido.zone  = msg.mis.origfido.zone;
       mis->destfido.net   = msg.mis.origfido.net;
       mis->destfido.node  = msg.mis.origfido.node;
       mis->destfido.point = msg.mis.origfido.point;

       mis->attr1 = aLOCAL | aPRIVATE;
       }
     }
   else // It's forward or MakeMsg
     {
     strcpy(mis->from, msg.mis.from);
     strcpy(mis->to, msg.mis.to);
     strcpy(mis->subj, msg.mis.subj);
     if(mis->subj[0] == '\0' && (msg.mis.attached || msg.mis.requested))
        strncpy(mis->subj, ExpandAttachesRequests(), 99);

     mis->origfido = msg.mis.origfido;
     mis->destfido = msg.mis.destfido;
     mis->attr1 = msg.mis.attr1;
     mis->attr2 = msg.mis.attr2;
 
     if(forwardmask->fromname[0] != '*') strcpy(mis->from, forwardmask->fromname);
     if(forwardmask->toname[0] != '*') strcpy(mis->to, forwardmask->toname);
     if(forwardmask->subject[0] != '*') strcpy(mis->subj, forwardmask->subject);

     if(forwardmask->fromaddress.zone != (word) -99)
        mis->origfido.zone = forwardmask->fromaddress.zone;

     if(forwardmask->fromaddress.net != (word) -99)
        mis->origfido.net = forwardmask->fromaddress.net;

     if(forwardmask->fromaddress.node != (word) -99)
        mis->origfido.node = forwardmask->fromaddress.node;

     if(forwardmask->fromaddress.node != (word) -99)
        mis->origfido.point = forwardmask->fromaddress.point;

     if(forwardmask->toaddress.zone != (word) -99)
        mis->destfido.zone = forwardmask->toaddress.zone;

     if(forwardmask->toaddress.net != (word) -99)
        mis->destfido.net = forwardmask->toaddress.net;

     if(forwardmask->toaddress.node != (word) -99)
        mis->destfido.node = forwardmask->toaddress.node;

     if(forwardmask->toaddress.node != (word) -99)
        mis->destfido.point = forwardmask->toaddress.point;

     if(forwardmask->yes_attribs1 != 0)
        mis->attr1 = forwardmask->yes_attribs1;

     if(forwardmask->yes_attribs2 != 0)
        mis->attr2 = forwardmask->yes_attribs2;
     }

   /* Generate date now */

   mis->msgwritten=JAMsysTime(NULL);

   // Check to see if we need to match the AKA (@myaka)

   matchaka(mis);

   GetRawMsg(0L);

   if((action != BOUNCE) && (action != XBOUNCE) && (action != FORWARD))    // So it's empty or header -> dump body
     {
     FreeBlocks(msg.fblk);
     msg.fblk = NULL;
     msg.bodyread = 0;
     }

   // Invalidate kludges.
   for(thisone = msg.fblk; thisone; thisone=thisone->next)
     {
     howmuch = thisone->curlen;
     s = thisone->txt;
     while(howmuch--)
       {
       if(*s == '\01')
          *s = '@';
       s++;
       }
     }

   if(action == FORWARD)
     {
     sprintf(templine, "* Forwarded by NetMgr " VERSION "%s\r\r", REGISTERED ? "+" : " [unreg]");
     AddToBlock(add, templine, -1);
     }

   if(action != EMPTYBOUNCE && action != XEMPTYBOUNCE &&  action != MAKEMSG)
     {
     sprintf(templine, "Original message:\r\r--\rDate: %s\rAttr: %s\r",
                       MakeT(msg.mis.msgwritten),
                       attr_to_txt(msg.mis.attr1, msg.mis.attr2));
     AddToBlock(add, templine, -1);

     sprintf(templine, "From: %s, (%s)\r",
                        msg.mis.from,
                        formaddress(&msg.mis.origfido));  // static buffer!
     AddToBlock(add, templine, -1);

     sprintf(templine, "To  : %s, (%s)\r",
                       msg.mis.to,
                       formaddress(&msg.mis.destfido));    // static buffer!
     AddToBlock(add, templine, -1);

     if(msg.mis.attached || msg.mis.requested)
       {
       sprintf(templine, "File: %s\r", ExpandAttachesRequests());
       AddToBlock(add, templine, -1);
       if(msg.mis.subj[0] != '\0')
         {
         sprintf(templine, "Subj: %s\r", msg.mis.subj);
         AddToBlock(add, templine, -1);
         }
       sprintf(templine, "--\r", msg.mis.subj);
       AddToBlock(add, templine, -1);
       }
     else
       {
       sprintf(templine, "Subj: %s\r--\r", msg.mis.subj);
       AddToBlock(add, templine, -1);
       }
     }

   for(thisone=add; thisone->next != NULL; thisone=thisone->next)
      /* nothing */  ;
   thisone->next = msg.fblk;
   msg.fblk = add;

   // Kludges! New ones!
   if(action == FORWARD || action == MAKEMSG)
      msgid = MakeKludge(NULL, &mis->origfido);
   else
      {
      toline = CheckInternet(msg.kludges, mis);  // Internet FSC54?
      msgid = MakeKludge(msg.kludges, &mis->origfido);

      if(toline)
        {
        internadd = InitRawblock(256, 2048, 64000);
        AddToBlock(internadd, toline, -1);
        mem_free(toline);
        internadd->next = msg.fblk;
        msg.fblk = internadd;
        }
      }

   if(destarea)      // We need to place the message in another area
     {
     if( (destareahandle = OpenArea(destarea, 0)) == NULL )
       goto getout;
     sprintf(reportarea, " in %s", destarea);
     }

   SaveMessage(destarea ? destareahandle : msg.areahandle,
               mis,
               msg.fblk,
               msgid,
               0,
               1);      // always add INTL.

   jamlog(destarea ? destarea : thisarea->dir, msg.areahandle, 0);

   if(destarea) MsgCloseArea(destareahandle);

   switch(action)
     {
     case BOUNCE:
     case HDRBOUNCE:
     case EMPTYBOUNCE:
     case XBOUNCE:
     case XHDRBOUNCE:
     case XEMPTYBOUNCE:
        print_and_log("Bounced msg#%d to %s (%s)%s\n", (int) current, mis->to, formaddress(&mis->destfido), reportarea);
        break;
     case FORWARD:
        print_and_log("Forwarded msg#%d to %s (%s)%s\n", (int) current, mis->to, formaddress(&mis->destfido), reportarea);
        break;
     case MAKEMSG:
        print_and_log("Made message to %s (%s) for msg#%d%s\n", mis->to, formaddress(&mis->destfido), (int) current, reportarea);
        break;
     default:
        break;
     }

   getout:

   if(msgid) mem_free(msgid);
   if(msg.fblk) FreeBlocks(msg.fblk);
   msg.fblk = NULL;

   if(msg.kludges) mem_free(msg.kludges);
   msg.kludges = NULL;

   if (mis)
     {
     FreeMIS(mis);
     mem_free(mis);
     }
   msg.bodyread = 0;

   if(freemask) mem_free(forwardmask);

}



/* ----------------------------------------------------------- */
/* Rewrite message: update the header in memory and write out  */
/* the header to the message base..                            */
/* Check for fields w/ a '*' or -99, don't update those..      */
/* ----------------------------------------------------------- */


void rewrite(MASK *writemask, dword current, int uucp)
{
   char temp[80], *newkludges;
   char uucp_address[120];
   int addintl = 0, newid = 0;
   RAWBLOCK *add, *thisone;

   GetRawMsg(0L);

   if(uucp)
     {
     memset(uucp_address, '\0', sizeof(uucp_address));
     sprintf(uucp_address, "TO: %s\r\r", msg.mis.to);
     }

   if(msg.kludges)
      {
      RemoveFromCtrl(msg.kludges, "FMPT");
      RemoveFromCtrl(msg.kludges, "TOPT");
      }

   /* Only strip MSGID if from-address is being changed, for later */
   /* update of address part of the MSGID                          */

   if( (writemask->fromaddress.zone  != (word) -99) ||
       (writemask->fromaddress.net   != (word) -99) ||
       (writemask->fromaddress.node  != (word) -99) ||
       (writemask->fromaddress.point != (word) -99) )
       {
       if(msg.kludges)
          {
          RemoveFromCtrl(msg.kludges, "MSGID");
          newid = 1;
          if(stristr(msg.kludges, "INTL") != NULL)
             addintl = 1;
          RemoveFromCtrl(msg.kludges, "INTL");
          }
       }

   if( (writemask->toaddress.zone  != (word) -99) ||
       (writemask->toaddress.net   != (word) -99) ||
       (writemask->toaddress.node  != (word) -99) ||
       (writemask->toaddress.point != (word) -99) )
       {
       if(msg.kludges)
          {
          if(stristr(msg.kludges, "INTL") != NULL)
             addintl = 1;
          RemoveFromCtrl(msg.kludges, "INTL");
          }
       }


   if(writemask->fromname[0] != '*')
     strcpy(msg.mis.from, writemask->fromname);

   if(writemask->toname[0] != '*')
     strcpy(msg.mis.to, writemask->toname);

   if(writemask->subject[0] != '*')
     strcpy(msg.mis.subj, writemask->subject);

   /* origination address */

   if(writemask->fromaddress.zone != (word) -99)
     msg.mis.origfido.zone = writemask->fromaddress.zone;

   if(writemask->fromaddress.net != (word) -99)
     msg.mis.origfido.net = writemask->fromaddress.net;

   if(writemask->fromaddress.node != (word) -99)
     msg.mis.origfido.node = writemask->fromaddress.node;

   if(writemask->fromaddress.point != (word) -99)
     msg.mis.origfido.point = writemask->fromaddress.point;

   /* destination address */

   if(writemask->toaddress.zone != (word) -99)
     msg.mis.destfido.zone = writemask->toaddress.zone;

   if(writemask->toaddress.net != (word) -99)
     msg.mis.destfido.net = writemask->toaddress.net;

   if(writemask->toaddress.node != (word) -99)
     msg.mis.destfido.node = writemask->toaddress.node;

   if(writemask->toaddress.point != (word) -99)
     msg.mis.destfido.point = writemask->toaddress.point;

   /* attributes */

   msg.mis.attr1 |= writemask->yes_attribs1;
   msg.mis.attr2 |= writemask->yes_attribs2;
   msg.mis.attr1 &= ~writemask->no_attribs1;
   msg.mis.attr2 &= ~writemask->no_attribs2;

   matchaka(&msg.mis);

   /* Close, open for creation, and write new msg */

   MsgCloseMsg(msg.msghandle);

   if( (writemask->fromaddress.zone  != (word) -99) ||
       (writemask->fromaddress.net   != (word) -99) ||
       (writemask->fromaddress.node  != (word) -99) ||
       (writemask->fromaddress.point != (word) -99) ||
       (writemask->toaddress.zone    != (word) -99) ||
       (writemask->toaddress.net     != (word) -99) ||
       (writemask->toaddress.node    != (word) -99) ||
       (writemask->toaddress.point   != (word) -99) )
        {
        if(!msg.kludges)
           msg.kludges = mem_calloc(1, 256);
        else if( (msg.kludges = mem_realloc(msg.kludges, strlen(msg.kludges)+160)) == NULL )
            {
            print_and_log("Out of memory!\n");
            return;
            }

        if(newid)
            {
            newkludges = MakeKludge(NULL, &msg.mis.origfido);
            strcat(msg.kludges, newkludges);
            mem_free(newkludges);
            }

        if(addintl)
           {
           sprintf(temp, "\01INTL %d:%d/%d %d:%d/%d",
              msg.mis.destfido.zone,
              msg.mis.destfido.net,
              msg.mis.destfido.node,
              msg.mis.origfido.zone,
              msg.mis.origfido.net,
              msg.mis.origfido.node);

           strcat(msg.kludges, temp);
           }
        }

   if(uucp)
     {
     add = InitRawblock(512, 1024, 4096);
     AddToBlock(add, uucp_address, -1);
     for(thisone=add; thisone->next != NULL; thisone=thisone->next)
        /* nothing */  ;
     thisone->next = msg.fblk;
     msg.fblk = add;
     }


   SaveMessage(msg.areahandle,
               &msg.mis,
               msg.fblk,
               msg.kludges,
               current,
               cfg.intlforce);

   jamlog(thisarea->dir, msg.areahandle, 0);

   if(uucp)
     print_and_log("UUCP-rewrote msg#%d.\n", (int) current);
   else
     print_and_log("Rewrote msg#%d.\n", (int) current);

   msg.msghandle = MsgOpenMsg(msg.areahandle, MOPEN_READ, current);
   if(msg.msghandle == NULL)
       {
       print_and_log("Can't reopen message #%d!\n", current);
       return;
       }

}


// ==========================================================
// Add a note at the top of a message, don't change anything
// else.
// ==========================================================

void AddNote(char * bouncefile, dword current, int replacebody)
{
   RAWBLOCK *add, *thisone;
   char templine[256];
   FILE *infile;
   char *expanded;

   add = InitRawblock(2048, 2048, 64000);

   if( (infile = _fsopen(bouncefile, "r", SH_DENYNO)) == NULL)
      {
      print_and_log("Can't open file (%s)!\n", bouncefile);
      FreeBlocks(add);
      return;
      }

   while(fgets(templine, 256, infile))
      {
      Strip_Trailing(templine, '\n');
      Add_Trailing(templine, '\r');
      if(templine[0] != '\0')
         {
         expanded = ExpandMacros(templine);
         if(expanded)
           {
           AddToBlock(add, expanded, -1);
           mem_free(expanded);
           }
         }
      else
         AddToBlock(add, "\r", -1);
      }

   fclose(infile);

   GetRawMsg(0L);

   MsgCloseMsg(msg.msghandle);

   if(!replacebody)
     {
     // Link this text before original message..
     thisone = msg.fblk;
     msg.fblk = add;
     while(add->next)
        add = add->next;
     add->next = thisone;
     }
   else
     {
     // Replace old body with new text.
     FreeBlocks(msg.fblk);
     msg.fblk = add;
     }

   SaveMessage(msg.areahandle,
               &msg.mis,
               msg.fblk,
               msg.kludges,
               current,
               cfg.intlforce);

   jamlog(thisarea->dir, msg.areahandle, 0);

   if(replacebody)
     print_and_log("Replaced body of msg#%d to %s (%s).\n", (int) current, msg.mis.to, formaddress(&msg.mis.destfido));
   else
     print_and_log("Added note to msg#%d to %s (%s).\n", (int) current, msg.mis.to, formaddress(&msg.mis.destfido));

   msg.msghandle = MsgOpenMsg(msg.areahandle, MOPEN_READ, current);
   if(msg.msghandle == NULL)
       {
       print_and_log("Can't reopen message #%d!\n", current);
       return;
       }

}


/* --------------------------------- */
/* Change path of message's subject  */
/* --------------------------------- */


void changepath(dword current, char *newpath, int movefiles)
{
   char tempmerge[120];
   STRINGLIST *curel, *todolist=NULL;
   char drive[MAXDRIVE],
        dir  [MAXDIR  ],
        file [MAXFILE ],
        ext  [MAXEXT  ];

   for(curel=msg.mis.attached; curel; curel=curel->next)
     {
     if(curel->s == NULL || curel->s[0] == '\0')
       continue;

     fnsplit(curel->s, drive, dir, file, ext);
     sprintf(tempmerge, "%s\\%s%s", newpath, file, ext);
     if(thisarea->dir[0] == '!' ||
        (StrListLen(msg.mis.attached) + strlen(tempmerge) - strlen(curel->s) < 72))
        {
        todolist = AddToStringList(todolist, curel->s, NULL, 0);
        if(curel->s) mem_free(curel->s);
        curel->s = mem_strdup(tempmerge);
        }
     else
        {
        print_and_log("Can't replace paths on msg#, subject gets too long!\n", (int) current);
        FreeStringList(todolist);
        return;
        }
     }

   GetRawMsg(0L);

   MsgCloseMsg(msg.msghandle);

   SaveMessage(msg.areahandle,
               &msg.mis,
               msg.fblk,
               msg.kludges,
               current,
               cfg.intlforce);

   jamlog(thisarea->dir, msg.areahandle, 0);

   print_and_log("Changed path of msg#%d to %s.\n", (int) current, newpath);

   if(todolist != NULL)
     {
     if(movefiles)
       {
       for(curel=todolist; curel; curel = curel->next)
          CopyFile(curel->s, newpath, 1);  // Reporting done in func itself
       }

     FreeStringList(todolist);
     }

   msg.msghandle = MsgOpenMsg(msg.areahandle, MOPEN_READ, current);
   if(msg.msghandle == NULL)
       {
       print_and_log("Can't reopen message #%d!\n", current);
       return;
       }

}


// ===========================================================

char *MakeKludge(char *curkludge, NETADDR *address)

{
/*	static char counter=0; */
	static time_t	t=0;
	char *buffer, temp[80], *msgid=NULL;


   if(
         (curkludge!=NULL) &&
         ( (msgid = stristr(curkludge, "MSGID:")) != NULL)
     )
      {
      msgid = strtok(msgid+7, "\1");
      }


   if(!t)
      {
	   (void)time(&t);
	   t = (t<<4);
      }

   buffer = mem_calloc(1, 500);
   sprintf(buffer, "\01MSGID: %hu:%hu/%hu.%hu %08lx", address->zone, address->net, address->node, address->point, t++);

   if (msgid != NULL)
      {
      sprintf(temp, "\01REPLY: %s", msgid);
      strcat(buffer, temp);
      }

	return buffer;

}


/* -------------------------------------------------------------- */
/* Strip CR's, PATHlines, SEEN-BY's, and VIA lines from the end   */
/* of the messages. This makes them ready for re-export (when     */
/* moving, copying etc)                                           */
/* -------------------------------------------------------------- */


dword clean_end_message(char *msgbuf)
{

   #define TRUE  1
   #define FALSE 0

   char *cptr, *end, *tmpptr, *curend=NULL;
   int chopit = FALSE, stophere = FALSE;


   if(msgbuf == NULL) return 0;

   end = cptr = strchr(msgbuf, '\0');  /* goto end */

   /* Do we have anything to work on? */

   if( (end == NULL) || (end == msgbuf) ) return 0;

   cptr--;  /* go back one char from the trailing '\0' */

   while(                      /* Strip all junk at the end */
             ( (*cptr == ' ' ) ||
               (*cptr == '\r') ||
               (*cptr == '\t') ||
               (*cptr == '\n') )   && (cptr > msgbuf)
          )
          *cptr-- = '\0';

   curend = cptr+1;

   while((--cptr >= msgbuf) && (stophere != TRUE) ) /* Go back no further than start of message! */
     {
     if(*cptr == '\r')      /* end of line, what's on beg. of next line? */
        {
        /* Skip LF's and Soft Returns */
        tmpptr = cptr+1;
        while( ((*tmpptr == 0x0A) || (*tmpptr == 0x8D)) && (tmpptr < end) )
               tmpptr++;

        if(*tmpptr == '\0')   /* We're still at the end */
          chopit = TRUE;

        else if(*tmpptr == '\01')   /* Kludge detected */
           {
           if(strncmpi(tmpptr+1, "via", 3) == 0)  /* 'via' kludge, strip */
               chopit = TRUE;
           else if (strncmp(tmpptr+1, "PATH", 4) == 0) /* path, strip */
               chopit = TRUE;
           else stophere = TRUE;    /* unknown, stop chopping! */
           }

        else if(*tmpptr == 'S')  /* Seen-by maybe? */
              {
              if (strncmp(tmpptr, "SEEN-BY:", 7) == 0) /* SB, strip */
                  chopit = TRUE;
              else stophere = TRUE;
              }

        else stophere = TRUE;

        if(chopit == TRUE)
           {
           *cptr = '\0';
           curend = cptr;
           chopit = FALSE;
           }

        }  /* if */

     } /* while */

   *curend++ = '\r';
   *curend = '\0';

   return (dword) (curend - msgbuf);
}


/* ------------------------------------------------------- */

/* Clean the end of message and check for origin */

int clean_origin(char *msgbuf, char **startorig)
{
   char *cptr, *end, *open, *close;

   if(startorig)
     (*startorig) = NULL;

   if(msgbuf == NULL) return 1;



   end = cptr = strchr(msgbuf, '\0');  /* goto end */

   if( (end == NULL) || (end == msgbuf) ) return 1;


   cptr--;

   /* strip junk at end of message */

   while(
          ( (*cptr == ' ' ) ||
            (*cptr == '\r') ||
            (*cptr == '\t') ||
            (*cptr == '\n') )   && (cptr > msgbuf)
        )
           *cptr-- = '\0';


   while(--cptr >= msgbuf)
     {
     if(*cptr == '\r')
       {
       cptr++;
       break;
       }
     }

   if(strncmp(cptr, " * Origin: ", 11) != 0)
      {
      if(startorig && strncmp(cptr, "---", 3) == 0)
        {
        *startorig = cptr-1;
        return 0;
        }
      return 1;
      }

   if(startorig)
     {
     *startorig = cptr;
     cptr--;
     while(--cptr >= msgbuf)
       {
       if(*cptr == '\r')
         {
         cptr++;
         if(strncmp(cptr, "---", 3) == 0)
           *startorig = cptr-1;
         return 0;
         }
       }
     return 0;
     }

   if (
        ((open  = strrchr(cptr, '(')) == NULL) ||
        ((close = strrchr(open, ')')) == NULL) ||
        (open > close)
      )
      return 1;

    *(close+1) = '\r';
    *(close+2) = '\0';

    return 0;

}


/* ------------------------------------------------------- */


void dumpconfig(void)
{
   MASKLIST *curmask, *submask;
   int teller=0;
   AREALIST *thisarea;

   for(thisarea = cfg.firstarea; thisarea; thisarea=thisarea->next)
     {
     printf("\n\n#### Area/directory to scan: %s\n", thisarea->dir);

     for(curmask = thisarea->firstmask; curmask; curmask = curmask->next)
        {
        printf("\nMask %d:\n\n", ++teller);

        if(curmask->xmask != NULL)
          ShowXMask(curmask->xmask);
        else
          ShowMask(&curmask->mask);

        printf("Action(s)   :\n");
        verboze(curmask);

        for(submask=curmask->nextaction; submask; submask=submask->nextaction)
           {
           printf("And:\n");
           verboze(submask);
           }
        }
     }
   printf("\n\n");

}

// ==============================================================

void ShowMask(MASK *mask)
{
   printf("Fromname    : %s\n", mask->fromname);
   printf("Fromaddress : %s%s\n", mask->fromnot ? "Not: " : "", formaddress(&mask->fromaddress));
   printf("Toname      : %s\n", mask->toname);
   printf("Toaddress   : %s%s\n",  mask->tonot ? "Not: " : "", formaddress(&mask->toaddress));
   printf("Subject     : %s\n", mask->subject);
   printf("Attribs on  : %s\n", getattribs(mask->yes_attribs1, mask->yes_attribs2));
   printf("Attribs off : %s\n", getattribs(mask->no_attribs1, mask->no_attribs2));
   if(mask->fromlisted != 0)
     printf("Origination : Must %sbe listed\n", mask->fromlisted == 1 ? "" : "not ");
   if(mask->tolisted != 0)
     printf("Destination : Must %sbe listed\n", mask->tolisted == 1 ? "" : "not ");

}

// ==============================================================

void ShowXMask(XMASK *xmask)
{
   struct tm tmbuf;
   ATTRLIST *attrlist;

   if(xmask->xmaskname) printf("Xmask name  : %s\n", xmask->xmaskname);

   if(xmask->fromname)
     {
     printf("From name   : ");
     ShowNameList(xmask->fromname);
     }

   if(xmask->toname)
     {
     printf("To name     : ");
     ShowNameList(xmask->toname);
     }

   if(xmask->subj)
     {
     printf("Subject     : ");
     ShowNameList(xmask->subj);
     }

   if(xmask->kludge)
     {
     printf("Kludge      : ");
     ShowNameList(xmask->kludge);
     }

   if(xmask->body)
     {
     printf("Body        : ");
     ShowNameList(xmask->body);
     printf("BodyBytes  : %lu", xmask->bodybytes);
     printf("BodyLines  : %lu", xmask->bodylines);
     }

   if(xmask->orig)
     {
     printf("Orig address: ");
     ShowAddressList(xmask->orig);
     }

   if(xmask->dest)
     {
     printf("Dest address: ");
     ShowAddressList(xmask->dest);
     }

   for(attrlist=xmask->attribs; attrlist; attrlist = attrlist->or)
     {
     if(attrlist != xmask->attribs)  // Not the first one
       printf("OR:\n");
     printf("Attribs on  : %s\n", getattribs(attrlist->yes_attribs1, attrlist->yes_attribs2));
     printf("Attribs off : %s\n", getattribs(attrlist->no_attribs1, attrlist->no_attribs2));
     }

   if(xmask->fromlisted != 0)
     printf("Origination : Must %sbe listed\n", xmask->fromlisted == 1 ? "" : "not ");
   if(xmask->fromlisted != 0)
     printf("Destination : Must %sbe listed\n", xmask->tolisted == 1 ? "" : "not ");

   if(xmask->olderwritten)
     {
     _localtime((time_t *)&xmask->olderwritten, &tmbuf);
     printf("Date written must be older than %s", asctime(&tmbuf));
     }

   if(xmask->olderprocessed)
     {
     _localtime((time_t *)&xmask->olderprocessed, &tmbuf);
     printf("Date processed must be older than %s", asctime(&tmbuf));
     }

   if(xmask->olderread)
     {
     _localtime((time_t *)&xmask->olderread, &tmbuf);
     printf("Date received/read must be older than %s", asctime(&tmbuf));
     }

}

// ==============================================================

void ShowAddressList(ADDRLIST *first)
{
   ADDRLIST *or, *and;

   for(and=first; and; and = and->and)
     {
     if(and != first)  // Not the first one.
       printf("AND         : ");

     for(or=and; or; or = or->or)
       {
       if(or != and)  // Not the first one..
         printf(" OR ");
       printf("%s%s", or->not ? "NOT " : "", formaddress(&or->addr));
       }
     printf("\n");
     }
}

// ==============================================================

void ShowNameList(NAMELIST *first)
{
   NAMELIST *or, *and;

   for(and=first; and; and = and->and)
     {
     if(and != first)  // Not the first one.
       printf("AND         : ");

     for(or=and; or; or = or->or)
       {
       if(or != and)  // Not the first one..
         printf(" OR ");
       printf("\"%s\"", or->name);
       }
     printf("\n");
     }
}

// ==============================================================

char * formaddress(NETADDR *this)
{
   static char temp[100];
   char *s, *d;

   if( (sword)this->zone  == (sword) -88 &&
       (sword)this->net   == (sword) -88 &&
       (sword)this->node  == (sword) -88 &&
       (sword)this->point == (sword) -88 )
       return "@myaka";

   sprintf(temp, "%d:%d/%d.%d", (sword) this->zone,
                                (sword) this->net,
                                (sword) this->node,
                                (sword) this->point);

   s = d = temp;

   while(*s)
     {
     if( (*s == '-') && (*(s+1)=='9') && (*(s+2)=='9') )
       {
       *d++='*';
       s+=3;
       }
     else if( (*s == '-') && (*(s+1)=='8') && (*(s+2)=='8') )
       {
       strcpy(d, "@myaka");
       d += 6;
       s+=3;
       }
     else
       *d++=*s++;
     }

   *d = '\0'; /* Add trailing '\0' as well */

   return temp;

}


/* ------------------------------------------------------------ */

char *getattribs(dword attr1, dword attr2)
{
	static char temp[80];

	memset(&temp, '\0', sizeof(temp));

   if(attr1 == 0 && attr2 == 0)
     {
     strcpy(temp, "*");
     return temp;
     }

   strcpy(temp, attr_to_txt(attr1, attr2));

	return temp;
}


/* ----------------------------------------------------------- */

void verboze(MASKLIST *mask)
{
   static char temp[200];

   memset(temp, '\0', sizeof(temp));

   switch(mask->action)
     {
     case GETNEXT:
        printf("See next mask\n");
        break;

     case DISPLAY:
        printf("Display the following: %s\n", (char *)mask->value);
        break;

     case RUNEXTERNAL:
        printf("Run external program: %s (parameters: %s)\n", mask->bodyfile, (char *)mask->value);
        break;

     case IGNORE:
        printf("Ignore\n");
        break;

     case COPY:
        printf("Copy message to %s\n", (char *) mask->value);
        break;

     case ADDNOTE:
        printf("Add %s as note to message\n", (char *) mask->value);
        break;

     case ECHOCOPY:
        printf("EchoCopy message to %s\n", (char *) mask->value);
        printf("Origin address: %s\n", formaddress(&mask->origaddr));
        if(mask->seenby) printf("Add %s to the SEEN-BY\n", mask->seenby);
        break;

     case MOVE:
        printf("Move message to %s\n", (char *) mask->value);
        break;

     case ECHOMOVE:
        printf("EchoMove message to %s\n", (char *) mask->value);
        printf("Origin address: %s\n", formaddress(&mask->origaddr));
        if(mask->seenby) printf("Add %s to the SEEN-BY\n", mask->seenby);
        break;

     case PACKMAIL:
        printf("Pack mail to Binkley style outbound\n");
        printf("Origin address: %s\n", formaddress(&mask->origaddr));
        printf("Destination address: %s\n", formaddress(&mask->destaddr));
        break;

     case MOVEMAIL:
        printf("Pack/Move mail to %s\n", (char *) mask->value);
        printf("Origin address: %s\n", formaddress(&mask->origaddr));
        printf("Destination address: %s\n", formaddress(&mask->destaddr));
        break;

     case DELETE:
        printf("Delete message\n");
        break;

     case DELETEATTACH:
        printf("Delete message, and delete/truncate attaches according to flags\n");
        break;

     case BOUNCE:
        printf("Bounce message in %s, add %s to body\n", mask->destarea ? mask->destarea : "current area", (char *) mask->bodyfile);
        printf("Origin address: %s\n", formaddress(&mask->origaddr));
        break;

     case EMPTYBOUNCE:
        printf("EmptyBounce message in %s, add %s to body\n", mask->destarea ? mask->destarea : "current area", (char *) mask->bodyfile);
        printf("Origin address: %s\n", formaddress(&mask->origaddr));
        break;

     case HDRBOUNCE:
        printf("HeaderBounce message in %s, add %s to body\n", mask->destarea ? mask->destarea : "current area", (char *) mask->bodyfile);
        printf("Origin address: %s\n", formaddress(&mask->origaddr));
        break;

     case XBOUNCE:
        printf("Bounce message in %s, add %s to body\n", mask->destarea ? mask->destarea : "current area", (char *) mask->bodyfile);
        printf("Use the following mask:\n");
        ShowMask((MASK *)mask->value);
        break;

     case XEMPTYBOUNCE:
        printf("EmptyBounce message in %s, add %s to body\n", mask->destarea ? mask->destarea : "current area", (char *) mask->bodyfile);
        printf("Use the following mask:\n");
        ShowMask((MASK *)mask->value);
        break;

     case XHDRBOUNCE:
        printf("HeaderBounce message in %s, add %s to body\n", mask->destarea ? mask->destarea : "current area", (char *) mask->bodyfile);
        printf("Use the following mask:\n");
        ShowMask((MASK *)mask->value);
        break;

     case WRITE:
        printf("Write message to file (%s)\n", (char *) mask->value);
        break;

     case HDRFILE:
        printf("Write header of message to file (%s)\n", (char *) mask->value);
        break;

     case CHANGEPATH:
        printf("Change path of file(s) in subject to (%s)\n", (char *) mask->value);
        break;

     case CHANGEPATHMOVE:
        printf("Change path of file(s) in subject, and move them to (%s)\n", (char *) mask->value);
        break;

     case SEMA:
        printf("Create/Touch semaphore (%s).\n", (char *) mask->value);
        break;

     case REWRITE:
        printf("Rewrite message, use this mask:\n");
        ShowMask((MASK *)mask->value);
        break;

     case FORWARD:
        printf("Forward message in %s, use this mask:\n", mask->destarea ? mask->destarea : "current area");
        ShowMask((MASK *)mask->value);
        break;

     case MAKEMSG:
        printf("Make message in %s, use this mask:\n", mask->destarea ? mask->destarea : "current area");
        ShowMask((MASK *)mask->value);
        break;

     case UUCP:
        printf("UUCP-rewrite message, use this mask:\n");
        ShowMask((MASK *)mask->value);
        break;

     }

}


/* ------------------------------------------------------- */


void writemsg(dword current, char *file, int writewhat)
{
   dword result;
   LINE *lines, *templine;
   FILE *out;

   lines = NULL;
   msg.body_len  = MsgGetTextLen(msg.msghandle);

    /* Sanity checks */

    #ifndef __FLAT__
    if( (dword) msg.body_len > (dword) 64000L )
       msg.body_len = (dword) 64000L;
    #endif

    msg.kludge_len = MsgGetCtrlLen(msg.msghandle);

    /* Get the required memory to store (unformatted) text and ctrlinfo */

    if (msg.body_len)
       {
       if(msg.body) mem_free(msg.body);
       msg.body = mem_calloc(1, (unsigned) msg.body_len + 1);
       }
    else
       msg.body = NULL;  /*(xmalloc(80);*/

    if (msg.kludge_len)
       {
       if(msg.kludges) mem_free(msg.kludges);
       msg.kludges = mem_calloc(1, (unsigned) msg.kludge_len + 1);
       }
    else msg.kludges = NULL;

    /* Get it from disk */


    result = MsgReadMsg(msg.msghandle, NULL, 0L, msg.body_len,
                                   msg.body, msg.kludge_len, msg.kludges);

    /* Make sure it is NULL terminated */

    if (msg.body_len) *(msg.body + msg.body_len) = '\0';

    if (result == (dword) -1)           /* Oh, oh, error! OLD: check != msg.body_len */
       {
       print_and_log("Error reading msg #%d!\n", current);
       goto close_up;
       }


    /* ---------------------------- */

    if (msg.body_len && (msg.body[0] != '\0'))
       {
       normalize(msg.body);

       lines = fastFormatText(msg.body, 79);
       }
    else      /* so it was an empty message */
       {
       if(msg.body)
           msg.body = mem_realloc(msg.body, 80);
       else msg.body = mem_calloc(1, 80);

       if(!msg.body)
         {
         print_and_log("Out of memory!\n");
         goto close_up;
         }

       strcpy(msg.body, "\r");
       lines = fastFormatText(msg.body, 79);
       }

   /* ----------------------------- */


   if((out = _fsopen(file, "at", SH_DENYRW)) == NULL)
     {
     print_and_log("Error opening output file (%s)!\n", file);
     print_and_log("Errno is %d.\n", errno);
     goto close_up;
     }

   setvbuf(out, NULL, _IOFBF, 4096);

   do_print(out, &msg.mis, lines, writewhat);

   fclose(out);

   print_and_log("FILEd msg#%d in %s\n", (int) current, file);

   /* Free mem taken by lines.. */

   close_up:

   while(lines)
     {
     templine = lines;
     lines    = lines->next;
     if(templine->ls) mem_free(templine->ls);
     mem_free(templine);
     }

   if(msg.kludges)
      {
      mem_free(msg.kludges);
      msg.kludges=NULL;
      }

   if(msg.body)
      {
      mem_free(msg.body);
      msg.body=NULL;
      }

}

/* ------------------------------------------------------- */


void pascal normalize(char *s)
{
    char   *tmp = s;

    while (*s)
        {
        if (*s == 0x8d)
            s++;
        else if (*s == '\n')
            s++;
        else
            *tmp++ = *s++;
        }

    *tmp = '\0';
}


/* ---------------------------------------------------------- */


void do_print(FILE *outfile, MIS *mis, LINE *firstline, int writewhat)
{
   LINE *curline;
   char temp[133];


   if(writewhat == WRITE_HEADER || writewhat == WRITE_ALL)
     {
     sprintf(temp, "\nDate : %-32.32s%40.40s\n", MakeT(mis->msgwritten), attr_to_txt(mis->attr1, mis->attr2));
     fputs(temp, outfile);
     sprintf(temp, "From : %-36.36s%29.29s\n", mis->from, formaddress(&mis->origfido));
     fputs(temp, outfile);
     sprintf(temp, "To   : %-36.36s%29.29s\n", mis->to, formaddress(&mis->destfido));
     fputs(temp, outfile);

     if(msg.mis.attached || msg.mis.requested)
       {
       sprintf(temp, "File: %s\n", ExpandAttachesRequests());
       fputs(temp, outfile);

       if(msg.mis.subj[0] != '\0')
         {
         sprintf(temp, "Subj: %s\n", msg.mis.subj);
         fputs(temp, outfile);
         }
       }
     else
       {
       sprintf(temp, "Subj : %-70.70s\n", mis->subj);
       fputs(temp, outfile);
       }
     strcpy(temp, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴\n\n");
     fputs(temp, outfile);
     }

   if(writewhat == WRITE_HEADER) return;

	curline = firstline;

	while(curline)
		{
      if(!(curline->status & KLUDGE))
         {
		   sprintf(temp, "%s\n", curline->ls);
		   fputs(temp, outfile);
         }
		curline = curline->next;
		}

}


/* ------------------------------------------------------- */

char *attr_to_txt(dword attr1, dword attr2)
{
   static char temp[160];  /* Watch maximum length (# of possible attrs)!! */
   int l;

   memset(&temp, '\0', sizeof(temp));

   for(l=0; l < NUMCONVS1; l++)
     {
     if(attr1 & Attr2Text1[l].bit)
        strcat(temp, Attr2Text1[l].txt);
     }

   for(l=0; l < NUMCONVS2; l++)
     {
     if(attr2 & Attr2Text2[l].bit)
        strcat(temp, Attr2Text2[l].txt);
     }

   return temp;
}


#ifdef __OS2__
#define READBUFSIZE 16386
#else
#define READBUFSIZE 4096
#endif


void GetRawMsg(dword bytelimit)
{
    MIS *fakemis;
    RAWBLOCK *current=NULL;

    dword   result  = 0,
            nowread = 0,
            howmuch = 0;

    if(msg.bodyread != 0) return;

    /* Read the length of this message */

    msg.body_len = MsgGetTextLen(msg.msghandle);

    if(msg.body_len  == (dword) -1L )
       msg.body_len = (dword) 0L;

    if(bytelimit && msg.body_len > bytelimit)
      msg.body_len = bytelimit;

    msg.kludge_len = MsgGetCtrlLen(msg.msghandle);

    if (msg.kludge_len == (dword) -1L)
        msg.kludge_len  = (dword) 0L;

    /* Get the required memory to store (unformatted) text and ctrlinfo */

    // If we read this before, dump the kludges. We'll read them fresh.

    if(msg.kludges) mem_free(msg.kludges);

    if (msg.kludge_len)
       msg.kludges = mem_calloc(1, (unsigned) msg.kludge_len + 1);
    else msg.kludges = NULL;

    if(msg.body_len)
      {
      howmuch = (msg.body_len > READBUFSIZE) ? READBUFSIZE : msg.body_len;
      msg.fblk = InitRawblock((unsigned) howmuch+1, 1024, 16000);
      current = msg.fblk;
      }

    /* Get first load from disk */

    fakemis = mem_calloc(1, sizeof(MIS));
    result = MsgReadMsg(msg.msghandle,
                        fakemis,
                        0L,
                        howmuch,
                        current ? current->txt : NULL,
                        msg.kludge_len,
                        msg.kludges);
    FreeMIS(fakemis);
    mem_free(fakemis);

    if(current)
       {
       current->curlen = strlen(current->txt);
       current->curend = current->txt + current->curlen;
       }


    if(result == (dword) -1)
      {
      print_and_log("Error reading message!\n");
      if(current)
         {
         mem_free(current->txt);
         mem_free(current);
         }
      if(msg.kludges)
        {
        mem_free(msg.kludges);
        msg.kludges = NULL;
        }
      return;
      }

    if(msg.kludges && msg.kludge_len) // Only do this if we actually have kludges!
      {
      while(msg.kludges[strlen(msg.kludges) - 1] == 0x01)
            msg.kludges[strlen(msg.kludges) - 1] = '\0';
      }

    nowread += howmuch;

    // could be NULL, if msg is only one line!

    while(nowread < msg.body_len)
      {
      howmuch = READBUFSIZE;
      if( (nowread + howmuch) > msg.body_len )
         howmuch = msg.body_len - nowread;

      current->next = InitRawblock((unsigned) howmuch+1, 1024, 16000);
      current = current->next;

      result = MsgReadMsg(msg.msghandle, NULL, nowread, howmuch, current->txt, 0, NULL);

      current->curlen = strlen(current->txt);
      current->curend = current->txt + current->curlen;

      nowread += howmuch;

      if(result == -1)
         {
         print_and_log("Error reading message!\n");
         // mem_free all mem here!
         return;
         }
     }

     msg.bodyread = 1;

}

// =========================================================================

int SaveMessage(MSG *areahandle, MIS *mis, RAWBLOCK *blk, char *kludges, dword no, int addintl)
{
   dword txtsize = 0L, ctrlsize = 0L;
   word append = 0;
   int didlock = 0;
   MSGH *msghandle;
   RAWBLOCK *tmpblk = blk;
   int retval = -1;
   char *newkludges = NULL;
   char addkludge[120];


   if(debug)
     print_and_log("Saving message as #%d\n", (int) no);

   if(!(areahandle->locked))
      {
      if(MsgLock(areahandle) == -1)
         {
         print_and_log("Error locking area (SaveMessage)!\n");
         return -1;
         }
      didlock = 1;
      }

   // Calc textlength..

   // First assure Trailing zero..
   if(tmpblk && GetLastChar(tmpblk) == '\0')
      StripLastChar(tmpblk);

   while(tmpblk)
     {
     txtsize += tmpblk->curlen;
     tmpblk = tmpblk->next;
     }

   tmpblk = blk;  // Set it back for later use..

   if(kludges) ctrlsize = strlen(kludges) + 1;

   if(addintl)
     {
     if(!kludges || (stristr(kludges, "INTL") == NULL))
       {
       sprintf(addkludge, "\01INTL %d:%d/%d %d:%d/%d", mis->destfido.zone,
                                                       mis->destfido.net,
                                                       mis->destfido.node,
                                                       mis->origfido.zone,
                                                       mis->origfido.net,
                                                       mis->origfido.node);
       newkludges = mem_calloc(1, ctrlsize + strlen(addkludge));
       strcpy(newkludges, addkludge);
       if(kludges) strcat(newkludges, kludges);
       ctrlsize = strlen(newkludges) + 1;
       }
     }

   if((msghandle=MsgOpenMsg(areahandle, MOPEN_CREATE, no)) == NULL)
     {
     print_and_log("Error creating message! (SaveMessage)\n");
     return -1;
     }

   do
    {
    if(MsgWriteMsg(msghandle,
                   append,
                   mis,
                   tmpblk ? tmpblk->txt : NULL,
                   tmpblk ? tmpblk->curlen : 0L,
                   txtsize,
                   ctrlsize,
                   newkludges ? newkludges : kludges))
      {
      print_and_log("Error writing message! (SaveMessage)\n");
      goto close_up;
      }

    if(tmpblk)
      tmpblk   = tmpblk->next;
    ctrlsize   = 0;            // After first write, no more ctrl..
    kludges    = NULL;
    mis        = NULL;         // Or header!
    append     = 1;            // And we start appending.
    if(newkludges)
      {
      mem_free(newkludges);
      newkludges = NULL;
      }
    }
   while(tmpblk);

   retval = 0;

   close_up:

   if(msghandle)
      {
      if(MsgCloseMsg(msghandle) == -1)
         print_and_log("Error closing message (SaveMessage)!\n");
      }

   if(didlock)
      {
      if(MsgUnlock(areahandle) == -1)
         print_and_log("Error unlocking area (SaveMessage)!\n");
      }

   return retval;

}


// =====================================================

void MakeSem(char *filename, dword current)
{
   if(creat_touch(filename) == -1)
     print_and_log("Can't create semaphore file (%s)!\n", filename);
   else
     print_and_log("Created semaphore (%s) for msg#%d.\n", filename, (int) current);

}


// ------------------------------------------------------------------------
//
// Check an area, to see if it is the area that is currently being scanned.
//
// ------------------------------------------------------------------------


int IsCurrentArea(char *area)
{
   char temp1[120], temp2[120];

   if(area[0] == '!' || area[0] == '$' || area[0] == '#')  // JAM, SQ or HMB?
     {
     if(area[0] != thisarea->dir[0])   // Not even same format.
        return 0;
     _fullpath(temp1, area+1, 119);  // Skip !, # or $.
     }
   else  // Not JAM, SQ or HMB.
    {
    if(thisarea->dir[0] == '!' || thisarea->dir[0] == '$' || thisarea->dir[0] == '#')
       return 0;  // But this area *is* JAM, SQ or HMB - not the same!
    _fullpath(temp1, area, 119);
    }

   // Compare paths.

   if(strchr("!$#", thisarea->dir[0]) != NULL)
     _fullpath(temp2, thisarea->dir+1, 119);
   else
     _fullpath(temp2, thisarea->dir, 119);

   if(strcmp(temp1, temp2) == 0)
      return 1;

   return 0;
}

//  ==========================================================

void OpenCache(void)
{
   int cachefile, result;
   char temp[120];

   if(cfg.nodebuf[0] == '\0') return;
   if(cfg.bufentries == 0)    return;

   cacheptr = (CACHE *)mem_calloc(cfg.bufentries, sizeof(CACHE));

   sprintf(temp, "%s\\nodes.buf", cfg.nodebuf);
   if( (cachefile=sopen(temp, O_BINARY | O_RDONLY, SH_DENYWR)) == -1)
      {
      if(errno == ENOENT)   // File doesn't exist, no problem.
         return;
      print_and_log("Error opening nodelist cache!\n");
      return;
      }

   result = read(cachefile, cacheptr, (unsigned) (cfg.bufentries * sizeof(CACHE)));

   if( (result == -1) || (result % sizeof(CACHE)) )
      print_and_log("Error reading nodelist cache!\n");
   else
      buffilled =  result / sizeof(CACHE);

   close(cachefile);

}

//  ==========================================================

void CloseCache(void)
{
   int cachefile, towrite;
   char temp[120];

   if(cfg.nodebuf[0] == '\0') return;
   if(cfg.bufentries == 0)    return;

   sprintf(temp, "%s\\nodes.buf", cfg.nodebuf);
   if( (cachefile=sopen(temp, O_BINARY | O_WRONLY | O_TRUNC | O_CREAT, SH_DENYRW, S_IREAD | S_IWRITE)) == -1)
      {
      print_and_log("Error opening nodelist cache!\n");
      return;
      }

   towrite = buffilled * sizeof(CACHE);

   if(write(cachefile, cacheptr, (unsigned) towrite) != (int) towrite)
      print_and_log("Error writing nodelist cache!\n");

   close(cachefile);

}

//  ==========================================================

void AddCache(NETADDR *address, int result)
{

   if(!cacheptr) return;

   if(buffilled > 0)
     {
     if(buffilled >= cfg.bufentries)   // Lose last entry?
        buffilled--;

     memmove(cacheptr+1, cacheptr, buffilled * sizeof(CACHE));
     }

    cacheptr->addr    = *address;
    cacheptr->listed  = result;
    buffilled++;

}

//  ============================================================
//  Check nodelist cache to see ifnode is listed.
//  Returns: -1 : node not found in cache
//            0 : node is unlisted
//            1 : node is listed
//  This routines moves a 'hit' to the beginning of the cache
//  to prevent it from 'scrolling off' when a new entry is
//  added. As a result, the LRU entry is dumped from the cache!
//  ============================================================

int CheckCache(NETADDR *address)
{
   CACHE *thisone = cacheptr;
   int checked;
   CACHE tmp;

   if(!cacheptr) return -1;       // No cache.

   for(checked=0; checked < buffilled; thisone++, checked++)
     {
     if(thisone->addr.zone == address->zone &&
        thisone->addr.net  == address->net  &&
        thisone->addr.node == address->node )
        {                                             // Match!!
        // Move found entry to be the first.
        memcpy(&tmp,     thisone,  sizeof(CACHE));
        memcpy(thisone,  cacheptr, sizeof(CACHE));
        memcpy(cacheptr, &tmp,     sizeof(CACHE));
        return (cacheptr->listed);
        }
     }

   return -1;
}

//  ==========================================================

int checknode(NETADDR *addr)
{
   NETADDR tmpaddr;
   int result;

   tmpaddr = *addr;
   tmpaddr.point = 0;

   if(cacheptr)       // There is a cache
     {
     if((result=CheckCache(&tmpaddr)) != -1)    // -1 == not found in cache!
        {
        cachehits++;      // Count number of 'hits' in cache.
        return result;
        }
     }

   result = checknodelist(&tmpaddr);

   AddCache(&tmpaddr, result);

   return result;
}

// =============================================================

int checknodelist(NETADDR *tmpaddr)
{
   int result = -1;

   lookups++;     // Count number of nodelist lookups on disk.

   if(cfg.nodelist[0] != '\0')
      result = ver7find(tmpaddr);

   if(cfg.fdnodelist[0] != '\0')
      result = getFDnode(tmpaddr);

   if(cfg.gigonodelist[0] != '\0')
      result = validaddress(tmpaddr->zone, tmpaddr->net, tmpaddr->node);

   if(result == -1)
     {
     print_and_log("Exiting due to nodelist error\n");
     exit(254);
     }

   return result;
}

// ================================================================

char * CheckInternet(char *kludges, MIS *mis)
{
   char *replyaddr, *replyto;
   char * tmpptr;
   char temp[120];
   char *toline= NULL;
   NETADDR gate;

   memset(&gate, '\0', sizeof(NETADDR));

   if(kludges == NULL) return NULL;

   replyaddr = GetCtrlToken(kludges, "REPLYADDR");
   if(replyaddr == NULL) return NULL;

   replyto = GetCtrlToken(kludges, "REPLYTO");
   if(replyto == NULL)
     {
     mem_free(replyaddr);
     return NULL;
     }

   // Get the 'replyaddr' name from the kludges..
   tmpptr = replyaddr + 10;
   tmpptr = strtok(tmpptr, " \r\n\t");
   if(!tmpptr)
     {
     mem_free(replyaddr);
     mem_free(replyto);
     return NULL;
     }

   sprintf(temp, "TO: %s\r\r", tmpptr);  // Make a correct TO: line.
   toline = mem_strdup(temp);               // Copy, so we can return it.

   // Now the REPLYTO kludge line..
   tmpptr = replyto + 8;
   if((tmpptr=strtok(tmpptr, " \t\n\r")) != NULL)    // Get address part.
      {
      if(sscanf(tmpptr, "%hu:%hu/%hu.%hu", &gate.zone,
                                           &gate.net,
                                           &gate.node,
                                           &gate.point) > 2)
                                           mis->destfido = gate;

      if((tmpptr=strtok(NULL, "\n\r")) != NULL)
         {
         while(*tmpptr==' ') tmpptr++;
         if(*tmpptr != '\0')
           {
           memset(mis->to, '\0', 100);
           strcpy(mis->to, tmpptr);
           }
         }
      }

   mem_free(replyaddr);
   mem_free(replyto);

   return toline;
}

// ================================================================


char *MakeT(dword t)
{
   static char temp[50];
   JAMTM *tm;

   if(t == 0L)
     {
     strcpy(temp, "-");
     return temp;
     }

   tm = JAMsysLocalTime(&t);

   if( (tm->tm_mon < 0) || (tm->tm_mon > 11))
        tm->tm_mon = 0;

   memset(temp, '\0', sizeof(temp));
   sprintf(temp, "%s %s %2.2i '%2.2i, %2.2i:%2.2i:%2.2i",
      weekday_ab[tm->tm_wday], months_ab[tm->tm_mon], tm->tm_mday, tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);

   return temp;
}


// ===============================================================

void jamlog(char *areadir, MSG *areahandle, int echo)
{
  int tosslog;
  char temp[120];
  dword newnumber;
  dword current;

  if(areadir[0] != '!')
    return;

  if(cfg.jamlog[0] == '\0')
     return;

  current = MsgGetCurMsg(areahandle);
  newnumber = MsgMsgnToUid(areahandle, current);

  if(echo)
     sprintf(temp, "%s\\echomail.jam", cfg.jamlog);
  else
     sprintf(temp, "%s\\netmail.jam", cfg.jamlog);

  if( (tosslog = sopen(temp, O_TEXT | O_CREAT | O_WRONLY, SH_DENYWR, S_IWRITE | S_IREAD)) == -1)
    {
    print_and_log("Error opening %s (errno: %d)!\n", temp, errno);
    return;
    }

  lseek(tosslog, 0L, SEEK_END);
  sprintf(temp, "%s %lu\n", Strip_Trailing(areadir+1, '\\'), newnumber);
  write(tosslog, temp, strlen(temp));
  close(tosslog);

}

// ==============================================================


int CheckBody(XMASK *xmask)
{
  int retval;

  NAMELIST *current;

  GetRawMsg(xmask->bodybytes);

  if(xmask->bodylines)
    ChopBody(msg.fblk, xmask->bodylines);

  for(current=xmask->body; current; current = current->and)
    {
    if(MatchORbody(current, msg.fblk) == 0)
      {
      retval = 0;
      goto getout;
      }
    }

  retval = 1;

  getout:

  if(msg.fblk) FreeBlocks(msg.fblk);
  msg.fblk = NULL;
  msg.bodyread = 0;

  return retval;

}

// ==============================================================

int MatchORbody(NAMELIST *root, RAWBLOCK *fblk)
{
   NAMELIST *current;
   RAWBLOCK *curblock;
   char temp[201];
   int len1, len2;

   for(current = root; current; current=current->or)
     {
     if(fblk == NULL) // No body, let's see.
       {
       if(current->name[0] == '!') // If we want something to be NOT there
         return 1;
       else
         continue;  // otherwise we'll go on scanning for a NOT search
       }

     // Go through all the blocks that make up the body

     for(curblock = fblk; curblock; curblock = curblock->next)
       {
       if(MatchBodyElement(current->name, curblock->txt) == 1)  // Match here!
         {
         // If we don't do a NOT search, we found it, so it's OK.
         // If we DO a NOT search, and we get a '1' back, it means we
         // did not find it (which is OK, but it could still be present
         // in a later block, so we only return it if this is the last block

         if(current->name[0] != '!' || curblock->next == NULL)
            return 1;
         }
       else
        {
        // If we were doing a NOT search, and we get here, it means that we
        // actually DID find it! So if that is the case, we return a 0 now
        // cause we have a mismatch.

        if(current->name[0] == '!')
          return 0;
        }


       if(curblock->next)
         {
         len1 = min(100, curblock->curlen);
         len2 = min(100, curblock->next->curlen);
         memcpy(temp, curblock->txt + curblock->curlen - len1, len1);
         memcpy(temp+len1, curblock->next->txt + curblock->next->curlen - len2, len2);
         temp[len1 + len2] = '\0';
         if(MatchBodyElement(current->name, temp) == 1)  // Match here!
           return 1;
         }
       }
     }

   // If we get here, no match was found.

   return 0;

}

// ==============================================================

int MatchBodyElement(char *what, char *where)
{
  int not = 0;

  if(*what == '!')
    {
    not = 1;
    what++;
    }

  if(*what == '~')   // Skip this, we always search for substring in body!
    what++;

  if(stristr(where, what) == NULL)  // Not found!
    {
    if(not)          // No match found, which is what we wanted
      return 1;
    else             // No match found, and we wanted one.
      return 0;
    }
  else  // We did find a substring match!
    {
    if(not)         // We didn't want one
      return 0;
    else            // We wanted one!
      return 1;
    }

}

// ==============================================================

void ChopBody(RAWBLOCK *blk, dword lines)
{
  char *charptr;

  if(blk == NULL || blk->curlen == 0) return;

  charptr = strchr(blk->txt, '\r');
  lines--;

  while(charptr && lines)
    {
    charptr = strchr(charptr+1, '\r');   // goto next line start
    lines--;
    }

  if(charptr)
    {
    *charptr = '\0';
    blk->curlen = strlen(blk->txt);
    }

}

// ==============================================================

char * ExpandMacros(char *s)
{
   char *new;
   int maxsize = 400;
   char tempaddress[50], templine[100];
   char *lineptr=s, *tempptr;
   JAMTM *t = JAMsysLocalTime(&msg.mis.msgwritten);
   char *rightptr;
   int pos;


   if(!s || *s == '\0') return NULL;

   if(!(REGISTERED))
     return mem_strdup(s);

   new = mem_calloc(1, maxsize);
   tempptr = new;

   if( (t->tm_mon < 0) || (t->tm_mon > 11))
        t->tm_mon = 0;

   while(*lineptr)
     {
     if(*lineptr == '%')
        {
        lineptr++;

        if(*lineptr == '%')
          *tempptr++ = *lineptr++;

        else if(strncmpi(lineptr, "to", 2)==0)    /* %to */
           {
           if(msg.mis.destinter[0] != '\0')
              {
              memcpy(tempptr, msg.mis.destinter, strlen(msg.mis.destinter));
              tempptr += strlen(msg.mis.to);
              }
           else
              {
              memcpy(tempptr, msg.mis.to, strlen(msg.mis.to));
              tempptr += strlen(msg.mis.to);
              }
           lineptr += 2;
           }

        else if(strncmpi(lineptr, "fto", 3)==0)   /* %fto */
           {
           if(msg.mis.destinter[0] != '\0')
             rightptr = msg.mis.destinter;
           else
             rightptr = msg.mis.to;

           /* Only copy till first space */
           if(strchr(rightptr, '@') == NULL)
             {
             memcpy(tempptr, rightptr, strcspn(rightptr, " "));
             tempptr += strcspn(rightptr, " ");
             }
           else
             {
             memcpy(tempptr, rightptr, strcspn(rightptr, "@"));
             tempptr += strcspn(rightptr, "@");
             }
           lineptr += 3;
           }

        else if(strncmpi(lineptr, "from", 4)==0)      /* %from */
           {
           memcpy(tempptr, msg.mis.from, strlen(msg.mis.from));
           lineptr += 4;
           tempptr += strlen(msg.mis.from);
           }

        else if(strncmpi(lineptr, "ffrom", 5)==0)      /* %ffrom */
           {
           /* Only copy till first space.. */
           if(strchr(msg.mis.from, '@') == NULL)
             {
             memcpy(tempptr, msg.mis.from, strcspn(msg.mis.from, " "));
             tempptr += strcspn(msg.mis.from, " ");
             }
           else
             {
             memcpy(tempptr, msg.mis.from, strcspn(msg.mis.from, "@"));
             tempptr += strcspn(msg.mis.from, "@");
             }
           lineptr += 5;
           }

        else if(strncmpi(lineptr, "subj", 4)==0)    /* %subj */
           {
           if(msg.mis.subj[0] == '\0' && (msg.mis.attached || msg.mis.requested))
             {
             memset(templine, '\0', 100);
             strncpy(templine, ExpandAttachesRequests(), 99);
             memcpy(tempptr, templine, strlen(templine));
             tempptr += strlen(templine);
             }
           else
             {
             memcpy(tempptr, msg.mis.subj, strlen(msg.mis.subj));
             tempptr += strlen(msg.mis.subj);
             }
           lineptr += 4;
           }

        else if(strncmpi(lineptr, "orig", 4)==0)    /* %orig */
           {
           strcpy(tempaddress, fancy_address(&msg.mis.origfido));
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "dest", 4)==0)    /* %dest */
           {
           strcpy(tempaddress, fancy_address(&msg.mis.destfido));
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "time", 4)==0)    /* %time */
           {
           sprintf(tempaddress, "%2.2i:%2.2i", t->tm_hour, t->tm_min);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "year", 4)==0)    /* %year */
           {
           sprintf(tempaddress, "19%2.2i", t->tm_year);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 4;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "mon", 3)==0)    /* %mon */
           {
           sprintf(tempaddress, "%s", months_ab[t->tm_mon]);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 3;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "day", 3)==0)    /* %day */
           {
           sprintf(tempaddress, "%2.2i", t->tm_mday);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 3;
           tempptr += strlen(tempaddress);
           }

        else if(strncmpi(lineptr, "dow", 3)==0)    /* %dow */
           {
           sprintf(tempaddress, "%s", weekday_ab[t->tm_wday]);
           memcpy(tempptr, tempaddress, strlen(tempaddress));
           lineptr += 3;
           tempptr += strlen(tempaddress);
           }

        }
     else if(*lineptr == '\\' && *(lineptr+1) == 'n')
        {
        *tempptr++ = '\r';
        lineptr += 2;
        }
     else *tempptr++ = *lineptr++;

     // The maximus length for an expansion is 100 chars (max fields in a
     // MIS struct). As soon as we get within 100 chars of the max, we
     // increase the mem area..

     if((tempptr - new) > (maxsize - 102))
       {
       pos = tempptr - new;
       maxsize += 200;
       new = mem_realloc(new, maxsize);
       tempptr = new + pos;
       }
     }

   *tempptr = '\0';

   return new;
}

// ==============================================================

void KillAttaches(void)
{
   STRINGLIST *att;

   for(att = msg.mis.attached; att; att = att->next)
     {
     if(msg.mis.attr1 & aKFS)
        {
        if(unlink(att->s) != 0)
          print_and_log("Error deleting %s!\n", att->s);
        else
          print_and_log("Deleted %s.\n", att->s);
        }
     else if(msg.mis.attr1 & aTFS)
        {
        if(Truncate(att->s) == -1)
          print_and_log("Error truncating %s!\n", att->s);
        }
     }

}

// ==============================================================

MSG * OpenArea(char *todir, int isecho)
{
  MSG  * areahandle;
  char * dir;
  word   type;


  if(IsCurrentArea(todir))
    {
    print_and_log("Attempt to copy/move/forward to/in current area! (%s)\n", todir);
    return NULL;
    }

  /* first open destination area for message */

  switch(todir[0])
    {
    case '$':
       type = MSGTYPE_SQUISH;
       dir = todir + 1; /* Skip the $ */
       break;

    case '!':
       type = MSGTYPE_JAM;
       dir = todir + 1; /* Skip the ! */
       break;

    case '#':
       type = MSGTYPE_HMB;
       dir = todir + 1; /* Skip the # */
       break;

    default:
       type = MSGTYPE_SDM;
       dir = todir;
       break;
    }

  if(isecho)
    type |= MSGTYPE_ECHO;
  else
    type |= MSGTYPE_NET;

  if( (areahandle = MsgOpenArea(dir, MSGAREA_CRIFNEC, type)) == NULL)
    {
    print_and_log("Can't open  area (%s)!\n", dir);
    return NULL;
    }

  return areahandle;

}

// ==============================================================

void StripPercent(char *s)
{

   if(!s) return;

   while(*s)
     {
     if(*s == '%') *s = ' ';
     s++;
     }

}

// ==============================================================

char * ExpandAttachesRequests(void)
{
   char *attreq;
   STRINGLIST *current;
   char temp[80];

   #define MAXSIZE 1024

   if(msg.mis.attached == NULL && msg.mis.requested == NULL)
     return mem_strdup("");

   attreq = mem_calloc(1, MAXSIZE);
   for(current=msg.mis.attached; current; current = current->next)
     {
     if( (current->s != NULL) &&
         (strlen(attreq) + strlen(current->s)) < (MAXSIZE-1) )
       {
       if(strlen(attreq) != 0)
         strcat(attreq, " ");
       strcat(attreq, current->s);
       }
     }

   for(current=msg.mis.requested; current; current = current->next)
     {
     if(current->s == NULL) continue;

     if(current->pw)
       sprintf(temp, "%s !%s", current->s, current->pw);
     else
       strcpy(temp, current->s);

     if( (strlen(attreq) + strlen(temp)) < (MAXSIZE-1) )
       {
       if(strlen(attreq) != 0)
         strcat(attreq, " ");
       strcat(attreq, temp);
       }
     }

   return attreq;
}

// ==============================================================
