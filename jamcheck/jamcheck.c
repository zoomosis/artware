#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <time.h>
#include <sys\stat.h>

#include <video.h>
#include <scrnutil.h>

#include "typedefs.h"
#include "jam.h"
#include "gestruct.h"
#include "fe_cfg.dat"
#include "xmalloc.h"
#include "xmail.h"

typedef struct _area
{
   char         *name;
   char         *dir;
   struct _area *next;

}  AREA;

AREA *first = NULL,
     *last  = NULL;

char tmpmsg[100];

unsigned long TotalErrors  = 0L,
              CorruptAreas = 0L,
              AreasScanned = 0L,
              TotalScanned = 0L,
              TotalDeleted = 0L,
              TotalActive  = 0L;

int log = -1;

void ana_GECHO_area(AREAFILE_GE *area);
int  ReadGEchoCFG(void);

void ana_FE_area(Area *area);
int  ReadFastechoCFG(void);

int  ReadxMailCFG(void);
void ana_xMail_area(xMailArea *area);
char *xpstrdup(char *str);

void CheckArea(char *dir);

long GetNextHeader(int hdrfile, JAMHDR *hdr);
int  GetIdx(int index, JAMHDR *hdr, JAMHDRINFO *hdrinfo, JAMIDXREC *idx);
void PutMessage(char *msg);
void logit(char *line);

static char *mtext[]={	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


int main(int argc, char *argv[])
{
   AREA *thisone;
   time_t begin, end;
   BOX *copyright, *cur_area, *personal, *activity;
   char temp[80], tmpno[20];

   time(&begin);

   video_init();

//   #ifndef __OS2__
//   dv_conio();
//   #endif

   copyright = initbox( 0, 0, 2,53,3,7,SINGLE,NO,' ');
   cur_area  = initbox( 3, 0, 7,79,3,7,SINGLE,NO,' ');
   personal  = initbox( 8, 0,12,79,3,7,SINGLE,NO,' ');
   activity  = initbox(13, 0,maxy-4,79,3,7,SINGLE,NO,' ');

   cls();
   MoveXY(1, maxy-3);

   print(0,54,4 , "ллллллллллллллллллллллллл");
   print(1,54,112," Made in The Netherlands ");
   print(2,54,1 , "ллллллллллллллллллллллллл");

   drawbox(copyright);
   #ifndef __OS2__
   boxwrite(copyright,0,0, " JAMINFO 1.00 (c) 1994 Gerard van Essen (2:281/527)");
   #else
   boxwrite(copyright,0,0, "JAMINFO/2 1.00 (c) 1994 Gerard van Essen (2:281/527)");
   #endif
   delbox(copyright);

   drawbox(cur_area);
   print(3,1,3," Totals: ");

   print(4,2,14,"Headers scanned : ");
   print(5,2,14,"Headers active  : ");
   print(6,2,14,"Headers deleted : ");

   print(4,39,14,"Errors found   : ");
   print(5,39,14,"Areas scanned  : ");
   print(6,39,14,"Areas corrupt  : ");

   delbox(cur_area);

   drawbox(personal);
   print(8,1,3," This area: ");
   print(9 ,2,14,"Headers scanned : ");
   print(10,2,14,"Headers active  : ");
   print(11,2,14,"Headers deleted : ");

   print(9 ,39,14,"Errors found   : ");
   print(10,39,14,"Area tag       : ");
   print(11,39,14,"Current offset : ");

   delbox(personal);

   drawbox(activity);
   print(13,1,3," Messages: ");
   delbox(activity);

   log = sopen("JAMINFO.LOG", O_CREAT | O_APPEND | O_RDWR | O_TEXT, SH_DENYWR, S_IWRITE | S_IREAD);
   logit("Begin, JAMINFO");

   if(argc > 1)
     {
     if(argv[1][strlen(argv[1])-1] == '\\')
            argv[1][strlen(argv[1])-1] = '\0';

     first = xcalloc(1, sizeof(AREA));
     first->name = strdup(argv[1]);
     first->dir = strdup(argv[1]);
     }
   else
     {

     if(ReadGEchoCFG() == -1)  // Try GEcho
       {
       if(ReadFastechoCFG() == -1)
         {
         if(ReadxMailCFG() == -1)
            {
            PutMessage("No command line parameters & no areafile found!");
            exit(254);
            }
         }
       }

     if(first == NULL)
       {
       PutMessage("No JAM areas found..");
       exit(254);
       }
     }


   for(thisone=first; thisone; thisone=thisone->next)
     {
     sprintf(tmpmsg, "Now working on: %s", thisone->name);
     PutMessage(tmpmsg);
     sprintf(temp, "%-23.23s", thisone->name);
     print(10,56,7,temp);
     CheckArea(thisone->dir);
     AreasScanned++;
     sprintf(tmpno, "%ld     ", TotalScanned);
     print(4,20,7,tmpno);
     sprintf(tmpno, "%ld     ", TotalActive);
     print(5,20,7,tmpno);
     sprintf(tmpno, "%ld     ", TotalDeleted);
     print(6,20,7,tmpno);
     sprintf(tmpno, "%ld     ", TotalErrors);
     print(4,56,7,tmpno);
     sprintf(tmpno, "%ld     ", AreasScanned);
     print(5,56,7,tmpno);
     sprintf(tmpno, "%ld     ", CorruptAreas);
     print(6,56,7,tmpno);
     }

   time(&end);

   sprintf(tmpmsg, "Done! Total runtime: %d seconds.", end - begin);
   PutMessage(tmpmsg);

   logit("End, JAMINFO\n");


   if(TotalErrors == 0)
      return 0;
   else
      return 1;
}


/* Read GEcho config files */

int ReadGEchoCFG(void)
{
   AREAFILE_HDR AreaHdr;
   AREAFILE_GE AreaFile;
   int AreaFileGE;
   word result, records, arearecsize, counter;


/*
**  Opening AREAFILE.GE and checking the header
*/
   AreaFileGE = open("areafile.ge", O_RDONLY|O_DENYNONE|O_BINARY);
   if (AreaFileGE == -1)
   {
   return -1;
   }

   PutMessage("Reading GEcho areafile");

   result = read(AreaFileGE, &AreaHdr, sizeof(AREAFILE_HDR));
   if (result < sizeof(AREAFILE_HDR))
   {
      PutMessage("Error reading AREAFILE header");
      exit(254);
   }


   arearecsize = AreaHdr.systems * sizeof(EXPORTENTRY) + AreaHdr.recsize;
   records = (word) ((filelength(AreaFileGE) - AreaHdr.hdrsize) / arearecsize);
   for (counter = 0; counter < records; counter++)
   {
      lseek(AreaFileGE, (long) AreaHdr.hdrsize + (long) arearecsize * counter, SEEK_SET);
      result = read(AreaFileGE, &AreaFile, sizeof(AREAFILE_GE));
      if (result < sizeof(AREAFILE_GE)) break;
      if ((AreaFile.options & REMOVED) == 0)
         ana_GECHO_area(&AreaFile);
   }

   close(AreaFileGE);

   return 0;

}


/* Analyse area found in GEcho areafile */

void ana_GECHO_area(AREAFILE_GE *area)
{
   AREA  *thisarea;

   if (area->areaformat != FORMAT_JAM)
      return;

   thisarea = calloc(1, sizeof(AREA));

   thisarea->name  = strdup(area->name);

   if(area->path[strlen(area->path)-1] == '\\')
      area->path[strlen(area->path)-1] = '\0';

   thisarea->dir = strdup(area->path);

   if (first == NULL)           /* first area */
        first = thisarea;
   else                             /* establish link */
      last->next = thisarea;

   last = thisarea;

}


void CheckArea(char *dir)
{
   JAMHDRINFO hdrinfo;
   JAMHDR hdr;
   JAMIDXREC idx;
   int hdrfile, index, l;
   char filename[120], tmpno[20];
   long deleted = 0,
        active  = 0,
        total = 0,
        Offset;

   unsigned long JDTsize = 0L;
   struct stat mystat;

   unsigned AreaErrors = 0;

   long totmsgs;

   unsigned curmax = 1000;

   unsigned long *ActiveNumbers;
            long *ActiveOffsets;


   print(10, 20, 7, "      ");
   print(11, 20, 7, "      ");

   sprintf(filename, "%s.JHR", dir);
   if( (hdrfile = open(filename, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
     {
     sprintf(tmpmsg, "Can't open %s!", filename);
     PutMessage(tmpmsg);
     goto Exit;
     }

   sprintf(filename, "%s.JDX", dir);
   if( (index = open(filename, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
     {
     sprintf(tmpmsg, "Can't open %s?!", filename);
     PutMessage(tmpmsg);
     close(hdrfile);
     goto Exit;
     }

   totmsgs = filelength(index) / sizeof(JAMIDXREC);

   sprintf(filename, "%s.JDT", dir);

   if(stat(filename, &mystat) != -1)
     {
     JDTsize = mystat.st_size;
     }

   if(read(hdrfile, &hdrinfo, sizeof(JAMHDRINFO)) != sizeof(JAMHDRINFO))
     {
     sprintf(tmpmsg, "Error reading JAM header from %s!", filename);
     PutMessage(tmpmsg);
     close(hdrfile);
     close(index);
     AreaErrors += 1;
     goto Exit;
     }

   if(strcmp(hdrinfo.Signature, "JAM") != 0)
      {
      sprintf(tmpmsg, "Invalid JAM signature in %s!", filename);
      PutMessage(tmpmsg);
      close(hdrfile);
      close(index);
      AreaErrors += 1;
      goto Exit;
      }


   if( (ActiveNumbers = calloc(curmax, sizeof(unsigned long))) == NULL)
      {
      PutMessage("Out of memory!");
      exit(254);
      }

   if( (ActiveOffsets = calloc(curmax, sizeof(long))) == NULL)
      {
      PutMessage("Out of memory!");
      exit(254);
      }


   while((Offset = GetNextHeader(hdrfile, &hdr)) != 0)
     {
     if(strcmp(hdr.Signature, "JAM") != 0)
        {
        PutMessage("Invalid JAM signature found!");
        AreaErrors += 1;
        break;
        }

     sprintf(tmpno, "%ld      ", ++total);
     print(9, 20, 7, tmpno);

     sprintf(tmpno, "%ld      ", Offset);
     print(11, 56, 7, tmpno);

     sprintf(tmpno, "%d      ", AreaErrors);
     print(9, 56, 7, tmpno);

     if(hdr.Attribute & MSG_DELETED) // Deleted, no further action..
        {
        sprintf(tmpno, "%ld     ", ++deleted);
        print(11, 20, 7, tmpno);
        }
     else  /* Read index, and verify Offset, check for double numbers */
        {
        /* See if this MsgNum was already found before.. */
        for(l=0; l<active; l++)
           {
           if(ActiveNumbers[l] == hdr.MsgNum)
             {
             sprintf(tmpmsg, "Warning! Found active header with MsgNum %lu at offset %lu, while", hdr.MsgNum, Offset);
             PutMessage(tmpmsg);
             sprintf(tmpmsg, "this number was also found in header at offset %lu!", ActiveOffsets[l]);
             PutMessage(tmpmsg);
             AreaErrors += 1;
             }
           }

        /* Store info for 'dupe' checking */

        if(active > curmax)
          {
          #ifndef __OS2__
          if(curmax == 16000)
             {
             PutMessage("Cannot handle areas with more than 16000 messages!");
             exit(254);
             }
          #endif

          curmax += 1000;
          if( (ActiveNumbers = realloc(ActiveNumbers, curmax * sizeof(unsigned long))) == NULL)
            {
            PutMessage("Out of memory!");
            exit(254);
            }

          if( (ActiveOffsets = realloc(ActiveOffsets, curmax * sizeof(long))) == NULL)
            {
            PutMessage("Out of memory!");
            exit(254);
            }
          }

        ActiveNumbers[(int) active] = hdr.MsgNum;
        ActiveOffsets[(int) active] = Offset;

        sprintf(tmpno, "%ld     ", ++active);
        print(10, 20, 7, tmpno);

        if(hdr.TxtOffset > JDTsize)
           {
           sprintf(tmpmsg, "Message at offset %lu has a TxtOffset of %lu", Offset, hdr.TxtOffset);
           PutMessage(tmpmsg);
           sprintf(tmpmsg, "But the .JDT file is only %lu bytes!", JDTsize);
           PutMessage(tmpmsg);
           AreaErrors += 1;
           continue;  // Can't read index for this!
           }


        if(hdr.MsgNum < hdrinfo.BaseMsgNum)
           {
           sprintf(tmpmsg, "Message at offset %lu has msgnum #%lu, BaseMsgNum is %lu!", Offset, hdr.MsgNum, hdrinfo.BaseMsgNum);
           PutMessage(tmpmsg);
           AreaErrors += 1;
           continue;  // Can't read index for this!
           }

        if(hdr.MsgNum > (hdrinfo.BaseMsgNum + totmsgs - 1))
           {
           sprintf(tmpmsg, "Message at offset %lu has msgnum #%lu, Highest should be %lu!", Offset, hdr.MsgNum, hdrinfo.BaseMsgNum + totmsgs - 1);
           PutMessage(tmpmsg);
           AreaErrors += 1;
           continue; // Can't read index for this!
           }

        /* Check index */

        if(GetIdx(index, &hdr, &hdrinfo, &idx) == -1)
           {
           PutMessage("Error reading index!");
           AreaErrors += 1;
           continue;
           }

        if(idx.HdrOffset != Offset)  /* Doesn't point to this entry! */
          {
          sprintf(tmpmsg, "Warning! Found active message header with MsgNum #%lu at", hdr.MsgNum);
          PutMessage(tmpmsg);
          sprintf(tmpmsg, "Offset %lu, but the equivalent .JDX entry .HdrOffset set to: %lu", Offset, idx.HdrOffset);
          PutMessage(tmpmsg);
          AreaErrors += 1;
          }

        }

     }

   close(hdrfile);
   GetIdx(-1, NULL, NULL, NULL);
   close(index);
   free(ActiveNumbers);
   free(ActiveOffsets);

   if(hdrinfo.ActiveMsgs != active)
      {
      sprintf(tmpmsg, "Expected active (JHR.ActiveMsgs): %lu", hdrinfo.ActiveMsgs);
      PutMessage(tmpmsg);
      sprintf(tmpmsg, "Found active   : %lu", active);
      PutMessage(tmpmsg);
      AreaErrors += 1;
      }

   Exit:

   TotalErrors += AreaErrors;
   if(AreaErrors)
      CorruptAreas++;

   TotalScanned += total;
   TotalDeleted += deleted;
   TotalActive  += active;

   sprintf(tmpno, "%d      ", AreaErrors);
   print(9, 56, 7, tmpno);

}


long GetNextHeader(int hdrfile, JAMHDR *hdr)
{
   long ThisHeaderOffset;
   static char *buf = NULL;
   static unsigned bufpos = 0, bufend;
   static long BufOffset;

   #define SIZE 16000u

   if(buf == NULL)   // Initialize
     {
     if( (buf = malloc(SIZE)) == NULL)
        {
        sprintf(tmpmsg, "Error: out of memory!");
        PutMessage(tmpmsg);
        exit(254);
        }

     BufOffset = lseek(hdrfile, 0L, SEEK_CUR);

     if( (bufend=read(hdrfile, buf, SIZE)) == (unsigned) -1)
        {
        sprintf(tmpmsg, "Error reading header file!");  // !!!!!!!!!!!!!!!!!
        PutMessage(tmpmsg);
        exit(254);
        }

     if(bufend < sizeof(JAMHDR))   // End of file reached, no more headers?!
       {
       free(buf);
       buf = NULL;
       if(bufend != 0)
          PutMessage("Possibly trailing data in Header file?");
       return 0L;
       }

     bufpos=0;
     }


   if(bufend == 0)
     {
     free(buf);
     buf=NULL;
     return 0;
     }

   if( (long) (bufpos + sizeof(JAMHDR)) > (long) bufend )
     {
     lseek(hdrfile, BufOffset + bufpos, SEEK_SET); // Sync to start new header
     BufOffset = BufOffset + bufpos;

     if( (bufend=read(hdrfile, buf, SIZE)) == (unsigned) -1)
        {
        sprintf(tmpmsg, "Error reading header file!");  // !!!!!!!!!!!!!
        PutMessage(tmpmsg);
        exit(254);
        }

     if(bufend < sizeof(JAMHDR))   // End of file reached, no more headers
       {
       free(buf);
       buf = NULL;
       if(bufend != 0)
          PutMessage("Possibly trailing data in Header file?");
       return 0L;
       }

     bufpos = 0;
     }

   memmove(hdr, buf+bufpos, sizeof(JAMHDR)); // Give header back
   ThisHeaderOffset = BufOffset + (long) bufpos;
   bufpos += sizeof(JAMHDR);

   if( (bufpos += (unsigned) hdr->SubfieldLen) > bufend )  // Running out of buf?
     {
     lseek(hdrfile, BufOffset + bufpos, SEEK_SET);
     BufOffset = BufOffset + bufpos;

     if( (bufend=read(hdrfile, buf, SIZE)) == (unsigned) -1)
        {
        sprintf(tmpmsg, "Error reading header file!");  // !!!!!!!!!!!!!!!
        PutMessage(tmpmsg);
        exit(254);
        }

     bufpos=0;
     }

   return ThisHeaderOffset;

}



int GetIdx(int index, JAMHDR *hdr, JAMHDRINFO *hdrinfo, JAMIDXREC *idx)
{
   static JAMIDXREC *idxbuf = NULL;
   static unsigned long bottom = 0, top = 0;
   long curno;
   int bytesread;
   int bufempty = 0;

   #define IDXBUFSIZE 500

   if(index == -1)  // Call with -1 to de-initialize..
     {
     if(idxbuf)
        free(idxbuf);
     idxbuf = NULL;
     top = bottom = 0;
     return 0;
     }

   if(idxbuf == NULL)
     {
     if( (idxbuf = calloc(IDXBUFSIZE, sizeof(JAMIDXREC))) == NULL)
       {
       sprintf(tmpmsg, "Out of memory!");
       PutMessage(tmpmsg);
       exit(254);
       }
     bufempty = 1;
     }

   curno = hdr->MsgNum - hdrinfo->BaseMsgNum;

   if( bufempty || ((curno < bottom) || (curno > top)) )
      {
      bottom = curno;

      if(lseek(index, (long) ((long) sizeof(JAMIDXREC) * (long)curno) , SEEK_SET) !=
             (long) ((long) sizeof(JAMIDXREC) * (long)curno))
        {
        free(idxbuf);
        idxbuf = NULL;
        bottom = top = 0;
        return -1;
        }

      if( (bytesread=read(index, idxbuf, sizeof(JAMIDXREC) * IDXBUFSIZE)) == -1)
        {
        free(idxbuf);
        idxbuf = NULL;
        bottom = top = 0;
        return -1;
        }

      if(bytesread % sizeof(JAMIDXREC) != 0)
        {
        PutMessage("Error in length of index file!?");
        }

      top = bottom + (bytesread/sizeof(JAMIDXREC)) - 1;
      }

   memmove(idx, &idxbuf[(int) (curno-bottom)], sizeof(JAMIDXREC));

   return 0;

}


void PutMessage(char *msg)
{

   bios_scroll_up(1, 14, 1, maxy-5, 78, 7);

   print(maxy-5, 2, 7, msg);

   logit(msg);

}






int ReadFastechoCFG()
{
   CONFIG *fecfg;
   int l, left, toread;
   long areaoffset = 0L;
   int cfgfile;
   Area *areas;


   if( (cfgfile=open("fastecho.cfg", O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      return -1;

   PutMessage("Reading Fastecho config file..");

   fecfg = xcalloc(1, sizeof(CONFIG));

   if(read(cfgfile, fecfg, sizeof(CONFIG)) != sizeof(CONFIG))
      {
      sprintf(tmpmsg, "Error reading fastecho.cfg!");
      PutMessage(tmpmsg);
      close(cfgfile);
      free(fecfg);
      return 0;
      }

   areaoffset = (long) sizeof(CONFIG) + (long) fecfg->offset + (long) (fecfg->NodeCnt * sizeof(Node));

   lseek(cfgfile, areaoffset, SEEK_SET);

   areas = xcalloc(25, sizeof(Area));  /* Read buffer to read disk */

   for(l=0, left = fecfg->AreaCnt; left > 0; left -= 25)
     {
     toread = min(left, 25);
     if(toread == 0)
        break;

     if(read(cfgfile, areas, (unsigned) (toread *sizeof(Area))) != (int) (toread * sizeof(Area)) )
        {
        sprintf(tmpmsg, "Error reading fastecho.cfg!");
        PutMessage(tmpmsg);
        close(cfgfile);
        free(fecfg);
        free(areas);
        return 0;
        }

     for(l=0; l<toread; l++)
       {
       ana_FE_area(&areas[l]);
       }
     }

   close(cfgfile);
   free(areas);

   return 0;

}


/* ------------------------------------------ */

void ana_FE_area(Area *area)
{
   AREA  *thisarea;


   if(area->flags.type == PT_BOARD)
        return;

   if (area->flags.type != JAM)
       return;

   thisarea = xcalloc(1, sizeof(AREA));

   strlwr(area->name);

   thisarea->name  = strdup(area->name);

   if(area->path[strlen(area->path)-1] == '\\')
      area->path[strlen(area->path)-1] = '\0';

   thisarea->dir = strdup(area->path);


   if (first == NULL)           /* first area */
        first = thisarea;
   else                             /* establish link */
      {
      last->next = thisarea;
      }

   last = thisarea;

}


/* ------------------------------------------ */


/* Write a line to the logfile (if logging is on..) */

void logit(char *line)

{
  time_t ltime;
  struct tm *tp;
  char temp[100];

  if(log == -1)
      return;

  (void)time (&ltime);
  tp = localtime (&ltime);

  sprintf (temp, "\n  %02i %03s %02i:%02i:%02i JINF %s",
      tp->tm_mday, mtext[tp->tm_mon], tp->tm_hour, tp->tm_min, tp->tm_sec,
      line);

  write(log, temp, strlen(temp));

}


int ReadxMailCFG()
{
   FILE *cfgfile;
   xMailArea area;


   if( (cfgfile=_fsopen("areas.xm", "rb", SH_DENYNO)) == NULL)
      {
      return -1;
      }

   setvbuf(cfgfile, NULL, _IOFBF, 4096);

   PutMessage("Reading xMail config files..");

   while(fread(&area, sizeof(area), 1, cfgfile) == 1 )
        {
        ana_xMail_area(&area);
        }

   fclose(cfgfile);

   return 0;

}



void ana_xMail_area(xMailArea *area)
{
   AREA  *thisarea;

   if(area->name[0] == '\0')
        return;

   if(area->base == xPT)
      return;

   if(area->base != xJAM)
      return;

   thisarea = xcalloc(1, sizeof(AREA));

   thisarea->name  = xpstrdup(area->name);

   thisarea->dir = xpstrdup(area->dir);


   if (first == NULL)           /* first area */
        first = thisarea;
   else                             /* establish link */
      last->next = thisarea;

   last = thisarea;

}


char *xpstrdup(char *str)
{
   char *out;

   out = xcalloc(1, str[0]+1);

   memmove(out, str + 1, str[0]);

   return out;

}
