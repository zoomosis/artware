#include "includes.h"

/* prototypes */

typedef struct _hdrlist
{

    MIS *mis;
    int hidden;
    struct _hdrlist *next;

} HDRLIST;

static HDRLIST *firsthdr, *last;

typedef struct _arealist
{

    AREA *areaptr;
    struct _arealist *next;

} AREALIST;

static AREALIST *firstarea, *lastarea;

char nowcc, nowxc, nowhc;

void check_element(char *charptr, MIS * header, AREA * area);
void write_one(MSGA * areahandle, AREA * area, MIS * curheader,
               char *msgbuf);
void get_bomb_list(char *charptr, MIS * header, AREA * area);
HDRLIST *get_copy(MIS * header);
void WriteCCs(MSGA * areahandle, AREA * area, RAWBLOCK ** first,
              MIS * header);
void WriteXCs(AREA * area, RAWBLOCK ** first, MIS * header);
void AddToArealist(char *charptr);
void FixEnd(RAWBLOCK ** first, AREA * area);
int which_aka(NETADDR * addr);
int MoreToCome(HDRLIST * thisone);


void check_cc(AREA * area, MSGA * areahandle, MIS * mis, RAWBLOCK ** first)
{

    char *start, *charptr, *nextnl, *nextel;
    char temp[256];
    char *end;
    size_t howmuch;

    if (*first == NULL)
        return;

    firsthdr = last = NULL;
    firstarea = lastarea = NULL;
    start = (*first)->txt;

    if (start == NULL)
        return;

    while ((nextnl = strchr(start, '\r')) != NULL) /* analyse line by
                                                      line.. */
    {
        memset(temp, '\0', sizeof(temp));
        howmuch = min(255, (size_t) (nextnl - start));
        strncpy(temp, start, howmuch); /* copy line (strtok messes it up) */
        end = temp + sizeof(temp);
        charptr = temp;
        start = nextnl + 1;

        while (*charptr == ' ' || *charptr == '\r') /* skip leading space */
            charptr++;

        nowcc = nowxc = nowhc = 0;

        if (strncmpi(charptr, "CC:", 3) == 0) /* CC: ? */
        {
            if (area->type == ECHOMAIL)
                break;
            nowcc = 1;
        }
        else if (strncmpi(charptr, "HC:", 3) == 0) /* HC: ? */
        {
            if (area->type == ECHOMAIL)
                break;
            nowcc = 1;
            nowhc = 1;
        }
        else
        {
            if (strncmpi(charptr, "XC:", 3) != 0) /* No CC:, return */
                break;
            nowxc = 1;
        }

        strcpy((*first)->txt, start);
        start = (*first)->txt;

        charptr = charptr + 3;  /* Skip CC: chars */

        while (*charptr == ' ') /* skip space again */
            charptr++;

        charptr = strtok(charptr, ","); /* get elements, separated by
                                           comma's */
        if (charptr != NULL)
            nextel = charptr + strlen(charptr) + 1;

        while (charptr)
        {
            while (*charptr == ' ')
                charptr++;
            if (nowcc)
            {
                if (*charptr == '<')
                    get_bomb_list(charptr + 1, mis, area);
                else
                    check_element(charptr, mis, area);
            }
            else
                AddToArealist(charptr);

            if (nextel <= end)
            {
                charptr = strtok(nextel, ",");
                if (charptr)
                    nextel = charptr + strlen(charptr) + 1;
            }
            else
                charptr = NULL;
        }

    }

    if (firsthdr)               // Only if we actually have any CC:'s to
                                // write!
        WriteCCs(areahandle, area, first, mis);

    if (firstarea)
        WriteXCs(area, first, mis);

    // mem_free memory taken by list of headers...

    while (firsthdr)
    {
        last = firsthdr->next;
        if (firsthdr->mis)
        {
            FreeMIS(firsthdr->mis);
            mem_free(firsthdr->mis);
        }

        mem_free(firsthdr);
        firsthdr = last;
    }

    while (firstarea)
    {
        lastarea = firstarea->next;
        mem_free(firstarea);
        firstarea = lastarea;
    }

}


/* -------------------------------------------------------------- */
/* Here we look at the element, try to expand it to a full header */
/* modified from the (passed) original header                     */
/* If we can't make it, we return NULL, else the created header   */
/* -------------------------------------------------------------- */


void check_element(char *el, MIS * mis, AREA * area)
{
    HDRLIST *thisheader;
    char *sep;
    char elcopy[80], *element;

    strcpy(elcopy, el);
    element = elcopy;


    thisheader = get_copy(mis);
    memset(thisheader->mis->to, '\0', sizeof(thisheader->mis->to));

    while (*element == ' ')
        element++;

    while (element[strlen(element) - 1] == ' ')
        element[strlen(element) - 1] = '\0';

    // Is this a 'hidden CC:? */

    if (nowhc == 1)
        thisheader->hidden = 1;

    if (*element == '#')
    {
        element++;
        thisheader->hidden = 1;
    }

    /* we check if there is an explicit address, using a '#' */

    if ((sep = strchr(element, '#')) != NULL)
    {
        strncpy(thisheader->mis->to, element, (size_t) (sep - element));
        address_expand(sep + 1, &thisheader->mis->destfido,
                       which_aka(&mis->origfido));
        return;
    }

    strcpy(thisheader->mis->to, element);

    /* we start to check if the element is a macro.. */

    if (check_alias(thisheader->mis)) /* return 0 if no luck, # of
                                         elements else */
        return;

    if ((strchr(element, '@') != NULL || strchr(element, '!') != NULL) &&
        cfg.usr.uucpname[0] != '\0')
    {
        strcpy(thisheader->mis->destinter, element);
        thisheader->mis->destfido = cfg.usr.uucpaddress;
        strcpy(thisheader->mis->to, cfg.usr.uucpname);
        return;
    }

    if (area->type == NETMAIL)
    {
        sprintf(msg, "Checking nodelist for %s..", thisheader->mis->to);
        statusbar(msg);
        if (check_node(thisheader->mis, which_aka(&mis->origfido), NOPROMPT) != 1) /* 0 
                                                                                      == 
                                                                                      not 
                                                                                      found, 
                                                                                      -1 
                                                                                      = 
                                                                                      abort 
                                                                                    */
        {
            sprintf(msg, "%s not found in nodelist!", thisheader->mis->to);
            Message(msg, -1, 0, YES);
            FreeMIS(thisheader->mis);
            mem_free(thisheader->mis);
            thisheader->mis = NULL;
            return;
        }
    }

}


// ===================================================================

void WriteCCs(MSGA * areahandle, AREA * area, RAWBLOCK ** first, MIS * mis)
{
    RAWBLOCK *ccinfo = *first, *thismsgstart;
    HDRLIST *thisone;
    char temp[MAX_SCREEN_WIDTH];
    char info[] = "* Carbon copies sent to: ";
    char *kludges;
    int totalccs = 0;


    // Reset info for first block (CC: lines removed!)
    (*first)->curlen = strlen((*first)->txt);
    (*first)->curend = (*first)->txt + (*first)->curlen;

    // Expand CC: names and build a textblock out of it

    if (MoreToCome(firsthdr))   // Are there actually any unhidden names?
    {
        ccinfo = InitRawblock(1024, 1024, 64000u);

        sprintf(temp, "* Original message addressed to: %s.",
                mis->destinter[0] == '\0' ? mis->to : mis->destinter);

        if (area->type == NETMAIL && mis->destinter[0] == '\0')
        {
            strcat(temp, " (");
            strcat(temp, FormAddress(&mis->destfido));
            strcat(temp, ").");
        }
        strcat(temp, "\r");

        AddToBlock(ccinfo, temp, -1);
        AddToBlock(ccinfo, info, -1);

        for (thisone = firsthdr; thisone; thisone = thisone->next)
        {
            if (thisone->mis)
            {
                totalccs++;
                if ((cfg.usr.status & CCVERBOSE) && (thisone->hidden == 0))
                {
                    if (area->type == NETMAIL || area->type == MAIL)
                    {
                        if (thisone->mis->destinter[0] != '\0')
                            sprintf(temp, "%s", thisone->mis->destinter);
                        else
                            sprintf(temp, "%s (%s)", thisone->mis->to,
                                    FormAddress(&thisone->mis->destfido));
                    }
                    else
                        sprintf(temp, "%s", thisone->mis->to);

                    AddToBlock(ccinfo, temp, -1);
                    if (MoreToCome(thisone->next))
                        AddToBlock(ccinfo, ", ", -1);
                }
            }
        }

        if (cfg.usr.status & CCVERBOSE)
            AddToBlock(ccinfo, ".\r", -1);
        else
        {
            sprintf(temp, "%d other recipient%s.\r", totalccs,
                    totalccs > 1 ? "s" : "");
            AddToBlock(ccinfo, temp, -1);
        }

        // Prepend expanded name block to text
        ccinfo->next = *first;
        *first = ccinfo;

    }                           // if(MoreToCome())

    // ============ First lines fiddling finished.. pff.. ===========

    // Write all messages
    for (thisone = firsthdr; thisone; thisone = thisone->next)
    {
        if (thisone->mis == NULL)
            continue;

        sprintf(temp, "Generating CC: message for %s", thisone->mis->to);
        statusbar(temp);

        thismsgstart = ccinfo;

//     if(thisone->usenet)   // Add extra TO: line in front (Usenet)
//       {
//       thismsgstart = InitRawblock(100, 100, 1024);
//       AddToBlock(thismsgstart, "TO: ", -1);
//       AddToBlock(thismsgstart, thisone->usenet, -1);
//       AddToBlock(thismsgstart, "\r\r", -1);
//       thismsgstart->next = ccinfo;
//       }

        /* First we try to match AKA's... */

        matchaka(thisone->mis);

        kludges = MakeKludge(NULL, thisone->mis, 1); /* Last parm == 1,
                                                        always netmail */

        zonegate(thisone->mis, kludges, 1, area->base);

        // Add kill attribute for cc:'s
        thisone->mis->attr1 |= aKILL;

        if (SaveMessage(areahandle,
                        area,
                        thisone->mis, thismsgstart, kludges, 0L, 1) == 0)
            add_tosslog(area, areahandle);

        if (kludges)
            mem_free(kludges);
        if (thismsgstart->next == ccinfo) // Free usenet address mem
        {
            thismsgstart->next = NULL;
            FreeBlocks(thismsgstart);
        }
    }

}


/* get a list from disk, with names to CC: the message to.. */

void get_bomb_list(char *charptr, MIS * mis, AREA * area)
{
    FILE *ccfile;
    char temp[256];


    while (*charptr == ' ')
        charptr++;

    while (charptr[strlen(charptr) - 1] == ' ')
        charptr[strlen(charptr) - 1] = '\0';

    if ((ccfile = fopen(charptr, "rt")) == NULL)
    {
        sprintf(msg, "Can't open %s!", charptr);
        Message(msg, -1, 0, YES);
        return;
    }

    while (fgets(temp, 256, ccfile))
    {
        if (temp[0] == '\0' || temp[0] == '\n' || temp[0] == ';')
            continue;

        charptr = temp;

        while (*charptr == ' ')
            charptr++;

        if (charptr[strlen(charptr) - 1] == '\n')
            charptr[strlen(charptr) - 1] = '\0';

        while (charptr[strlen(charptr) - 1] == ' ')
            charptr[strlen(charptr) - 1] = '\0';

        if (nowcc)
            check_element(charptr, mis, area);
        else
            AddToArealist(charptr);
    }

    fclose(ccfile);

}


HDRLIST *get_copy(MIS * mis)
{
    MIS *newmis;
    HDRLIST *thisone;

    thisone = mem_calloc(1, sizeof(HDRLIST));
    if (!firsthdr)
        firsthdr = thisone;
    else
        last->next = thisone;
    last = thisone;

    newmis = thisone->mis = mem_calloc(1, sizeof(MIS));

    strcpy(newmis->from, mis->from);
    strcpy(newmis->to, mis->to);

    newmis->origfido = mis->origfido;

    strcpy(newmis->subj, mis->subj);

    newmis->attr1 = mis->attr1;
    newmis->attr2 = mis->attr2;

    newmis->msgwritten = JAMsysTime(NULL);

    CopyStringList(mis->attached, &newmis->attached);
    CopyStringList(mis->requested, &newmis->requested);

    return thisone;
}

// ============================================

void AddToArealist(char *charptr)
{
    AREALIST *thisarea;

    thisarea = mem_calloc(1, sizeof(AREALIST));

    if ((thisarea->areaptr = FindArea(charptr)) == NULL)
    {
        sprintf(msg, "Can't find area: %s!", charptr);
        Message(msg, -1, 0, YES);
        mem_free(thisarea);
        return;
    }

    if (!firstarea)
        firstarea = thisarea;
    else
        lastarea->next = thisarea;

    lastarea = thisarea;

}


// -----------------------------------------------------

void WriteXCs(AREA * area, RAWBLOCK ** first, MIS * mis)
{
    RAWBLOCK *ccinfo = NULL;
    AREALIST *thisone;
    char temp[MAX_SCREEN_WIDTH];
    char info[] = "* Crossposted in: ";
    char *kludges;
    MSGA *thisareahandle;
    HDRLIST *ourhdr = get_copy(mis);

    // Expand XC: names and build a textblock out of it

    ccinfo = InitRawblock(1024, 1024, 32000u);
    sprintf(temp, "* Original message posted in: %s.\r", area->tag);

    AddToBlock(ccinfo, temp, -1);
    AddToBlock(ccinfo, info, -1);

    for (thisone = firstarea; thisone; thisone = thisone->next)
    {
        sprintf(temp, "%s", thisone->areaptr->tag);

        AddToBlock(ccinfo, temp, -1);
        if (thisone->next)
            AddToBlock(ccinfo, ", ", -1);
    }

    AddToBlock(ccinfo, ".\r", -1);

    // Reset info for first block (CC: lines removed!)
    (*first)->curlen = strlen((*first)->txt);
    (*first)->curend = (*first)->txt + (*first)->curlen;

    // Prepend expanded name block to text
    ccinfo->next = *first;
    *first = ccinfo;

    // Write all messages
    for (thisone = firstarea; thisone; thisone = thisone->next)
    {
        if (thisone->areaptr == area) // Crossposting in the same area?
            continue;

        sprintf(temp, "Generating crosspost message in %s",
                thisone->areaptr->tag);
        statusbar(temp);

        // !!!!!!

        if (thisone->areaptr->type == NETMAIL)
        {
            sprintf(msg, "Checking nodelist for %s..", ourhdr->mis->to);
            statusbar(msg);
            if (check_node(ourhdr->mis, which_aka(&ourhdr->mis->origfido), NOPROMPT) != 1) /* 0 
                                                                                              == 
                                                                                              not 
                                                                                              found, 
                                                                                              -1 
                                                                                              = 
                                                                                              abort 
                                                                                            */
            {
                sprintf(msg, "%s not found in nodelist!", ourhdr->mis->to);
                Message(msg, -1, 0, YES);
            }
        }


//     thismsgstart = ccinfo;

        if ((thisareahandle =
             MsgOpenArea(thisone->areaptr->dir, MSGAREA_CRIFNEC,
                         thisone->areaptr->base)) == NULL)
        {
            sprintf(msg, "Error opening area: %s!", thisone->areaptr->tag);
            Message(msg, -1, 0, YES);
            showerror();
            continue;
        }

        get_custom_info(thisone->areaptr);

        ourhdr->mis->origfido = cfg.usr.address[custom.aka];

        kludges = MakeKludge(NULL, ourhdr->mis, 0); /* Last parm == 1,
                                                       always netmail */

        FixEnd(first, thisone->areaptr);

        if (SaveMessage(thisareahandle,
                        thisone->areaptr,
                        ourhdr->mis, *first, kludges, 0L, 1) == 0)
            add_tosslog(thisone->areaptr, thisareahandle);

        ScanArea(thisone->areaptr, thisareahandle, 1);

        MsgCloseArea(thisareahandle);

        if (kludges)
            mem_free(kludges);
    }

    get_custom_info(area);

    FixEnd(first, area);

}

// --------------------------------------------------

AREA *FindArea(char *tag)
{
    AREA *thisone = cfg.first;

    while (thisone)
    {
        if (!strcmpi(thisone->tag, tag))
            return thisone;
        thisone = thisone->next;
    }

    return NULL;
}


// Strip off Origin, add a new one if necessary, or a tearline in netmail (if nec..)

void FixEnd(RAWBLOCK ** first, AREA * area)
{
    RAWBLOCK *lastblock;
    char *startorig;
    char temp[MAX_SCREEN_WIDTH];

    lastblock = JoinLastBlocks(*first, 1024);

    if (!lastblock)
        return;

    lastblock->maxlen += 100;   // please keep all this in one block!

    if (clean_origin(lastblock->txt, &startorig) == 0)
    {
        if (startorig != NULL)
            *startorig = '\0';
    }

    lastblock->curlen = strlen(lastblock->txt);
    lastblock->curend = lastblock->txt + lastblock->curlen;

    if (area->type == ECHOMAIL)
        AddToBlock(lastblock, make_origin(custom.aka), -1);
    else if (area->type == NETMAIL)
    {
        if (cfg.usr.status & NETTEAR)
        {
            sprintf(temp, "\r--- %s",
                    (cfg.usr.status & EMPTYTEAR) ? "" : myname);
            AddToBlock(lastblock, temp, -1);
        }
    }

}

// =====================================================
// Get index number of a certain AKA

int which_aka(NETADDR * addr)
{
    int i;

    for (i = 0; i < tMAXAKAS; i++)
    {
        if (addrcmp(addr, &cfg.usr.address[i]) == 0)
            return i;
    }

    return 0;
}

// =====================================================
//
// See if any more filled headers that are not hidden
// remain in the list of CC:'s...
//
// =====================================================

int MoreToCome(HDRLIST * thisone)
{

    while (thisone)
    {
        if (thisone->mis && (thisone->hidden == 0))
            return 1;
        thisone = thisone->next;
    }

    return 0;
}
