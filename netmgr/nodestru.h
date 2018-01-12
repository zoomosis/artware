
#ifdef __OS2__

   #define word      unsigned short
   #define sword     short
   #define  INCL_NOPMAPI

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


/* For the results of nodelist searches */

//typedef struct _addrlist
//{
//
//   NETADDR           address;
//   char              name[40];
//   char              system[30];
//   char              location[30];
//   char              flags[40];
//   char              phone[20];
//   long              baud;
//   struct _addrlist  *next;
//
//}  ADDRLIST;




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



/* Should timEd ask confirmation if only 1 address found? */

#define NOPROMPT 0
#define PROMPT   1


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


