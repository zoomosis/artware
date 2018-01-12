#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <ctype.h>
#include <sys\stat.h>

#include <msgapi.h>
#include <progprot.h>
#include "nstruct.h"
#include "txtbuild.h"
#include "netfuncs.h"
#include "xfile.h"

extern CFG         cfg;
extern MESSAGE     msg;
extern COMMANDINFO CommandInfo;
extern XMASKLIST   *firstxmask;


int MaskMatch(NETADDR *adr, NETADDR *mask);


/* Check TO: address and all AKA's to pick the most appropriate one.. */

int matchaka(MIS *mis)
{
   int hits, i, firsthit = -1, found_zone_net=0;
   static int akano = -1;
   int currentmatcheszone = 0; /* The current address matches */
   AKAFORCE *thisone;

   if(mis->origfido.zone  != (word) -88 ||
      mis->origfido.net   != (word) -88 ||
      mis->origfido.node  != (word) -88 ||
      mis->origfido.point != (word) -88 )
      return 0;

   // Start with the first address

   mis->origfido = cfg.homeaddress[0];

   if( (thisone=cfg.akaforce) != NULL )
     {
     while(thisone)
       {
       if(MaskMatch(&mis->destfido, &thisone->mask) != 0)  // Match?
         {
         if(addrcmp(&cfg.homeaddress[thisone->aka], &mis->origfido)==0)
            return 0; // Found match, but no change needed..

         mis->origfido = cfg.homeaddress[thisone->aka];
         return 1;   // Change was needed & forced
         }
       thisone = thisone->next;
       }
     }

//   if((cfg.usr.status & AKAMATCHING) == 0) return 0; /* AKA matching is off.. */

   if(akano == -1)     /* No of AKA's not counted yet */
      {
      for(akano=0, i=0; i< tMAXAKAS; i++)  /* Count no of AKA's */
         {
         if(cfg.homeaddress[i].zone != 0)
            akano++;
         else
            break;
         }
      }

   if(akano == 1) return 0;   /* Only 1 AKA, go home! */

   for(hits=0, i=0; i<akano; i++)  /* Check for zone-match for all AKA's */
      {
      if(cfg.homeaddress[i].zone == mis->destfido.zone)
         {
         if(addrcmp(&cfg.homeaddress[i], &mis->origfido)==0)
           currentmatcheszone = 1;

         if(firsthit == -1)
             firsthit = i;  /* Remember first zone hit for later */
         hits++;
         }
      }

   if(hits == 0)
      return 0;      /* No zone hits found, use current address */

   /* If more than one 'zone match' was found, check nets too.. */

   if(hits > 1)
       {
       for(hits=0, i=0; i<akano; i++)  /* Check for zone AND net */
         {
         if(
             (cfg.homeaddress[i].zone == mis->destfido.zone) &&
             (cfg.homeaddress[i].net  == mis->destfido.net )
           )
             {
             if(addrcmp(&cfg.homeaddress[i], &mis->origfido)==0)
               return 0;

             if(found_zone_net == 0)
                {
                firsthit=i; /* Found a match, use it, even if there would be more */
                found_zone_net = 1;
                }
             }  /* if */
         } /* for */

       }

   if(currentmatcheszone && !found_zone_net) /* The current address is one of the matches, no change */
     return 0;

   if(addrcmp(&cfg.homeaddress[firsthit], &mis->origfido)==0)
     return 0;    /* Current address, so no AKA change */

   /* Fill in new adress */

   mis->origfido = cfg.homeaddress[firsthit];

   /* Return 1 to signal change */

   return 1;
}


/* ----------------------------------------------------------- */

int addrcmp(NETADDR *one, NETADDR *two)
{
  if(one->zone != two->zone)
    return (one->zone - two->zone);

  if(one->net != two->net)
    return (one->net - two->net);

  if(one->node != two->node)
    return (one->node - two->node);

  if(one->point != two->point)
    return (one->point - two->point);

  return 0;
}


// --------------------------------------------------------------

// -- Check if an address matches an address mask (akamatching force)

int MaskMatch(NETADDR *adr, NETADDR *mask)
{
   if(
       (mask->zone != (word) -99) &&
       (adr->zone != mask->zone)
     )
     return 0;

   if(
       (mask->net != (word) -99) &&
       (adr->net != mask->net)
     )
     return 0;

   if(
       (mask->node != (word) -99) &&
       (adr->node != mask->node)
     )
     return 0;

   if(
       (mask->point != (word) -99) &&
       (adr->point != mask->point)
     )
     return 0;

   return 1;
}

// ========================================================

