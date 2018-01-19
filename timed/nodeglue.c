#include "includes.h"


int NodeSelect(ADDRLIST *first);
void show_node_details(ADDRLIST *thisone, int curline);

// Return :  0 - not found
//           1 - found
//          -1 - abort

int check_node(MIS *mis, int aka, int prompt)
{
   ADDRLIST *first = NULL;
   NETADDR tempaddr;

   if(mis->to[0] == '\0') return 0;

   /* Check for an address entered (instead of name) */

   if ( (!isdigit(mis->to[0])) && (mis->to[0] != '.') )
      {
      /* name given, look it up */
      first = NodeLookup(mis->to, 1, prompt);
      }
   else     /* Address given, look up address */
      {
      address_expand(mis->to, &tempaddr, aka);
      first = NodeLookup((char *)&tempaddr, 0, prompt);
      }

   if(first == NULL) /* No match found */
      return 0;

   mis->destfido = first->address;
   strncpy(mis->to, first->name, 99);
   mis->to[99] = '\0';

   if(first)
      mem_free(first);

   /* Return 0 if no address selected */

   return 1;

}

