#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <time.h>

#include "typedefs.h"
#include "jam.h"
#include "gestruct.h"

typedef struct _area
{
   char         *name;
   char         *dir;
   struct _area *next;

}  AREA;

AREA *first = NULL,
     *last  = NULL;

void ana_GECHO_area(AREAFILE_GE *area);
void ReadGEchoCFG(void);
void CheckArea(char *dir);

long GetNextHeader(int hdrfile, JAMHDR *hdr);
int  GetIdx(int index, JAMHDR *hdr, JAMHDRINFO *hdrinfo, JAMIDXREC *idx);


int main(int argc, char *argv[])
{
   AREA *thisone;
   time_t begin, end;

   time(&begin);

   if(argc > 1)
     {
     if(argv[1][strlen(argv[1])-1] == '\\')
            argv[1][strlen(argv[1])-1] = '\0';
     CheckArea(argv[1]);

     time(&end);

     printf("\nDone! Total runtime: %d seconds.\n", end - begin);

     exit(0);
     }

   printf("\nReading GEcho areafile..\n");

   ReadGEchoCFG();

   printf("\nDone!\n");

   if(first == NULL)
     {
     printf("\nNo JAM areas found..\n");
     exit(1);
     }

   for(thisone=first; thisone; thisone=thisone->next)
     {
     printf("\nNow working on: %s", thisone->name);
     CheckArea(thisone->dir);
     }

   time(&end);

   printf("\nDone! Total runtime: %d seconds.\n", end - begin);

   return 0;
}


/* Read GEcho config files */

void ReadGEchoCFG(void)
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
      printf("Unable to open AREAFILE.GE");
      exit(1);
   }

   result = read(AreaFileGE, &AreaHdr, sizeof(AREAFILE_HDR));
   if (result < sizeof(AREAFILE_HDR))
   {
      printf("Error reading AREAFILE header");
      exit(1);
   }

/*
**  Reading AREAFILE.GE
**
**  Method 2: Sequentially reading
**  This will read the area records in the order in which they area stored.
**  You will have to check each record to see if it has been removed.
*/

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

}

/* Analyse area found in GEcho areafile */
void ana_GECHO_area(AREAFILE_GE *area)
{
   AREA  *thisarea;

   if( area->areatype != ECHOMAIL )
        return;

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
   char filename[120];
   long deleted = 0,
        active  = 0,
        total = 0,
        Offset;

   long totmsgs;

   typedef struct
   {
      unsigned long MsgNum;
               long Offset;

   } NumIdx;

   NumIdx *ActiveNumbers;

   sprintf(filename, "%s.JHR", dir);
   if( (hdrfile = open(filename, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
     {
     printf("\nError opening %s!\n", filename);
     return;
     }

   sprintf(filename, "%s.JDX", dir);
   if( (index = open(filename, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
     {
     printf("\nError opening %s!\n", filename);
     return;
     }

   totmsgs = filelength(index) / sizeof(JAMIDXREC);

   if(read(hdrfile, &hdrinfo, sizeof(JAMHDRINFO)) != sizeof(JAMHDRINFO))
     {
     printf("\nError reading JAM header from %s!\n", filename);
     close(hdrfile);
     return;
     }

   if(strcmp(hdrinfo.Signature, "JAM") != 0)
      {
      printf("\nInvalid JAM signature in %s!\n", filename);
      close(hdrfile);
      return;
      }


   if( (ActiveNumbers = calloc(8000, sizeof(NumIdx))) == NULL)
      {
      printf("\nOut of memory!\n");
      exit(1);
      }


   while((Offset = GetNextHeader(hdrfile, &hdr)) != 0)
     {
     if(strcmp(hdr.Signature, "JAM") != 0)
        {
        printf("\nInvalid JAM signature found!\n");
        break;
        }

     if(!(total++ % 100))
       {
       putchar('.');
       }

     if(hdr.Attribute & MSG_DELETED) // Deleted, no further action..
        deleted++;
     else  /* Read index, and verify Offset, check for double numbers */
        {
        /* See if this MsgNum was already found before.. */
        for(l=0; l<active; l++)
           {
           if(ActiveNumbers[l].MsgNum == hdr.MsgNum)
             {
             printf("\nWarning! Found active header with MsgNum %lu at offset %lu, while\n", hdr.MsgNum, Offset);
             printf("this number was also found in header at offset %lu!\n", ActiveNumbers[l].Offset);
             }
           }

        /* Store info for 'dupe' checking */

        ActiveNumbers[(int) active].MsgNum = hdr.MsgNum;
        ActiveNumbers[(int) active].Offset = Offset;

        active++;

        if(hdr.MsgNum < hdrinfo.BaseMsgNum)
           {
           printf("\nMessage at offset %lu has msgnum #%lu, BaseMsgNum is %lu!\n", Offset, hdr.MsgNum, hdrinfo.BaseMsgNum);
           continue;  // Can't read index for this!
           }

        if(hdr.MsgNum > (hdrinfo.BaseMsgNum + totmsgs - 1))
           {
           printf("\nMessage at offset %lu has msgnum #%lu, Highest should be %lu!\n", Offset, hdr.MsgNum, hdrinfo.BaseMsgNum + totmsgs - 1);
           continue; // Can't read index for this!
           }

        /* Check index */

        if(GetIdx(index, &hdr, &hdrinfo, &idx) == -1)
           {
           printf("\nError reading index!\n");
           }

        if(idx.HdrOffset != Offset)  /* Doesn't point to this entry! */
          {
          printf("\nWarning! Found active message header with MsgNum #%lu at\n", hdr.MsgNum);
          printf("Offset %lu, but the equivalent .JDX entry .HdrOffset set to: %lu\n", Offset, idx.HdrOffset);
          }

        }

     /* Seek past Subfields.. */


     }

   close(hdrfile);
   close(index);
   free(ActiveNumbers);

   if(hdrinfo.ActiveMsgs != active)
      printf("\n ========> Warning! <=========\n");

   printf("\nExpected active (JHR.ActiveMsgs): %lu\nFound active   : %lu\n\n", hdrinfo.ActiveMsgs, active);


}


long GetNextHeader(int hdrfile, JAMHDR *hdr)
{
   long ThisHeaderOffset;
   static char *buf = NULL;
   static unsigned bufpos = 0, bufend;
   static long BufOffset;

   #define SIZE 64000u

   if(buf == NULL)   // Initialize
     {
     if( (buf = malloc(SIZE)) == NULL)
        {
        printf("\nError: out of memory!\n");
        exit(254);
        }

     BufOffset = lseek(hdrfile, 0L, SEEK_CUR);

     if( (bufend=read(hdrfile, buf, SIZE)) == (unsigned) -1)
        {
        printf("\nError reading header file!\n");
        exit(254);
        }

     if(bufend < sizeof(JAMHDR))   // End of file reached, no more headers?!
       {
       free(buf);
       buf = NULL;
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
        printf("\nError reading header file!\n");
        exit(254);
        }

     if(bufend < sizeof(JAMHDR))   // End of file reached, no more headers
       {
       free(buf);
       buf = NULL;
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
        printf("\nError reading header file!\n");
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
   unsigned long curno;
   int bytesread;

   #define IDXBUFSIZE 8000

   if(idxbuf == NULL)
     {
     if( (idxbuf = calloc(IDXBUFSIZE, sizeof(JAMIDXREC))) == NULL)
       {
       printf("\nOut of memory!\n");
       exit(254);
       }
     }

   curno = hdr->MsgNum - hdrinfo->BaseMsgNum;

   if( (curno < bottom) || (curno > top) )
      {
      bottom = curno;

      if(lseek(index, (long) ((long) sizeof(JAMIDXREC) * curno) , SEEK_SET) !=
             (long) ((long) sizeof(JAMIDXREC) * curno))
        {
        printf("\nError seeking index file!\n");
        free(idxbuf);
        idxbuf = NULL;
        bottom = top = 0;
        return -1;
        }

      if( (bytesread=read(index, idxbuf, sizeof(JAMIDXREC) * IDXBUFSIZE)) == -1)
        {
        printf("\nError reading index file!\n");
        free(idxbuf);
        idxbuf = NULL;
        bottom = top = 0;
        return -1;
        }

      if(bytesread % sizeof(JAMIDXREC) != 0)
        {
        printf("\nError in length of index file!\n");
        }

      top = bottom + (bytesread/sizeof(JAMIDXREC));
      }

   memmove(idx, &idxbuf[(int) (curno-bottom)], sizeof(JAMIDXREC));

   return 0;

}