

typedef struct _ADDRESS2
{
   word  Zone;
   word  Net;
   word  Node;
   word  Point;
   char  *Domain;
} *ADDRP;


/*struct _newnode
{
   word NetNumber;
   word NodeNumber;
   word Cost;                                    /* cost to user for a
                                                  * message */
   char SystemName[34];                          /* node name */
   char PhoneNumber[40];                         /* phone number */
   char MiscInfo[30];                            /* city and state */
   char Password[8];                             /* WARNING: not necessarily
                                                  * null-terminated */
   word RealCost;                                /* phone company's charge */
   word HubNode;                                 /* node # of this node's hub
                                                  * or point number if node is a point */
   byte BaudRate;                                /* baud rate divided by 300 */
   byte ModemType;                               /* RESERVED for modem type */
   word NodeFlags;                               /* set of flags (see below) */
   word NodeFiller;
};*/



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
            int     IndxCnt;    /* Count of Items in block     */
            word    IndxStr;    /* Offset in block of 1st str  */
            /* If IndxFirst is NOT -1, this is INode:          */
            struct _IndxRef {
                word   IndxOfs; /* Offset of string into block */
                word   IndxLen; /* Length of string            */
                long   IndxData;/* Record number of string     */
                long   IndxPtr; /* Block number of lower index */
            } IndxRef[20];
        } INodeBlk;

        struct _LNodeBlk {
                                /* IndxFirst is -1 in LNodes   */
            long    IndxFirst;  /* Pointer to next lower level */
            long    IndxBLink;  /* Pointer to previous link    */
            long    IndxFLink;  /* Pointer to next link        */
            int     IndxCnt;    /* Count of Items in block     */
            word    IndxStr;    /* Offset in block of 1st str  */
            struct _LeafRef {
                word   KeyOfs;  /* Offset of string into block */
                word   KeyLen;  /* Length of string            */
                long   KeyVal;  /* Pointer to data block       */
            } LeafRef[30];
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
        int    Zone;
        int    Net;
        int    Node;
        int    HubNode;        /* If node is a point, this is point number. */
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
/*
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
*/

/* prototypes */

int ver7find (ADDRP opus_addr, int have_boss_data);
void opususer (char *name, ADDRP faddr);
