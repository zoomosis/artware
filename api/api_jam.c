/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#define MSGAPI_HANDLERS

#ifndef __GNUC__
#include <mem.h>
#include <io.h>
#include <dos.h>
#include <share.h>
#else
#include <unistd.h>
#endif

#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dr.h"
#include "prog.h"
#include "alc.h"
#include "old_msg.h"
#include "msgapi.h"
#include "jam.h"
#include "api_jam.h"
#include "api_jamp.h"
#include "apidebug.h"


/* Prototypes */

void near JAM2SQ(struct _msgh *msgh, byte * Subfields);
void near addkludge(struct _msgh *msgh, char *text, int txtlen, char *data,
                    dword len, word * cursize);
void near Init_JAMheader(MSGA * sq);
void near SQ2JAM(MIS * mis, char *ctxt, MSGA * sq, word mode,
                 byte ** SubFieldPtr);

byte near JAMMsgExists(MSGA * sq, UMSGID umsgid);

int near JAMmbOpen(MSGA * sq, byte * name);
int near JAMmbClose(MSGA * sq);

int near JAMReadIdx(MSGA * sq, dword number, JAMIDXREC * idx);
int near JAMReadHeader(MSGA * sq, long offset, JAMHDR * hdr);

int near JAMWriteIdx(MSGA * sq, dword number, JAMIDXREC * idx);
int near JAMWriteHeader(MSGA * sq, long offset, JAMHDR * hdr);

/* Functions stolen from the JAM API (but modified) */

void near JAMmbAddField(byte * start,
                        dword WhatField,
                        size_t DatLen, word * Position, byte * Data);

int near JAMmbUpdateHeaderInfo(MSGA * sq, int WriteIt);

int JAMmbStoreLastRead(MSGA * sq, dword last, dword highest, dword CRC);
dword JAMmbFetchLastRead(MSGA * sq, dword UserCRC, int getlast);

int ParseFido(char *s, NETADDR * addr, char *domain);


static JAMDATA *jamdata;

#define NUMATTR1 26

static dword AttrTable1[NUMATTR1][2] = {

    {aPRIVATE, MSG_PRIVATE},
    {aCRASH, MSG_CRASH},
    {aREAD, MSG_READ},
    {aSENT, MSG_SENT},
    {aFILE, MSG_FILEATTACH},
    {aFWD, MSG_INTRANSIT},
    {aORPHAN, MSG_ORPHAN},
    {aKILL, MSG_KILLSENT},
    {aLOCAL, MSG_LOCAL},
    {aHOLD, MSG_HOLD},
    {aFRQ, MSG_FILEREQUEST},
    {aRRQ, MSG_RECEIPTREQ},
    {aDIR, MSG_DIRECT},
    {aAS, MSG_ARCHIVESENT},
    {aIMM, MSG_IMMEDIATE},
    {aKFS, MSG_KILLFILE},
    {aTFS, MSG_TRUNCFILE},
    {aCFM, MSG_CONFIRMREQ},
    {aLOK, MSG_LOCKED},
    {aZGT, MSG_GATE},
    {aENC, MSG_ENCRYPT},
    {aCOM, MSG_COMPRESS},
    {aESC, MSG_ESCAPED},
    {aFPU, MSG_FPU},
    {aNODISP, MSG_NODISP},
    {aDEL, MSG_DELETED}

};



/*
     ----------------------------

      Function to open a JAM area.

      ----------------------------
*/


MSGA *JAMOpenArea(byte * name, word mode, word type)
{
    MSGA *sq;

    NW(mode);

    msgapierr = MERR_NONE;

    if ((sq = CreateAreaHandle(type)) == NULL)
        return NULL;

    if ((sq->apidata = (JAMDATA *) calloc(1, sizeof(JAMDATA))) == NULL)
    {
        free(sq->api);
        free(sq);
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    /* For debugging : */
    jamdata = sq->apidata;

    *sq->api = JAM_funcs;

    /* Now try to open the message area, using JAM_API functions */

    if (JAMmbOpen(sq, name) == -1) /* -1 == failed! */
    {
        free(sq->apidata);
        free(sq->api);
        free(sq);
        return NULL;
    }

    sq->num_msg = JamData->HdrInfo.ActiveMsgs;
    sq->high_msg = JAMUidToMsgn(sq,
                                (dword) ((filelength(JamData->IdxHandle) /
                                          sizeof(JAMIDXREC)) +
                                         JamData->HdrInfo.BaseMsgNum - 1),
                                UID_PREV);

    return sq;
}


/*
      -----------------------------

      Function to close a JAM area.

      -----------------------------
*/


sword JAMCloseArea(MSGA * sq)
{

    msgapierr = MERR_NONE;

    if (InvalidMh(sq))
        return -1;

    if (sq->locked)
        JAMUnlock(sq);

    JAMmbClose(sq);

    free(sq->api);
    free((JAMDATA *) sq->apidata);
    sq->id = 0L;
    free(sq);

    return 0;
}


/*
      -------------------------------

      Function to open a JAM message.

      -------------------------------
*/


MSGH *JAMOpenMsg(MSGA * sq, word mode, dword msgnum)
{
    struct _msgh *msgh;
    byte *SubFieldPtr = NULL;

    msgapierr = MERR_NONE;

    if (InvalidMh(sq))
        return NULL;

    if (mode == MOPEN_WRITE)
        return NULL;            // Not supported

    if (((mode == MOPEN_CREATE) || (mode == MOPEN_RW)) && (!(sq->locked)))
        return NULL;            // We must be locked if we write!

    /* Translate special 'message numbers' to 'real' msg numbers */

    switch (msgnum)
    {
    case MSGNUM_CUR:
        msgnum = sq->cur_msg;
        break;

    case MSGNUM_PREV:
        msgnum = JAMUidToMsgn(sq, sq->cur_msg - 1, UID_PREV);
        if (msgnum == 0)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
        break;

    case MSGNUM_NEXT:
        msgnum = JAMUidToMsgn(sq, sq->cur_msg + 1, UID_NEXT);
        if (msgnum == 0)
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
        break;

    default:
        break;
    }

    /* If we're not creating a new msg, check existence */

    if ((mode != MOPEN_CREATE) || (msgnum != 0L))
    {
        if ((msgnum < JamData->HdrInfo.BaseMsgNum) ||
            (msgnum > sq->high_msg))
        {
            msgapierr = MERR_NOENT;
            return NULL;
        }
    }


    /* Alloc space for handle, and initialize */

    if ((msgh = calloc(1, sizeof(struct _msgh))) == NULL)
    {
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    msgh->sq = sq;
    msgh->id = MSGH_ID;
    msgh->mode = mode;


    /* Now get message index, to learn about msg */

    if (msgnum != 0)            /* Not a new addition, so we want info */
    {
        sq->cur_msg = msgnum;

        // Read Index

        if (JAMReadIdx(sq, msgnum, &JamData->Idx) == -1)
            goto ErrorExit;

        if (JamData->Idx.HdrOffset == -1) // Deleted message!
            goto ErrorExit;

        // Get header

        if (JAMReadHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
            goto ErrorExit;

        if (JamData->Hdr.Attribute & MSG_DELETED)
        {
            msgapierr = MERR_DELETED;
            goto ErrorExit;
        }

        if (JamData->Hdr.MsgNum != msgnum) // Weird! Corrupt?
        {
            msgapierr = MERR_MSGNO;
            goto ErrorExit;
        }

        // Don't read subfields if we are in MOPEN_RW mode.

        if ((mode == MOPEN_READ) || (mode == MOPEN_RDHDR))
        {
            if (JamData->Hdr.SubfieldLen != 0L)
            {
                if ((SubFieldPtr =
                     calloc(1, JamData->Hdr.SubfieldLen)) == NULL)
                {
                    msgapierr = MERR_NOMEM;
                    goto ErrorExit;
                }

                // No seek here, we just read the header..
                if (read(JamData->HdrHandle,
                         SubFieldPtr,
                         (unsigned)JamData->Hdr.SubfieldLen) !=
                    (int)JamData->Hdr.SubfieldLen)
                {
                    msgapierr = MERR_READ;
                    goto ErrorExit;
                }
            }
        }


        if (mode == MOPEN_CREATE) /* Re-create existing msg, zap out old */
        {
            JamData->Hdr.Attribute |= MSG_DELETED;
            JamData->Hdr.TxtLen = 0L;

            if (JAMWriteHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr)
                == -1)
                goto ErrorExit;
        }

        msgh->hdroffset = JamData->Idx.HdrOffset;
        msgh->txtoffset = JamData->Hdr.TxtOffset;
    }


    if (mode == MOPEN_CREATE)   /* New addition to the message base */
    {
        /* Point to end, to add to msgbase */
        msgh->hdroffset = lseek(JamData->HdrHandle, 0L, SEEK_END);
        msgh->txtoffset = lseek(JamData->TxtHandle, 0L, SEEK_END);

        if (msgnum == 0)
        {
            msgnum =
                (filelength(JamData->IdxHandle) / sizeof(JAMIDXREC)) +
                JamData->HdrInfo.BaseMsgNum;
            sq->high_msg = msgnum;
            sq->num_msg++;
            JamData->HdrInfo.ActiveMsgs += 1; /* New addition, inc # msgs */
        }

        sq->cur_msg = msgnum;
    }

    if (mode != MOPEN_CREATE)   /* So we need to know lengths etc. */
    {
        JAM2SQ(msgh, SubFieldPtr);
    }

    msgh->msgnum = msgnum;

    if (SubFieldPtr)
        free(SubFieldPtr);

    return (MSGH *) msgh;

    // We only get here if we had an error!

  ErrorExit:

    if (SubFieldPtr)
        free(SubFieldPtr);
    free(msgh);
    return NULL;

}


/*
      --------------------------------

      Function to close a JAM message.

      --------------------------------
*/


sword JAMCloseMsg(MSGH * msgh)
{
    msgapierr = MERR_NONE;

    if (InvalidMsgh(msgh))
        return -1;

    FreeMIS(&msgh->mis);

    msgh->id = 0L;
    if (msgh->kludges)
        free(msgh->kludges);

    free((struct _msgh *)msgh);

    return 0;
}



/*
      --------------------------------

      Function to read a JAM message.

      --------------------------------
*/

dword JAMReadMsg(MSGH * msgh, MIS * mis,
                 dword offset, dword bytes, byte * text,
                 dword clen, byte * ctxt)
{
    MSGA *sq = msgh->sq;
    dword bytesread = 0L;

    msgapierr = MERR_NONE;

    if (InvalidMsgh(msgh))
        return -1;

    /* Copy header info */

    if (mis)
        CopyMIS(&msgh->mis, mis);

    /* Copy kludges */

    if (ctxt && clen)
    {
        if (clen > msgh->clen)
            clen = msgh->clen;

        if (msgh->kludges)
            memcpy(ctxt, msgh->kludges, (size_t) clen);
        else
            *ctxt = '\0';
    }

    /* Read message text */

    if (text && bytes)
    {
        if (bytes > (msgh->cur_len - offset))
            bytes = (msgh->cur_len - offset); /* No crazy len */

        /* Only disk */

        if (lseek(JamData->TxtHandle,
                  (long)(msgh->txtoffset + offset), SEEK_SET) == -1)
        {
            msgapierr = MERR_SEEK;
            return -1;
        }

        if (read(JamData->TxtHandle, text, (unsigned)bytes) != (int)bytes)
        {
            msgapierr = MERR_READ;
            return -1;
        }

        bytesread += bytes;

        // Check for NULL termination - we don't want it here as it will
        // mess up addition of SB + Path!
        if (*(text + bytes - 1) == '\0')
            *(text + bytes - 1) = '\n'; // This is UGLY. But what can I
                                        // do? :-)
    }

    return bytesread;

}


/*
      --------------------------------

      Function to write a JAM message.

      --------------------------------
*/


sword JAMWriteMsg(MSGH * msgh, word append, MIS * mis, byte * text,
                  dword textlen, dword totlen, dword clen, byte * ctxt)
{
    MSGA *sq = msgh->sq;
    byte temp[101];
    byte *SubFieldPtr = NULL;

    msgapierr = MERR_NONE;

    if (InvalidMsgh(msgh))
        return -1;

    if (!ctxt)
        clen = 0L;

    if (!text)
        textlen = 0L;

    if (textlen == 0L)
        text = NULL;

    if (clen == 0L)
        ctxt = NULL;

    if (!append)
    {
        if (!mis)
            return -1;          /* We need a header! */

        if (msgh->mode != MOPEN_RW) // if MOPEN_RW we retain old header
                                    // for update!
        {
            Init_JAMheader(sq);

            /* Set index values */
            JamData->Idx.HdrOffset = msgh->hdroffset;

            strcpy(temp, mis->to);
            strlwr(temp);
            JamData->Idx.UserCRC = JAMsysCrc32(temp, strlen(temp), -1L);

            /* Write out the index */

            if (JAMWriteIdx(sq, msgh->msgnum, &JamData->Idx) == -1)
                return -1;      /* Write failed? */

            /* Convert header + kludges */

            if (totlen)
            {
                msgh->totlen = totlen;
                JamData->Hdr.TxtLen = totlen;
            }
            else
                JamData->Hdr.TxtLen = msgh->cur_len;

            JamData->Hdr.TxtOffset = msgh->txtoffset;

            if (clen)
                msgh->clen = clen;

            JamData->Hdr.MsgNum = msgh->msgnum;
        }

        // Convert MIS structure, for MOPEN_RW only partially (attribs,
        // links)

        SQ2JAM(mis, ctxt, sq, msgh->mode, &SubFieldPtr);

        /* Write out the header */

        if (JAMWriteHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) ==
            -1)
        {
            if (SubFieldPtr)
                free(SubFieldPtr);
            return -1;
        }

        if (msgh->mode == MOPEN_RW) // No Subfield/Text write then..
            return 0;

        /* Subfields */

        if (SubFieldPtr)
        {
            if (write(JamData->HdrHandle,
                      SubFieldPtr,
                      (unsigned)JamData->Hdr.SubfieldLen) !=
                (int)JamData->Hdr.SubfieldLen)
            {
                free(SubFieldPtr);
                msgapierr = MERR_WRITE;
                return -1;
            }
            free(SubFieldPtr);
        }

        if (textlen)
        {
            if ((msgh->bytes_written + textlen) > totlen) /* They want too 
                                                             much */
            {
                msgapierr = MERR_BADA;
                return -1;
            }
            if (write(JamData->TxtHandle, text, (unsigned)textlen) !=
                (int)textlen)
            {
                msgapierr = MERR_WRITE;
                return -1;
            }
            msgh->bytes_written += textlen;
        }
    }
    else                        /* append */
    {
        if (textlen)
        {
            if ((msgh->bytes_written + textlen) > msgh->totlen) /* They
                                                                   want
                                                                   too
                                                                   much */
            {
                msgapierr = MERR_BADA;
                return -1;
            }
            if (write(JamData->TxtHandle, text, (size_t) textlen) !=
                (size_t) textlen)
            {
                msgapierr = MERR_WRITE;
                return -1;
            }
            msgh->bytes_written += textlen;
        }
    }

    return 0;

}



/*
      -------------------------------

      Function to kill a JAM message.

      -------------------------------
*/


sword JAMKillMsg(MSGA * sq, dword msgnum)
{
    sword retval = -1;
    word didlock = 0;

    msgapierr = MERR_NONE;

    if (InvalidMh(sq))
        return -1;

    if (!sq->locked)
    {
        if (JAMLock(sq) == -1)
            return -1;
        didlock = 1;
    }

    if (JAMReadIdx(sq, msgnum, &JamData->Idx) == -1)
        goto Exit;

    if (JamData->Idx.HdrOffset == -1) // Deleted message!
        goto Exit;

    // Get header

    if (JAMReadHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
        goto Exit;

    if (JamData->Hdr.Attribute & MSG_DELETED)
        goto Exit;

    if (JamData->Hdr.MsgNum != msgnum) // Weird! Corrupt?
        goto Exit;

    JamData->Hdr.Attribute |= MSG_DELETED;

    if (JAMWriteHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
        goto Exit;

    JamData->Idx.UserCRC = 0xFFFFFFFFL;

    if (JAMWriteIdx(sq, msgnum, &JamData->Idx) == -1)
        goto Exit;

    sq->num_msg--;

    if (msgnum == sq->high_msg) /* Last Msg deleted? New high_msg! */
    {
        sq->high_msg = JAMUidToMsgn(sq,
                                    (dword) ((filelength
                                              (JamData->IdxHandle) /
                                              sizeof(JAMIDXREC)) +
                                             JamData->HdrInfo.BaseMsgNum -
                                             1), UID_PREV);
    }

    JamData->HdrInfo.ActiveMsgs--;

    retval = 0;

  Exit:

    if (didlock)
    {
        if (JAMUnlock(sq) == -1)
            return -1;
    }

    return retval;
}


/*
      ----------------------------

      Function to lock a JAM area.

      ----------------------------
*/

sword JAMLock(MSGA * sq)
{
    dword oldcounter = JamData->HdrInfo.ModCounter;

    msgapierr = MERR_NONE;

    if (InvalidMh(sq))
        return -1;

    /* Don't do anything if already locked */

    if (sq->locked)
        return 0;

    /* Set the flag in the _sqdata header */

    if (JAMmbUpdateHeaderInfo(sq, 0) == -1)
        return -1;

    if (mi.haveshare)
    {
        if (lock(JamData->HdrHandle, 0L, 1L) == -1)
        {
            msgapierr = MERR_NOLOCK;
            return -1;
        }
    }

    sq->locked = TRUE;

    if (JamData->HdrInfo.ModCounter != oldcounter) /* Area was updated in
                                                      meantime */
    {
        sq->num_msg = JamData->HdrInfo.ActiveMsgs;
        sq->high_msg = JAMUidToMsgn(sq,
                                    (dword) ((filelength
                                              (JamData->IdxHandle) /
                                              sizeof(JAMIDXREC)) +
                                             JamData->HdrInfo.BaseMsgNum -
                                             1), UID_PREV);
    }

    return 0;
}


/*
      ------------------------------

      Function to unlock a JAM area.

      ------------------------------
*/

/* Undo the above "lock" operation */

sword JAMUnlock(MSGA * sq)
{

    msgapierr = MERR_NONE;

    if (InvalidMh(sq))
        return -1;

    if (!sq->locked)
        return -1;

    if (JAMmbUpdateHeaderInfo(sq, 1) == -1)
        return -1;

    flush_handle2(JamData->IdxHandle);
    flush_handle2(JamData->TxtHandle);
    flush_handle2(JamData->HdrHandle);

    if (mi.haveshare)
    {
        if (unlock(JamData->HdrHandle, 0L, 1L) == -1)
        {
            msgapierr = MERR_NOUNLOCK;
            return -1;
        }
    }

    sq->locked = FALSE;

    return 0;
}



/*
      --------------------------------

      Function to validate a JAM area.

      --------------------------------
*/

sword JAMValidate(byte * name)
{
    NW(name);

    return TRUE;
}


/*
      ----------------------------------------------

      Function to set current position in a JAM msg.

      ----------------------------------------------
*/


sword JAMSetCurPos(MSGH * msgh, dword pos)
{
    return msgh->cur_pos = pos;
}


/*
      ----------------------------------------------

      Function to get current position in a JAM msg.

      ----------------------------------------------
*/



dword JAMGetCurPos(MSGH * msgh)
{
    if (InvalidMsgh(msgh))
        return -1;

    return (msgh->cur_pos);
}


/*
      ------------------------------------

      Function to convert # to UID (fake).

      ------------------------------------
*/


UMSGID JAMMsgnToUid(MSGA * sq, dword msgnum)
{

    msgapierr = MERR_NONE;

    if (InvalidMh(sq))
        return 0L;

    return (UMSGID) msgnum;

}


/*
      -----------------------------------------------

      Function to convert UID to existing msg number.

      -----------------------------------------------
*/


dword JAMUidToMsgn(MSGA * sq, UMSGID umsgid, word type)
{
    sdword size;
    dword curpos;
    dword found = 0;


    msgapierr = MERR_NONE;

    if (InvalidMh(sq))
        return 0L;

    /* If number doesn't exist anymore, start at first existing msg */

    if (umsgid < JamData->HdrInfo.BaseMsgNum)
    {
        if (type != UID_NEXT)
        {
            msgapierr = MERR_NOENT;
            return 0L;
        }
        else
            umsgid = JamData->HdrInfo.BaseMsgNum;
    }

    if ((size = filelength(JamData->IdxHandle) / sizeof(JAMIDXREC)) == 0L)
    {
        msgapierr = MERR_NOENT;
        return 0L;
    }

    /* Maybe it is too high? */

    if (umsgid > (JamData->HdrInfo.BaseMsgNum + size - 1))
    {
        if (type != UID_PREV)
        {
            msgapierr = MERR_NOENT;
            return 0L;
        }
        else
            umsgid = JamData->HdrInfo.BaseMsgNum + size - 1;
    }

    /* An exact match is easy to check */

    if (type == UID_EXACT)
    {
        if (JAMMsgExists(sq, umsgid))
            return umsgid;
        else
        {
            msgapierr = MERR_NOENT;
            return 0L;
        }
    }

    /* Otherwise, we walk through the index until we find a non-deleted
       msg.. */

    curpos = umsgid;

    if (type == UID_NEXT)       /* Look forward */
    {
        for (; curpos < (JamData->HdrInfo.BaseMsgNum + size); curpos++)
        {
            if (JAMMsgExists(sq, curpos))
            {
                found = curpos;
                break;
            }
        }
    }

    else if (type == UID_PREV)  /* Look backward.. */
    {
        for (; curpos >= JamData->HdrInfo.BaseMsgNum; curpos--)
        {
            if (JAMMsgExists(sq, curpos))
            {
                found = curpos;
                break;
            }
        }
    }

    return (dword) found;
}

/*
      ---------------------------

      Function to get msg lenght.

      ---------------------------
*/

dword JAMGetTextLen(MSGH * msgh)
{
    return msgh->cur_len;
}

/*
      ------------------------------

      Function to get kludge length.

      ------------------------------
*/

dword JAMGetCtrlLen(MSGH * msgh)
{
    return msgh->clen;
}

/*
      --------------------------------

      Function to get high water mark.

      --------------------------------
*/

dword JAMGetHighWater(MSGA * sq)
{
    NW(sq);
    return 0;
}

/*
      ---------------------------

      Function to set high water.

      ---------------------------
*/

sword JAMSetHighWater(MSGA * sq, dword hwm)
{
    NW(sq);
    NW(hwm);
    return -1;
}


/*
      --------------------------------------------------------------------

      Function to convert JAM subfields to MIS struct + 'normal' kludges.

      --------------------------------------------------------------------
*/

void near JAM2SQ(struct _msgh *msgh, byte * Subfields)
{
    JAMSUBFIELD *SFptr;
    char *bufptr, *new;
    char temp[101];
    MSGA *sq = msgh->sq;
    word cursize = 1024, copylen;
    sword SFleft = (sword) JamData->Hdr.SubfieldLen;
    int l;
    word ExtraSubPosition = 0L;


    if (msgh->mode == MOPEN_RW) // Don't convert Subfields for MOPEN_RW,
        SFleft = 0;             // can't write them anyway!

    msgh->mis.replyto = JamData->Hdr.ReplyTo;
    msgh->mis.replies[0] = JamData->Hdr.Reply1st;
    msgh->mis.nextreply = JamData->Hdr.ReplyNext;

    msgh->cur_len = JamData->Hdr.TxtLen;

    msgh->mis.origbase = sq->type;
    msgh->mis.msgno = JamData->Hdr.MsgNum;

    /* Convert message attributes to MIS standard */

    for (l = 0; l < NUMATTR1; l++)
    {
        if (JamData->Hdr.Attribute & AttrTable1[l][1])
            msgh->mis.attr1 |= AttrTable1[l][0];
    }

    msgh->mis.msgwritten = JamData->Hdr.DateWritten;
    msgh->mis.msgprocessed = JamData->Hdr.DateProcessed;
    msgh->mis.msgreceived = JamData->Hdr.DateReceived;

    if (SFleft == 0)
        return;                 /* No subfields to process */

    if ((msgh->kludges = calloc(1, cursize)) == NULL)
        return;
    msgh->clen = 1;

    SFptr = (JAMSUBFIELD *) Subfields;

    while (SFleft > sizeof(JAMBINSUBFIELD))
    {
        bufptr = (char *)SFptr + sizeof(JAMBINSUBFIELD);

        if (SFptr->DatLen > (SFleft - sizeof(JAMBINSUBFIELD)))
            SFptr->DatLen = SFleft - sizeof(JAMBINSUBFIELD);

        switch (SFptr->LoID)
        {
        case JAMSFLD_OADDRESS:
            memset(temp, '\0', sizeof(temp));
            copylen = min(100, SFptr->DatLen);
            memcpy(temp, bufptr, copylen);
            if (ParseFido(temp, &msgh->mis.origfido, msgh->mis.origdomain)
                == -1)
                strcpy(msgh->mis.originter, temp); // Not Fido-type
                                                   // address, so..
            break;

        case JAMSFLD_DADDRESS:
            memset(temp, '\0', sizeof(temp));
            copylen = min(100, SFptr->DatLen);
            memcpy(temp, bufptr, copylen);
            if (ParseFido(temp, &msgh->mis.destfido, msgh->mis.destdomain)
                == -1)
                strcpy(msgh->mis.destinter, temp); // Not Fido-type
                                                   // address, so..
            break;

        case JAMSFLD_SENDERNAME:
            copylen = min(100, SFptr->DatLen);
            memcpy(&msgh->mis.from, bufptr, copylen);
            break;

        case JAMSFLD_RECVRNAME:
            copylen = min(100, SFptr->DatLen);
            memcpy(&msgh->mis.to, bufptr, copylen);
            break;

        case JAMSFLD_SUBJECT:
            copylen = min(100, SFptr->DatLen);
            memcpy(&msgh->mis.subj, bufptr, copylen);
            break;

        case JAMSFLD_MSGID:
            addkludge(msgh, "\01MSGID: ", 8, bufptr, SFptr->DatLen,
                      &cursize);
            break;

        case JAMSFLD_REPLYID:
            addkludge(msgh, "\01REPLY: ", 8, bufptr, SFptr->DatLen,
                      &cursize);
            break;

        case JAMSFLD_PID:
            addkludge(msgh, "\01PID: ", 6, bufptr, SFptr->DatLen,
                      &cursize);
            break;

        case JAMSFLD_FLAGS:
            memset(temp, '\0', sizeof(temp));
            copylen = min(100, SFptr->DatLen);
            memcpy(temp, bufptr, copylen);
            Flags2Attr(temp, &msgh->mis.attr1, &msgh->mis.attr2);
            break;

        case JAMSFLD_TZUTCINFO:
            addkludge(msgh, "\01TZUTC: ", 8, bufptr, SFptr->DatLen,
                      &cursize);
            break;

        case JAMSFLD_FTSKLUDGE:
            addkludge(msgh, "\01", 1, bufptr, SFptr->DatLen, &cursize);
            break;

        case JAMSFLD_SEENBY2D:
            msgh->mis.seenby =
                AddnToStringList(msgh->mis.seenby, bufptr, SFptr->DatLen,
                                 NULL, 0);
            break;

        case JAMSFLD_PATH2D:
            msgh->mis.path =
                AddnToStringList(msgh->mis.path, bufptr, SFptr->DatLen,
                                 NULL, 0);
            break;

        case JAMSFLD_TRACE:
            msgh->mis.via =
                AddnToStringList(msgh->mis.via, bufptr, SFptr->DatLen,
                                 NULL, 0);
            break;

        case JAMSFLD_ENCLFREQ:

            // We have to check for an embedded NULL in this SubField. If
            // there
            // is one, the FREQ consists of two parts:
            // <name><NULL><password>
            memset(temp, '\0', sizeof(temp));
            memcpy(temp, bufptr, min(100, SFptr->DatLen));
            if (strlen(temp) == SFptr->DatLen) // So there's no NULL!
                msgh->mis.requested =
                    AddnToStringList(msgh->mis.requested, bufptr,
                                     SFptr->DatLen, NULL, 0);
            else                // Split in two parts....
                msgh->mis.requested =
                    AddToStringList(msgh->mis.requested, temp,
                                    strchr(temp, '\0') + 1, 0);
            break;

        case JAMSFLD_ENCLFILE:
        case JAMSFLD_ENCLFILEWC:

            msgh->mis.attached =
                AddnToStringList(msgh->mis.attached, bufptr, SFptr->DatLen,
                                 NULL, 0);
            break;

        default:

            msgh->mis.extrasublen +=
                (SFptr->DatLen + sizeof(JAMBINSUBFIELD));
            new = realloc(msgh->mis.extrasub, msgh->mis.extrasublen);
            if (new)
            {
                msgh->mis.extrasub = new;
                JAMmbAddField((byte *) (msgh->mis.extrasub),
                              SFptr->LoID,
                              SFptr->DatLen,
                              &ExtraSubPosition, (byte *) bufptr);
            }
            else                // We ran out of memory. Try and prevent
                                // further SHIT, free all extra subs now..
            {
                if (msgh->mis.extrasub)
                    free(msgh->mis.extrasub);
                msgh->mis.extrasub = NULL;
                msgh->mis.extrasublen = 0L;
            }
            break;
        }

        SFleft -= (sizeof(JAMBINSUBFIELD) + SFptr->DatLen); /* How much
                                                               left? */
        SFptr = (JAMSUBFIELD *) (bufptr + SFptr->DatLen); /* Point to next
                                                             SubField */
    }

}


/*
    -----------------------------------------------------------------

    Function to add a kludge line to the kludges that are 'buffered'.

    -----------------------------------------------------------------
*/

void near addkludge(struct _msgh *msgh, char *text, int txtlen, char *data,
                    dword len, word * cursize)
{
    int thislen;
    char *oldkludges;

    // Max lenght of 'FTSCKLUDGE' subfield. Nice maximum...
    if (len > 255)
        len = 255;

    thislen = txtlen + len;
    msgh->clen += thislen;
    if (msgh->clen > (*cursize - 10))
    {
        *cursize += 512;
        oldkludges = msgh->kludges;
        msgh->kludges = realloc(msgh->kludges, *cursize);
        if (msgh->kludges == NULL) /* Out of memory!! */
        {
            msgh->kludges = oldkludges;
            msgh->clen -= thislen;
            return;
        }
    }

    memcpy(msgh->kludges + msgh->clen - thislen - 1, text, txtlen);
    memcpy(msgh->kludges + msgh->clen - thislen + txtlen - 1, data, len);
    *(msgh->kludges + msgh->clen - 1) = '\0';

}



/*
      ---------------------------------------------

      Function to initialize a JAM header (JAMHDR).

      ---------------------------------------------
*/


void near Init_JAMheader(MSGA * sq)
{

    memset(&JamData->Hdr, '\0', sizeof(JAMHDR));

    strcpy(JamData->Hdr.Signature, "JAM");
    JamData->Hdr.Revision = CURRENTREVLEV;
    JamData->Hdr.MsgIdCRC = -1L;
    JamData->Hdr.ReplyCRC = -1L;
    JamData->Hdr.PasswordCRC = -1L;

}


/*
      ------------------------------------------------------

      Function to convert MIS and kludges to JAM subfields.

      ------------------------------------------------------
*/

void near SQ2JAM(MIS * mis, char *ctxt, MSGA * sq, word mode,
                 byte ** SubFieldPtr)
{
    char temp[300];
    word position;
    char *kludgeptr, *endptr;
    word curlen = 2048;         // Current length of Subfields
    int l, len;
    STRINGLIST *curstring;

    *SubFieldPtr = NULL;

    /* We start by converting MIS header info.. */

    JamData->Hdr.ReplyTo = mis->replyto;
    JamData->Hdr.Reply1st = mis->replies[0];
    JamData->Hdr.ReplyNext = mis->nextreply;

    /* Convert message attributes to MIS standard */

    JamData->Hdr.Attribute = 0L; // Clean out first, for MOPEN_RW

    for (l = 0; l < NUMATTR1; l++)
    {
        if (mis->attr1 & AttrTable1[l][0])
            JamData->Hdr.Attribute |= AttrTable1[l][1];
    }

    if (sq->type & (MSGTYPE_ECHO | MSGTYPE_NEWS))
        JamData->Hdr.Attribute |= MSG_TYPEECHO;
    else if (sq->type & (MSGTYPE_NET | MSGTYPE_MAIL))
        JamData->Hdr.Attribute |= MSG_TYPENET;
    else
        JamData->Hdr.Attribute |= MSG_TYPELOCAL;

    JamData->Hdr.DateWritten = mis->msgwritten;
    JamData->Hdr.DateProcessed = mis->msgprocessed;
    JamData->Hdr.DateReceived = mis->msgreceived;

    if (mis->attr1 & aREAD)
    {
        if (JamData->Hdr.DateReceived == 0L)
            JamData->Hdr.DateReceived = JAMsysTime(NULL);
        if (mis->timesread == 0)
            JamData->Hdr.TimesRead = 1;
    }

    if (mode == MOPEN_RW)
        return;                 // No need to convert Subfields..

    position = 0L;

    // We start with 2048 bytes - enough to hold items below. When we
    // start adding kludges and long stuff like SB/PAth, we'll check if
    // it's enough every time..

    if ((*SubFieldPtr = calloc(1, curlen)) == NULL)
        return;

    JAMmbAddField(*SubFieldPtr, JAMSFLD_SENDERNAME, strlen(mis->from),
                  &position, mis->from);
    JAMmbAddField(*SubFieldPtr, JAMSFLD_RECVRNAME, strlen(mis->to),
                  &position, mis->to);

    if (sq->type & (MSGTYPE_NET | MSGTYPE_MAIL))
    {
        // Now we convert to FLAGS kludges (if necessary). Only a few
        // FLAGS
        // are not covered by the standard JAM attributes.
        if (mis->attr2 & (aFAX | aLET | aSIG | aCOV | aHIR | aXMA | aHUB))
        {
            memset(temp, '\0', sizeof(temp));
            Attr2Flags(temp, 0L,
                       (mis->
                        attr2 & (aFAX | aLET | aSIG | aCOV | aHIR | aXMA |
                                 aHUB)));
            if (strlen(temp) > 7)
            {
                strcpy(temp, temp + 7); // Skip "^AFLAGS "
                JAMmbAddField(*SubFieldPtr, JAMSFLD_FLAGS, strlen(temp),
                              &position, temp);
            }
        }

        if (mis->origfido.point == 0)
            sprintf(temp, "%hu:%hu/%hu", mis->origfido.zone,
                    mis->origfido.net, mis->origfido.node);
        else
            sprintf(temp, "%hu:%hu/%hu.%hu", mis->origfido.zone,
                    mis->origfido.net,
                    mis->origfido.node, mis->origfido.point);

        if (mis->origdomain[0] != '\0')
        {
            strcat(temp, "@");
            strcat(temp, mis->origdomain);
        }

        JAMmbAddField(*SubFieldPtr, JAMSFLD_OADDRESS, strlen(temp),
                      &position, temp);


        if (mis->destfido.point == 0)
            sprintf(temp, "%hu:%hu/%hu", mis->destfido.zone,
                    mis->destfido.net, mis->destfido.node);
        else
            sprintf(temp, "%hu:%hu/%hu.%hu", mis->destfido.zone,
                    mis->destfido.net,
                    mis->destfido.node, mis->destfido.point);

        if (mis->destdomain[0] != '\0')
        {
            strcat(temp, "@");
            strcat(temp, mis->destdomain);
        }

        JAMmbAddField(*SubFieldPtr, JAMSFLD_DADDRESS, strlen(temp),
                      &position, temp);
    }

    JAMmbAddField(*SubFieldPtr, JAMSFLD_SUBJECT, strlen(mis->subj),
                  &position, mis->subj);

    // ------- Check for attached files

    if (mis->attached != NULL)
    {
        JamData->Hdr.Attribute |= MSG_FILEATTACH;
        for (curstring = mis->attached; curstring;
             curstring = curstring->next)
        {
            if (curstring->s == NULL || curstring->s[0] == '\0')
                continue;
            JAMmbAddField(*SubFieldPtr, JAMSFLD_ENCLFILE,
                          strlen(curstring->s), &position, curstring->s);
        }
    }

    // ---------- Check for file requests

    if (mis->requested != NULL)
    {
        JamData->Hdr.Attribute |= MSG_FILEREQUEST;
        for (curstring = mis->requested; curstring;
             curstring = curstring->next)
        {
            if (curstring->s == NULL || curstring->s[0] == '\0')
                continue;

            strcpy(temp, curstring->s);
            len = strlen(temp);

            if (curstring->pw != NULL)
            {
                sprintf(temp, "%s\0%s", curstring->s, curstring->pw);
                strcpy(temp + len + 1, curstring->pw);
                len += (strlen(curstring->pw) + 1);
            }

            JAMmbAddField(*SubFieldPtr, JAMSFLD_ENCLFREQ, len, &position,
                          temp);
        }
    }

    // --------- Check for SEEN-BY lines

    if (mis->seenby != NULL && (sq->type & (MSGTYPE_ECHO | MSGTYPE_NEWS)))
    {
        for (curstring = mis->seenby; curstring;
             curstring = curstring->next)
        {
            if (curstring->s == NULL || curstring->s[0] == '\0')
                continue;

            len = strlen(curstring->s);

            if ((position + len + 20) > curlen)
            {
                curlen += (512 + len);
                if ((*SubFieldPtr = realloc(*SubFieldPtr, curlen)) == NULL)
                    return;     // Out of mem :-(
            }

            JAMmbAddField(*SubFieldPtr, JAMSFLD_SEENBY2D,
                          strlen(curstring->s), &position, curstring->s);
        }
    }

    // ------------- Check for PATH lines.

    if (mis->path != NULL && (sq->type & (MSGTYPE_ECHO | MSGTYPE_NEWS)))
    {
        for (curstring = mis->path; curstring; curstring = curstring->next)
        {
            if (curstring->s == NULL || curstring->s[0] == '\0')
                continue;

            len = strlen(curstring->s);

            if ((position + len + 20) > curlen)
            {
                curlen += (512 + len);
                if ((*SubFieldPtr = realloc(*SubFieldPtr, curlen)) == NULL)
                    return;     // Out of mem :-(
            }

            JAMmbAddField(*SubFieldPtr, JAMSFLD_PATH2D,
                          strlen(curstring->s), &position, curstring->s);
        }
    }

    // ---------- Check for VIA lines

    if (mis->via != NULL && (sq->type & (MSGTYPE_NET | MSGTYPE_MAIL)))
    {
        for (curstring = mis->via; curstring; curstring = curstring->next)
        {
            if (curstring->s == NULL || curstring->s[0] == '\0')
                continue;

            len = strlen(curstring->s);

            if ((position + len + 20) > curlen)
            {
                curlen += (512 + len);
                if ((*SubFieldPtr = realloc(*SubFieldPtr, curlen)) == NULL)
                    return;     // Out of mem :-(
            }

            JAMmbAddField(*SubFieldPtr, JAMSFLD_TRACE,
                          strlen(curstring->s), &position, curstring->s);
        }
    }

    // ---------- Add extra unknown SubFields.

    if (mis->extrasublen != 0L)
    {
        if ((position + mis->extrasublen) > curlen)
        {
            curlen += (mis->extrasublen);
            if ((*SubFieldPtr = realloc(*SubFieldPtr, curlen)) == NULL)
                return;         // Out of mem :-(
        }

        memcpy((*SubFieldPtr + position), mis->extrasub, mis->extrasublen);
        position += mis->extrasublen;
    }

    /* Update SubFieldLen, just in case there are no kludges */

    JamData->Hdr.SubfieldLen = position;

    if (ctxt == NULL)
        return;

    kludgeptr = ctxt;
    while ((kludgeptr = strchr(kludgeptr, '\01')) != NULL)
    {
        // First we check the size of the memory block we use.
        // Realloc if too small, take 255 chars as a MAX for
        // Subfields to add. Add 10 bytes for LoID, HiID etc.

        if ((position + 266) > curlen)
        {
            curlen += 512;
            if ((*SubFieldPtr = realloc(*SubFieldPtr, curlen)) == NULL)
                return;         // Out of mem :-(
        }

        endptr = strchr(kludgeptr + 1, '\01');

        if (endptr == NULL)
            endptr = strchr(kludgeptr + 1, '\0');

        if (!strncmp(kludgeptr + 1, "MSGID: ", 7))
        {
            JAMmbAddField(*SubFieldPtr, JAMSFLD_MSGID,
                          endptr - (kludgeptr + 8), &position,
                          kludgeptr + 8);
            memset(temp, '\0', sizeof(temp));
            memcpy(temp, kludgeptr + 8, endptr - (kludgeptr + 8));
            strlwr(temp);
            JamData->Hdr.MsgIdCRC = JAMsysCrc32(temp, strlen(temp), -1L);
        }
        else if (!strncmp(kludgeptr + 1, "REPLY: ", 7))
        {
            JAMmbAddField(*SubFieldPtr, JAMSFLD_REPLYID,
                          endptr - (kludgeptr + 8), &position,
                          kludgeptr + 8);
            memset(temp, '\0', sizeof(temp));
            memcpy(temp, kludgeptr + 8, endptr - (kludgeptr + 8));
            strlwr(temp);
            JamData->Hdr.ReplyCRC = JAMsysCrc32(temp, strlen(temp), -1L);
        }
        else if (!strncmp(kludgeptr + 1, "PID: ", 5))
        {
            JAMmbAddField(*SubFieldPtr, JAMSFLD_PID,
                          endptr - (kludgeptr + 6), &position,
                          kludgeptr + 6);
        }
        else if (!strncmp(kludgeptr + 1, "TZUTC: ", 7))
        {
            JAMmbAddField(*SubFieldPtr, JAMSFLD_TZUTCINFO,
                          endptr - (kludgeptr + 8), &position,
                          kludgeptr + 8);
        }
        else
            JAMmbAddField(*SubFieldPtr, JAMSFLD_FTSKLUDGE,
                          endptr - (kludgeptr + 1), &position,
                          kludgeptr + 1);
        kludgeptr++;
    }



    JamData->Hdr.SubfieldLen = position;
}

/*
   ---------------------------------------------------------------------

    Check if a certain message exists (non deleted), return 1 if Exists

   ---------------------------------------------------------------------
*/


byte near JAMMsgExists(MSGA * sq, UMSGID umsgid)
{
    JAMHDR hdr;

    /* Read index entry.. */

    if (JAMReadIdx(sq, umsgid, &JamData->Idx) == -1)
        return 0;

    /* If both are 0xFFFFFFFF there is no corresponding header.. */

    if ((JamData->Idx.UserCRC == 0xFFFFFFFFL) &&
        (JamData->Idx.HdrOffset == 0xFFFFFFFFL))
        return 0;

    if (JAMReadHeader(sq, JamData->Idx.HdrOffset, &hdr) == -1)
        return 0;

    if (hdr.Attribute & MSG_DELETED)
        return 0;
    else
        return 1;

}


/* ----------------------------------------------------------- */
/* - Function that opens all JAM files, creates if necessary - */
/* - Returns 0 on success, -1 on error.                      - */
/* ----------------------------------------------------------- */


int near JAMmbOpen(MSGA * sq, byte * tempname)
{
    char FileName[120], name[100];

#ifndef __GNUC__
    int caccess = O_RDWR | O_BINARY | O_CREAT;
#else
    int caccess = O_RDWR | O_CREAT;
#endif

    unsigned mode = S_IREAD | S_IWRITE;


    strcpy(name, tempname);
#ifdef __GNUC__
    Strip_Trailing(name, '/');
#else
    Strip_Trailing(name, '\\');
#endif

    JamData->HdrHandle =
        JamData->IdxHandle = JamData->TxtHandle = JamData->LrdHandle = -1;

    /* .JHR file */

    sprintf(FileName, "%s%s", name, EXT_HDRFILE);
    if ((JamData->HdrHandle =
         sopen(FileName, caccess, SH_DENYNO, mode)) == -1)
    {
        msgapierr = MERR_OPENFILE;
        return -1;
    }

    if (filelength(JamData->HdrHandle) == 0L) // Zero length, create new
                                              // header
    {
        memset(&JamData->HdrInfo, 0, sizeof(JAMHDRINFO));
        strcpy(JamData->HdrInfo.Signature, HEADERSIGNATURE);
        JamData->HdrInfo.DateCreated = JAMsysTime(NULL);
        JamData->HdrInfo.PasswordCRC = 0xffffffffL;
        JamData->HdrInfo.BaseMsgNum = 1L;

        lseek(JamData->HdrHandle, 0L, SEEK_SET);
        if (write
            (JamData->HdrHandle, &JamData->HdrInfo,
             sizeof(JAMHDRINFO)) != sizeof(JAMHDRINFO))
        {
            msgapierr = MERR_WRITE;
            goto ErrorExit;
        }
    }
    else if (JAMmbUpdateHeaderInfo(sq, 0) == -1)
        goto ErrorExit;

    /* .JDT file */

    sprintf(FileName, "%s%s", name, EXT_TXTFILE);
    if ((JamData->TxtHandle =
         sopen(FileName, caccess, SH_DENYNO, mode)) == -1)
        goto ErrorExit;

    /* .JDX file */

    sprintf(FileName, "%s%s", name, EXT_IDXFILE);
    if ((JamData->IdxHandle =
         sopen(FileName, caccess, SH_DENYNO, mode)) == -1)
        goto ErrorExit;

    /* .JLR file */

    sprintf(FileName, "%s%s", name, EXT_LRDFILE);
    if ((JamData->LrdHandle =
         sopen(FileName, caccess, SH_DENYNO, mode)) == -1)
        goto ErrorExit;

    return 0;                   // Success!

  ErrorExit:

    if (!msgapierr)
        msgapierr = MERR_OPENFILE;

    if (JamData->HdrHandle != -1)
        close(JamData->HdrHandle);
    if (JamData->IdxHandle != -1)
        close(JamData->IdxHandle);
    if (JamData->TxtHandle != -1)
        close(JamData->TxtHandle);
    if (JamData->LrdHandle != -1)
        close(JamData->LrdHandle);

    return -1;                  // Something wrong :-(

}


/* -------------------------------------- */
/* - Function that closes all JAM files - */
/* -------------------------------------- */


int near JAMmbClose(MSGA * sq)
{
    /* Close all handles */

    close(JamData->HdrHandle);
    close(JamData->TxtHandle);
    close(JamData->IdxHandle);
    close(JamData->LrdHandle);

    return 0;
}


/* --------------------------------------------------- */
/* - Read a JAM header, check signature and revision - */
/* - Returns -1 on error, 0 if all went well.        - */
/* --------------------------------------------------- */

int near JAMReadHeader(MSGA * sq, long offset, JAMHDR * hdr)
{

    /* Fetch header */
    if (lseek(JamData->HdrHandle, offset, SEEK_SET) != offset)
    {
        msgapierr = MERR_SEEK;
        return -1;
    }

    if (read(JamData->HdrHandle, hdr, sizeof(JAMHDR)) != sizeof(JAMHDR))
    {
        msgapierr = MERR_READ;
        return -1;
    }

    /* Check header */
    if (strcmp(hdr->Signature, HEADERSIGNATURE) != 0)
    {
        msgapierr = MERR_BADSIG;
        return -1;
    }

    if (hdr->Revision != CURRENTREVLEV)
    {
        msgapierr = MERR_BADREV;
        return -1;
    }

    return 0;
}


/* -------------------------------------------- */
/* - Write out a JAM header                   - */
/* - Returns -1 on error, 0 if all went well. - */
/* -------------------------------------------- */

int near JAMWriteHeader(MSGA * sq, long offset, JAMHDR * hdr)
{

    if (lseek(JamData->HdrHandle, offset, SEEK_SET) != offset)
    {
        msgapierr = MERR_SEEK;
        return -1;
    }

    if (write(JamData->HdrHandle, hdr, sizeof(JAMHDR)) != sizeof(JAMHDR))
    {
        msgapierr = MERR_WRITE;
        return -1;
    }

    return 0;
}


/* ------------------------------------------------------ */
/* - Read a JAM index entry, use 'cache' to speed up    - */
/* - repeated, sequential access (UidToMsgn for example - */
/* - Returns -1 on error, 0 if all went well.           - */
/* ------------------------------------------------------ */


int near JAMReadIdx(MSGA * sq, dword number, JAMIDXREC * idx)
{
    long WhatOffset;
    int BytesRead;


    if ((JamData->IdxCache.active != 0) && (number <= JamData->IdxCache.high) && (number >= JamData->IdxCache.base) && (time(NULL) < (JamData->IdxCache.birth + 5))) /* Cache 
                                                                                                                                                                        hit? 
                                                                                                                                                                      */

    {
        memcpy(idx,
               &JamData->IdxCache.contents[number -
                                           JamData->IdxCache.base],
               sizeof(JAMIDXREC));

        return 0;
    }

    /* No cache hit, fill cache */

    JamData->IdxCache.active = 0; /* Invalidate contents */
    JamData->IdxCache.high = 0;

    JamData->IdxCache.base = number - (IDXBUFSIZE / 2);

    if ((JamData->IdxCache.base < JamData->HdrInfo.BaseMsgNum) ||
        ((IDXBUFSIZE / 2) > number))
        JamData->IdxCache.base = JamData->HdrInfo.BaseMsgNum;

    /* Fetch index record */

    WhatOffset =
        (JamData->IdxCache.base -
         JamData->HdrInfo.BaseMsgNum) * (long)sizeof(JAMIDXREC);
    if (lseek(JamData->IdxHandle, WhatOffset, SEEK_SET) != WhatOffset)
    {
        msgapierr = MERR_SEEK;
        return -1;
    }

    if ((BytesRead = read(JamData->IdxHandle,
                          &JamData->IdxCache.contents[0],
                          sizeof(JAMIDXREC) * IDXBUFSIZE)) == -1)
    {
        msgapierr = MERR_READ;
        return -1;
    }

    JamData->IdxCache.high = JamData->IdxCache.base +
        (BytesRead / sizeof(JAMIDXREC)) - 1;

    if (BytesRead < sizeof(JAMIDXREC))
    {
        msgapierr = MERR_READ;
        return -1;
    }

    JamData->IdxCache.birth = time(NULL);
    JamData->IdxCache.active = 1;

    memcpy(idx,
           &JamData->IdxCache.contents[number - JamData->IdxCache.base],
           sizeof(JAMIDXREC));

    return 0;
}


/* -------------------------------------------- */
/* - Write a JAM index entry.                 - */
/* - Returns -1 on error, 0 if all went well. - */
/* -------------------------------------------- */


int near JAMWriteIdx(MSGA * sq, dword number, JAMIDXREC * idx)
{
    long offset;

    JamData->IdxCache.active = 0; /* Invalidate contents of cache! */
    JamData->IdxCache.high = 0;

    /* Fetch index record */

    offset =
        (number - JamData->HdrInfo.BaseMsgNum) * (long)sizeof(JAMIDXREC);

    if (lseek(JamData->IdxHandle, offset, SEEK_SET) != offset)
    {
        msgapierr = MERR_SEEK;
        return -1;
    }

    if (write(JamData->IdxHandle, idx, sizeof(JAMIDXREC)) !=
        sizeof(JAMIDXREC))
    {
        msgapierr = MERR_WRITE;
        return -1;
    }

    return 0;
}





/* =======================================================

       Here we add some functions stolen from JAM API

   ======================================================= */


/* ---------------------- */
/* - Add a JAM Subfield - */
/* ---------------------- */

/*
**  Add specified field to WorkBuf. It requires that POSITION has been
**  reset to zero (0) prior to the first call. The POSITION parameter is
**  updated to point to the first position after the newly added field.
*/

void near JAMmbAddField(byte * start, dword WhatField,
                        size_t DatLen, word * Position, byte * Data)
{
    JAMSUBFIELD *SubFieldPtr;

    if (DatLen > 255)
        return;

    SubFieldPtr = (JAMSUBFIELD *) (start + *Position);
    SubFieldPtr->LoID = (word) WhatField;
    SubFieldPtr->HiID = 0;
    SubFieldPtr->DatLen = 0L;
    *Position += sizeof(JAMBINSUBFIELD);

    memcpy((start + *Position), Data, DatLen);
    SubFieldPtr->DatLen += DatLen;
    *Position += DatLen;
}




int near JAMmbUpdateHeaderInfo(MSGA * sq, int WriteIt)
{

    /* Seek to beginning of file */
    if (lseek(JamData->HdrHandle, 0L, SEEK_SET) != 0L)
    {
        msgapierr = MERR_SEEK;
        return -1;
    }

    /* Update ModCounter if told to */
    if (WriteIt)
    {
        JamData->HdrInfo.ModCounter++;

        if (JamData->HdrInfo.BaseMsgNum == 0L)
            JamData->HdrInfo.BaseMsgNum = 1L;

        /* Update header info record */
        if (write
            (JamData->HdrHandle, &JamData->HdrInfo,
             sizeof(JAMHDRINFO)) != sizeof(JAMHDRINFO))
        {
            JamData->HdrInfo.ModCounter--;
            msgapierr = MERR_WRITE;
            return -1;
        }
    }
    else
        /* Fetch header info record */
    {
        if (read(JamData->HdrHandle, &JamData->HdrInfo, sizeof(JAMHDRINFO))
            != sizeof(JAMHDRINFO))
        {
            msgapierr = MERR_READ;
            return -1;
        }

        if (JamData->HdrInfo.BaseMsgNum == 0L)
            JamData->HdrInfo.BaseMsgNum = 1L;
    }

    return 0;
}


/*
**  Fetch LastRead for passed UserID. Returns 1 upon success and 0 upon
**  failure.
*/

dword JAMmbFetchLastRead(MSGA * sq, dword UserCRC, int getlast)
{
    sdword ReadCount;
    dword LastReadRec;
    JAMLREAD new;

  begin:

    /* Seek to beginning of file */

    if (lseek(JamData->LrdHandle, 0L, SEEK_SET) != 0L)
        return -1;

    /* Read file from top to bottom */

    LastReadRec = 0L;
    while (1)
    {
        ReadCount =
            read(JamData->LrdHandle, &new, (sdword) sizeof(JAMLREAD));
        if (ReadCount != (sdword) sizeof(JAMLREAD))
        {
            if (!ReadCount)
                /* End of file */
            {
                memset(&new, '\0', sizeof(JAMLREAD));
                new.UserCRC = UserCRC;
                new.UserID = UserCRC;
                write(JamData->LrdHandle, &new, sizeof(JAMLREAD));
                goto begin;
            }
            else
                /* Read error */
                return -1;
        }

        /* See if it matches what we want */
        if (new.UserCRC == UserCRC)
        {
            JamData->lastread = LastReadRec;
            if (getlast)
                return new.LastReadMsg;
            else
                return max(new.LastReadMsg, new.HighReadMsg);
        }

        /* Next record number */
        LastReadRec++;
    }                           /* while */

}

/*
**  Store LastRead record and if successful, optionally updates the header
**  info block (and its ModCounter) at the beginning of the header file.
**  Returns 1 upon success and 0 upon failure.
*/

int JAMmbStoreLastRead(MSGA * sq, dword last, dword highest, dword CRC)
{
    sdword UserOffset;
    JAMLREAD new;


    /* Seek to the appropriate position */

    UserOffset = (sdword) (JamData->lastread * (sdword) sizeof(JAMLREAD));
    if (lseek(JamData->LrdHandle, UserOffset, SEEK_SET) != UserOffset)
        return -1;

    memset(&new, '\0', sizeof(JAMLREAD));
    new.UserCRC = CRC;
    new.UserID = CRC;
    new.HighReadMsg = highest;
    new.LastReadMsg = last;

    /* Write record */
    if (write(JamData->LrdHandle, &new, (sdword) sizeof(JAMLREAD)) !=
        (sdword) sizeof(JAMLREAD))
        return -1;

    return 0;
}


// ===============================================================
// Parse a FidoNet style address (incl. domain). If the string is
// not a fido address, return -1
// ===============================================================

int ParseFido(char *s, NETADDR * addr, char *domain)
{
    char *charptr;

    if ((strchr(s, ':') == NULL) || (strchr(s, '/') == NULL))
        return -1;              // Not a Fido address

    if ((charptr = strchr(s, '@')) != NULL) // Address has a domain
    {
        strcpy(domain, charptr + 1);
        *charptr = '\0';
    }

    sscanf(s, "%hu:%hu/%hu.%hu", &addr->zone, &addr->net, &addr->node,
           &addr->point);

    return 0;
}

// ========================================================
