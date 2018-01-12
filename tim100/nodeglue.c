#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <ctype.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "nodeglue.h"
#include "v7.h"
#include "fdnode.h"
#include "fidouser.h"
#include "reply.h"
#include "xmalloc.h"
#include "pickone.h"
#include "header.h"

int check_node(XMSG *hdr, int aka, int prompt)
{
   ADDRLIST *first = NULL, *current;
   NETADDR tempaddr;
   int found, i, chosen;
   char **picklist, temp[81];

   if(hdr->to[0] == '\0') return 0;

   /* Check for an address entered (instead of name) */

   if ( (!isdigit(hdr->to[0])) && (hdr->to[0] != '.') )
      {
      /* name given, look it up */

      if(cfg.usr.fidouser[0] != '\0')  /* Fidouser.lst ? */
         first = fido_lookup(hdr->to);

      if( (first==NULL) && (cfg.usr.fdnodelist[0] != '\0') )   /* FD ? */
         first = getFDname(hdr->to);

      if( (first==NULL) && (cfg.usr.nodelist[0] != '\0') ) /* Version 7 ? */
         first = opususer(hdr->to);
      }
   else     /* Address given, look up address */
      {
      address_expand(hdr->to, &tempaddr, aka);

      if(cfg.usr.nodelist[0] != '\0')       /* Version 7 present? */
         first = ver7find(&tempaddr);

      if( (first==NULL) && (cfg.usr.fdnodelist[0] != '\0') )  /* FD nodelist? */
         first = getFDnode(&tempaddr);

      /* No address lookup in fidouser.lst possible */
      }

   if(first == NULL) /* No match found */
      return 0;

   /* Count elements */

   for(current = first, found=0; current; current = current->next)
      found++;

   /* Make a list for the 'pickone' function */

   picklist = xcalloc(found+1, sizeof(char *));

   for(i=0, current=first; current; current = current->next, i++)
      {
      picklist[i] = xcalloc(1, 79);
      sprintf(temp, "%-25.25s  %-30.30s  %d:%d/%d.%d",
                                                   current->name,
                                                   current->system,
                                                   current->address.zone,
                                                   current->address.net,
                                                   current->address.node,
                                                   current->address.point);
      sprintf(picklist[i], " %-76.76s ", temp);
      }


   if( (found == 1) && (prompt == NOPROMPT) )   /* Take first address */
       chosen = 0;
   else                                         /* Show list          */
      {
      statusbar("Use arrow keys to move, <ENTER> to select..");
      chosen = pickone(picklist, 5, 0, 23, 79);
      }

   /* Free mem & fill in chosen address */

   for(i=0, current=first; current; current = current->next, i++)
      {
      if(i == chosen)
         {
         hdr->dest.zone  = current->address.zone;    /* Fill in address */
         hdr->dest.net   = current->address.net;
         hdr->dest.node  = current->address.node;
         hdr->dest.point = current->address.point;
         strncpy(hdr->to, current->name, 35);
         hdr->to[35] = '\0';
         break;
         }
      }

   free_picklist(picklist);

   while(first)
      {
      current = first;
      first = first->next;
      free(current);
      }

   /* Return 0 if no address selected */

   return (chosen == -1) ? 0 : 1;

}

