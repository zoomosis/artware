#ifndef __FE142_H__
#define __FE142_H__

#pragma pack(__push, 1)

/********************************************************/
/* 'C' Structures of FastEcho 1.42�, File: FASTECHO.CFG */
/* (c)1994 by Tobias Burchhardt, Updated: 27 Nov 1994   */
/********************************************************/

/********************************************************/
/* FASTECHO.CFG = <CONFIG>                              */
/*                + <optional extensions>               */
/*                + <CONFIG.NodeCnt * Node>             */
/*                + <CONFIG.AreaCnt * Area>             */
/********************************************************/

// #define REVISION        6       /* current revision     */

/* Note: there is a major change in this revision - the
     Node records have no longer a fixed length !       */

//#define MAX_AREAS       2048    /* max # of areas       */
//#define MAX_NODES       1024    /* max # of nodes       */
//#define MAX_GROUPS      26      /* max # of groups      */
//#define MAX_AKAS        32      /* max # of akas        */
#define MAX_ROUTE       15      /* max # of 'vias' */
//#define MAX_ORIGINS     20      /* max # of origins     */

/*
  Note: The MAX_AREAS and MAX_NODES are only the absolute maximums
        as the handling is flexible. To get the maximums which are
        used for the config file you read, you have to examine the
        CONFIG.MaxAreas and CONFIG.MaxNodes variables !

  Note: The MAX_AREAS and MAX_NODES maximums are subject to change
        with any new version, therefore - if possible - make hand-
        ling as flexible  possible  and  use  CONFIG.MaxAreas  and
        .MaxNodes whereever possible. But be aware that you  might
        (under normal DOS and depending on the way you handle it)
        hit the 64kB segment limit pretty quickly!

        Same goes for the # of AKAs and Groups - use  the  values
        found in CONFIG.AkaCnt and CONFIG.GroupCnt!
*/

/********************************************************/
/* CONFIG.BBSSoftware                                   */
/********************************************************/
//enum BBSSoft { NoBBSSoft = 0, RemoteAccess111, QuickBBS,
//          SuperBBS, ProBoard122 /* Unused */, TagBBS,
//          RemoteAccess200, ProBoard130 /* Unused */,
//          ProBoard200, ProBoard202 };

/********************************************************/
/* CONFIG.CC.what                                       */
/********************************************************/
#define CC_FROM             1
#define CC_TO               2
#define CC_SUBJECT          3

/********************************************************/
/* CONFIG.QuietLevel                                    */
/********************************************************/
#define QUIET_PACK      0x0001
#define QUIET_UNPACK	0x0002
#define QUIET_EXTERN    0x0004

/********************************************************/
/* CONFIG.Buffers                                       */
/********************************************************/
#define BUF_LARGE       0x0000
#define BUF_MEDIUM      0x0001
#define BUF_SMALL       0x0002

/********************************************************/
/* CONFIG.arcext.inb/outb                               */
/********************************************************/
//enum ARCmailExt { ARCDigits = 0, ARCHex, ARCAlpha };

/********************************************************/
/* CONFIG.AreaFixFlags                                  */
/********************************************************/

/********************************************************/
/* Area.board (1-200 = Hudson)                          */
/********************************************************/
#define NO_BOARD        0x4000u /* JAM/Sq/Passthru etc. */
#define AREA_DELETED    0x8000u /* usually never written */

/********************************************************/
/* Area.flags.storage                                   */
/********************************************************/
#define FENEW_QBBS      0
#define FENEW_FIDO      1
#define FENEW_SQUISH    2
#define FENEW_JAM       3
#define FENEW_PASSTHRU  7

/********************************************************/
/* Area.flags.atype                                     */
/********************************************************/
#define FENEW_AREA_ECHOMAIL        0
#define FENEW_AREA_NETMAIL         1
#define FENEW_AREA_LOCAL           2
#define FENEW_AREA_BADMAILBOARD    3
#define FENEW_AREA_DUPEBOARD       4

/********************************************************/
/* Types and other definitions                          */
/********************************************************/

//enum ARCers { ARC_Unknown = -1, ARC_SeaArc, ARC_PkArc, ARC_Pak,
//         ARC_ArcPlus, ARC_Zoo, ARC_PkZip, ARC_Lha, ARC_Arj,
//         ARC_Sqz, ARC_RAR, ARC_UC2 }; /* for Unpackers */

//enum NetmailStatus { NetNormal = 0, NetHold, NetCrash /*, NetImm */ };

//enum AreaFixType { NoAreaFix = 0, NormalAreaFix, FSC57AreaFix };
//enum AreaFixSendTo { AreaFix = 0, AreaMgr, AreaLink, EchoMgr };

/********************************************************/
/* Structures                                           */
/********************************************************/

typedef struct
{
    word zone, net, node, point;
} FENEW_Address;

#define FENEW_MAXPATH 56

typedef struct FENEW_CONFIGURATION
{
    word revision;
    dword flags;
    word NodeCnt, AreaCnt, unused1;
    char NetMPath[FENEW_MAXPATH],
        MsgBase[FENEW_MAXPATH],
        InBound[FENEW_MAXPATH],
        OutBound[FENEW_MAXPATH],
        Unpacker[FENEW_MAXPATH],
        LogFile[FENEW_MAXPATH],
        unused2[448],
        StatFile[FENEW_MAXPATH],
        SwapPath[FENEW_MAXPATH],
        SemaphorePath[FENEW_MAXPATH],
        BBSConfigPath[FENEW_MAXPATH],
        DBQueuePath[FENEW_MAXPATH],
        unused3[32],
        RetearTo[40],
        SecurePath[FENEW_MAXPATH],
        ExtAfter[FENEW_MAXPATH - 4], ExtBefore[FENEW_MAXPATH - 4];
    byte unused4[480];
    struct
    {
        byte what;
        char object[31];
        word conference;
    } CC[10];
    byte security, loglevel;
    word def_days, def_messages;
    byte unused5[462];
    word autorenum;
    word def_recvdays;
    word openQQQs;
    word compressafter;
    word afixmaxmsglen;
    word compressfree;
    char TempPath[FENEW_MAXPATH];
    byte graphics, BBSSoftware;
    char AreaFixHelp[FENEW_MAXPATH];
    byte unused6[504];
    word AreaFixFlags;
    byte QuietLevel, Buffers;
    byte FWACnt, GDCnt;         /* # of ForwardAreaFix records, # of Group 
                                   Default records */
    struct
    {
        word flags;
        word days[2];
        word msgs[2];
    } rescan_def;
    dword duperecords;
    struct
    {
        byte inb;
        byte outb;
    } arcext;
    word AFixRcptLen;
    word AkaCnt;                /* # of Aka records stored */
    word maxPKT;
    byte sharing, sorting;
    struct
    {
        char name[36];
        dword resv;
    } sysops[11];
    char AreaFixLog[FENEW_MAXPATH];
    char TempInBound[FENEW_MAXPATH];
    word maxPKTmsgs;
    word RouteCnt;              /* # of PackRoute records */
    word maxPACKratio;
    byte PackerCnt, UnpackerCnt; /* # of Packers and Unpackers records */
    byte GroupCnt, OriginCnt;   /* # of GroupNames and Origin records */
    word mailer;
    char resv[810];
    word AreaRecSize, GrpDefRecSize; /* Size of Area and GroupDefaults
                                        records stored in this file */
    word MaxAreas, MaxNodes;    /* Current max values for this config */
    word NodeRecSize;           /* Size of each stored Node record */
    dword offset;               /* This is the offset from the current
                                   file-pointer to the 1st Node */
} FENEW_CONFIG;

/* To directly access the 'Nodes' and/or 'Areas' while bypassing the */
/* Extensions, perform an absolute (from beginning of file) seek to  */
/*                   sizeof(CONFIG) + CONFIG.offset                  */
/* If you want to access the 'Areas', you have to add the following  */
/* value to the above formula:  CONFIG.NodeCnt * CONFIG.NodeRecSize  */

typedef struct
{
    FENEW_Address addr;         /* Main address */
    FENEW_Address arcdest;      /* ARCmail fileattach address */
    byte aka, autopassive, newgroup, resv1;
    struct
    {
        word passive:1;
        word dddd:1;            /* Type 2+/4D */
        word arcmail060:1;
        word tosscan:1;
        word umlautnet:1;
        word exportbyname:1;
        word allowareacreate:1;
        word disablerescan:1;
        word arc_status:2;      /* NetmailStatus for ARCmail attaches */
        word arc_direct:1;      /* Direct flag for ARCmail attaches */
        word noattach:1;        /* don't create a ARCmail file attach */
        word mgr_status:2;      /* NetMailStatus for AreaFix receipts */
        word mgr_direct:1;      /* Direct flag for ...  */
        word not_help:1;
        word not_notify:1;
        word packer:4;          /* # of Packer used, 0xf = send .PKT */
        word packpriority:1;    /* system has priority packing ARCmail */
        word resv:2;
    } flags;
    struct
    {
        word type:2;            /* Type of AreaFix: None (human), Normal
                                   or Advanced (FSC-57) */
        word noforward:1;       /* Don't forward AFix requests */
        word allowremote:1;
        word allowdelete:1;     /* flags for different FSC-57 requests */
        word allowrename:1;     /* all 3 reserved for future use */
        word binarylist:1;
        word addplus:1;         /* add '+' when requesting new area */
        word addtear:1;         /* add tearline to the end of requests */
        word sendto:3;          /* name of this systems's AreaFix robot */
        word resv:4;
    } afixflags;
    word resv2;
    char password[9];           /* .PKT password */
    char areafixpw[9];          /* AreaFix password */
    word sec_level;
    dword groups;               /* Bit-field, Byte 0/Bit 7 = 'A' etc.  */
    /* FALSE means group is active */
    dword resv3[2];
    char name[36];              /* Name of sysop */
    byte areas[1];              /* Bit-field with CONFIG.MaxAreas / 8
                                   bits, Byte 0/Bit 7 is conference #0 */
} FENEW_Node;                   /* Total size of each record is stored in
                                   CONFIG.NodeRecSize */

typedef struct
{
    char name[52];
    word board;                 /* 1-200 Hudson, others reserved/special */
    word conference;            /* 0 ... CONFIG.MaxAreas-1 */
    word read_sec, write_sec;
    struct
    {
        word aka:8;             /* 0 ... CONFIG.AkaCnt */
        word group:8;           /* 0 ... CONFIG.GroupCnt */
    } info;
    struct
    {
        word storage:4;
        word atype:4;
        word origin:5;          /* # of origin line */
        word resv:3;
    } flags;
    struct
    {
        word autoadded:1;
        word tinyseen:1;
        word cpd:1;
        word passive:1;
        word keepseen:1;
        word mandatory:1;
        word keepsysop:1;
        word killread:1;
        word disablepsv:1;
        word keepmails:1;
        word hide:1;
        word nomanual:1;
        word umlaut:1;
        word resv:3;
    } advflags;
    word resv1;
    dword seenbys;              /* LSB = Aka0, MSB = Aka31 */
    dword resv2;
    short days;
    short messages;
    short recvdays;
    char path[FENEW_MAXPATH];
    char desc[52];
} FENEW_Area;

/********************************************************/
/* Optional Extensions                                  */
/********************************************************/
/* These are the variable length extensions between     */
/* CONFIG and the first Node record. Each extension has */
/* a header which contains the info about the type and  */
/* the length of the extension. You can read the fields */
/* using the following algorithm:                       */
/*                                                      */
/* offset := 0;                                         */
/* while (offset<CONFIG.offset) do                      */
/*  read_header;                                        */
/*  if(header.type==EH_abc) then                        */
/*   read_and_process_data;                             */
/*    else                                              */
/*  if(header.type==EH_xyz) then                        */
/*   read_and_process_data;                             */
/*    else                                              */
/*   [...]                                              */
/*    else  // unknown or unwanted extension found      */
/*  seek_forward(header.offset); // Seek to next header */
/*  offset = offset + header.offset;                    */
/* end;                                                 */
/********************************************************/

typedef struct
{
    word type;                  /* EH_...  */
    dword offset;               /* length of field excluding header */
} FENEW_ExtensionHeader;


#define EH_AREAFIX      0x0001  /* CONFIG.FWACnt * <ForwardAreaFix> */

//enum AreaFixAreaListFormat { Areas_BBS = 0, Area_List };
//typedef struct
//{
// word nodenr;
// struct
// {
//  word newgroup : 8;
//  word active   : 1;
//  word valid    : 1;
//  word uncond   : 1;
//  word format   : 3;
//  word resv : 2;
// } flags;
// char file[FENEW_MAXPATH];
// word sec_level;
// word resv1;
// dword groups;
// char resv2[4];
//} ForwardAreaFix;

//#define EH_GROUPS    0x000C /* CONFIG.GroupCnt * <GroupNames> */

//typedef struct
//{
// char name[36];
//} GroupNames;
//
//#define EH_GRPDEFAULTS  0x0006  /* CONFIG.GDCnt * <GroupDefaults> */
//            /* Size of each full GroupDefault
//               record is CONFIG.GrpDefResSize */
//typedef struct
//{
// byte group;
// b/yte resv[15];
// Area area;
// byte nodes[1];         /* variable, c.MaxNodes / 8 bytes */
//} GroupDefaults;

#define EH_AKAS         0x0007  /* CONFIG.AkaCnt * <SysAddress> */

typedef struct
{
    FENEW_Address main;
    char domain[28];
    word pointnet;
    dword flags;                /* unused */
} FENEW_SysAddress;

#define EH_ORIGINS      0x0008  /* CONFIG.OriginCnt * <OriginLines> */

typedef struct
{
    char line[62];
} OriginLines;

//#define EH_PACKROUTE    0x0009  /* CONFIG.RouteCnt * <PackRoute> */
//
//typedef struct
//{
// FENEW_Address dest;
// FENEW_Address routes[MAX_ROUTE];
//} PackRoute;

#define EH_PACKERS 	0x000A      /* CONFIG.Packers * <Packers> */

typedef struct
{
    char tag[6];
    char command[FENEW_MAXPATH];
    char list[4];
    dword resv[2];
} Packers;

#define EH_UNPACKERS	0x000B     /* CONFIG.Unpackers * <Unpackers> */

typedef struct
{
    char command[FENEW_MAXPATH];
    dword resv[2];
} Unpackers;

#define EH_RA111_MSG    0x0100  /* Original records of BBS systems */
#define EH_QBBS_MSG     0x0101
#define EH_SBBS_MSG     0x0102
#define EH_TAG_MSG     	0x0104
#define EH_RA200_MSG    0x0105
#define EH_PB200_MSG    0x0106  /* See BBS package's documentation */
#define EH_PB202_MSG    0x0107  /* for details */

/********************************************************/
/* Routines to access Node.areas, Node.groups           */
/********************************************************/

#if 0

word AddBam(byte * bam, word nr)
{
    byte c = (1 << (7 - (nr & 7))), d;

    d = bam[nr / 8] & c;
    bam[nr / 8] |= c;
    return (d);
}

void FreeBam(byte * bam, word nr)
{
    bam[nr / 8] &= ~(1 << (7 - (nr & 7)));
}

word GetBam(byte * bam, word nr)
{
    if (bam[nr / 8] & (1 << (7 - (nr & 7))))
        return (TRUE);
    return (FALSE);
}

#define IsActive(nr,area)      GetBam(Node[nr].areas,area)
#define SetActive(nr,area)     AddBam(Node[nr].areas,area)
#define SetDeActive(nr,area)   FreeBam(Node[nr].areas,area)

#endif

#pragma pack(__pop)

#endif
