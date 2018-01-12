#include "includes.h"


#define seperators " .,?!-=+:;"


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

   first = (LINE *)mem_calloc(1, sizeof(LINE));

   current = last = first;

   curptr = begin = txt;			/* We start at the beginning ;-) */

   charsleft = strlen(begin);

   while( charsleft > (column-1))				/* Until end of text */
      {
      curptr = begin + (column-2);

      // We may have a line ending in two spaces. In that case, we will not
      // separate the two spaces (some typists use two spaces to separate
      // sentences).

      if(*curptr == ' ' && *(curptr+1) == ' ' && *(curptr-1) != ' ')
        curptr--;

      while( (*curptr != ' ')  && (curptr>begin) )
             curptr--;

      if (curptr == begin)       /* couldn't find seperator, take all */
         {
         curptr = begin + (column-2);
         current->status |= NOSPACE;
         }

      len = (int) (curptr - begin + 1);
      current->ls = mem_calloc(1, len+1);
		memcpy(current->ls, begin, len);
      current->len = len;
      charsleft -= len;
      if(IsQuote(current->ls))
         current->status |= QUOTE;

      begin = curptr + 1;

      last = mem_calloc(1, sizeof(LINE));
      last->prev = current;
      current->next = last;
      current = last;

      }

   current->ls   = mem_strdup(begin);
   current->len  = strlen(begin);
   current->next = NULL;
   current->status |= HCR;
   if(IsQuote(current->ls))
      current->status |= QUOTE;

   return (LINE *)first;
}

