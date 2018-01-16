#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <share.h>

#include "includes.h"

#include "cbtreex.h"

int fidolist;
long reclength = -1;
long nrecs;
long mid;
long fidomark;

int ReadUser(long mid, ADDRLIST *current);


// ==============================================================

int FIDOinit(void)
{
  struct stat buf;
  char midname[100];

  stat(cfg.usr.fidouser, &buf);   /* get the file size */

  if ((fidolist = sopen(cfg.usr.fidouser, O_RDONLY|O_BINARY, SH_DENYNO)) == -1) {
       reclength = -1; /* Reset all on open failure */
       return NULL;
   }

   memset(midname, 0, sizeof(midname));
   read(fidolist, midname, sizeof(midname)-1);     /* Read 1 record */
   if(strchr(midname, '\n') == NULL)
      {
      reclength = -1; /* Reset all on open failure */
      close(fidolist);
      return -1;
      }

   reclength = (long) (strchr(midname, '\n') - midname) + 1; /* FindEnd       */
   nrecs = (long) (buf.st_size / reclength);   /* Now get num of records      */

   return 0;

}

// ==============================================================

int FIDOsearch(char *name, ADDRLIST *found)
{
  long    low, high, cond;
  int     namelen;
  char    midname[100];


  namelen = strlen(name);

  /* Binary search algorithm */
  low = 0;
  high = nrecs - 1;

  while (low <= high)
    {
    mid = low + (high - low) / 2;
    lseek(fidolist, (long) ((long) mid * (long) reclength), SEEK_SET);
    read(fidolist, midname, reclength);
    midname[reclength] = '\0';
    strlwr(midname);

    if ((cond = strncmp(name, midname, namelen)) < 0)
        high = mid - 1;
    else
      {
      if (cond > 0)
         low = mid + 1;
      else
        {               /* Match, match! */
        if(found && ReadUser(mid, found) == -1)
          return ERROR;
        else
          return FOUND;
        }
      }
    }

  return NOTFOUND;

}

// ==============================================================

int FIDOcurr(ADDRLIST *found)
{

   if(ReadUser(mid, found) == -1)
     return ERROR;

   return OK;
}

// ==============================================================

int FIDOnext(ADDRLIST *found)
{

   if(mid == (nrecs-1))
     return EOI;

   mid++;

   if(found)
     {
     if(ReadUser(mid, found) == -1)
       return ERROR;
     else
       return OK;
     }
   else
     return OK;

}

// ==============================================================

int FIDOprev(ADDRLIST *found)
{

   if(mid == 0)
     return BOI;

   mid--;

   if(found)
     {
     if(ReadUser(mid, found) == -1)
       return ERROR;
     else
       return OK;
     }
   else
     return OK;

}

// ==============================================================

int FIDOhead(ADDRLIST *found)
{

   if(nrecs <= 0)
     return ERROR;

   mid = 0;

   if(found)
     {
     if(ReadUser(mid, found) == -1)
       return ERROR;
     else
       return OK;
     }
   else
     return OK;

}

// ==============================================================

int FIDOtail(ADDRLIST *found)
{

   if(nrecs <= 0)
     return ERROR;

   mid = nrecs-1;

   if(found)
     {
     if(ReadUser(mid, found) == -1)
       return ERROR;
     else
       return OK;
     }
   else
     return OK;

}

// ==============================================================

int FIDOmark(void)
{

   fidomark = mid;
   return OK;

}

// ==============================================================

int FIDOfindmark(ADDRLIST *found)
{

   mid = fidomark;

   if(found)
     {
     if(ReadUser(mid, found) == -1)
       return ERROR;
     else
       return OK;
     }
   else
     return OK;

}

// ==============================================================

int FIDOclose(void)
{

   close(fidolist);
   return OK;

}

// ==============================================================

int ReadUser(long mid, ADDRLIST *current)
{
  char midname[100];
  char *c;
  char temp[100];

  memset(current, '\0', sizeof(ADDRLIST));

  lseek(fidolist, (long) ((long) mid * (long) reclength), SEEK_SET);   /* goto first match */

  if(read(fidolist, midname, reclength) == -1)
    return -1;

  midname[reclength] = '\0';

  c = midname + reclength - 1;

  while (isspace(*c)) *c-- = '\0'; c--;
  while (!isspace(*c)) c--;

  strcpy(current->system, "n.a.");
  strcpy(current->location, "n.a.");
  address_expand(c, &current->address, 0);

  /* find name and terminate, copy */

  c--;
  while(isspace(*c)) c--; c++; *c = '\0';
  if( (c = strchr(midname, ',')) != NULL)
     {
     sprintf(current->name, "%s ", (char *)c+2);
     memset(temp, '\0', sizeof(temp));
     memcpy(temp, midname, (size_t) (c - midname));
     strcat(current->name, temp);
     }
  else strcpy(current->name, midname);

  fancy_str(current->name);

  return 0;
}

// ==============================================================

