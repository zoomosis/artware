/*
**  GEcho 1.00 Developers Kit Demonstration Source Code
**
**  Written by Gerard J. van der Land
**
**  Copyright (C) 1992 Gerard J. van der Land. All rights reserved.
**
**  Last updates: 12-Oct-92
*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

typedef unsigned char  byte;
typedef unsigned int   word;
typedef unsigned long  dword;

#include "gestruct.h"

SETUP_GE sys;
AREAFILE_HDR AreaHdr;
AREAFILE_GE AreaFile;
AREAFILE_GEX AreaIdx[MAXAREAS];
int SetupGE, AreaFileGE, AreaFileGEX;
word result, records, areas, arearecsize, counter;

int main(void)
{
/*
**  Opening and reading SETUP.GE
*/
   SetupGE = open("SETUP.GE", O_RDONLY|O_DENYWRITE|O_BINARY);
   if (SetupGE == -1)
   {
      printf("Unable to open SETUP.GE\n");
      return(255);
   }
   result = read(SetupGE, &sys, sizeof(SETUP_GE));
   close(SetupGE);
   if (result < sizeof(SETUP_GE))
   {
      printf("Error reading SETUP.GE\n");
      return(255);
   }
   if (sys.sysrev != GE_THISREV)
   {
      printf("System file revision level mismatch\n");
      return(251);
   }

   printf("SysOp: %s\n", sys.username[0]);

/*
**  Opening AREAFILE.GE and checking the header
*/
   AreaFileGE = open("AREAFILE.GE", O_RDWR|O_DENYWRITE|O_BINARY);
   if (AreaFileGE == -1)
   {
      printf("Unable to open AREAFILE.GE\n");
      return(255);
   }
   result = read(AreaFileGE, &AreaHdr, sizeof(AREAFILE_HDR));
   if (result < sizeof(AREAFILE_HDR))
   {
      printf("Error reading AREAFILE header\n");
      return(255);
   }

   if (AreaHdr.recsize < sizeof(AREAFILE_GE))
   {
      printf("Incompatible AREAFILE record size\n");
      return(255);
   }

/*
**  Reading AREAFILE.GE
**
**  Method 1: Using AREAFILE.GEX index file
**  This will read the area records in alphabetical order, removed area
**  records are automatically skipped.
*/
   AreaFileGEX = open("AREAFILE.GEX", O_RDWR|O_DENYWRITE|O_BINARY);
   if (AreaFileGEX == -1)
   {
      printf("Unable to open AREAFILE.GEX\n");
      return(255);
   }
   result = read(AreaFileGEX, &AreaIdx, sizeof(AreaIdx));
   areas = result / sizeof(AREAFILE_GEX);
   close(AreaFileGEX);

   for (counter = 0; counter < areas; counter++)
   {
      lseek(AreaFileGE, AreaIdx[counter].offset, SEEK_SET);
      result = read(AreaFileGE, &AreaFile, sizeof(AREAFILE_GE));
      if (result == sizeof(AREAFILE_GE))
      {
         printf("%u %s\n", counter+1, AreaFile.name);
      }
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
   areas = 0;
   for (counter = 0; counter < records; counter++)
   {
      lseek(AreaFileGE, (long) AreaHdr.hdrsize + (long) arearecsize * counter, SEEK_SET);
      result = read(AreaFileGE, &AreaFile, sizeof(AREAFILE_GE));
      if (result < sizeof(AREAFILE_GE)) break;
      if ((AreaFile.options & REMOVED) == 0)
         printf("%u %s\n", ++areas, AreaFile.name);
   }
   return(0);
}

/* end of file "ge_demo.c" */
