#define VERSION "1.00"

// #define VERSION "1.00.b8"

// Registration junk

#define NETMGRCODE 0xAEAE5656L

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
   char   rand6[33];

} STRKEY;


typedef struct
{

   char   bytes[128];

} ARRKEY;


union KEY
{
   STRKEY strkey;
   ARRKEY arrkey;
};

#define REGISTERED (cfg.registered == 1)

// ====================================================


/* Structure of a 'mask' that can be matched */

typedef struct
{

   char      fromname[36];
   NETADDR   fromaddress;
   int       fromlisted;  // 0: don't care, 1: must be listed, -1 must NOT be listed
   int       fromnot;     // If 1: must be NOT this address
   char      toname[36];
   NETADDR   toaddress;
   int       tolisted;    // 0: don't care, 1: must be listed, -1 must NOT be listed
   int       tonot;       // If 1: must be NOT this address
   char      subject[72];
   dword     yes_attribs1;
   dword     yes_attribs2;
   dword     no_attribs1;
   dword     no_attribs2;

} MASK;


typedef struct _addrlist
{

   NETADDR            addr;
   int                not;
   struct _addrlist * and;
   struct _addrlist * or;

} ADDRLIST;


typedef struct _namelist
{

   char             * name;
   struct _namelist * and;
   struct _namelist * or;

} NAMELIST;


typedef struct _attrlist
{
   dword              yes_attribs1;
   dword              yes_attribs2;
   dword              no_attribs1;
   dword              no_attribs2;
   struct _attrlist * or;

} ATTRLIST;


typedef struct
{

   char     * xmaskname;
   NAMELIST * fromname;       // List of name requirements, from name.
   NAMELIST * toname;         // List of name requirements, to name.
   NAMELIST * subj;           // List of subj reqs
   NAMELIST * kludge;
   NAMELIST * body;
   // For all lists above: if ->pw is filled in, it holds an ASCII file
   // with a list of elements. If _any_ of the elements match, it is
   // a match (an OR search).
   dword        bodybytes;      // No of bytes to search in body.
   dword        bodylines;      // No of lines to search in body.
   ADDRLIST   * orig;
   ADDRLIST   * dest;
   int          fromlisted;
   int          tolisted;
   ATTRLIST   * attribs;
   dword        olderwritten;
   dword        olderprocessed;
   dword        olderread;

} XMASK;

/* List of masks and action/arguments for that mask */

typedef struct _masklist
{

   XMASK   *xmask;        /* Extended mask. If this != NULL, use it    */
   MASK    mask;          /* Mask to match                             */
   sword   action;        /* What action to take when match found      */
   void    *value;        /* Value, like destdir or rewrite mask       */
   char    *seenby;       /* Optionally points to seen-by text         */
   char    *bodyfile;     /* MakeMsg action: file to make body         */
   char    *destarea;     /* Destination area, if bounce in other area */
   NETADDR origaddr;      /* Origination address for EchoCopy/Move     */
   NETADDR destaddr;      /* Destination address to pack mail to.      */
   char    password[8];   /* Packet Password. NOT NULL TERMINATED!     */
   int     no;            /* Number of this mask for debug output      */
   struct _masklist *next;
   struct _masklist *nextaction;


} MASKLIST;


// List struct to hold defined masks for later use.


typedef struct _xmasklist
{

   XMASK             * xmask;
   struct _xmasklist * next;

} XMASKLIST;


/* Constants for actions */

#define GETNEXT         0    // Get next mask to find corresponding action

#define COPY            1
#define MOVE            2
#define DELETE          3
#define REWRITE         4
#define BOUNCE          5
#define ECHOCOPY        6
#define ECHOMOVE        7
#define WRITE           8
#define UUCP            9
#define SEMA           10
#define HDRFILE        11
#define EMPTYBOUNCE    12
#define HDRBOUNCE      13
#define MAKEMSG        14
#define FORWARD        15
#define CHANGEPATH     16
#define ADDNOTE        17
#define PACKMAIL       18
#define MOVEMAIL       19
#define IGNORE         20
#define DISPLAY        21
#define XBOUNCE        22
#define XHDRBOUNCE     23
#define XEMPTYBOUNCE   24
#define DELETEATTACH   25
#define CHANGEPATHMOVE 26
#define RUNEXTERNAL    27


typedef struct _arealist
{
   char             *dir;
   struct _arealist *next;
   MASKLIST         *firstmask;
   int               renumber;

} AREALIST;

#define tMAXAKAS  35

// -----------

typedef struct _akaforce
{
   NETADDR            mask;   // -9 in a mask means '*'.
   int                aka;
   struct _akaforce * next;

} AKAFORCE;

// -----------


/* configuration struct */

typedef struct
{

   AREALIST  *firstarea;
   char      logfile[100];
   char      origin[80];
   char      nodelist[120];
   char      fdnodelist[120];
   char      gigonodelist[120];
   char      hudsonpath[120];
   char      jamlog[120];
   char      nodebuf[120];
   char      outbound[120];
   char      registered;
   int       bufentries;
   int       useflags;
   int       frodolog;
   int       intlforce;
   NETADDR   homeaddress[tMAXAKAS];
   union KEY key;
   MASKLIST  *firstmask;
   char      registername[36];
   char      homedir[128];
   char      cfgdir[128];
   AKAFORCE  *akaforce;

} CFG;



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

  MSG   *areahandle;
  MSGH  *msghandle;

  MIS   mis;

  int   fromlisted;
  int   tolisted;

  char  *body;
  dword body_len;

  int      bodyread;
  RAWBLOCK *fblk;

  char  *kludges;
  dword kludge_len;


} MESSAGE;


// ======================================

typedef struct
{

   NETADDR addr;
   char    listed;

} CACHE;

// =======================================

typedef struct _macrolist
{

   char                name[20];    // 'Name' of the macro/define.
   int                 num;         // Number of addresses in list.
   NETADDR           * addrlist;    // Pointer to array of addresses.
   struct _macrolist * next;        // Ptr to next macro.

} MACROLIST;

// =========================================

extern char   months_ab[][4];    // From MSGAPI
extern char   weekday_ab[][4];   // From MSGAPI

// ==============================================================

typedef struct
{

   int     CommandMode;
   char    area[120];
   char    file[120];
   char    xmask[40];
   char    password[40];
   NETADDR address;
   char    flavour;
   char    newflavour;
   int     isecho;

} COMMANDINFO;

// Commands for CommandMode variable, to be used when CommandMode != 0

#define POST   1
#define POLL   2
#define CHANGE 3
#define GET    4
#define UPDATE 5
#define SEND   6


#define WRITE_ALL    0
#define WRITE_HEADER 1
#define WRITE_BODY   2


// Proto's for memory allocation functions.

void * mem_malloc(unsigned n);
void * mem_calloc(unsigned n, unsigned t);
void * mem_realloc(void * org, unsigned n);
char * mem_strdup(char *s);
void mem_free(void *p);


