#include <string.h>
#include <stdio.h>
#include <video.h>
#include <scrnutil.h>
#include <msgapi.h>
#include <alloc.h>

#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "idlekey.h"
#include "showhdr.h"
#include "xmalloc.h"
#include "pickone.h"

int choose_name(void);
int choose_address(void);

int choose_name(void)
{
   int tot=0, l=0, ret;
   char **picklist;

   while( (cfg.usr.name[tot].name[0] != '\0') && (tot<10) )
      tot++;

   picklist = xcalloc(tot+1, sizeof(char *));
   for(l=0; l<tot; l++)
     {
     picklist[l] = (char *) xcalloc(1, 35);
     sprintf(picklist[l], " %-32.32s ", cfg.usr.name[l].name);
     }

   ret = pickone(picklist, 5, 21, 23, 56);

   free_picklist(picklist);

   return ret;

}


int choose_address(void)
{
   int tot=0, l=0, ret;
   char **picklist;

   while( (cfg.usr.address[tot].net != 0) && (tot<25) )
      tot++;

   picklist = xcalloc(tot+1, sizeof(char *));
   for(l=0; l<tot; l++)
     {
     picklist[l] = (char *) xcalloc(1, 21);
     sprintf(picklist[l], " %-18.18s ", FormAddress(&cfg.usr.address[l]));
     }

   ret = pickone(picklist, 5, 26, 21, 51);

   free_picklist(picklist);

   return ret;

}
