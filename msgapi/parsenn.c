#define NOVARS
#define NOVER
#define MAX_NOPROTO

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"

static char *colon=":";
static char *slash="/";

void _fast Parse_NetNode(char *netnode,word *zone,word *net,word *node,word *point)
{
  ParseNN(netnode,zone,net,node,point,FALSE);
}

void _fast ParseNN(char *netnode,word *zone,word *net,word *node,word *point,word all)
{
  char *p;

  p=netnode;
  
  if (all && point)
    *point=POINT_ALL;

  if (all && toupper(*netnode)=='W')  /* World */
  {
    if (zone)
      *zone=ZONE_ALL;

    if (net)
      *net=NET_ALL;

    if (node)
      *node=NODE_ALL;

    return;
  }

  /* If we have a zone (and the caller wants the zone to be passed back).. */

  if (strchr(netnode,':'))
  {
    if (zone)
    {
      if (all && toupper(*p)=='A')  /* All */
        *zone=ZONE_ALL;
      else *zone=atoi(p);
    }

    p=firstchar(p,colon,2);
  }

  /* If we have a net number... */

  if (p && *p)
  {
    if (strchr(netnode,'/'))
    {
      if (net)
      {
        if (all && toupper(*p)=='A')  /* All */
          *net=NET_ALL;
        else *net=atoi(p);
      }

      p=firstchar(p,slash,2);
    }
    else if (all && toupper(*p)=='A')
    {
      /* If it's in the form "1:All" or "All" */

      if (strchr(netnode,':')==NULL && zone)
        *zone=ZONE_ALL;

      *net=NET_ALL;
      *node=NODE_ALL;
      p += 3;
    }
  }

  /* If we got a node number... */

  if (p && *p && node && *netnode != '.')
  {
    if (all && toupper(*p)=='A')  /* All */
    {
      *node=NODE_ALL;

      /* 1:249/All implies 1:249/All.All too... */

      if (point && all)
        *point=POINT_ALL;
    }
    else *node=atoi(p);
  }

  if (p)
    while (*p && isdigit(*p))
      p++;

  /* And finally check for a point number... */

  if (p && *p=='.')
  {
    p++;

    if (point)
    {
      if (!p && *netnode=='.')
        p=netnode+1;

      if (p && *p)
      {
        *point=atoi(p);

        if (all && toupper(*p)=='A')  /* All */
          *point=POINT_ALL;
      }
      else *point=0;
    }
  }
}

