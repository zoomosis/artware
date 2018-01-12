#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <conio.h>
#include <ctype.h>
#include <share.h>
#include <video.h>
#include <scrnutil.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "message.h"
#include "fdnode.h"
#include "xmalloc.h"
#include "global.h"


int name_index_init(void); /* 0 on success, -1 on error */
int node_index_init(void);

int get_node_node(unsigned blockno, FDNNODE *node);
int get_name_node(unsigned blockno, FDUNODE *node);

void search_name   (unsigned startblock);
void search_address(unsigned startblock);

static sword  addmatch    (sword zone, sword net, sword node, sword point, long offset);
sword         swaphilo    (sword in);
int           addr_comp   (FDNENTRY *this);
void          strip_under (char *start);
static void   reset       (void);

static unsigned nodeblocklen = 0;
static unsigned nameblocklen = 0;
static unsigned nodestart = 0;
static unsigned namestart = 0;
static char nodelistname[100] = "";

int    rawnode  = -1;
int    rawpoint = -1;
int    rawfdnet = -1;

int            fdidx;
static char    matchname[16];
static int     namelen;
static NETADDR matchaddress;

static ADDRLIST *first = NULL;


/* debug */

/*  CFG cfg; */

/* debug */


/* void main()

{
   NETADDR addr;
   char naam[40], temp[100];
   ADDRLIST *current, *last;

   memset (&cfg, '\0', sizeof(CFG));
   strcpy(cfg.usr.fdnodelist, "c:/frodo/nodelist");

   while(1)
     {
     clrscr();
     gotoxy(1,1);
     gets(naam);
     getFDname(naam);

     for(current=first; current; current = current->next)
       {
       printf("%-30.30s  %-20.20s  %d:%d/%d.%d\n", current->name,
                                                   current->system,
                                                   current->address.zone,
                                                   current->address.net,
                                                   current->address.node,
                                                   current->address.point);
       }

     current=first;
     while(current)
       {
       last=current;
       current=current->next;
       free(last);
       }

     printf("\n---\n");
     first=NULL;
     rawnode =  NULL;
     rawpoint = NULL;
     rawfdnet = NULL;

     getch();
     }

/*
   while(1)
     {
     clrscr();
     printf("Zone: "); gets(naam);
     addr.zone = atoi(naam);
     printf("Net : "); gets(naam);
     addr.net = atoi(naam);
     printf("Node: "); gets(naam);
     addr.node = atoi(naam);
     printf("Point: "); gets(naam);
     addr.point = atoi(naam);

     getFDnode(&addr);

     for(current=first; current; current = current->next)
       {
       printf("%-30.30s  %-20.20s  %d:%d/%d.%d\n", current->name,
                                                   current->system,
                                                   current->address.zone,
                                                   current->address.net,
                                                   current->address.node,
                                                   current->address.point);
       }

     current=first;
     while(current)
       {
       last=current;
       current=current->next;
       free(last);
       }

     printf("\n---\n");
     first=NULL;
     rawnode =  NULL;
     rawpoint = NULL;
     rawfdnet = NULL;

     getch();

     }

*/

} */



ADDRLIST *getFDnode (NETADDR *address)
{

    reset();

    if(cfg.usr.fdnodelist[0] == '\0')
       return NULL;

    if(node_index_init() == -1)
       return NULL;

    matchaddress.zone  = address->zone;
    matchaddress.net   = address->net;
    matchaddress.node  = address->node;
    matchaddress.point = address->point;

    search_address(nodestart);

    close(fdidx);
    if(rawnode  != -1) close(rawnode );
    if(rawpoint != -1) close(rawpoint);
    if(rawfdnet != -1) close(rawfdnet);

    return first;
}



ADDRLIST *getFDname (char *name)
{
   char last_name_first[80];
   char midname[80];
   char *c, *p, *m;

   reset();

   if(cfg.usr.fdnodelist[0] == '\0')
       return NULL;

   if(name_index_init() == -1)
       return NULL;


   c = midname;                                  /* Start of temp name buff   */
   p = name;                                     /* Point to start of name    */
   m = NULL;                                     /* Init pointer to space     */

   *c = *p++;
   while (*c)                                    /* Go entire length of name  */
   {
      if (*c == ' ')                             /* Look for space            */
         m = c;                                  /* Save location             */
      c++;
      *c = *p++;
   }

   if (m != NULL)                                /* If we have a pointer,     */
   {
      *m++ = '\0';                               /* Terminate the first half  */
      (void) strcpy (last_name_first, m);        /* Now copy the last name    */
      (void) strcat (last_name_first, " ");      /* Insert a space  */
      (void) strcat (last_name_first, midname);  /* Finally copy first half   */
   }
   else (void) strcpy (last_name_first, midname);/* Use whole name otherwise  */

   memset(matchname, '\0', sizeof(matchname));
   strncpy(matchname, last_name_first, 15);
   strupr(matchname);

   namelen = (int) strlen (matchname);     /* Calc length now           */

   search_name(namestart);

   close(fdidx);
   if(rawnode  != -1) close(rawnode );
   if(rawpoint != -1) close(rawpoint);
   if(rawfdnet != -1) close(rawfdnet);

   return first;
}



void search_name(unsigned startblock)
{
    int j, k;
    FDUNODE *thisnode;
    unsigned nextblock=startblock;
    int count;

/*
 * Read the first Index node.
 */

     thisnode = xcalloc(1, sizeof(FDUNODE));

    if (get_name_node(startblock, thisnode) == -1)
       {
       free(thisnode);
       return;
       }


    while (thisnode->nextlevel != 0)   /* Until we get to a leaf node */
       {
       if ((count = thisnode->count) == 0) return;  /* No nodes ?! */

       if(count == 16) count = 32;   /* Dunno why, full node == 32, but says 16 [BUT NOT ALWAYS!] */

       for (j = 0; j < count; j++) /* check 32 or maybe less, if count was not 16 */
          {
          if( (j > 15) && (strcmp(thisnode->nodedata[j-1].name,
                                  thisnode->nodedata[j].name) > 0) )
             break;  // Weird, count was prob 16, but really 16 entries, not 32!

          k = strncmp(thisnode->nodedata[j].name, matchname, namelen);

          if (k > 0)     /* Key in node larger than what we're looking for, go into subtree of entry before this one */
             break;

          if (k == 0)   /* match! */
             {
             /* Recurse into previous entry's pointer to find earlier matches */

             if(j == 0)      /* Was this the first entry? */
                {
                if(thisnode->nextlevel != 0)         /* Is there a 'next level'? */
                   search_name(thisnode->nextlevel);
                }
             else
                {
                search_name(thisnode->nodedata[j-1].ptr);
                }

             /* Add this index node to list */

             if(addmatch(swaphilo(thisnode->nodedata[j].zone),
                      swaphilo(thisnode->nodedata[j].net),
                      swaphilo(thisnode->nodedata[j].node),
                      swaphilo(thisnode->nodedata[j].point),
                      thisnode->nodedata[j].offset) == -1)
                {      /* Error or too many matches, stop here */
                free(thisnode);
                return;
                }

             }
          }

       if (j == 0)
          nextblock = thisnode->nextlevel;
       else
          nextblock = thisnode->nodedata[j-1].ptr;

       if (get_name_node(nextblock, thisnode) == -1)
          {
          free(thisnode);
          return;
          }

       }


    /* If we are here we are searching a leafnode */


    if ((count = thisnode->count) != 0)
       {
       if(count == 16) count = 32;  /* Dunno why, full == 32 nodes! */

       for (j = 0; j < count; j++) /* check 32 or less */
          {
          if( (j > 15) && (strcmp(thisnode->nodedata[j-1].name,
                                   thisnode->nodedata[j].name) > 0) )
             break;  // Weird, count was prob 16, but really 16 entries, not 32!


          k = strncmp(thisnode->nodedata[j].name, matchname, namelen);

          if (k > 0)
             break;
          if (k == 0)
             {
             /* Add it to list, but search on.. */

             if (addmatch(swaphilo(thisnode->nodedata[j].zone),
                          swaphilo(thisnode->nodedata[j].net),
                          swaphilo(thisnode->nodedata[j].node),
                          swaphilo(thisnode->nodedata[j].point),
                          thisnode->nodedata[j].offset) == -1)
                 break; /* Error or too much, stop here */

             }  /* if */

          } /* for */
       }

    free(thisnode);

}



void search_address(unsigned startblock)
{
    int j, k;
    FDNNODE *thisnode;
    unsigned nextblock=startblock;
    int count;

/*
 * Read the first Index node.
 */

    thisnode = xcalloc(1, sizeof(FDNNODE));

    if (get_node_node(startblock, thisnode) == -1)
       {
       free(thisnode);
       return;
       }



    while (thisnode->nextlevel != 0)   /* Until we get to a leaf node */
       {
       if ((count = thisnode->count) == 0) return;  /* No nodes ?! */

       if(count == 16) count = 32;   /* Dunno why, full node == 32, but says 16 */

       for (j = 0; j < count; j++) /* check 32 or maybe less, if count was not 16 */
          {

          k = addr_comp(&thisnode->nodedata[j]);

          if (k > 0)     /* Key in node larger than what we're looking for, go into subtree of entry before this one */
             break;

          if (k == 0)   /* match! */
             {
             /* Add this index node to list */

             addmatch(swaphilo(thisnode->nodedata[j].zone),
                      swaphilo(thisnode->nodedata[j].net),
                      swaphilo(thisnode->nodedata[j].node),
                      swaphilo(thisnode->nodedata[j].point),
                      thisnode->nodedata[j].offset);

             /* Only one match needed/expected, bail out */

             free(thisnode);
             return;
             }
          }

       if (j == 0)
          nextblock = thisnode->nextlevel;
       else
          nextblock = thisnode->nodedata[j-1].ptr;

       if (get_node_node(nextblock, thisnode) == -1)
          {
          free(thisnode);
          return;
          }

       }


    /* If we are here we are searching a leafnode */


    if ((count = thisnode->count) != 0)
       {
       if(count == 16) count = 32;  /* Dunno why, full == 32 nodes! */

       for (j = 0; j < count; j++) /* check 32 or less */
          {
          k = addr_comp(&thisnode->nodedata[j]);

          if (k > 0)
             break;
          if (k == 0)
             {
             addmatch(swaphilo(thisnode->nodedata[j].zone),
                      swaphilo(thisnode->nodedata[j].net),
                      swaphilo(thisnode->nodedata[j].node),
                      swaphilo(thisnode->nodedata[j].point),
                      thisnode->nodedata[j].offset);

             break; /* Only one match needed, stop here */
             }      /* if */

          } /* for */
       }

    free(thisnode);

}


int get_node_node(unsigned blockno, FDNNODE *node)
{

    if( (lseek (fdidx, (long) (blockno * 741L), SEEK_SET)) == -1)
       {
       Message("Error seeking node index!", -1, 0, YES);
       return -1;
       }

    if ( (read (fdidx, node, sizeof (FDNNODE))) != sizeof(FDNNODE))
       {
        Message("Error reading nodelist!", -1, 0, YES);
        return -1;
        }

    return 0;
}

/* --------------------------------------------- */

int get_name_node(unsigned blockno, FDUNODE *node)
{

    if( (lseek (fdidx, (long) (blockno * 1061L), SEEK_SET)) == -1)
       {
       Message("Error seeking name index!", -1, 0, YES);
       return -1;
       }

    if ( (read (fdidx, node, sizeof (FDUNODE))) != sizeof(FDUNODE))
       {
        Message("Error reading nodelist!", -1, 0, YES);
        return -1;
        }

    return 0;
}



/* ---------------------------------------------- */
/* Open index and read controlblock, if necessary */
/* ---------------------------------------------- */

int name_index_init(void)
{
   char  nameindex[100];
   FDCTL *ctl;

   if(nodelistname[0] == '\0')
      {
      node_index_init();
      close(fdidx);
      }

   sprintf(nameindex, "%s\\userlist.fdx", cfg.usr.fdnodelist);

   if( (fdidx=open(nameindex, O_RDONLY | O_BINARY | SH_DENYNO)) == -1)
       {
       sprintf(msg, "Can't open %s!", nameindex);
       Message(msg, -1, 0, YES);
       return -1;
       }

   if(nameblocklen != 0) return 0;  /* Data already available */

   /* Read first few bytes of index, read blocklen & startblock */

   ctl = xcalloc(1, sizeof(FDCTL));

   if(read(fdidx, ctl, sizeof(FDCTL)) != sizeof(FDCTL))
      {
      Message("Error reading FD name index!", -1, 0, YES);
      close(fdidx);
      free(ctl);
      return -1;
      }

   if( (nameblocklen = ctl->blocklen) != 1061 )
      {
      Message("Unexpected blocklen in FD nodelist index!", -1, 0, YES);
      close(fdidx);
      free(ctl);
      return -1;
      }

   namestart = ctl->startblock;

   free(ctl);

   return 0;

}


/* ---------------------------------------------- */
/* Open index and read controlblock, if necessary */
/* ---------------------------------------------- */

int node_index_init(void)
{
   char  nodeindex[100];
   FDCTL *ctl;

   sprintf(nodeindex, "%s\\nodelist.fdx", cfg.usr.fdnodelist);

   if( (fdidx=open(nodeindex, O_RDONLY | O_BINARY | SH_DENYNO)) == -1)
       {
       sprintf(msg, "Can't open %s!", nodeindex);
       Message(msg, -1, 0, YES);
       return -1;
       }

   if(nodeblocklen != 0) return 0;  /* Data already available */

   /* Read first few bytes of index, read blocklen & startblock */

   ctl = xcalloc(1, sizeof(FDCTL));

   if(read(fdidx, ctl, sizeof(FDCTL)) != sizeof(FDCTL))
      {
      Message("Error reading FD nodelist index!", -1, 0, YES);
      close(fdidx);
      free(ctl);
      return -1;
      }

   if( (nodeblocklen = ctl->blocklen) != 741 )
      {
      Message("Unexpected blocklen in FD node index!", -1, 0, YES);
      close(fdidx);
      free(ctl);
      return -1;
      }

   nodestart = ctl->startblock;

   sprintf(nodelistname, "%s\\nodelist.%-3.3s", cfg.usr.fdnodelist, ctl->node_ext);

   free(ctl);

   return 0;

}



sword swaphilo(sword in)
{
   word out;

   out = (word) in >> 8;

   return (out | (word ) (in << 8));

}


static sword addmatch(sword zone, sword net, sword node, sword point, long offset)
{
   ADDRLIST *current;
   static ADDRLIST *last=NULL;
   int thisraw, fdafile;
   char rawname[100], temp[256];
   char *bbsname, *sysopname;
   int howmany = 0;
   char private = 0;
   FDANODE fdanode;

   for(current=first; current; current=current->next)
      {
      if(                                      /* Check for dupe */
         (current->address.zone  == zone ) &&
         (current->address.net   == net  ) &&
         (current->address.node  == node ) &&
         (current->address.point == point)
        )
        return 0;

      if(howmany++ > 100) return -1;    /* Don't get in (semi) endless loops */
      }

   current = xcalloc(1, sizeof(ADDRLIST));

   current->address.zone  = zone;
   current->address.net   = net ;
   current->address.node  = node;
   current->address.point = point;


   if(offset & 0x10000000L)       /* FDNET.PVT file */
      {
      if(rawfdnet == -1)
         {
         sprintf(rawname, "%s\\fdnet.pvt", cfg.usr.fdnodelist);
         if( (rawfdnet = open(rawname, O_RDONLY | O_TEXT | SH_DENYNO)) == -1)
             {
             sprintf(msg, "Can't open %s!", rawname);
             Message(msg, -1, 0, YES);
             free(current);
             return -1;
             }
         }
      thisraw = rawfdnet;
      offset &= 0x0FFFFFFFL;
      }
   else if(offset & 0x20000000L)
      {
      if(rawpoint == -1)
         {
         sprintf(rawname, "%s\\fdpoint.pvt", cfg.usr.fdnodelist);
         if( (rawpoint = open(rawname, O_RDONLY | O_TEXT | SH_DENYNO)) == -1)
             {
             sprintf(msg, "Can't open %s!", rawname);
             Message(msg, -1, 0, YES);
             free(current);
             return -1;
             }
         }
      thisraw = rawpoint;
      offset &= 0x0FFFFFFFL;
      }
   else if(offset & 0x01000000L)
      {
      private = 1;
      offset &= 0x00FFFFFFL;
      }
   else
      {
      if(rawnode == -1)
          {
          sprintf(rawname, "%s", nodelistname);
          if( (rawnode = open(rawname, O_RDONLY | O_TEXT | SH_DENYNO)) == -1)
             {
             sprintf(msg, "Can't open %s!", rawname);
             Message(msg, -1, 0, YES);
             free(current);
             return -1;
             }
          }
      thisraw = rawnode;
      }


   if(!private)  /* Not a private nodelist (.FDA) entry, search raw list */
     {
     lseek(thisraw, offset, SEEK_SET);

     if(read(thisraw, &temp, 80) == -1)
       {
       Message("Error reading raw nodelist!", -1, 0, YES);
       free(current);
       return -1;
       }


     bbsname = strtok(temp, ",");
     if(bbsname != NULL)
        {
        strncpy(current->system, bbsname, 29);
        current->system[29]='\0';
        strip_under(current->system);
        }

     sysopname = strtok(NULL, ",");   /* skip city name */
     sysopname = strtok(NULL, ",");
     if(sysopname != NULL)
        {
        strncpy(current->name  , sysopname, 39);
        current->name[39]='\0';
        strip_under(current->name);
        }

     current->next = NULL;         /* Just to be sure, clean link */
     }
   else   /* Entry in .FDA file */
     {
     sprintf(rawname, "%s\\fdnode.fda", cfg.usr.fdnodelist);
     if( (fdafile = open(rawname, O_RDONLY | O_BINARY | SH_DENYNO)) == -1)
        {
        sprintf(msg, "Can't open %s!", rawname);
        Message(msg, -1, 0, YES);
        free(current);
        return -1;
        }

     if(lseek(fdafile, (long) ((long) offset * (long) sizeof(FDANODE)), SEEK_SET)
        != (long) ((long) offset * (long) sizeof(FDANODE)))
        Message("Error seeking FDA file!", -1, 0, YES);

     if(read(fdafile, &fdanode, sizeof(FDANODE)) != sizeof(FDANODE))
        {
        sprintf(msg, "Can't read %s!", rawname);
        Message(msg, -1, 0, YES);
        free(current);
        close(fdafile);
        return -1;
        }

     close(fdafile);

     strncpy(current->system, fdanode.Name+1, 29);
     current->system[29]='\0';
     strip_under(current->system);

     strncpy(current->name  , fdanode.User+1, 36);
     current->name[36]='\0';
     strip_under(current->name);

     }

   if(first == NULL)             /* first element found */
      first = current;
   else last->next = current;    /* add to list */

   last = current;               /* remember last */

   return 0;
}


/* Convert underscores to spaces */

void strip_under(char *start)
{

   while(*start)
     {
     if(*start == '_')
        *start = ' ';
     start++;
     }

}


int addr_comp(FDNENTRY *this)
{

   if(swaphilo(this->zone) !=  matchaddress.zone)
     return (int) (swaphilo(this->zone) - matchaddress.zone);

   if(swaphilo(this->net) !=  matchaddress.net)
     return (int) (swaphilo(this->net) - matchaddress.net);

   if(swaphilo(this->node) !=  matchaddress.node)
     return (int) (swaphilo(this->node) - matchaddress.node);

   return (int) (swaphilo(this->point) - matchaddress.point);

}


static void reset(void)
{

   rawnode  = -1;
   rawpoint = -1;
   rawfdnet = -1;

   fdidx = 0;
   memset(matchname, '\0', sizeof(matchname));
   namelen = 0;
   memset(&matchaddress, '\0', sizeof(NETADDR));

   first = NULL;

}