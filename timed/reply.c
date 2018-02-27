#include "includes.h"

#define NORMALCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'. !?,*&^#$%()-_=+[]{}\\\"|';:,.<>/?`~1234567890"
#define ADDRESSCHARS "0123456789:/.-"

/* ------ Prototypes -------- */

void MakeQuote(MMSG * curmsg, AREA * area, word replytype, MIS * newmis,
               char *addtop);
char *getinitials(char *name);
void MakeTemplate(MIS * mis, AREA * area);
char *make_origin(int aka);
char *make_rephello(MIS * mis, MIS * newmis, word replytype);
char *make_hello(MIS * mis);
int check_gated(MMSG * msg);
int gateconfirm(MIS * header, int showinfo);
char *mtime(void);

void near addit(char *temp, FILE * repfile);
void EndMessage(AREA * area, MIS * mis, MIS * newmis, FILE * repfile);

void near rton(char *s);
RAWBLOCK *MakeCopy(RAWBLOCK * first);


LINE *firstmemline = NULL;      /* For use with the internal editor */
LINE *lastmemline = NULL;
int curtxtline = 0;

void add_text(char *line);      /* Add a line to text in mem */
int WriteReplyLink(AREA * area, MSGA * areahandle, dword orig, dword new);
void AskDumpBody(RAWBLOCK * blk);


/* ----------------------------------------------- */
/* reply == 1 : normal reply                       */
/* reply == 2 : reply (also) to original addressee */
/* reply == 3 : turbo reply                        */
/* reply == 4 : bounce reply                       */
/* ----------------------------------------------- */

void MakeMessage(MMSG * curmsg, AREA * area, MSGA * areahandle, word reply,
                 UMSGID reply_to_id, char *add_to_top)
{
    dword origmsg;
    char *kludges = NULL, msgfile[100];
    UMSGID new_msg_id;
    RAWBLOCK *blk = NULL;
    MMSG *newmsg;
    int retval;


    if (area->readonly)
    {
        if (reply && curmsg->ctxt
            && (GetCtrlToken(curmsg->ctxt, "AREA:") != NULL))
        {
            // stuffkey(cREADreplyother);
            ReplyOther(curmsg, area);
        }
        else
            Message("This is a Read-Only area!", -1, 0, YES);
        return;
    }

    firstmemline = lastmemline = NULL;
    curtxtline = 0;

    ClsRectWith(5, 0, maxy - 2, maxx - 1, cfg.col[Cmsgtext], ' ');

    newmsg = mem_calloc(1, sizeof(MMSG));

    retval =
        MakeHeader(areahandle, curmsg, reply, area, reply_to_id, NULL,
                   newmsg);

    if (retval == -1)
    {
        FreeMIS(&newmsg->mis);
        mem_free(newmsg);
        return;
    }

    sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
    unlink(msgfile);

    if (reply)
        MakeQuote(curmsg, area, reply, &newmsg->mis, add_to_top);
    else
        MakeTemplate(&newmsg->mis, area);


    /* Now we will get the body (spawn editor), so memory could be */
    /* an issue. That's why we de-allocate the mem for the message */
    /* body of the 'curmsg'. If it's a (very) large message this */
    /* could save us from a 'out of mem' or save swapping time */

    if (curmsg)
    {
        ReleaseMsg(curmsg, 0);
        _heapmin();
    }


    if (!(newmsg->mis.attr1 & aFRQ)
        || confirm("Do you want to add a message body? (y/N)"))
    {
        if ((blk = GetBody(area, areahandle, newmsg, make_origin(custom.aka), firstmemline, 1, curtxtline)) == NULL) /* Empty 
                                                                                                                        msg 
                                                                                                                        allowed 
                                                                                                                        for 
                                                                                                                        freq 
                                                                                                                        and 
                                                                                                                        attach 
                                                                                                                      */
        {
            if ((newmsg->mis.attr1 & aFILE) || (newmsg->mis.attr1 & aFRQ))
            {
                blk = InitRawblock(1, 1024, 64000u);
            }
            else
            {
                Message("Message aborted", 10, 0, YES);
                goto close_up;
            }
        }
    }
    else
    {
        blk = InitRawblock(1, 1024, 64000u);
    }

    kludges = MakeKludge(curmsg, &newmsg->mis, area->type == NETMAIL);

    if (AttemptLock(areahandle) == -1)
    {
        AskDumpBody(blk);
        goto close_up;
    }

    /* Check for any CC:'s in the message, write other */
    /* msgs in that function, and the original over here */

    check_cc(area, areahandle, &newmsg->mis, &blk);

    if (area->type == NETMAIL)
    {
        /* Checked CC: first! Don't mess up zonegate etc!! */
        zonegate(&newmsg->mis, kludges, 0, area->base);
    }

    if (SaveMessage(areahandle, area, &newmsg->mis, blk, kludges, 0, 0) ==
        -1)
        goto close_up;

    add_tosslog(area, areahandle);

    if (reply && reply_to_id)   /* Write reply-info in msg we replied to */
    {
        if ((area->type == NETMAIL || area->type == MAIL)
            && (cfg.usr.delreply != 0))
        {
            if (                /* 1 == no confirm */
                   (cfg.usr.delreply == 1) ||
                   (confirm("Delete original message? (y/N)") != 0))
            {
                if (MsgKillMsg
                    (areahandle,
                     MsgUidToMsgn(areahandle, reply_to_id,
                                  UID_EXACT)) == -1)
                {
                    Message("Can't kill message!", -1, 0, YES);
                    showerror();
                }
                RemoveMarkList(area->mlist, reply_to_id);
                goto close_up;
            }
        }

        new_msg_id = MsgMsgnToUid(areahandle, MsgGetHighMsg(areahandle));
        origmsg = MsgUidToMsgn(areahandle, reply_to_id, UID_EXACT);
        WriteReplyLink(area, areahandle, origmsg, new_msg_id);
    }

  close_up:

    MsgUnlock(areahandle);

    if (blk)
        FreeBlocks(blk);
    if (kludges)
        mem_free(kludges);
    FreeMIS(&newmsg->mis);
    mem_free(newmsg);

    ScanArea(area, areahandle, 0);

}

/* ----------------------------------------------- */

void ReplyOther(MMSG * curmsg, AREA * area)
{
    AREA *toarea, *preselarea = cfg.first;
    MSGA *toareahandle;
    char temp[100];
    char *s;

    savescreen();
    cls();

    if (curmsg->ctxt
        && ((s = GetCtrlToken(curmsg->ctxt, "AREA:")) != NULL))
    {
        if (FindArea(s + 5) != NULL)
            preselarea = FindArea(s + 5);
        else if (FindArea(s + 6) != NULL)
            preselarea = FindArea(s + 6);

        mem_free(s);
    }

    toarea = SelectArea(cfg.first, 1, preselarea);

    putscreen();

    if (toarea == NULL)
        return;

    if (toarea->readonly)
    {
        Message("This is a Read-Only area!", -1, 0, YES);
        return;
    }

    sprintf(temp, "* Reply to a message in %s.", area->tag);

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
        get_custom_info(toarea);
    }
    else
    {
//      Message("You selected the same area!", -1, 0, YES);
        stuffkey(cREADreply);
        return;
    }

    MakeMessage(curmsg, toarea, toareahandle, 1, 0, temp);

    MsgCloseArea(toareahandle);

    get_custom_info(area);

}

/* ----------------------------------------------- */

// bodytoo == -1 means attributes only

void ChangeMessage(MMSG * curmsg, AREA * area, MSGA * areahandle,
                   int bodytoo)
{
    int editret = 0, zonegated = check_gated(curmsg);
    char *newkludges;
    MMSG *tempmsg;
    MSGH *msghandle;
    MIS *mis;
    dword thisone;
    RAWBLOCK *rawbody = NULL;

    if (area->readonly)
    {
        Message("This is a Read-Only area!", -1, 0, YES);
        return;
    }

    thisone = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));

    firstmemline = lastmemline = NULL;
    curtxtline = 0;

    // First we re-read the message. It's status might have changed since
    // we read it.

    if ((msghandle =
         MsgOpenMsg(areahandle, MOPEN_READ,
                    MsgGetCurMsg(areahandle))) == NULL)
    {
        sprintf(msg, "Error reading message attributes!");
        Message(msg, -1, 0, YES);
        showerror();
        return;
    }

    mis = mem_calloc(1, sizeof(MIS));

    if ((MsgReadMsg(msghandle, mis, 0L, 0L, NULL, 0L, NULL) == -1) ||
        (MsgCloseMsg(msghandle) == -1))
    {
        Message("Error reading message!", -1, 0, 1);
        showerror();
        FreeMIS(mis);
        mem_free(mis);
        return;
    }

    // Put the newly read attribs on the 'old' header, so we can dump the
    // new MIS struct and forget about it.

    curmsg->mis.attr1 = mis->attr1;
    curmsg->mis.attr2 = mis->attr2;
    FreeMIS(mis);
    mem_free(mis);

    /* Check if the message is sent already.. */

    if ((curmsg->mis.attr1 & aSENT) || (curmsg->mis.attr1 & aSCANNED))
    {
        if (confirm("Message already sent/scanned! Edit anyway? (y/N)") ==
            0)
            return;
    }

    if (bodytoo != -1)
    {
        curmsg->mis.msgwritten = JAMsysTime(NULL);

        if (area->type == NETMAIL)
            editret =
                EditHeader(areahandle, curmsg, 1, custom.aka, 0, area);
        else
            editret = EditHeader(areahandle, curmsg, 0, 0, 0, area);

        if (editret == ABORT)   /* ESC pressed */
            return;
    }
    else                        /* bodytoo == -1, set attributes only */
    {
        /* Edit attribs until <ESC> or <ENTER> */
        while (((editret = SetAttributes(curmsg, area->base, 0)) != ESC)
               && (editret != 0))
            ;

        if (editret == ESC)
            return;
    }

    if (bodytoo == 1)
    {
        MakeText(curmsg, NULL); /* Literally write out text */

        /* We can't mem_free up mem here, might need original text if no
           body change.. */

        rawbody = GetBody(area, areahandle, curmsg, make_origin(custom.aka), firstmemline, 1, curtxtline); /* could 
                                                                                                              be 
                                                                                                              NULL, 
                                                                                                              then 
                                                                                                              no 
                                                                                                              TEXT 
                                                                                                              write 
                                                                                                              in 
                                                                                                              MsgWrite 
                                                                                                              Msg 
                                                                                                              below.. 
                                                                                                            */

        if (rawbody == NULL)
        {
            Message("Change aborted.", 10, 0, YES);
            return;
        }
    }

    /* We may have no body, but need to rewrite the entire message, */
    /* to allow the kludges to grow (for longer msgid addresses, or */
    /* zonegating, DIR flags etc.) */
    /* If we have no raw body, get it..  */

    if (rawbody == NULL)
    {
        tempmsg =
            GetRawMsg(MsgUidToMsgn(areahandle, thisone, UID_EXACT),
                      areahandle, READALL, 1);
        if (tempmsg)
        {
            rawbody = tempmsg->firstblock; /* Get raw text */
            RemoveTOline(rawbody, NULL);
            tempmsg->firstblock = NULL; /* So this won't be released.. */
            ReleaseMsg(tempmsg, 1);
        }
    }

    /* Clean out the old kludges.. */

    if (curmsg->ctxt)
    {
        RemoveFromCtrl(curmsg->ctxt, "FLAGS");
        RemoveFromCtrl(curmsg->ctxt, "MSGID");
        if (!zonegated)
            RemoveFromCtrl(curmsg->ctxt, "INTL");
    }

    /* Make us some new ones (with correct address, might have changed) */

    newkludges = MakeKludge(NULL, &curmsg->mis, area->type == NETMAIL);

    /* Add the old one (might be REPLY kludge left), mem_free old ones */

    if (zonegated)
        RemoveFromCtrl(newkludges, "INTL");

    RemoveFromCtrl(newkludges, "PID");

    if (curmsg->ctxt)
    {
        // mem_realloc, with extra space for expansion (INTL for example)
        if ((newkludges =
             mem_realloc(newkludges,
                         strlen(newkludges) + strlen(curmsg->ctxt) +
                         256)) == NULL)
            Message("Out of memory!", -1, 254, NO);
        strcat(newkludges, curmsg->ctxt);
        if (curmsg->ctxt)
        {
            mem_free(curmsg->ctxt);
            curmsg->ctxt = NULL;
        }
    }

    curmsg->ctxt = newkludges;  /* Put them in place */

    if (curmsg->mis.attr1 & aLOCAL) // Only for local messages checking..
        check_cc(area, areahandle, &curmsg->mis, &rawbody);

    if (area->type == NETMAIL)
    {
        /* Checked CC: first! Don't mess up zonegate etc!! */
        zonegate(&curmsg->mis, curmsg->ctxt, 0, area->base);
    }


    SaveMessage(areahandle,
                area,
                &curmsg->mis,
                rawbody,
                curmsg->ctxt,
                MsgUidToMsgn(areahandle, thisone, UID_EXACT), 0);

    FreeBlocks(rawbody);

    add_tosslog(area, areahandle);

    ScanArea(area, areahandle, 0);

}


/* ----------------------------------------------- */

void MakeQuote(MMSG * curmsg, AREA * area, word replytype, MIS * newmis,
               char *addtop)
{
    FILE *repfile;
    LINE *curline, *nextline, *quote, *lineptr;
    char temp[350], *fbuf = NULL, inits[10], msgfile[100];
    int len;
    RAWBLOCK *blk;

    if (!cfg.usr.internal_edit)
    {
        sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
        if (!(repfile = fopen(msgfile, "wt")))
        {
            Message("Error opening output file", -1, 0, YES);
            return;
        }

        fbuf = mem_malloc(4096);
        setvbuf(repfile, fbuf, _IOFBF, 4096);
    }

//   if( (curmsg->status & REPLYTO) && (area->type == NETMAIL || area->type == MAIL) )
//      {
//      sprintf(temp, "TO: %s\r\r", curmsg->rep_name);
//      addit(temp, repfile);
//      }

    if (addtop != NULL)
    {
        sprintf(temp, "%s\r\r", addtop);
        addit(temp, repfile);
    }

    if (replytype == 4)         // Bouncereply..
    {
        if (custom.hello[0] != '\0')
        {
            sprintf(temp, "%s\r\r", make_hello(newmis));
            addit(temp, repfile);
        }

        // calculate position of cursor in the text.
        for (curline = firstmemline; curline; curline = curline->next)
            curtxtline++;

        if (custom.signoff[0] != '\0')
        {
            sprintf(temp, "\r%s",
                    expand(custom.signoff, &curmsg->mis, newmis));
            addit(temp, repfile);
        }

        sprintf(temp, "\r\rOriginal message:\r\r--\rDate: %s\rAttr: %s\r",
                MakeT(curmsg->mis.msgwritten, DATE_HDR),
                attr_to_txt(curmsg->mis.attr1, curmsg->mis.attr2));
        addit(temp, repfile);

        sprintf(temp, "From: %s, (%s)\r", curmsg->mis.from, FormAddress(&curmsg->mis.origfido)); // static 
                                                                                                 // buffer!

        addit(temp, repfile);

        sprintf(temp, "To  : %s, (%s)\rSubj: %s\r--\r", curmsg->mis.to, FormAddress(&curmsg->mis.destfido), // static 
                                                                                                            // buffer!
                curmsg->mis.subj);

        addit(temp, repfile);

        MakeText(curmsg, repfile);

        if (cfg.usr.status & NETTEAR)
        {
            sprintf(temp, "\r--- %s",
                    (cfg.usr.status & EMPTYTEAR) ? "" : myname);
            addit(temp, repfile);
        }

        if (!cfg.usr.internal_edit)
        {
            fclose(repfile);
            mem_free(fbuf);
        }

        return;
    }

    if (custom.rephello[0] != '\0')
    {
        sprintf(temp, "%s\r\r",
                make_rephello(&curmsg->mis, newmis, replytype));
        addit(temp, repfile);
    }

    // calculate position of cursor in the text
    for (curline = firstmemline; curline; curline = curline->next)
        curtxtline++;

    if (area->type == NEWS || area->type == MAIL)
        strcpy(inits, " ");
    else
        strcpy(inits, getinitials(curmsg->mis.from));

    curline = curmsg->txt;

    while (curline)
    {
        if (curline->status & (ORIGIN | TEAR))
        {
            /* escape the tearline and origin line */
            if (strncmp(curline->ls, "---", 3) == 0 ||
                strncmp(curline->ls, " * Origin: ", 11) == 0)
            {
                curline->ls[1] = '+';
            }
        }

        if (curline->status & KLUDGE)
        {
            if (cfg.usr.status & SHOWKLUDGES)
            {
                sprintf(temp, "%s> @%s\r", inits,
                        *curline->ls ==
                        0x01 ? curline->ls + 1 : curline->ls);
                addit(temp, repfile);
            }
        }
        else if (curline->status & QUOTE)
        {
            char *quoteptr;
            if ((quoteptr = strchr(curline->ls, '>')) != NULL)
            {
                *quoteptr = 0;
                ++quoteptr;
                if (*curline->ls == ' ')
                    sprintf(temp, "%s>>%s\r", curline->ls, quoteptr);
                else
                    sprintf(temp, " %s>>%s\r", curline->ls, quoteptr);
            }
            else
                sprintf(temp, "%s\r", curline->ls);
            addit(temp, repfile);
        }
        else if ((len = curline->len) < (maxx - 20))
        {
            if (len)
                sprintf(temp, "%s> %s\r", inits, curline->ls);
            else
                sprintf(temp, "\r");

            addit(temp, repfile);
        }
        else                    /* Build a paragraph of text, so we can
                                   wrap it for quoting */
        {
            blk = InitRawblock(2048, 1024, 64000u);
//        blk = InitRawblock(200, 200, 64000u);

            AddToBlock(blk, curline->ls, curline->len);
            if ((blk->curend[0] != ' ') && !(curline->status & NOSPACE))
                AddToBlock(blk, " ", 1);

            while (1)           /* and append until end of paragraph */
            {
                nextline = curline->next;

                if ((nextline == NULL) ||
                    (nextline->status & (QUOTE | KLUDGE)) ||
                    (nextline->ls[0] == '\0'))
                    break;

                if (curmsg->txt == curline)
                {
                    curmsg->txt = curline->next;
                }
                else
                {
                    Message("Not first! (wrapper)", -1, 0, YES);
                }
                if (curline->ls)
                    mem_free(curline->ls);
                curline->ls = NULL;
                mem_free(curline);

                curline = nextline; /* got next line, nextline == NULL was 
                                       catched in beginning */

                AddToBlock(blk, curline->ls, curline->len);

                if (curline->len < (maxx - 20)) /* short line, end of
                                                   paragraph */
                    break;      /* End of adding to paragraph text */
                else            /* set up for more adding */
                {
                    if ((blk->curend[0] != ' ')
                        && !(curline->status & NOSPACE))
                        AddToBlock(blk, " ", 1);
                }
            }

// !!!!!!!!!        lineptr = quote = wraptext(blk->txt, 60, 1, 0);  // 1 == also give last line back
            lineptr = quote = wraptext(blk->txt, 72 - strlen(inits) - 2, 1, 0); // 1 
                                                                                // == 
                                                                                // also 
                                                                                // give 
                                                                                // last 
                                                                                // line 
                                                                                // back

            mem_free(blk->txt);
            mem_free(blk);

            while (lineptr)
            {
                if (lineptr->len)
                    sprintf(temp, "%s> %s\r", inits, lineptr->ls);
                else
                    sprintf(temp, "\r");

                addit(temp, repfile);
                lineptr = lineptr->next;
            }

            FreeLines(quote);
        }

        if (curline)
        {
            if (curmsg->txt == curline)
            {
                curmsg->txt = curline->next;
            }
            else
            {
                Message("Not first!", -1, 0, YES);
            }
            if (curline->ls)
                mem_free(curline->ls);
            curline->ls = NULL;
            nextline = curline->next;
            mem_free(curline);
            curline = nextline;
        }
    }

    EndMessage(area, &curmsg->mis, newmis, repfile);

    if (!cfg.usr.internal_edit)
    {
        fclose(repfile);
        mem_free(fbuf);
    }

}


/* ----------------------------------------------- */

char *MakeKludge(MMSG * curmsg, MIS * mis, int netmail)
{
/*	static char counter=0; */
    static time_t t;
    char *buffer, temp[80];

    if (!t)
    {
        (void)time(&t);
        t = (t << 4);
    }

    buffer = mem_calloc(1, 256);
    sprintf(buffer, "\01MSGID: %hu:%hu/%hu.%hu %08lx", mis->origfido.zone,
            mis->origfido.net,
            mis->origfido.node, mis->origfido.point, t++);

    if (curmsg && (strcmpi(curmsg->id, "") != 0))
    {
        sprintf(temp, "\01REPLY: %s", curmsg->id);
        strcat(buffer, temp);
    }

    if (cfg.usr.status & EMPTYTEAR)
    {
        strcat(buffer, "\01PID: ");
        strcat(buffer, myname);
    }

    if (netmail && (cfg.usr.status & INTLFORCE))
    {
        sprintf(temp, "\01INTL %hu:%hu/%hu %hu:%hu/%hu",
                mis->destfido.zone,
                mis->destfido.net,
                mis->destfido.node,
                mis->origfido.zone, mis->origfido.net, mis->origfido.node);
        strcat(buffer, temp);
    }

    return buffer;

}


/* ----------------------------------------------- */


void MakeText(MMSG * curmsg, FILE * parameterfile)
{

    FILE *repfile;
    LINE *curline, *lastline;
    char temp[256], *fbuf = NULL, msgfile[100];

    if (parameterfile != NULL)
        repfile = parameterfile;

    if (!parameterfile && !cfg.usr.internal_edit)
    {
        sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
        if (!(repfile = fopen(msgfile, "wt")))
        {
            Message("Error opening output file!", -1, 0, YES);
            return;
        }

        fbuf = mem_malloc(4096);
        setvbuf(repfile, fbuf, _IOFBF, 4096);
    }

    curline = curmsg->txt;
    while (curline)
    {
        if (!((curline->status & KLUDGE) && !parameterfile))
        {
            if (curline->status & KLUDGE) /* Don't put your own kludges in 
                                             when change! */
                curline->ls[0] = '@';

            if (parameterfile && (curline->status & (ORIGIN | TEAR))) // Invalidate 
                                                                      // first...
            {
                if (strncmpi(curline->ls, "---", 3) == 0)
                    memset(curline->ls, '_', 3);
                else if (strncmpi(curline->ls, " * Origin: ", 11) == 0)
                    curline->ls[1] = '-';
            }

            if (!cfg.usr.internal_edit)
            {
                sprintf(temp, "%s\n", curline->ls);
                fputs(temp, repfile);
            }
            else
            {
                strcpy(temp, curline->ls);
                if (curline->status & HCR)
                    strcat(temp, "\r");

                add_text(temp);
            }
        }

        lastline = curline;
        curline = curline->next;
        if (lastline->ls)
        {
            mem_free(lastline->ls);
            lastline->ls = NULL;
        }
        if (curmsg->txt == lastline)
            curmsg->txt = lastline->next;
        mem_free(lastline);
    }

    if (!parameterfile && !cfg.usr.internal_edit)
    {
        fclose(repfile);
        mem_free(fbuf);
    }

}


/* ------------------------------------------------------------------------- */
/* -- Expand address string using 'defaults' for your AKA, 0 if aka == -1 -- */
/* ------------------------------------------------------------------------- */

void address_expand(char *line, NETADDR * fa, int aka)
{
    char *c;

    if ((c = strchr(line, ':')) != NULL) /* Do we have a zone number? */
    {
        fa->zone = atoi(line);
        line = c + 1;
    }
    else if (aka != -1)         /* No, default to our own zone.. */
    {
        fa->zone = cfg.usr.address[aka].zone;
    }

    if ((c = strchr(line, '/')) != NULL) /* Do we have a net number? */
    {
        fa->net = atoi(line);
        line = c + 1;
    }
    else if (aka != -1)
    {
        fa->net = cfg.usr.address[aka].net;
    }

    if ((c = strchr(line, '.')) != NULL) /* Do we have a pointnumber? */
    {
        if (c == line)          /* String is like: ".8" */
        {
            if (aka != -1)
                fa->node = cfg.usr.address[aka].node;
            fa->point = atoi(++c);
        }
        else                    /* String is like: "527.8" */
        {
            fa->node = atoi(line);
            fa->point = atoi(++c);
        }
    }
    else                        /* There's no point(number) */
    {
        fa->node = atoi(line);
        fa->point = 0;
    }

}

/* ------------------------------------------------- */
/* -- Get someone's initials for quoting ( GvE> ) -- */
/* ------------------------------------------------- */


char *getinitials(char *name)
{
    static char inits[20];
    char *charptr, *result, *tempptr;

    memset(inits, '\0', sizeof(inits));
    charptr = name;
    result = inits;

    if (cfg.usr.internal_edit)  // We can add a leading space for int.
                                // edit
        *result++ = ' ';        // Not ext. edit, editors may align to it
                                // :-(

    *result++ = *name;

    if (((tempptr = strchr(charptr, '@')) == NULL) &&
        ((tempptr = strchr(charptr, '%')) == NULL))
    {
        while (*charptr)
        {
            if (*charptr == ' ' || *charptr == '.' || *charptr == '_')
            {
                if (!(strncmpi(charptr, ". ", 2) == 0 ||
                      strncmpi(charptr, " /", 2) == 0))
                    *result++ = *++charptr;
            }

            if (*charptr == '/') // Stop! 'Malte Erikson / Beta tester'
                                 // style
                break;

            if (*charptr != '\0')
                charptr++;
        }
    }
    else
    {
        if (*(tempptr + 1) != ' ') // Don't do this if it is a space.
            *result++ = *(++tempptr);
    }

    *result = '\0';

    return inits;

}


/* ------------------------------------------------- */

void MakeTemplate(MIS * mis, AREA * area)
{
    FILE *repfile;
    char temp[256], msgfile[100];
    LINE *curline;

    if (!cfg.usr.internal_edit)
    {
        sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
        if (!(repfile = fopen(msgfile, "wt")))
        {
            Message("Error opening output file!", -1, 0, YES);
            return;
        }
    }

//   /* Do we need to write a Usenet address at the top? */
//
//   if(usenet_address[0] != '\0')
//     {
//     sprintf(temp, "TO: %s\r\r", usenet_address);
//     addit(temp, repfile);
//     }

    if (custom.hello[0] != '\0')
    {
        sprintf(temp, "%s\r\r", make_hello(mis));
        addit(temp, repfile);
    }

    // calculate position of cursor in the text
    for (curline = firstmemline; curline; curline = curline->next)
        curtxtline++;

    EndMessage(area, mis, mis, repfile);

    if (!cfg.usr.internal_edit)
        fclose(repfile);

}


/* ---------------------------------------------------------- */


char *make_origin(int aka)
{
    static char temp[125];
    char address[40];


    sprintf(address, "(%s)", FormAddress(&cfg.usr.address[aka]));

    custom.origin[67 - strlen(address)] = '\0';

    sprintf(temp, "\r--- %s\r * Origin: %s %s\r",
            (cfg.usr.status & EMPTYTEAR) ? "" : myname,
            custom.origin, address);

    return temp;
}

/* ------------------------------------------------------- */

char *make_rephello(MIS * mis, MIS * newmis, word replytype)
{
    static char hello[350];


    if (replytype == 2)
        strcpy(hello, expand(custom.followhello, mis, newmis));
    else
        strcpy(hello, expand(custom.rephello, mis, newmis));

    return hello;

}


/* ----------------------------------------------------- */

char *make_hello(MIS * mis)
{
    static char hello[350];


    strcpy(hello, expand(custom.hello, mis, mis));

    return hello;

}


/* ----------------------------------------------------------- */
/* - Expand an 'hellostring', so fill in %to and %from names - */
/* ----------------------------------------------------------- */


char *expand(char *line, MIS * mis, MIS * newmis)
{
    static char temp[400];
    char tempaddress[50];
    char *lineptr = line, *tempptr = temp;
    JAMTM *t = JAMsysLocalTime(&mis->msgwritten);
    char *rightptr;

    memset(temp, '\0', sizeof(temp));

    if ((t->tm_mon < 0) || (t->tm_mon > 11))
        t->tm_mon = 0;

    while (*lineptr)
    {
        if (*lineptr == '%')
        {
            lineptr++;

            if (*lineptr == '%')
                *tempptr++ = *lineptr++;

            else if (strncmpi(lineptr, "to", 2) == 0) /* %to */
            {
                if (mis->destinter[0] != '\0')
                {
                    memcpy(tempptr, mis->destinter,
                           strlen(mis->destinter));
                    tempptr += strlen(mis->to);
                }
                else
                {
                    memcpy(tempptr, mis->to, strlen(mis->to));
                    tempptr += strlen(mis->to);
                }
                lineptr += 2;
            }

            else if ((strncmpi(lineptr, "thisto", 6) == 0) && newmis) /* %to 
                                                                       */
            {
                if (newmis->destinter[0] != '\0')
                {
                    memcpy(tempptr, newmis->destinter,
                           strlen(newmis->destinter));
                    tempptr += strlen(newmis->to);
                }
                else
                {
                    memcpy(tempptr, newmis->to, strlen(newmis->to));
                    tempptr += strlen(newmis->to);
                }

                lineptr += 6;
            }

            else if (strncmpi(lineptr, "fto", 3) == 0) /* %fto */
            {
                if (mis->destinter[0] != '\0')
                    rightptr = mis->destinter;
                else
                    rightptr = mis->to;

                /* Only copy till first space */
                if (strchr(rightptr, '@') == NULL)
                {
                    memcpy(tempptr, rightptr, strcspn(rightptr, " "));
                    tempptr += strcspn(rightptr, " ");
                }
                else
                {
                    memcpy(tempptr, rightptr, strcspn(rightptr, "@"));
                    tempptr += strcspn(rightptr, "@");
                }
                lineptr += 3;
            }

            else if ((strncmpi(lineptr, "thisfto", 7) == 0) && newmis) /* %fto 
                                                                        */
            {
                if (newmis->destinter[0] != '\0')
                    rightptr = newmis->destinter;
                else
                    rightptr = newmis->to;

                /* Only copy till first space */
                if (strchr(rightptr, '@') == NULL)
                {
                    memcpy(tempptr, rightptr, strcspn(rightptr, " "));
                    tempptr += strcspn(rightptr, " ");
                }
                else
                {
                    memcpy(tempptr, rightptr, strcspn(rightptr, "@"));
                    tempptr += strcspn(rightptr, "@");
                }
                lineptr += 7;
            }

            else if (strncmpi(lineptr, "from", 4) == 0) /* %from */
            {
                memcpy(tempptr, mis->from, strlen(mis->from));
                lineptr += 4;
                tempptr += strlen(mis->from);
            }

            else if ((strncmpi(lineptr, "thisfrom", 8) == 0) && newmis) /* %from 
                                                                         */
            {
                memcpy(tempptr, newmis->from, strlen(newmis->from));
                lineptr += 8;
                tempptr += strlen(newmis->from);
            }

            else if (strncmpi(lineptr, "ffrom", 5) == 0) /* %ffrom */
            {
                /* Only copy till first space.. */
                if (strchr(mis->from, '@') == NULL)
                {
                    memcpy(tempptr, mis->from, strcspn(mis->from, " "));
                    tempptr += strcspn(mis->from, " ");
                }
                else
                {
                    memcpy(tempptr, mis->from, strcspn(mis->from, "@"));
                    tempptr += strcspn(mis->from, "@");
                }
                lineptr += 5;
            }

            else if ((strncmpi(lineptr, "thisffrom", 9) == 0) && newmis) /* %ffrom 
                                                                          */
            {
                /* Only copy till first space.. */
                if (strchr(newmis->from, '@') == NULL)
                {
                    memcpy(tempptr, newmis->from,
                           strcspn(newmis->from, " "));
                    tempptr += strcspn(newmis->from, " ");
                }
                else
                {
                    memcpy(tempptr, newmis->from,
                           strcspn(newmis->from, "@"));
                    tempptr += strcspn(newmis->from, "@");
                }
                lineptr += 9;
            }

            else if (strncmpi(lineptr, "subj", 4) == 0) /* %subj */
            {
                memcpy(tempptr, mis->subj, strlen(mis->subj));
                lineptr += 4;
                tempptr += strlen(mis->subj);
            }

            else if (strncmpi(lineptr, "orig", 4) == 0) /* %orig */
            {
                strcpy(tempaddress, FormAddress(&mis->origfido));
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 4;
                tempptr += strlen(tempaddress);
            }

            else if (strncmpi(lineptr, "dest", 4) == 0) /* %dest */
            {
                strcpy(tempaddress, FormAddress(&mis->destfido));
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 4;
                tempptr += strlen(tempaddress);
            }

            else if (strncmpi(lineptr, "time", 4) == 0) /* %time */
            {
                sprintf(tempaddress, "%2.2i:%2.2i", t->tm_hour, t->tm_min);
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 4;
                tempptr += strlen(tempaddress);
            }

            else if (strncmpi(lineptr, "year", 4) == 0) /* %year */
            {
                sprintf(tempaddress, "%4.4i", t->tm_year + 1900);
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 4;
                tempptr += strlen(tempaddress);
            }
            else if (strncmpi(lineptr, "syear", 5) == 0) /* %syear */
            {
                sprintf(tempaddress, "%2.2i", (t->tm_year + 1900) % 100);
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 5;
                tempptr += strlen(tempaddress);
            }
            else if (strncmpi(lineptr, "mmon", 4) == 0) /* %mmon */
            {
                sprintf(tempaddress, "%2.2i", t->tm_mon + 1);
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 4;
                tempptr += strlen(tempaddress);
            }
            else if (strncmpi(lineptr, "mon", 3) == 0) /* %mon */
            {
                sprintf(tempaddress, "%s", months_ab[t->tm_mon]);
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 3;
                tempptr += strlen(tempaddress);
            }

            else if (strncmpi(lineptr, "day", 3) == 0) /* %day */
            {
                sprintf(tempaddress, "%2.2i", t->tm_mday);
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 3;
                tempptr += strlen(tempaddress);
            }

            else if (strncmpi(lineptr, "dow", 3) == 0) /* %dow */
            {
                sprintf(tempaddress, "%s", weekday_ab[t->tm_wday]);
                memcpy(tempptr, tempaddress, strlen(tempaddress));
                lineptr += 3;
                tempptr += strlen(tempaddress);
            }

        }
        else if (*lineptr == '\\' && *(lineptr + 1) == 'n')
        {
            *tempptr++ = '\r';
            lineptr += 2;
        }
        else
            *tempptr++ = *lineptr++;
    }

    return temp;
}


/* Walk through the list of aliases (macro's...) and check if    */
/* there is a match. If there is, fill header, and return number */
/* of items filled in (that can be skipped in editheader..)      */


int check_alias(MIS * mis)
{
    MACROLIST *thismacro = cfg.firstmacro;

    while (thismacro)
    {
        if (!strcmpi(thismacro->macro, mis->to))
        {
            if (thismacro->usenet[0] != '\0')
                strncpy(mis->destinter, thismacro->usenet, 100);

            strcpy(mis->to, thismacro->toname);
            if (thismacro->toaddress.zone == 0)
                return 0;
            mis->destfido = thismacro->toaddress;
            if (thismacro->subject[0] == '\0')
                return 1;
            strcpy(mis->subj, thismacro->subject);
            return 2;
        }
        thismacro = thismacro->next;
    }

    return 0;                   /* Not found.. */
}


/*
 * -------------------------------------------------------------------
 * Check if a message needs to be zonegated (only if user set it up).
 * Add INTL kludge & rewrite header, don't gate freqs etc
 * -------------------------------------------------------------------
*/

void zonegate(MIS * mis, char *kludges, int showinfo, dword base)
{
    char temp[100];
    dword mask = aCRASH | aHOLD | aFRQ | aFILE | aDIR | aIMM;


    if ((mis == NULL) || (kludges == NULL))
        return;

    if (cfg.usr.zonegate == 0)
        return;                 /* User doesn't want zonegating */

    if (mis->attr1 & mask)
        return;                 /* Don't gate crash or hold or frq/attach
                                   etc */

    if (mis->origfido.zone != mis->destfido.zone) /* Inter zone msg! */
    {
        if ((cfg.usr.zonegate == 1) || (gateconfirm(mis, showinfo) != 0))
        {
            RemoveFromCtrl(kludges, "INTL");

            // If this is JAM, and we want to use the Zonegate bit, set
            // it!
            if ((base & MSGTYPE_JAM) && (cfg.usr.status & JAMZGTBIT))
            {
                mis->attr1 |= aZGT;
                return;         // No INTL kludge needed in that case!
            }

            sprintf(temp, "\01INTL %hu:%hu/%hu %hu:%hu/%hu",
                    mis->destfido.zone,
                    mis->destfido.net,
                    mis->destfido.node,
                    mis->origfido.zone,
                    mis->origfido.net, mis->origfido.node);

            mis->destfido.node = mis->destfido.zone;
            mis->destfido.zone = mis->origfido.zone;
            mis->destfido.net = mis->origfido.zone;
            strcat(kludges, temp);
        }
    }

}


/* Check a message to see if it is a zonegated message             */
/* Now used to see if we can remove INTL kludge for change message */
/* function                                                        */

int check_gated(MMSG * msg)
{
    char *intl;
    int destzone = msg->mis.origfido.zone;


    if (msg->mis.origfido.zone != msg->mis.destfido.zone)
        return 0;

    if ((intl = stristr(msg->ctxt, "INTL")) == NULL)
        return 0;

    sscanf(intl, "INTL %hu:", &destzone);

    if (destzone != msg->mis.origfido.zone)
        return 1;

    return 0;

}



// ====================================================================
/* Add text to the lines in memory, for the internal editor */


void add_text(char *line)
{
    LINE *thisline, *lineptr, *lastline, *oldlast;
    char *dupline, *charptr;
    int quoted = 0;
    char quote[20], temp[MAX_SCREEN_WIDTH];

    charptr = dupline = mem_strdup(line);

    /* Now, if the previous line did NOT end in a HCR, we need to append
       this text to it, and rewrap the whole thing.. */

    if (firstmemline)           /* Only if we actually have previous lines 
                                 */
    {
        if (!(lastmemline->status & HCR)) /* No HCR? Then append */
        {
            charptr = mem_calloc(1, strlen(dupline) + lastmemline->len + 2); // + 
                                                                             // 2 
                                                                             // for 
                                                                             // trailing 
                                                                             // zero 
                                                                             // and 
                                                                             // possible 
                                                                             // added 
                                                                             // space 
                                                                             // */
            strcpy(charptr, lastmemline->ls); // start with previous
                                              // line..

            /* Check if we need to add a space */
            if ((charptr[strlen(charptr) - 1] != ' ') &&
                (dupline[0] != ' ') && (!(lastmemline->status & NOSPACE)))
                strcat(charptr, " ");

            strcat(charptr, dupline); // add new part..
            mem_free(dupline);
            dupline = charptr;

            if (firstmemline == lastmemline) /* Are we changing/rewrapping 
                                                the first line? */
                firstmemline = NULL;
            oldlast = lastmemline; // Save value, we need to mem_free() it 
                                   // later
            if (lastmemline->prev)
            {
                lastmemline->prev->next = NULL; // Destroy forward link
                lastmemline = lastmemline->prev; // Set back last line to
                                                 // existing line
            }

            if (oldlast->ls)
                mem_free(oldlast->ls);
            mem_free(oldlast);
        }
    }

    // We will now wrap the text (if necessary). If this is a quote, we
    // wrap at a narrower margin, because we need to add the quotestring
    // to
    // the lines following the first. Wrapping at 78 will lead to trouble
    // then, because 78 + strlen(quotestring) will exceed screenwidth.

    if (IsQuote(dupline) && (strlen(dupline) > 78))
        lastline = lineptr = thisline = wraptext(dupline, 70, 1, 0);
    else
        lastline = lineptr = thisline = wraptext(dupline, maxx - 2, 1, 0);

    mem_free(dupline);

    checklines(thisline);       // Set status

    if ((lineptr->status & QUOTE) &&
        (lineptr->next) && (!(lineptr->status & HCR)))
    {
        quoted = 1;
        lineptr->status |= HCR;
        get_quote_string(lineptr->ls, quote);
    }

    /* So we copy the lines... */
    while (lineptr)
    {
        if (quoted && (!(lineptr->status & QUOTE))
            && (lineptr->ls[0] != '\0'))
        {
            sprintf(temp, "%s%s", quote, lineptr->ls);
            if (lineptr->ls)
                mem_free(lineptr->ls);
            lineptr->ls = mem_strdup(temp);
            lineptr->len = strlen(temp);
            lineptr->status &= ~NORMTXT;
            lineptr->status |= (HCR | QUOTE);
        }

        lastline = lineptr;
        lineptr = lineptr->next;
    }

    if (!firstmemline)
        firstmemline = thisline;
    else
    {
        lastmemline->next = thisline;
        thisline->prev = lastmemline;
    }

    /* Find new end of textlines */

    lastmemline = thisline;
    while (lastmemline->next != NULL)
        lastmemline = lastmemline->next;

}



/* -------------------------------------------------------- */
/* - Write a (forward) reply link to the original message - */
/* -       Returns 0 if all is well, -1 on error          - */
/* -------------------------------------------------------- */


int WriteReplyLink(AREA * area, MSGA * areahandle, dword orig, dword new)
{
    MIS replymis;
    int n;
    MSGH *msghandle;


    if (!(area->base & MSGTYPE_JAM))
    {
        if ((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, orig)) == NULL)
            return -1;

        if ((MsgReadMsg(msghandle, &replymis, 0, 0, NULL, 0, NULL)) != -1)
        {
            for (n = 0; n < 9; n++)
            {
                if (replymis.replies[n] == 0) /* Look for a mem_free
                                                 'slot' */
                {
                    replymis.replies[n] = new;
                    break;
                }
            }
            MsgWriteMsg(msghandle, 0, &replymis, NULL, 0, 0, 0, NULL);
        }

        MsgCloseMsg(msghandle);
        FreeMIS(&replymis);

        return 0;
    }

    // JAM, fix up strange reply stuff now..

    if ((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, orig)) == NULL)
        return -1;

    if ((MsgReadMsg(msghandle, &replymis, 0, 0, NULL, 0, NULL)) == -1)
    {
        MsgCloseMsg(msghandle);
        return -1;
    }

    if (replymis.replies[0] == 0) // Replyfirst?
    {
        replymis.replies[0] = new;
        MsgWriteMsg(msghandle, 0, &replymis, NULL, 0, 0, 0, NULL);
        MsgCloseMsg(msghandle);
        return 0;
    }

    MsgCloseMsg(msghandle);

    // If we get here, if we have to search replynext fields for an empty
    // space

    /* First init with start value: the first reply: */
    replymis.nextreply = replymis.replies[0];

    while (replymis.nextreply != 0) // Loop until space found..
    {
        if ((msghandle =
             MsgOpenMsg(areahandle, MOPEN_RW, replymis.nextreply)) == NULL)
            return -1;

        FreeMIS(&replymis);     // De-alloc old MIS

        if ((MsgReadMsg(msghandle, &replymis, 0, 0, NULL, 0, NULL)) == -1)
        {
            MsgCloseMsg(msghandle);
            return -1;
        }

        if (replymis.nextreply != 0) // Replynext?
            MsgCloseMsg(msghandle);
    }

    replymis.nextreply = new;
    MsgWriteMsg(msghandle, 0, &replymis, NULL, 0, 0, 0, NULL);
    MsgCloseMsg(msghandle);
    FreeMIS(&replymis);

    return 0;

}


// ======================================================================

void near addit(char *temp, FILE * repfile)
{
    if (!cfg.usr.internal_edit)
    {
        rton(temp);
        fputs(temp, repfile);
    }
    else
        add_text(temp);
}


// ==================================================================


void EndMessage(AREA * area, MIS * mis, MIS * newmis, FILE * repfile)
{
    char temp[320];

    if (custom.signoff[0] != '\0')
    {
        sprintf(temp, "\r%s", expand(custom.signoff, mis, newmis));
        addit(temp, repfile);
    }

    if (area->type == ECHOMAIL)
    {
        addit(make_origin(custom.aka), repfile);
    }
    else if (area->type == NETMAIL && (cfg.usr.status & NETTEAR))
    {
        sprintf(temp, "\r--- %s",
                (cfg.usr.status & EMPTYTEAR) ? "" : myname);
        addit(temp, repfile);
    }

}

// =======================================================================

// Convert all '\r' characters in a string to '\n' for disk writing..

void near rton(char *s)
{
    for (; *s; s++)
        if (*s == '\r')
            *s = '\n';
}

// =======================================================================

// preserve ==  1 : preserve original body, untranslated
// preserve == -1 : do NOT do any character translation.

int SaveMessage(MSGA * areahandle, AREA * area, MIS * mis, RAWBLOCK * blk,
                char *kludges, dword no, int preserve)
{
    dword txtsize = 0L, ctrlsize = 0L;
    word append = 0;
    int didlock = 0;
    MSGH *msghandle;
    RAWBLOCK *tmpblk, *interblock = NULL;
    int retval = -1;
    char *newkludges = NULL;
    MIS *newmis;

    if (preserve == 1)          // We need to preserve the original. Make
                                // a copy and use that!
    {
        blk = MakeCopy(blk);
        newmis = mem_calloc(1, sizeof(MIS));
        CopyMIS(mis, newmis);
        mis = newmis;           // Let mis point to the new copy.
    }

    tmpblk = blk;

    // First we need to check Character Translation stuff.

    if (preserve != -1)
        newkludges = TranslateBlocks(area->cswrite, blk, kludges, mis);

    if (mis && mis->destinter[0] != '\0')
    {
        tmpblk = InitRawblock(120, 100, 1024);
        AddToBlock(tmpblk, "TO: ", 4);
        AddToBlock(tmpblk, mis->destinter, -1);
        AddToBlock(tmpblk, "\r", 1);

        // Allow _no_ empty line between TO: and rest of body for those
        // who need it.
        if (cfg.usr.status & INTERNETEMPTY)
            AddToBlock(tmpblk, "\r", 1);

        interblock = tmpblk;    // We use this later to free this block
                                // again.
        tmpblk->next = blk;     // Link with old start..
        blk = tmpblk;           // This is the new start..
    }

    if (!(areahandle->locked))
    {
        if (AttemptLock(areahandle) == -1)
        {
            AskDumpBody(blk);
            if (preserve == 1)
            {
                FreeBlocks(blk); // Free our 'work' copy. Backup points to 
                                 // orig..
                FreeMIS(newmis);
                mem_free(newmis);
            }
            return -1;
        }
        didlock = 1;
    }

    // Calc textlength..

    // First assure Trailing zero..
    if (tmpblk && GetLastChar(tmpblk) == '\0')
        StripLastChar(tmpblk);

    while (tmpblk)
    {
        txtsize += tmpblk->curlen;
        tmpblk = tmpblk->next;
    }

    tmpblk = blk;               // Set it back for later use..

    if (kludges || newkludges)
    {
        if (newkludges)
            ctrlsize = strlen(newkludges) + 1;
        else
            ctrlsize = strlen(kludges) + 1;
    }

    if ((msghandle = MsgOpenMsg(areahandle, MOPEN_CREATE, no)) == NULL)
    {
        Message("Error creating message!", -1, 0, YES);
        showerror();
        AskDumpBody(blk);
        if (preserve == 1)
        {
            FreeBlocks(blk);
            FreeMIS(newmis);
            mem_free(newmis);
        }
        return -1;
    }

    do
    {
        if (MsgWriteMsg(msghandle,
                        append,
                        mis,
                        tmpblk ? tmpblk->txt : NULL,
                        tmpblk ? tmpblk->curlen : 0L,
                        txtsize,
                        ctrlsize, newkludges ? newkludges : kludges))
        {
            Message("Error writing message!", -1, 0, YES);
            showerror();
            AskDumpBody(blk);
            goto close_up;
        }

        if (tmpblk)
            tmpblk = tmpblk->next;
        ctrlsize = 0;           // After first write, no more ctrl..
        kludges = NULL;
        mis = NULL;             // Or header!
        append = 1;             // And we start appending.
    }
    while (tmpblk);

    retval = 0;

  close_up:

    if (msghandle)
        MsgCloseMsg(msghandle);
    if (didlock)
        MsgUnlock(areahandle);
    if (newkludges)
        mem_free(newkludges);

    if (preserve == 1)
    {
        FreeBlocks(blk);
        FreeMIS(newmis);
        mem_free(newmis);
    }
    else if (interblock != NULL) // We added internet address in front.
                                 // Dump mem for that.
    {
        interblock->next = NULL;
        FreeBlocks(interblock);
    }

    return retval;

}


// =================

int gateconfirm(MIS * mis, int showinfo)
{
    int retval;
    BOX *hdrinfo;
    char temp[120];

    if (showinfo)
    {
        hdrinfo =
            initbox(1, 1, 3, 78, cfg.col[Cpopframe], cfg.col[Cpoptext],
                    SINGLE, YES, ' ');
        drawbox(hdrinfo);
        sprintf(temp, "Message to: %s (%s)", mis->to,
                FormAddress(&mis->destfido));
        boxwrite(hdrinfo, 0, 0, temp);
    }

    retval = confirm("Send message through zonegate? (y/N)");

    if (showinfo)
        delbox(hdrinfo);

    return retval;
}

// ===============================================================

RAWBLOCK *MakeCopy(RAWBLOCK * first)
{
    RAWBLOCK *start = NULL, *current, *last;

    while (first)
    {
        current = mem_malloc(sizeof(RAWBLOCK));

        current->txt = mem_malloc(first->curmaxlen);
        memcpy(current->txt, first->txt, first->curmaxlen);

        current->curlen = first->curlen;
        current->maxlen = first->maxlen;
        current->curmaxlen = first->curmaxlen;
        current->delta = first->delta;
        current->full = first->full;
        current->curend = current->txt + current->curlen;
        current->next = NULL;

        if (!start)
            start = current;
        else
            last->next = current;

        last = current;

        first = first->next;
    }

    return start;
}

// ===============================================================
//
// This function checks to see whether a block starts with TO:.
// That may be the case in internet messages. We remove the
// TO: from the body, so we can later rewrite the message with
// a (possibly new) TO: line..
// If 'mis' != NULL, put the found address in the MIS structure
//
// ================================================================

void RemoveTOline(RAWBLOCK * rawbody, MIS * mis)
{
    char *eol;
    char *start;

    if (!rawbody || !rawbody->txt || rawbody->curlen < 4)
        return;

    if (strnicmp(rawbody->txt, "to:", 3) != 0) // NO to: line?
        return;

    // there is a to: line. Remove it.
    eol = strchr(rawbody->txt, '\r');
    if (mis)
    {
        start = rawbody->txt + 3;
        while (isspace(*start))
            start++;
        *eol = '\0';
        strncpy(mis->destinter, start, 100);
    }

    if (!eol)
        return;                 // weird.

    eol++;                      // skip this CR
    if (*eol == '\r')
        eol++;                  // also skip a next newline if it is
                                // there.

    strcpy(rawbody->txt, eol);
    rawbody->curlen = strlen(rawbody->txt);

}


// ===============================================================

void AskDumpBody(RAWBLOCK * blk)
{
    char temp[120];

    sprintf(temp, "%s\\msgtxt.tim", cfg.homedir);
    if (confirm
        ("Save message body to MSGTXT.TIM in timEd directory? (Y/n)"))
        WriteRawBody(blk, temp);

}

// ================================================================
