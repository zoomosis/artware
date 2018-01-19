#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "wrap.h"

#include <msgapi.h>
#include "nstruct.h"

static void near checkline(LINE *current);
char * pascal near nextline(void);



LINE  *current;

int lastsep;
char *curptr;

char  buf[140];
char  *s = buf;
unsigned char   pos, ll;



LINE *fastFormatText(char *txt, int llen)
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
//         current->status |= NOSPACE;
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
           memcpy(buf, buf+lastsep+1, (pos - lastsep - 1));
           buf[pos-lastsep-1] = '\0';
           pos -= (lastsep+1);
           }
         }

      lastsep=0;

      checkline(current);
      last = mem_calloc(1, sizeof(LINE));

      last->prev = current;
      current->next = last;

      current = last;
      }

   //Take care of last part..

   current->ls = mem_calloc(1, pos+1);
   memcpy(current->ls, buf, pos);
   current->len = pos;
   checkline(current);

   return first;
}



// ==============================================================

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

// ==============================================================


/* Check out what kind of line the current line is, set status-bit */


static void near checkline(LINE *current)

{
   if(current->ls[0] == 0x01)
      current->status |= KLUDGE;

   else if(
           (current->prev != NULL) &&
           (current->prev->status & KLUDGE) &&
           (!(current->prev->status & HCR))
          )
      current->status |= KLUDGE;

   else
      current->status |= NORMTXT;

}



