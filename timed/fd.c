#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <share.h>

#include "includes.h"
#include <cbtree.h>

typedef struct
{
   char   flag[7];
   dword  value;

} CAPSTRINGS;

#define MAXCAPSTRINGS 29
CAPSTRINGS capstrings[MAXCAPSTRINGS] =
{
 {"CM",      0x00000002L},
 {"MO",      0x00000004L},
 {"HST",     0x00000008L},
 {"H96",     0x00000010L},
 {"PEP",     0x00000020L},
 {"MAX",     0x00000040L},
 {"XX",      0x00000080L},
 {"XB",      0x00000100L},
 {"XR",      0x00000200L},
 {"XP",      0x00000400L},
 {"XW",      0x00000800L},
 {"MNP",     0x00001000L},
 {"HST14",   0x00002000L},
 {"V32",     0x00004000L},
 {"V33",     0x00008000L},
 {"V34",     0x00010000L},
 {"V42",     0x00020000L},
 {"XC",      0x00040000L},
 {"XA",      0x00080000L},
 {"V42b",    0x00100000L},
 {"V32b",    0x00200000L},
 {"HST16",   0x00400000L},
 {"LO",      0x00800000L},
 {"ZYX",     0x01000000L},
 {"UISDNA",  0x02000000L},
 {"UISDNB",  0x04000000L},
 {"UISDNC",  0x08000000L},
 {"FAX",     0x10000000L}
};


typedef struct
{
   long          baud;
   unsigned char contents;

} BAUDFLAGS;


#define MAXBAUDFLAGS 15
BAUDFLAGS baudflags[MAXBAUDFLAGS] =
{

 { 300L   ,    2 },
 { 1200L  ,    4 },
 { 2400L  ,    5 },
 { 4800L  ,    6 },
 { 7200L  ,    10},
 { 9600L  ,    7 },
 { 12000L ,    11},
 { 14400L ,    12},
 { 16800L ,    13},
 { 19200L ,    14},
 { 38400L ,    15},
 { 57600L ,    16},
 { 64000L ,    17},
 { 76800L ,    18},
 { 115200L,    19}

};

unsigned char _MyUprTab[256] = {
      0,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,
     24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,
     40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,
     72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,
     88,  89,  90,  91,  92,  93,  94,  95,
     96,  65,  66,  67,  68,  69,  70,  71,
     72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,
     88,  89,  90, 123, 124, 125, 126, 127,
    128, 154, 144, 131, 142, 133, 143, 128,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 153, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 165, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255
};


// Upper case function for FD's index, that also tries to make upper case
// for high ASCII characters..

void FDupper(char * in);



typedef struct
{
   word  node;
   sbyte entry;

} SEARCHSTACKITEM;


#define SSTACKSIZE 32

static SEARCHSTACKITEM sstack[SSTACKSIZE];
static SEARCHSTACKITEM markstack[SSTACKSIZE];
static int sstacksize = 0;
static int markstacksize = 0;

FILE * rawnode  = NULL;
FILE * rawpoint = NULL;
FILE * rawfdnet = NULL;

static char nodelistname[_MAX_PATH];

static int     fdidx = 0;
static FDCTL   fdctl;
static FDUNODE fdunode;
static FDNNODE fdnnode;

// Define whether we are doing a name or node lookup.

static int curaction = -1;

#define CURACTIONNAME 0
#define CURACTIONNODE 1


short int  FDsearch_name(char *name, ADDRLIST *found);
short int  FDsearch_node(NETADDR *addr, ADDRLIST *found);
int        get_node(word blockno, void *node, int size);
int        DiveInto(word blockno, void *node, int len);

short int  BackupFromFDU(FDUNODE *node);
short int  BackupFromFDN(FDNNODE *node);
int        GetCurNode(void);
int        GetCurEntry(void);
int        SetCurEntry(int i);
sword near swaphilo(sword in);
int        FDnextname(void);
int        FDprevname(void);
int        FDprevnode(void);

int   addr_comp(FDNENTRY *this, NETADDR *address);
sword ReadRawNodeList(sword zone, sword net, sword node, sword point, long offset, ADDRLIST *found);
void  InterpretNodelistLine(char * temp, ADDRLIST * current);
void  CopyTillComma(char **in, char *out, int maxlen);
void  strip_under(char *s);

// ==============================================================

int FDinit(int syslookup)
{
  char temp[120];

  if(syslookup)
    {
    if(FDinit(0) == -1)    // To get the name (extention) of list, only in node index..
       return -1;
    close(fdidx);
    sprintf(temp, "%s\\userlist.fdx", cfg.usr.fdnodelist);
    fdidx = sopen(temp, O_BINARY | O_RDONLY, SH_DENYNO);
    }
  else
    {
    sprintf(temp, "%s\\nodelist.fdx", cfg.usr.fdnodelist);
    fdidx = sopen(temp, O_BINARY | O_RDONLY, SH_DENYNO);
    }

  if(fdidx == -1)
    {
    Message("Can't open index", -1, 0, YES);
    return -1;
    }

  if(read(fdidx, &fdctl, sizeof(FDCTL)) != sizeof(FDCTL))
    {
    Message("Error reading nodelist control record!", -1, 0, YES);
    close(fdidx);
    return -1;
    }

  memset(&fdunode, '\0', sizeof(FDUNODE));
  memset(&fdnnode, '\0', sizeof(FDNNODE));
  sstacksize = 0;
  markstacksize = 0;

  if(syslookup)
    curaction = CURACTIONNAME;
  else
    {
    sprintf(nodelistname, "%s\\nodelist.%-3.3s", cfg.usr.fdnodelist, fdctl.node_ext);
    curaction = CURACTIONNODE;
    }

  return 0;

}

// ==============================================================

int FDclose(void)
{

  if(fdidx > 0) close(fdidx);
  fdidx = 0;

  if(rawnode  != NULL) fclose(rawnode );
  if(rawpoint != NULL) fclose(rawpoint);
  if(rawfdnet != NULL) fclose(rawfdnet);
  rawnode = rawpoint = rawfdnet = NULL;

  memset(&fdunode, '\0', sizeof(FDUNODE));
  memset(&fdnnode, '\0', sizeof(FDNNODE));
  memset(&fdctl  , '\0', sizeof(FDCTL  ));

  sstacksize    =  0;
  markstacksize =  0;
  curaction     = -1;

  return 0;

}

// ==============================================================

short int FDcurr(ADDRLIST *found)
{

  if(curaction == -1)
    return ERROR;

  if(sstacksize == 1 && sstack[0].entry == -1)
    return BOI;

  if(curaction == CURACTIONNAME)
    {
    if((GetCurEntry() < 0) || (GetCurEntry() > (fdunode.count-1)))
      return ERROR;
    }
  else
    {
    if((GetCurEntry() < 0) || (GetCurEntry() > (fdnnode.count-1)))
      return ERROR;
    }

  if(curaction == CURACTIONNODE)
    {
    if(ReadRawNodeList(swaphilo(fdnnode.nodedata[GetCurEntry()].zone),
                       swaphilo(fdnnode.nodedata[GetCurEntry()].net),
                       swaphilo(fdnnode.nodedata[GetCurEntry()].node),
                       swaphilo(fdnnode.nodedata[GetCurEntry()].point),
                       fdnnode.nodedata[GetCurEntry()].offset,
                       found) == -1)
                       return ERROR;
    }
  else
    {
    if(ReadRawNodeList(swaphilo(fdunode.nodedata[GetCurEntry()].zone),
                       swaphilo(fdunode.nodedata[GetCurEntry()].net),
                       swaphilo(fdunode.nodedata[GetCurEntry()].node),
                       swaphilo(fdunode.nodedata[GetCurEntry()].point),
                       fdunode.nodedata[GetCurEntry()].offset,
                       found) == -1)
                       return ERROR;
    }

  return OK;

}

// ==============================================================

short int FDhead(ADDRLIST *found)
{

  if(curaction == -1)
    return ERROR;

  sstacksize = 0;

  if(curaction == CURACTIONNAME)
    {
    if(DiveInto(fdctl.startblock, &fdunode, sizeof(FDUNODE)) == -1)
      return ERROR;

    while(fdunode.nextlevel != 0)   // Go back in index, until we hit a leaf
      {
      SetCurEntry(-1);
      if(DiveInto(fdunode.nextlevel, &fdunode, sizeof(FDUNODE)) == -1)
        return ERROR;
      }

    // We are now in a leaf. Set the pointer to the very first entry
    SetCurEntry(0);
    }
  else      // Node lookup
    {
    if(DiveInto(fdctl.startblock, &fdnnode, sizeof(FDNNODE)) == -1)
      return ERROR;

    while(fdnnode.nextlevel != 0)   // Go back in index, until we hit a leaf
      {
      SetCurEntry(-1);
      if(DiveInto(fdnnode.nextlevel, &fdnnode, sizeof(FDNNODE)) == -1)
        return ERROR;
      }

    // We are now in a leaf. Set the pointer to the very first entry
    SetCurEntry(0);
    }

  if(found)
    return FDcurr(found);
  else
    return OK;

}


// ==============================================================

short int FDtail(ADDRLIST *found)
{

  if(curaction == -1)
    return ERROR;

  sstacksize = 0;

  if(curaction == CURACTIONNAME)
    {
    if(DiveInto(fdctl.startblock, &fdunode, sizeof(FDUNODE)) == -1)
      return ERROR;

    while(fdunode.nextlevel != 0)   // Go fwd in index, until we hit a leaf
      {
      SetCurEntry(fdunode.count - 1);
      if(DiveInto(fdunode.nodedata[fdunode.count-1].ptr, &fdunode, sizeof(FDUNODE)) == -1)
        return ERROR;
      }

    // We are now in a leaf. Set the pointer to the very last entry
    SetCurEntry(fdunode.count-1);
    }
  else      // Node lookup
    {
    if(DiveInto(fdctl.startblock, &fdnnode, sizeof(FDNNODE)) == -1)
      return ERROR;

    while(fdnnode.nextlevel != 0)   // Go fwd in index, until we hit a leaf
      {
      SetCurEntry(fdnnode.count - 1);
      if(DiveInto(fdnnode.nodedata[fdnnode.count-1].ptr, &fdnnode, sizeof(FDNNODE)) == -1)
        return ERROR;
      }

    // We are now in a leaf. Set the pointer to the very last entry
    SetCurEntry(fdnnode.count-1);
    }

  if(found)
    return FDcurr(found);
  else
    return OK;

}

// ==============================================================

short int FDmark(void)
{

  memcpy(&markstack, &sstack, SSTACKSIZE * sizeof(SEARCHSTACKITEM));
  markstacksize = sstacksize;
  return OK;

}

// ==============================================================

short int FDfindmark(ADDRLIST *found)
{

  memcpy(&sstack, &markstack, SSTACKSIZE * sizeof(SEARCHSTACKITEM));
  sstacksize = markstacksize;

  // Read in the current block again, so we can reference it in mem.
  if(curaction == CURACTIONNAME)
    {
    if(get_node(sstack[sstacksize-1].node, &fdunode, sizeof(FDUNODE)) == -1)
      return ERROR;
    }
  else
    {
    if(get_node(sstack[sstacksize-1].node, &fdnnode, sizeof(FDNNODE)) == -1)
      return ERROR;
    }

  if(found)
    return FDcurr(found);
  else
    return OK;

}

// ==============================================================

// End of externally visible functions
// -----------------------------------

// ==============================================================


short int FDsearch_name(char *name, ADDRLIST *found)
{
    int j, k;
    unsigned nextblock=fdctl.startblock;
    int count;
    char matchname[16] = "";
    int namelen = strlen(name);

/*
 * Read the first Index node.
 */

    strncpy(matchname, name, 15);
    strupr(matchname);

    if(DiveInto(fdctl.startblock, &fdunode, sizeof(FDUNODE)) == -1)
      {
      return ERROR;
      }

    while (fdunode.nextlevel != 0)   /* Until we get to a leaf node */
       {
       if ((count = fdunode.count) == 0) return NOTFOUND;  /* No nodes ?! */

       for (j = 0; j < count; j++) /* check 32 or maybe less, if count was not 32 */
          {
          SetCurEntry(j);
          k = strncmp(fdunode.nodedata[j].name, matchname, namelen);

          if (k > 0)     /* Key in node larger than what we're looking for, go into subtree of entry before this one */
             break;

          if (k == 0)   /* match! */
             {
             if(found)
               ReadRawNodeList(swaphilo(fdunode.nodedata[j].zone),
                               swaphilo(fdunode.nodedata[j].net),
                               swaphilo(fdunode.nodedata[j].node),
                               swaphilo(fdunode.nodedata[j].point),
                               fdunode.nodedata[j].offset,
                               found);
             return FOUND;
             }
          }

       if (j == 0)
          {
          SetCurEntry(-1);
          nextblock = fdunode.nextlevel;
          }
       else
          {
          SetCurEntry(j-1);
          nextblock = fdunode.nodedata[j-1].ptr;
          }

       if(DiveInto(nextblock, &fdunode, sizeof(FDUNODE)) == -1)
         {
         return ERROR;
         }
       }

    /* If we are here we are searching a leafnode */

    if ((count = fdunode.count) != 0)
       {
       for (j = 0; j < count; j++) /* check 32 or less */
          {
          SetCurEntry(j);

          k = strncmp(fdunode.nodedata[j].name, matchname, namelen);

          if (k > 0)
             break;

          if (k == 0)
            {
            if(found)
              ReadRawNodeList(swaphilo(fdunode.nodedata[j].zone),
                              swaphilo(fdunode.nodedata[j].net),
                              swaphilo(fdunode.nodedata[j].node),
                              swaphilo(fdunode.nodedata[j].point),
                              fdunode.nodedata[j].offset,
                              found);
            return FOUND;
            }

          } /* for */
       }

    return NOTFOUND;
}

// ==============================================================

short int FDsearch_node(NETADDR *address, ADDRLIST *found)
{
    int j, k;
    unsigned nextblock=fdctl.startblock;
    int count;

/*
 * Read the first Index node.
 */

    if(DiveInto(fdctl.startblock, &fdnnode, sizeof(FDNNODE)) == -1)
      {
      return ERROR;
      }

    while (fdnnode.nextlevel != 0)   /* Until we get to a leaf node */
       {
       if ((count = fdnnode.count) == 0) return NOTFOUND;  /* No nodes ?! */

       for (j = 0; j < count; j++) /* check 32 or maybe less, if count was not 32 */
          {
          SetCurEntry(j);
          k = addr_comp(&fdnnode.nodedata[j], address);

          if (k > 0)     /* Key in node larger than what we're looking for, go into subtree of entry before this one */
             break;

          if (k == 0)   /* match! */
             {
             if(found)
               ReadRawNodeList(swaphilo(fdnnode.nodedata[j].zone),
                               swaphilo(fdnnode.nodedata[j].net),
                               swaphilo(fdnnode.nodedata[j].node),
                               swaphilo(fdnnode.nodedata[j].point),
                               fdnnode.nodedata[j].offset,
                               found);
             return FOUND;
             }
          }

       if (j == 0)
          {
          SetCurEntry(-1);
          nextblock = fdnnode.nextlevel;
          }
       else
          {
          SetCurEntry(j-1);
          nextblock = fdnnode.nodedata[j-1].ptr;
          }

       if(DiveInto(nextblock, &fdnnode, sizeof(FDNNODE)) == -1)
         {
         return ERROR;
         }
       }


    /* If we are here we are searching a leafnode */

    if ((count = fdnnode.count) != 0)
       {
       for (j = 0; j < count; j++) /* check 32 or less */
          {
          SetCurEntry(j);

          k = addr_comp(&fdnnode.nodedata[j], address);

          if (k > 0)
             break;

          if (k == 0)
            {
            if(found)
              ReadRawNodeList(swaphilo(fdnnode.nodedata[j].zone),
                              swaphilo(fdnnode.nodedata[j].net),
                              swaphilo(fdnnode.nodedata[j].node),
                              swaphilo(fdnnode.nodedata[j].point),
                              fdnnode.nodedata[j].offset,
                              found);
            return FOUND;
            }

          } /* for */
       }

    return NOTFOUND;
}


// ==============================================================

int get_node(word blockno, void * node, int size)
{

   if(blockno == 0)
     {
     Message("Block number 0 requested!", -1, 0, YES);
     return -1;
     }

   if(lseek (fdidx, (long) ((long)blockno * (long)size), SEEK_SET) == -1)
     {
     Message("Error seeking index!", -1, 0, YES);
     return -1;
     }

   if(read (fdidx, node, (unsigned)size) != (int)size)
     {
     Message("Error reading nodelist index!", -1, 0, YES);
     return -1;
     }

   return 0;
}

// ==============================================================

int DiveInto(word blockno, void *node, int size)
{

   if(sstacksize == SSTACKSIZE)
     {
     Message("Maximum height of tree reached!", -1, 0, YES);
     return -1;
     }

   sstack[sstacksize].node  = blockno;
   sstack[sstacksize].entry = 0;
   sstacksize++;

   if(get_node(blockno, node, size) == -1)
     return -1;

   return 0;

}

// ==============================================================

short int BackupFromFDU(FDUNODE *node)
{
  if(sstacksize <= 1)
    return EOI;

  sstacksize--;

  if(get_node(sstack[sstacksize-1].node, node, sizeof(FDUNODE)) == -1)
    return ERROR;

  return OK;
}

// ==============================================================

short int BackupFromFDN(FDNNODE *node)
{
  if(sstacksize <= 1)
    return EOI;

  sstacksize--;

  if(get_node(sstack[sstacksize-1].node, node, sizeof(FDNNODE)) == -1)
    return ERROR;

  return OK;
}


// ==============================================================

int GetCurNode(void)
{
   if(sstacksize < 1)
     return -1;

   return sstack[sstacksize-1].node;

}

// ==============================================================

int GetCurEntry(void)
{
   if(sstacksize < 1)
     return -1;

   return sstack[sstacksize-1].entry;

}

// ==============================================================

int SetCurEntry(int i)
{
   if(sstacksize < 1)
     return -1;

   sstack[sstacksize-1].entry = i;

   return 0;
}

// ==============================================================

sword near swaphilo(sword in)
{
   word out;

   out = (word) in >> 8;

   return (out | (word ) (in << 8));

}

// ==============================================================

int FDnextname(void)
{
  short int status;


  if(fdunode.nextlevel == 0)    // We are in a leaf node.
    {
    if(GetCurEntry() < (fdunode.count-1)) // Can we get next item from this leaf?
      {
      SetCurEntry(GetCurEntry() + 1);
      return OK;
      }

    // We can not get item from this leaf.
    // Go back if we can, and go into next leaf.

    do
      {
      if((status=BackupFromFDU(&fdunode)) != OK)     // Go back into index node.
         return status;

      } while(GetCurEntry() >= (fdunode.count-1));

    SetCurEntry(GetCurEntry() + 1);
    }
  else   // We are in an index node.
    {
    do
      {
      if(GetCurEntry() == -1)
        {
        if(DiveInto(fdunode.nextlevel, &fdunode, sizeof(FDUNODE)) != 0)
          return ERROR;
        }
      else
        {
        if(DiveInto(fdunode.nodedata[GetCurEntry()].ptr, &fdunode, sizeof(FDUNODE)) != 0)
          return ERROR;
        }

      if(fdunode.nextlevel != 0)
        SetCurEntry(-1);

      } while(fdunode.nextlevel != 0);
    }

  return OK;
}

// ==============================================================

int FDnextnode(void)
{
  short int status;


  if(fdnnode.nextlevel == 0)    // We are in a leaf node.
    {
    if(GetCurEntry() < (fdnnode.count-1)) // Can we get next item from this leaf?
      {
      SetCurEntry(GetCurEntry() + 1);
      return OK;
      }

    // We can not get item from this leaf.
    // Go back if we can, and go into next leaf.

    do
      {
      if((status=BackupFromFDN(&fdnnode)) != OK)     // Go back into index node.
         return status;

      } while(GetCurEntry() >= (fdnnode.count-1));

    SetCurEntry(GetCurEntry() + 1);
    }
  else   // We are in an index node.
    {
    do
      {
      if(GetCurEntry() == -1)
        {
        if(DiveInto(fdnnode.nextlevel, &fdnnode, sizeof(FDNNODE)) != 0)
          return ERROR;
        }
      else
        {
        if(DiveInto(fdnnode.nodedata[GetCurEntry()].ptr, &fdnnode, sizeof(FDNNODE)) != 0)
          return ERROR;
        }

      if(fdnnode.nextlevel != 0)
        SetCurEntry(-1);

      } while(fdnnode.nextlevel != 0);
    }

  return OK;

}


// ==============================================================

int FDprevname(void)
{
  short int status;

  if(fdunode.nextlevel == 0)    // We are in a leaf node.
    {
    if(GetCurEntry() > 0) // Can we get next item from this leaf?
      {
      SetCurEntry(GetCurEntry() - 1);
      return OK;
      }

    // We can not get item from this leaf.
    // Go back if we can, and go into previous leaf.

    do
      {
      if((status=BackupFromFDU(&fdunode)) != OK)     // Go back into index node.
        {
        if(status == EOI)
          return BOI;
        else
          return status;
        }
      } while(GetCurEntry() == -1);

    }
  else   // We are in an index node.
    {
    SetCurEntry(GetCurEntry() - 1);
    do    // Dive into previous link, all the way to the leaf node.
      {
      if(GetCurEntry() == -1)
        {
        if(DiveInto(fdunode.nextlevel, &fdunode, sizeof(FDUNODE)) != 0) // special case, nxtlvl
          return ERROR;
         }
      else
        {
        if(DiveInto(fdunode.nodedata[GetCurEntry()].ptr, &fdunode, sizeof(FDUNODE)) != 0)
          return ERROR;
        }

      SetCurEntry(fdunode.count - 1);

      } while(fdunode.nextlevel != 0);
    }

  return OK;
}

// ==============================================================

int FDprevnode(void)
{
  short int status;

  if(fdnnode.nextlevel == 0)    // We are in a leaf node.
    {
    if(GetCurEntry() > 0) // Can we get next item from this leaf?
      {
      SetCurEntry(GetCurEntry() - 1);
      return OK;
      }

    // We can not get item from this leaf.
    // Go back if we can, and go into previous leaf.

    do
      {
      if((status=BackupFromFDN(&fdnnode)) != OK)     // Go back into index node.
         {
         if(status == EOI)
           return BOI;
         else
           return status;
         }
      } while(GetCurEntry() == -1);

    }
  else   // We are in an index node.
    {
    SetCurEntry(GetCurEntry() - 1);
    do    // Dive into previous link, all the way to the leaf node.
      {
      if(GetCurEntry() == -1)
        {
        if(DiveInto(fdnnode.nextlevel, &fdnnode, sizeof(FDNNODE)) != 0) // special case, nxtlvl
          return ERROR;
         }
      else
        {
        if(DiveInto(fdnnode.nodedata[GetCurEntry()].ptr, &fdnnode, sizeof(FDNNODE)) != 0)
          return ERROR;
        }

      SetCurEntry(fdnnode.count - 1);

      } while(fdnnode.nextlevel != 0);
    }

  return OK;
}

// ==============================================================

int addr_comp(FDNENTRY *this, NETADDR *address)
{

   if(swaphilo(this->zone) !=  address->zone)
     return (int) (swaphilo(this->zone) - address->zone);

   if(swaphilo(this->net) !=  address->net)
     return (int) (swaphilo(this->net) - address->net);

   if(swaphilo(this->node) !=  address->node)
     return (int) (swaphilo(this->node) - address->node);

   return (int) (swaphilo(this->point) - address->point);

}

// ==============================================================

sword ReadRawNodeList(sword zone, sword net, sword node, sword point, long offset, ADDRLIST *current)
{
   int fdafile;
   FILE *thisraw;
   char rawname[100], temp[256];
   char *charptr;
   char private = 0;
   FDANODE fdanode;
   int i;

   memset(current, '\0', sizeof(ADDRLIST));

   current->address.zone  = zone;
   current->address.net   = net;
   current->address.node  = node;
   current->address.point = point;

   if(offset & 0x10000000L)       /* FDNET.PVT file */
      {
      if(rawfdnet == NULL)
         {
         sprintf(rawname, "%s\\fdnet.pvt", cfg.usr.fdnodelist);
         if( (rawfdnet = _fsopen(rawname, "rt", SH_DENYNO)) == NULL)
             {
             sprintf(msg, "Can't open %s!", rawname);
             Message(msg, -1, 0, YES);
             return -1;
             }
         }
      thisraw = rawfdnet;
      offset &= 0x0FFFFFFFL;
      }
   else if(offset & 0x20000000L)
      {
      if(rawpoint == NULL)
         {
         sprintf(rawname, "%s\\fdpoint.pvt", cfg.usr.fdnodelist);
         if( (rawpoint = _fsopen(rawname, "rt", SH_DENYNO)) == NULL)
             {
             sprintf(msg, "Can't open %s!", rawname);
             Message(msg, -1, 0, YES);
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
      if(rawnode == NULL)
          {
          if( (rawnode = _fsopen(nodelistname, "rt", SH_DENYNO)) == NULL)
             {
             sprintf(msg, "Can't open %s!", nodelistname);
             Message(msg, -1, 0, YES);
             return -1;
             }
          }
      thisraw = rawnode;
      }


   if(!private)  /* Not a private nodelist (.FDA) entry, search raw list */
     {
     memset(temp, '\0', sizeof(temp));
     fseek(thisraw, offset, SEEK_SET);

     if(fgets(temp, 249, thisraw) == NULL)
       {
       Message("Error reading raw nodelist!", -1, 0, YES);
       return -1;
       }

     // NULL terminate, and terminate string at end of line (prevent next
     // line to be used as part of this entry.

     temp[250] = '\0';
     if((charptr=strchr(temp, '\r')) != NULL)
       *charptr = '\0';
     if((charptr=strchr(temp, '\n')) != NULL)
       *charptr = '\0';

     InterpretNodelistLine(temp, current);

     }
   else   /* Entry in .FDA file */
     {
     sprintf(rawname, "%s\\fdnode.fda", cfg.usr.fdnodelist);
     if( (fdafile = sopen(rawname, O_RDONLY | O_BINARY, SH_DENYNO)) == -1)
        {
        sprintf(msg, "Can't open %s!", rawname);
        Message(msg, -1, 0, YES);
        return -1;
        }

     if(lseek(fdafile, (long) ((long) offset * (long) sizeof(FDANODE)), SEEK_SET)
        != (long) ((long) offset * (long) sizeof(FDANODE)))
        Message("Error seeking FDA file!", -1, 0, YES);

     if(read(fdafile, &fdanode, sizeof(FDANODE)) != sizeof(FDANODE))
        {
        sprintf(msg, "Can't read %s!", rawname);
        Message(msg, -1, 0, YES);
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

     strncpy(current->location  , fdanode.Location+1, 28);
     current->location[29]='\0';
     strip_under(current->name);

     strncpy(current->phone, fdanode.Telephone+1, 19);
     current->phone[19]='\0';


     for(i=0; i<MAXBAUDFLAGS; i++)
       {
       if(fdanode.MaxBaud == baudflags[i].contents)
          {
          current->baud = baudflags[i].baud;
          break;
          }
       }

     for(i=0; i<MAXCAPSTRINGS; i++)
       {
       if(fdanode.Capability & capstrings[i].value)
         {
         if(current->flags[0] != '\0')
            strcat(current->flags, ",");
         if(strlen(current->flags) < 33)
            strcat(current->flags, capstrings[i].flag);
         }
       }
     }

   return 0;
}


// =============================================================
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

// ==============================================================

void InterpretNodelistLine(char * temp, ADDRLIST * current)
{
  char *charptr = temp;
  char tempbaud[10];

  CopyTillComma(&charptr, current->system, 45);
  CopyTillComma(&charptr, current->location, 30);
  CopyTillComma(&charptr, current->name, 40);
  CopyTillComma(&charptr, current->phone, 20);
  CopyTillComma(&charptr, tempbaud, 10);
  current->baud = atol(tempbaud);
  strncpy(current->flags, charptr, 59);

  strip_under(current->system);
  strip_under(current->location);
  strip_under(current->name);

}

// ==============================================================

void CopyTillComma(char **in, char *out, int maxlen)
{
   char *inptr = *in;
   int curlen = 0;

   while(*inptr && *inptr != ',' && ++curlen < maxlen)
     *out++ = *inptr++;

   *out = '\0';
   // Maybe string is longer than maxlen chars, skip to comma then anyway.

   while(*inptr != '\0' && *inptr != ',')
     inptr++;

   if(*inptr == ',')
     inptr++;           // skip current comma

   *in = inptr;   // set original pointer, so calling f'ion knows where he is
}

// ==============================================================



void FDupper(char *in)
{
  if(!in) return;

  while(*in)
    {
    *in = _MyUprTab[*in];
    in++;
    }

}

// ==============================================================


