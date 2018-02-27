#include "includes.h"

typedef struct
{
    char flag[7];
    dword value;

} CAPSTRINGS;

#define MAXCAPSTRINGS 29
CAPSTRINGS capstrings[MAXCAPSTRINGS] = {
    {"CM", 0x00000002L},
    {"MO", 0x00000004L},
    {"HST", 0x00000008L},
    {"H96", 0x00000010L},
    {"PEP", 0x00000020L},
    {"MAX", 0x00000040L},
    {"XX", 0x00000080L},
    {"XB", 0x00000100L},
    {"XR", 0x00000200L},
    {"XP", 0x00000400L},
    {"XW", 0x00000800L},
    {"MNP", 0x00001000L},
    {"HST14", 0x00002000L},
    {"V32", 0x00004000L},
    {"V33", 0x00008000L},
    {"V34", 0x00010000L},
    {"V42", 0x00020000L},
    {"XC", 0x00040000L},
    {"XA", 0x00080000L},
    {"V42b", 0x00100000L},
    {"V32b", 0x00200000L},
    {"HST16", 0x00400000L},
    {"LO", 0x00800000L},
    {"ZYX", 0x01000000L},
    {"UISDNA", 0x02000000L},
    {"UISDNB", 0x04000000L},
    {"UISDNC", 0x08000000L},
    {"FAX", 0x10000000L}
};



typedef struct
{
    long baud;
    unsigned char contents;

} BAUDFLAGS;


#define MAXBAUDFLAGS 15
BAUDFLAGS baudflags[MAXBAUDFLAGS] = {

    {300L, 2},
    {1200L, 4},
    {2400L, 5},
    {4800L, 6},
    {7200L, 10},
    {9600L, 7},
    {12000L, 11},
    {14400L, 12},
    {16800L, 13},
    {19200L, 14},
    {38400L, 15},
    {57600L, 16},
    {64000L, 17},
    {76800L, 18},
    {115200L, 19}

};



int near name_index_init(void); /* 0 on success, -1 on error */
int near node_index_init(void);

int near get_node_node(unsigned blockno, FDNNODE * node);
int near get_name_node(unsigned blockno, FDUNODE * node);

void near search_name(unsigned startblock);
void near search_address(unsigned startblock);

static sword near addmatch(sword zone, sword net, sword node, sword point,
                           long offset);
sword near swaphilo(sword in);
int near addr_comp(FDNENTRY * this);
int near two_addr_comp(FDNENTRY * this, FDNENTRY * that);
void near strip_under(char *start);
static void near reset(void);

static unsigned nodeblocklen = 0;
static unsigned nameblocklen = 0;
static unsigned nodestart = 0;
static unsigned namestart = 0;
static char nodelistname[100] = "";

int rawnode = -1;
int rawpoint = -1;
int rawfdnet = -1;
int foundamatch;                // When browsing, is starting point
                                // already found?

int fdidx;
static char matchname[16];
static int namelen;
static NETADDR matchaddress;
static NETADDR lastone;


static ADDRLIST *first = NULL;


ADDRLIST *getFDnode(NETADDR * address)
{

    reset();

    if (cfg.usr.fdnodelist[0] == '\0')
        return NULL;

    if (node_index_init() == -1)
        return NULL;

    matchaddress.zone = address->zone;
    matchaddress.net = address->net;
    matchaddress.node = address->node;
    matchaddress.point = address->point;

    search_address(nodestart);

    close(fdidx);
    if (rawnode != -1)
        close(rawnode);
    if (rawpoint != -1)
        close(rawpoint);
    if (rawfdnet != -1)
        close(rawfdnet);

    return first;
}



ADDRLIST *getFDname(char *name)
{
    char last_name_first[80];
    char midname[80];
    char *c, *p, *m;

    reset();

    if (cfg.usr.fdnodelist[0] == '\0')
        return NULL;

    if (name_index_init() == -1)
        return NULL;

    foundamatch = 0;

    c = midname;                /* Start of temp name buff */
    p = name;                   /* Point to start of name */
    m = NULL;                   /* Init pointer to space */

    *c = *p++;
    while (*c)                  /* Go entire length of name */
    {
        if (*c == ' ')          /* Look for space */
            m = c;              /* Save location */
        c++;
        *c = *p++;
    }

    if (m != NULL)              /* If we have a pointer, */
    {
        *m++ = '\0';            /* Terminate the first half */
        (void)strcpy(last_name_first, m); /* Now copy the last name */
        (void)strcat(last_name_first, " "); /* Insert a space */
        (void)strcat(last_name_first, midname); /* Finally copy first half 
                                                 */
    }
    else
        (void)strcpy(last_name_first, midname); /* Use whole name
                                                   otherwise */

    memset(matchname, '\0', sizeof(matchname));
    strncpy(matchname, last_name_first, 15);
    strupr(matchname);

    namelen = (int)strlen(matchname); /* Calc length now */

    search_name(namestart);

    close(fdidx);
    if (rawnode != -1)
        close(rawnode);
    if (rawpoint != -1)
        close(rawpoint);
    if (rawfdnet != -1)
        close(rawfdnet);

    return first;
}



void near search_name(unsigned startblock)
{
    int j, k;
    FDUNODE *thisnode;
    unsigned nextblock = startblock;
    int count;

/*
 * Read the first Index node.
 */


    thisnode = mem_calloc(1, sizeof(FDUNODE));

    if (get_name_node(startblock, thisnode) == -1)
    {
        mem_free(thisnode);
        return;
    }


    while (thisnode->nextlevel != 0) /* Until we get to a leaf node */
    {
        if ((count = thisnode->count) == 0)
            return;             /* No nodes ?! */

//       if(count == 16) count = 32;   /* Dunno why, full node == 32, but says 16 [BUT NOT ALWAYS!] */

        for (j = 0; j < count; j++) /* check 32 or maybe less, if count
                                       was not 16 */
        {
//          if( (j > 15) && (strcmp(thisnode->nodedata[j-1].name,
//                                  thisnode->nodedata[j].name) > 0) )
//             break;  // Weird, count was prob 16, but really 16 entries, not 32!

            k = strncmp(thisnode->nodedata[j].name, matchname, namelen);

            if (k > 0)          /* Key in node larger than what we're
                                   looking for, go into subtree of entry
                                   before this one */
                break;

            if (k == 0)         /* match! */
            {
                /* Recurse into previous entry's pointer to find earlier
                   matches */

                if (j == 0)     /* Was this the first entry? */
                {
                    if (thisnode->nextlevel != 0) /* Is there a 'next
                                                     level'? */
                        search_name(thisnode->nextlevel);
                }
                else
                {
                    search_name(thisnode->nodedata[j - 1].ptr);
                }

                /* Add this index node to list */

                if (addmatch(swaphilo(thisnode->nodedata[j].zone),
                             swaphilo(thisnode->nodedata[j].net),
                             swaphilo(thisnode->nodedata[j].node),
                             swaphilo(thisnode->nodedata[j].point),
                             thisnode->nodedata[j].offset) == -1)
                {               /* Error or too many matches, stop here */
                    mem_free(thisnode);
                    return;
                }

            }
        }

        if (j == 0)
            nextblock = thisnode->nextlevel;
        else
            nextblock = thisnode->nodedata[j - 1].ptr;

        if (get_name_node(nextblock, thisnode) == -1)
        {
            mem_free(thisnode);
            return;
        }

    }


    /* If we are here we are searching a leafnode */


    if ((count = thisnode->count) != 0)
    {
//       if(count == 16) count = 32;  /* Dunno why, full == 32 nodes! */

        for (j = 0; j < count; j++) /* check 32 or less */
        {
//          if( (j > 15) && (strcmp(thisnode->nodedata[j-1].name,
//                                   thisnode->nodedata[j].name) > 0) )
//             break;  // Weird, count was prob 16, but really 16 entries, not 32!


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
                    break;      /* Error or too much, stop here */

            }                   /* if */

        }                       /* for */
    }

    mem_free(thisnode);

}



void near search_address(unsigned startblock)
{
    int j, k;
    FDNNODE *thisnode;
    unsigned nextblock = startblock;
    int count;

/*
 * Read the first Index node.
 */

    thisnode = mem_calloc(1, sizeof(FDNNODE));

    if (get_node_node(startblock, thisnode) == -1)
    {
        mem_free(thisnode);
        return;
    }



    while (thisnode->nextlevel != 0) /* Until we get to a leaf node */
    {
        if ((count = thisnode->count) == 0)
            return;             /* No nodes ?! */

//       if(count == 16) count = 32;   /* Dunno why, full node == 32, but says 16 */

        for (j = 0; j < count; j++) /* check 32 or maybe less, if count
                                       was not 16 */
        {
//          if( (j > 15) && (two_addr_comp(&thisnode->nodedata[j-1],
//                                         &thisnode->nodedata[j]) > 0) )
//             break;  // Weird, count was prob 16, but really 16 entries, not 32!

            k = addr_comp(&thisnode->nodedata[j]);

            if (k > 0)          /* Key in node larger than what we're
                                   looking for, go into subtree of entry
                                   before this one */
                break;

            if (k == 0)         /* match! */
            {
                /* Add this index node to list */

                addmatch(swaphilo(thisnode->nodedata[j].zone),
                         swaphilo(thisnode->nodedata[j].net),
                         swaphilo(thisnode->nodedata[j].node),
                         swaphilo(thisnode->nodedata[j].point),
                         thisnode->nodedata[j].offset);

                /* Only one match needed/expected, bail out */

                mem_free(thisnode);
                return;
            }
        }

        if (j == 0)
            nextblock = thisnode->nextlevel;
        else
            nextblock = thisnode->nodedata[j - 1].ptr;

        if (get_node_node(nextblock, thisnode) == -1)
        {
            mem_free(thisnode);
            return;
        }

    }


    /* If we are here we are searching a leafnode */


    if ((count = thisnode->count) != 0)
    {
//       if(count == 16) count = 32;  /* Dunno why, full == 32 nodes! */

        for (j = 0; j < count; j++) /* check 32 or less */
        {
//          if( (j > 15) && (two_addr_comp(&thisnode->nodedata[j-1],
//                                         &thisnode->nodedata[j]) > 0) )
//             break;  // Weird, count was prob 16, but really 16 entries, not 32!

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

                break;          /* Only one match needed, stop here */
            }                   /* if */

        }                       /* for */
    }

    mem_free(thisnode);

}


int near get_node_node(unsigned blockno, FDNNODE * node)
{
    if (blockno == 0)
    {
        Message("Block number 0 requested!", -1, 0, YES);
        return -1;
    }

    if ((lseek(fdidx, (long)(blockno * 741L), SEEK_SET)) == -1)
    {
        Message("Error seeking node index!", -1, 0, YES);
        return -1;
    }

    if ((read(fdidx, node, sizeof(FDNNODE))) != sizeof(FDNNODE))
    {
        Message("Error reading nodelist!", -1, 0, YES);
        return -1;
    }

    return 0;
}

/* --------------------------------------------- */

int near get_name_node(unsigned blockno, FDUNODE * node)
{

    if (blockno == 0)
    {
        Message("Block number 0 requested!", -1, 0, YES);
        return -1;
    }

    if ((lseek(fdidx, (long)(blockno * 1061L), SEEK_SET)) == -1)
    {
        Message("Error seeking name index!", -1, 0, YES);
        return -1;
    }

    if ((read(fdidx, node, sizeof(FDUNODE))) != sizeof(FDUNODE))
    {
        Message("Error reading nodelist!", -1, 0, YES);
        return -1;
    }

    return 0;
}



/* ---------------------------------------------- */
/* Open index and read controlblock, if necessary */
/* ---------------------------------------------- */

int near name_index_init(void)
{
    char nameindex[100];
    FDCTL *ctl;

    if (nodelistname[0] == '\0')
    {
        node_index_init();
        close(fdidx);
    }

    sprintf(nameindex, "%s\\userlist.fdx", cfg.usr.fdnodelist);

    if ((fdidx = sopen(nameindex, O_RDONLY | O_BINARY, SH_DENYNO)) == -1)
    {
        sprintf(msg, "Can't open %s!", nameindex);
        Message(msg, -1, 0, YES);
        return -1;
    }

    if (nameblocklen != 0)
        return 0;               /* Data already available */

    /* Read first few bytes of index, read blocklen & startblock */

    ctl = mem_calloc(1, sizeof(FDCTL));

    if (read(fdidx, ctl, sizeof(FDCTL)) != sizeof(FDCTL))
    {
        Message("Error reading FD name index!", -1, 0, YES);
        close(fdidx);
        mem_free(ctl);
        return -1;
    }

    if ((nameblocklen = ctl->blocklen) != 1061)
    {
        Message("Unexpected blocklen in FD nodelist index!", -1, 0, YES);
        close(fdidx);
        mem_free(ctl);
        return -1;
    }

    namestart = ctl->startblock;

    mem_free(ctl);

    return 0;

}


/* ---------------------------------------------- */
/* Open index and read controlblock, if necessary */
/* ---------------------------------------------- */

int near node_index_init(void)
{
    char nodeindex[100];
    FDCTL *ctl;

    sprintf(nodeindex, "%s\\nodelist.fdx", cfg.usr.fdnodelist);

    if ((fdidx = sopen(nodeindex, O_RDONLY | O_BINARY, SH_DENYNO)) == -1)
    {
        sprintf(msg, "Can't open %s!", nodeindex);
        Message(msg, -1, 0, YES);
        return -1;
    }

    if (nodeblocklen != 0)
        return 0;               /* Data already available */

    /* Read first few bytes of index, read blocklen & startblock */

    ctl = mem_calloc(1, sizeof(FDCTL));

    if (read(fdidx, ctl, sizeof(FDCTL)) != sizeof(FDCTL))
    {
        Message("Error reading FD nodelist index!", -1, 0, YES);
        close(fdidx);
        mem_free(ctl);
        return -1;
    }

    if ((nodeblocklen = ctl->blocklen) != 741)
    {
        Message("Unexpected blocklen in FD node index!", -1, 0, YES);
        close(fdidx);
        mem_free(ctl);
        return -1;
    }

    nodestart = ctl->startblock;

    sprintf(nodelistname, "%s\\nodelist.%-3.3s", cfg.usr.fdnodelist,
            ctl->node_ext);

    mem_free(ctl);

    return 0;

}



sword near swaphilo(sword in)
{
    word out;

    out = (word) in >> 8;

    return (out | (word) (in << 8));

}


static sword near addmatch(sword zone, sword net, sword node, sword point,
                           long offset)
{
    ADDRLIST *current;
    static ADDRLIST *last = NULL;
    int thisraw, fdafile;
    char rawname[100], temp[256];
    char *bbsname, *sysopname, *charptr;
    int howmany = 0;
    char private = 0;
    FDANODE fdanode;
    int i;


    for (current = first; current; current = current->next)
    {
        if (                    /* Check for dupe */
               (current->address.zone == zone) &&
               (current->address.net == net) &&
               (current->address.node == node) &&
               (current->address.point == point))
        {
//        return 0;
        }

        if (howmany++ > 100)
            return -1;          /* Don't get in (semi) endless loops */
    }

    current = mem_calloc(1, sizeof(ADDRLIST));

    lastone.zone = current->address.zone = zone;
    lastone.net = current->address.net = net;
    lastone.node = current->address.node = node;
    lastone.point = current->address.point = point;


    if (offset & 0x10000000L)   /* FDNET.PVT file */
    {
        if (rawfdnet == -1)
        {
            sprintf(rawname, "%s\\fdnet.pvt", cfg.usr.fdnodelist);
            if ((rawfdnet =
                 sopen(rawname, O_RDONLY | O_TEXT, SH_DENYNO)) == -1)
            {
                sprintf(msg, "Can't open %s!", rawname);
                Message(msg, -1, 0, YES);
                mem_free(current);
                return -1;
            }
        }
        thisraw = rawfdnet;
        offset &= 0x0FFFFFFFL;
    }
    else if (offset & 0x20000000L)
    {
        if (rawpoint == -1)
        {
            sprintf(rawname, "%s\\fdpoint.pvt", cfg.usr.fdnodelist);
            if ((rawpoint =
                 sopen(rawname, O_RDONLY | O_TEXT, SH_DENYNO)) == -1)
            {
                sprintf(msg, "Can't open %s!", rawname);
                Message(msg, -1, 0, YES);
                mem_free(current);
                return -1;
            }
        }
        thisraw = rawpoint;
        offset &= 0x0FFFFFFFL;
    }
    else if (offset & 0x01000000L)
    {
        private = 1;
        offset &= 0x00FFFFFFL;
    }
    else
    {
        if (rawnode == -1)
        {
            sprintf(rawname, "%s", nodelistname);
            if ((rawnode =
                 sopen(rawname, O_RDONLY | O_TEXT, SH_DENYNO)) == -1)
            {
                sprintf(msg, "Can't open %s!", rawname);
                Message(msg, -1, 0, YES);
                mem_free(current);
                return -1;
            }
        }
        thisraw = rawnode;
    }


    if (!private)               /* Not a private nodelist (.FDA) entry,
                                   search raw list */
    {
        memset(temp, '\0', sizeof(temp));
        lseek(thisraw, offset, SEEK_SET);

        if (read(thisraw, &temp, 250) == -1)
        {
            Message("Error reading raw nodelist!", -1, 0, YES);
            mem_free(current);
            return -1;
        }

        // NULL terminate, and terminate string at end of line (prevent
        // next
        // line to be used as part of this entry.

        temp[250] = '\0';
        if ((charptr = strchr(temp, '\r')) != NULL)
            *charptr = '\0';
        if ((charptr = strchr(temp, '\n')) != NULL)
            *charptr = '\0';


        bbsname = strtok(temp, ",");
        if (bbsname != NULL)
        {
            strncpy(current->system, bbsname, 29);
            current->system[29] = '\0';
            strip_under(current->system);
        }

        sysopname = strtok(NULL, ","); /* city name */
        {
            strncpy(current->location, sysopname, 29);
            current->location[29] = '\0';
            strip_under(current->location);
        }

        sysopname = strtok(NULL, ",");
        if (sysopname != NULL)
        {
            strncpy(current->name, sysopname, 39);
            current->name[39] = '\0';
            strip_under(current->name);
        }

        sysopname = strtok(NULL, ","); // Phone
        if (sysopname != NULL)
        {
            strncpy(current->phone, sysopname, 19);
            current->phone[19] = '\0';
        }

        sysopname = strtok(NULL, ","); // Baud
        if (sysopname != NULL)
        {
            current->baud = atol(sysopname);
        }

        sysopname = strtok(NULL, "\n\r"); // Flags
        if (sysopname != NULL)
        {
            strncpy(current->flags, sysopname, 39);
            current->flags[39] = '\0';
        }

        current->next = NULL;   /* Just to be sure, clean link */
    }
    else                        /* Entry in .FDA file */
    {
        sprintf(rawname, "%s\\fdnode.fda", cfg.usr.fdnodelist);
        if ((fdafile =
             sopen(rawname, O_RDONLY | O_BINARY, SH_DENYNO)) == -1)
        {
            sprintf(msg, "Can't open %s!", rawname);
            Message(msg, -1, 0, YES);
            mem_free(current);
            return -1;
        }

        if (lseek
            (fdafile, (long)((long)offset * (long)sizeof(FDANODE)),
             SEEK_SET) != (long)((long)offset * (long)sizeof(FDANODE)))
            Message("Error seeking FDA file!", -1, 0, YES);

        if (read(fdafile, &fdanode, sizeof(FDANODE)) != sizeof(FDANODE))
        {
            sprintf(msg, "Can't read %s!", rawname);
            Message(msg, -1, 0, YES);
            mem_free(current);
            close(fdafile);
            return -1;
        }

        close(fdafile);

        strncpy(current->system, fdanode.Name + 1, 29);
        current->system[29] = '\0';
        strip_under(current->system);

        strncpy(current->name, fdanode.User + 1, 36);
        current->name[36] = '\0';
        strip_under(current->name);

        strncpy(current->location, fdanode.Location + 1, 28);
        current->location[29] = '\0';
        strip_under(current->name);

        strncpy(current->phone, fdanode.Telephone + 1, 19);
        current->phone[19] = '\0';


        for (i = 0; i < MAXBAUDFLAGS; i++)
        {
            if (fdanode.MaxBaud == baudflags[i].contents)
            {
                current->baud = baudflags[i].baud;
                break;
            }
        }

        for (i = 0; i < MAXCAPSTRINGS; i++)
        {
            if (fdanode.Capability & capstrings[i].value)
            {
                if (current->flags[0] != '\0')
                    strcat(current->flags, ",");
                if (strlen(current->flags) < 33)
                    strcat(current->flags, capstrings[i].flag);
            }
        }
    }

    if (first == NULL)          /* first element found */
        first = current;
    else
        last->next = current;   /* add to list */

    last = current;             /* remember last */

    return 0;
}


/* Convert underscores to spaces */

void near strip_under(char *start)
{

    while (*start)
    {
        if (*start == '_')
            *start = ' ';
        start++;
    }

}


int near addr_comp(FDNENTRY * this)
{

    if (swaphilo(this->zone) != matchaddress.zone)
        return (int)(swaphilo(this->zone) - matchaddress.zone);

    if (swaphilo(this->net) != matchaddress.net)
        return (int)(swaphilo(this->net) - matchaddress.net);

    if (swaphilo(this->node) != matchaddress.node)
        return (int)(swaphilo(this->node) - matchaddress.node);

    return (int)(swaphilo(this->point) - matchaddress.point);

}


int near two_addr_comp(FDNENTRY * this, FDNENTRY * that)
{

    if (this->zone != that->zone)
        return (int)(swaphilo(this->zone) - swaphilo(that->zone));

    if (this->net != that->net)
        return (int)(swaphilo(this->net) - swaphilo(that->net));

    if (this->node != that->node)
        return (int)(swaphilo(this->node) - swaphilo(that->node));

    return (int)(swaphilo(this->point) - swaphilo(that->point));

}


static void near reset(void)
{

    rawnode = -1;
    rawpoint = -1;
    rawfdnet = -1;

    fdidx = 0;
    memset(matchname, '\0', sizeof(matchname));
    namelen = 0;
    memset(&matchaddress, '\0', sizeof(NETADDR));

    first = NULL;

}
