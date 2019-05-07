#include "includes.h"

RAWBLOCK *spawn_editor(int checkchange, char *areatag);
RAWBLOCK *DoReplace(RAWBLOCK * blk, AREA * area, MMSG * curmsg,
                    char *command, int origin, int raw);
RAWBLOCK *internal_edit(LINE * first, AREA * area, MSGA * areahandle,
                        MMSG * curmsg, int startline);
int CheckForOutputFile(char *curfile);
int WriteFmtBody(RAWBLOCK * blk, char *curfile);

//int ExecAnySessionType(PSZ pszTitle, PSZ pszPgmName, PSZ pszInputs);

/* ----------------------------------------------- */

RAWBLOCK *GetBody(AREA * area,
                  MSGA * areahandle,
                  MMSG * curmsg,
                  char *origline,
                  LINE * firstline, int checkchange, int startline)
{
    int i;
    RAWBLOCK *blk, *last;
    int retval;


    if (!cfg.usr.internal_edit)
        blk = spawn_editor(checkchange, area->tag);
    else
        blk =
            internal_edit(firstline, area, areahandle, curmsg, startline);

    if (blk)
    {
        if (!cfg.usr.internal_edit && (cfg.usr.status & EDITSAVEMENU))
        {
            retval = ShowEditMenu(curmsg, 0);
            if (retval == -1)   // Message aborted!
            {
                FreeBlocks(blk);
                return NULL;
            }
        }

        last = JoinLastBlocks(blk, 1024);
        if (last && (area->type == ECHOMAIL))
        {
            i = clean_origin(last->txt, NULL);
            last->curlen = strlen(last->txt);
            last->curend = last->txt + last->curlen;
            if (i)              // We need to add origin!
            {
                Message
                    ("Warning! No (valid) Origin line detected, adding one..",
                     -1, 0, YES);
                AddToBlock(blk, origline, -1);
            }
        }

        // Now check for spellchecker, encrypt, sign etc.

        if (curmsg->status & (SPELLCHECK | SIGN | ENCRYPT))
        {
            if ((curmsg->status & SPELLCHECK)
                && cfg.usr.exespell[0] != '\0')
                blk = DoReplace(blk, area, curmsg, cfg.usr.exespell, 0, 1);

            // Check for crypting and signing at the same time.
            if ((curmsg->status & SIGN) &&
                (curmsg->status & ENCRYPT) &&
                (cfg.usr.execryptsign[0] != '\0'))
            {
                blk =
                    DoReplace(blk, area, curmsg, cfg.usr.execryptsign, 1,
                              0);
            }
            else                // maybe just signing or just encrypting?
            {
                if ((curmsg->status & ENCRYPT)
                    && cfg.usr.execrypt[0] != '\0')
                    blk =
                        DoReplace(blk, area, curmsg, cfg.usr.execrypt, 1,
                                  0);

                if ((curmsg->status & SIGN) && cfg.usr.exesign[0] != '\0')
                    blk =
                        DoReplace(blk, area, curmsg, cfg.usr.exesign, 1,
                                  0);
            }
        }
    }

    return blk;

}

// Spawn the external editor and read text back in...

RAWBLOCK *spawn_editor(int checkchange, char *areatag)
{
    char *temp;
    static char commandline[120], msgfile[120], filename[120];
    int lasthard = 1, forcehard = 0, editret, hadhcr = 0;
    FILE *infile;
    struct stat before, after;
    char tmpmsg[120];
    unsigned len, no_of_lines = 0;
    RAWBLOCK *blk;
    int oldlines = maxy;


    memset(commandline, '\0', sizeof(commandline));
    memset(msgfile, '\0', sizeof(msgfile));

    sprintf(msgfile, "%s" DIRSEP "timed.msg", cfg.homedir);
    stat(msgfile, &before);
    strcpy(filename, msgfile);

    cls();
#ifdef __WATCOMC__
    _settextcursor(0x0607);
#else
    _setcursortype(_NORMALCURSOR);
#endif
    cls();
    MoveXY(1, 3);

#if defined(__FLAT__) || defined(__UNIX__)
    if ((strcmpi(cfg.usr.editor + strlen(cfg.usr.editor) - 3, "cmd") == 0)
        || (strcmpi(cfg.usr.editor + strlen(cfg.usr.editor) - 3, "btm") == 0)
        || (strcmpi(cfg.usr.editor + strlen(cfg.usr.editor) - 2, "sh") == 0)
        || (strcmpi(cfg.usr.editor + strlen(cfg.usr.editor) - 3, "bat") == 0))
    {
        sprintf(commandline, "%s %s %s", cfg.usr.editor, msgfile, areatag);
        sprintf(tmpmsg, "þ Executing (batch) %s..", commandline);
        print(0, 0, 7, tmpmsg);
        savescreen();
        editret = system(commandline);
    }
    else
    {
        sprintf(tmpmsg, "þ Executing (direct) %s %s", cfg.usr.editor,
                msgfile);
        print(0, 0, 7, tmpmsg);
        savescreen();
#ifdef __UNIX__
        editret = do_exec(cfg.usr.editor, msgfile);
#else
        editret = spawnlp(P_WAIT, cfg.usr.editor, cfg.usr.editor, msgfile, NULL);
#endif
//       editret = ExecAnySessionType("timEd external editor", cfg.usr.editor, msgfile);
    }
#else
    {
        char *lastdot;
        // Batchfile?
        if ((lastdot = strrchr(cfg.usr.editor, '.')) != NULL &&
            ((strcmpi(lastdot + 1, "bat") == 0)
             || (strcmpi(lastdot + 1, "btm") == 0)))
        {
            strcat(msgfile, " ");
            strcat(msgfile, areatag);
        }
        sprintf(tmpmsg, "þ Executing %s %s", cfg.usr.editor, msgfile);
        print(0, 0, 7, tmpmsg);
        savescreen();

//   if(heapcheck() == -1)
//     Message("Corrupt heap _before_ spwaning editor!", -1, 0, YES);

        undo09();
        editret =
            do_exec(cfg.usr.editor, msgfile,
                    USE_ALL | HIDE_FILE | CHECK_NET,
                    (cfg.usr.status & SWAPEDIT) ? 0xFFFF : 0x0000,
                    environ);
        ins09();

//   if(heapcheck() == -1)
//     Message("Corrupt heap _after_ spawning editor!", -1, 0, YES);
    }
#endif

    video_init();

    if (oldlines != maxy)
    {
        setlines(oldlines);
        video_init();
    }

#ifdef __WATCOMC__
    _settextcursor(0x2000);
#else
    _setcursortype(_NOCURSOR);
#endif

    putscreen();
    print(2, 0, 7, "þ Back in timEd..");

#ifndef __OS2__
    if (editret & 0xFF00)
#else
    if (editret == -1)
#endif
    {
#ifdef __OS2__
        sprintf(tmpmsg, "Error spawning editor (%s)! Errno: %d.",
                cfg.usr.editor, errno);
#else
        sprintf(tmpmsg, "Error spawning editor (%s)!", cfg.usr.editor);
#endif
        Message(tmpmsg, -1, 0, YES);
        return NULL;
    }

    stat(filename, &after);

    if (checkchange)
    {
#if !defined(__OS2__) && !defined(__NT__)
        if (before.st_atime == after.st_atime)
            return NULL;
#else
        if (before.st_mtime == after.st_mtime)
            return NULL;
#endif
    }

    sprintf(msgfile, "%s" DIRSEP "timed.msg", cfg.homedir);
    if ((infile = fopen(msgfile, "rt")) == NULL)
    {
        Message("Can't open input file!", -1, 0, YES);
        return NULL;
    }

    blk = InitRawblock(4096, 2048, 32000u);

    temp = mem_calloc(1, 1024);

    setvbuf(infile, NULL, _IOFBF, 4096);

    print(4, 0, 7, "þ Reading message..");

    while (fgets(temp, 1023, infile))
    {
        if (!(no_of_lines++ % 25))
        {
            if (no_of_lines > 100)
                working(maxy - 1, 79, 7);
        }

        len = strlen(temp);

        hadhcr = 0;

        while (len &&
               ((temp[len - 1] == 0x0A) ||
                (temp[len - 1] == 0x0D) || (temp[len - 1] == 0x8D)))
        {
            temp[--len] = '\0';
            hadhcr = 1;
        }

        if (!lasthard && (strchr(" -*.,\tþ", temp[0])) != NULL) /* Never
                                                                   'pull'
                                                                   such a
                                                                   line at 
                                                                   the end 
                                                                   of
                                                                   prev.
                                                                   line */
        {
            if (GetLastChar(blk) == ' ')
                SetLastChar(blk, '\r');
            else
                AddToBlock(blk, "\r", 1);

            lasthard = 1;
        }

        if ((temp[0] == '~') && (temp[1] == '~')) /* Force hard returns
                                                     for next section... */
            forcehard = forcehard ? 0 : 1;

        else if (IsQuote(temp) || (len && temp[len - 1] == '~') || forcehard || (((temp[0] == 'C') || (temp[0] == 'c')) && (strncmpi(temp, "CC:", 3) == 0))) /* ~ 
                                                                                                                                                                forces 
                                                                                                                                                                HCR 
                                                                                                                                                              */
        {
            if (len && temp[len - 1] == '~') /* Strip 'control' char that
                                                forces HCR */
                temp[--len] = '\0';

            AddToBlock(blk, temp, len);
            AddToBlock(blk, "\r", 1);

            lasthard = 1;
        }

        else if (len < (maxx - 20)) /* short line, end w/ HCR */
        {
            if (len == 0 && !lasthard) /* White line was intended.. */
            {
                if (GetLastChar(blk) == ' ')
                    SetLastChar(blk, '\r');
                else
                    AddToBlock(blk, "\r", 1);
            }

            AddToBlock(blk, temp, len);

            /* Strip useless ending space if added before.. */

            if (GetLastChar(blk) == ' ')
                SetLastChar(blk, '\r');
            else
                AddToBlock(blk, "\r", 1);

            lasthard = 1;
        }
        else                    /* add to paragraph, no HCR */
        {
            AddToBlock(blk, temp, len);

            /* Don't add space if it's a split long line, some editors */
            /* don't end all lines with cr/lf */

            if (hadhcr && (temp[len - 1] != ' '))
            {
                if (GetLastChar(blk) != ' ')
                    AddToBlock(blk, " ", 1);
            }
            lasthard = 0;
        }
    }

    fclose(infile);
    if (temp)
        mem_free(temp);

    unlink(msgfile);

//   if(heapcheck() == -1)
//     Message("Corrupt heap at exit GetBody!", -1, 0, YES);

    return blk;
}


// ==============================================================

RAWBLOCK *internal_edit(LINE * firstline, AREA * area, MSGA * areahandle,
                        MMSG * curmsg, int startline)
{
    int lasthard = 1;
    LINE *oldline = NULL;
    RAWBLOCK *blk;


    if ((firstline =
         edittext(firstline, area, areahandle, curmsg, startline)) == NULL)
        return NULL;

#ifdef __WATCOMC__
    _settextcursor(0x2000);
#else
    _setcursortype(_NOCURSOR);
#endif

    blk = InitRawblock(4096, 1024, 8192); // Updated for perfbeta

    while (firstline)
    {
        /* Do we have to add a space (if we're appending two lines) */
        if ((!lasthard) &&
            (GetLastChar(blk) != ' ') &&
            (GetLastChar(blk) != '\0') &&
            (firstline->ls) &&
            (firstline->ls[0] != '\0') && (firstline->ls[0] != ' '))
        {
            AddToBlock(blk, " ", 1);
        }

        if (firstline->len)
            AddToBlock(blk, firstline->ls, firstline->len);

        if (firstline->status & HCR)
        {
            AddToBlock(blk, "\r", 1);
            lasthard = 1;
        }
        else
            lasthard = 0;

        oldline = firstline;
        firstline = firstline->next;
        if (oldline->ls)
            mem_free(oldline->ls);
        else
            Message("No line!", -1, 0, YES);
        mem_free(oldline);
    }

    return blk;

}


// =========================================================

#ifdef __NEVER__

/*
    The way I have been using this function is to call it with pszInputs
    formatted like:

        /C program.exe switch1[, switch2...]

    pszTitle is just the text that goes in the title bar.
*/

#define INCL_DOS
#include <os2.h>

int ExecAnySessionType(PSZ pszTitle, PSZ pszPgmName, PSZ pszInputs)
{
    APIRET rc;
    int ok = -1;
    STARTDATA std;
    PID pidSession;
    ULONG idSession;
    CHAR szObjFail[50];

    memset(&std, 0, sizeof(std));
    std.Length = sizeof(STARTDATA);
    std.FgBg = SSF_FGBG_FORE;
    std.TraceOpt = SSF_TRACEOPT_NONE;
    std.PgmTitle = pszTitle;
    std.InheritOpt = SSF_INHERTOPT_PARENT;
    std.SessionType = SSF_TYPE_DEFAULT;
    std.PgmControl = SSF_CONTROL_VISIBLE;
    std.ObjectBuffer = szObjFail;
    std.ObjectBuffLen = sizeof(szObjFail);
    std.Related = SSF_RELATED_CHILD;
    std.PgmInputs = pszInputs;
    std.PgmName = pszPgmName;

    rc = DosStartSession(&std, &idSession, &pidSession);

    if ((rc) && (rc != ERROR_SMG_START_IN_BACKGROUND))
    {
        ok = -1;
        Message(szObjFail, -1, 0, YES);
    }
    else
        ok = 0;

    return (ok);
}

#endif

// =============================================================

RAWBLOCK *DoReplace(RAWBLOCK * blk, AREA * area, MMSG * curmsg,
                    char *command, int origin, int raw)
{
    char temp[100];
    char *prog, *parms;
    char curfile[120], newfile[120];
    RAWBLOCK *newblock, *last;
    char *originptr = NULL, *addorigin = NULL;
    int result;


    sprintf(curfile, "%s" DIRSEP "timed.msg", cfg.homedir); // Output file for
                                                    // temp msg
    sprintf(newfile, "%s" DIRSEP "timed.new", cfg.homedir); // Possible new input
                                                    // file

    if (origin)                 // Strip the origin from txt, save for
                                // later in addorigin
    {
        last = JoinLastBlocks(blk, 1024);
        if (last)
        {
            if (clean_origin(last->txt, &originptr) == 0
                && originptr != NULL)
            {
                *originptr = '\0';
                last->curlen = strlen(last->txt);
                last->curend = last->txt + last->curlen;
                addorigin = mem_strdup(originptr + 1);
            }
        }
    }


    if (raw)
        result = WriteRawBody(blk, curfile);
    else
        result = WriteFmtBody(blk, curfile);

    if (result == -1)
    {
        if (addorigin)          // We stripped the origin, so put it back
                                // on..
        {
            AddToBlock(blk, "\r", -1);
            AddToBlock(blk, addorigin, -1);
            mem_free(addorigin);
        }
        return blk;
    }

    strcpy(temp, command);
    prog = strtok(temp, " \t\r\n");
    parms =
        BuildCommandLine(strtok(NULL, "\r\n"), area, NULL, curmsg, curfile,
                         newfile);

    runaprog(prog, parms, 0);

    if (parms)
        mem_free(parms);

    CheckForOutputFile(curfile); // See if we read timed.msg or timed.new

    newblock = ReadBodyFromDisk(curfile);
    if (newblock)
    {
        FreeBlocks(blk);
        blk = newblock;
    }

    if (addorigin)              // Put back the origin that we stripped.
    {
        AddToBlock(blk, "\r", -1);
        AddToBlock(blk, addorigin, -1);
        mem_free(addorigin);
    }

    sprintf(curfile, "%s" DIRSEP "timed.msg", cfg.homedir); // Output file for
                                                    // temp msg
    sprintf(newfile, "%s" DIRSEP "timed.new", cfg.homedir); // Possible new input
                                                    // file
    unlink(curfile);
    unlink(newfile);

    return newblock;

}

// ======================================================================

int WriteRawBody(RAWBLOCK * blk, char *curfile)
{
    int outfile;

    outfile =
        open(curfile, O_BINARY | O_CREAT | O_TRUNC | O_WRONLY,
             S_IREAD | S_IWRITE);

    if (outfile == -1)
    {
        sprintf(msg, "Can't create %s (errno %d)!", curfile, errno);
        Message(msg, -1, 0, YES);
        return -1;
    }

    while (blk)
    {
        if (blk->curlen > 0)
        {
            if (write(outfile, blk->txt, (unsigned)blk->curlen) !=
                (int)blk->curlen)
            {
                sprintf(msg, "Error writing to %s!", curfile);
                Message(msg, -1, 0, YES);
                close(outfile);
                return -1;
            }
        }
        blk = blk->next;
    }

    close(outfile);
    return 0;
}

// ======================================================================

int WriteFmtBody(RAWBLOCK * blk, char *curfile)
{
    FILE *outfile;
    LINE *firstline, *curline;
    char *tmpbuf;

    outfile = fopen(curfile, "w+t");

    if (outfile == NULL)
    {
        sprintf(msg, "Can't create %s (errno %d)!", curfile, errno);
        Message(msg, -1, 0, YES);
        return -1;
    }

    while (blk)
    {
        if (blk->curlen > 0)
        {
            tmpbuf = mem_calloc(1, blk->curlen + 1);
            memcpy(tmpbuf, blk->txt, blk->curlen);
            curline = firstline = wraptext(tmpbuf, 79, 1, 0);
            while (curline)
            {
                fprintf(outfile, "%s\n", curline->ls);
                curline = curline->next;
            }
            FreeLines(firstline);
            mem_free(tmpbuf);
        }
        blk = blk->next;
    }

    fclose(outfile);
    return 0;
}

// =============================================================

int ShowEditMenu(MMSG * curmsg, int escallowed)
{
    BOX *editmenu;
    int whatnow = 0, stop = 0, retval = 0;
    int key;
    int top, left;

    top = maxy / 2 - 4;
    left = maxx / 2 - 11;
    editmenu =
        initbox(top, left, top + 6, left + 19, cfg.col[Casframe],
                cfg.col[Castext], SINGLE, YES, ' ');
    drawbox(editmenu);

    do
    {
        print(top + 1, left + 2,
              whatnow == 0 ? cfg.col[Cashigh] : cfg.col[Castext],
              " Save message    ");
        print(top + 2, left + 2,
              whatnow == 1 ? cfg.col[Cashigh] : cfg.col[Castext],
              " Sign message    ");
        print(top + 3, left + 2,
              whatnow == 2 ? cfg.col[Cashigh] : cfg.col[Castext],
              " Encrypt message ");
        print(top + 4, left + 2,
              whatnow == 3 ? cfg.col[Cashigh] : cfg.col[Castext],
              " Run spellcheck  ");
        if (escallowed)
            print(top + 5, left + 2,
                  whatnow == 4 ? cfg.col[Cashigh] : cfg.col[Castext],
                  " Quit this menu  ");
        else
            print(top + 5, left + 2,
                  whatnow == 4 ? cfg.col[Cashigh] : cfg.col[Castext],
                  " Abort message   ");

        if (curmsg->status & SPELLCHECK)
            printc(top + 4, left + 1, cfg.col[Castext], 'û');
        else
            printc(top + 4, left + 1, cfg.col[Castext], ' ');

        if (curmsg->status & SIGN)
            printc(top + 2, left + 1, cfg.col[Castext], 'û');
        else
            printc(top + 2, left + 1, cfg.col[Castext], ' ');

        if (curmsg->status & ENCRYPT)
            printc(top + 3, left + 1, cfg.col[Castext], 'û');
        else
            printc(top + 3, left + 1, cfg.col[Castext], ' ');

        key = get_idle_key(1, GLOBALSCOPE);

        switch (key)
        {
        case 27:               /* esc */
            if (escallowed)
            {
                stop = 1;
                retval = -1;
            }
            break;

        case 336:              /* down */
            if (++whatnow > 4)
                whatnow = 4;
            break;

        case 328:              /* up */
            if (--whatnow < 0)
                whatnow = 0;
            break;

        case 327:              /* home */
            whatnow = 0;
            break;

        case 335:              /* end */
            whatnow = 4;
            break;

        case 32:
        case 13:
            switch (whatnow)
            {
            case 0:
                stop = 1;
                break;

            case 1:
                curmsg->status ^= SIGN;
                break;

            case 2:
                curmsg->status ^= ENCRYPT;
                break;

            case 3:
                curmsg->status ^= SPELLCHECK;
                break;

            case 4:
                stop = 1;
                retval = -1;
                break;
            }
            break;
        }

    }
    while (stop == 0);


    delbox(editmenu);
    return retval;

}

// ==============================================================

int CheckForOutputFile(char *curfile)
{
    struct stat mystatnew, mystatmsg;
    char temp[120];

    sprintf(temp, "%s" DIRSEP "timed.new", cfg.homedir);
    if (stat(temp, &mystatnew) == -1)
        return 0;               // no timEd.new, leave curfile set to
                                // ..\timed.msg

    if (stat(curfile, &mystatmsg) == -1)
    {
        strcpy(curfile, temp);  // let 'curfile' point to timed.new
        return 0;
    }

    // If timed.msg is actually newer than timed.new, we still use
    // timed.msg
    // because timed.new cannot be a result of timed.msg in that case.
    // Note that if the time is the same (quite possible, fast processing
    // within
    // a fraction of a second!), timed.new is chosen.

#ifndef __OS2__
    if (mystatmsg.st_atime > mystatnew.st_atime)
        return 0;
#else
    if (mystatmsg.st_mtime > mystatnew.st_mtime)
        return 0;
#endif

    // timed.new is same time or newer, let's use that.

    strcpy(curfile, temp);

    return 0;

}

// ==============================================================
