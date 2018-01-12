#include <alloc.h>
#include <string.h>
#include "wrap.h"
#include "xmalloc.h"

#include <video.h>

#define seperators " .,?!-=+:;"

int pascal IsQuote(char far *line);

LINE *fastFormatText(char *txt, int column)

{
   LINE	*first,
        *current,
        *last;

   char *curptr,
	*begin;

   int len;
   unsigned charsleft;

	/* Setup start of list */

   first = (LINE *)xcalloc(1, sizeof(LINE));

   current = last = first;

   curptr = begin = txt;			/* We start at the beginning ;-) */

   charsleft = strlen(begin);

   while( charsleft > (column-1))				/* Until end of text */
      {
      curptr = begin + (column-2);

      while( (*curptr != ' ')  && (curptr>begin) )
             curptr--;

      if (curptr == begin)       /* couldn't find seperator, take all */
         curptr = begin + (column-2);

      len = (int) (curptr - begin + 1);
      current->ls = xcalloc(1, len+1);
		memmove(current->ls, begin, len);
      current->len = len;
      charsleft -= len;
      if(IsQuote(current->ls))
         current->status |= QUOTE;

      begin = curptr + 1;

      last = xcalloc(1, sizeof(LINE));
      last->prev = current;
      current->next = last;
      current = last;

      }

   current->ls   = xstrdup(begin);
   current->len  = strlen(begin);
   current->next = NULL;
   current->status |= HCR;
   if(IsQuote(current->ls))
      current->status |= QUOTE;

   return (LINE *)first;
}

/*
#ifndef __OS2__
int pascal IsQuote(char far *line)
{
   asm push ds

   #ifndef __SMALL__
   asm lds si,line    /* ds:si points to line      */
   #endif
   #ifdef __SMALL__
   asm lea si,line
   #endif
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
}
#endif
*/
