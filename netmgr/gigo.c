#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <process.h>
#include <stdlib.h>
#include <share.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>

#include <msgapi.h>
#include "nstruct.h"
#include "nodestru.h"
#include "netfuncs.h"

extern CFG cfg;


int static near _validaddress (word czone,word cnet,word cnode);
int static near getnewload(void);


#define READBUF 4096         // In words!
word * bufstart;
int    leftinbuffer = 0;

static word * bufptr       = NULL;


static int nodefile;

// ===============================================================


int static near getnewload(void)
{
   int result;

   result = read(nodefile, bufstart, READBUF * sizeof(word));

   if(result == -1)
     {
     print_and_log("Error reading nodelist.gig!\n"); /* Error */
     return -1;
     }

   bufptr = bufstart;

   return (result>>1);   // This one counts in words, not bytes.

}

// ================================================================


#define eofidx (totalleft <= 0)

int static near _validaddress(word czone,word cnet,word cnode)
{
  word zone,
       net,
       node;

  word nextidx;
  long totalleft    = filelength(nodefile)/2L;
  word leftinbuffer = 0;

  nextidx = zone = net = node = 0;

  lseek(nodefile, 0L, SEEK_SET);

  while (!eofidx)
     {
     if(!leftinbuffer) leftinbuffer = getnewload();
     node = *bufptr++;
     leftinbuffer--;
     totalleft--;

     if (node==65535)  // Found a Zone entry..
       {
 newzone:
       if(!leftinbuffer) leftinbuffer = getnewload();
       leftinbuffer--;
       totalleft--;
       nextidx = *bufptr++;
       zone=net=nextidx;
       node=0;

        if (zone != czone)
          {
          while(!eofidx)  // Find next zone..
            {
            if(!leftinbuffer) leftinbuffer = getnewload();
            leftinbuffer--;
            totalleft--;
            nextidx = *bufptr++;
            if (nextidx==65535) goto newzone;
            }
          }
        }

     if (node==65534)                  // Found Net entry..
       {
       if(!leftinbuffer) leftinbuffer = getnewload();
       net = *bufptr++;
       leftinbuffer--;
       totalleft--;
       node=0;
       }

     if ((cnet==net) && (cnode==node) && (czone==zone)) // A match?
        return 1;                                       // Yep, return success.
     }

  return 0;  // No luck, report nothing found..
}

// ================================================

int validaddress(word czone,word cnet,word cnode)
{
 int i;
 char temp[120];

 sprintf(temp, "%s\\nodelist.gig", cfg.gigonodelist);
 nodefile=sopen(temp, O_RDONLY | O_BINARY, SH_DENYWR);

 if (!nodefile)
  {
  print_and_log("Can't open NODELIST.GIG!\n");
  return 0;
  }

 bufstart = mem_calloc(1, READBUF * sizeof(word));

 i=_validaddress(czone,cnet,cnode);

 close(nodefile);
 mem_free(bufstart);

 return i;
}

