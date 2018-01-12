#include "includes.h"

int choose_name(void);
int choose_address(void);

int choose_name(void)
{
   int tot=0, l=0, ret;
   char **picklist;

   while( (cfg.usr.name[tot].name[0] != '\0') && (tot<tMAXNAMES) )
      tot++;

   picklist = mem_calloc(tot+1, sizeof(char *));
   for(l=0; l<tot; l++)
     {
     picklist[l] = (char *) mem_calloc(1, 35);
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

   while( (cfg.usr.address[tot].net != 0) && (tot<tMAXAKAS) )
      tot++;

   picklist = mem_calloc(tot+1, sizeof(char *));
   for(l=0; l<tot; l++)
     {
     picklist[l] = (char *) mem_calloc(1, 21);
     sprintf(picklist[l], " %-18.18s ", FormAddress(&cfg.usr.address[l]));
     }

   ret = pickone(picklist, 5, 26, 21, 51);

   free_picklist(picklist);

   return ret;

}
