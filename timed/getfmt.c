#include "includes.h"

#include "timer.h"

void checkaline(LINE *current);

char * pascal near nextline(void);
int IsPersonal(char *toname, NETADDR *address, AREA *area);

void FixReplies(MMSG *curmsg, MSG *areahandle);
void checkaddress(MMSG *curmsg, AREA *area);
void cleanup(char *s);
void get_JAM_thread(MMSG *curmsg, MSG *areahandle);
void AppendAtEnd(MMSG *curmsg, char *keyword, STRINGLIST *l);

LINE  *current;

char  lastsep,
      *curptr;

char  buf[140];
char  *s = buf;
unsigned char   pos, ll;


#ifdef __OS2__
//#define READBUFSIZE 16386
#define READBUFSIZE 1024
#else
#define READBUFSIZE 4096
#endif


MMSG *GetFmtMsg(long curno, MSG *areahandle, AREA *area)
{
    MMSG  *curmsg;
    MSGH  *msghandle;
    LINE *templptr = NULL, *last;
    char temp[256];
    sword level = 0;
    char *txtbuf, *newbuf, *oldstuff;
    int leftinbuf;
    unsigned len;
    char notforyou[] = "Sorry, this is a private message not from or addressed to you!";

    dword   result  = 0,
            txtlen  = 0,
            nowread = 0,
            howmuch = 0,
            ctrllen = 0;

//    sprintf(msg, "Before opening: %d", heapcheck());
//    Message(msg, -1, 0, YES);

    if (!(msghandle=MsgOpenMsg(areahandle, MOPEN_READ, curno)))
       {
       sprintf(temp, "Can't open message (%d)!", curno);
       Message(temp, 2, 0, YES);
       showerror();
       return NULL;
       }

    /* Get space for a msg */

    curmsg = mem_calloc(1, sizeof(MMSG));

    /* Read the length of this message */

    txtlen  = MsgGetTextLen(msghandle);

    if( (dword) txtlen == (dword) -1L )
       txtlen = (dword) 0L;

    curmsg->txtsize = txtlen;

    ctrllen = MsgGetCtrlLen(msghandle);

    if (ctrllen == (dword) -1L)
       ctrllen = (dword) 0L;

    curmsg->ctrlsize = (dword) ctrllen;

    /* Get the required memory to store (unformatted) text and ctrlinfo */

    txtbuf = mem_calloc(1, READBUFSIZE+1);

    howmuch = (txtlen > READBUFSIZE) ? READBUFSIZE : txtlen;

    if (ctrllen)
       curmsg->ctxt = mem_calloc(1, (unsigned) ctrllen + 1);
    else curmsg->ctxt=NULL;

    /* Get first load from disk */

    result = MsgReadMsg(msghandle, &curmsg->mis, 0L, howmuch,
                                             txtbuf, ctrllen, curmsg->ctxt);


    if(result == (dword) -1)
      {
      sprintf(temp, "Error reading msg no %d!", curno);
      Message(temp, 2, 0, YES);
      showerror();
      mem_free(txtbuf);
      if(curmsg->ctxt)   mem_free(curmsg->ctxt);
      mem_free(curmsg);
      return NULL;
      }

    nowread += howmuch;

      if((len=strlen(txtbuf)) < result)
       nowread=txtlen;                    //stop reading

    // could be NULL, if msg is only one line!

    level = CheckForCharset(curmsg->ctxt);

    if(level)   // We need to translate chars, a charset kludge was present.
      {
      // We start with the header fields..
      TranslateHeader(&curmsg->mis, level);

      newbuf = TranslateChars(txtbuf, &len, level);
      if(newbuf)
        {
        mem_free(txtbuf);
        txtbuf = newbuf;
        }
      }

    curmsg->txt = last = wraptext(txtbuf, maxx, 0, 0);

    while(last && last->next)
      last = last->next;  // Find last one for later append


    while(nowread < txtlen)
      {
      #ifndef __FLAT__
      if(coreleft() < (5 * READBUFSIZE))
        {
        txtbuf[0] = '\0';  // We stop here! No more formatting!
        Message("Not enough memory! Message only partially read!", -1, 0, YES);
        break;
        }
      #endif

      leftinbuf = strlen(txtbuf);
      howmuch = READBUFSIZE - leftinbuf;
      if( (nowread + howmuch) > txtlen )
         howmuch = txtlen - nowread;

      // We fiddled with the buffer so much, let's check if the size is
      // still ok..
      if(_msize(txtbuf) < (READBUFSIZE+1))
        txtbuf = mem_realloc(txtbuf, READBUFSIZE+1);

      result = MsgReadMsg(msghandle, NULL, nowread, howmuch, txtbuf+leftinbuf, 0, NULL);

      *(txtbuf + leftinbuf + (int) howmuch) = '\0';  // NULL terminate.

      if((len=strlen(txtbuf)) < (result+leftinbuf))
        nowread=txtlen;                            // stop reading

      nowread += howmuch;

      if(result == -1)
         {
         Message("Error reading message!", -1, 0, YES);
         showerror();
         // mem_free all mem here!
         return NULL;
         }

      // Fiddle with character translation. Don't forget there was still some
      // old junk in the buffer left, that was already translated. We do not
      // want to tranlsate that again!

      len -= leftinbuf;    // So we deduct old stuff.

      if(level)   // We need to translate chars, a charset kludge was present.
        {
        newbuf = TranslateChars(txtbuf+leftinbuf, &len, level); // xlat new stuff
        if(newbuf)
          {
          oldstuff = txtbuf;
          txtbuf = mem_calloc(1, leftinbuf + len + 1);
          memcpy(txtbuf, oldstuff, leftinbuf);    // Old stuff from prev read
          memcpy(txtbuf+leftinbuf, newbuf, len);  // The new translated stuff
          mem_free(oldstuff);
          mem_free(newbuf);
          }
        }

      if(last)
         last->next = wraptext(txtbuf, maxx, 0, 0);
      else
         curmsg->txt = last = wraptext(txtbuf, maxx, 0, 0);

      if(last && last->next)
        {
        last->next->prev = last;           // Make backward link!
        while(last->next)
          last = last->next;
        }
      }

    // Get last bit in buffer and append it to text.

    if( (leftinbuf = strlen(txtbuf)) != 0)
      {
      if(last)
        {
        last->next = mem_calloc(1, sizeof(LINE));
        last->next->ls = mem_strdup(txtbuf);
        last->next->len = strlen(txtbuf);
        last->next->prev = last;  // backward link..
        }
      else
        {
        curmsg->txt = mem_calloc(1, sizeof(LINE));
        curmsg->txt->ls = mem_strdup(txtbuf);
        curmsg->txt->len = strlen(txtbuf);
        }
      }

    mem_free(txtbuf);

    if(MsgCloseMsg(msghandle) == -1)
         Message("Error closing!", -1, 0, YES);

    curmsg->uid = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));

    checklines(curmsg->txt);

    /* Check if it is a personal message, if so set received bit */

    if(IsPersonal(curmsg->mis.to, &curmsg->mis.destfido, area))
      {
      curmsg->status |= PERSONAL;
      if (!(curmsg->mis.attr1 & aREAD))
         {
         if(MarkReceived(areahandle, area,
                  MsgGetCurMsg(areahandle), 0, (curmsg->mis.attr1 & aCFM) ? 1 : 0) == -1)
            {
            Message("Error updating message to 'Received' status!", -1, 0, YES);
            showerror();
            }
         }
      }


    if(ctrllen && (curmsg->ctxt[0] != '\0'))
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

    checkaddress(curmsg, area);

    if(curmsg->mis.seenby)
       AppendAtEnd(curmsg, "SEEN-BY: ", curmsg->mis.seenby);

    if(curmsg->mis.path)
       AppendAtEnd(curmsg, "PATH: ", curmsg->mis.path);

    if(curmsg->mis.via)
       AppendAtEnd(curmsg, "Via ", curmsg->mis.via);

    if(area->base & MSGTYPE_JAM)
       get_JAM_thread(curmsg, areahandle);

    // Finally, we'll check whether we are in an area where we need
    // to repsect the private flag. If so, _and_ this is a message
    // that we are not allowed to see, we'll dump the message and
    // replace the text with a message telling so.
    // Usually, these messages are not encountered, but if someone
    // manages to get here anyway..

    if(BePrivate(area) && !MaySee(areahandle, curno))
      {
      if (curmsg->msgtext) mem_free(curmsg->msgtext);
      curmsg->msgtext=NULL;

      FreeLines(curmsg->txt);
      curmsg->txt = NULL;

      if(curmsg->firstblock)
         FreeBlocks(curmsg->firstblock);
      curmsg->firstblock = NULL;

      strcpy(curmsg->mis.from, "timEd");
      strcpy(curmsg->mis.to, "Reader");
      strcpy(curmsg->mis.subj, "Sorry.");

      curmsg->txt = mem_calloc(1, sizeof(LINE));
      curmsg->txt->ls = mem_strdup(notforyou);
      curmsg->txt->len = strlen(notforyou);
      checklines(curmsg->txt);
      }

    return curmsg;
}


// ---------------------------------------------------------------

MMSG *GetRawMsg(long curno, MSG *areahandle, int what, int convert)
{
    MMSG  *curmsg;
    MSGH  *msghandle;

    RAWBLOCK * current=NULL,
             * blk;

    char   temp[256];
    char * tmp;
    int   bodytoo = (what == READNOBODY) ? 0 : 1;
    sword level;
    dword result  = 0,
          nowread = 0,
          howmuch = 0;


    if (!(msghandle=MsgOpenMsg(areahandle, MOPEN_READ, curno)))
       {
       sprintf(temp, "Can't open message (%d)!", curno);
       Message(temp, 2, 0, YES);
       showerror();
       return NULL;
       }

    /* Get space for a msg */

    curmsg = mem_calloc(1, sizeof(MMSG));

    /* Read the length of this message */

    curmsg->txtsize = MsgGetTextLen(msghandle);

    if(curmsg->txtsize  == (dword) -1L )
       curmsg->txtsize = (dword) 0L;

    curmsg->ctrlsize = MsgGetCtrlLen(msghandle);

    if (curmsg->ctrlsize == (dword) -1L)
        curmsg->ctrlsize = (dword) 0L;

    /* Get the required memory to store (unformatted) text and ctrlinfo */

    if (curmsg->ctrlsize)
       curmsg->ctxt = mem_calloc(1, (unsigned) curmsg->ctrlsize + 1);
    else curmsg->ctxt=NULL;

    if(bodytoo && curmsg->txtsize)
      {
      howmuch = (curmsg->txtsize > READBUFSIZE) ? READBUFSIZE : curmsg->txtsize;
      curmsg->firstblock = InitRawblock((unsigned) howmuch+1, 1024, 16000);
      current = curmsg->firstblock;
      }

    /* Get first load from disk */

    result = MsgReadMsg(msghandle,
                        &curmsg->mis,
                        0L,
                        howmuch,
                        current ? current->txt : NULL,
                        curmsg->ctrlsize,
                        curmsg->ctxt);

    if(current)
       {
       current->curlen = strlen(current->txt);
       current->curend = current->txt + current->curlen;
       }


    if(result == (dword) -1)
      {
      sprintf(temp, "Error reading msg no %d!", curno);
      Message(temp, 2, 0, YES);
      showerror();
      if(current)
         {
         mem_free(current->txt);
         mem_free(current);
         }
      if(curmsg->ctxt)   mem_free(curmsg->ctxt);
      mem_free(curmsg);
      return NULL;
      }

    nowread += howmuch;

    // Check if CharSet is present. We may then need to remap..

    if(convert)   // Do we need to do conversion, or 'real raw' read?
       level = CheckForCharset(curmsg->ctxt);
    else
       level = 0;

    while(bodytoo && (nowread < curmsg->txtsize))
      {
      howmuch = READBUFSIZE;
      if( (nowread + howmuch) > curmsg->txtsize )
         howmuch = curmsg->txtsize - nowread;

      current->next = InitRawblock((unsigned) howmuch+1, 1024, 16000);
      current = current->next;

      result = MsgReadMsg(msghandle, NULL, nowread, howmuch, current->txt, 0, NULL);

      current->curlen = strlen(current->txt);
      current->curend = current->txt + current->curlen;

      nowread += howmuch;

      if(result == -1)
         {
         Message("Error reading message!", -1, 0, YES);
         showerror();
         // mem_free all mem here!
         return NULL;
         }
     }

    if(MsgCloseMsg(msghandle) == -1)
         Message("Error closing!", -1, 0, YES);


    if(level)   // If a CharSet kludge was there, remap to local architecture.
      {
      TranslateHeader(&curmsg->mis, level);

      blk = curmsg->firstblock;
      while(blk)
        {
        if(blk->txt)
          {
          tmp = TranslateChars(blk->txt, &(blk->curlen), level);
          mem_free(blk->txt);
          blk->txt = tmp;
          blk->curend = blk->txt + blk->curlen;
          }
        blk = blk->next;
        }
      }

    return curmsg;
}



LINE *wraptext(char *txt, int llen, int end, int savelast)
{
   LINE  *first,
         *last;

   pos = 0;
   ll = (unsigned char)llen;

   if(txt == NULL)
      return NULL;

   /* Setup start of list */

   first = (LINE *)mem_calloc(1, sizeof(LINE));
   current = last = first;

   curptr = txt;        /* We start at the beginning ;-) */
   lastsep = 0;

   while(*curptr != '\0')              /* Until end of text */
      {
      curptr = nextline();

      if(*(curptr-1) == '\0') // Exit from nextline because of '\0'?
        {
        //curptr--;
        break;
        }

      if (!lastsep && !(current->status & HCR))
         {
         current->ls = mem_malloc(ll+1);
         memcpy(current->ls, buf, ll);
         current->ls[ll] = '\0';
         current->status |= NOSPACE;
         current->len = ll;
         buf[0] = '\0';
         pos = 0;
         curptr--;    // Go one position back, this isn't a space we can disregard!
         }
      else  // Ends in HCR or space
         {
         current->ls = mem_malloc(lastsep+1);
         memcpy(current->ls, buf, lastsep);
         current->ls[lastsep] = '\0';
         current->len = lastsep;

         if(current->status & HCR)
           {
           buf[0] = '\0';
           pos = 0;
           }
         else
           {
           if(pos == lastsep) Message("pos == lastsep!", -1, 0, YES);
           memcpy(buf, buf+lastsep+1, (pos - lastsep - 1));
           buf[pos-lastsep-1] = '\0';
           pos -= (lastsep+1);
           }
         }

      lastsep=0;

      if((last = mem_calloc(1, sizeof(LINE))) == NULL)
          Message("Out of mem!", -1, 254, NO);

      last->prev = current;
      current->next = last;

      current = last;
      }

   //Take care of last part..

   if(!savelast && (!end || !pos))    // Shift rest back to begin of buffer for next pass..
     {
     memcpy(txt, buf, pos);
     txt[pos] = '\0';

     if(current->ls == NULL)    // Last line was not filled, strip it
       {
       if(current->prev)        // Only do this if there is a previous line
          current->prev->next = NULL;
       if(current == first)     // We didn't do anything? (End of msg).
          first = NULL;
       mem_free(current);
       }
     }
   else  // Make line out of end of buffer..
     {
     current->ls = mem_calloc(1, pos+1);
     memcpy(current->ls, buf, pos);
     current->len = pos;
     }

   return first;
}


#ifndef __WATCOMC__
char * pascal near nextline(void)
{

   asm push ds
   asm mov dl, ll
   asm mov bh, 0
   asm mov bl, pos
   asm les di, s
   asm add di, bx
   asm mov bh, lastsep

   asm lds si, curptr

loop:
   asm lodsb
   asm cmp al, 0
   asm je zero

   asm cmp al, 32
   asm je space

   asm cmp al, 0Dh
   asm je hcr

   asm cmp al, 9
   asm je tab

   asm cmp al, 8Dh
   asm je loop

   asm cmp al, 0Ah
   asm je loop

store:
   asm stosb
   asm inc bl
   asm cmp bl, dl
   asm ja out
   asm jmp loop

space:
   asm mov bh, bl
   asm jmp store

tab:
   asm mov al, 32
   asm stosb
   asm stosb
   asm add bl, 2
   asm cmp bl, dl
   asm jbe goon
   asm mov bl, dl
goon:
   asm mov bh, bl
   asm jmp store

hcr:
   asm pop ax
   asm push ax
   asm push ds
   asm push si
   asm mov ds, ax

   asm mov byte ptr pos, bl
   asm mov byte ptr lastsep, bl

   current->status |= HCR;
   asm pop ax
   asm pop dx

   goto final;

zero:
//   asm dec si

out:
   asm pop ax        // restore old data segment for pos & lastsep
   asm push ax
   asm push ds       // push segment we have now.. (curptr)
   asm mov ds, ax
   asm mov byte ptr pos, bl
   asm mov byte ptr lastsep, bh

   asm pop dx       // dx:ax becomes curptr (input ptr)
   asm mov ax, si

final:
  asm pop ds

}

#else
char * pascal near nextline(void)
{
   register char al;
   char *out = s+pos;

   do
     {
     al = *curptr++;
     switch(al)
       {
       case '\0':
         return curptr;

       case 0x8D:
         break;

       case 0x0A:
         break;

       case 0x0D:
        lastsep = pos;
        current->status |= HCR;
        return curptr;

       case '\t':

         memcpy(out, "   ", 3);
         out += 3;
         pos += 3;
         if(pos > ll)   // Beyond end of line?
            {
            pos = ll+1;
            lastsep = ll;
            }
         else
           lastsep = pos;
         break;
    
       case ' ':
        lastsep = pos;
        // fallthru naar 'store'

       default:
         *out++ = al;
         pos++;
       }
     } while(pos <= ll);

  // First we trap a situation where a line ends in a space, and is the
  // immediately followed by a HCR. If this space is exactly at the last
  // position on the screen, timEd would wrap to the next line, and THEN
  // display the HCR, causing a blank line...

  if(lastsep == ll && *curptr == 0x0D) // Ends in space, but followed by HCR!
    {
    current->status |= HCR; // Indicate HCR
    curptr++;               // Skip this HCR, effectively added it to line now
    }

  // Now a situation where a word is separated by TWO spaces, and the FIRST
  // space is the end of the line. This would result in the next line starting
  // with the second space and looks bad. Strip the second space now, so it
  // won't happen.

  else if(lastsep == ll && *curptr == ' ' && *(curptr+1) != ' ')
    curptr++;  // Skip second space..

  return curptr;

}

#endif


void checklines(LINE *first)
{
   LINE *current, *lastorigin=NULL, *lasttear=NULL;
   int i;

   for(current=first; current; current = current->next)
     {
     if(current->ls[0] == 0x01)
        current->status |= KLUDGE;

     else if(IsQuote(current->ls) &&
             (current->prev == NULL || (current->prev->status & HCR)) )
        current->status |= QUOTE;

     else if(
             (current->prev != NULL) &&
             (current->prev->status & KLUDGE) &&
             (!(current->prev->status & HCR))
            )
        current->status |= KLUDGE;
     else
       {
       current->status |= NORMTXT;

       if((strncmp(current->ls, " * Origin:", 10)) == 0)
         lastorigin = current;
       else if((strncmp(current->ls, "--- ", 4)) == 0)
         lasttear = current;
       }
     }

   if(lastorigin)             /* Check for last origin in message */
     {
     current = lastorigin;
     current->status &= (NOSPACE|HCR);    /* clear all bits except TAB */
     current->status |= ORIGIN;           /* Set origin bit            */

     if( (current->prev) &&
         (strncmp(current->prev->ls, "---", 3) == 0)
        ) /* Is there a tearline? Just before the origin? */
         {
         current->prev->status &= (NOSPACE|HCR);   /* clear all bits except TAB */
         current->prev->status |= TEAR;      /* Set tearline bit          */
         }

     /* following the origin, there may be SEEN-BY lines.. */

//     for(current = current->next; current; current = current->next)
//       {
//       if( (strncmp(current->ls, "SEEN-BY:", 8)) == 0)
//           {
//           current->status &= (NOSPACE|HCR);    /* Clear all bits except TAB */
//           current->status |= KLUDGE;     /* Set kludge bit            */
//           }
//       else if( (current->prev) &&
//                (current->prev->status & KLUDGE) &&
//                (!(current->prev->status & HCR)) )
//                {
//                current->status &= (NOSPACE|HCR);    /* Clear all bits except TAB */
//                current->status |= KLUDGE;     /* Set kludge bit            */
//                }
//       }
     }
   else if(lasttear)
     {
     current = lasttear;
     i = 0;                // Count number of lines after possible tearline
     while(current->next)
       {
       i++;
       current = current->next;
       }
     if(i < 3) // Only if not more than 3 lines after tearline.
       {
       lasttear->status &= (NOSPACE|HCR);    /* clear all bits except TAB */
       lasttear->status |= TEAR;             /* Set origin bit            */
       }
     }
}


int IsPersonal(char *toname, NETADDR *address, AREA *area)
{
   int l, p;

    for(l=0; ((cfg.usr.name[l].name[0] != '\0') && (l<tMAXNAMES)); l++)
        {
        if(strcmpi(toname, cfg.usr.name[l].name)==0)
           {
           for(p=0; ((cfg.usr.address[p].zone != 0) && (p<tMAXAKAS)); p++)
              {
              if(
                 (area->type != NETMAIL) ||
                 (addrcmp(&cfg.usr.address[p], address)==0)
                )
              return 1;
              }
           }
        }

    return 0;
}


void FreeLines(LINE *line)
{
   LINE *next;

   while(line)
      {
      next = line->next;
      if(!(line->ls))
         Message("Attempt to mem_free a NULL line!", -1, 0, YES);
      else
         {
//         memset(line->ls, '%', line->len);   // !!!!!!!!!!!!!!
         mem_free(line->ls);
         }
//      memset(line, '%', sizeof(LINE));  // !!!!!!!!!!!!!!!
      mem_free(line);
      line = next;
      }

}

// ================================================================

void AppendAtEnd(MMSG *curmsg, char *keyword, STRINGLIST *l)
{
   char temp[85]="";
   int len = strlen(keyword);
   LINE *lastline, *curline;

   lastline = curmsg->txt;
   while(lastline && lastline->next)
     lastline = lastline->next;

   while(l)
     {
     if(l->s != NULL && l->s[0] != '\0')
       {
       strcpy(temp, keyword);
       strncpy(temp+len, l->s, 80-len-1);
       curline = mem_calloc(1, sizeof(LINE));
       curline->ls     = mem_strdup(temp);
       curline->status = (KLUDGE|HCR);
       curline->len    = strlen(temp);
       if(!lastline)  // first line of message!
         curmsg->txt = curline;
       else
         {
         lastline->next = curline;
         curline->prev = lastline;
         }
       lastline = curline;
       }
     l = l->next;
     }
}

// ================================================================
