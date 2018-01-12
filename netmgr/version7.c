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


/* This table has been modified to minimize searches */
char unwrk[] = " EANROSTILCHBDMUGPKYWFVJXZQ-'0123456789";

int  btree         (int (*)(void *, int), long startblock);
int  addr_compare  (void *, int);
int  get7node      (long blockno, struct _ndx  *ndx);
static void reset  (void);


static long nodestart = 0;

int  curindex = 0;       /* File handle for index that is searched */


static NETADDR targetaddress;

extern CFG cfg;


int ver7find (NETADDR *opus_addr)
{
    char filename[100];
    struct _ndx *ctl;
    int result = 0;

    reset();

    if(cfg.nodelist[0] == '\0')
       return 0;

    targetaddress.zone  = opus_addr->zone;
    targetaddress.net   = opus_addr->net;
    targetaddress.node  = opus_addr->node;
    targetaddress.point = opus_addr->point;


    sprintf(filename, "%s\\nodex.ndx", cfg.nodelist);

    if ((curindex = sopen(filename, O_RDONLY|O_BINARY, SH_DENYNO)) == -1)
        {
        print_and_log("Can't open %s!", filename);
        return -1;                          /* no file, no work to do */
        }

    if(nodestart == 0)     /* Is root already known? Read ctl-block if not */
       {
       ctl = mem_calloc(1, sizeof(struct _ndx));
       if ( (get7node (0, ctl)) ==  -1 )
          {
          close(curindex);
          mem_free(ctl);
          return -1;
          }
        nodestart = ctl->ndx.CtlBlk.CtlRoot;
        mem_free(ctl);
        }

    result = btree (addr_compare, nodestart);

    close(curindex);

    return result;

}

// =====================================================================

int addr_compare (void *key, int len)
{
   int k;

   k = ((NETADDR *)key)->zone - targetaddress.zone;
   if (k)
      return (k);

   k = ((NETADDR *)key)->net - targetaddress.net;
   if (k)
      return (k);

   k = ((NETADDR *)key)->node - targetaddress.node;
   if (k)
      return (k);
/*
 * Node matches.
 *
 * The rule for points:
 *  1) If len == 6, treat key value for Point as Zero.
 *  2) Return comparison of key Point and desired Point.
 */
   if (len == 6)
      ((NETADDR *)key)->point = 0;

   return ((NETADDR *)key)->point - targetaddress.point;
}


/*
 * General V7 nodelist engine. Used by both the by-node and by-sysop
 * lookups.
 *
 * Thanks to Phil Becker for showing me how nice it looks in assembler.
 * It helped me see how nice it could be in C.
 *
 * (I know, Phil, it's still nicer in assembler!)
 *
 */



int btree (int (*compare)(void *key, int len), long startblock)
{
    int j, k, l;
    struct _ndx *thisnode;
    struct _IndxRef  *ip = NULL;
    struct _LeafRef  *lp = NULL;
    char aline[160];
    char  *tp;
    char *np;
    long nextblock;
    int count;

    thisnode = mem_calloc(1, sizeof(struct _ndx));

    if (get7node(startblock, thisnode) == -1)
        {
        mem_free(thisnode);
        return -1;
        }

/*
 * Follow the node tree until we either match a key right in the index
 * node, or locate the leaf node which must contain the entry.
 */

    while (thisnode->ndx.INodeBlk.IndxFirst != -1)
       {
       if ((count = thisnode->ndx.INodeBlk.IndxCnt) == 0)
          {
          mem_free(thisnode);
          return 0;
          }


       for (j = 0; j < count; j++) /* check 20 or less */
          {
          ip = &(thisnode->ndx.INodeBlk.IndxRef[j]);
          tp = (char  *) thisnode + ip->IndxOfs;

          k = l = ip->IndxLen;

          for (np = aline; k > 0; k--)
              *np++ = *tp++;


          k = (*compare) ((void *)aline, l);

          if (k > 0)
             break;

          if (k == 0)   /* Match right in the index */
             {
             /* Return SUCCESS! */
             mem_free(thisnode);
             return 1;
             }
          }

       if (j == 0)
          nextblock = thisnode->ndx.INodeBlk.IndxFirst;
       else
          nextblock = (thisnode->ndx.INodeBlk.IndxRef[--j]).IndxPtr;

       if (get7node(nextblock, thisnode) == -1)
          {
          mem_free(thisnode);
          return -1;
          }

       }

/*
 * We can only get here if we've found the leafnode which must
 * contain our entry.
 *
 * Find our guy here or die trying.
 */

    if ((count = thisnode->ndx.LNodeBlk.IndxCnt) != 0)
       {
       /* Search for a higher key */


       for (j = 0; j < count; j++) /* check 30 or less */
          {
          lp = &(thisnode->ndx.LNodeBlk.LeafRef[j]);
          tp = (char  *) thisnode + lp->KeyOfs;

         /* Leaf node search!!!! */

          k = l = lp->KeyLen;

          for (np = aline; k > 0; k--)

              *np++ = *tp++;

         k = (*compare) ((void *)aline, l);

          if (k > 0)
             break;
          if (k == 0)
             {
             /* Add this match */
             mem_free(thisnode);
             return 1;   // Return success.
             }
          }
       }

    mem_free(thisnode);
    return 0;   // Too bad, not found.
}


// ==================================================================


int get7node(long blockno, struct _ndx  *ndx)
{

   if(blockno < 0)
     {
     print_and_log("Index block number < 0 requested!");
     return -1;
     }

    if(lseek (curindex, (long) blockno * 512L, SEEK_SET) != (long) blockno * 512L)
       {
       print_and_log("Error seeking nodelist!\n");
       return -1;
       }

    if ( (read (curindex, ndx, sizeof (struct _ndx))) != sizeof(struct _ndx))
       {
       print_and_log("Error reading nodelist!\n");
       return -1;
       }

    return 0;
}


// =========================================================

static void reset(void)
{

    curindex = 0;       /* File handle for index that is searched */
    memset(&targetaddress, '\0', sizeof(NETADDR));

}

