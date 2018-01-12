
#define VERSION   "1.00"


#ifdef __OS2__

   #define word      unsigned short
   #define sword     short
   #define  INCL_NOPMAPI

#else

   #define word      unsigned int
   #define sword     int

#endif


#define ECHOMAIL  0
#define NETMAIL   1
#define LOCAL     2

#define NEXT         -1
#define PREV         -2
#define ESC          -3
#define REPLY        -4
#define ENTER        -5
#define KILL         -6
#define CHANGE       -7
#define MOVE         -8
#define REPLYOTHER   -9
#define PRINT        -10
#define KLUDGES      -11
#define HOME         -12
#define END          -13
#define UP           -14
#define DOWN         -15
#define TAB          -16
#define BTAB         -17
#define ABORT        -18
#define LIST         -17
#define FIND         -18
#define RESET        -19
#define ACCEPT       -20
#define COPY         -21
#define REPORIG      -22
#define EXIT         -23
#define EDITHELLO    -24
#define SHOWINFO     -25
#define HELP         -26
#define TURBOREP     -27
#define NEXTAREA     -28
#define PREVAREA     -29
#define CHANGEHDR    -30
#define BROADLIST    -31
#define HARDCOPY     -32
#define F2           -33
#define MSGMAINT     -34
#define REQUEST      -35
#define SAVE         -36



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

#define REGISTERED *(cfg.usr.registered) == 1

// ====================================================

typedef struct
{

   XMSG     hdr;           /* Header of msg                            */
   char     *msgtext;      /* Pointer to raw msgtext                   */
   LINE     *txt;          /* Pointer to first line of msg             */
   char     *ctxt;         /* Pointer to control info                  */
   char     *ctext;        /* Pointer to control info, formatted dupe  */
   char     id[80];        /* Holds MSGID (if present)                 */
   NETADDR  rep_addr;      /* FSC35, replyto: kludge, gate's fido addr */
   char     rep_gate[36];  /* FSC35, name in to: field                 */
   char     rep_name[80];  /* FSC35, TO: name in msg body              */
   UMSGID   uid;           /* Holds squish' UMSGID                     */
   UMSGID   replynext;     /* Points to next reply in chain (JAM only) */
   dword    txtsize;       /* Size of msg body                         */
   dword    ctrlsize;      /* Size of kludges                          */
   char     status;        /* See status bits below..                  */

}  MMSG;

/* Status bits for MMSG.status */

#define PERSONAL 0x01
#define USENET   0x02
#define REPLYTO  0x04

/* typedef enum {Yes, NO} YesNo; */

typedef struct arearec
{

   char   *desc;          /* Description of area           */
   char   *tag;           /* Official area tag             */
   char   group;          /* Group of an area              */
   char   *dir;           /* Directory/Base name           */
   word   base;           /* MSGTYPE_SDM or MSGTYPE_SQUISH */
   word   type;           /* ECHOMAIL, NETMAIL or LOCAL    */
   long   stdattr;        /* "Standard" attributes of area */
   long   lr;             /* Lastread pointer              */
   long   nomsgs;         /* Number of msgs in area        */
   long   highest;        /* Highest messagenumber in area */
   long   lowest;         /* Lowest messagenumber in area  */
   char   scanned;        /* Is the area scanned? 0=No     */
   char   tagged;         /* Is this area tagged? 0=No     */
   char   aka;            /* What AKA should be used?      */
   char   newmail;        /* Any new mail entered in area? */
   struct arearec *next;  /* Pointer to next area          */
   struct arearec *prev;  /* Pointer to previous area      */

}  AREA;

typedef struct
{
   char name[40];
   long hash;              /* Hash value of name, for Squish areas */
   unsigned long crc;      /* CRC value of name, for JAM areas     */

} NAME;

typedef struct
{

   NAME     name[10];         /* Name of the user               */
   NETADDR  address[25];      /* Addresses                      */
   char     nodelist[100];    /* Path to V7 nodelist files      */
   char     fdnodelist[100];  /* Path to FD nodelist files      */
   char     fidouser[100];    /* Path and name for fidouser.lst */
   char     origin[100];      /* Default origin to use          */
   char     signoff[221];     /* Default signoff (greetings..)  */
   char     echolog[100];     /* Echotoss.log file              */
   char     editor[100];      /* Name of the editor to be used  */
   char     internal_edit;    /* 1 == use internal editor       */
   char     hudsonpath[120];  /* Path to the Hudson msgbase     */
   char     jamlogpath[120];  /* Path to the Echomail.JAM files */
   char     netsema[120];     /* Semaphore for new netmail      */
   dword    status;           /* Some bits (showkludge etc)     */
   char     delreply;         /* Netmail: del orig after rep. 0=no, 1=yes, 2=ask_confirm */
   char     zonegate;         /* Do zonegate, values above      */
   char     akamatch;         /* Akamatching active? 0 = No     */
   char     emptytear;        /* 1 = empty tearline, use PID    */
   char     echoinfo;         /* Add tear + origin for echomail */
   char     localinfo;        /*                       local    */
   char     arcmail;          /* System running ArcMail Attach? */
   char     lr[20];           /* Name of lastread file          */
   word     sqoff;            /* Offset in Squish .SQL file     */
   char     rephello[201];    /* To start reply with..          */
   char     hello[201];       /* To start msg with..            */
   char     followhello[201]; /* To a followup message with...  */
   char     orighello[201];   /* To start orig-reply with..     */
   char     swap_edit;        /* Swap when spawning editor?     */
   char     swap_shell;       /* Swap when shelling to DOS?     */
   char     writename[100];   /* Default name to write to       */
   char     printer[20];      /* Printerdevice to write to      */
   char     *registered;      /* Pointer to char that = 1 if reg'ed */

} USER;

/* status bits: */

#define   SHOWKLUDGES     0x0001
#define   ALLAREAS        0x0002
#define   TAGGEDAREAS     0x0004
#define   NEWMAILAREAS    0x0008
#define   CONFIRMDELETE   0x0010
#define   CONFIRMEXIT     0x0020
#define   SKIPRECEIVED    0x0040
#define   NETTEAR         0x0080
#define   SHOWNOTES       0x0100
#define   INTLFORCE       0x0200
#define   ENDAREAMENU     0x0400
#define   CONFIRMEDITEXIT 0x0800
#define   SHOWEDITHCR     0x1000
#define   JUMPYEDIT       0x2000
#define   LOWLEVELKB      0x4000


typedef struct _macrolist
{

   char              macro[20];
   char              toname[36];
   NETADDR           toaddress;
   char              subject[72];
   char              usenet[80];
   struct _macrolist *next;

} MACROLIST;


typedef struct
{

   word  asbar;               /* Area selection top bar          */
   word  asframe;             /*                frame            */
   word  astext;              /*                text             */
   word  ashigh;              /*                highlighted bar  */
   word  asspecial;           /*                newmail/personal */

   word  msgheader;           /* Message reading header (name etc) */
   word  msgdate;             /*                 date              */
   word  msgdata;             /*                 from:, to:        */
   word  msglinks;            /*                 reply links       */
   word  msgattribs;          /*                 attributes        */
   word  msgline;             /*                 horiz. line       */
   word  msgtext;             /*                 normal text       */
   word  msgquote;            /*                 quotes            */
   word  msgorigin;           /*                 origin            */
   word  msgkludge;           /*                 kludge            */
   word  msgbar;              /*                 bottom bar        */
   word  msgspecial;          /*                 personal msgs     */

   word  entry;               /* Colour of data entry fields       */

   word  editcurnormal;       /* Current line, normal text         */
   word  editcurquote;        /* Current line, quoted text         */
   word  edithcr;             /* Hard return token                 */
   word  editblock;           /* Block                             */
   word  editcurblock;        /* Current line in block             */

   word  popframe;            /* Popup boxes frame                 */
   word  poptext;             /* Popup boxes text                  */

}  COLORS;


typedef struct
{

   USER         usr;           /*  User data  */
   AREA         *first;
   MACROLIST    *firstmacro;
   COLORS       col;
   char         homedir[80];
   union KEY    key;

}  CFG;

/* Some 'custom' info for a certain area, taken from xxxx.SQT */

typedef struct
{

   char string[26];

} sstr;

typedef struct
{

   char   hello[100];
   char   rephello[100];
   char   followhello[100];
   char   signoff[100];
   char   origin[100];
   word   mode;          /* DIRECTLIST or NORMAL_MODE, mode to enter area */
   word   aka;
   word   name;
   sstr   sargs[10];
   char   allareas;
   char   allmsgs;

} CUSTOM;


/* Defines for the 'mode' parameter area entry */


#define DIRECTLIST     0   /* Enter area in L)ist mode */
#define NORMAL_MODE    1   /* Normal entry of area     */


/* For the results of nodelist searches */

typedef struct _addrlist
{

   NETADDR           address;
   char              name[40];
   char              system[30];
   struct _addrlist  *next;

}  ADDRLIST;


