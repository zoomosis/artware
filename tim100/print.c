#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <video.h>
#include <scrnutil.h>
#include <sys\stat.h>
#include <dos.h>
#include <alloc.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "input.h"
#include "global.h"
#include "message.h"
#include "print.h"
#include "showmail.h"
#include "reply.h"
#include "showhdr.h"
#include "idlekey.h"
#include "xmalloc.h"

/* Valid filenames.. */



/* ------- the interrupt function registers -------- */

#ifndef __OS2__
typedef struct
{

    int bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,fl;

} IREGS;


static void (interrupt far *oldcrit)(void);

static void interrupt far newcrit(IREGS ir);

#endif

void PrintMessage(MMSG *curmsg, AREA *area, int hardcopy)

{
   FILE *outfile;
   char *buf;

   inst_newcrit();

   if(!hardcopy)
      {
      if ( (outfile = OpenExist(cfg.usr.writename)) == NULL)
   	  {
	     inst_oldcrit();
    	  return;
        }
      }
   else
      {
      #ifndef __OS2__
      if(strcmpi(cfg.usr.printer, "stdprn") == 0)
         outfile = stdprn;
      else
      #endif
      if( (outfile = fopen(cfg.usr.printer, "at")) == NULL)
         {
         inst_oldcrit();
         return;
         }
      }

//   buf = xmalloc(4096);
//   setvbuf(outfile, buf, _IOFBF, 4096);

   do_print(outfile, area, curmsg);

   if (
	(hardcopy) ||
	(strcmpi (cfg.usr.writename, "PRN")    == 0) ||
	(strncmpi(cfg.usr.writename, "LPT", 3) == 0) ||
	(strncmpi(cfg.usr.writename, "COM", 3) == 0)
      )
      fputc(12, outfile);


   fflush(outfile);
   #ifndef __OS2__
   if( !(hardcopy && (strcmpi(cfg.usr.printer, "stdprn") == 0)) )
   #endif
       fclose(outfile);
//   free(buf);

   inst_oldcrit();

}



FILE * OpenExist(char *filename)

{

	FILE *outfile;
	char c;
   BOX  *inputbox;
   int ret;
   struct stat mystat;


   inputbox = initbox(10,10,15,70,cfg.col.popframe,cfg.col.poptext,SINGLE,YES,' ');
   drawbox(inputbox);
   boxwrite(inputbox,0,1,"Give the name of the file to write to:");
   ret = getstring(13, 12, filename, 56, VFN,cfg.col.entry);
   delbox(inputbox);

   if (ret == ESC) return NULL;


   if ( (strcmpi (filename, "PRN")    != 0) &&
	(strncmpi(filename, "LPT", 3) != 0) &&
	(strncmpi(filename, "COM", 3) != 0) &&
        (stat(filename, &mystat) != -1) )
      {
      inputbox = initbox(10,15,18,65,cfg.col.popframe,cfg.col.poptext,S_HOR,YES,' ');
      drawbox(inputbox);
      boxwrite(inputbox,1,4,"File already exists! Append?");
      boxwrite(inputbox,3,4,"[Y]es : Append to file (or press <ENTER>)");
      boxwrite(inputbox,4,4,"[N]o  : Overwrite file.");
      boxwrite(inputbox,5,4,"<ESC> : Argh! Dunno, quit!");

      do 
         {
	 c = get_idle_key(1);
         }
      while (c != 13 && c !='y' && c != 'Y' && c != 'n' && c != 'N' && c != 27);

      delbox(inputbox);

      if (c == 27) return NULL;

      if (c == 'n' || c == 'N')
         outfile = fopen(filename, "wt");
      else
         outfile = fopen(filename, "at");

      }

   else
       outfile = fopen(filename, "at");

	if (!outfile)
		{
		Message("Error opening output file!", -1, 0, YES);
		return NULL;
		}

   return outfile;

}




void do_print(FILE *outfile, AREA *area, MMSG *curmsg)

{
   LINE *curline;
   char temp[81];


	sprintf(temp, "\n\nArea: %s\n\n", area->desc);
	fputs(temp, outfile);

	sprintf(temp, "Date : %-32.32s%40.40s\n", MakeT(&curmsg->hdr.date_written), attr_to_txt(curmsg->hdr.attr));
	fputs(temp, outfile);
	sprintf(temp, "From : %-36.36s%29.29s\n", curmsg->hdr.from, FormAddress(&curmsg->hdr.orig));
	fputs(temp, outfile);
   if(area->type == NETMAIL)
      sprintf(temp, "To   : %-36.36s%29.29s\n", curmsg->hdr.to, FormAddress(&curmsg->hdr.dest));
   else
      sprintf(temp, "To   : %-36.36s\n", curmsg->hdr.to);
	fputs(temp, outfile);
	sprintf(temp, "Subj : %-70.70s\n", curmsg->hdr.subj);
	fputs(temp, outfile);
	strcpy(temp, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴\n\n");
	fputs(temp, outfile);

	curline = curmsg->txt;

	while(curline)
		{
      if( (!(curline->status & KLUDGE)) || (cfg.usr.status & SHOWKLUDGES) )
         {
         if(curline->ls[0] == '\01')
            sprintf(temp, "@%s\n", curline->ls+1);
         else
		      sprintf(temp, "%s\n", curline->ls);
		   fputs(temp, outfile);
         }
		curline = curline->next;
		}



}

/* ------ critical error interrupt service routine ------ */

#ifndef __OS2__
static void interrupt far newcrit(IREGS ir)
{

   /* always fail */

    ir.ax = 0;

}
#endif

void inst_newcrit(void)
{
#ifndef __OS2__
   oldcrit = getvect(0x24);
   setvect(0x24, newcrit);
#endif
}

void inst_oldcrit(void)
{
   #ifndef __OS2__
   setvect(0x24, oldcrit);
   #endif
}