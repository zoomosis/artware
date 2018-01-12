#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "msgapi.h"

// ==========================================================================
// Add a new string to a 'STRINGLIST' list.
// Returns start of list on success, NULL on error.
// If NULL is passed for 'slist', a new list will be set up (and the start
// of the list is still returned, of course).
// ==========================================================================

STRINGLIST * AddToStringList(STRINGLIST *root, char *s, char *pw, int first)
{
   STRINGLIST * new,
              * slist;

   if(!s) return NULL;

   if( (new=calloc(1, sizeof(STRINGLIST))) == NULL )
     return NULL;

   if( (new->s = strdup(s)) == NULL )  // Make a copy of the new string
     {
     free(new);
     return NULL;
     }

   if(pw != NULL)
     {
     if( (new->pw = strdup(pw)) == NULL )  // Make a copy of the new string
       {
       free(new->s);
       free(new);
       return NULL;
       }
     }

   // if root was NULL, we set up a new one.

   if(!root)
     root = new;
   else            // Link at end of list
     {
     if(!first)    // Append at end..
       {
       slist = root;
       while(slist->next)         // Find end of list.
          slist = slist->next;
       slist->next = new;
       }
     else         // Insert at beginning
       {
       new->next = root;
       root = new;
       }
     }

   return root;

}

// ==========================================================================
// Add a new string to a 'STRINGLIST' list, with fixed length, NOT NULL
// terminated.
// Returns start of list on success, NULL on error.
// If NULL is passed for 'slist', a new list will be set up (and the start
// of the list is still returned, of course).
// ==========================================================================

STRINGLIST * AddnToStringList(STRINGLIST *root, char *s, word len, char *pw, word pwlen)
{
   STRINGLIST * new,
              * slist;

   if(!s || !len) return NULL;

   if( (new=calloc(1, sizeof(STRINGLIST))) == NULL )
     return NULL;

   // Make room for a copy of the new string, with trailing '\0'
   if( (new->s = calloc(1, len+1)) == NULL )
     {
     free(new);
     return NULL;
     }

   if(pw)
     {
     if( (new->pw = calloc(1, pwlen+1)) == NULL )
       {
       free(new->s);
       free(new);
       return NULL;
       }
     memcpy(new->pw, pw, pwlen);
     }


   memcpy(new->s, s, len);

   // if root was NULL, we set up a new one.

   if(!root)
     root = new;
   else
     {
     slist = root;
     while(slist->next)         // Find end of list.
        slist = slist->next;
     slist->next = new;
     }

   return root;

}

// =======================================================
//         Free a stringlist (return memory)
// =======================================================

void FreeStringList(STRINGLIST *start)
{
   STRINGLIST *next;

   while(start)
     {
     next = start->next;
     if(start->s)  free(start->s);
     if(start->pw) free(start->pw);
     free(start);
     start = next;
     }
}

// =======================================================
//                   Copy a stringlist
// =======================================================

void CopyStringList(STRINGLIST *in, STRINGLIST **out)
{
   for( ; in; in = in->next)
     {
     *out = AddToStringList(*out, in->s, in->pw, 0);
     }
}

// ===================================================================
//         Calc length of a stringlist, when written as subject
// ===================================================================

int StrListLen(STRINGLIST *l)
{
   int len =  0, ellen;

   while(l)
     {
     ellen = StrListElLen(l);
     if(ellen != 0)
       {
       if(len) len++;      // If not first element, add space..
       len += ellen;
       }
     l = l->next;
     }

   return len;
}

// =======================================================
//         Calc length of one element in a stringlist
// =======================================================

int StrListElLen(STRINGLIST *l)
{
  int len = 0;

  if(l && l->s)
    {
    len += strlen(l->s);
    if(l->pw)
      len += (strlen(l->pw) + 2);            // Add space, '!', pw
    }

  return len;
}


