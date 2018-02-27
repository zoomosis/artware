
#include "includes.h"

extern LINE *firstmemline, *lastmemline;
extern int curtxtline;

MMSG *ForwardMessage(MSG * toareahandle, AREA * toarea, AREA * area,
                     MMSG * curmsg);
void mutilate_origin(LINE * first);


/* -------------------------------------------------------------- */

void CopyMarked(AREA * area, AREA * toarea, MSG * areahandle,
                MSG * toareahandle, int kill)
{
    MMSG *curmsg;
    char temp[80];
    int i, add = 0, result;
    dword msgno;


    if (AttemptLock(toareahandle) == -1)
        return;

    if (AttemptLock(areahandle) == -1)
    {
        MsgUnlock(toareahandle);
        return;
    }

    for (i = 0; i < area->mlist->active; i++)
    {
        if (xkbhit() && get_idle_key(0, GLOBALSCOPE) == 27)
            break;

        if ((msgno =
             MsgUidToMsgn(areahandle, area->mlist->list[i],
                          UID_EXACT)) == 0L)
            continue;           // Doesn't exist (anymore?).

        if ((curmsg = GetRawMsg(msgno, areahandle, READALL, 0)) == NULL)
        {
            sprintf(temp, "Error reading message %d!", msgno);
            Message(temp, -1, 0, YES);
            continue;
        }

        sprintf(temp, "þ Now working on message %lu..", msgno + add);
        statusbar(temp);

        memset(&curmsg->mis.replyto, '\0', sizeof(curmsg->mis.replyto));
        memset(&curmsg->mis.replies, '\0', sizeof(curmsg->mis.replies));
        memset(&curmsg->mis.nextreply, '\0',
               sizeof(curmsg->mis.nextreply));

        result = SaveMessage(toareahandle, toarea, &curmsg->mis, curmsg->firstblock, curmsg->ctxt, 0, -1); // preserve 
                                                                                                           // == 
                                                                                                           // -1: 
                                                                                                           // no 
                                                                                                           // character 
                                                                                                           // translation!

        if (result == -1)
            stuffkey(27);       // Let's stop if there's an error!

        add_tosslog(toarea, toareahandle);

        if (kill && (result != -1)) // Only kill if 'save' went OK.
        {
            if (MsgKillMsg(areahandle, msgno) == -1)
            {
                Message("Error deleting message!", -1, 0, YES);
                showerror();
            }
            if (area->base & (MSGTYPE_SQUISH | MSGTYPE_HMB))
                add++;
        }

        ReleaseMsg(curmsg, 1);
    }

    if (kill)                   // Dump all marks and get fresh list
    {
        DumpMarkList(area->mlist);
        if ((area->mlist = InitMarkList()) == NULL)
            Message("Error recreating marklist!", -1, 0, YES);
    }

    MsgUnlock(areahandle);
    MsgUnlock(toareahandle);

}


/* ----------------------------------------------- */


void DeleteMarked(AREA * area, MSG * areahandle)
{
    char temp[80];
    int i, add = 0;
    dword msgno;

    if (AttemptLock(areahandle) == -1)
        return;

    for (i = 0; i < area->mlist->active; i++)
    {
        if (xkbhit() && get_idle_key(0, GLOBALSCOPE) == 27)
            break;

        if ((msgno =
             MsgUidToMsgn(areahandle, area->mlist->list[i],
                          UID_EXACT)) == 0L)
            continue;           // Doesn't exist (anymore?).

        sprintf(temp, " þ Now deleting message %lu..", msgno + add);
        statusbar(temp);

        if (area->base & (MSGTYPE_SQUISH | MSGTYPE_HMB))
            add++;

        if (MsgKillMsg(areahandle, msgno) == -1)
        {
            Message("Error deleting message!", -1, 0, YES);
            showerror();
        }
    }

    DumpMarkList(area->mlist);
    area->mlist = InitMarkList();

    MsgUnlock(areahandle);

}


/* ----------------------------------------------- */


void MoveMessage(MSG * areahandle, AREA * area, MMSG * curmsg)
{
    BOX *border;
    char infotext[320];
    int c = 32;
    dword no;
    MMSG *message = NULL;
    AREA *toarea = NULL;
    MSG *toareahandle = NULL;
    UMSGID absolute;
    RAWBLOCK *infront, *last;

    no = MsgGetCurMsg(areahandle);
    absolute = MsgMsgnToUid(areahandle, no);

    border =
        initbox(10, 19, 14, 60, cfg.col[Cpopframe], cfg.col[Cpoptext],
                SINGLE, YES, ' ');
    drawbox(border);
    boxwrite(border, 1, 1, "    [M]ove, [C]opy or [F]orward?");
    MoveXY(1, 13);

    while ((c != 'm') &&
           (c != 'M') &&
           (c != 'c') &&
           (c != 'C') && (c != 'f') && (c != 'F') && (c != 27))
        c = get_idle_key(1, GLOBALSCOPE);

    delbox(border);

    if (c == 27)
        return;

    savescreen();
//   clsw(cfg.usr..);

    if ((toarea = SelectArea(cfg.first, 1, cfg.first)) == NULL)
    {
        putscreen();
        goto close_up;
    }

    if (toarea == area)
        toarea = NULL;

    putscreen();

    if (toarea != NULL)
    {
        if (!
            (toareahandle =
             MsgOpenArea(toarea->dir, MSGAREA_CRIFNEC, toarea->base)))
        {
            Message("Error opening area!", -1, 0, YES);
            showerror();
            goto close_up;
        }
        get_custom_info(toarea);
    }
    else
        toareahandle = areahandle; /* Forward / Move / Copy to same area */

    if (c == 'f' || c == 'F')
    {
        message =
            ForwardMessage(toareahandle, toarea ? toarea : area, area,
                           curmsg);
        if (!message)
            goto close_up;
    }
    else
    {
        if (area->mlist->active != 0) // We have Marked msgs!
        {
            if (confirm("Process all marked messages? [y/N]") != 0)
            {
                if (toarea != NULL)
                    CopyMarked(area, toarea, areahandle, toareahandle,
                               (c == 'm' || c == 'M'));
                else
                    Message
                        ("Mass Move/Copy to current area is not possible",
                         -1, 0, YES);

                goto close_up;
            }
            // else we do only this one mesage..
        }

        if ((message = GetRawMsg(no, areahandle, READALL, 1)) == NULL)
        {
            Message("Error reading Message!", -1, 0, YES);
            return;
        }

        last = JoinLastBlocks(message->firstblock, 2048);

//     if(last && last->txt)
//       {
//       clean_end_message(last->txt);
//       last->curlen = strlen(last->txt);
//       last->curend = last->txt + last->curlen;
//       }

        // Strip trailing junk.
        if (message->mis.seenby)
            FreeStringList(message->mis.seenby);
        if (message->mis.path)
            FreeStringList(message->mis.path);
        if (message->mis.via)
            FreeStringList(message->mis.via);
        message->mis.seenby = message->mis.path = message->mis.via = NULL;

        memset(&message->mis.replyto, '\0', sizeof(message->mis.replyto));
        memset(&message->mis.replies, '\0', sizeof(message->mis.replies));
        memset(&message->mis.nextreply, '\0',
               sizeof(message->mis.nextreply));

        // ZAP all attribs except the ones below:
        message->mis.attr1 &= (aFILE | aFRQ | aURQ | aZGT);

        // New attribs.
        if (toarea)
            message->mis.attr1 |= toarea->stdattr; /* Reset msgbits */
        else
            message->mis.attr1 |= area->stdattr; /* Reset msgbits */


        memset(infotext, '\0', sizeof(infotext));

        if (cfg.usr.status & SHOWNOTES)
        {
            sprintf(infotext, "* %s (from: %s) by %s using %s.\r\r",
                    (c == 'm'
                     || c == 'M') ? "Moved" : "Copied", area->tag,
                    cfg.usr.name[custom.name].name, myname);
            RemoveTOline(message->firstblock, &message->mis);
        }

        infront = InitRawblock(512, 512, 64000u);
        AddToBlock(infront, infotext, strlen(infotext));
        infront->next = message->firstblock;
        message->firstblock = infront;
    }

    if (c == 'f' || c == 'F')
    {
        check_cc(toarea ? toarea : area, toareahandle, &message->mis,
                 &message->firstblock);

        if ((toarea && (toarea->type == NETMAIL))
            || (!toarea && (area->type == NETMAIL)))
        {
            /* Checked CC: first! Don't mess up zonegate etc!! */
            zonegate(&message->mis, message->ctxt, 0,
                     toarea ? toarea->base : area->base);
        }
    }

    if (SaveMessage(toareahandle,
                    toarea ? toarea : area,
                    &message->mis,
                    message->firstblock, message->ctxt, 0, 0) == -1)
        Message("Error creating message!", -1, 0, YES);

    if (toarea)
    {
        add_tosslog(toarea, toareahandle);
    }
    else
    {
        add_tosslog(area, areahandle);
    }


    if ((c == 'M') || (c == 'm'))
    {
        RemoveMarkList(area->mlist, absolute);
        MsgKillMsg(areahandle,
                   MsgUidToMsgn(areahandle, absolute, UID_EXACT));
        if ((area->type == NETMAIL) && (cfg.usr.netsema[0] != '\0'))
            creat_touch(cfg.usr.netsema);
    }

    /* Here we close & release all */

  close_up:

    if (toareahandle != NULL)   /* Only if we actually have
                                   opened/locked/written */
        MsgUnlock(toareahandle);

    if ((toarea != NULL) && (toareahandle != NULL))
    {
        ScanArea(toarea, toareahandle, 1);
        MsgCloseArea(toareahandle);
    }

    if (message)
        ReleaseMsg(message, 1);

    ScanArea(area, areahandle, 0);

    get_custom_info(area);
}


// Routines to forward message, let user edit the text etc.

MMSG *ForwardMessage(MSG * toareahandle, AREA * toarea, AREA * area,
                     MMSG * curmsg)
{
    MMSG *message;
    char infotext[300], from[50], to[50];
    LINE *firstadd, *last;
    char *replace_origin;
    int retval;

    message = mem_calloc(1, sizeof(MMSG));

    clsw(cfg.col[Cmsgtext]);
    if ((retval = MakeHeader(toareahandle,
                             NULL,
                             0,
                             toarea, 0, &curmsg->mis.subj, message)) == -1)
    {
        mem_free(message);
        return NULL;
    }

    message->ctxt = MakeKludge(NULL, &message->mis,
                               (toarea->type == NETMAIL));

    mutilate_origin(curmsg->txt);

    if (toarea->type == ECHOMAIL)
    {
        get_custom_info(toarea);

        // Make new origin, wrap it, append to end.

        replace_origin = make_origin(custom.aka);
        firstadd = wraptext(replace_origin, maxx - 2, 1, 0);
        if (!curmsg->txt)
            curmsg->txt = firstadd;
        else
        {
            last = curmsg->txt;
            while (last->next)
                last = last->next;
            last->next = firstadd;
        }
    }

    curtxtline = 0;             // to update line pos in txt for later
                                // edit

    if (area->type == NETMAIL || area->type == ECHOMAIL)
        sprintf(from, "%s (%s)", &curmsg->mis.from,
                FormAddress(&curmsg->mis.origfido));
    else
        sprintf(from, "%s", &curmsg->mis.from);

    if (area->type == NETMAIL && curmsg->mis.destinter[0] != '\0')
        sprintf(to, "%s", &curmsg->mis.destinter);
    else if (area->type == NETMAIL)
        sprintf(to, "%s (%s)", &curmsg->mis.to,
                FormAddress(&curmsg->mis.destfido));
    else
        sprintf(to, "%s", &curmsg->mis.to);

    sprintf(infotext,
            "* Forwarded (from: %s) by %s using %s.\r* Originally from %s to %s.\r* Original dated: %s\r\r",
            area->tag, cfg.usr.name[custom.name].name, myname, from, to,
            MakeT(curmsg->mis.msgwritten, DATE_HDR));

    // Put information in front of message...

    last = curmsg->txt;
    curmsg->txt = firstadd = wraptext(infotext, maxx - 2, 1, 0);
    while (firstadd->next)
    {
        curtxtline++;
        firstadd = firstadd->next;
    }
    firstadd->next = last;

    // Now we have to write out all this stuff, and edit the body..

    firstmemline = lastmemline = NULL;

    MakeText(curmsg, NULL);     /* Literally write out text */

    message->firstblock = GetBody(toarea,
                                  toareahandle,
                                  message,
                                  make_origin(custom.aka),
                                  firstmemline, 0, curtxtline);

    if (message->firstblock == NULL)
    {
        Message("Forward aborted.", 10, 0, YES);
        ReleaseMsg(message, 1);
        return NULL;
    }

    get_custom_info(area);

    return message;

}



void mutilate_origin(LINE * first)
{
    LINE *curline;
    LINE *lastline;

    for (curline = first; curline; curline = curline->next)
    {
        lastline = curline;
        if (curline->status & (ORIGIN | TEAR)) // Invalidate first...
        {
            if (strncmpi(curline->ls, "---", 3) == 0)
                memset(curline->ls, '_', 3);
            else if (strncmpi(curline->ls, " * Origin: ", 11) == 0)
                curline->ls[1] = '-';
        }
        else if (curline->status & KLUDGE)
        {
            curline->ls[0] = '@';
        }
    }

    if (lastline)               // Check for a sole tearline... (netmail).
    {
        if (lastline->ls && strncmp(lastline->ls, "---", 3) == 0)
            memset(lastline->ls, '_', 3);
        else if (lastline->ls == NULL || lastline->ls[0] == '\0')
        {
            if (lastline->prev)
            {
                lastline = lastline->prev;
                if (lastline->ls && strncmp(lastline->ls, "---", 3) == 0)
                    memset(lastline->ls, '_', 3);
            }
        }
    }
}


/* -------------------------------------------------------------- */
/* Strip CR's, PATHlines, SEEN-BY's, and VIA lines from the end   */
/* of the messages. This makes them ready for re-export (when     */
/* moving, copying etc)                                           */
/* -------------------------------------------------------------- */


dword clean_end_message(char *msgbuf)
{

#define TRUE  1
#define FALSE 0

    char *cptr, *end, *tmpptr, *curend = NULL;
    int chopit = FALSE, stophere = FALSE;


    if (msgbuf == NULL)
        return 0;

    end = cptr = strchr(msgbuf, '\0'); /* goto end */

    /* Do we have anything to work on? */

    if ((end == NULL) || (end == msgbuf))
        return 0;

    cptr--;                     /* go back one char from the trailing '\0' 
                                 */

    while (                     /* Strip all junk at the end */
              ((*cptr == ' ') ||
               (*cptr == '\r') ||
               (*cptr == '\t') || (*cptr == '\n')) && (cptr > msgbuf))
        *cptr-- = '\0';

    curend = cptr + 1;

    while ((--cptr >= msgbuf) && (stophere != TRUE)) /* Go back no further 
                                                        than start of
                                                        message! */
    {
        if (*cptr == '\r')      /* end of line, what's on beg. of next
                                   line? */
        {
            /* Skip LF's and Soft Returns */
            tmpptr = cptr + 1;
            while (((*tmpptr == 0x0A) || (*tmpptr == 0x8D))
                   && (tmpptr < end))
                tmpptr++;

            if (*tmpptr == '\0') /* We're still at the end */
                chopit = TRUE;

            else if (*tmpptr == '\01') /* Kludge detected */
            {
                if (strncmpi(tmpptr + 1, "via", 3) == 0) /* 'via' kludge,
                                                            strip */
                    chopit = TRUE;
                else if (strncmp(tmpptr + 1, "PATH", 4) == 0) /* path,
                                                                 strip */
                    chopit = TRUE;
                else
                    stophere = TRUE; /* unknown, stop chopping! */
            }

            else if (*tmpptr == 'S') /* Seen-by maybe? */
            {
                if (strncmp(tmpptr, "SEEN-BY:", 7) == 0) /* SB, strip */
                    chopit = TRUE;
                else
                    stophere = TRUE;
            }

//        /* else if(*tmpptr == ' ')  /* Origin maybe? */
//              {
//              if (strncmp(tmpptr, " * Origin: ", 11) == 0) /* Origin! */
//                  stophere = TRUE;
//
//              } */
            else
                stophere = TRUE;

            if (chopit == TRUE)
            {
                *cptr = '\0';
                curend = cptr;
                chopit = FALSE;
            }

        }                       /* if */

    }                           /* while */

    *curend++ = '\r';
    *curend = '\0';

    return (dword) (curend - msgbuf);
}



/* Clean the end of message and check for origin */

int clean_origin(char *msgbuf, char **startorig)
{
    char *cptr, *end, *open, *close;

    if (startorig)
        (*startorig) = NULL;

    if (msgbuf == NULL)
        return 1;



    end = cptr = strchr(msgbuf, '\0'); /* goto end */

    if ((end == NULL) || (end == msgbuf))
        return 1;

//   if( (*(cptr-1) == '\r') && (*(cptr-2) == ')') )
//      return 0;    /* All fine and dandy.. */

    cptr--;

    /* strip junk at end of message */

    while (((*cptr == ' ') ||
            (*cptr == '\r') ||
            (*cptr == '\t') || (*cptr == '\n')) && (cptr > msgbuf))
        *cptr-- = '\0';


    while (--cptr >= msgbuf)
    {
        if (*cptr == '\r')
        {
            cptr++;
            break;
        }
    }

    if (strncmp(cptr, " * Origin: ", 11) != 0)
    {
        if (startorig && strncmp(cptr, "---", 3) == 0)
        {
            *startorig = cptr - 1;
            return 0;
        }
        return 1;
    }

    if (startorig)
    {
        *startorig = cptr;
        cptr--;
        while (--cptr >= msgbuf)
        {
            if (*cptr == '\r')
            {
                cptr++;
                if (strncmp(cptr, "---", 3) == 0)
                    *startorig = cptr - 1;
                return 0;
            }
        }
        return 0;
    }

    if (((open = strrchr(cptr, '(')) == NULL) ||
        ((close = strrchr(open, ')')) == NULL) || (open > close))
        return 1;

    *(close + 1) = '\r';
    *(close + 2) = '\0';

    if (strlen(cptr) > 80)
        Message("Warning, Origin line too long!!", -1, 0, 1);

    return 0;

}
