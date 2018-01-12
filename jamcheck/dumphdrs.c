#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>

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
int DisplaySub(JAMHDR *hdr, char *buf );

CHAR8 * AttrToStr( UINT32 Attr );

CHAR8     AttrName [32][16] =
              {
              "Local",
              "Transit",
              "Pvt",
              "Rcvd",
              "Sent",
              "Kill",
              "Arch",
              "Hold",
              "Crash",
              "Imm",
              "Dir",
              "Gate",
              "Req",
              "File",
              "Trunc/Sent",
              "Kill/Sent",
              "Rcpt",
              "Conf",
              "Orphan",
              "Encrypt",
              "Comp",
              "Esc",
              "Fpu",
              "TypeLocal",
              "TypeEcho",
              "TypeNet",
              "n/a1",
              "n/a2",
              "n/a3",
              "NoDisp",
              "Lock",
              "Del"
              };




int main(int argc, char *argv[])
{
   AREA *thisone;

   if(argc > 1)
     {
     if(argv[1][strlen(argv[1])-1] == '\\')
            argv[1][strlen(argv[1])-1] = '\0';
     CheckArea(argv[1]);
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
   records = (filelength(AreaFileGE) - AreaHdr.hdrsize) / arearecsize;
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
   int hdrfile;
   char filename[120], *buf;
   long deleted = 0,
        active  = 0,
        total = 0;


   sprintf(filename, "%s.JHR", dir);
   if( (hdrfile = open(filename, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
     {
     printf("\nError opening %s!\n", filename);
     return;
     }

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


   while(read(hdrfile, &hdr, sizeof(JAMHDR)) == sizeof(JAMHDR))
     {
     printf("* Offset: %lu\n", lseek(hdrfile, 0L, SEEK_CUR) - sizeof(JAMHDR));
     if(strcmp(hdr.Signature, "JAM") != 0)
        {
        printf("\nInvalid JAM signature found!\n");
        break;
        }

/*     if(!(total++ % 100))
       {
       putchar('.');
       } */

       printf( "SubfieldLen  : %lu\n",   hdr.SubfieldLen );
       printf( "TimesRead    : %lu\n",   hdr.TimesRead );
       printf( "DateWritten  : %lu\n",   hdr.DateWritten );
       printf( "DateReceived : %lu\n",   hdr.DateReceived );
       printf( "DateProcessed: %lu\n",   hdr.DateProcessed );
       printf( "MsgNum       : %lu    <-----\n",   hdr.MsgNum );
       printf( "Attribute    : %s\n",    AttrToStr( hdr.Attribute ));
       printf( "Attribute2   : %lu\n",   hdr.Attribute2 );
       printf( "TxtOffset    : %lu\n",   hdr.TxtOffset );
       printf( "TxtLen       : %lu\n",   hdr.TxtLen );
       printf("\n");

     if(hdr.Attribute & MSG_DELETED)
        deleted++;
     else
        active++;

/*     if(lseek(hdrfile, hdr.SubfieldLen, SEEK_CUR) == -1)
        {
        printf("\nError seeking next header!\n");
        break;
        } */

     buf = calloc(1, hdr.SubfieldLen);
     if(read(hdrfile, buf, hdr.SubfieldLen) != hdr.SubfieldLen)
        {
        printf("\nError seeking next header!\n");
        break;
        }

     DisplaySub(&hdr, buf);

     printf("\n--------------------------------------------\n\n");

     free(buf);
     }

   close(hdrfile);

   if(hdrinfo.ActiveMsgs != active)
      printf("\n ========> Error detected!! <=========\n");

   printf("\nExpected active: %lu\nFound active   : %lu\n\n", hdrinfo.ActiveMsgs, active);


}


CHAR8 * AttrToStr( UINT32 Attr )
{
  static CHAR8    Buf [256];
  CHAR8         * p = Buf;
  int             i;

  Buf [0] = '\0';

  for( i = 0; i < 32; i++ )
    {
    if( Attr & 0x00000001L )
      {
      if( p != Buf )
        {
        *p++ = ',';
        *p++ = ' ';
        }
      strcpy( p, AttrName [i] );
      p = strchr( p, '\0' );
      }

    Attr >>= 1;
    }

  return( Buf );
}




int DisplaySub(JAMHDR *hdr, char *buf )
{

   JAMSUBFIELD *SFptr;
   char *bufptr, temp[100];
   word cursize = 1024, addsize = 512, copylen;
   sword SFleft = hdr->SubfieldLen;
   int fieldlen;

   if (SFleft == 0) return; /* No subfields to process */

   SFptr = (JAMSUBFIELD *)buf;

   while(SFleft > 0)
     {
     bufptr = (char *)SFptr + sizeof(JAMBINSUBFIELD);
     memset(temp, '\0', 100);
     switch(SFptr->LoID)
       {
       case 2:
         copylen = min(35, SFptr->DatLen);
         memmove(temp, bufptr, copylen);
         printf("Sender   : %s\n", temp);
         break;

       case 3:
         copylen = min(35, SFptr->DatLen);
         memmove(temp, bufptr, copylen);
         printf("Receiver : %s\n", temp);
         break;

       case 6:
         copylen = min(71, SFptr->DatLen);
         memmove(temp, bufptr, copylen);
         printf("Subject  : %s\n", temp);
         break;
       }

     SFleft -= (sizeof(JAMSUBFIELD) + SFptr->DatLen); /* How much left? */
     SFptr = (JAMSUBFIELD *) (bufptr + SFptr->DatLen); /* Point to next
                                                              SubField */

     }

}

