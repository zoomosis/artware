#ifndef __FMAIL120_H__
#define __FMAIL120_H__

#pragma pack(__push, 1)

/*
   FMSTRUCT.H

        File structures for FMail 1.20
        Copyright (C) 1996 Folkert J. Wijnstra. All rights reserved.

	All information in this document is subject to change at any time
	without prior notice!

	Strings are NUL terminated arrays of char type.
	Path names always end on a \ character (followed by NUL).
*/


/**** Modify the type definitions below if necessary for your compiler ****/

#define fhandle signed int
#define uchar unsigned char
#define schar char
#ifndef __FLAT__
#define u16 unsigned int
#define s16 signed int
#else
#define u16 short unsigned int
#define s16 short signed int
#endif
#define u32 long unsigned int
#define s32 long signed int
#define udef unsigned int
#define sdef signed int

#define MAX_U32	0xFFFFFFFFL


/* ********** General structures ********** */

typedef struct
{
    uchar programName[46];
    u16 memRequired;
} FM12archiverInfo;

typedef uchar pathType[48];

typedef struct
{
    u16 zone;
    u16 net;
    u16 node;
    u16 point;
} nodeNumType;

typedef struct
{
    nodeNumType nodeNum;
    u16 fakeNet;
} nodeFakeType;


/* ********** File header structure ********** */

#define DATATYPE_CF    0x0102   /* not used yet */
#define DATATYPE_NO    0x0202   /* node file */
#define DATATYPE_AD    0x0401   /* area file for echo mail defaults */
#define DATATYPE_AE    0x0402   /* area file for echo mail */

typedef struct
{
    uchar versionString[32];    /* Always starts with 'FMail' */
    u16 revNumber;              /* Is now 0x0100 */
    u16 dataType;               /* See #defines above */
    u16 headerSize;
    s32 creationDate;
    s32 lastModified;
    u16 totalRecords;
    u16 recordSize;
} headerType;


/* The structure below is used by the Areas File and (only partly)
   by the Config File */

typedef struct
{
    word active:1;              /* Bit 0 */
    word tinySeenBy:1;          /* Bit 1 */
    word security:1;            /* Bit 2 */
     word:1;                    /* Bit 3 */
    word allowPrivate:1;        /* Bit 4 */
    word impSeenBy:1;           /* Bit 5 */
    word checkSeenBy:1;         /* Bit 6 */
     word:1;                    /* Bit 7 */
    word local:1;               /* Bit 8 */
    word disconnected:1;        /* Bit 9 */
    word _reserved:1;           /* Bit 10 */
    word allowAreafix:1;        /* Bit 11 */
     word:2;                    /* Bit 12-13 */
    word arrivalDate:1;         /* Bit 14 */
    word sysopRead:1;           /* Bit 15 */
} areaOptionsType;


/* ********** FMAIL.CFG ********** */

#define FM12MAX_AKAS      32
#define FM12MAX_AKAS_F    64
#define FM12MAX_AKAS_OLD  16
#define MAX_NA_OLD    11
#define MAX_NETAKAS   32
#define MAX_NETAKAS_F 64
#define MAX_USERS     16
#define FM12MAX_UPLREQ    32
#define MAX_MATCH     16        // not used yet

//#define LOG_NEVER     0x0000
//#define LOG_INBOUND   0x0001
//#define LOG_OUTBOUND  0x0002
//#define LOG_PKTINFO   0x0004
//#define LOG_XPKTINFO  0x0008
//#define LOG_UNEXPPWD  0x0010
//#define LOG_SENTRCVD  0x0020
//#define LOG_STATS     0x0040
//#define LOG_PACK      0x0080
//#define LOG_MSGBASE   0x0100
//#define LOG_ECHOEXP   0x0200
//#define LOG_NETIMP    0x0400
//#define LOG_NETEXP    0x0800
//#define LOG_OPENERR   0x1000
//#define LOG_EXEC      0x2000
//#define LOG_NOSCRN    0x4000
//#define LOG_ALWAYS    0x8000
//#define LOG_DEBUG     0x8000

typedef nodeFakeType _OLDFMAILakaListType[16];
typedef nodeFakeType FM12akaListType[FM12MAX_AKAS_F];

typedef struct
{
    word useEMS:1;              /* BIT 0 */
    word checkBreak:1;          /* BIT 1 */
    word swap:1;                /* BIT 2 */
    word swapEMS:1;             /* BIT 3 */
    word swapXMS:1;             /* BIT 4 */
     word:1;
    word monochrome:1;          /* BIT 6 */
    word commentFFD:1;          /* BIT 7 */
    word PTAreasBBS:1;          /* BIT 8 */
    word commentFRA:1;          /* BIT 9 */
     word:1;                    /* BIT 10 */
    word incBDRRA:1;            /* BIT 11 */
     word:1;                    /* BIT 12 */
     word:2;
    word _RA2:1;                /* BIT 15 */
} FM12genOptionsType;

typedef struct
{
    word removeNetKludges:1;    /* Bit 0 */
    word addPointToPath:1;      /* Bit 1 */
    word checkPktDest:1;        /* Bit 2 */
    word neverARC060:1;         /* Bit 3 */
    word createSema:1;          /* Bit 4 */
    word dailyMail:1;           /* Bit 5 */
    word warnNewMail:1;         /* bit 6 */
    word killBadFAtt:1;         /* Bit 7 */
    word dupDetection:1;        /* Bit 8 */
    word ignoreMSGID:1;         /* Bit 9 */
    word ARCmail060:1;          /* Bit 10 */
    word extNames:1;            /* Bit 11 */
    word persNetmail:1;         /* Bit 12 */
    word privateImport:1;       /* Bit 13 */
    word keepExpNetmail:1;      /* Bit 14 */
    word killEmptyNetmail:1;    /* Bit 15 */
} FM12mailOptionsType;


typedef struct
{
    word sortNew:1;             /* bit 0 */
    word sortSubject:1;         /* bit 1 */
    word updateChains:1;        /* bit 2 */
    word reTear:1;              /* bit 3 */
     word:1;                    /* bit 4 */
     word:1;                    /* bit 5 */
    word removeRe:1;            /* bit 6 */
    word removeLfSr:1;          /* bit 7 */
    word scanAlways:1;          /* bit 8 */
    word scanUpdate:1;          /* bit 9 */
    word multiLine:1;           /* bit 10 */
     word:1;                    /* bit 11 */
    word quickToss:1;           /* bit 12 */
     word:1;                    /* bit 13 */
     word:1;                    /* bit 14 */
    word sysopImport:1;         /* bit 15 */
} FM12mbOptionsType;

typedef struct
{
    word keepRequest:1;         /* Bit 0 */
    word keepReceipt:1;         /* Bit 1 */
     word:2;                    /* Bit 2-3 */
    word autoDiscArea:1;        /* Bit 4 */
    word autoDiscDel:1;         /* Bit 5 has temp. no effect, rec is
                                   always deleted */
     word:3;                    /* Bit 6-8 */
    word allowAddAll:1;         /* Bit 9 */
    word allowActive:1;         /* Bit 10 */
     word:1;                    /* Bit 11 */
    word allowPassword:1;       /* Bit 12 */
    word allowPktPwd:1;         /* Bit 13 */
    word allowNotify:1;         /* Bit 14 */
    word allowCompr:1;          /* Bit 15 */
} FM12mgrOptionsType;


#if 0
typedef struct
{
    word active:1;              /* Bit 0 */
    word tinySeenBy:1;          /* Bit 1 */
    word security:1;            /* Bit 2 */
     word:1;                    /* Bit 3 */
    word allowPrivate:1;        /* Bit 4 */
    word impSeenBy:1;           /* Bit 5 */
    word checkSeenBy:1;         /* Bit 6 */
     word:1;                    /* Bit 7 */
    word local:1;               /* Bit 8 */
     word:1;                    /* Bit 9 */
    word passThrough:1;         /* Bit 10 */
     word:3;                    /* Bit 11-13 */
    word arrivalDate:1;         /* Bit 14 */
    word sysopRead:1;           /* Bit 15 */
} defaultOptionsType;
#endif

typedef struct
{
    word addPlusPrefix:1;       /* BIT 0 */
     word:3;
    word unconditional:1;       /* BIT 4 */
     word:11;
} FM12uplOptType;

typedef struct
{
    uchar userName[36];
    uchar reserved[28];
} FM12userType;

typedef struct
{
    nodeNumType node;
    uchar program[9];
    uchar password[17];
    uchar fileName[13];
    uchar fileType;
    u32 groups;
    uchar originAka;
    FM12uplOptType options;
    uchar reserved[9];
} FM12uplinkReqType;

typedef struct
{
    u16 valid;
    u16 zone;
    u16 net;
    u16 node;
} FM12akaMatchNodeType;

typedef struct
{
    FM12akaMatchNodeType amNode;
    u16 aka;
} FM12akaMatchType;

/* ATTENTION: FMAIL.CFG does NOT use the new config file type yet (no header) !!! */

typedef struct
{
    uchar versionMajor;
    uchar versionMinor;
    s32 creationDate;
    u32 key;
    u32 reservedKey;
    u32 relKey1;
    u32 relKey2;
    uchar reserved1[22];
    FM12mgrOptionsType mgrOptions;
    _OLDFMAILakaListType _akaList;
    u16 _netmailBoard[MAX_NA_OLD];
    u16 _reservedNet[16 - MAX_NA_OLD];
    FM12genOptionsType genOptions;
    FM12mbOptionsType mbOptions;
    FM12mailOptionsType mailOptions;
    u16 maxPktSize;
    u16 kDupRecs;
    u16 mailer;
    u16 bbsProgram;
    u16 maxBundleSize;
    u16 extraHandles;           /* 0-235 */
    u16 autoRenumber;
    u16 bufSize;
    u16 ftBufSize;
    u16 allowedNumNetmail;
    u16 logInfo;
    u16 logStyle;
    uchar reserved2[68];
    u16 colorSet;
    uchar sysopName[36];
    u16 defaultArc;
    u16 _adiscDaysNode;
    u16 _adiscDaysPoint;
    u16 _adiscSizeNode;
    u16 _adiscSizePoint;
    uchar reserved3[16];
    uchar tearType;
    uchar tearLine[25];
    pathType summaryLogName;
    u16 recBoard;
    u16 badBoard;
    u16 dupBoard;
    uchar topic1[16];
    uchar topic2[16];
    pathType bbsPath;
    pathType netPath;
    pathType sentPath;
    pathType rcvdPath;
    pathType inPath;
    pathType outPath;
    pathType securePath;
    pathType logName;
    pathType swapPath;
    pathType semaphorePath;
    pathType pmailPath;
    pathType areaMgrLogName;
    pathType autoRAPath;
    pathType autoFolderFdPath;
    pathType autoAreasBBSPath;
    pathType autoGoldEdAreasPath;
    FM12archiverInfo unArc;
    FM12archiverInfo unZip;
    FM12archiverInfo unLzh;
    FM12archiverInfo unPak;
    FM12archiverInfo unZoo;
    FM12archiverInfo unArj;
    FM12archiverInfo unSqz;
    FM12archiverInfo GUS;
    FM12archiverInfo arc;
    FM12archiverInfo zip;
    FM12archiverInfo lzh;
    FM12archiverInfo pak;
    FM12archiverInfo zoo;
    FM12archiverInfo arj;
    FM12archiverInfo sqz;
    FM12archiverInfo customArc;
    pathType autoFMail102Path;
    uchar reserved4[35];
    areaOptionsType _optionsAKA[MAX_NA_OLD];
    uchar _groupsQBBS[MAX_NA_OLD];
    u16 _templateSecQBBS[MAX_NA_OLD];
    uchar _templateFlagsQBBS[MAX_NA_OLD][4];
    uchar _attr2RA[MAX_NA_OLD];
    uchar _aliasesQBBS[MAX_NA_OLD];
    u16 _groupRA[MAX_NA_OLD];
    u16 _altGroupRA[MAX_NA_OLD][3];
    uchar _qwkName[MAX_NA_OLD][13];
    u16 _minAgeSBBS[MAX_NA_OLD];
    u16 _daysRcvdAKA[MAX_NA_OLD];
    uchar _replyStatSBBS[MAX_NA_OLD];
    u16 _attrSBBS[MAX_NA_OLD];
    uchar groupDescr[26][27];
    uchar reserved5[9];
    uchar _msgKindsRA[MAX_NA_OLD];
    uchar _attrRA[MAX_NA_OLD];
    u16 _readSecRA[MAX_NA_OLD];
    uchar _readFlagsRA[MAX_NA_OLD][4];
    u16 _writeSecRA[MAX_NA_OLD];
    uchar _writeFlagsRA[MAX_NA_OLD][4];
    u16 _sysopSecRA[MAX_NA_OLD];
    uchar _sysopFlagsRA[MAX_NA_OLD][4];
    u16 _daysAKA[MAX_NA_OLD];
    u16 _msgsAKA[MAX_NA_OLD];
    uchar _descrAKA[MAX_NA_OLD][51];
    FM12userType users[MAX_USERS];
    FM12akaMatchType akaMatch[MAX_MATCH]; // not used yet
    uchar reserved6[1040 - 10 * MAX_MATCH];
    pathType sentEchoPath;
    FM12archiverInfo preUnarc;
    FM12archiverInfo postUnarc;
    FM12archiverInfo preArc;
    FM12archiverInfo postArc;
    FM12archiverInfo unUc2;
    FM12archiverInfo unRar;
    FM12archiverInfo resUnpack[6];
    FM12archiverInfo uc2;
    FM12archiverInfo rar;
    FM12archiverInfo resPack[6];
    FM12uplinkReqType uplinkReq[FM12MAX_UPLREQ + 32];
    FM12archiverInfo unArc32;
    FM12archiverInfo unZip32;
    FM12archiverInfo unLzh32;
    FM12archiverInfo unPak32;
    FM12archiverInfo unZoo32;
    FM12archiverInfo unArj32;
    FM12archiverInfo unSqz32;
    FM12archiverInfo unUc232;
    FM12archiverInfo unRar32;
    FM12archiverInfo GUS32;
    FM12archiverInfo resUnpack32[6];
    FM12archiverInfo preUnarc32;
    FM12archiverInfo postUnarc32;
    FM12archiverInfo arc32;
    FM12archiverInfo zip32;
    FM12archiverInfo lzh32;
    FM12archiverInfo pak32;
    FM12archiverInfo zoo32;
    FM12archiverInfo arj32;
    FM12archiverInfo sqz32;
    FM12archiverInfo uc232;
    FM12archiverInfo rar32;
    FM12archiverInfo customArc32;
    FM12archiverInfo resPack32[6];
    FM12archiverInfo preArc32;
    FM12archiverInfo postArc32;
    uchar descrAKA[MAX_NETAKAS][51];
    uchar qwkName[MAX_NETAKAS][13];
    areaOptionsType optionsAKA[MAX_NETAKAS];
    uchar msgKindsRA[MAX_NETAKAS];
    u16 daysAKA[MAX_NETAKAS];
    u16 msgsAKA[MAX_NETAKAS];
    uchar groupsQBBS[MAX_NETAKAS];
    uchar attrRA[MAX_NETAKAS];
    uchar attr2RA[MAX_NETAKAS];
    u16 attrSBBS[MAX_NETAKAS];
    uchar aliasesQBBS[MAX_NETAKAS];
    u16 groupRA[MAX_NETAKAS];
    u16 altGroupRA[MAX_NETAKAS][3];
    u16 minAgeSBBS[MAX_NETAKAS];
    u16 daysRcvdAKA[MAX_NETAKAS];
    uchar replyStatSBBS[MAX_NETAKAS];
    u16 readSecRA[MAX_NETAKAS];
    uchar readFlagsRA[MAX_NETAKAS][8];
    u16 writeSecRA[MAX_NETAKAS];
    uchar writeFlagsRA[MAX_NETAKAS][8];
    u16 sysopSecRA[MAX_NETAKAS];
    uchar sysopFlagsRA[MAX_NETAKAS][8];
    u16 templateSecQBBS[MAX_NETAKAS];
    uchar templateFlagsQBBS[MAX_NETAKAS][8];
    uchar reserved7[512];
    u16 netmailBoard[MAX_NETAKAS_F];
    FM12akaListType akaList;
} FM12configType;



/* ********** FMAIL.AR ********** */

#if defined(__FMAILX__) || defined(__32BIT__)
#define FM12MAX_AREAS   4096
#else
#define FM12MAX_AREAS    512
#endif
#define MAX_FORWARD   64

#define MB_PATH_LEN_OLD   19
#define MB_PATH_LEN       61
#define ECHONAME_LEN_090  25
#define ECHONAME_LEN      51
#define COMMENT_LEN       51
#define ORGLINE_LEN       59

typedef uchar areaNameType[ECHONAME_LEN];

/* See Area File for file header structure !!! */

typedef struct
{
    word tossedTo:1;            /* BIT 0 */
     word:15;                   /* BIT 1-15 */
} areaStatType;

typedef struct
{
    u16 signature;              /* contains "AE" for echo areas in
                                   FMAIL.AR and */
    /* "AD" for default settings in FMAIL.ARD */
    u16 writeLevel;
    areaNameType areaName;
    uchar comment[COMMENT_LEN];
    areaOptionsType options;
    u16 boardNumRA;
    uchar msgBaseType;
    uchar msgBasePath[MB_PATH_LEN];
    u16 board;
    uchar originLine[ORGLINE_LEN];
    u16 address;
    u32 group;
    u16 _alsoSeenBy;            /* obsolete: see the 32-bit alsoSeenBy
                                   below */
    u16 msgs;
    u16 days;
    u16 daysRcvd;

    nodeNumType export[MAX_FORWARD];

    u16 readSecRA;
    uchar flagsRdRA[4];
    uchar flagsRdNotRA[4];
    u16 writeSecRA;
    uchar flagsWrRA[4];
    uchar flagsWrNotRA[4];
    u16 sysopSecRA;
    uchar flagsSysRA[4];
    uchar flagsSysNotRA[4];
    u16 templateSecQBBS;
    uchar flagsTemplateQBBS[4];
    uchar _internalUse;
    u16 netReplyBoardRA;
    uchar boardTypeRA;
    uchar attrRA;
    uchar attr2RA;
    u16 groupRA;
    u16 altGroupRA[3];
    uchar msgKindsRA;
    uchar qwkName[13];
    u16 minAgeSBBS;
    u16 attrSBBS;
    uchar replyStatSBBS;
    uchar groupsQBBS;
    uchar aliasesQBBS;
    u32 lastMsgTossDat;
    u32 lastMsgScanDat;
    u32 alsoSeenBy;
    areaStatType stat;
    uchar reserved[180];
} FM12rawEchoType;

#pragma pack(__pop)

#endif
