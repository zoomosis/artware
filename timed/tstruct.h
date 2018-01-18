
//#define __SENTER__ 1

#define VERSION   "1.11.a1"

#define LOCALCHARSET "IBMPC"

#define tMAXNAMES 20
#define tMAXAKAS  35

#ifdef __OS2__

   #define  INCL_NOPMAPI

#endif

#ifdef __FLAT__

   #define word      unsigned short
   #define sword     short

#else

   #define word      unsigned int
   #define sword     int

#endif


#ifdef __WATCOMC__
  #define strncmpi   strnicmp
  #define fnsplit    _splitpath
  #define fnmerge    _makepath
  #define heapcheck  _heapchk
  #define clrscr()   cls()
  #define textattr
#endif

#define NETMAIL   1
#define LOCAL     2
#define ECHOMAIL  3
#define NEWS      4
#define MAIL      5


#define NEXT           -1
#define PREV           -2
#define ESC            -3
#define REPLY          -4
#define ENTER          -5
#define KILL           -6
#define CHANGE         -7
#define MOVE           -8
#define REPLYOTHER     -9
#define PRINT          -10
#define KLUDGES        -11
#define HOME           -12
#define END            -13
#define UP             -14
#define DOWN           -15
#define TAB            -16
#define BTAB           -17
#define ABORT          -18
#define LIST           -17
#define FIND           -18
#define RESET          -19
#define ACCEPT         -20
#define COPY           -21
#define REPORIG        -22
#define EXIT           -23
#define EDITHELLO      -24
#define SHOWINFO       -25
#define HELP           -26
#define TURBOREP       -27
#define NEXTAREA       -28
#define PREVAREA       -29
#define CHANGEHDR      -30
#define BROADLIST      -31
#define HARDCOPY       -32
#define F2             -33
#define MSGMAINT       -34
#define REQUEST        -35
#define SAVE           -36
#define BOUNCEREPLY    -37
#define ATTACH         -38
#define EXTERNAL       -39
#define FILTERMSG      -40
#define SQUNDELETE     -41
#define SDMRENUMBER    -42
#define FILTERREALBODY -43
#define CHANGECHARSET  -44

// ==============================================

#define TIMEDCODE 0xFAFA9090L

typedef struct
{
   char   rand1[12];
   dword  generated;
   char   rand2[10];
   dword  keyval;
   char   rand3[7];
   dword  ascvals;
   dword  crc2;
   char   regsite;
   word   serial;
   dword  ascvals2;
   dword  ascvals3;
   char   rand4[17];
   word   ascvals4;
   word   zero;
   char   rand5[14];
   dword  total;
   char   rand6[5];

} STRKEY;


typedef struct
{

   char   bytes[100];

} ARRKEY;


union KEY
{
   STRKEY strkey;
   ARRKEY arrkey;
};

#define TIMREGISTERED 1

// ====================================================

typedef struct linelist      /* Used to store "wrapped" lines  */
   {
   char            *ls;      /* line start                     */
   char            status;   /* what kind of line? (see below) */
   char            len;
   struct linelist *next;    /* next line                      */
   struct linelist *prev;
   } LINE;

/* defines for status of a line */

#define    NORMTXT    0x01
#define    QUOTE      0x02
#define    ORIGIN     0x04
#define    KLUDGE     0x08
#define    TEAR       0x10
#define    NOSPACE    0x20
#define    HCR        0x40
#define    HIGHLIGHT  0x80


typedef struct _rawblock
{

   char *             txt;
   unsigned           curlen;
   unsigned           maxlen;
   unsigned           curmaxlen;
   unsigned           delta;
   char               full;
   char *             curend;
   struct _rawblock * next;

} RAWBLOCK;


typedef struct
{

   int   active;
   int   max;
   int   current;
   dword *list;

} MARKLIST;


// Structure to hold an entire message in memory.

typedef struct
{

   MIS      mis;           /* Header of msg                            */
   char     *msgtext;      /* Pointer to raw msgtext                   */
   LINE     *txt;          /* Pointer to first line of msg             */
   RAWBLOCK *firstblock;   /* Pointer to list of raw txt blocks        */
   char     *ctxt;         /* Pointer to control info                  */
   char     *ctext;        /* Pointer to control info, formatted dupe  */
   char     id[300];       /* Holds MSGID (if present)                 */
   NETADDR  rep_addr;      /* FSC35, replyto: kludge, gate's fido addr */
   char     rep_gate[36];  /* FSC35, name in to: field                 */
   char     rep_name[300]; /* FSC35, TO: name in msg body              */
   char     org[80];       /* Organisation of a mail/news message      */
   UMSGID   uid;           /* Holds squish' UMSGID                     */
   UMSGID   replynext;     /* Points to next reply in chain (JAM only) */
   dword    txtsize;       /* Size of msg body                         */
   dword    ctrlsize;      /* Size of kludges                          */
   word     status;        /* See status bits below..                  */

}  MMSG;

/* Status bits for MMSG.status */

#define PERSONAL   0x01
#define USENET     0x02
#define REPLYTO    0x04
#define SPELLCHECK 0x08
#define SIGN       0x10
#define ENCRYPT    0x20

/* typedef enum {Yes, NO} YesNo; */

typedef struct arearec
{

   char   *         desc;       /* Description of area                  */
   char   *         tag;        /* Official area tag                    */
   char             group;      /* Group of an area                     */
   char   *         dir;        /* Directory/Base name                  */
   word             base;       /* MSGTYPE_SDM or MSGTYPE_SQUISH        */
   word             type;       /* ECHOMAIL, NETMAIL or LOCAL           */
   long             stdattr;    /* "Standard" attributes of area        */
   long             lr;         /* Lastread pointer                     */
   long             nomsgs;     /* Number of msgs in area               */
   long             highest;    /* Highest messagenumber in area        */
   long             lowest;     /* Lowest messagenumber in area         */
   char             scanned;    /* Is the area scanned? 0=No            */
   char             tagged;     /* Is this area tagged? 0=No            */
   char             aka;        /* What AKA should be used?             */
   char             newmail;    /* Any new mail entered in area?        */
   char             readonly;   /* Is this a readonly area?             */
   char             csread[9];  /* Charset used to convert to for read  */
   char             cswrite[9]; /* Charset used to convert to for write */
   MARKLIST *       mlist;      /* List of marked messages              */
   MARKLIST *       mayseelist; /* List of messages user may see (pvt)  */
   struct arearec * next;       /* Pointer to next area                 */
   struct arearec * prev;       /* Pointer to previous area             */

}  AREA;


typedef struct
{
   char name[40];
   long hash;              /* Hash value of name, for Squish areas */
   unsigned long crc;      /* CRC value of name, for JAM areas     */

} NAME;


typedef struct _outfilelist
{

   char                * filename;
   struct _outfilelist * next;

} OUTFLIST;



typedef struct
{
   NAME     name[tMAXNAMES];   /* Name of the user               */
   char     *registered;       /* Pointer to char that = 1 if reg'ed */
   NETADDR  address[tMAXAKAS]; /* Addresses                      */
   NETADDR  uucpaddress;       /* Address of UUCP gate           */
   char     uucpname[36];      /* Username to use for gated msgs */
   char     nodelist[100];     /* Path to V7 nodelist files      */
   char     fdnodelist[100];   /* Path to FD nodelist files      */
   char     fidouser[100];     /* Path and name for fidouser.lst */
   char     localfiles[100];   /* Path to copy local attaches to */
   char     origin[100];       /* Default origin to use          */
   char     signoff[301];      /* Default signoff (greetings..)  */
   char     echolog[100];      /* Echotoss.log file              */
   char     editor[200];       /* Name of the editor to be used  */
   char     exesign[100];      /* What to execute for signing    */
   char     execrypt[100];     /*                     crypting   */
   char     execryptsign[100]; /*                     cryp + sig */
   char     exespell[100];     /*                     spellcheck */
   char     internal_edit;     /* 1 == use internal editor       */
   char     hudsonpath[100];   /* Path to the Hudson msgbase     */
   char     jamlogpath[100];   /* Path to the Echomail.JAM files */
   char     netsema[100];      /* Semaphore for new netmail      */
   dword    status;            /* Some bits (showkludge etc)     */
   char     delreply;          /* Netmail: del orig after rep. 0=no, 1=yes, 2=ask_confirm */
   char     zonegate;          /* Do zonegate, values above      */
   char     echoinfo;          /* Add tear + origin for echomail */
   char     localinfo;         /*                       local    */
   dword    frqattr;           /* Standard attributes for freq   */
   char     alistsort[6];      /* Arealist sorting               */
   char     lr[20];            /* Name of lastread file          */
   word     sqoff;             /* Offset in Squish .SQL file     */
   word     hudsonoff;         /* Offset in Hudson lastread file */
   char     rephello[301];     /* To start reply with..          */
   char     hello[301];        /* To start msg with..            */
   char     followhello[301];  /* To a followup message with...  */
   char     writename[120];    /* Default name to write to       */
   char     cfmfile[120];      /* Tempplate to use for CFM msgs  */
   OUTFLIST *outfiles;         /* List extra defined writenames  */
   char     printer[20];       /* Printerdevice to write to      */
   char     lines;             /* Number of rows on the screen   */

} USER;

/* status bits: */

#define   SHOWKLUDGES     0x00000001L
#define   CCVERBOSE       0x00000002L
#define   SWAPEDIT        0x00000004L
#define   SWAPSHELL       0x00000008L
#define   CONFIRMDELETE   0x00000010L
#define   CONFIRMEXIT     0x00000020L
#define   SKIPRECEIVED    0x00000040L
#define   NETTEAR         0x00000080L
#define   SHOWNOTES       0x00000100L
#define   INTLFORCE       0x00000200L
#define   ENDAREAMENU     0x00000400L
#define   CONFIRMEDITEXIT 0x00000800L
#define   SHOWEDITHCR     0x00001000L
#define   JUMPYEDIT       0x00002000L
#define   LOWLEVELKB      0x00004000L
#define   CLOCK           0x00008000L
#define   AKAMATCHING     0x00010000L
#define   EMPTYTEAR       0x00020000L
#define   ARCMAIL         0x00040000L
#define   READNET         0x00080000L
#define   READLOCAL       0x00100000L
#define   ENTERMARKS      0x00200000L
#define   JAMZGTBIT       0x00400000L
#define   JAMGETLAST      0x00800000L
#define   EDITSAVEMENU    0x01000000L
#define   SPELLCHECKDFLT  0x02000000L
#define   INTERNETEMPTY   0x04000000L
#define   NOSPACEPASSWORD 0x08000000L
#define   REPLYTOCFM      0x10000000L
#define   RESPECTPRIVATE  0x20000000L


typedef struct _macrolist
{

   char              macro[20];
   char              toname[101];
   NETADDR           toaddress;
   char              subject[101];
   char              usenet[101];
   struct _macrolist *next;

} MACROLIST;

#define Casbar           0
#define Casframe         1
#define Castext          2
#define Cashigh          3
#define Casspecial       4
#define Casaccent        5
#define Cashighaccent    6

#define Cmsgheader       7
#define Cmsgdate         8
#define Cmsgdata         9
#define Cmsglinks       10
#define Cmsgattribs     11
#define Cmsgline        12
#define Cmsgtext        13
#define Cmsgquote       14
#define Cmsgorigin      15
#define Cmsgkludge      16
#define Cmsgbar         17
#define Cmsgbaraccent   18
#define Cmsgspecial     19

#define Centry          20

#define Ceditcurnormal  21
#define Ceditcurquote   22
#define Cedithcr        23
#define Ceditblock      24
#define Ceditcurblock   25

#define Cpopframe       26
#define Cpoptext        27
#define Cpoptitle       28

#define Cfindactive     29
#define Cfindpassive    30
#define Cfindtext       31
#define Cfinddata       32
#define Cfindhigh       33
#define Cfindaccent     34

#define MAXCOL 35


// -----------

typedef struct _akaforce
{
   NETADDR            mask;   // -9 in a mask means '*'.
   int                aka;
   struct _akaforce * next;

} AKAFORCE;

// -----------

typedef struct
{

   word value;
   char flag[10];
   char type;

} V7FLAGS;

// Types for V7flags:

#define MODEMBIT   1
#define MODEMVALUE 2
#define NODEBIT    3

#define MAXV7FLAGS 50

typedef struct
{

   AREA         *first;
   MACROLIST    *firstmacro;
   word         col[MAXCOL];
   AKAFORCE     *akaforce;
   char         homedir[80];
   char         mode;
   char         drivemap[26];
   union KEY    key;
   USER         usr;           /*  User data  */
   V7FLAGS      V7flags[MAXV7FLAGS];
   STRINGLIST   *FirstLevelOne;

}  CFG;

// #defines for 'mode' above..

#define SHOWALL       0
#define SHOWTAGGED    1
#define SHOWNEW       2
#define SHOWNEWTAGGED 3


/* Some 'custom' info for a certain area, taken from xxxx.SQT */

typedef struct
{

   char   hello[301];
   char   rephello[301];
   char   followhello[301];
   char   signoff[301];
   char   origin[101];
   char   csread[9];
   sword  csreadlevel;
   word   mode;          /* DIRECTLIST or NORMAL_MODE, mode to enter area */
   word   aka;
   word   name;

} CUSTOM;


/* Defines for the 'mode' parameter area entry */


#define DIRECTLIST     0   /* Enter area in L)ist mode */
#define NORMAL_MODE    1   /* Normal entry of area     */


/* For the results of nodelist searches */

typedef struct _addrlist
{

   NETADDR           address;
   char              name[40];
   char              system[45];
   char              location[30];
   char              flags[60];
   char              phone[20];
   dword             baud;
   struct _addrlist  *next;

}  ADDRLIST;


/*
 *  bmg.h ->    Boyer-Moore-Gosper search definitions
 *
 *  see bmg.c for documentation
 */


#define bmgMAXPAT   22      /*  max pattern length  */
typedef struct
    {
    char    delta[256];         /*  ASCII only deltas      */
    char    pat[bmgMAXPAT + 1]; /*  the pattern            */
    char    ignore;             /*  ignore case flag       */
    char    wholeword;          /*  match whole words only */
    }
    bmgARG;

// defines for 'howstrict' above..

#define ALWAYS    0x00
#define NOTFIRST  0x01
#define NOTLAST   0x02

/*
** DOW.H - day-of-week macro.  From the FidoNet CECHO - by
** Paul Schlyter.
*/

 #define dow(y,m,d)  \
        ( ( ( 3*(y) - (7*((y)+((m)+9)/12))/4 + (23*(m))/9 + (d) + 2    \
        + (((y)-((m)<3))/100+1) * 3 / 4 - 15 ) % 7 ) )



/* First block, control block, to get node-size and Root */

typedef struct
{

   char         fill1[8];
   word         startblock;
   char         fill2[2];
   word         blocklen;  /* 1061 for Userlist, 741 for Node-index */
   char         fill3[243];
   char         node_ext[3];

}  FDCTL;


/* This is (repeated) part of a node in the userlist index */

typedef struct
{
   long         offset;          /* Offset in raw nodelist       */
   word         ptr;             /* Nodenumber for index entries */
   char         fill[3];         /* No idea                      */
   char         name[15];        /* Sysop name, Upper case, last name first */
   sword        zone;            /* Address zone, hi <-> lo      */
   sword        net;             /*         net                  */
   sword        node;            /*         node                 */
   sword        point;           /*         point                */
   char         status;          /* Host, Hub or down etc        */

} FDUENTRY;


/* A User-index node */

typedef struct
{

   char         count;
   word         nextlevel;
   char         fill[2];
   FDUENTRY     nodedata[32];

}  FDUNODE;


/* This is a (repeated) part of a node in the nodelist index */

typedef struct
{

   unsigned long offset;         /* Offset in raw nodelist       */
   word          ptr;            /* Nodenumber for index entries */
   char          fill1[3];       /* No idea                      */
   sword         zone;           /* Address zone, hi <-> lo      */
   sword         net;            /*         net                  */
   sword         node;           /*         node                 */
   sword         point;          /*         point                */
   sword         host;           /* Net number of Host (host/0)  */
   sword         hub;            /* Node number of Hub           */
   char          fill2[2];       /* Who knows?                   */

} FDNENTRY;


/* A node-index node */

typedef struct
{
   char         count;        /* Number of entries in nodedata[]  */
   word         nextlevel;    /* First entry too large, goto this */
   char         fill[2];      /* Who knows?                       */
   FDNENTRY     nodedata[32]; /* Count Fido-Nodes in this Node    */

} FDNNODE;


typedef struct
{

   long                  Erased;             /*Used to signal erased status*/
   unsigned char         Status;                    /*Zone, host, hub, etc.*/
   unsigned short int    NodeNo,                          /*Network address*/
                         NetNo,
                         Zone,
                         Point,
                         RoutNode,            /*Default routing within zone*/
                         RoutNet,
                         Cost;                 /*Cost per minute for system*/
   long                  Capability;                     /*Capability flags*/
   unsigned char         MaxBaud;                       /*Maximum baud rate*/
   char                  Name[31];                         /*Name of system*/
   char                  Telephone[41];              /*Raw telephone number*/
   char                  Location[41];                 /*Location of system*/
   char                  User[37];                             /*SysOp name*/
   char                  SelectTag[4];                        /*Group field*/

}  FDANODE;



struct my_idx
{

  dword fill;
  UMSGID umsgid;
  dword hash;

};



typedef struct _msglist
{

   char              from[21];
   char              to[21];
   char              subj[28];
   int               tagged;
   dword             n;
   struct _msglist   *next;

} MSGLIST;


struct _mysqdata
{

  int sfd;                /* SquishFile handle */
  int ifd;                /* SquishIndex handle */

  char base[80];          /* Base name for SquishFile */

  long begin_frame;       /* Offset of first frame in file */
  long last_frame;        /* Offset to last frame in file */
  long free_frame;        /* Offset of first FREE frame in file */
  long last_free_frame;   /* Offset of LAST free frame in file */
  long end_frame;         /* Pointer to end of file */

  long next_frame;
  long prev_frame;
  long cur_frame;

  dword uid;
  dword max_msg;
  dword skip_msg;
/*dword zero_ofs;*/
  word keep_days;

  byte flag;
  byte rsvd1;

  word sz_sqhdr;
  byte rsvd2;

  word len;              /* Old length of sqb structure                     */

  dword idxbuf_size;     /* Size of the allocated buffer                    */
  dword idxbuf_used;     /* # of bytes being used to hold messages          */
  dword idxbuf_write;    /* # of bytes we should write to index file        */
  dword idxbuf_delta;    /* Starting position from which the index has chhg */

  byte  delta[256];      /* Copy of last-read sqbase, to determine changes   */
  word msgs_open;

};


/* Should timEd ask confirmation if only 1 address found? */

#define NOPROMPT 0
#define PROMPT   1


/* defines for 'displaytype' in ShowMsg function */

#define NORMAL_DISPLAY 0
#define SCAN_DISPLAY   1


/*--------------------------------------------------------------------------*/
/* nodex.ndx                                                                */
/*                                                                          */
/* Version 7 Nodelist Index structure.  This is a 512-byte record, which    */
/* is defined by three structures:  Record 0 is the Control Record, then    */
/* some number of Leaf Node (LNode) Records, then the Index Node (INode)    */
/* Records.  This defines an unbalanced binary tree.                        */
/*                                                                          */
/* This description is based on Scott Samet's CBTREE.PAS program.           */
/*                                                                          */
/*--------------------------------------------------------------------------*/

struct _ndx {
    union
    {
        struct _CtlBlk {
            word    CtlBlkSize; /* Blocksize of Index Blocks   */
            long    CtlRoot;    /* Block number of Root        */
            long    CtlHiBlk;   /* Block number of last block  */
            long    CtlLoLeaf;  /* Block number of first leaf  */
            long    CtlHiLeaf;  /* Block number of last leaf   */
            long    CtlFree;    /* Head of freelist            */
            word    CtlLvls;    /* Number of index levels      */
            word    CtlParity;  /* XOR of above fields         */
        } CtlBlk;

        struct _INodeBlk {
            long    IndxFirst;  /* Pointer to next lower level */
            long    IndxBLink;  /* Pointer to previous link    */
            long    IndxFLink;  /* Pointer to next link        */
            sword   IndxCnt;    /* Count of Items in block     */
            word    IndxStr;    /* Offset in block of 1st str  */
            /* If IndxFirst is NOT -1, this is INode:          */
            struct _IndxRef {
                word   IndxOfs; /* Offset of string into block */
                word   IndxLen; /* Length of string            */
                long   IndxData;/* Record number of string     */
                long   IndxPtr; /* Block number of lower index */
            } IndxRef[27];
        } INodeBlk;

        struct _LNodeBlk {
                                /* IndxFirst is -1 in LNodes   */
            long    IndxFirst;  /* Pointer to next lower level */
            long    IndxBLink;  /* Pointer to previous link    */
            long    IndxFLink;  /* Pointer to next link        */
            sword   IndxCnt;    /* Count of Items in block     */
            word    IndxStr;    /* Offset in block of 1st str  */
            struct _LeafRef {
                word   KeyOfs;  /* Offset of string into block */
                word   KeyLen;  /* Length of string            */
                long   KeyVal;  /* Pointer to data block       */
            } LeafRef[35];
        } LNodeBlk;

        char RawNdx[512];

    } ndx;
};

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* OPUS 1.20 Version 7 Nodelist structure. Copyright 1991 Wynn Wagner III   */
/* and Doug Boone. Used by permission.                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/

struct _vers7 {
        sword  Zone;
        sword  Net;
        sword  Node;
        sword  HubNode;        /* If node is a point, this is point number. */
        word CallCost;         /* phone company's charge */
        word MsgFee;           /* Amount charged to user for a message */
        word NodeFlags;        /* set of flags (see below) */
        byte ModemType;        /* RESERVED for modem type */
        byte Phone_len;
        byte Password_len;
        byte Bname_len;
        byte Sname_len;
        byte Cname_len;
        byte pack_len;
        byte BaudRate;         /* baud rate divided by 300 */
};

/*------------------------------------------------------------------------*/
/* Values for the `NodeFlags' field                                       */
/*------------------------------------------------------------------------*/
#define B_hub    0x0001  /* node is a net hub     0000 0000 0000 0001 */
#define B_host   0x0002  /* node is a net host    0000 0000 0000 0010 */
#define B_region 0x0004  /* node is region coord  0000 0000 0000 0100 */
#define B_zone   0x0008  /* is a zone gateway     0000 0000 0000 1000 */
#define B_CM     0x0010  /* runs continuous mail  0000 0000 0001 0000 */
#define B_res1   0x0020  /* reserved by Opus      0000 0000 0010 0000 */
#define B_res2   0x0040  /* reserved by Opus      0000 0000 0100 0000 */
#define B_res3   0x0080  /* reserved by Opus      0000 0000 1000 0000 */
#define B_res4   0x0100  /* reserved by Opus      0000 0001 0000 0000 */
#define B_res5   0x0200  /* reserved for non-Opus 0000 0010 0000 0000 */
#define B_res6   0x0400  /* reserved for non-Opus 0000 0100 0000 0000 */
#define B_res7   0x0800  /* reserved for non-Opus 0000 1000 0000 0000 */
#define B_point  0x1000  /* node is a point       0001 0000 0000 0000 */
#define B_res9   0x2000  /* reserved for non-Opus 0010 0000 0000 0000 */
#define B_resa   0x4000  /* reserved for non-Opus 0100 0000 0000 0000 */
#define B_resb   0x8000  /* reserved for non-Opus 1000 0000 0000 0000 */



struct _xfile {
    int   fd;
    int   bufSize;
    char *buf;
    char *nextChar;
    char *lastChar;
};

typedef struct _xfile XFILE;

// Valid filename chars. Well, anything is allowed now.. (OS/2 Scandinavians w/ „ etc)

#define VFN ""
#define VFNWW ""

// Some stuff used by the 'dirlist' routines.

#define FTAGGED 0x01
#define FDIR    0x02
#define UPDIR   0x04


typedef struct _flist
{

   char          name[90];
   long          size;
   char          status;

} FLIST;


typedef struct
{

   FLIST **  liststart;
   unsigned  curnum;
   unsigned  max;

} FILELIST;

// Some defs for date types

#define DATE_FULL 0
#define DATE_LIST 1
#define DATE_HDR  2

// Some defs for GetRawMsg

#define READNOBODY 0
#define READALL    1


typedef struct _attachlist
{
   char name[30];
   long size;
   struct _attachlist *next;

} ATTACHLIST;

// =============================================================

typedef struct
{

   int     len;
   sword * start;

} KEYMACRO;


// c - command, AS - Area Selection

#define cAStag          -1
#define cASsetview      -2
#define cASup           -3
#define cASdown         -4
#define cAShome         -5
#define cASend          -6
#define cASpageup       -7
#define cASpagedown     -8
#define cASshell        -9
#define cASscanthorough -10
#define cASscan         -11
#define cASscanpersonal -12
#define cAStagsetwrite  -13
#define cAStagsetread   -14
#define cASexit         -15
#define cASlist         -16
#define cASenter        -17
#define cASquit         -18
#define cAShelp         -19

// c - command, EDIT - Internal Editor

#define cEDITbegline        -100
#define cEDITendline        -101
#define cEDITbegtext        -102
#define cEDITendtext        -103
#define cEDITbegpage        -104
#define cEDITendpage        -105
#define cEDITup             -106
#define cEDITdown           -107
#define cEDITright          -108
#define cEDITleft           -109
#define cEDITenter          -110
#define cEDITdel            -111
#define cEDITback           -112
#define cEDITtab            -113
#define cEDITbacktab        -114
#define cEDITpageup         -115
#define cEDITpagedown       -116
#define cEDITtoggleinsert   -117
#define cEDITabort          -118
#define cEDITsave           -119
#define cEDITshell          -120
#define cEDITimportfile     -121
#define cEDITdeltoeol       -122
#define cEDITmarkblock      -123
#define cEDITunmarkblock    -124
#define cEDITdelblock       -125
#define cEDITcopyblock      -126
#define cEDITmoveblock      -127
#define cEDITzapquotes      -128
#define cEDITdupline        -129
#define cEDITeditheader     -130
#define cEDITdelwordright   -131
#define cEDITdelwordleft    -132
#define cEDITunerase        -133
#define cEDITjumpwordright  -134
#define cEDITjumpwordleft   -135
#define cEDITdelline        -136
#define cEDIThelp           -137
#define cEDITtogglehcr      -138
#define cEDITsavemenu       -139
#define cEDITrunexternal    -140
#define cEDITwriteraw       -141
#define cEDITwritefmt       -142
#define cEDITwriterawblock  -143
#define cEDITwritefmtblock  -144
#define cEDITfiledelete     -145


// c - command, READ - message reading screen.

#define cREADup                -200
#define cREADdown              -201
#define cREADbegtext           -202
#define cREADendtext           -203
#define cREADpageup            -204
#define cREADpagedown          -205
#define cREADnext              -206
#define cREADprevious          -207
#define cREADfirstmsg          -208
#define cREADlastmsg           -209
#define cREADfind              -210
#define cREADlist              -211
#define cREADbroadlist         -212
#define cREADedithello         -213
#define cREADexit              -214
#define cREADdelete            -215
#define cREADwrite             -216
#define cREADprint             -217
#define cREADmove              -218
#define cREADmarkchain         -219
#define cREADreply             -220
#define cREADfollowup          -221
#define cREADturboreply        -222
#define cREADshell             -223
#define cREADgoback            -224
#define cREADfreqfiles         -225
#define cREADenter             -226
#define cREADreplyother        -227
#define cREADchange            -228
#define cREADchangeheader      -229
#define cREADchangeattributes  -230
#define cREADunreceive         -231
#define cREADbouncereply       -232
#define cREADmark              -233
#define cREADinfo              -234
#define cREADchangeaddress     -235
#define cREADchangename        -236
#define cREADtogglekludges     -237
#define cREADmaintenance       -238
#define cREADnextareanewmail   -239
#define cREADprevareanewmail   -240
#define cREADnextmarked        -241
#define cREADprevmarked        -242
#define cREADnextmsgorpage     -243
#define cREADgotoreply         -244
#define cREADgotooriginal      -245
#define cREADnextreply         -246
#define cREADsetbookmark       -247
#define cREADreturnbookmark    -248
#define cREADlookuporigaddress -249
#define cREADlookuptoname      -250
#define cREADlookupfromname    -251
#define cREADhelp              -252
#define cREADrunexternal       -253
#define cREADfiltermsg         -254
#define cREADfiltermemory      -255
#define cREADfilterrealbody    -256
#define cREADwritebody         -257
#define cREADwriterealbody     -258
#define cREADsqundelete        -259
#define cREADsdmrenumber       -260
#define cREADwriteheader       -261
#define cREADfiledelete        -262
#define cREADsearchcurmsg      -263
#define cREADchangecharset     -264

#define cLISTup          -300
#define cLISTdown        -301
#define cLISTpageup      -302
#define cLISTpagedown    -303
#define cLISThome        -304
#define cLISTend         -305
#define cLISTmove        -306
#define cLISTcopy        -307
#define cLISTdelete      -308
#define cLISTwrite       -309
#define cLISTexit        -310
#define cLISTswitch      -311
#define cLISTprint       -312
#define cLISThelp        -313
#define cLISTabort       -314
#define cLISTdetails     -315
#define cLISTshell       -316
#define cLISTreadmsg     -317
#define cLISTtag         -318
#define cLISTtagrange    -319
#define cLISTuntagrange  -320
#define cLISTtagall      -321
#define cLISTuntagall    -322

#define GLOBALSCOPE 0
#define AREASCOPE   1
#define READERSCOPE 2
#define EDITORSCOPE 3
#define LISTSCOPE   4
#define MENUSCOPE   5


// ==============================================================
//
// Stuff used to build the 'Menu' with user macro's in reader
//
// ==============================================================

// These are negative, to see a difference from normal keycodes
// Not -1, because that is ERROR.

#define MENUALL    -11
#define MENUJAM    -12
#define MENUSQUISH -13
#define MENUSDM    -14
#define MENUHMB    -15

typedef struct
{

   sword   where;   // One of MENUALL, MENUJAM etc. above
   char  * desc;    // Description, with ~ for highlighting, in menu
   sword   macro;   // Macro number.

} MENUENTRY;

#define MAXMENUENTRIES 30


// ==============================================================
//
// Character Translation stuff
//
// ==============================================================

typedef struct
{
     
  sdword id;                   // Must be 0-65536 for official FTSC
                               // charset modules, normally 0.
                               // Use a timestamp to mark your own
                               // derivates.
  sword  version;              // Version of this module.
  sword  level;                // charset level.
  sbyte  filler[8];            // reserved, set to zeros.
  sbyte  from_set[8];          // Original charset.
  char   to_set[8];            // Charset this translates to.
  byte   lookup_table[128][2];

} CHARREC;

/*****************************************************
 *
 * This defines the first bytes of a mapping file
 *
 *****************************************************/

typedef struct
{

  char  architecture;         /* Hardware architecture of the machine */
  char  filler[3];            /* this mapping file was written on.    */
  char  charset[8];           /* Charset it was intended for.         */

} CHARIDENTIFY;

// ==============================================================

// Some defines for printing messages

#define PRINTALL      0
#define PRINTHDR      1
#define PRINTBODY     2
#define PRINTREALBODY 3

// Some defines for SumAttachRequests

#define SARattach    0x01
#define SARrequest   0x02
#define SARboth      0x04

