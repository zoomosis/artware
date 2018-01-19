#include "includes.h"

/* Valid filenames.. */



/* ------- the interrupt function registers -------- */

#if !defined(__OS2__) && !defined(__NT__)
typedef struct
{

    int bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,fl;

} IREGS;


static void (interrupt far *oldcrit)(void);

static void interrupt far newcrit(IREGS ir);

#endif


int ShowOutFiles(char *outstring);



void PrintMessage(MMSG *curmsg, AREA *area, MSG *areahandle, int hardcopy, int tagged, int what)

{
   FILE *outfile;
   int i;
   dword msgno;
   MMSG *thismsg;
   char temp[80];

   if(tagged && area->mlist->active == 0)
     {
     Message("There are no tagged messages!", -1, 0, YES);
     return;
     }

   if(hardcopy)
     {
     if(PrinterReady(NULL) == 0)
       {
       Message("Printer not ready.", -1, 0, YES);
       return;
       }

     inst_newcrit();
     }

   if(!hardcopy)
      {
      if ( (outfile = OpenExist(cfg.usr.writename, area, areahandle, curmsg)) == NULL)
         return;
      }
   else  // Printer stuff..
      {
      #if !defined(__OS2__) && !defined(__NT__)
      if(strcmpi(cfg.usr.printer, "stdprn") == 0)
         outfile = stdprn;
      else
      #endif
      if( (outfile = _fsopen(cfg.usr.printer, "wt", SH_DENYWR)) == NULL)
        {
        inst_oldcrit();
        Message("Error opening output stream to printer!", -1, 0, YES);
        return;
        }
      }

   if( (tagged == 0) &&
       ((area->mlist->active == 0) ||
       (confirm("Process all marked messages? [y/N]")==0)) )
      {
      do_print(outfile, area, curmsg, what); // Do only this message..
      }
   else  // Do all marked messages...
     {
     savescreen();
     for(i=0; i<area->mlist->active; i++)
       {
       if(xkbhit() && get_idle_key(0, GLOBALSCOPE) == 27)
          break;

       if( (msgno=MsgUidToMsgn(areahandle, area->mlist->list[i], UID_EXACT)) == 0L)
          continue;  // Doesn't exist (anymore?).

       if ( (thismsg = GetFmtMsg(msgno, areahandle, area)) == NULL)
          continue;

       sprintf(temp, " þ Now working on message %lu..", msgno);
       statusbar(temp);

       do_print(outfile, area, thismsg, what);
       ReleaseMsg(thismsg, 1);
       }
     putscreen();
     }

   if (
        (hardcopy) ||
        (strcmpi (cfg.usr.writename, "PRN")    == 0) ||
        (strncmpi(cfg.usr.writename, "LPT", 3) == 0) ||
        (strncmpi(cfg.usr.writename, "COM", 3) == 0)
      )
      fputc(12, outfile);


   if(fflush(outfile) != 0)
     Message("Error flushing output file!", -1, 0, YES);

   #if !defined(__OS2__) && !defined(__NT__)
   if( !(hardcopy && (strcmpi(cfg.usr.printer, "stdprn") == 0)) )
   #endif
   if(fclose(outfile) != 0)
     Message("Error closing output file!", -1, 0, YES);

   if(hardcopy)
      inst_oldcrit();

}



FILE * OpenExist(char *filename, AREA *area, MSG *areahandle, MMSG *curmsg)
{
   FILE *outfile;
   char c;
   BOX  *inputbox;
   int ret;
   int listreturn;
   struct stat mystat;
   char *WhatToOpen;

   inputbox = initbox(10,2,15,77,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   drawbox(inputbox);
   boxwrite(inputbox,0,1,"Give the name of the file to write to:");
   do
     {
     listreturn = 0;
     ret = getstring(13, 4, filename, 72, 72, "",cfg.col[Centry], cfg.col[Cpoptext]);
     if(ret == TAB)
        listreturn = ShowOutFiles(filename);
     }
   while(listreturn == -1);

   delbox(inputbox);

   if (ret == ESC) return NULL;

   if(area && areahandle && curmsg)
     WhatToOpen = BuildCommandLine(filename, area, areahandle, curmsg, NULL, NULL);
   else
     WhatToOpen = mem_strdup(filename);

   if(WhatToOpen == NULL)
      return NULL;

   if ( (strcmpi (WhatToOpen, "PRN")    != 0) &&
        (strncmpi(WhatToOpen, "LPT", 3) != 0) &&
        (strncmpi(WhatToOpen, "COM", 3) != 0) &&
        (stat(WhatToOpen, &mystat) != -1) )
      {
      inputbox = initbox(10,15,18,65,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
      drawbox(inputbox);
      boxwrite(inputbox,1,4,"File already exists! Append?");
      boxwrite(inputbox,3,4,"[Y]es : Append to file (or press <ENTER>)");
      boxwrite(inputbox,4,4,"[N]o  : Overwrite file.");
      boxwrite(inputbox,5,4,"<ESC> : Argh! Dunno, quit!");

      do
         {
         c = get_idle_key(1, GLOBALSCOPE);
         }
      while (c != 13 && c !='y' && c != 'Y' && c != 'n' && c != 'N' && c != 27);

      delbox(inputbox);

      if (c == 27) return NULL;

      if (c == 'n' || c == 'N')
         outfile = _fsopen(WhatToOpen, "w+t", SH_DENYWR);
      else
         outfile = _fsopen(WhatToOpen, "a+t", SH_DENYWR);

      }

   else
       outfile = _fsopen(WhatToOpen, "a+t", SH_DENYWR);

   mem_free(WhatToOpen);

   if (!outfile)
      {
      Message("Error opening output file!", -1, 0, YES);
      return NULL;
      }

   return outfile;

}



void do_print(FILE *outfile, AREA *area, MMSG *curmsg, int what)

{
   LINE *curline;
   char temp[200];


   if(what == PRINTALL)
     {
     sprintf(temp, "\nArea : %s\n\n", area->desc);
     fputs(temp, outfile);
     }

   if(what == PRINTALL || what == PRINTHDR)
     {
     sprintf(temp, "Date : %-32.32s%40.40s\n", MakeT(curmsg->mis.msgwritten, DATE_HDR), attr_to_txt(curmsg->mis.attr1, curmsg->mis.attr2));
     fputs(temp, outfile);
     sprintf(temp, "From : %-36.36s%29.29s\n", curmsg->mis.from, FormAddress(&curmsg->mis.origfido));
     fputs(temp, outfile);

     if(area->type == NETMAIL)
       {
       if(curmsg->mis.destinter[0] == '\0')
          sprintf(temp, "To   : %-36.36s%29.29s\n", curmsg->mis.to, FormAddress(&curmsg->mis.destfido));
       else
          sprintf(temp, "To   : %s (via %s, %s)\n", curmsg->mis.destinter, curmsg->mis.to, FormAddress(&curmsg->mis.destfido));
       }
     else if(area->type != NEWS)
       sprintf(temp, "To   : %-36.36s\n", curmsg->mis.to);
     else
       sprintf(temp, "Org  : %-36.36s\n", curmsg->org);

     fputs(temp, outfile);

     if(curmsg->mis.subj[0] == '\0' && (curmsg->mis.attr1 & (aFRQ|aFILE)))
        {
        SumAttachesRequests(&curmsg->mis, curmsg->mis.subj, 99, SARboth);
        }

     sprintf(temp, "Subj : %-70.70s\n", curmsg->mis.subj);
     fputs(temp, outfile);
     strcpy(temp, "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n\n");
     fputs(temp, outfile);
     }


   if(what != PRINTHDR)
     {
     curline = curmsg->txt;

     while(curline)
        {
        if(curline->status & KLUDGE)
          {
          if(what == PRINTALL && (cfg.usr.status & SHOWKLUDGES))
            {
            if(curline->ls[0] == '\01')
               sprintf(temp, "@%s\n", curline->ls+1);
            else
               sprintf(temp, "%s\n", curline->ls);
            fputs(temp, outfile);
            }
          }
        else if(curline->status & (ORIGIN|TEAR))
          {
          if(what != PRINTREALBODY)
            {
            sprintf(temp, "%s\n", curline->ls);
            fputs(temp, outfile);
            }
          }
        else  // Normal line
          {
          sprintf(temp, "%s\n", curline->ls);
          fputs(temp, outfile);
          }

        curline = curline->next;
        }
     }

}

/* ------ critical error interrupt service routine ------ */

#if !defined(__OS2__) && !defined(__NT__)
static void interrupt far newcrit(IREGS ir)
{

   /* always fail */

    ir.ax = 0;

}
#endif

void inst_newcrit(void)
{
#if !defined(__OS2__) && !defined(__NT__)
#ifdef __WATCOMC__

  #define getvect _dos_getvect
  #define setvect _dos_setvect

#endif

   oldcrit = getvect(0x24);
   setvect(0x24, newcrit);

#endif
}

void inst_oldcrit(void)
{
   #if !defined(__OS2__) && !defined(__NT__)
   setvect(0x24, oldcrit);
   #endif
}


// ====================================================

int ShowOutFiles(char *outstring)
{
   OUTFLIST *current;
   int count = 1, maxlen=1, i, ret;
   char **flist;
   char temp[135], format[20];

   // How many do we have?
   for(current = cfg.usr.outfiles; current; current = current->next)
     {
     count++;
     maxlen = max(strlen(current->filename), maxlen);
     }

   if(count == 1) return -1;  // No names added..

   sprintf(format, " %%-%d.%ds ", maxlen, maxlen);
   flist = mem_calloc(count, sizeof(char *));

   for(current = cfg.usr.outfiles, i=0; current; current = current->next, i++)
     {
     sprintf(temp, format, current->filename);
     flist[i] = mem_strdup(temp);
     }

   maxlen = min(maxx-6, maxlen);
   maxlen = (maxlen/2)+2;

   ret = pickone(flist, 5, (maxx/2)-maxlen, maxy-2, (maxx/2)+maxlen);

   if(ret != -1)
     {
     for(i=0, current=cfg.usr.outfiles; i<ret; i++, current=current->next)
       ; /* nothing */
     strcpy(outstring, current->filename);
     }

   free_picklist(flist);

   return (ret == -1) ? -1 : 0;

}

// ====================================================

