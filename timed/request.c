#include "includes.h"


char seps[] = " \t()[]'`,\":.!?";


typedef struct _reqlist
{

    char name[13];              /* Name of the file */
    int tagged;                 /* 1 = tagged */
    struct _reqlist *next;      /* Link to next file-rec */

} REQLIST;

static REQLIST *firstfile = NULL;

char *exts[] = { "arj",
    "zip",
    "arc",
    "pak",
    "lzh",
    "sqz",
    "com",
    "exe",
    "lha",
    "zoo",
    "txt",
    "sdn",
    "sda",
    "ans",
    "jpg",
    "gif",
    "tar",
    "ico",
    "bat",
    "cmd",
    "uc2",
    "rar",
    "bmp",
    "wav",
    "avi",
    ""
};

void CheckLineForFile(char *s);
int file_ext(char *s);
void addfilename(char *p, char *first);
int showlist(REQLIST * first);
void writerequest(REQLIST * first, AREA * area, MSGA * areahandle,
                  MMSG * origmsg);



int get_request(MMSG * curmsg, AREA * area, MSGA * areahandle)
{
    LINE *curptr;               /* Point to first line of message */
    int retval;
    REQLIST *thisfile;

    if (firstfile == NULL)      /* A 're-entry', don't scan if so? */
    {
        CheckLineForFile(curmsg->mis.subj);

        for (curptr = curmsg->txt; curptr; curptr = curptr->next)
        {
            if ((curptr->status & KLUDGE) ||
                (curptr->status & TEAR) || (curptr->ls[0] == '\0'))
                continue;       /* No files in kludges or tears! (Origs
                                   maybe!) */

            CheckLineForFile(curptr->ls);
        }
    }

    savescreen();

    retval = showlist(firstfile);

    if (retval == ACCEPT)
        writerequest(firstfile, area, areahandle, curmsg);

    /* Check if we need to dump the list from mem, or keep it */
    /* in order to move the msg one page up or down and */
    /* re-disp[lay the list and continue..  */

    if ((retval == ACCEPT) || (retval == ESC))
    {
        while (firstfile)       /* mem_free mem taken by list w/ files */
        {
            thisfile = firstfile->next;
            mem_free(firstfile);
            firstfile = thisfile;
        }

        firstfile = NULL;
    }

    putscreen();

    return retval;

}

// ==============================================================

void CheckLineForFile(char *s)
{
    char *p;

    for (p = strchr(s + 1, '.'); p; p = strchr(p, '.'))
    {
        if ((strchr(seps, *(p + 1)) == NULL) &&
            (strchr(seps, *(p - 1)) == NULL) && (*(p + 1) != '\0'))
        {
            if (file_ext(p + 1) != 0) /* Yep, we got a file here! */
            {
                addfilename(p - 1, s);
                p += 3;
            }
            else
            {
                p++;            /* Skip dot..  */
                if (*p == '.')  /* Yet another dot! Maybe ......... ? */
                    while (*p == '.')
                        p++;    /* Skip it fast */
            }
        }
        else
            p++;                /* Move beyond dot..  */
    }
}

// ==============================================================

/* Check if the string starting with *s is a file-extension, */
/* return 1 if this is the case, 0 otherwise.                */


int file_ext(char *s)
{
    int i;

    if (isdigit(*s))            /* Like timEd 1.00 etc. */
        return 0;


    for (i = 0; exts[i][0] != '\0'; i++)
    {
        if ((strncmpi(s, exts[i], 3) == 0) && /* extension matched */
            (strchr(seps, *(s + 3)) != NULL) /* fname ends here too! */
            )
            return 1;
    }

    if (((*s == 'a') || (*s == 'A') || (*s == 'z') || (*s == 'Z')) && isdigit(*(s + 1)) && isdigit(*(s + 2)) && (strchr(seps, *(s + 3)) != NULL) /* fname 
                                                                                                                                                    ends 
                                                                                                                                                    here 
                                                                                                                                                    too! 
                                                                                                                                                  */
        )
        return 1;               /* Catch nodediff.a22 etc */

    return 0;
}

/* Extract filename from line, and add it to the list */

void addfilename(char *p, char *first)
{
    char temp[100];
    char *charptr = p;
    REQLIST *thisone;
    static REQLIST *last = NULL;

    while ((strchr(seps, *charptr) == NULL) && (charptr > first))
        charptr--;

    if (strchr(seps, *charptr) != NULL)
        charptr++;              /* Skip the found space */

    if ((p - charptr) > 7)      /* Check length of filename */
        return;

    memset(temp, '\0', sizeof(temp));
    memmove(temp, charptr, (size_t) (p - charptr + 5));

    // Check for dupe filename
    thisone = firstfile;
    while (thisone)
    {
        if (strcmpi(thisone->name, temp) == 0) // Dupe!
            return;
        thisone = thisone->next;
    }

    thisone = mem_calloc(1, sizeof(REQLIST));
    strcpy(thisone->name, temp);

    if (firstfile == NULL)
        firstfile = thisone;
    else
        last->next = thisone;

    last = thisone;

}


/* Show list of files found, let user select some of them, and manually add */


int showlist(REQLIST * first)
{

    BOX *filebox, *extrabox;

    REQLIST *curptr, *highlighted;
    int numlines, rows, l;
    int curline, start, editret;
    char temp[81], extrafile[13];

    if (!first)
    {
        firstfile = mem_calloc(1, sizeof(REQLIST));
        strcpy(firstfile->name, "FILES");
        first = firstfile;
    }

  startup:

    start = curline = 0;

    for (curptr = first, numlines = 0; curptr->next != NULL;
         curptr = curptr->next)
        numlines++;

    rows = numlines >= maxy - 8 ? maxy - 8 : numlines + 1;

    filebox =
        initbox(5, maxx - 17, rows + 6, maxx - 1, cfg.col[Casframe],
                cfg.col[Castext], SINGLE, YES, ' ');
    drawbox(filebox);
    statusbar
        ("<enter> or <space> to select, ctrl-enter to accept, <ins> to add filenames");

    while (1)                   /* Exit is w/ return statement that gives
                                   command back */
    {
        curptr = first;

        /* search first line to display */

        for (l = 0; l < start; l++)
            curptr = curptr->next;

        /* display them.. */

        for (l = start; l < start + rows; l++, curptr = curptr->next)
        {
            sprintf(temp, "* %-12.12s ", curptr->name);

            if (!curptr->tagged)
                temp[0] = ' ';

            if (l == curline)
            {
                print(l - start + 6, maxx - 16, cfg.col[Cashigh], temp);
                highlighted = curptr;
                MoveXY(maxx - 14, l - start + 7);
            }
            else
                print(l - start + 6, maxx - 16, cfg.col[Castext], temp);
        }

        /* Now check for commands... */

        switch (get_idle_key(1, GLOBALSCOPE))
        {
        case 328:              /* up */
            if (curline)
            {
                curline--;
                if (curline < start)
                    start = curline;
            }
            else
            {
                curline = numlines;
                start = numlines - rows + 1;
            }
            break;

        case 336:              /* down */
            if (curline < numlines)
            {
                curline++;
                if (curline >= start + rows)
                    start++;
            }
            else
                start = curline = 0;
            break;

        case 327:              /* home */
            curline = start = 0;
            break;

        case 335:              /* end */
            curline = numlines;
            start = numlines - rows + 1;
            break;

        case 338:              /* <INS> */
            memset(extrafile, '\0', sizeof(extrafile));
            extrabox =
                initbox(13, 24, 17, 56, cfg.col[Cpopframe],
                        cfg.col[Cpoptext], SINGLE, YES, ' ');
            drawbox(extrabox);
            print(15, 26, cfg.col[Cpoptext], "Filename to add:");
            editret =
                getstring(15, 43, extrafile, 12, 12, VFNWW,
                          cfg.col[Centry], cfg.col[Cpoptext]);
            delbox(extrabox);
            if ((editret == 0) && (extrafile[0] != '\0'))
            {
                for (curptr = first; curptr->next; curptr = curptr->next)
                    /* nop, goto last entry */ ;
                curptr->next = mem_calloc(1, sizeof(REQLIST));
                strcpy(curptr->next->name, extrafile);
                curptr->next->tagged = 1;
                delbox(filebox); /* New one will be made */
                goto startup;
            }

            break;

        case 329:              /* page up */

            if (start == 0)
            {
                curline = 0;
                break;
            }

            start = curline - rows;

            if (start < 0)
                start = 0;

            if (curline > start + rows - 1)
                curline = start;

            break;

        case 337:              /* page down */

            if (start == numlines - rows + 1)
            {
                curline = numlines;
                break;
            }

            start = curline + rows;

            if (start > numlines - rows + 1)
                start = numlines - rows + 1;

            if (curline < start)
                curline = start;

            break;

        case 315:
            /* show_help(4); */
            break;

        case 388:              /* ctrl pgup */

            delbox(filebox);
            return UP;

        case 374:              /* ctrl pgdwn */

            delbox(filebox);
            return DOWN;

        case 10:
            delbox(filebox);
            return ACCEPT;

        case 27:
            delbox(filebox);
            return ESC;

        case 13:               /* <CR> */
        case 32:               /* Space */

            highlighted->tagged = highlighted->tagged ? 0 : 1;
            stuffkey(336);
            break;

        case 43:               /* + */

            curptr = first;
            while (curptr)
            {
                curptr->tagged = 1;
                curptr = curptr->next;
            }
            break;

        case 45:               /* - */

            curptr = first;
            while (curptr)
            {
                curptr->tagged = 0;
                curptr = curptr->next;
            }
            break;

        }                       /* switch */

    }                           /* while(1) */

}


/* ---------------------------------------------------- */

void writerequest(REQLIST * first, AREA * area, MSGA * areahandle,
                  MMSG * origmsg)
{
    AREA *toarea;
    REQLIST *curfile = first;
    MMSG *curmsg;
    MSGA *toareahandle;
    int retval;

    if ((toarea = SelectArea(cfg.first, 1, cfg.first)) == NULL) /* Select
                                                                   a
                                                                   netmail 
                                                                   area to 
                                                                   use */
        return;

    curmsg = mem_calloc(1, sizeof(MMSG)); /* Message */

    curmsg->mis.attr1 = cfg.usr.frqattr | (aFRQ | aLOCAL);

    strcpy(curmsg->mis.from, cfg.usr.name[custom.name].name);
    sprintf(curmsg->mis.to, "%s", FormAddress(&origmsg->mis.origfido));
    if (check_node(&curmsg->mis, 0, 0) != 1)
        strcpy(curmsg->mis.to, "SysOp");

    curmsg->mis.origfido = cfg.usr.address[(int)toarea->aka]; /* Fill this 
                                                                 all */

    curmsg->mis.destfido = origmsg->mis.origfido;
    matchaka(&curmsg->mis);

    curmsg->mis.msgwritten = JAMsysTime(NULL);

    if (toarea != area)
    {
        if (!
            (toareahandle =
             MsgOpenArea(toarea->dir, MSGAREA_CRIFNEC, toarea->base)))
        {
            Message("Error opening area", -1, 0, YES);
            showerror();
            return;
        }
    }
    else
        toarea = NULL;

    // Fill subject, first load of files.
    while (curfile)
    {
        if (curfile->tagged)
            curmsg->mis.requested =
                AddToStringList(curmsg->mis.requested, curfile->name, NULL,
                                0);
        curfile = curfile->next;
    }

    /* Now let the user edit the attributes */

    clsw(cfg.col[Cmsgtext]);
    retval = EditHeader(toarea ? toareahandle : areahandle,
                        curmsg, 1, 0, 0, toarea ? toarea : area);

// !!   memset(header->subj, '\0', sizeof(header->subj));

//   paint_header(header, 1, 0);
//   retval=UP;
//   while( (retval!=ESC) && (retval!=0) )
//      retval=SetAttributes(header, 1);

    if (retval == ABORT)
        goto close_up;

    if (AttemptLock(toarea ? toareahandle : areahandle) == -1)
        goto close_up;

    // write out first subject.
    if (curmsg->mis.requested != NULL)
        writefa(&curmsg->mis, toarea ? toareahandle : areahandle,
                toarea ? toarea : area, 0);

  close_up:

    FreeMIS(&curmsg->mis);
    mem_free(curmsg);

    MsgUnlock(toarea ? toareahandle : areahandle);

    ScanArea(toarea ? toarea : area, toarea ? toareahandle : areahandle,
             toarea ? 1 : 0);

    if (toarea)
        MsgCloseArea(toareahandle);

}
