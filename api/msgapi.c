#define MSGAPI_PROC

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#ifdef __WATCOMC__
#include <dos.h>
#endif

#include "alc.h"
#include "prog.h"
#include "msgapi.h"
#include "apidebug.h"
#include "canlock.h"
#include "sqsdm.h"

byte *intl = "INTL";
byte *fmpt = "FMPT";
byte *topt = "TOPT";
byte *flags = "FLAGS";
byte *area_colon = "AREA:";
byte *vialine = "Via";
byte *pathline = "PATH";

static char *copyright =
    "MSGAPI - Copyright 1991 by Scott J. Dudley.  All rights reserved.";
static char *copyGvE =
    " JAM & Hudson specific MSGAPI extensions by Gerard van Essen (2:281/527), (c) 1993, 1994";

word _stdc msgapierr = 0;       /* Global error value for message API
                                   routines */

char *errmsgs[] = {

    "No error information available", // MERR_NONE 
    "Bad handle",               // MERR_BADH 
    "Bad file (corrupted)",     // MERR_BADF 
    "Out of memory",            // MERR_NOMEM 
    "Disk full",                // MERR_NODS 
    "File/Message doesn't exist", // MERR_NOENT 
    "Bad argument",             // MERR_BADA 
    "Still open msgs",          // MERR_EOPEN 
    "Error seeking file",       // MERR_SEEK 
    "Error reading file",       // MERR_READ 
    "Error writing file",       // MERR_WRITE 
    "Invalid signature/id found", // MERR_BADSIG 
    "Message board mismatch",   // MERR_BOARD 
    "Message number mismatch",  // MERR_MSGNO 
    "Msginfo.bbs invalid",      // MERR_BADMSGINFO
    "Message is deleted",       // MERR_DELETED 
    "Can't lock file",          // MERR_NOLOCK 
    "Can't unlock file",        // MERR_NOUNLOCK 
    "Can't open file",          // MERR_OPENFILE 
    "Incorrect revision number", // MERR_BADREV
    "Can't open Hudson base files" // MERR_NOHMB
};

struct _minf _stdc mi;

int _mdays[13] = {
/* Jan */ 0,
/* Feb */ 31,
/* Mar */ 31 + 28,
/* Apr */ 31 + 28 + 31,
/* May */ 31 + 28 + 31 + 30,
/* Jun */ 31 + 28 + 31 + 30 + 31,
/* Jul */ 31 + 28 + 31 + 30 + 31 + 30,
/* Aug */ 31 + 28 + 31 + 30 + 31 + 30 + 31,
/* Sep */ 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
/* Oct */ 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
/* Nov */ 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
/* Dec */ 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
/* Jan */ 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
};


sword MsgOpenApi(struct _minf *minf, char *path, word useflags,
                 char *hudson)
{
    NW(copyright);
    NW(copyGvE);

    mi = *minf;
#ifndef __OS2__
    mi.haveshare = canlock(path);
#else
    mi.haveshare = 1;
#endif

    mi.useflags = useflags;

    strcpy(mi.hudsonpath, hudson);

    return 0;
}


sword MsgCloseApi(void)
{
    return 0;
}


MSG *MsgOpenArea(byte * name, word mode, word type)
{

    if (type & MSGTYPE_SQUISH)
    {
        return (SquishOpenArea(name, mode, type));
    }
    else if (type & MSGTYPE_JAM)
    {
        return (JAMOpenArea(name, mode, type));
    }
    else if (type & MSGTYPE_HMB)
    {
        return (HMBOpenArea(name, mode, type));
    }
    else
    {
        return (SdmOpenArea(name, mode, type));
    }
}


MSG *CreateAreaHandle(word type)
{
    MSG *sq;


    if ((sq = calloc(1, sizeof(MSG))) == NULL)
    {
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    if ((sq->api =
         (struct _apifuncs *)calloc(1, sizeof(struct _apifuncs))) == NULL)
    {
        free(sq);
        msgapierr = MERR_NOMEM;
        return NULL;
    }

    sq->id = MSGAPI_ID;
    sq->type = type;
    if (type & MSGTYPE_ECHO)
        sq->isecho = TRUE;

    sq->sz_xmsg = sizeof(XMSG);
    sq->len = sizeof(MSG);

    return sq;

}


sword MsgValidate(word type, byte * name)
{

    if (type & MSGTYPE_SQUISH)
        return (SquishValidate(name));

    else if (type == MSGTYPE_SDM)
        return (SdmValidate(name));

    else if (type == MSGTYPE_JAM)
        return (JAMValidate(name));

    else if (type == MSGTYPE_HMB)
        return (HMBValidate(name));

    else
        return FALSE;
}


/* Check to see if a message handle is valid.  This function should work	 *
 * for ALL handlers tied into MsgAPI.	This also checks to make sure that	 *
 * the area which the message is from is also valid.	(ie. The message		 *
 * handle isn't valid, unless the area handle of that message is also       *
 * valid.)																						 */

sword InvalidMsgh(MSGH * msgh)
{

    if (msgh == NULL || msgh->id != MSGH_ID || InvalidMh(msgh->sq))
    {
        msgapierr = MERR_BADH;
        return TRUE;
    }

    return FALSE;
}

/* Check to ensure that a message area handle is valid.							 */

sword InvalidMh(MSG * mh)
{
    if (mh == NULL || mh->id != MSGAPI_ID)
    {
        msgapierr = MERR_BADH;
        return TRUE;
    }

    return FALSE;
}


byte *StripNasties(byte * str)
{
    byte *s;

    for (s = str; *s; s++)
        if (*s < ' ')
            *s = ' ';

    return str;
}



/* Copy the text itself to a buffer, or count its length if out==NULL */

word _CopyToBuf(byte * p, byte * out, byte ** end, word maxlen)
{
    word len = 0;
    byte *origstart = p;


    if (out)
        *out++ = '\x01';

    len++;


    for (; *p == '\x0d' || *p == '\x0a' || *p == (byte) '\x8d'; p++)
        ;

    while ((*p == '\x01' || strncmp(p, area_colon, 5) == 0) &&
           (!maxlen || len <= maxlen) &&
           (strnicmp(p + 1, vialine, 3) != 0) &&
           (strnicmp(p + 1, pathline, 4) != 0))
    {
        /* Skip over the first ^a */

        if (*p == '\x01')
            p++;

        while (*p && *p != '\x0d' && *p != '\x0a' && *p != (byte) '\x8d')
        {
            if (out)
                *out++ = *p;

            p++;
            len++;
        }

        if (out)
            *out++ = '\x01';

        len++;

        while (*p == '\x0d' || *p == '\x0a' || *p == (byte) '\x8d')
            p++;
    }

    /* Cap the string */

    if (out)
        *out = '\0';

    len++;


    /* Make sure to leave no trailing x01's. */

    if (out && out[-1] == '\x01')
        out[-1] = '\0';


    /* Now store the new end location of the kludge lines */

    // Ignore trailing '\r' lines, could be start of text with '\r'...
    while ((p > (origstart + 1)) &&
           (*(p - 1) == '\x0d' || *(p - 1) == '\x0a'
            || *(p - 1) == (byte) '\x8d') && (*(p - 2) == '\x0d'
                                              || *(p - 2) == '\x0a'
                                              || *(p - 2) ==
                                              (byte) '\x8d'))
    {
        p--;
    }

    if (end)
        *end = p;

    return len;
}



byte *CopyToControlBuf(byte * txt, byte ** newtext, unsigned *length)
{
    byte *cbuf, *end;

    word clen;


    /* Figure out how long the control info is */

    clen = _CopyToBuf(txt, NULL, NULL, 0);

    /* Allocate memory for it */

#define SAFE_CLEN 20

    if ((cbuf = calloc(1, clen + SAFE_CLEN)) == NULL)
        return NULL;

//  memset(cbuf, '\0', clen+SAFE_CLEN);

    /* Now copy the text itself */

    clen = _CopyToBuf(txt, cbuf, &end, clen);

    if (length)
        *length -= (size_t) (end - txt);

    if (newtext)
        *newtext = end;

    return cbuf;
}


byte *GetCtrlToken(byte * where, byte * what)
{
    byte *end, *found, *out;


    if (where && (found = strstr(where, what)) != NULL && ((found[-1] == '\x01') || // real 
                                                                                    // kludge
                                                           (found[-1] == '\r') || // no 
                                                                                  // \01 
                                                                                  // needed, 
                                                                                  // beginning 
                                                                                  // line
                                                           (found == where) // no 
                                                                            // \01 
                                                                            // needed, 
                                                                            // start
        ))
    {
        end = strchr(found, '\x01');
        if (!end)
            end = strchr(found, '\r');

        if (!end)
            end = found + strlen(found);

        if ((out = malloc((size_t) (end - found) + 1)) == NULL)
            return NULL;

        memmove(out, found, (size_t) (end - found));
        out[(size_t) (end - found)] = '\0';
        return out;
    }

    return NULL;
}



void ConvertControlInfo(byte * ctrl, MIS * mis)
{
    byte *p, *s;
    int newzone = 0;


    if ((p = s = GetCtrlToken(ctrl, intl)) != NULL)
    {
        NETADDR norig, ndest;

        /* Copy the defaults from the original address */

        norig = mis->origfido;
        ndest = mis->destfido;

        /* Parse the destination part of the kludge */

        s += 5;
        Parse_NetNode(s, &ndest.zone, &ndest.net, &ndest.node,
                      &ndest.point);

        while (*s != ' ' && *s)
            s++;

        if (*s)
            s++;

        Parse_NetNode(s, &norig.zone, &norig.net, &norig.node,
                      &norig.point);

        free(p);

        /* Only use this as the "real" zonegate address if the net/node *
           addresses in the INTL line match those in the message * body.
           Otherwise, it's probably a gaterouted message! */

        if (ndest.net == mis->destfido.net
            && ndest.node == mis->destfido.node
            && norig.net == mis->origfido.net
            && norig.node == mis->origfido.node)
        {
            mis->destfido = ndest;
            mis->origfido = norig;

            /* Only remove the INTL line if it's not gaterouted, which is
               * why we do it here.  */

            /* RemoveFromCtrl(ctrl,intl); *//* NOT! GvE */
        }
        else
        {
            mis->origfido.zone = mis->destfido.zone = norig.zone;
        }
    }                           /* GvE: */
    else if ((p = stristr(ctrl, "MSGID:")) != NULL) /* No explicit zone
                                                       info, look for
                                                       MSGID */
    {
        p += 6;                 /* Found one, look for zone */
        while ((!isdigit(*p)) && (*p != '\0') && (*p != '\01')) /* Skip
                                                                   fidonet# 
                                                                   stuff */
            p++;

        if ((*p != '\0') &&
            (*p != '\01') &&
            (isdigit(*p)) &&
            ((newzone = atoi(p)) != 0) &&
            (strchr(p, ':') != NULL) &&
            (strchr(p, '/') != NULL) &&
            ((strchr(p, '\01') == NULL) ||
             (strchr(p, '\01') > strchr(p, '/'))))

        {
            mis->origfido.zone = newzone;
            mis->destfido.zone = newzone;
        }
    }

    /* Handle the FMPT kludge */

    if ((s = GetCtrlToken(ctrl, fmpt)) != NULL)
    {
        mis->origfido.point = atoi(s + 5);
        free(s);

        RemoveFromCtrl(ctrl, fmpt);
    }


    /* Handle TOPT too */

    if ((s = GetCtrlToken(ctrl, topt)) != NULL)
    {
        mis->destfido.point = atoi(s + 5);
        free(s);

        RemoveFromCtrl(ctrl, topt);
    }

    // And the flags....

    Convert_Flags(ctrl, &mis->attr1, &mis->attr2);

}



byte *CvtCtrlToKludge(byte * ctrl)
{
    byte *from;
    byte *buf;
    byte *to;
    size_t clen;


    clen = strlen(ctrl) + NumKludges(ctrl) + 20;

    if ((buf = calloc(1, clen)) == NULL)
        return NULL;

    to = buf;

    /* Convert ^aKLUDGE^aKLUDGE... into ^aKLUDGE\r^aKLUDGE\r... */

    for (from = ctrl; *from == '\x01' && from[1];)
    {
        /* Only copy out the ^a if it's NOT the area: line */

//    if (!eqstrn(from+1, area_colon, 5))
        *to++ = *from;

        from++;

        while (*from && *from != '\x01')
            *to++ = *from++;

        *to++ = '\r';
    }

    *to = '\0';

    return buf;
}




void RemoveFromCtrl(byte * ctrl, byte * what)
{
    byte *search;
    byte *p, *s;

    if (!ctrl || !what)
        return;

    if ((search = malloc(strlen(what) + 2)) == NULL)
        return;

    strcpy(search, "\x01");
    strcat(search, what);

    /* Now search for this token in the control buffer, and remove it. */

    while ((p = strstr(ctrl, search)) != NULL)
    {
        for (s = p + 1; *s && *s != '\x01'; s++)
            ;

//  len = s-p;
//  strocpy(p, s);  == :
        memmove(p, s, strlen(s) + 1);
//  s = p + strlen(p);
    }

    free(search);
}


word NumKludges(char *txt)
{
    word nk = 0;
    char *p;

    for (p = txt; (p = strchr(p, '\x01')) != NULL; p++)
        nk++;

    return nk;
}




void Convert_Flags(char *kludges, dword * attr1, dword * attr2)
{
    char *s;


    if (!kludges || !attr1 || !attr2)
        return;

    if ((s = GetCtrlToken(kludges, flags)) != NULL)
    {
        Flags2Attr(s + 6, attr1, attr2);
        free(s);
        RemoveFromCtrl(kludges, flags);
    }
}


void Attr2Flags(char *temp, dword attr1, dword attr2)
{


    strcpy(temp, "\x01" "FLAGS");

    if (attr1 & aPRIVATE)
        strcat(temp, " PVT");

    if (attr1 & aHOLD)
        strcat(temp, " HLD");

    if (attr1 & aCRASH)
        strcat(temp, " CRA");

    if (attr1 & aKILL)
        strcat(temp, " K/S");

    if (attr1 & aSENT)
        strcat(temp, " SNT");

    if (attr1 & aREAD)
        strcat(temp, " RCV");

    if (attr1 & aAS)
        strcat(temp, " A/S");

    if (attr1 & aDIR)
        strcat(temp, " DIR");

    if (attr1 & aZGT)
        strcat(temp, " ZON");

    if (attr2 & aHUB)
        strcat(temp, " HUB");

    if (attr1 & aFILE)
        strcat(temp, " FIL");

    if (attr1 & aFRQ)
        strcat(temp, " FRQ");

    if (attr1 & aIMM)
        strcat(temp, " IMM");

    if (attr2 & aXMA)
        strcat(temp, " XMA");

    if (attr1 & aKFS)
        strcat(temp, " KFS");

    if (attr1 & aTFS)
        strcat(temp, " TFS");

    if (attr1 & aLOK)
        strcat(temp, " LOK");

    if (attr1 & aRRQ)
        strcat(temp, " RRQ");

    if (attr1 & aCFM)
        strcat(temp, " CFM");

    if (attr2 & aHIR)
        strcat(temp, " HIR");

    if (attr2 & aCOV)
        strcat(temp, " COV");

    if (attr2 & aSIG)
        strcat(temp, " SIG");

    if (attr2 & aLET)
        strcat(temp, " LET");

    if (attr2 & aFAX)
        strcat(temp, " FAX");

}


// ===============================================================

void Flags2Attr(char *s, dword * attr1, dword * attr2)
{

    if (strstr(s, "PVT") != NULL)
        *attr1 |= aPRIVATE;

    if (strstr(s, "HLD") != NULL)
        *attr1 |= aHOLD;

    if (strstr(s, "CRA") != NULL)
        *attr1 |= aCRASH;

    if (strstr(s, "K/S") != NULL)
        *attr1 |= aKILL;

    if (strstr(s, "SNT") != NULL)
        *attr1 |= aSENT;

    if (strstr(s, "RCV") != NULL)
        *attr1 |= aREAD;

    if (strstr(s, "A/S") != NULL)
        *attr1 |= aAS;

    if (strstr(s, "DIR") != NULL)
        *attr1 |= aDIR;

    if (strstr(s, "ZON") != NULL)
        *attr1 |= aZGT;

    if (strstr(s, "HUB") != NULL)
        *attr2 |= aHUB;

    if (strstr(s, "FIL") != NULL)
        *attr1 |= aFILE;

    if (strstr(s, "FRQ") != NULL)
        *attr1 |= aFRQ;

    if (strstr(s, "IMM") != NULL)
        *attr1 |= aIMM;

    if (strstr(s, "XMA") != NULL)
        *attr2 |= aXMA;

    if (strstr(s, "KFS") != NULL)
        *attr1 |= aKFS;

    if (strstr(s, "TFS") != NULL)
        *attr1 |= aTFS;

    if (strstr(s, "LOK") != NULL)
        *attr1 |= aLOK;

    if (strstr(s, "RRQ") != NULL)
        *attr1 |= aRRQ;

    if (strstr(s, "CFM") != NULL)
        *attr1 |= aCFM;

    if (strstr(s, "HIR") != NULL)
        *attr2 |= aHIR;

    if (strstr(s, "COV") != NULL)
        *attr2 |= aCOV;

    if (strstr(s, "SIG") != NULL)
        *attr2 |= aSIG;

    if (strstr(s, "LET") != NULL)
        *attr2 |= aLET;

    if (strstr(s, "FAX") != NULL)
        *attr2 |= aFAX;

}

static int isleap(int year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

#define UNIX_EPOCH 1970

static dword unixtime(const struct tm *tm)
{
    int year, i;
    dword result;

    result = 0UL;
    year = tm->tm_year + 1900;

    /* Traverse through each year */
    for (i = UNIX_EPOCH; i < year; i++)
    {
        result += 31536000UL;  /* 60s * 60m * 24h * 365d = 31536000s */
        if (isleap(i))
        {
            /* It was a leap year; add a day's worth of seconds */
            result += 86400UL;  /* 60s * 60m * 24h = 86400s */
        }
    }

    /* Traverse through each day of the year, adding a day's worth
     * of seconds each time. */
    for (i = 0; i < tm->tm_yday; i++)
    {
        result += 86400UL;  /* 60s * 60m * 24h = 86400s */
    }

    /* Now add the number of seconds remaining */
    result += 3600UL * tm->tm_hour;
    result += 60UL * tm->tm_min;
    result += (dword) tm->tm_sec;

    return result;
}

// ===============================================================

/*
**  JAMsysTime
**
**  Calculates the number of seconds (in local time) that has passed from January 1, 1970
**  until the current date and time.
**
**      Parameters:   UINT32ptr pTime     - Ptr to buffer where the number
**                                          number of seconds will be stored
**
**         Returns:   Number of seconds since 1970 to specified date/time
*/
dword JAMsysTime(dword * pTime)
{
#if 0
    dword ti;
    struct dosdate_t d;
    struct dostime_t t;
    struct JAMtm m;

    getdate(&d);
    gettime(&t);

    memset(&m, 0, sizeof m);

    m.tm_year = d.da_year - 1900;
    m.tm_mon = d.da_mon - 1;
    m.tm_mday = d.da_day;
    m.tm_hour = t.ti_hour;
    m.tm_min = t.ti_min;
    m.tm_sec = t.ti_sec;

    ti = JAMsysMkTime(&m);

    if (pTime)
        *pTime = ti;

    return (ti);
#else
    struct tm *tm;
    time_t now;
    now = time(NULL);

    if (now != (time_t)-1)
    {
        tm = localtime(&now);
        return unixtime(tm);
    }

    return (dword) 0;
#endif
}

/*
**  JAMsysMkTime
**
**  Calculates the number of seconds (in local time) that has passed from January 1, 1970
**  until the specified date and time.
**
**      Parameters:   JAMTMptr pTm        - Ptr to structure containing time
**
**         Returns:   Number of seconds since 1970 to specified date/time
*/
dword JAMsysMkTime(JAMTM * pTm)
{
    dword Days;
    int Years;

    /* Get number of years since 1970 */
    Years = pTm->tm_year - 70;

    /* Calculate number of days during these years, */
    /* including extra days for leap years */
    Days = Years * 365 + ((Years + 1) / 4);

    /* Add the number of days during this year */
    Days += _mdays[pTm->tm_mon] + pTm->tm_mday - 1;
    if ((pTm->tm_year & 3) == 0 && pTm->tm_mon > 1)
        Days++;

    /* Convert to seconds, and add the number of seconds this day */
    return (((dword) Days * 86400L) + ((dword) pTm->tm_hour * 3600L) +
            ((dword) pTm->tm_min * 60L) + (dword) pTm->tm_sec);
}

/*
**  JAMsysLocalTime
**
**  Converts the specified number of seconds (in local time) since January 1, 1970, to
**  the corresponding date and time.
**
**      Parameters:   dword * pt        - Number of seconds since Jan 1, 1970
**
**         Returns:   Ptr to struct JAMtm area containing date and time
*/

JAMTM *JAMsysLocalTime(dword * pt)
{
    static struct JAMtm m;
    sdword t = *pt;
    int LeapDay;

    m.tm_sec = (int)(t % 60);
    t /= 60;
    m.tm_min = (int)(t % 60);
    t /= 60;
    m.tm_hour = (int)(t % 24);
    t /= 24;
    m.tm_wday = (int)((t + 4) % 7);

    m.tm_year = (int)(t / 365 + 1);
    do
    {
        m.tm_year--;
        m.tm_yday = (int)(t - m.tm_year * 365 - ((m.tm_year + 1) / 4));
    }
    while (m.tm_yday < 0);
    m.tm_year += 70;

    LeapDay = ((m.tm_year & 3) == 0 && m.tm_yday >= _mdays[2]);

    for (m.tm_mon = m.tm_mday = 0; m.tm_mday == 0; m.tm_mon++)
        if (m.tm_yday < _mdays[m.tm_mon + 1] + LeapDay)
            m.tm_mday =
                m.tm_yday + 1 - (_mdays[m.tm_mon] +
                                 (m.tm_mon != 1 ? LeapDay : 0));

    m.tm_mon--;

    m.tm_isdst = -1;

    return (&m);
}

// =================================================================
// Give back mem for all elements of a MIS.

void FreeMIS(MIS * mis)
{

    FreeStringList(mis->requested);
    FreeStringList(mis->attached);
    FreeStringList(mis->seenby);
    FreeStringList(mis->path);
    FreeStringList(mis->via);

    if (mis->extrasub)
        free(mis->extrasub);

    memset(mis, '\0', sizeof(MIS));
}

// ====================================================================
// Copy a MIS, including the stringlists etc. This allows easy copying,
// without having to sort out who de-allocates the stringlists..

void CopyMIS(MIS * in, MIS * out)
{
    memcpy(out, in, sizeof(MIS));

    out->requested = out->attached = out->seenby = out->path = out->via =
        NULL;
    out->extrasub = NULL;
    out->extrasublen = 0L;

    if (in->extrasub)
    {
        if ((out->extrasub = malloc(in->extrasublen)) != NULL)
        {
            memcpy(out->extrasub, in->extrasub, in->extrasublen);
            out->extrasublen = in->extrasublen;
        }
    }

    CopyStringList(in->requested, &out->requested);
    CopyStringList(in->attached, &out->attached);
    CopyStringList(in->seenby, &out->seenby);
    CopyStringList(in->path, &out->path);
    CopyStringList(in->via, &out->via);

}
