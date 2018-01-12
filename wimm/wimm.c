#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>
#include <conio.h>
#include <time.h>
#include <dos.h>
#include <stdlib.h>
#include <ctype.h>
#include <msgapi.h>
#include <progprot.h>
#include <video.h>
#include <scrnutil.h>
#include <\sys\stat.h>

#ifndef __OS2__
   #include "dvaware.h"
#endif

#include "idx.h"
#include "wstruct.h"
#include "readcfg.h"
#include "xmalloc.h"
#include "lastread.h"
#include "xfile.h"

#define VERSION "1.31"

PERSMSGLIST   *firstpers = NULL;

void readparms (int argc, char *argv[]);
void ScanAreas(AREA *first);
void ScanAllAreas(AREA *first);
void ScanLogAreas(AREA *first);
AREA *GetArea(AREA *first, char *line);
void DoScan(AREA *curarea);
void PersonalScan(MSG *areahandle, AREA *curarea);
void PersMsg(MSG *areahandle, AREA *curarea);
void PersSquish(AREA *curarea);
void ProcessMessages(MSG *areahandle, MSGLIST *firstmsg, AREA *curarea);
void AddMessage(char *tag, long number, char *from, char *subject);
void MakeList(void);
void showparms(void);
int  SQReadIndex(char *path);
void areastatus(char *area, char scroll);
void what_act(char *desc);
void message(char *desc);
void getout(char *error);
void logit(char *line, int hrt);
void dealloc_areas(AREA *first);
void dealloc_names(NAMELIST *first);
dword GetLastread(char *lastname);

char cfgname[100]     = "WIMM.CFG";
char TossLogFile[100] = "";
char currentarea[80]  = "";


typedef struct idxdata
   {
   struct my_idx *idxptr;
   long n;
   long last;
   } IDXDATA;

IDXDATA idxmem;
int log;
int dolog = 0;
int foundmail = 0;

static char *mtext[]={	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


void main(int argc, char *argv[])
{
   time_t begin, end;
   struct _minf   minf;
   BOX *copyright, *cur_area, *personal, *activity;
   char temp[80];


   time(&begin);

   video_init();

   #ifndef __OS2__
   dv_conio();
   #endif

   copyright = initbox( 0, 0, 2,51,3,7,SINGLE,NO,' ');
   cur_area  = initbox( 3, 0,10,79,3,7,SINGLE,NO,' ');
   personal  = initbox(11, 0,16,79,3,7,SINGLE,NO,' ');
   activity  = initbox(17, 0,21,79,3,7,SINGLE,NO,' ');

   cls();
   MoveXY(1,maxy-2);

   print(0,52,4 , "ллллллллллллллллллллллллллл");
   print(1,52,112,"  Made in The Netherlands  ");
   print(2,52,1 , "ллллллллллллллллллллллллллл");

   drawbox(copyright);
   #ifndef __OS2__
   boxwrite(copyright,0,0, " WIMM 1.31  (c) 1994 Gerard van Essen (2:281/527)");
   #else
   boxwrite(copyright,0,0, "WIMM/2 1.31  (c) 1994 Gerard van Essen (2:281/527)");
   #endif
   delbox(copyright);

   drawbox(cur_area);
   print(3,1,3," Scanning Area ");
   delbox(cur_area);

   drawbox(personal);
   print(11,1,3," Personal Messages ");
   delbox(personal);

   drawbox(activity);
   print(17,1,3," Recent Activity ");
   delbox(activity);

   minf.def_zone    = 0;
   minf.req_version = 0;

   MsgOpenApi(&minf, "", 1, "");

   readparms(argc, argv);

   GetConfig(cfgname);

   if (LogFile[0] != 0)
      {
      if ( (log = sopen(LogFile, O_CREAT | O_APPEND | O_RDWR | O_TEXT, SH_DENYWR, S_IWRITE | S_IREAD)) == -1)
         {
         sprintf(temp, "Can't open logfile! (%s)", LogFile);
         what_act(temp);
         dolog = 0;
         }
      else
          {
          dolog = 1;
          #ifndef __OS2__
          sprintf(temp, "Begin, WIMM " VERSION " (%lu K)", coreleft()/1024);
          #else
          sprintf(temp, "Begin, WIMM/2 " VERSION " ");
          #endif
          logit(temp, 1);
          }
      }


   ScanAreas(firstarea);

   time(&end);

   sprintf(temp, "Done! Total runtime: %d seconds.", end - begin);

   what_act(temp);

   #ifndef __OS2__
     sprintf(temp, "End, WIMM " VERSION " (%lu K)", coreleft()/1024);
   #else
     sprintf(temp, "End, WIMM/2 " VERSION " ");
   #endif

   logit(temp, 1);

   if (dolog)
      {
      write(log, "\n", strlen("\n"));
      close(log);
      }

   MsgCloseApi();

   MoveXY(1,23);

   dealloc_areas(firstarea);
   dealloc_names(firstname);

   exit(foundmail);

}


void ScanAreas(AREA *first)

{

   if (TossLogFile[0] == 0)
      ScanAllAreas(first);
   else
      ScanLogAreas(first);

   if (firstpers != NULL)
                      MakeList();

}



void ScanAllAreas(AREA *first)

{
    AREA    *curarea;

    for (curarea = first; curarea; curarea=curarea->next)

        DoScan(curarea);
}


void ScanLogAreas(AREA *first)

{


    AREA    *curarea = NULL;
    XFILE    *tosslog;
    char    *line, msg[80];


    if ((tosslog = xopen(TossLogFile)) == NULL)
       {
       sprintf(msg, "Can't open %s!", TossLogFile);
       what_act(msg);
       }
    else
      {
      sprintf(msg, "Scanning areas found in %s", TossLogFile);
      what_act(msg);

      while ( (line=xgetline(tosslog)) != NULL)
        {
        if (strlen(line) <1) continue;    /* Too short */

        if ((curarea = GetArea(first, line)) == NULL)
           {
           sprintf(msg, "Unknown area: %s", line);
           what_act(msg);
           continue;
           }

        DoScan(curarea);
        }

      xclose(tosslog);
      }

    curarea = firstarea;   /* Look for any Forced but unscanned areas */
    while(curarea)
       {
       if ( (curarea->status & FORCED) && !(curarea->status & SCANNED) )
          DoScan(curarea);
       curarea = curarea->next;
       }

}



AREA *GetArea(AREA *first, char *line)

{
   AREA *current;

   if (line[strlen(line)-1] == '\n')
      line[strlen(line)-1] = '\0';

   for(current = first; current; current = current->next)
      {
      if(stricmp(line, current->tag) == 0)
         return (current);
      }

   return NULL;

}


void DoScan(AREA *curarea)

{
   MSG     *areahandle = NULL;
   char temp[80], msg[80];


   strcpy(currentarea, curarea->tag);

   if (curarea->status & SCANNED) {

//      sprintf(temp, "%-40.40s  Already Scanned, skipping..", curarea->tag);
//      areastatus(temp,1);
      return;
   }

   curarea->status |= SCANNED;

   if (curarea->status & PASSTHRU) {

//      sprintf(temp, "%-40.40s  Passthru, skipping..", curarea->tag);
//      areastatus(temp,1);
      return;
   }

   if (curarea->status & EXCLUDE) {

//      sprintf(temp, "%-40.40s  Excluded, skipping..", curarea->tag);
//      areastatus(temp,1);
      return;
   }

	sprintf(temp, "%-40.40s  ", curarea->tag);
   areastatus(temp,1);

   if (curarea->base == MSGTYPE_SDM)    /* Scan a *.MSG area */
     {
     if(!(areahandle=MsgOpenArea(curarea->dir, MSGAREA_NORMAL, curarea->base)))
        {
        if (msgapierr == MERR_NOENT)
           {
           sprintf(msg, "Empty area..");
           areastatus(msg, 0);
           }
        else
           {
           sprintf(msg, "Error opening %s! (MSGAPI returned NULL-handle)", curarea->dir);
           what_act(msg);
           }
	     return;
        }

     PersMsg(areahandle, curarea);

     if (MsgCloseArea(areahandle) == -1)
        {
        sprintf(msg, "Error closing Area %s!", curarea->tag);
        what_act(msg);
        }

     }

   else    /* Squish area */
      PersSquish(curarea);

}


void PersMsg(MSG *areahandle, AREA *curarea)

{
	long  curno, last, scanned=0;
	MSGH	*msghandle;
	XMSG  *header;
   NAMELIST *curname;
   MSGLIST  *firstmsg = NULL, *thismsg, *lastmsg = NULL;
   char msg[80];


   last = (scanfrom == ALL) ? 0 : GetLast(curarea, areahandle);

   if (last >= MsgGetHighMsg(areahandle))
      {
      sprintf(msg, "No new messages..");
      areastatus(msg, 0);
      return;
      }

	for (curno=last+1; curno <= MsgGetHighMsg(areahandle); curno++) {

		if ((msghandle=MsgOpenMsg(areahandle, MOPEN_READ, curno))==NULL) continue;       /* open failed, try next message */

		header = xcalloc(1, sizeof(XMSG));

		if (MsgReadMsg(msghandle, header, 0L, 0L, NULL, 0L, NULL)!=-1) {

         scanned++;
         curname = firstname;
         while(curname) {             /* Try all names in list */

            if (strcmpi(header->to, curname->name)==0) {      /* Yep! Match... */

               thismsg = xmalloc(sizeof(MSGLIST));
               if (firstmsg == NULL)
                  firstmsg = thismsg;
               else
                  lastmsg->next = thismsg;
               thismsg->id   = MsgMsgnToUid(areahandle, curno);
               thismsg->next = NULL;
               lastmsg = thismsg;
            }
            curname = curname->next;       /* Select next "alias" */
         }

	   }

		free(header);
		MsgCloseMsg(msghandle);
	}

   if (scanned > 0)
      {
      sprintf(msg, "Scanned %ld messages.", scanned);
      areastatus(msg, 0);
      }

   if (firstmsg)           /* Found any personal messages? */
      {
      ProcessMessages(areahandle, firstmsg, curarea);

      while(firstmsg)             /* Give mem for msglist back */
        {
        thismsg = firstmsg;
        firstmsg = firstmsg->next;
        free(thismsg);
        }
      }
}



void PersSquish(AREA *curarea)
{
	struct my_idx	*l, *idxptr;
	long total, scanned=0;
   NAMELIST *curname;
   MSGLIST  *firstmsg = NULL, *thismsg, *lastmsg = NULL;
   MSG *areahandle;
   char msg[80];
   int index;
   char idxname[100], lastname[100];
   unsigned bytes;
   dword lastread = (dword) -1;
   dword lastuid = 0;
   int stop;

   #define CHUNKSIZE 1000

   sprintf(idxname,  "%s.SQI", curarea->dir);
   sprintf(lastname, "%s.SQL", curarea->dir);

   if(
      ((index = sopen(idxname, O_RDONLY | O_BINARY, SH_DENYNO)) == -1) ||
      (filelength(index) == 0L)
     )
      {
      sprintf(msg, "Empty area..");
      areastatus(msg, 0);
      if(index != -1) close(index);
      return;
      }

   if (scanfrom == ALL)
      idxmem.last = 0;

   idxptr = xcalloc(1, CHUNKSIZE * sizeof(struct my_idx));

   lseek(index, 0L, SEEK_SET);

   stop = 0;

   while((bytes = read(index,
                       idxptr,
                       (unsigned) CHUNKSIZE * sizeof(struct my_idx))) > 0)
     {
     if(stop) break;
     total = (long)bytes/sizeof(struct my_idx);

     if (total < 1)
        break;

     l = idxptr;

     while(total)
       {
       total--;
       if( (l->umsgid == -1) || (l->umsgid<lastuid) )
         {
         stop = 1;
         break;
         }

       scanned++;
       curname = firstname;
       while(curname)
          {                             /* Try all names in list */
		    if(l->hash == curname->hash)
            {
            if(lastread == -1)
               lastread = GetLastread(lastname);

            if( (scanfrom == LASTREAD) && (l->umsgid <= lastread) )
              {
              curname = curname->next;
              continue;
              }

            thismsg = xmalloc(sizeof(MSGLIST));
            if (firstmsg == NULL)
               firstmsg = thismsg;
            else
               lastmsg->next = thismsg;

            thismsg->id   = l->umsgid;
            thismsg->next = NULL;
            lastmsg = thismsg;
		      }
          curname = curname->next;       /* Select next "alias" */
          }
       l++;
	    }
     }

   sprintf(msg, "Scanned %ld index entries.", scanned);
   areastatus(msg, 0);

   free(idxptr);

   close(index);


   if (firstmsg) {

      if(!(areahandle=MsgOpenArea(curarea->dir, MSGAREA_NORMAL, curarea->base)))
        {
        sprintf(msg, "Error opening %s! (MSGAPI returned NULL-handle)", curarea->dir);
        what_act(msg);
	     return;
        }

      ProcessMessages(areahandle, firstmsg, curarea);

      if (MsgCloseArea(areahandle) == -1)
        {
        sprintf(msg, "Error closing Area %s!", curarea->tag);
        what_act(msg);
        }

      while(firstmsg) {

        thismsg = firstmsg;
        firstmsg = firstmsg->next;
        free(thismsg);
      }
   }
}


dword GetLastread(char *lastname)
{
   dword lastread;
   int last;

   if ( ((last = sopen(lastname, O_RDONLY | O_BINARY, SH_DENYNO)) == -1))
     return 0;

   if (read(last, &lastread, sizeof(long)) != sizeof(long))
       lastread = 0;

   close(last);

   return lastread;

}


void ProcessMessages(MSG *areahandle, MSGLIST  *firstmsg, AREA *curarea)

{
   char  *ctxt = NULL, temp[256], msg[80], errortext[100];
   char  *msgtxt = NULL;
   dword   ctrllen, totlen, origlen, safe=0;
   MSG   *toarea = NULL;
   XMSG  *header = NULL;
   MSGH  *msghandle = NULL;
   MSGLIST *thismsg = NULL;
   NAMELIST *curname = NULL;
   dword msgno = 0;

   foundmail = 1;

   sprintf(msg, "Personal mail found in %s!", currentarea);
   logit(msg, 1);

   if ((toarea = MsgOpenArea(LocalArea, MSGAREA_CRIFNEC, LocalType | MSGTYPE_LOCAL)) == NULL) {

      sprintf(msg, "Error opening local area (%s)!", LocalArea);
      getout(msg);
   }

   if(MsgLock(toarea) == -1)
     {
     sprintf(msg, "Error locking local area (%s)!", LocalArea);
     getout(msg);
     }

   thismsg = firstmsg;
   while(thismsg) {

      safe = 0;      /* Used to double-check if a message is TO: you..! */

      if ( (msgno = MsgUidToMsgn(areahandle, thismsg->id, UID_EXACT)) == 0 )
         {
         thismsg = thismsg->next;
         sprintf(errortext, "UID #%d in %s doesn't exist..", (int) thismsg->id, curarea->tag);
         what_act(errortext);
         continue;
         }

      if ((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, msgno)) == NULL) {

         sprintf(msg, "Error opening message #%d!", (int) msgno);
         what_act(msg);
         thismsg = thismsg->next;
         continue;
      }


      ctrllen = MsgGetCtrlLen(msghandle);
      totlen  = MsgGetTextLen(msghandle);

      #ifndef __OS2__
      if (totlen > 64000)
         totlen = 64000;
      #endif

      header = xcalloc(1, sizeof(XMSG));

      if (ctrllen)
         {
         ctxt   = xcalloc(1, ctrllen + 100);  // Add 100 for possible AREA: kludge
         }
      else
         ctxt = NULL;

      if (totlen)
         {
         msgtxt = xcalloc(1, totlen);
         }
      else
         msgtxt = NULL;

      if(MsgReadMsg(msghandle, header, 0L, totlen, msgtxt, ctrllen, ctxt) == -1)
         {
         sprintf(errortext, "Error reading message #d!", (int) msgno);
         what_act(errortext);
         MsgCloseMsg(msghandle);
         goto close_all;
         }

      if (header->attr & MSGREAD) {

         MsgCloseMsg(msghandle);
         sprintf(msg, "Msg from %s (rcvd, skipping..)", header->from);
         message(msg);
         goto close_all;
      }

      if (markreceived) {

        if(MsgLock(areahandle) == -1)
          {
          sprintf(msg, "Error locking original area");
          what_act(msg);
          }

         header->attr |= MSGREAD;
         if(MsgWriteMsg(msghandle, 0, header, 0L, 0L, 0L, 0L, 0L) == -1)
            {
            sprintf(errortext, "Error setting 'received' status!");
            what_act(errortext);
            }

         if(MsgUnlock(areahandle) == -1)
           {
           sprintf(msg, "Error unlocking original area");
           what_act(msg);

           }
      }

      if(MsgCloseMsg(msghandle) == -1)
         {
         sprintf(errortext, "Error closing msg #%d", (int) msgno);
         what_act(errortext);
         }

      curname = firstname;         /* Double check if it's a personal msg */
      while(curname) {             /* Try all names in list */

         if (strcmpi(header->to, curname->name)==0) {      /* Yep! Match... */
            safe = 1;
            break;
         }
         curname = curname->next;       /* Select next "alias" */
      }

      if (!safe)
         {
         sprintf(errortext, "Message %d doesn't seem to be personal anyway?!", (int) msgno);
         what_act(errortext);
         goto close_all;
         }

      sprintf(msg, "Msg from %-0.26s (%-0.39s)", header->from, header->subj);
      message(msg);

      if (mode == MOVE || mode == COPY)
         {
         if (mode == MOVE)
            {
            if(MsgKillMsg(areahandle, msgno) == -1)
               {
               sprintf(errortext, "Cannot kill message #d!", msgno);
               what_act(errortext);
               }
            }

         if (!nonotes)
            {
            if (mode == MOVE)
               #ifndef __OS2__
               sprintf(temp, "-=> Note: Moved from %s by WIMM " VERSION "\r\r", curarea->tag);
               #else
               sprintf(temp, "-=> Note: Moved from %s by WIMM/2 " VERSION "\r\r", curarea->tag);
               #endif
               else
                   #ifndef __OS2__
                   sprintf(temp, "-=> Note: Copied from %s by WIMM " VERSION "\r\r", curarea->tag);
                   #else
                   sprintf(temp, "-=> Note: Copied from %s by WIMM/2 " VERSION "\r\r", curarea->tag);
                   #endif


               totlen  = totlen + strlen(temp) + 1;
            }
         else
             memset(temp, '\0', sizeof(temp));

         origlen = totlen;

         header->attr = attr;
         header->replyto = 0;
         memset(&header->replies, '\0', 40);

         /* Lets see if we want to add an AREA kludge */
         if( (addareakludge) && (stristr(ctxt, "AREA:") == NULL) )
             // None found, add one..
            {
            sprintf(errortext, "\01AREA: %s", curarea->tag);
            strcat(ctxt, errortext);
            ctrllen = strlen(ctxt) + 1;
            }

         if ((msghandle = MsgOpenMsg(toarea, MOPEN_CREATE, 0)) == NULL)
               {
               sprintf(msg, "Error opening message for write!");
               what_act(msg);
               goto close_all;
               }

         if ((MsgWriteMsg(msghandle, 0, header, temp, strlen(temp), totlen, ctrllen, ctxt)) == -1)
               {
               sprintf(msg, "Error writing message!");
               getout(msg);
               }

         if ((MsgWriteMsg(msghandle, 1, 0L, msgtxt, origlen, totlen, 0L, 0L)) == -1)
               {
               sprintf(msg, "Error writing message!");
               getout(msg);
               }

         if(MsgCloseMsg(msghandle) == -1)
            {
            sprintf(errortext, "Problem closing message!");
            what_act(errortext);
            }

         }                          /* End " move or copy" */

      if (mode == LIST)
         AddMessage(curarea->tag, msgno, header->from, header->subj);

      close_all:

      if (ctxt)
         free(ctxt);
      if (msgtxt)
         free(msgtxt);
      if (header)
         free(header);

      thismsg = thismsg->next;
   }

   if(MsgUnlock(toarea) == -1)
     {
     sprintf(errortext, "Error unlocking WIMM area!");
     what_act(errortext);
     }

   if(MsgCloseArea(toarea) == -1)
      {
      sprintf(errortext, "Error closing WIMM area!");
      what_act(errortext);
      }

}


void AddMessage(char *tag, long number, char *from, char *subject)

{

          PERSMSGLIST  *thismsg;
   static PERSMSGLIST  *lastmsg;

   thismsg = xmalloc(sizeof(PERSMSGLIST));

   if (firstpers == NULL)
      firstpers = thismsg;
   else
      lastmsg->next = thismsg;

   strcpy(thismsg->tag,     tag);
   strcpy(thismsg->from,    from);
   strcpy(thismsg->subject, subject);

   thismsg->number = number;
   thismsg->next   = NULL;

   lastmsg = thismsg;

}

void MakeList(void)

{
   char  *msgtxt, temp[256], lastarea[30], msg[80];
   dword totlen;
   MSG   *toarea;
   XMSG  *header;
   MSGH  *msghandle;
   PERSMSGLIST *curmsg;
   time_t      now;
   union stamp_combo combo;
   struct tm *tmdate;


   foundmail = 1;

   strcpy(lastarea, "none");

   if ((toarea = MsgOpenArea(LocalArea, MSGAREA_CRIFNEC, LocalType | MSGTYPE_LOCAL)) == NULL)
      {
      sprintf(msg, "Error opening LocalArea (%s)!", LocalArea);
      getout(msg);
      }

   if(MsgLock(toarea) == -1)
     {
     sprintf(msg, "Error locking local area (%s)!", LocalArea);
     getout(msg);
     }


   header = xcalloc(1, sizeof(XMSG));

   strcpy(header->from, "WIMM");
   strcpy(header->to, firstname->name);
   strcpy(header->subj, "Your mail!");

   header->attr = attr;

   time(&now);
   tmdate=localtime(&now);
   header->date_written=header->date_arrived=(TmDate_to_DosDate(tmdate,&combo))->msg_st;


   if ((msghandle = MsgOpenMsg(toarea, MOPEN_CREATE, 0)) == NULL) {

      sprintf(msg, "Error opening message for writing!");
      getout(msg);
   }

   msgtxt = xcalloc(1, 16000);

   curmsg = firstpers;

   strcpy(msgtxt, "You have the following personal messages:\r");

   while(curmsg) {

      if ( (strcmpi(lastarea, "none")==0) || (strcmpi(lastarea, curmsg->tag) != 0)) {

        sprintf(temp, "\rArea: %s\r\r", curmsg->tag);
        strcat(msgtxt, temp);
      }

      sprintf(temp, "#%4.4lu From: %0.26s (%0.40s)\r", curmsg->number, curmsg->from, curmsg->subject);
      if (strlen(temp) > 78) temp[79] = '\0';
      strcat(msgtxt, temp);
      strcpy(lastarea, curmsg->tag);

      if (strlen(msgtxt) > 15500) {

         strcat(msgtxt, "\rText too long, message aborted..\r");
         goto write_it;
      }
      curmsg = curmsg->next;
   }

   #ifndef __OS2__
   strcat(msgtxt, "\r\r--- WIMM " VERSION "\r * Origin:             (c) 1994  Gerard van Essen              (2:281/527)\r");
   #else
   strcat(msgtxt, "\r\r--- WIMM/2 " VERSION "\r * Origin:            (c) 1994  Gerard van Essen             (2:281/527)\r");
   #endif

   write_it:

   totlen = strlen(msgtxt) + 1;

   if ((MsgWriteMsg(msghandle, 0, header, msgtxt, totlen, totlen, 0L, 0L))==-1)
      {
      sprintf(msg, "Error writing message!");
      getout(msg);
      }

   MsgCloseMsg(msghandle);

   free(header);
   free(msgtxt);

   MsgUnlock(toarea);

   MsgCloseArea(toarea);

   curmsg = firstpers;

   while(curmsg) {

      firstpers = curmsg;
      curmsg = curmsg->next;
      free(firstpers);
   }

}



void readparms (int argc, char *argv[])
{
    int i;
    char *p;

    for(i=1;i<argc;i++) {

        p=argv[i];
        if(*p == '-' || *p == '/') {

            switch(tolower(*(++p))) {

                case 'c':                 /* Configfile */
                     strcpy(cfgname,++p);
                     break;

                case 'f':                 /* Echotoss.log */
                     strcpy(TossLogFile,++p);
                     break;

                default : showparms();
            }
        }
        else showparms();
    }
}

/* Show command line parms if I can't make sense out of them */

void showparms()

{
   char msg[80];

   #ifndef __OS2__
   sprintf(msg, "Usage: WIMM [-Cconfigfile] [-Fechotoss.log]");
   what_act(msg);
   sprintf(msg, "Example: WIMM -Cc:\\Wimm\\mycfg.cfg -Fc:\\echotoss.log");
   getout(msg);
   #else
   sprintf(msg, "Usage: WIMMP [-Cconfigfile] [-Fechotoss.log]");
   what_act(msg);
   sprintf(msg, "Example: WIMMP -Cc:\\Wimm\\mycfg.cfg -Fc:\\echotoss.log");
   getout(msg);
   #endif

}


/* Read a Squish index from disk, and obtain lastread info if needed */



/* Write current area to 'scanned area' box, append 'xx entries scanned' later */


void areastatus(char *area, char scroll)
{
   static int pos;
   char *end;

   if (scroll)
      {
      bios_scroll_up(1,4,1,9,78,7);
      print(9,2,7,area);
      pos = strlen(area);
      }
   else
      print(9,2+pos,7,area);

}

/* Write name and subject to the 'personal messages' box */

void message(char *desc)
{
  bios_scroll_up(1,12,1,15,78,7);
  print(15,2,7,desc);
  logit(desc, 1);
}


/* Write string to recent activity box */

void what_act(char *desc)
{
  bios_scroll_up(1,18,1,20,78,7);
  print(20,2,7,desc);
  logit(desc, 1);
}

/* Write error to screen and exit */

void getout(char *error)
{
   what_act(error);
   logit(error, 1);
   logit("End, exiting with error!", 1);
   MoveXY(1,23);
   if (logit)
      close(log);
   exit(254);
}


/* Write a line to the logfile (if logging is on..) */

void logit(char *line, int hrt)

{
  time_t ltime;
  struct tm *tp;
  char temp[100];

  if (!dolog) return;

  (void)time (&ltime);
  tp = localtime (&ltime);

  if (strlen(line) > 50)
                   line[50] = '\0';

  if (hrt)
     sprintf (temp, "\n  %02i %03s %02i:%02i:%02i WIMM %s",
      tp->tm_mday, mtext[tp->tm_mon], tp->tm_hour, tp->tm_min, tp->tm_sec,
      line);

  else sprintf(temp, "%s", line);

  write(log, temp, strlen(temp));

}


void dealloc_areas(AREA *first)
{
   AREA *ptr1, *ptr2;

   ptr1 = first;
   while(ptr1)
      {
      ptr2 = ptr1->next;
      free(ptr1);
      ptr1 = ptr2;
      }
}


void dealloc_names(NAMELIST *first)
{
   NAMELIST *ptr1, *ptr2;

   ptr1 = first;
   while(ptr1)
      {
      ptr2 = ptr1->next;
      free(ptr1);
      ptr1 = ptr2;
      }
}
