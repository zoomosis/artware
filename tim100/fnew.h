
#define MAX_AREAS    512
#define MAX_FORWARD   64

#define MB_PATH_LEN_OLD   19
#define MB_PATH_LEN       61
#define ECHONAME_LEN_090  25
#define ECHONAME_LEN      51
#define COMMENT_LEN       51
#define ORGLINE_LEN       59

typedef char areaNameType[ECHONAME_LEN];

typedef struct
{
   unsigned active      : 1; /* Bit  0 */
   unsigned tinySeenBy  : 1; /* Bit  1 */
   unsigned security    : 1; /* Bit  2 */
   unsigned             : 1; /* Bit  3 */
   unsigned private     : 1; /* Bit  4 */
   unsigned impSeenBy   : 1; /* Bit  5 */
   unsigned checkSeenBy : 1; /* Bit  6 */
   unsigned             : 1; /* Bit  7 */
   unsigned local       : 1; /* Bit  8 */
   unsigned disconnected: 1; /* Bit  9 */
   unsigned reserved    : 1; /* Bit 10 */
   unsigned             : 3; /* Bit 11-13 */
   unsigned arrivalDate : 1; /* Bit 14 */
   unsigned sysopRead   : 1; /* Bit 15 */   } areaOptionsType;

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


