#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <share.h>

#include "includes.h"
#include "cbtreex.h"
#include "nodelist.h"


void       show_node_details(ADDRLIST *current, int curline);
ADDRLIST * GetLoad(NODEHANDLE *nhandle);
ADDRLIST * GetAddrCopy(ADDRLIST *in);
ADDRLIST * NodeLookup(char *name, int sysop, int prompt);
void       FreeNodesList(ADDRLIST *first);
int        DoesMatch(char *name, char *foundname);
char     * LastNameFirst(char *name, int withcomma);

// ==============================================================

ADDRLIST * NodeLookup(char *name, int sysop, int prompt)
{
   unsigned short status;
   ADDRLIST *first = NULL, *current, *highlighted;
   int i, key, lastnumber;
   int top;
   int moveback = 0;
   BOX *nodebox = NULL;
   int curpos = 0;
   int colour;
   int doread = 1;
   int prevmatches, nextmatches;
   NODEHANDLE *nhandle;
   ADDRLIST found;
   ADDRLIST *retnode = NULL;
   int clockstate = showclock;
   int lookupfirstmatch = 0;  // If this is 1, we 'walk' through displayed
                              // list to find first match, so we can high-
                              // light it.


   if( (nhandle = NodeListOpen(sysop)) == NULL)
     return NULL;

   status = NodeListSearch(nhandle,
                           sysop ? NULL : (NETADDR *)name,
                           sysop ? name : NULL,
                           0,
                           NULL);

   if(status == ERROR)
     {
     NodeListClose(nhandle);
     return NULL;
     }

   if(!sysop && prompt == 0)  // Forced lookup of node number, no prompting
     {
     if(status == FOUND)
       {
       NodeListCurr(nhandle, &found);
       retnode = GetAddrCopy(&found);
       }

     NodeListClose(nhandle);
     return retnode;
     }

   if(sysop && prompt == 0)   // Lookup name, no prompting
     {
     if(status != FOUND)
       {
       NodeListClose(nhandle);
       return NULL;
       }

     NodeListMark(nhandle);
     prevmatches = nextmatches = 0;
     status = NodeListPrev(nhandle, &found);
     if(status == OK && DoesMatch(name, found.name))
       prevmatches = 1;

     NodeListFindMark(nhandle, NULL);

     status = NodeListNext(nhandle, &found);
     if(status == OK && DoesMatch(name, found.name))
       nextmatches = 1;

     NodeListFindMark(nhandle, NULL);
     if(nextmatches == 0 && prevmatches == 0)   // Only one match?
       {
       NodeListCurr(nhandle, &found);
       retnode = GetAddrCopy(&found);
       NodeListClose(nhandle);
       return retnode;
       }
     }

   if(status == EOI) // Not found, end of list. Go back 'one screen'.
     {
     moveback = (maxy-2);
     }

   else if(status == NOTFOUND)  // Not found, go back half screen.
     {
     moveback = (maxy-2)/2;
     curpos = (maxy-2)/2;
     }

   else if(status == FOUND)  // We found it, now go back until first match
     {
     lookupfirstmatch = 1;

     if(sysop != 0)  // For address lookups, there can only be one match!
       {
       do
          {
          status = NodeListPrev(nhandle, &found);
          if(status != OK)
            {
            if(status == ERROR)
               break;
            if(status == BOI)
               NodeListHead(nhandle, NULL);
            }
          } while(DoesMatch(name, found.name));

       // If we went back one too far, go one forward for first match
       NodeListCurr(nhandle, &found);
       if(!DoesMatch(name, found.name))
         NodeListNext(nhandle, NULL);
       }
     }


   while(1)
     {
     while(moveback)
       {
       if(NodeListPrev(nhandle, NULL) != OK)
         break;
       moveback--;
       }

     moveback = 0;

     if(doread)
       {
       if(first)
          FreeNodesList(first);
       first = GetLoad(nhandle);
       if(!first)
         {
         NodeListClose(nhandle);
         return NULL;
         }
       }

     // Count 'em
     for(current=first, i=0; current; current=current->next)
       i++;

     if(nodebox && lastnumber != i)   // New box different size..
       {
       delbox(nodebox);
       nodebox = NULL;
       }

     lastnumber = i;
     if(curpos > (lastnumber-1)) curpos = (lastnumber-1);

     if(!nodebox)
       {
       top = (maxy/2 - i/2 - 1);
       if((top + i + 2) > (maxy-1))
         top--;
       if(top < 0) top = 0;
       nodebox = initbox(top, 0, top+i+1, 79, cfg.col[Casframe], cfg.col[Castext], SINGLE, YES, ' ');
       clockoff();
       drawbox(nodebox);
       biprinteol(maxy-1,0,cfg.col[Cmsgbar],cfg.col[Cmsgbaraccent]," Use ~TAB~ for details, cursor keys and page-up/down to move, ~enter~ to accept",'~');
       }

     if(lookupfirstmatch)  // Highlight first match we found.
       {
       lookupfirstmatch = 0;
       for(current=first, i=0; current; current=current->next, i++)
         {
         if( (sysop && DoesMatch(name, current->name)) ||
             (!sysop && addrcmp((NETADDR *)name, &current->address)==0) )
           {
           curpos = i;
           break;
           }
         }
       }

     for(current=first, i=0; current; current=current->next, i++)
       {
       colour = (i == curpos) ? cfg.col[Cashigh] : cfg.col[Castext];
       if(i == curpos)
         {
         highlighted = current;
         MoveXY(2, top+2+i);
         }
       vprint(top+1+i, 1, colour, "%-25.25s  %-30.30s  %-19.19s",
                                                 current->name,
                                                 current->system,
                                                 FormAddress(&current->address));
       }

     doread = 0;

     key = get_idle_key(1, GLOBALSCOPE);

     switch(key)
       {
       case 337:          // Page down.
         moveback = 0;
         curpos = 0;
         doread = 1;
         break;

       case 329:          // Page up.
         if(NodeListFindMark(nhandle, NULL) != OK)
           {
           NodeListClose(nhandle);
           return NULL;
           }

         moveback = maxy-4;
         curpos = 0;
         doread = 1;
         break;

       case 328:         // Up
         if(curpos == 0)
           {
           if(NodeListFindMark(nhandle, NULL) != OK)
             {
             NodeListClose(nhandle);
             return NULL;
             }

           moveback = 1;
           doread =1;
           }
         else
           curpos--;
         break;

       case 336:           // Down
         if(curpos >= (lastnumber-1))
           {
           curpos = lastnumber-1;
           if(NodeListFindMark(nhandle, NULL) != OK)
              {
              NodeListClose(nhandle);
              return NULL;
              }

           if(NodeListNext(nhandle, NULL) == ERROR)
             {
             NodeListClose(nhandle);
             return NULL;
             }
           doread = 1;
           }
         else
           curpos++;
         break;

       case 9:             // Tab
         show_node_details(highlighted, top+curpos);
         break;

       case 13:
         retnode = GetAddrCopy(highlighted);
         if(first)
            FreeNodesList(first);
         delbox(nodebox);
         NodeListClose(nhandle);
         if(clockstate == 1) clockon();
         return retnode;

       case 27:
         if(first)
           FreeNodesList(first);
         delbox(nodebox);
         NodeListClose(nhandle);
         if(clockstate == 1) clockon();
         return NULL;
       }
     }

   return NULL;
}


// ==============================================================
// Show details about a node in the nodelist
// ==============================================================


void show_node_details(ADDRLIST *current, int curline)
{
   BOX *details;
   int top;
   char temp[80];

   top = curline-1;
   if(top<0) top=0;
   if(top>maxy-6)
      top=maxy-6;

   details = initbox(top, 0, top+4, 79, cfg.col[Cpopframe], cfg.col[Cpoptext], SINGLE, YES, ' ');
   drawbox(details);
   MoveXY(2, top+1);
   sprintf(temp, " %-65.65s %6lu BPS ", current->system, current->baud);
   boxwrite(details, 0, 0, temp);

   sprintf(temp, " %-55.55s %20.20s ", current->location, current->phone);
   boxwrite(details, 1, 0, temp);

   sprintf(temp, " %-30.30s %45.45s ", current->name, current->flags);
   boxwrite(details, 2, 0, temp);

   get_idle_key(0, GLOBALSCOPE);
   delbox(details);

}

// ==============================================================


ADDRLIST *GetLoad(NODEHANDLE *nhandle)
{
  int status, i;
  ADDRLIST *first, *last;
  ADDRLIST found;
  int retry = 0;

  start:

  // Now let's read the first element.
  if((status=NodeListCurr(nhandle, &found)) != OK)
    {
    switch(status)
      {
      case ERROR:
        return NULL;

      case BOI:
        if(NodeListHead(nhandle, &found) != OK)
          return NULL;
        break;

      case EOI:
        if(NodeListTail(nhandle, &found) != OK)
          return NULL;
        break;
      }
    }


  // Mark the start of our 'screenfull'.
  
  if(NodeListMark(nhandle) == ERROR)
    return NULL;

  if( (first = GetAddrCopy(&found)) == NULL)
    return NULL;

  last = first;

  for(i=0; i<(maxy-4); i++)  // Read rest (maxy-4) of elements.
    {
    status = NodeListNext(nhandle, &found);

    if(status == EOI && retry == 0)
      {
      retry = 1;

      if(NodeListTail(nhandle, &found) != OK)
        return NULL;

      for(i=0; i<(maxy-4); i++)
        NodeListPrev(nhandle, &found);

      goto start;
      }

    if(status != OK)
      break;

    last->next = GetAddrCopy(&found);

    if(last->next)
      last = last->next;
    }

  return first;
}


// ==============================================================

ADDRLIST *GetAddrCopy(ADDRLIST *in)
{
   ADDRLIST *out;

   out = mem_malloc(sizeof(ADDRLIST));
   memcpy(out, in, sizeof(ADDRLIST));
   return out;

}

// ==============================================================

void FreeNodesList(ADDRLIST *first)
{
   ADDRLIST *last;

   while(first)
     {
     last = first;
     first = first->next;
     mem_free(last);
     }

}

// ==============================================================

int DoesMatch(char *name, char *foundname)
{
   char lastfirst1[101], lastfirst2[101];

   strcpy(lastfirst1, LastNameFirst(name, 0));
   strcpy(lastfirst2, LastNameFirst(foundname, 0));

   if(strnicmp(lastfirst1, lastfirst2, strlen(name)) == 0)
     return 1;

   return 0;

}

// ==============================================================

