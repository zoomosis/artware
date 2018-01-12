#include <alloc.h>
#include <string.h>
#include <stdio.h>
#include "wrap.h"
#include "xmalloc.h"

#include <video.h>
#include <scrnutil.h>
#include "working.h"
#include <msgapi.h>
#include "tstruct.h"
#include "global.h"
#include "message.h"
#include "timer.h"
#include "conio.h"


static void near checkline(LINE *current);

int   pascal IsQuote     (char *line);           /* 'Guest' prototype, reply.c/h */

LINE *FormatText(char *txt, int llen)
{
	LINE 	*first,
			*current,
			*last,
         *lastorigin=NULL;

    char  *lastsep,
          *begin,
          *curptr;


    register char thisone;

	unsigned char   pos=0, ll=(unsigned char)llen;

   #define TABSIZE 3

   unsigned no_of_lines=1; /* for working indicator only */

	/* Setup start of list */

	first = (LINE *)xcalloc(1, sizeof(LINE));
	current = last = first;

	curptr = begin = txt;			/* We start at the beginning ;-) */
	lastsep = NULL;

    while((thisone=*curptr)!='\0')              /* Until end of text */
		{
      if( (thisone) > 32)
        {
        curptr++;
        }
      else if(thisone == ' ')
			{
         lastsep = curptr++;
         }
      else if(thisone == 0x0D)
         {
         lastsep = curptr++;
         current->status |= HCR;
         pos = ll;               /* force new line */
         }
      else if(thisone == '\t')
         {
         current->status |= WITHTAB;
         pos += (TABSIZE-1);
         lastsep = curptr++;
         }
      else
         {
         curptr++;
         }

		if (++pos > ll)		/* Here we update the pos pointer as well.. */
			{
			if (!lastsep)
					lastsep=begin+ll-2;

			current->ls = begin;

         if( (strncmp(current->ls, " * Origin:", 10)) == 0)
             lastorigin = current;

         *lastsep='\0';

         checkline(current);

			curptr = begin = lastsep+1;
			lastsep = NULL;
			pos=0;


			if((last = xcalloc(1, sizeof(LINE))) == NULL)
             Message("Out of mem!", -1, 254, NO);

			last->prev = current;
			current->next = last;

			current = last;

         if(!(no_of_lines++ % 50))
            {
            if(no_of_lines > 100)
               working(maxy-1,79, cfg.col.msgbar);
            }
			}
		}

   if( (current != first) && (*begin == '\0') && lastorigin)
     {
     current->prev->next=NULL;
     free(current);
     }
   else
     {
	  current->ls = begin;
     current->next = NULL;
     checkline(current);

     if( (strncmp(current->ls, " * Origin:", 10)) == 0)
      lastorigin = current;
     }

   if(lastorigin)             /* Check for last origin in message */
     {
     current = lastorigin;
     current->status &= (WITHTAB|HCR);    /* clear all bits except TAB */
     current->status |= ORIGIN;     /* Set origin bit            */

     if( (current->prev) &&
         (strncmp(current->prev->ls, "---", 3) == 0)
        ) /* Is there a tearline? Just before the origin? */
         {
         current->prev->status &= (WITHTAB|HCR);   /* clear all bits except TAB */
         current->prev->status |= TEAR;      /* Set tearline bit          */
         }

     /* following the origin, there may be SEEN-BY lines.. */

     for(current = current->next; current; current = current->next)
       {
       if( (strncmp(current->ls, "SEEN-BY:", 8)) == 0)
           {
           current->status &= (WITHTAB|HCR);    /* Clear all bits except TAB */
           current->status |= KLUDGE;     /* Set kludge bit            */
           }
       else if( (current->prev) &&
                (current->prev->status & KLUDGE) &&
                (!(current->prev->status & HCR)) )
                {
                current->status &= (WITHTAB|HCR);    /* Clear all bits except TAB */
                current->status |= KLUDGE;     /* Set kludge bit            */
                }
       }
     }

	return first;
}



/* Check out what kind of line the current line is, set status-bit */


static void near checkline(LINE *current)

{
   if(current->ls[0] == 0x01)
      current->status |= KLUDGE;

   else if(IsQuote(current->ls))
      current->status |= QUOTE;


   else if(
           (current->prev != NULL) &&
           (current->prev->status & KLUDGE) &&
           (!(current->prev->status & HCR))
          )
      current->status |= KLUDGE;

   else
      current->status |= NORMTXT;

}

/*
int slowquote(char *line)
{
  register char i;

  for(i=0; (i<7) && (*line!='\0'); i++)
     {
     if(*line== '>') return 1;
     if(*line++== '<') return 0;
     }

  return 0;

}


int fastquote(char *line)
{
   asm push ds

   asm lds si,line    /* ds:si points to line      */
   asm mov cx,7       /* No more than 7 characters */
   asm cld            /* We move forward           */

again:
   asm lodsb         /* Get a char in al           */
   asm or al,al      /* Is it a 0? End of string?  */
   asm jz nogo       /* Yes, return 0              */
   asm cmp al,60     /* Is it a '<' ?              */
   asm je nogo       /* Yes, return 0              */
   asm cmp al,62     /* Is it '>' ?                */
   asm loopne again  /* no, Load next char         */
   asm jne nogo      /* 7 chars scanned, return 0  */
   asm mov ax,1      /* Lets return 1              */
   asm jmp getout    /* And get out                */

nogo:
   asm xor ax,ax     /* Return 0                   */

getout:

   asm pop ds
}*/
