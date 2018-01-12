#include "includes.h"
#include <cbtree.h>

#include "v7funcs.h"
#include "fdfuncs.h"
#include "fidofunc.h"

typedef struct
{

   byte typesearch;     // 0 == name, 1 == node
   byte typenodelist;   // 0 == V7,   1 == Frodo, 2 == fidouser

} NODEHANDLE;

#define NAMESEARCH 0
#define NODESEARCH 1

#define NODEV7    0
#define NODEFRODO 1
#define NODEFIDO  2


char *LastNameFirst(char *name, int withcomma);


NODEHANDLE *NodeListOpen(int sysop)
{
  NODEHANDLE *nhandle;

  nhandle = mem_calloc(1, sizeof(NODEHANDLE));

  if(sysop)
    nhandle->typesearch = NAMESEARCH;
  else
    nhandle->typesearch = NODESEARCH;

  if(cfg.usr.nodelist[0] != '\0')        // Version 7 nodelist
    {
    nhandle->typenodelist = NODEV7;
    if(V7init(sysop ? 1 : 0) == -1)
      {
      mem_free(nhandle);
      return NULL;
      }
    }
  else if(cfg.usr.fdnodelist[0] != '\0')  // Frodo nodelist
    {
    nhandle->typenodelist = NODEFRODO;
    if(FDinit(sysop ? 1 : 0) == -1)
      {
      mem_free(nhandle);
      return NULL;
      }
    }
  else if(cfg.usr.fidouser[0] != '\0')
    {
    if(!sysop) return NULL;
    nhandle->typenodelist = NODEFIDO;
    if(FIDOinit() == -1)
      {
      mem_free(nhandle);
      return NULL;
      }
    }
  else
    {
    mem_free(nhandle);
    return NULL;
    }

  return nhandle;
}

// ==============================================================

short int NodeListSearch(NODEHANDLE *nhandle, NETADDR *addr, char *sysop, int mode, ADDRLIST *found)
{
  int len;
  char name[101] = "";

  if(nhandle->typenodelist == NODEV7)
    {
    if(nhandle->typesearch == NAMESEARCH)
      {
      strcpy(name, LastNameFirst(sysop, 1));
      return V7search(name, strlen(name), found);
      }
    else
      {
      len = addr->point ? 8 : 6;
      return V7search((char *)addr, len, found);
      }
    }
  else if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    if(nhandle->typesearch == NAMESEARCH)
      {
      strncpy(name, LastNameFirst(sysop, 0), 14);
      name[14] = '\0';
      FDupper(name);
      return FDsearch_name(name, found);
      }
    else
      {
      return FDsearch_node(addr, found);
      }
    }
  else   // Fidouser.lst
    {
    strcpy(name, LastNameFirst(sysop, 1));
    strlwr(name);
    return FIDOsearch(name, found);
    }

  return ERROR;
}

// ==============================================================

short int NodeListCurr(NODEHANDLE *nhandle, ADDRLIST *found)
{
  if(nhandle->typenodelist == NODEV7)
    {
    return V7curr(found);
    }
  else if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    return FDcurr(found);
    }
  else
    return FIDOcurr(found);

}

// ==============================================================

short int NodeListNext(NODEHANDLE *nhandle, ADDRLIST *found)
{
  short int status;

  if(nhandle->typenodelist == NODEV7)
    {
    return V7next(found);
    }
  else if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    if(nhandle->typesearch == NAMESEARCH)
      {
      if( (status = FDnextname()) != OK)
        return status;

      if(found)
        return FDcurr(found);
      else
        return OK;
      }
    else  // Node search
      {
      if( (status = FDnextnode()) != OK)
        return status;

      if(found)
        return FDcurr(found);
      else
        return OK;
      }
    }
  else
    return FIDOnext(found);

}

// ==============================================================

short int NodeListPrev(NODEHANDLE *nhandle, ADDRLIST *found)
{
  short int status;

  if(nhandle->typenodelist == NODEV7)
    {
    return V7prev(found);
    }
  else if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    if(nhandle->typesearch == NAMESEARCH)
      {
      if( (status = FDprevname()) != OK)
        return status;

      if(found)
        return FDcurr(found);
      else
        return OK;
      }
    else  // Node search
      {
      if( (status = FDprevnode()) != OK)
        return status;

      if(found)
        return FDcurr(found);
      else
        return OK;
      }
    }
  else // fidouser
    return FIDOprev(found);

}

// ==============================================================

short int NodeListHead(NODEHANDLE *nhandle, ADDRLIST *found)
{
  if(nhandle->typenodelist == NODEV7)
    {
    return V7head(found);
    }
  else if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    return FDhead(found);
    }
  else
    {
    return FIDOhead(found);
    }


}

// ==============================================================

short int NodeListTail(NODEHANDLE *nhandle, ADDRLIST *found)
{
  if(nhandle->typenodelist == NODEV7)
    {
    return V7tail(found);
    }
  else if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    return FDtail(found);
    }
  else
    {
    return FIDOtail(found);
    }

}

// ==============================================================

short int NodeListMark(NODEHANDLE *nhandle)
{
  if(nhandle->typenodelist == NODEV7)
    {
    return V7mark();
    }
  else if(nhandle->typenodelist == NODEFRODO)    // Frodo nodelist
    {
    return FDmark();
    }
  else
    return FIDOmark();


}

// ==============================================================

short int NodeListFindMark(NODEHANDLE *nhandle, ADDRLIST *found)
{

  if(nhandle->typenodelist == NODEV7)
    {
    return V7findmark(found);
    }
  else if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    return FDfindmark(found);
    }
  else
    return FIDOfindmark(found);

}

// ==============================================================

short int NodeListClose(NODEHANDLE *nhandle)
{

  if(nhandle->typenodelist == NODEV7)
    {
    V7close();
    mem_free(nhandle);
    return 0;
    }
  else  if(nhandle->typenodelist == NODEFRODO)   // Frodo nodelist
    {
    FDclose();
    mem_free(nhandle);
    return 0;
    }
  else
    {
    FIDOclose();
    mem_free(nhandle);
    return 0;
    }

}

// ==============================================================

char *LastNameFirst(char *name, int withcomma)
{
   static char last_name_first[101];
   char midname[101];
   char *c, *p, *m;

   c = midname;                                  /* Start of temp name buff   */
   p = name;                                     /* Point to start of name    */
   m = NULL;                                     /* Init pointer to space     */

   *c = *p++;
   while (*c)                                    /* Go entire length of name  */
   {
      if (*c == ' ' || *c == '_')                /* Look for space            */
         {
         *c = ' ';                               /* Convert underscore        */
         m = c;                                  /* Save location             */
         }
      c++;
      *c = *p++;
   }

   if (m != NULL)                                /* If we have a pointer,     */
   {
      *m++ = '\0';                               /* Terminate the first half  */
      (void) strcpy (last_name_first, m);        /* Now copy the last name    */
      if(withcomma)
         (void) strcat (last_name_first, ", ");      /* Insert a space  */
      else
         (void) strcat (last_name_first, " ");      /* Insert a space  */
      (void) strcat (last_name_first, midname);  /* Finally copy first half   */
   }
   else (void) strcpy (last_name_first, midname);/* Use whole name otherwise  */

   return last_name_first;

}

// ===============================================================

