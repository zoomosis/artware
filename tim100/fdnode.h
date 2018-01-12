
/* Proto's, include tstruct.h first */

ADDRLIST *getFDname(char    *name   );
ADDRLIST *getFDnode(NETADDR *address);



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

