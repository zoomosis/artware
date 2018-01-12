#include "includes.h"

LINE * ReadFmtBodyFromDisk(char *filename);
char * ExtractOrigin(MMSG *curmsg);


// =========================================================================

void FilterMessage(MMSG *curmsg, AREA *area, MSG *areahandle, int realbody)
{
   dword thisone;
   BOX  *inputbox;
   int ret;
   char *origin = NULL;
   char filename[80] = "";


   thisone = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));


   inputbox = initbox(10,10,15,70,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   drawbox(inputbox);
   boxwrite(inputbox,0,1,"Give the name of the file to replace body with:");

   do {
     ret = getstring(13, 12, filename, 56, 79, "",cfg.col[Centry], cfg.col[Cpoptext]);
     } while(ret != 0 && ret != ESC);

   delbox(inputbox);

   if (ret == ESC) return;

   if(realbody)   // We need to figure out what the tearline/origin is.
     origin = ExtractOrigin(curmsg);

   // We don't need the original body anymore, so let's free up the mem used
   // by it.

   FreeBlocks(curmsg->firstblock);
   curmsg->firstblock = NULL;

   curmsg->firstblock = ReadBodyFromDisk(filename);

   if(curmsg->firstblock)
     {
     if(origin) AddToBlock(curmsg->firstblock, origin, -1);

     SaveMessage(areahandle,
                 area,
                 &curmsg->mis,
                 curmsg->firstblock,
                 curmsg->ctxt,
                 MsgUidToMsgn(areahandle, thisone, UID_EXACT),
                 0);
     }

   if(origin) mem_free(origin);

}

// ==============================================================


void FilterMemory(MMSG *curmsg, int realbody)
{
   BOX  *inputbox;
   int ret;
   char *origin = NULL;
   char filename[80] = "";
   LINE *curline, *templptr, *last;


   inputbox = initbox(10,10,15,70,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   drawbox(inputbox);
   boxwrite(inputbox,0,1,"Give the name of the file to replace body with:");

   do {
     ret = getstring(13, 12, filename, 56, 79, "",cfg.col[Centry], cfg.col[Cpoptext]);
     } while(ret != 0 && ret != ESC);

   delbox(inputbox);

   if (ret == ESC) return;

   if(realbody)   // We need to figure out what the tearline/origin is.
     origin = ExtractOrigin(curmsg);

   // We don't need the original body anymore, so let's free up the mem used
   // by it.

   FreeBlocks(curmsg->firstblock);  //  <---- !?!?!?!!
   curmsg->firstblock = NULL;
   if(curmsg->txt) FreeLines(curmsg->txt);
   
   curmsg->txt = ReadFmtBodyFromDisk(filename);

   if(!curmsg->txt && !origin)  // Apparently no body?! Add some..
     origin = mem_strdup("\rNo body found!\r");

   if(origin)
     {
     if(!curmsg->txt)
        curmsg->txt = wraptext(origin, maxx, 1, 1);
     else
        {
        curline = curmsg->txt;
        while(curline->next)
          curline = curline->next;
        curline->next = wraptext(origin, maxx, 1, 1);
        if(curline->next) curline->next->prev = curline;  // backward link.
        }
     }

   if(curmsg->txt)
      CheckLines(curmsg->txt);

   if(origin) mem_free(origin);

   if(curmsg->ctxt && (curmsg->ctxt[0] != '\0'))
      {
      curmsg->ctext = CvtCtrlToKludge(curmsg->ctxt);      /* Gives a formatted copy, original kept for C)hange msg for example */
      templptr = last = wraptext(curmsg->ctext, maxx, 1, 0);  /* Format kludges */
      mem_free(curmsg->ctext);
      curmsg->ctext = NULL;
      while(last && last->next)    /* Get last line and mark as kludge.. */
          {
          last->status |= KLUDGE;
          last = last->next;
          }
      if(last)
        {
        last->status |= KLUDGE;
        last->next = curmsg->txt;
        if(curmsg->txt)
          curmsg->txt->prev=last;
        }
      if(templptr)
         curmsg->txt = templptr;   /* let it point to beginning of kludges, which is now message start */
      }


}


// =========================================================================

char *ExtractOrigin(MMSG *curmsg)
{
   LINE *thisline;
   char origin[200] = "";

   thisline = curmsg->txt;
   while(thisline)
     {
     if(thisline->status & (TEAR|ORIGIN))
        {
        strcat(origin, "\r"); // Also for first line, make sure Orig start at beginning of line!
        strcat(origin, thisline->ls);
        }
     thisline = thisline->next;
     }

   if(origin[0] != '\0')
     return mem_strdup(origin);
   else
     return NULL;

}

// =========================================================================

LINE *ReadFmtBodyFromDisk(char *filename)
{
  XFILE *in;
  char *line;
  LINE *first = NULL, *current, *last;

  in = xopen(filename);
  if(in == NULL)
    {
    sprintf(msg, "Can't open %s!", filename);
    Message(msg, -1, 0, YES);
    return NULL;
    }

  while( (line=xgetline(in)) != NULL)
    {
    current = wraptext(line, maxx, 1, 1);
    if(current)
      {
      last = current;
      while(last->next) last = last->next;
      last->status |= HCR;
      }

    if(!first)
      first = current;
    else if(current != NULL)
      {
      last = first;
      while(last->next) last = last->next;
      last->next = current; // fwd link
      current->prev = last; // backwd link
      }

    }

  xclose(in);

  return first;
}


// ==============================================================

RAWBLOCK *ReadBodyFromDisk(char *filename)
{
   XFILE *in;
   char *line;
   RAWBLOCK *blk;

   in = xopen(filename);
   if(in == NULL)
     {
     sprintf(msg, "Can't open %s!", filename);
     Message(msg, -1, 0, YES);
     return NULL;
     }

   blk = InitRawblock(8192, 2048, 8192);   // Updated for perfbeta

   while( (line=xgetline(in)) != NULL)
     {
     AddToBlock(blk, line, -1);
     AddToBlock(blk, "\r", 1);
     }

   xclose(in);

   if(blk->curlen < 2)
     {
     sprintf(msg, "File (%s) appears to be empty..", filename);
     Message(msg, -1, 0, YES);
     FreeBlocks(blk);
     blk = NULL;
     }

   return blk;

}


// =========================================================================

