#ifndef __SQAPI_H_DEFINED
#define __SQAPI_H_DEFINED

#include "stamp.h"
#include "compiler.h"
#include "typedefs.h"

#ifdef __OS2__
#define INCL_BASE
#include <os2.h>
#else
#endif


#define MSGAREA_NORMAL  0x00
#define MSGAREA_CREATE  0x01
#define MSGAREA_CRIFNEC 0x02

#define MSGTYPE_SDM     0x0001
#define MSGTYPE_SQUISH  0x0002
#define MSGTYPE_JAM     0x0004
#define MSGTYPE_HMB     0x0008

#define MSGTYPE_LOCAL   0x0100
#define MSGTYPE_NET     0x0200
#define MSGTYPE_ECHO    0x0400
#define MSGTYPE_MAIL    0x0800
#define MSGTYPE_NEWS    0x1000

#define MSGNUM_CUR      -1L
#define MSGNUM_PREV     -2L
#define MSGNUM_NEXT     -3L

#define MSGNUM_current  MSGNUM_CUR
#define MSGNUM_previous MSGNUM_PREV
#define MSGNUM_next     MSGNUM_NEXT

#define MOPEN_CREATE    0
#define MOPEN_READ      1
#define MOPEN_WRITE     2
#define MOPEN_RW        3
#define MOPEN_RDHDR     4

struct _msgapi;
struct _msgh;
struct _netaddr;

typedef struct _msgapi MSG;
typedef struct _msgh MSGH;
typedef dword UMSGID;
typedef struct _netaddr NETADDR;

#include "spack.h"  /* #pragma pack */

struct _minf
{
    word req_version;
    word def_zone;
    word haveshare;             /* filled in by msgapi routines - no need
                                   to set this */

    /* GvE: */

    word useflags;              /* 1 = use ^AFLAGS kludges */
    word nospace;               /* 1 = NOT use space between filename & pw 
                                   for freq */
    char hudsonpath[120];       /* Path to Hudson base files, no trailing
                                   backslash */
};


/* The network address structure.  The z/n/n/p fields are always             *
 * maintained in parallel to the 'ascii' field, which is simply an ASCII     *
 * representation of the address.  In addition, the 'ascii' field can        *
 * be used for other purposes (such as internet addresses), so the           *
 * contents of this field are implementation-defined, but for most cases,    *
 * should be in the format "1:123/456.7" for Fido addresses.                 */

struct _netaddr
{
    word zone;
    word net;
    word node;
    word point;
};

// ==================================


typedef struct _stringlist
{

    char *s;
    char *pw;
    struct _stringlist *next;

} STRINGLIST;


  /* Bitmasks for 'attr1' */

#define aPRIVATE 0x00000001L
#define aCRASH   0x00000002L
#define aREAD    0x00000004L
#define aSENT    0x00000008L
#define aFILE    0x00000010L
#define aFWD     0x00000020L
#define aORPHAN  0x00000040L
#define aKILL    0x00000080L
#define aLOCAL   0x00000100L
#define aHOLD    0x00000200L
#define aXX2     0x00000400L
#define aFRQ     0x00000800L
#define aRRQ     0x00001000L
#define aCPT     0x00002000L
#define aARQ     0x00004000L
#define aURQ     0x00008000L
#define aSCANNED 0x00010000L
#define aUID     0x00020000L
#define aDIR     0x00040000L
#define aAS      0x00080000L
#define aIMM     0x00100000L
#define aKFS     0x00200000L
#define aTFS     0x00400000L
#define aCFM     0x00800000L
#define aLOK     0x01000000L
#define aZGT     0x02000000L    // ZoneGated (JAM)
#define aENC     0x04000000L    // Encrypted (JAM)
#define aCOM     0x08000000L    // Compressed (JAM)
#define aESC     0x10000000L    // Escaped (JAM)
#define aFPU     0x20000000L    // Force pickup (JAM)
#define aNODISP  0x40000000L    // Don't display to user (JAM)
#define aDEL     0x80000000L    // Deleted

  /* Bitmasks for 'attr2' */

#define aTLOC    0x00000001L    // Local message type
#define aTECHO   0x00000002L    // Echo message type
#define aTNET    0x00000004L    // Netmail message type
#define aUNSENT  0x00000008L    // HMB - netmail - unsent
#define aUNMOVED 0x00000010L    // HMB - echo - unmoved
#define aHUB     0x00000020L    // HUB routing (FLAG)
#define aXMA     0x00000040L    // XMAIL alternative compression (FLAG)
#define aHIR     0x00000080L    // FAX: HIRES FAX attached (FLAG)
#define aCOV     0x00000100L    // FAX: Coversheet (FLAG)
#define aSIG     0x00000200L    // FAX: Signature (FLAG)
#define aLET     0x00000400L    // FAX: Letterhead (FLAG)
#define aFAX     0x00000800L    // FAX: FAX attached (FLAG)


typedef struct
{
    dword attr1;
    dword attr2;

    byte from[101];
    byte to[101];
    byte subj[101];

    NETADDR origfido;           /* Origination and destination addresses */
    NETADDR destfido;

    byte origdomain[101];       /* Origination & destination domains */
    byte destdomain[101];

    byte originter[101];        /* Origination & destination Internet */
    byte destinter[101];

    dword msgwritten;           /* When user wrote the msg (unix) */
    dword msgprocessed;         /* When system processed msg (unix) */
    dword msgreceived;          /* When user wrote the msg (unix) */
    byte ftsc_date[20];         /* ASCII time/date string */

    dword timesread;            /* How many times msg was read */

    sword utc_ofs;              /* Offset from UTC of message writer, in *
                                   minutes.  */

    dword replyto;              /* Message # this message is a reply to */
    dword replies[9];           /* Up to 9 replies to this message */
    dword nextreply;            /* JAM: next reply to original message */

    dword origbase;             /* Msgbase format and type msg originated */
    dword msgno;                /* Message number (or UID for Sq/Hudson) */

    STRINGLIST *attached;       /* List of attached files */
    STRINGLIST *requested;      /* List of requested files */
    STRINGLIST *seenby;
    STRINGLIST *path;
    STRINGLIST *via;

    char *extrasub;             /* Extra unknown subfields (JAM) */
    dword extrasublen;          /* Length of unknown extra subfields */

} MIS;                          // Message Information Structure


// ==================================


/* This is a 'message area handle', as returned by MsgOpenArea(), and       *
 * required by calls to all other message functions.  This structure        *
 * must always be accessed through the API functions, and never             *
 * modified directly.                                                       */

struct _msgapi
{
#define MSGAPI_ID   0x0201414dL

    dword id;                   /* Must always equal MSGAPI_ID */

    word len;                   /* LENGTH OF THIS STRUCTURE! */
    word type;

    dword num_msg;
    dword cur_msg;
    dword high_msg;
    dword high_water;

    word sz_xmsg;

    byte locked;                /* Base is locked from use by other tasks */
    byte isecho;                /* Is this an EchoMail area? */

    /* Function pointers for manipulating messages within this area.  */
    struct _apifuncs
    {
        sword(*CloseArea) (MSG * mh);
        MSGH *(*OpenMsg) (MSG * mh, word mode, dword n);
         sword(*CloseMsg) (MSGH * msgh);
         dword(*ReadMsg) (MSGH * msgh, MIS * mis, dword ofs,
                          dword bytes, byte * text, dword cbyt,
                          byte * ctxt);
         sword(*WriteMsg) (MSGH * msgh, word append, MIS * mis,
                           byte * text, dword textlen, dword totlen,
                           dword clen, byte * ctxt);
         sword(*KillMsg) (MSG * mh, dword msgnum);
         sword(*Lock) (MSG * mh);
         sword(*Unlock) (MSG * mh);
         sword(*SetCurPos) (MSGH * msgh, dword pos);
         dword(*GetCurPos) (MSGH * msgh);
         UMSGID(*MsgnToUid) (MSG * mh, dword msgnum);
         dword(*UidToMsgn) (MSG * mh, UMSGID umsgid, word type);
         dword(*GetHighWater) (MSG * mh);
         sword(*SetHighWater) (MSG * mh, dword hwm);
         dword(*GetTextLen) (MSGH * msgh);
         dword(*GetCtrlLen) (MSGH * msgh);
    } *api;

    /* Pointer to application-specific data.  API_SQ.C and API_SDM.C use *
       this for different things, so again, no applications should muck *
       with anything in here.  */

    void /* far */ *apidata;
};

#include "spop.h"  /* #pragma pop */


/* This is a 'dummy' message handle.  The other message handlers (contained *
 * in API_SQ.C and API_SDM.C) will define their own structures, with some   *
 * application-specified variables instead of other[].  Applications should *
 * not mess with anything inside the _msgh (or MSGH) structure.             */

#define MSGH_ID  0x0302484dL

#if !defined(MSGAPI_HANDLERS) && !defined(NO_MSGH_DEF)
struct _msgh
{
    MSG *sq;
    dword id;

    dword bytes_written;
    dword cur_pos;
};
#endif

/* This variable is modified whenever an error occurs with the Msg...()     *
 * functions.  If msgapierr==0, then no error occurred.                     */

extern word _stdc msgapierr;
extern char *errmsgs[];
extern struct _minf _stdc mi;
extern char dbgmsg[200];

/* Constants for 'type' argument of MsgUidToMsgn() */

#define UID_EXACT     0x00
#define UID_NEXT      0x01
#define UID_PREV      0x02


/* Values for 'msgapierr', above. */

#define MERR_NONE        0      /* No error */
#define MERR_BADH        1      /* Invalid handle passed to function */
#define MERR_BADF        2      /* Invalid or corrupted file */
#define MERR_NOMEM       3      /* Not enough memory for specified
                                   operation */
#define MERR_NODS        4      /* Maybe not enough disk space for
                                   operation */
#define MERR_NOENT       5      /* File/message does not exist */
#define MERR_BADA        6      /* Bad argument passed to msgapi function */
#define MERR_EOPEN       7      /* Couldn't close - messages still open */
#define MERR_SEEK        8      /* Error seeking file */
#define MERR_READ        9      /* Error reading file */
#define MERR_WRITE      10      /* Error writing file */
#define MERR_BADSIG     11      /* Bad signature in JAM message header */
#define MERR_BOARD      12      /* Hudson board doesn't match expected
                                   board */
#define MERR_MSGNO      13      /* Message number doesn't match requested */
#define MERR_BADMSGINFO 14      /* Msginfo.bbs file (HMB) lists incorrect
                                   # of msgs! */
#define MERR_DELETED    15      /* Message seems to be deleted! */
#define MERR_NOLOCK     16      /* Cannot lock actual file */
#define MERR_NOUNLOCK   17      /* Cannot unlock actual file */
#define MERR_OPENFILE   18      /* Cannot open actual file */
#define MERR_BADREV     19      /* Bad revision numder (JAM) */
#define MERR_NOHMB      20      /* Can't open HMB files */
#define MERR_TOOMANYMSG 21      /* Too many msgs in area (Squish, DOS) */

/* Now, a set of macros, which call the specified API function.  These      *
 * will map calls for 'MsgOpenMsg()' into 'SquishOpenMsg()',                *
 * 'SdmOpenMsg()', or '<insert fave message type here>'.  Applications      *
 * should always call these macros, instead of trying to call the           *
 * manipulation functions directly.                                         */

#define MsgCloseArea(mh)       (*(mh)->api->CloseArea) (mh)
#define MsgOpenMsg(mh,mode,n)  (*(mh)->api->OpenMsg)          (mh,mode,n)
#define MsgCloseMsg(msgh)      ((*(((struct _msgh *)msgh)->sq->api->CloseMsg))(msgh))
#define MsgReadMsg(msgh,msg,ofs,b,t,cl,ct) (*(((struct _msgh *)msgh)->sq->api->ReadMsg))(msgh,msg,ofs,b,t,cl,ct)
#define MsgWriteMsg(gh,a,m,t,tl,ttl,cl,ct) (*(((struct _msgh *)gh)->sq->api->WriteMsg))(gh,a,m,t,tl,ttl,cl,ct)
#define MsgKillMsg(mh,msgnum)  (*(mh)->api->KillMsg)(mh,msgnum)
#define MsgLock(mh)            (*(mh)->api->Lock)(mh)
#define MsgUnlock(mh)          (*(mh)->api->Unlock)(mh)
#define MsgGetCurPos(msgh)     (*(((struct _msgh *)msgh)->sq->api->GetCurPos))(msgh)
#define MsgSetCurPos(msgh,pos) (*(((struct _msgh *)msgh)->sq->api->SetCurPos))(msgh,pos)
#define MsgMsgnToUid(mh,msgn)  (*(mh)->api->MsgnToUid)(mh,msgn)
#define MsgUidToMsgn(mh,umsgid,t) (*(mh)->api->UidToMsgn)(mh,umsgid,t)
#define MsgGetHighWater(mh)   (*(mh)->api->GetHighWater)(mh)
#define MsgSetHighWater(mh,n) (*(mh)->api->SetHighWater)(mh,n)
#define MsgGetTextLen(msgh)   (*(((struct _msgh *)msgh)->sq->api->GetTextLen))(msgh)
#define MsgGetCtrlLen(msgh)   (*(((struct _msgh *)msgh)->sq->api->GetCtrlLen))(msgh)

/* These don't actually call any functions, but are macros used to access    *
 * private data inside the _msgh structure.                                  */

#define MsgCurMsg(mh)         ((mh)->cur_msg)
#define MsgNumMsg(mh)         ((mh)->num_msg)
#define MsgHighMsg(mh)        ((mh)->high_msg)

#define MsgGetCurMsg(mh)      ((mh)->cur_msg)
#define MsgGetNumMsg(mh)      ((mh)->num_msg)
#define MsgGetHighMsg(mh)     ((mh)->high_msg)

sword MsgOpenApi(struct _minf *minf, char *path, word useflags,
                 char *hudson);
sword MsgCloseApi(void);

MSG *MsgOpenArea(byte * name, word mode, word type);
sword MsgValidate(word type, byte * name);

sword InvalidMsgh(MSGH * msgh);
sword InvalidMh(MSG * mh);

void SquishSetMaxMsg(MSG * sq, dword max_msgs, dword skip_msgs, dword age);
dword SquishHash(byte * f);
dword SquishUndelete(MSG * sq);

MSG *SdmOpenArea(byte * name, word mode, word type);
sword SdmValidate(byte * name);

int SDMRenumber(MSG * mh);

MSG *SquishOpenArea(byte * name, word mode, word type);
sword SquishValidate(byte * name);

/* GvE: */

MSG *JAMOpenArea(byte * name, word mode, word type);
sword JAMValidate(byte * name);

MSG *HMBOpenArea(byte * name, word mode, word type);
sword HMBValidate(byte * name);

int HMBOpenBase(void);
int HMBCloseBase(void);

byte *CvtCtrlToKludge(byte * ctrl);
byte *GetCtrlToken(byte * where, byte * what);
word _CopyToBuf(byte * p, byte * out, byte ** end, word maxlen);
byte *CopyToControlBuf(byte * txt, byte ** newtext, unsigned *length);
void ConvertControlInfo(byte * ctrl, MIS * mis);
word NumKludges(char *txt);
void RemoveFromCtrl(byte * ctrl, byte * what);
void Convert_Flags(char *kludges, dword * attr1, dword * attr2);
void Attr2Flags(char *temp, dword attr1, dword attr2);
void Flags2Attr(char *s, dword * attr1, dword * attr2);
MSG *CreateAreaHandle(word type);

byte *_fast Address(NETADDR * a);
byte *StripNasties(byte * str);

int AnalyseTrail(char *s, unsigned len, MIS * mis);

STRINGLIST *AddToStringList(STRINGLIST * root, char *s, char *pw,
                            int first);
STRINGLIST *AddnToStringList(STRINGLIST * root, char *s, word len,
                             char *pw, word pwlen);
void FreeStringList(STRINGLIST * start);
void CopyStringList(STRINGLIST * in, STRINGLIST ** out);
int StrListLen(STRINGLIST * l);
int StrListElLen(STRINGLIST * l);

void Extract_Attaches(MIS * mis);
void Extract_Requests(MIS * mis);
void Files2Subject(MIS * mis, char *outsubj);

int AddToString(char **start, dword * curmax, dword * cursize,
                char *string);

void FreeMIS(MIS * mis);
void CopyMIS(MIS * in, MIS * out);

/*
**  Number of days since January 1, the 1st of each month
*/

/*
**  Structure to contain date/time information
*/

#include "spack.h"

typedef struct JAMtm
{
    int tm_sec,                 /* Seconds 0..59 */
     tm_min,                    /* Minutes 0..59 */
     tm_hour,                   /* Hour of day 0..23 */
     tm_mday,                   /* Day of month 1..31 */
     tm_mon,                    /* Month 0..11 */
     tm_year,                   /* Years since 1900 */
     tm_wday,                   /* Day of week 0..6 (Sun..Sat) */
     tm_yday,                   /* Day of year 0..365 */
     tm_isdst;                  /* Daylight savings time (not used) */
} JAMTM;

#include "spop.h"

dword JAMsysTime(dword * pTime);
dword JAMsysMkTime(JAMTM * pTm);
JAMTM *JAMsysLocalTime(dword * pt);

#endif
