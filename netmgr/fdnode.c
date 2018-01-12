#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include <msgapi.h>
#include "nstruct.h"
#include "nodestru.h"
#include "netfuncs.h"

extern CFG cfg;

int node_index_init(void);
int get_node_node(unsigned blockno, FDNNODE *node);
int search_address(unsigned startblock);

sword         swaphilo    (sword in);
int           addr_comp   (FDNENTRY *this);
int           two_addr_comp(FDNENTRY *this, FDNENTRY *that);
static void   reset       (void);

static unsigned nodeblocklen = 0;
static unsigned nodestart = 0;

int    rawnode  = -1;
int    rawpoint = -1;
int    rawfdnet = -1;

int            fdidx;
static NETADDR targetaddress;


int getFDnode (NETADDR *address)
{
   int result = 0;

   reset();

   if(cfg.fdnodelist[0] == '\0')
      return 0;

   if(node_index_init() == -1)
      return -1;

   targetaddress.zone  = address->zone;
   targetaddress.net   = address->net;
   targetaddress.node  = address->node;
   targetaddress.point = address->point;

   result = search_address(nodestart);

   close(fdidx);
   if(rawnode  != -1) close(rawnode );
   if(rawpoint != -1) close(rawpoint);
   if(rawfdnet != -1) close(rawfdnet);

   return result;

}


// ============================================================


int search_address(unsigned startblock)
{
    int j, k;
    FDNNODE *thisnode;
    unsigned nextblock=startblock;
    int count;

/*
 * Read the first Index node.
 */

    thisnode = mem_calloc(1, sizeof(FDNNODE));

    if (get_node_node(startblock, thisnode) == -1)
       {
       mem_free(thisnode);
       return -1;
       }



    while (thisnode->nextlevel != 0)   /* Until we get to a leaf node */
       {
       if ((count = thisnode->count) == 0) return 0;  /* No nodes ?! */

       for (j = 0; j < count; j++) /* check 32 or maybe less, if count was not 16 */
          {
          k = addr_comp(&thisnode->nodedata[j]);

          if (k > 0)     /* Key in node larger than what we're looking for, go into subtree of entry before this one */
             break;

          if (k == 0)   /* match! */
             {
             mem_free(thisnode);
             return 1;
             }
          }

       if (j == 0)
          nextblock = thisnode->nextlevel;
       else
          nextblock = thisnode->nodedata[j-1].ptr;

       if (get_node_node(nextblock, thisnode) == -1)
          {
          mem_free(thisnode);
          return -1;
          }

       }


    /* If we are here we are searching a leafnode */


    if ((count = thisnode->count) != 0)
       {
       for (j = 0; j < count; j++) /* check 32 or less */
          {
          k = addr_comp(&thisnode->nodedata[j]);

          if (k > 0)
             break;
          if (k == 0)
             {
             return 1;
             }      /* if */

          } /* for */
       }

    mem_free(thisnode);

    return 0;

}


int get_node_node(unsigned blockno, FDNNODE *node)
{
    if(blockno == 0)
      {
      print_and_log("Block number 0 requested!\n");
      return -1;
      }

    if( (lseek (fdidx, (long) (blockno * 741L), SEEK_SET)) == -1)
       {
       print_and_log("Error seeking node index!\n");
       return -1;
       }

    if ( (read (fdidx, node, sizeof (FDNNODE))) != sizeof(FDNNODE))
       {
        print_and_log("Error reading nodelist!\n");
        return -1;
        }

    return 0;
}


/* ---------------------------------------------- */
/* Open index and read controlblock, if necessary */
/* ---------------------------------------------- */

int node_index_init(void)
{
   char  nodeindex[100];
   FDCTL *ctl;

   sprintf(nodeindex, "%s\\nodelist.fdx", cfg.fdnodelist);

   if( (fdidx=sopen(nodeindex, O_RDONLY | O_BINARY, SH_DENYNO)) == -1)
       {
       print_and_log("Can't open %s!\n", nodeindex);
       return -1;
       }

   if(nodeblocklen != 0) return 0;  /* Data already available */

   /* Read first few bytes of index, read blocklen & startblock */

   ctl = mem_calloc(1, sizeof(FDCTL));

   if(read(fdidx, ctl, sizeof(FDCTL)) != sizeof(FDCTL))
      {
      print_and_log("Error reading FD nodelist index!\n");
      close(fdidx);
      mem_free(ctl);
      return -1;
      }

   if( (nodeblocklen = ctl->blocklen) != 741 )
      {
      print_and_log("Unexpected blocklen in FD node index!\n");
      close(fdidx);
      mem_free(ctl);
      return -1;
      }

   nodestart = ctl->startblock;

   mem_free(ctl);

   return 0;

}



sword swaphilo(sword in)
{
   word out;

   out = (word) in >> 8;

   return (out | (word ) (in << 8));

}


// ==========================================================

int addr_comp(FDNENTRY *this)
{

   if(swaphilo(this->zone) !=  targetaddress.zone)
     return (int) (swaphilo(this->zone) - targetaddress.zone);

   if(swaphilo(this->net) !=  targetaddress.net)
     return (int) (swaphilo(this->net) - targetaddress.net);

   if(swaphilo(this->node) !=  targetaddress.node)
     return (int) (swaphilo(this->node) - targetaddress.node);

   return (int) (swaphilo(this->point) - targetaddress.point);

}

// ===============================================================

int two_addr_comp(FDNENTRY *this, FDNENTRY *that)
{

   if(this->zone !=  that->zone)
     return (int) (swaphilo(this->zone) - swaphilo(that->zone));

   if(this->net  !=  that->net)
     return (int) (swaphilo(this->net) - swaphilo(that->net));

   if(this->node !=  that->node)
     return (int) (swaphilo(this->node) - swaphilo(that->node));

   return (int) (swaphilo(this->point) - swaphilo(that->point));

}


// ================================================================

static void reset(void)
{

   rawnode  = -1;
   rawpoint = -1;
   rawfdnet = -1;

   fdidx = 0;
   memset(&targetaddress, '\0', sizeof(NETADDR));

}
