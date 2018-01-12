/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

/*
**  Number of days since January 1, the 1st of each month
*/
static int _mdays [13] =
            {
/* Jan */   0,
/* Feb */   31,
/* Mar */   31+28,
/* Apr */   31+28+31,
/* May */   31+28+31+30,
/* Jun */   31+28+31+30+31,
/* Jul */   31+28+31+30+31+30,
/* Aug */   31+28+31+30+31+30+31,
/* Sep */   31+28+31+30+31+30+31+31,
/* Oct */   31+28+31+30+31+30+31+31+30,
/* Nov */   31+28+31+30+31+30+31+31+30+31,
/* Dec */   31+28+31+30+31+30+31+31+30+31+30,
/* Jan */   31+28+31+30+31+30+31+31+30+31+30+31
            };


/*
**  Structure to contain date/time information
*/
typedef struct JAMtm
    {
    int     tm_sec,                    /* Seconds 0..59                     */
            tm_min,                    /* Minutes 0..59                     */
            tm_hour,                   /* Hour of day 0..23                 */
            tm_mday,                   /* Day of month 1..31                */
            tm_mon,                    /* Month 0..11                       */
            tm_year,                   /* Years since 1900                  */
            tm_wday,                   /* Day of week 0..6 (Sun..Sat)       */
            tm_yday,                   /* Day of year 0..365                */
            tm_isdst;                  /* Daylight savings time (not used)  */
    } JAMTM;




/*
**  File extensions
*/
#define EXT_HDRFILE     ".jhr"
#define EXT_TXTFILE     ".jdt"
#define EXT_IDXFILE     ".jdx"
#define EXT_LRDFILE     ".jlr"

/*
**  Revision level and header signature
*/
#define CURRENTREVLEV   1
#define HEADERSIGNATURE "JAM"

/*
**  Header file information block, stored first in all .JHR files
*/

typedef struct
    {
    char   Signature[4];              /* <J><A><M> followed by <NUL> */
    dword  DateCreated;               /* Creation date */
    dword  ModCounter;                /* Last processed counter */
    dword  ActiveMsgs;                /* Number of active (not deleted) msgs */
    dword  PasswordCRC;               /* CRC-32 of password to access */
    dword  BaseMsgNum;                /* Lowest message number in index file */
    char   RSRVD[1000];               /* Reserved space */
    }
    JAMHDRINFO;

/*
**  Message status bits
*/
#define MSG_LOCAL       0x00000001L    /* Msg created locally */
#define MSG_INTRANSIT   0x00000002L    /* Msg is in-transit */
#define MSG_PRIVATE     0x00000004L    /* Private */
#define MSG_READ        0x00000008L    /* Read by addressee */
#define MSG_SENT        0x00000010L    /* Sent to remote */
#define MSG_KILLSENT    0x00000020L    /* Kill when sent */
#define MSG_ARCHIVESENT 0x00000040L    /* Archive when sent */
#define MSG_HOLD        0x00000080L    /* Hold for pick-up */
#define MSG_CRASH       0x00000100L    /* Crash */
#define MSG_IMMEDIATE   0x00000200L    /* Send Msg now, ignore restrictions */
#define MSG_DIRECT      0x00000400L    /* Send directly to destination */
#define MSG_GATE        0x00000800L    /* Send via gateway */
#define MSG_FILEREQUEST 0x00001000L    /* File request */
#define MSG_FILEATTACH  0x00002000L    /* File(s) attached to Msg */
#define MSG_TRUNCFILE   0x00004000L    /* Truncate file(s) when sent */
#define MSG_KILLFILE    0x00008000L    /* Delete file(s) when sent */
#define MSG_RECEIPTREQ  0x00010000L    /* Return receipt requested */
#define MSG_CONFIRMREQ  0x00020000L    /* Confirmation receipt requested */
#define MSG_ORPHAN      0x00040000L    /* Unknown destination */
#define MSG_ENCRYPT     0x00080000L    /* Msg text is encrypted */
#define MSG_COMPRESS    0x00100000L    /* Msg text is compressed */
#define MSG_ESCAPED     0x00200000L    /* Msg text is seven bit ASCII */
#define MSG_FPU         0x00400000L    /* Force pickup */
#define MSG_TYPELOCAL   0x00800000L    /* Msg is for local use only (not for export) */
#define MSG_TYPEECHO    0x01000000L    /* Msg is for conference distribution */
#define MSG_TYPENET     0x02000000L    /* Msg is direct network mail */
#define MSG_NODISP      0x20000000L    /* Msg may not be displayed to user */
#define MSG_LOCKED      0x40000000L    /* Msg is locked, no editing possible */
#define MSG_DELETED     0x80000000L    /* Msg is deleted */

/*
**  Message header
*/
typedef struct
    {
    char   Signature[4];              /* <J><A><M> followed by <NUL> */
    word   Revision;                  /* CURRENTREVLEV */
    word   ReservedWord;              /* Reserved */
    dword  SubfieldLen;               /* Length of subfields */
    dword  TimesRead;                 /* Number of times message read */
    dword  MsgIdCRC;                  /* CRC-32 of MSGID line */
    dword  ReplyCRC;                  /* CRC-32 of REPLY line */
    dword  ReplyTo;                   /* This msg is a reply to.. */
    dword  Reply1st;                  /* First reply to this msg */
    dword  ReplyNext;                 /* Next msg in reply chain */
    dword  DateWritten;               /* When msg was written */
    dword  DateReceived;              /* When msg was received/read */
    dword  DateProcessed;             /* When msg was processed by packer */
    dword  MsgNum;                    /* Message number (1-based) */
    dword  Attribute;                 /* Msg attribute, see "Status bits" */
    dword  Attribute2;                /* Reserved for future use */
    dword  TxtOffset;                 /* Offset of text in text file */
    dword  TxtLen;                    /* Length of message text */
    dword  PasswordCRC;               /* CRC-32 of password to access msg */
    dword  Cost;                      /* Cost of message */
    }
    JAMHDR;

/*
**  Message header subfield types
*/
#define JAMSFLD_OADDRESS    0
#define JAMSFLD_DADDRESS    1
#define JAMSFLD_SENDERNAME  2
#define JAMSFLD_RECVRNAME   3
#define JAMSFLD_MSGID       4
#define JAMSFLD_REPLYID     5
#define JAMSFLD_SUBJECT     6
#define JAMSFLD_PID         7
#define JAMSFLD_TRACE       8
#define JAMSFLD_ENCLFILE    9
#define JAMSFLD_ENCLFWALIAS 10
#define JAMSFLD_ENCLFREQ    11
#define JAMSFLD_ENCLFILEWC  12
#define JAMSFLD_ENCLINDFILE 13
#define JAMSFLD_EMBINDAT    1000
#define JAMSFLD_FTSKLUDGE   2000
#define JAMSFLD_SEENBY2D    2001
#define JAMSFLD_PATH2D      2002
#define JAMSFLD_FLAGS       2003
#define JAMSFLD_TZUTCINFO   2004
#define JAMSFLD_UNKNOWN     0xffff

/*
**  Message header subfield
*/
typedef struct
    {
    word   LoID;                      /* Field ID, 0 - 0xffff */
    word   HiID;                      /* Reserved for future use */
    dword  DatLen;                    /* Length of buffer that follows */
    char   Buffer[1];                 /* DatLen bytes of data */
    }
    JAMSUBFIELD;

typedef struct
    {
    word   LoID;                      /* Field ID, 0 - 0xffff */
    word   HiID;                      /* Reserved for future use */
    dword  DatLen;                    /* Length of buffer that follows */
    }
    JAMBINSUBFIELD;

/*
**  Message index record
*/
typedef struct
    {
    dword  UserCRC;                   /* CRC-32 of destination username */
    dword  HdrOffset;                 /* Offset of header in .JHR file */
    }
    JAMIDXREC;

/*
**  Lastread structure, one per user
*/
typedef struct
    {
    dword  UserCRC;                   /* CRC-32 of user name (lowercase) */
    dword  UserID;                    /* Unique UserID */
    dword  LastReadMsg;               /* Last read message number */
    dword  HighReadMsg;               /* Highest read message number */
    }
    JAMLREAD;




/*

    Here go some changes by Gerard van Essen, to buffer index entries.
    The buffer will be quite small (512 bytes) but may speed up
    sequential 'processing' of the index with repeated JAMmbFetchIdx calls
    dramatically.
    It only does READ buffering. Invalidate cache as soon as writes are
    made. Always use special write function (that does that) to write
    index entries, to prevent troubles..

*/

#define IDXBUFSIZE 60    /* Number of index entries to buffer */

typedef struct _idxcache
{

    time_t    birth;                 /* Unix time when data was read */
    dword     base;                  /* Lowest msgnumber in cache    */
    dword     high;                  /* Highest msg number in cache  */
    JAMIDXREC contents[IDXBUFSIZE];  /* The cached index entries     */
    byte      active;                /* 1 if any useful data cached  */

} IDXCACHE;


/* End of index buffering changes */

typedef struct
{

   int        HdrHandle;
   int        IdxHandle;
   int        LrdHandle;
   int        TxtHandle;

   JAMHDRINFO HdrInfo;
   JAMHDR     Hdr;
   JAMIDXREC  Idx;

   IDXCACHE   IdxCache;

   dword      lastread;

} JAMDATA;


