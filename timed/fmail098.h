
/*
   FMSTRUCT.H

   File structures for FMail 0.98
   Copyright (C) 1993 Folkert J. Wijnstra. All rights reserved.

   All information in this document is subject to change at any time
   without prior notice!

   Strings are NUL terminated arrays of char type.
   Path names are \ terminated.
*/

#ifndef __FMAIL098_H__
#define __FMAIL098_H__

#pragma pack(__push, 1)

/* ********** General structures ********** */

typedef struct
{
   char     programName[46];
   word memRequired;
}
archiverInfo;

typedef char pathType[48];

//typedef struct
//{
//   word zone;
//   word net;
//   word node;
//   word point; } nodeNumType;

//typedef struct
//{
//   nodeNumType nodeNum;
//   word    fakeNet; } nodeFakeType;


/* ********** File header structure ********** */

#define DATATYPE_CF    0x0102
#define DATATYPE_NO    0x0202
#define DATATYPE_AD    0x0401
#define DATATYPE_AE    0x0402

//typedef struct
//{
//   char            versionString[32]; /* Always starts with 'FMail' */
//   word        revNumber;         /* Is now 0x0100 */
//   word        dataType;          /* See #defines above */
//   word        headerSize;
//   long            creationDate;
//   long            lastModified;
//   word        totalRecords;
//   word        recordSize;
//} headerType;


/* ********** FMAIL.CFG ********** */

#define FMMAX_AKAS   11
#define MAX_USERS  16
#define MAX_UPLREQ 16
#define MAX_MATCH  16

#define LOG_INBOUND   0x0001
#define LOG_OUTBOUND  0x0002
#define LOG_PKTINFO   0x0004
#define LOG_XPKTINFO  0x0008
#define LOG_UNEXPPWD  0x0010
#define LOG_SENTRCVD  0x0020
#define LOG_STATS     0x0040
#define LOG_MSGBASE   0x8000
#define LOG_ECHOEXP   0x8000
#define LOG_NETIMP    0x8000
#define LOG_NETEXP    0x8000
#define LOG_WARNINGS  0x8000
#define LOG_ALWAYS    0x8000
#define LOG_DEBUG     0x8000

typedef nodeFakeType akaListType[FMMAX_AKAS];

typedef struct
{
   word useEMS       :  1; /* BIT 0 */
   word checkBreak   :  1; /* BIT 1 */
   word swap         :  1; /* BIT 2 */
   word swapEMS      :  1; /* BIT 3 */
   word swapXMS      :  1; /* BIT 4 */
   word              :  1;
   word monochrome   :  1; /* BIT 6 */
   word commentFFD   :  1; /* BIT 7 */
   word PTAreasBBS   :  1; /* BIT 8 */
   word commentFRA   :  1; /* BIT 9 */
   word              :  1; /* BIT 10 */
   word incBDRRA     :  1; /* BIT 11 */
   word              :  1; /* BIT 12 */
   word              :  2;
   word RA2          :  1; /* BIT 15 */  } genOptionsType;

typedef struct
{
   word removeNetKludges : 1; /* Bit 0 */
   word                  : 1;
   word checkPktDest     : 1; /* Bit 2 */
   word                  : 1;
   word createSema       : 1; /* Bit 4 */
   word                  : 3;
   word dupDetection     : 1; /* Bit 8 */
   word ignoreMSGID      : 1; /* Bit 9 */
   word ARCmail060       : 1; /* Bit 10 */
   word                  : 1; /* Bit 11 */
   word persNetmail      : 1; /* Bit 12 */
   word privateImport    : 1; /* Bit 13 */
   word keepExpNetmail   : 1; /* Bit 14 */
   word killEmptyNetmail : 1; /* Bit 15 */  } mailOptionsType;

typedef struct
{
   word sortNew      : 1; /* bit  0   */
   word sortSubject  : 1; /* bit  1   */
   word updateChains : 1; /* bit  2   */
   word reTear       : 1; /* bit  3   */
   word              : 1; /* bit  4   */
   word              : 1; /* bit  5   */
   word removeRe     : 1; /* bit  6   */
   word removeLfSr   : 1; /* bit  7   */
   word scanAlways   : 1; /* bit  8   */
   word scanUpdate   : 1; /* bit  9   */
   word multiLine    : 1; /* bit 10   */
   word              : 1; /* bit 11   */
   word quickToss    : 1; /* bit 12   */
   word              : 2; /* bit 13-14 */
   word sysopImport  : 1; /* bit 15   */ } mbOptionsType;

typedef struct
{
   word keepRequest  : 1; /* Bit  0 */
   word keepReceipt  : 1; /* Bit  1 */
   word              : 2; /* Bit 2-3 */
   word autoDiscArea : 1; /* Bit  4 */
   word              : 4; /* Bit 5-8 */
   word allowAddAll  : 1; /* Bit  9 */
   word allowActive  : 1; /* Bit 10 */
   word              : 1; /* Bit 11 */
   word allowPassword: 1; /* Bit 12 */
   word allowPktPwd  : 1; /* Bit 13 */
   word allowNotify  : 1; /* Bit 14 */
   word allowCompr   : 1; /* Bit 15 */  } mgrOptionsType;

typedef struct
{
   word addPlusPrefix :  1; /* BIT 0 */
   word               :  3;
   word unconditional :  1; /* BIT 4 */
   word               : 11;    } uplOptType;

typedef struct
{
   char  userName[36];
   char  reserved[28];
} userType;

typedef struct
{
   nodeNumType node;
   char        program[9];
   char        password[17];
   char        fileName[13];
   char        fileType;
   long        groups;
   char        originAka;
   uplOptType  options;
   char        reserved[9];  } uplinkReqType;

typedef struct
{
   word valid;
   word zone;
   word net;
   word node;    } akaMatchNodeType;

typedef struct
{
   akaMatchNodeType amNode;
   word         aka;    } akaMatchType;

typedef struct
{
   char            versionMajor;
   char            versionMinor;
   long            creationDate;
   unsigned long   key;
   unsigned long   reservedKey;
   unsigned long   relKey1;
   unsigned long   relKey2;
   char            reserved1[22];
   mgrOptionsType  mgrOptions;
   akaListType     akaList;
   nodeFakeType    reservedAka[16-FMMAX_AKAS];
   word            netmailBoard[FMMAX_AKAS];
   word            reservedNet[16-FMMAX_AKAS];
   genOptionsType  genOptions;
   mbOptionsType   mbOptions;
   mailOptionsType mailOptions;
   word            maxPktSize;
   word            reserved2;
   word            mailer;
   word            bbsProgram;
   word            maxBundleSize;
   word            extraHandles; /* 0-235 */
   word            autoRenumber;
   word            bufSize;
   word            ftBufSize;
   word            allowedNumNetmail;
   word            logInfo;
   word            logStyle;
   char            reserved3[68];
   word            colorSet;
   char            sysopName[36];
   word            defaultArc;
   char            reserved4[24];
   char            tearType;
   char            tearLine[25];
   pathType        summaryLogName;
   word            recBoard;
   word            badBoard;
   word            dupBoard;
   char            topic1[16];
   char            topic2[16];
   pathType        bbsPath;
   pathType        netPath;
   pathType        sentPath;
   pathType        rcvdPath;
   pathType        inPath;
   pathType        outPath;
   pathType        securePath;
   pathType        logName;
   pathType        swapPath;
   pathType        semaphorePath;
   pathType        pmailPath;
   pathType        areaMgrLogName;
   pathType        autoRAPath;
   pathType        autoFolderFdPath;
   pathType        autoAreasBBSPath;
   pathType        autoGoldEdAreasPath;
   archiverInfo    unArc;
   archiverInfo    unZip;
   archiverInfo    unLzh;
   archiverInfo    unPak;
   archiverInfo    unZoo;
   archiverInfo    unArj;
   archiverInfo    unSqz;
   archiverInfo    GUS;
   archiverInfo    arc;
   archiverInfo    zip;
   archiverInfo    lzh;
   archiverInfo    pak;
   archiverInfo    zoo;
   archiverInfo    arj;
   archiverInfo    sqz;
   archiverInfo    customArc;
   char            reserved5[105];
   char            groupsQBBS[11];
   word            templateSecQBBS[11];
   char            templateFlagsQBBS[11][4];
   char            attr2RA[11];
   char            aliasesQBBS[11];
   word            groupRA[11];
   word            altGroupRA[11][3];
   char            qwkNameSBBS[11][13];
   word            minAgeSBBS[11];
   word            daysRcvdAKA[11];
   char            replyStatSBBS[11];
   word            attrSBBS[11];
   char            groupDescr[26][27];
   char            reserved6[9];
   char            msgKindsRA[11];
   char            attrRA[11];
   word            readSecRA[11];
   char            readFlagsRA[11][4];
   word            writeSecRA[11];
   char            writeFlagsRA[11][4];
   word            sysopSecRA[11];
   char            sysopFlagsRA[11][4];
   word            daysAKA[11];
   word            msgsAKA[11];
   char            descrAKA[11][51];
   userType        users[MAX_USERS];
   akaMatchType    akaMatch[MAX_MATCH];
   char            reserved7[2048-10*MAX_MATCH];
   uplinkReqType   uplinkReq[MAX_UPLREQ];   } configType;



/* ********** FMAIL.AR ********** */

#define FMMAX_AREAS    512
#define MAX_FORWARD   64

#define MB_PATH_LEN_OLD   19
#define MB_PATH_LEN       61
#define ECHONAME_LEN_090  25
#define ECHONAME_LEN      51
#define COMMENT_LEN       51
#define ORGLINE_LEN       59

typedef char areaNameType[ECHONAME_LEN];

//typedef struct
//{
//   word active      : 1; /* Bit  0 */
//   word tinySeenBy  : 1; /* Bit  1 */
//   word security    : 1; /* Bit  2 */
//   word             : 1; /* Bit  3 */
//   word private     : 1; /* Bit  4 */
//   word impSeenBy   : 1; /* Bit  5 */
//   word checkSeenBy : 1; /* Bit  6 */
//   word             : 1; /* Bit  7 */
//   word local       : 1; /* Bit  8 */
//   word disconnected: 1; /* Bit  9 */
//   word reserved    : 1; /* Bit 10 */
//   word             : 3; /* Bit 11-13 */
//   word arrivalDate : 1; /* Bit 14 */
//   word sysopRead   : 1; /* Bit 15 */   } areaOptionsType;

typedef struct      /* OLD!!! */
{
   char            zero; /* Should always be zero */
   char            msgBasePath[MB_PATH_LEN_OLD];
   char            groupsQBBS;
   char            flagsTemplateQBBS[4];
   char            comment[COMMENT_LEN];
   long            group;
   word            board;
   word            address;
   word            alsoSeenBy;
   word            groupRA;
   word            altGroupRA[3];
   areaOptionsType options;
   word            outStatus;
   word            days;
   word            msgs;
   word            daysRcvd;
   word            templateSecQBBS;
   word            readSecRA;
   char            flagsRdRA[4];
   word            writeSecRA;
   char            flagsWrRA[4];
   word            sysopSecRA;
   char            flagsSysRA[4];
   char            attrRA;
   char            msgKindsRA;
   word            attrSBBS;
   char            replyStatSBBS;
   areaNameType    areaName;
   char            qwkName[13];
   word            minAgeSBBS;
   char            attr2RA;
   char            aliasesQBBS;
   char            originLine[ORGLINE_LEN];
   nodeNumType     export[MAX_FORWARD];  } rawEchoTypeOld;

typedef struct
{
   word            signature;
   word            reserved;
   areaNameType    areaName;
   char            comment[COMMENT_LEN];
   areaOptionsType options;
   word            reserved1;
   char            msgBaseType;
   char            msgBasePath[MB_PATH_LEN];
   word            board;
   char            originLine[ORGLINE_LEN];
   word            address;
   long            group;
   word            alsoSeenBy;
   word            msgs;
   word            days;
   word            daysRcvd;

   nodeNumType     export[MAX_FORWARD];

   word            readSecRA;
   char            flagsRdNotRA[4];
   char            flagsRdRA[4];
   word            writeSecRA;
   char            flagsWrNotRA[4];
   char            flagsWrRA[4];
   word            sysopSecRA;
   char            flagsSysRA[4];
   char            flagsSysNotRA[4];
   word            templateSecQBBS;
   char            flagsTemplateQBBS[4];
   char            flagsReserved[4];

   char            attrRA;
   char            attr2RA;
   word            groupRA;
   word            altGroupRA[3];
   char            msgKindsRA;
   char            qwkName[13];
   word            minAgeSBBS;
   word            attrSBBS;
   char            replyStatSBBS;
   char            groupsQBBS;
   char            aliasesQBBS;      } rawEchoType;



/* ********** FMAIL.NOD ********** */

#define MAX_NODEMGR      256
#define PKT_TYPE_2PLUS   1
#define CAPABILITY       PKT_TYPE_2PLUS

typedef struct
{
   word fixDate     : 1; /* Bit 0 */
   word tinySeenBy  : 1; /* Bit 1 */
   word             : 1; /* Bit 2 */
   word ignorePwd   : 1; /* Bit 3 */
   word active      : 1; /* Bit 4 */
   word             : 2;
   word packNetmail : 1; /* Bit 7 */
   word             : 1; /* Bit 8 */
   word             : 3;
   word forwardReq  : 1; /* Bit 12 */
   word remMaint    : 1; /* Bit 13 */
   word allowRescan : 1; /* Bit 14 */
   word notify      : 1;   } nodeOptionsType;

typedef struct /* OLD !!! */
{
   nodeNumType     node;
   char            reserved1[2];
   word        capability;
   word        archiver;
   nodeOptionsType options;
   unsigned long   groups;
   word        outStatus;
   char            reserved2[32];
   char            password[18];
   char            packetPwd[10];
   char            reserved[2];
   nodeNumType     viaNode;
   char            sysopName[36];   } nodeInfoTypeOld;

/* See Area File for header structure */

typedef struct
{
   word        signature; /* contains "ND" */
   word        reserved;
   nodeNumType     node;
   nodeNumType     viaNode;
   word        capability;
   nodeOptionsType options;
   word        archiver;
   unsigned long   groups;
   word        outStatus;
   char            sysopName[36];
   char            password[18];
   char            packetPwd[10]; } nodeInfoType;



/* ********** FMAIL.PCK ********** */

#define PACK_STR_SIZE 64
#define MAX_PACK      64

typedef char packEntryType[PACK_STR_SIZE];
typedef packEntryType packType[MAX_PACK];



/* ********** FMAIL.BDE ********** */

#define MAX_BAD_ECHOS 50

typedef struct
{
   areaNameType badEchoName;
   nodeNumType  srcNode;
   sword          destAka;      } badEchoType;

#pragma pack(__pop)

#endif
