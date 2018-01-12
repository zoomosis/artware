
#include "includes.h"


// Allocate the control block, set up initial values

RAWBLOCK * InitRawblock(unsigned startlen, unsigned delta, unsigned maxlen)
{
   RAWBLOCK *thisone = mem_calloc(1, sizeof(RAWBLOCK));

   thisone->txt       = mem_calloc(1, (unsigned)startlen);
   thisone->curmaxlen = startlen;
   thisone->maxlen    = maxlen;
   thisone->delta     = delta;
   thisone->curend    = thisone->txt;

   return thisone;
}

// Add a stream of charcters to the block chain..

void AddToBlock(RAWBLOCK *blk, char *s, unsigned len)
{
   char *tmp;

   while(blk->next)     // Get a block with space left..
      blk = blk->next;

   if(s == NULL)
      return;

   if(*s == '\0' && len != 1)
      return;

   if(len == (unsigned) -1)
      len = strlen(s);

   if( (blk->curlen + len + 1) > blk->curmaxlen )  // Not big enough to add, mem_realloc
      {
      if(len > blk->delta) blk->delta = (len+10);  // Otherwise we trap with huge additions..

      if( (blk->curmaxlen + blk->delta) > blk->maxlen )  // Can't add to this one
         {
         blk->next = InitRawblock(blk->delta, blk->delta, blk->maxlen);
         blk = blk->next;
         }
      else
         {
         tmp = mem_realloc(blk->txt, blk->curmaxlen + blk->delta);

         if(!tmp)
           {
           Message("Out of memory!", -1, 254, YES);
           }
         blk->txt = tmp;
         blk->curmaxlen += blk->delta;
         blk->curend = blk->txt + blk->curlen;
         }
      }

   memcpy(blk->curend, s, len);
   blk->curend += len;
   *(blk->curend) = '\0';
   blk->curlen += len;
}

// Give back last character of block

char GetLastChar(RAWBLOCK *blk)
{
   RAWBLOCK *prev;

   if(blk->curlen == 0)
      return '\0';

   if(blk->next == NULL)  // There is only one block
     return *(blk->curend-1);

   while(blk->next)
      {
      prev = blk;
      blk = blk->next;
      }

   if(blk->curlen == 0)    // Nothing here, give last char of prev blk
      return *(prev->curend-1);

   return *(blk->curend-1);

}

// Strip last character of block

void StripLastChar(RAWBLOCK *blk)
{
   RAWBLOCK *prev;

   if(blk->curlen == 0)
      return;

   if(blk->next == NULL)  // There is only one block
     {
     *(blk->curend-1) = '\0';
     blk->curlen--;
     return;
     }

   while(blk->next)
      {
      prev = blk;
      blk = blk->next;
      }

   if(blk->curlen == 0)    // Nothing here, strip last char of prev blk
      {
      *(prev->curend-1) = '\0';
      prev->curlen--;
      return;
      }

   *(blk->curend-1) = '\0';
   blk->curlen--;

}


// Set last char of block to..

void SetLastChar(RAWBLOCK *blk, char c)
{
   RAWBLOCK *prev;
   char tmp[2];

   if(blk->curlen == 0)
      {
      sprintf(tmp, "%c", &c);
      AddToBlock(blk, tmp, 1);
      return;
      }

   if(blk->next == NULL)  // There is only one block
     {
     *(blk->curend-1) = c;
     return;
     }

   while(blk->next)
      {
      prev = blk;
      blk = blk->next;
      }

   if(blk->curlen == 0)    // Nothing here, goto last char of prev blk
      blk = prev;

   *(blk->curend-1) = c;

}



RAWBLOCK * JoinLastBlocks(RAWBLOCK *first, int minsize)
{
   RAWBLOCK *last, *prev = NULL;
   char * tmp;

   if(!first) return NULL;

   for(last=first; last->next; last=last->next)
     prev = last;

   // Only 1 block, or last block big enough.
   if( (last->curlen >= minsize) || (!prev) )
      return last;

   if( (tmp=realloc(prev->txt, last->curlen + prev->curlen + 1)) == NULL)
      return last;

   prev->txt = tmp;
   prev->curmaxlen = last->curlen + prev->curlen + 1;

   memcpy(prev->txt+prev->curlen, last->txt, last->curlen+1);

   prev->curlen += (last->curlen+1);
   prev->curend = prev->txt + prev->curlen;

   prev->next = NULL;

   FreeBlocks(last);

   return prev;

}



void FreeBlocks(RAWBLOCK *blk)
{
   RAWBLOCK *next;

   while(blk)
      {
      next = blk->next;
      if(!(blk->txt))
         Message("Attempt to mem_free a NULL txt block!", -1, 0, YES);
      else
         {
//         memset(blk->txt, '%', blk->curmaxlen);
         mem_free(blk->txt);
         }
//      memset(blk, '%', sizeof(RAWBLOCK));
      mem_free(blk);
      blk = next;
      }

}
