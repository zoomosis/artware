#include "includes.h"

int maint_menu(AREA *area)
{
   char **optlist;
   sword *macrolist;
   int i, add, action;
   int itemno = 0, longest = 0;
   char temp[MAX_SCREEN_WIDTH], format[20];

   optlist   = mem_calloc(MAXMENUENTRIES + 1, sizeof(char *));
   macrolist = mem_calloc(MAXMENUENTRIES + 1, sizeof(sword));

   for(i=0; i < NumMenuEntries; i++)
     {
     add = 0;
     switch(ReaderMenu[i].where)
       {
       case MENUALL:
         add = 1;
         break;

       case MENUJAM:
         if(area->base & MSGTYPE_JAM)
           add = 1;
         break;

       case MENUSDM:
         if(area->base & MSGTYPE_SDM)
           add = 1;
          break;

       case MENUSQUISH:
         if(area->base & MSGTYPE_SQUISH)
           add = 1;
          break;

       case MENUHMB:
         if(area->base & MSGTYPE_HMB)
           add = 1;
          break;
        }

     if(add)
       {
       optlist[itemno] = ReaderMenu[i].desc;
       if(strlen(ReaderMenu[i].desc) > longest)
          longest = strlen(ReaderMenu[i].desc);
       macrolist[itemno] = ReaderMenu[i].macro;
       itemno++;
       }
     }

   if(longest > (maxx-3)) longest = maxx-3;

   // Make strings same length, ad space in front and at the end

   sprintf(format, " %%-%d.%ds ", longest, longest);
   for(i=0; i<itemno; i++)
     {
     sprintf(temp, format, optlist[i]);
     optlist[i] = mem_strdup(temp);
     }

   if(itemno > 0)
     action = picklist(optlist, NULL, 5, maxx/2 - (longest/2) - 1, 23, maxx-1);
   else
     action = -1;

   if(action != -1)
     MacroStart(macrolist[action]);

   free_picklist(optlist);
   mem_free(macrolist);
   return 0;

}

