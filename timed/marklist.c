#include "includes.h"

#define MLDELTA 100

char logwhat[100];

typedef int (*fptr)(const void*, const void*);
int numeric (const dword *p1, const dword *p2);

void savelist(MARKLIST *lst, FILE *out);
void log(MARKLIST *lst, char * msg, int withlist);

// Set up a MARKLIST structure

MARKLIST * InitMarkList(void)
{
   MARKLIST *ptr;

   ptr = mem_calloc(1, sizeof(MARKLIST));
   ptr->list = mem_malloc(MLDELTA * sizeof(dword));
   ptr->max = MLDELTA;

   return ptr;
}

// Release a Marklist

void DumpMarkList(MARKLIST *lst)
{
   if(lst == NULL)
      return;

   lst->max = 0;
   mem_free(lst->list);
   mem_free(lst);
}


// Add a number to the list of marked messages

void AddMarkList(MARKLIST * lst, dword no)
{
   dword *tmp;
   int i;

   if(lst == NULL)
      {
      Message("Passed a NULL marklist!", -1, 0, YES);
      return;
      }

   if(IsMarked(lst, no))   // Don't do this if already marked..
      return;

   #ifndef __OS2__
   if(lst->active >= 16299)
     {
     if(lst->active == 16299)
        Message("Cannot tag more than 16300 messages!", -1, 0, YES);
     else
        return;
     }
   #endif

   if(lst->active == lst->max)
      {
      if( (tmp=mem_realloc(lst->list, (lst->max + MLDELTA) * sizeof(dword))) == NULL)
         return;
      lst->list = tmp;
      lst->max += MLDELTA;
      }

   (lst->active)++;

   if(lst->active == 1) // First element..
     {
     lst->list[0] = no;
     return;
     }

   if(no > lst->list[lst->active-2])  // Append at end..
      {
      lst->list[lst->active-1] = no;
      return;
      }

   for(i=0; lst->list[i] < no; i++) // First one that is higher
      /* nothing */ ;

   // Shift all one up..
   memmove(&lst->list[i+1], &lst->list[i], ((lst->active-1) - i) * sizeof(dword));

   lst->list[i] = no;  // Insert our value

}

// Remove a message from the list.

void RemoveMarkList(MARKLIST *lst, dword no)
{
   int i;
   dword *ptr;

   if(lst == NULL)
      {
      Message("Passed a NULL marklist!", -1, 0, YES);
      return;
      }

   if(IsMarked(lst, no) == 0)
     return;

   // Can we shrink memory block taken by list?
   if( (lst->max > MLDELTA) &&  (lst->active < (lst->max-MLDELTA)) )
     {
     ptr = mem_realloc(lst->list, (lst->max - MLDELTA) * sizeof(dword));
     if(ptr)
       {
       lst->list = ptr;
       lst->max -= MLDELTA;
       }
     }

   if(lst->active == NULL)
     {
     mem_free(lst->list);
     lst->list = mem_calloc(1, MLDELTA * sizeof(dword));
     lst->max = MLDELTA;
     }

   for(i=0; lst->list[i] != no; i++) // First one that is higher
      /* nothing */ ;

   memmove(&lst->list[i], &lst->list[i+1], (lst->active-i-1) * sizeof(dword));
   lst->list[lst->active-1]=0;
   (lst->active)--;

}


// Check if a message is marked. Returns 1 if marked, 0 otherwise..

int IsMarked(MARKLIST *lst, dword no)
{
   if(lst == NULL)
      {
      Message("Passed a NULL marklist!", -1, 0, YES);
      return 0;
      }

   if(lst->active == 0)
      return 0;

   if(bsearch(&no, lst->list, lst->active, sizeof(dword), (fptr)numeric) != 0)
      return 1;

   return 0;

}


int numeric (const dword *p1, const dword *p2)
{
   if(*p1 < *p2)
      return -1;
   if(*p1 == *p2)
      return 0;
   return 1;
}

// Get the next Marked message, next to 'start'.

dword NextMarked(MARKLIST *lst, dword start)
{
   dword * ptr;
   int i;

   if(lst == NULL)
      {
      Message("Passed a NULL marklist!", -1, 0, YES);
      return 0;
      }

   if(lst->active == 0)
     return 0;

   ptr = (dword *)bsearch(&start, lst->list, lst->active, sizeof(dword), (fptr)numeric);

   if(ptr != NULL)
     {
     if(lst->list[lst->active-1] == start)
        return 0;
     ptr++;
     return *ptr;
     }

   for(i=0; lst->list[i]<start && i<lst->active; i++)
     /* nothing */ ;

   if(i == lst->active)  // Not found!
      return 0;

   return lst->list[i];


}

// Get the previous Marked message, next to 'start'.

dword PrevMarked(MARKLIST *lst, dword start)
{
   dword * ptr;
   int i;

   if(lst == NULL)
      {
      Message("Passed a NULL marklist!", -1, 0, YES);
      return 0;
      }


   if(lst->active == 0)
     return 0;

   ptr = (dword *)bsearch(&start, lst->list, lst->active, sizeof(dword), (fptr)numeric);

   if(ptr != NULL)
     {
     if(lst->list[0] == start)
        return 0;
     ptr--;
     return *ptr;
     }

   for(i=0; lst->list[i]<start && i<lst->active; i++)
     /* nothing */ ;

   if(i==0)
      return 0;

   return lst->list[i-1];

}

