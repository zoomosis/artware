#include "includes.h"


#define TAGGED    0x01
#define PERSMSG   0x02
#define FROMMSG   0x04

#define TAG    0x01
#define UNTAG  0x02

int PrivateFillStruct(AREA * area, dword curmsgno);

#ifdef __FLAT__

#ifdef __OS2__
unsigned long coreleft(void);
#endif

#define farfree(a) free(a)
#endif

#if defined(__WATCOMC__)

#ifndef __FLAT__
#define farfree hfree
#define farcalloc halloc
#endif

#endif


/* Internal structure for API_SDM, abused here to get info... */

#pragma pack(__push, 1)

struct _sdmdata
{
    byte base[80];

    unsigned *msgnum;           /* has to be of type 'int' for qksort() fn 
                                 */
    word msgnum_len;

    dword hwm;
    word hwm_chgd;

    word msgs_open;
};

#pragma pack(__pop)

typedef struct
{

    dword uid;
    char *line;
    char status;

} HDRLIST;



/* Info is stored in a "index" of the structures below */

#ifndef __FLAT__
dword huge *uidlist = NULL;
#else
dword *uidlist = NULL;
#endif

HDRLIST *screennow = NULL;

static char wide_display = 0;

int oldstart = -1, oldcur = -1;

int fillstruct(MSGA * areahandle, AREA * area, dword current);
void readheader(unsigned int num, MSGA * areahandle, AREA * area,
                HDRLIST * thisone);
void checkfilled(int start, int rows, MSGA * areahandle, AREA * area);
void showthem(int start, int rows, int curpos);
void print_msg(MSGA * areahandle, AREA * area, long msgno, FILE * outfile);
dword process_msgs(MSGA * areahandle, AREA * area, int curpos,
                   int operation);
void get_range(char dowhat, int maxpos, AREA * area);
void tagall(char dowhat, int maxpos, AREA * area);
void freescreen(HDRLIST * screennow);
void details(AREA * area, MSGA * areahandle, int num, int start);
void CheckInternet(MIS * mis, MSGA * areahandle, dword no);



dword MsgList(MSGA * areahandle, AREA * area, dword current, char wide)
{
    dword lastmsg, marker;
    BOX *listbox;
    int maxlines = maxy, rows = maxlines - 2;
    int start = 0, curpos = 0, maxpos;
    char movedown = 1;
    char temp[120];

    screennow = NULL;

    wide_display = wide;        /* 0 == normal, 1 == wide subject */

  start:

    oldstart = oldcur = -1;

    lastmsg = MsgMsgnToUid(areahandle, current); /* Set mark to return
                                                    to.. */

    maxpos = (int)area->nomsgs - 1;

    curpos = fillstruct(areahandle, area, lastmsg); /* Make "index" to use 
                                                     */


    if (maxpos < rows - 1)
    {
        rows = maxpos + 1;
        start = 0;
    }
    else
    {
        start = curpos - (rows / 2);
        if (start < 0)
            start = 0;
        if (start + rows > maxpos)
            start = maxpos - rows + 1;
    }

    listbox =
        initbox(0, 0, rows + 1, maxx - 1, cfg.col[Casframe],
                cfg.col[Castext], SINGLE, NO, -1);
    clsw(cfg.col[Castext]);

    drawbox(listbox);
    sprintf(temp, " Area: %s ", area->tag);
    titlewin(listbox, TCENTER, temp, 0);
    delbox(listbox);


    while (curpos != -1)
    {
        checkfilled(start, rows, areahandle, area);

        showthem(start, rows, curpos);

        switch (get_idle_key(1, LISTSCOPE))
        {
        case cLISTexit:        /* ALT-X */

            stuffkey(cREADexit);
            stuffkey(cLISTabort);
            break;

        case cLISTswitch:      /* ALT - S */

            oldstart = -1;
            freescreen(screennow);
            screennow = NULL;
            wide_display = (wide_display == 0) ? 1 : 0;
            break;

        case cLISTwrite:       /* ALT - W */

            PrintMessage(NULL, area, areahandle, 0, 1, PRINTALL);
            break;

        case cLISTprint:       /* ALT - P */

            PrintMessage(NULL, area, areahandle, 1, 1, PRINTALL);
            break;

        case cLISTdelete:      /* ALT - D */

            if (area->mlist->active == 0)
            {
                Message("No messages are currently marked!", -1, 0, YES);
                break;
            }

            if (cfg.usr.status & CONFIRMDELETE)
            {
                if (!confirm("Really delete all tagged messages? (y/N)"))
                    break;
            }

            oldstart = -1;
            marker = uidlist[curpos];
            freescreen(screennow);
            screennow = NULL;
            farfree(uidlist);
            uidlist = NULL;

            DeleteMarked(area, areahandle);
            ScanArea(area, areahandle, 0);
            if ((current =
                 MsgUidToMsgn(areahandle, marker, UID_NEXT)) == 0)
                current = MsgUidToMsgn(areahandle, marker, UID_PREV);
            if (area->nomsgs == 0)
                return 0;

            goto start;

        case cLISTcopy:        /* ALT - C */

            if (area->mlist->active == 0)
            {
                Message("No messages are currently marked!", -1, 0, YES);
                break;
            }

            oldstart = -1;
            freescreen(screennow);
            screennow = NULL;

            if ((current =
                 process_msgs(areahandle, area, curpos, COPY)) == 0)
            {
                // Don't farfree(uidlist) here! Is already done!
                return 0;
            }
            else if (current == -1)
                break;
            goto start;

        case cLISTmove:        /* ALT - M */

            if (area->mlist->active == 0)
            {
                Message("No messages are currently marked!", -1, 0, YES);
                break;
            }

            oldstart = -1;
            marker = uidlist[curpos];
            freescreen(screennow);
            screennow = NULL;

            if ((current =
                 process_msgs(areahandle, area, curpos, MOVE)) == 0)
            {
                // Don't farfree(uidlist) here! Is already done!
                return 0;
            }
            else if (current == -1)
                break;
            if ((current =
                 MsgUidToMsgn(areahandle, marker, UID_NEXT)) == 0)
                current = MsgUidToMsgn(areahandle, marker, UID_PREV);

            goto start;

        case cLISThelp:        /* F1 */

            show_help(2);
            break;

        case cLISTup:          /* up */

            if (curpos)
                curpos--;
            if (curpos < start)
                start--;
            movedown = 0;

            break;

        case cLISTdown:        /* down */

            if (curpos < maxpos)
                curpos++;
            if (curpos > start + rows - 1)
                start++;
            if (start + rows > maxpos)
                start = maxpos - rows + 1;
            movedown = 1;

            break;

        case cLISThome:        /* home */

            curpos = 0;
            start = 0;
            movedown = 1;

            break;

        case cLISTend:         /* end */

            curpos = maxpos;
            start = maxpos - rows + 1;
            movedown = 0;
            break;

        case cLISTpageup:      /* page up */

            curpos = start = start - rows;
            if (curpos < 0)
                curpos = start = 0;
            movedown = 0;

            break;

        case cLISTpagedown:    /* page down */

            curpos = start = start + rows;

            if (curpos > maxpos)
                curpos = maxpos;
            if (start + rows > maxpos)
                start = maxpos - rows + 1;
            movedown = 1;

            break;


        case cLISTabort:       // Esc

            curpos = -1;
            break;

        case cLISTdetails:     // TAB
            details(area, areahandle, curpos, start);
            break;

        case cLISTshell:       /* ALT - J */
        case 106:              /* J */
        case 74:

            shell_to_DOS();
            break;

        case cLISTreadmsg:     /* <CR> */

            lastmsg = uidlist[curpos];
            curpos = -1;
            break;

        case cLISTtag:         /* Space */

            screennow[curpos - start].status ^= TAGGED;
            if (screennow[curpos - start].status & TAGGED)
                AddMarkList(area->mlist, uidlist[curpos]);
            else
                RemoveMarkList(area->mlist, uidlist[curpos]);

            if (movedown == 1)
                stuffkey(cLISTdown);
            else
                stuffkey(cLISTup);
            break;

        case cLISTtagrange:    /* + */

            oldstart = -1;
            get_range(TAG, maxpos, area);
            freescreen(screennow);
            screennow = NULL;

            break;

        case cLISTtagall:

            oldstart = -1;
            tagall(TAG, maxpos, area);
            freescreen(screennow);
            screennow = NULL;

            break;

        case cLISTuntagall:

            oldstart = -1;
            tagall(UNTAG, maxpos, area);
            freescreen(screennow);
            screennow = NULL;

            break;

        case cLISTuntagrange:  /* - */

            oldstart = -1;
            get_range(UNTAG, maxpos, area);
            freescreen(screennow);
            screennow = NULL;

            break;

        }                       /* switch */

    }

    freescreen(screennow);
    farfree(uidlist);
    uidlist = NULL;

    if ((current = MsgUidToMsgn(areahandle, lastmsg, UID_NEXT)) == 0)
        current = MsgUidToMsgn(areahandle, lastmsg, UID_PREV);

    return current;

}


/* ------------------------------------------------- */


void readheader(unsigned num, MSGA * areahandle, AREA * area,
                HDRLIST * thisone)
{
    MSGH *msghandle;
    MIS mis;
    dword no;
    int l;
    char temp[MAX_SCREEN_WIDTH], temp2[80], *tilde;

    if (BePrivate(area))
        no = MsgUidToMsgn(areahandle, uidlist[num], UID_EXACT);
    else
        no = (!
              ((area->base & MSGTYPE_HMB)
               || (area->base & MSGTYPE_SQUISH))) ? uidlist[num] : num + 1;

    thisone->line = mem_malloc(maxx + 3); // 1 for trailing '\0', 2 for
                                          // ~xxx~

    if ((msghandle = MsgOpenMsg(areahandle, MOPEN_RDHDR, no)) == NULL)
    {
        memset(thisone->line, ' ', maxx);
        sprintf(temp, " ** Cannot open message (%5lu) **  ", no);
        memcpy(thisone->line, temp, strlen(temp));
        thisone->line[maxx - 2] = '\0';
        showerror();
    }
    else
    {
        if (MsgReadMsg(msghandle, &mis, 0L, 0L, NULL, 0L, NULL) == -1)
            Message("Error reading message!", -1, 0, 1);

        if (MsgCloseMsg(msghandle) == -1)
            Message("Error closing message!", -1, 0, 1);

        // Let's check for internet addresses, read body to get the TO:
        // line
        // from it if it's addressed to the gate.

        if (area->type == MAIL || area->type == NETMAIL)
            CheckInternet(&mis, areahandle, no);

        // Fix up empty subjects due to attaches/requests

        if (mis.subj[0] == '\0' && (mis.attr1 & (aFRQ | aFILE)))
        {
            SumAttachesRequests(&mis, mis.subj, 99, SARboth);
        }

        sprintf(temp2, "%%-%d.%ds", MAX_SCREEN_WIDTH - 2,
                MAX_SCREEN_WIDTH - 2);
        sprintf(temp, temp2, mis.subj);


        // Check for personal message
        for (l = 0; (cfg.usr.name[l].name[0] != '\0') && (l < 10); l++)
        {
            if (strcmpi(mis.to, cfg.usr.name[l].name) == 0)
                thisone->status |= PERSMSG;
            else if (strcmpi(mis.from, cfg.usr.name[l].name) == 0)
                thisone->status |= FROMMSG;
        }

        // Are we even allowed to see this message?

        if (BePrivate(area) &&
            (mis.attr1 & aPRIVATE) &&
            !(thisone->status & (PERSMSG | FROMMSG)))
        {
            strcpy(mis.from, "timEd");
            strcpy(mis.to, "Reader");
            strcpy(temp,
                   "Personal message, not to/from you                                 ");
        }

        // Replace #255 with space. Not nice, but it messes up biprint!

        while ((tilde = strchr(temp, 0xFF)) != NULL)
            *tilde = ' ';

        while ((tilde = strchr(mis.from, 0xFF)) != NULL)
            *tilde = ' ';

        while ((tilde = strchr(mis.to, 0xFF)) != NULL)
            *tilde = ' ';

        if (wide_display)
        {
            temp[maxx - 37] = '\0';
            if (thisone->status & PERSMSG)
                sprintf(thisone->line, "\xFF %5ld %-20.20s %s %s\xFF", no,
                        mis.from, temp, MakeT(mis.msgwritten, DATE_LIST));
            else if (thisone->status & FROMMSG)
                sprintf(thisone->line, " %5ld \xFF%-20.20s\xFF %s %s", no,
                        mis.from, temp, MakeT(mis.msgwritten, DATE_LIST));
            else
                sprintf(thisone->line, " %5ld %-20.20s %s %s", no,
                        mis.from, temp, MakeT(mis.msgwritten, DATE_LIST));
        }
        else
        {
            temp[maxx - 58] = '\0';
            if (thisone->status & PERSMSG)
                sprintf(thisone->line,
                        " %5ld %-20.20s \xFF%-20.20s\xFF %s %s", no,
                        mis.from, mis.to, temp, MakeT(mis.msgwritten,
                                                      DATE_LIST));
            else if (thisone->status & FROMMSG)
                sprintf(thisone->line,
                        " %5ld \xFF%-20.20s\xFF %-20.20s %s %s", no,
                        mis.from, mis.to, temp, MakeT(mis.msgwritten,
                                                      DATE_LIST));
            else
                sprintf(thisone->line, " %5ld %-20.20s %-20.20s %s %s", no,
                        mis.from, mis.to, temp, MakeT(mis.msgwritten,
                                                      DATE_LIST));
        }

        if (IsMarked(area->mlist, uidlist[num]))
            thisone->status |= TAGGED;

        FreeMIS(&mis);
    }
}

/* Fill the "index" of msgs, w/ pointers to 'header string' to be displayed */

int fillstruct(MSGA * areahandle, AREA * area, dword curmsgno)
{
    struct _sdmdata *dataptr = (struct _sdmdata *)areahandle->apidata;
    unsigned *msglijst = (unsigned *)dataptr->msgnum;
    int l, curpos = 0;
    BOX *wait;
    dword realno;


    if (BePrivate(area))
        return PrivateFillStruct(area, curmsgno);

#ifndef __FLAT__
    uidlist =
        (dword huge *) farcalloc((unsigned long)MsgGetNumMsg(areahandle) +
                                 1, (unsigned long)sizeof(UMSGID));
#else
    uidlist =
        calloc((size_t) MsgGetNumMsg(areahandle) + 1, sizeof(UMSGID));
#endif


    if (!(area->base & MSGTYPE_JAM))
    {
        for (l = 0; l < MsgGetNumMsg(areahandle); l++)
        {
            uidlist[l] =
                (area->
                 base & MSGTYPE_SDM) ? *msglijst++ :
                MsgMsgnToUid(areahandle, l + 1);
            if (uidlist[l] == curmsgno)
                curpos = l;
        }
    }
    else if (area->base & MSGTYPE_JAM)
    {
        wait =
            initbox(13, 33, 15, 47, cfg.col[Cpopframe], cfg.col[Cpoptext],
                    SINGLE, YES, ' ');
        drawbox(wait);
        boxwrite(wait, 0, 1, "Wait...");

        realno = MsgGetLowMsg(areahandle);

        /* 'contigious' msg numbers? So no deleted msgs in area? */
        /* Cause then it's very easy. Otherwise we have to check */
        /* which message number actually exist :-( */

        if (MsgGetNumMsg(areahandle) ==
            (MsgGetHighMsg(areahandle) - realno + 1))
        {
            for (l = 0; l < MsgGetNumMsg(areahandle); l++, realno++)
            {
                uidlist[l] = realno;
                if (uidlist[l] == curmsgno)
                    curpos = l;
            }
        }
        else
        {
            boxwrite(wait, 0, 1, "Scanning...");

            for (l = 0;
                 l < MsgGetNumMsg(areahandle)
                 && realno <= MsgGetHighMsg(areahandle); l++)
            {
                while ((MsgUidToMsgn(areahandle, realno, UID_EXACT) == 0)
                       && realno <= MsgGetHighMsg(areahandle))
                    realno++;
                uidlist[l] = realno;
                if (uidlist[l] == curmsgno)
                    curpos = l;
                realno++;
            }
        }

        delbox(wait);
    }


    return curpos;
}


// In areas where we respect the private bit, we already have a good
// list of all messages.. Easy to use, huh?

int PrivateFillStruct(AREA * area, dword curmsgno)
{
    int l, curpos = 0;

#ifndef __FLAT__
    uidlist =
        (dword huge *) farcalloc((unsigned long)area->nomsgs + 1,
                                 (unsigned long)sizeof(UMSGID));
#else
    uidlist = calloc((size_t) area->nomsgs + 1, sizeof(UMSGID));
#endif

    uidlist[0] = area->mayseelist->list[0];

    for (l = 1; l < area->nomsgs; l++)
    {
        uidlist[l] = NextMarked(area->mayseelist, uidlist[l - 1]);
        if (uidlist[l] == curmsgno)
            curpos = l;
    }

    return curpos;
}




/* Walk through msgs to be displayed, if header not read yet, do it now! */


void checkfilled(int start, int rows, MSGA * areahandle, AREA * area)
{

    unsigned int l, n, i;
    HDRLIST *newscreen;

    if (!uidlist)
        Message("NULL UIDlist in listmode!", -1, 254, YES);

    newscreen = mem_calloc(maxy - 2, sizeof(HDRLIST));

    for (n = start, l = 0; l < rows; l++, n++)
    {
        newscreen[l].uid = uidlist[n]; // Fill UID
        if (screennow)          // Lines already on screen (no need for
                                // reading hdr)?
        {
            for (i = 0; i < (maxy - 2); i++)
            {
                if (screennow[i].uid == newscreen[l].uid && screennow[i].line != NULL) // Yep! 
                                                                                       // Copy 
                                                                                       // info..
                {
                    newscreen[l].line = screennow[i].line;
                    newscreen[l].status = screennow[i].status;
                    screennow[i].line = NULL;
                    break;
                }
            }
            if (i == maxy - 2)
                readheader(n, areahandle, area, &newscreen[l]);
        }
        else
            readheader(n, areahandle, area, &newscreen[l]);
    }

    if (screennow)              // free memory taken by old screen.
        freescreen(screennow);

    screennow = newscreen;
}



/* Display the headers on the screen */


void showthem(int start, int rows, int curpos)
{
    unsigned int l, i;
    int doredraw;

    doredraw = (start == oldstart) ? 0 : 1;
    oldstart = start;

    for (l = start, i = 0; l < start + rows; i++, l++)
    {
        if (curpos == l)
        {
            MoveXY(2, l - start + 2);
            if ((doredraw) || (l == oldcur) || (curpos == l))
            {
                biprint(l - start + 1, 1, cfg.col[Cashigh],
                        cfg.col[Cashighaccent], screennow[i].line, 0xFF);

                if (screennow[i].status & TAGGED)
                    printc(l - start + 1, 1, cfg.col[Cashigh], 'þ');
            }
        }
        else
        {
            if ((doredraw) || (l == oldcur) || (curpos == l))
            {
                biprint(l - start + 1, 1, cfg.col[Castext],
                        cfg.col[Casspecial], screennow[i].line, 0xFF);

                if (screennow[i].status & TAGGED)
                    printc(l - start + 1, 1, cfg.col[Castext], 'þ');
            }
        }
    }

    oldcur = curpos;

}


/* Kill, Move, Copy, Print all tagged messages */

dword process_msgs(MSGA * areahandle, AREA * area, int curpos,
                   int operation)
{
    dword marker, current;
    AREA *toarea;
    MSGA *toareahandle;
    char temp[80];

    marker = uidlist[curpos];

    savescreen();
    toarea = SelectArea(cfg.first, 1, cfg.first);
    putscreen();

    if (toarea == NULL)
        return -1L;

    if (toarea == area)
    {
        Message("Can't mass move/copy to the same area!", -1, 0, YES);
        return -1L;
    }

    if ((toareahandle =
         MsgOpenArea(toarea->dir, MSGAREA_CRIFNEC, toarea->base)) == NULL)
    {
        sprintf(temp, "Can't open area (%-0.30s)!", toarea->dir);
        Message(temp, -1, 0, YES);
        showerror();
        return -1L;
    }

    CopyMarked(area, toarea, areahandle, toareahandle,
               (operation == MOVE) ? 1 : 0);

    ScanArea(toarea, toareahandle, 1);
    MsgCloseArea(toareahandle);

    farfree(uidlist);
    uidlist = NULL;

    ScanArea(area, areahandle, 0);
    if (area->nomsgs == 0)
        return 0;

    if ((current = MsgUidToMsgn(areahandle, marker, UID_NEXT)) == 0)
        current = MsgUidToMsgn(areahandle, marker, UID_PREV);

    return current;
}




void print_msg(MSGA * areahandle, AREA * area, long msgno, FILE * outfile)
{

    MMSG *curmsg;

    curmsg = GetFmtMsg(msgno, areahandle, area); /* Read msg from disk,
                                                    formatted */

    if (curmsg)
    {
        do_print(outfile, area, curmsg, PRINTALL);
        ReleaseMsg(curmsg, 1);
    }

}


void get_range(char dowhat, int maxpos, AREA * area)
{
    dword n;
    int ret = 1, action = 0;
    long begin, end;
    char nbegin[7], nend[7];
    BOX *numbox;
    unsigned curmsg;


    memset(nbegin, '\0', sizeof(nbegin));
    memset(nend, '\0', sizeof(nend));


    numbox =
        initbox(8, 21, 15, 47, cfg.col[Cpopframe], cfg.col[Cpoptext],
                DOUBLE, YES, ' ');
    drawbox(numbox);
    if (dowhat == TAG)
        boxwrite(numbox, 1, 2, "Tag:");
    else
        boxwrite(numbox, 1, 2, "Untag:");
    boxwrite(numbox, 3, 2, "Starting msg #");
    boxwrite(numbox, 4, 2, "Ending   msg #");

    while (action < 2)
    {
        switch (action)
        {
        case 0:

            while ((ret != ESC) && (ret != 0))
                ret =
                    getstring(12, 40, nbegin, 6, 6, "0123456789",
                              cfg.col[Centry], cfg.col[Cpoptext]);
            action = (ret == 0) ? 1 : 2;
            ret = 1;
            break;

        case 1:

            while ((ret != ESC) && (ret != 0) && (ret != UP)
                   && (ret != BTAB))
                ret =
                    getstring(13, 40, nend, 6, 6, "0123456789",
                              cfg.col[Centry], cfg.col[Cpoptext]);
            action = ((ret == UP) || (ret == BTAB)) ? 0 : 2;
            ret = 1;
            break;

        default:
            break;              /* nothing */
        }
    }

    delbox(numbox);

    if (ret == ESC)
        return;

    begin = atol(nbegin);
    end = atol(nend);

    for (curmsg = 0, n = 1; n <= maxpos + 1; curmsg++, n++)
    {
        if ((area->base & MSGTYPE_SDM) || (area->base & MSGTYPE_JAM))
        {
            if (uidlist[curmsg] > end)
                break;

            if ((uidlist[curmsg] >= begin) && (uidlist[curmsg] <= end))
            {
                if (dowhat == TAG)
                    AddMarkList(area->mlist, uidlist[curmsg]);
                else
                    RemoveMarkList(area->mlist, uidlist[curmsg]);
            }
        }
        else
        {
            if (n > end)
                break;

            if ((n >= begin) && (n <= end))
            {
                if (dowhat == TAG)
                    AddMarkList(area->mlist, uidlist[curmsg]);
                else
                    RemoveMarkList(area->mlist, uidlist[curmsg]);
            }
        }
    }

}


#ifdef __OS2__
unsigned long coreleft(void)
{
    return 1;
}
#endif


void tagall(char dowhat, int maxpos, AREA * area)
{
    dword n;
    unsigned curmsg;


    if (dowhat == TAG)
    {
        for (curmsg = 0, n = 1; n <= maxpos + 1; curmsg++, n++)
            AddMarkList(area->mlist, uidlist[curmsg]);
    }
    else                        // Release all marks
    {
        DumpMarkList(area->mlist);
        area->mlist = InitMarkList();
    }

}



void freescreen(HDRLIST * screennow)
{
    int i;

    for (i = 0; i < (maxy - 2); i++)
    {
        if (screennow[i].line)
            mem_free(screennow[i].line);
    }

    mem_free(screennow);

}

// Show details about a certain message.

void details(AREA * area, MSGA * areahandle, int num, int start)
{
    MMSG *thismsg;
    dword no;
    int pos;
    BOX *hdrbox;

    if (BePrivate(area))
        no = MsgUidToMsgn(areahandle, uidlist[num], UID_EXACT);
    else
        no = (!
              ((area->base & MSGTYPE_HMB)
               || (area->base & MSGTYPE_SQUISH))) ? uidlist[num] : num + 1;

    if ((thismsg = GetFmtMsg(no, areahandle, area)) == NULL)
        return;

    if (thismsg->mis.subj[0] == '\0'
        && (thismsg->mis.attr1 & (aFRQ | aFILE)))
    {
        SumAttachesRequests(&thismsg->mis, thismsg->mis.subj, 99, SARboth);
    }

    pos = num - start - 2;
    if (pos < 0)
        pos = 0;
    if (pos > (maxy - 6))
        pos = maxy - 6;

    hdrbox =
        initbox(pos, 0, pos + 5, 79, cfg.col[Cpopframe], cfg.col[Cpoptext],
                SINGLE, YES, ' ');
    drawbox(hdrbox);
    titlewin(hdrbox, TCENTER, " Detail view ", 0);

    vprint(pos + 1, 1, cfg.col[Cpoptext], " Date: %-31.31s %38.38s",
           MakeT(thismsg->mis.msgwritten, DATE_HDR),
           attr_to_txt(thismsg->mis.attr1, thismsg->mis.attr2));
    vprint(pos + 2, 1, cfg.col[Cpoptext], " From: %-42.42s %-28.28s",
           thismsg->mis.from, FormAddress(&thismsg->mis.origfido));
    vprint(pos + 3, 1, cfg.col[Cpoptext], " To  : %-42.42s %-28.28s",
           thismsg->mis.to,
           (area->type ==
            NETMAIL) ? FormAddress(&thismsg->mis.destfido) : "");
    vprint(pos + 4, 1, cfg.col[Cpoptext], " Subj: %-71.71s",
           thismsg->mis.subj);

    get_idle_key(1, GLOBALSCOPE);
    delbox(hdrbox);

    ReleaseMsg(thismsg, 1);

}

// ==============================================================

void CheckInternet(MIS * mis, MSGA * areahandle, dword no)
{
    MSGH *msghandle;
    char *body, *inet = NULL, *newline;
    dword bodylen;
    int i, len;

    if ((strcmpi(mis->to, "uucp") != 0) &&
        (strcmpi(mis->to, "postmaster") != 0) &&
        (strcmpi(mis->to, cfg.usr.uucpname) != 0))
        return;

    // If we get here, there _is_ an inet address.

    if ((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, no)) == NULL)
    {
        showerror();
        return;
    }

    bodylen = MsgGetTextLen(msghandle);

    if ((dword) bodylen == (dword) - 1L)
        bodylen = (dword) 0L;

    if (bodylen == 0)
    {
        MsgCloseMsg(msghandle);
        return;
    }

    if (bodylen > 511)
        bodylen = 511;

    body = mem_calloc(1, bodylen + 1);

    if (MsgReadMsg(msghandle, mis, 0L, bodylen, body, 0L, NULL) == -1)
        Message("Error reading message!", -1, 0, 1);

    body[bodylen] = '\0';

    if (MsgCloseMsg(msghandle) == -1)
        Message("Error closing message!", -1, 0, 1);

    if (strnicmp(body, "to:", 3) == 0)
        inet = body;
    else
    {
        newline = strchr(body, '\r');
        i = 0;
        while (newline && i < 4)
        {
            while (*newline == 0x0A || *newline == 0x0D
                   || *newline == 0x8D)
                newline++;      // skip the '\r' etc
            if (strnicmp(newline, "to:", 3) == 0)
            {
                inet = newline;
                break;
            }
            newline = strchr(newline, '\r');
            i++;
        }
    }

    if (inet)
    {
        inet += 3;
        while (*inet == ' ')
            inet++;
        len = strcspn(inet, " \t\r\n");
        memset(mis->to, '\0', 100);
        memcpy(mis->to, inet, len);
    }

    mem_free(body);

}

// ==============================================================
