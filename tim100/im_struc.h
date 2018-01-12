
/* *
 * *
 * *   I M A I L   D E F I N I T I O N S   A N D   S T R U C T U R E S
 * *
 * */

/*
 *
 * Copyright (C) 1992/1993 by Andreas Klein. All rights reserved.
 *
 * The information in this file may not be passed on to others
 * without prior permission from the authors.
 *
 * The contents of this file are subject to change without notice!
 * Fields marked reserved should NOT be used.
 *
 * It is an ERROR to write to any of the configuration files
 * while IMAIL (or any of its companion programs) is running.
 * 
 */

#include <dir.h>
#include <io.h>
#include <time.h>

#define IMAIL_MAJ_VERSION     1
#define IMAIL_MIN_VERSION     40
#define STRUCT_MAJ_VERSION    1
#define STRUCT_MIN_VERSION    01
#define IM_PRD_CODE           0x4B

/*
 *
 *  Data type definitions
 *
 */

//#define byte                 unsigned char
/* #define word                 unsigned short */
#define boolean              short
// #define sword              short
// typedef unsigned short       bit;

/*
 *
 *  Function definitions
 *
 */

#ifdef __OS2__
  #define farmalloc    malloc
  #define farcalloc    calloc
  #define farfree      free
  #define _fstrchr     strchr
  #define _fstrstr     strstr
#endif

/*
 *
 *  Internal limits
 *
 */

#define MAXAKAS              16      // Max # of addresses
#define MAXPACKERS           11      // Max # of packer def
#define MAXEXPORT            60      // Max export defs
#define MAXVIA               40      // max nodes packed via
#define MAXGROUPS            26      // max nodes packed via
#define MAXEXCEPT            10      // max EXCEPT nodes
#define MAXPACK              32      // max default pack cmd
#define MAXFWDLINK           10      // max fwd link structs
#define MAXNOIMPT            20      // max # names for IMPORT
#define ZONESDOM             10      // zones per domain entry
#define MAXTAG               51      // max areatag length
#define MAXNAME              37      // max namefield lenght
#define MAXPACKNAME          50      // max packer length
#define MAXORIGIN            64      // max origin length

/*
 *
 *  Log Style Definitions
 *
 */

#define LOG_NORMAL           0                        // normal log level
#define LOG_VERBOSE          1                        // verbose log level

#define LOG_LOG              252                      // Always, but to file
#define LOG_SCREEN           253                      // Always, but to scrn
#define LOG_ALL              254                      // Always log this
#define LOG_ERROR            255                      // Force error logging

/*
 *
 *  Mailer Environment Type  (FrontDoor, Binkley or InterMail)
 *
 */

#define ENV_FRODO            0                        // FrontDoor
#define ENV_BINK             1                        // Binkley
#define ENV_IM               2                        // Intermail

/*
 *
 *  BBS Environment Type
 *
 */

#define BBS_RA2              0                        // Remote Access 2.0
#define BBS_OTHER            1                        // Other

/*
 *
 *  File Attach Message Status (used in IMAIL.ND)
 *
 */

/*
enum message_status{S_NORMAL, S_HOLD, S_CRASH, S_IMMEDIATE, S_DIRECT,
                    S_NORMAL_DIR, S_HOLD_DIR, S_CRASH_DIR, S_IMMEDIATE_DIR};
*/

#define S_NORMAL             0
#define S_HOLD               1
#define S_CRASH              2
#define S_IMMEDIATE          3
#define S_DIRECT             4
#define S_NORMAL_DIR         4
#define S_HOLD_DIR           5
#define S_CRASH_DIR          6
#define S_IMMEDIATE_DIR      7


/*
 *
 *  Message Base Types
 *
 */

  #define IMSGTYPE_SDM     0x01
  #define IMSGTYPE_SQUISH  0x02
  #define IMSGTYPE_HUDSON  0x03
  #define IMSGTYPE_QBBS    0x03
  #define IMSGTYPE_JAM     0x04
  #define IMSGTYPE_PASSTH  0x0F
  #define IMSGTYPE_ECHO    0x80
  #define IMSGTYPE_LOCAL   0x90
  #define IMSGTYPE_NET     0xA0

/*
 *
 *  Via Line Switches
 *
 */

#ifndef VIA_NONE
  #define VIA_NONE        0x01
#endif
#ifndef VIA_EXP
  #define VIA_EXP         0x02
#endif
#ifndef VIA_IMP
  #define VIA_IMP         0x03
#endif
#ifndef VIA_BOTH
  #define VIA_BOTH        0x04
#endif

/*
 *
 *  Dupe ring check switches
 *
 */

#ifndef DUPE_NONE
  #define DUPE_NONE        0x00
#endif
#ifndef DUPE_WARN
  #define DUPE_WARN        0x01
#endif
#ifndef DUPE_ZONE
  #define DUPE_ZONE        0x02
#endif
#ifndef DUPE_KILL
  #define DUPE_KILL        0x03
#endif

/*
 *
 *  Circular path detection switches
 *
 */

#ifndef CPD_NONE
  #define CPD_NONE        0x00
#endif
#ifndef CPD_WARN
  #define CPD_WARN        0x01
#endif
#ifndef CPD_KILL
  #define CPD_KILL        0x02
#endif

/*
 *
 *  Unlink handling
 *
 */

#ifndef ULNK_NONE
  #define ULNK_NONE        0x00
#endif
#ifndef ULNK_PASSTH
  #define ULNK_PASSTH      0x01
#endif
#ifndef ULNK_ALL
  #define ULNK_ALL         0x02
#endif

/*
 *
 *  Personal Mail handling
 *
 */

#ifndef PERSM_NONE
  #define PERSM_NONE       0x00
#endif
#ifndef PERSM_LOG
  #define PERSM_LOG        0x01
#endif
#ifndef PERSM_MSG
  #define PERSM_MSG        0x02
#endif
#ifndef PERSM_COPY
  #define PERSM_COPY       0x03
#endif

/*
 *
 *  AreaLink Request Handling
 *
 */

#ifndef KEEP_NONE
  #define KEEP_NONE        0x00
#endif
#ifndef KEEP_MSG
  #define KEEP_MSG         0x01
#endif
#ifndef KEEP_ALL
  #define KEEP_ALL         0x02
#endif

/*
 *
 *  PKTs not for us Handling
 *
 */

#ifndef PKT_TOSS
  #define PKT_TOSS         0x00
#endif
#ifndef PKT_FORWARD
  #define PKT_FORWARD      0x01
#endif
#ifndef PKT_RENAME
  #define PKT_RENAME       0x02
#endif

/*
 *
 *  Macros to make life easier
 *
 */

#define BASEMASK          0x0F
#define TYPEMASK          0xF0

#define IsSdm(Type)       ((Type & BASEMASK) == IMSGTYPE_SDM)
#define IsMsg(Type)       ((Type & BASEMASK) == IMSGTYPE_SDM)
#define IsSquish(Type)    ((Type & BASEMASK) == IMSGTYPE_SQUISH)
#define IsJam(Type)       ((Type & BASEMASK) == IMSGTYPE_JAM)
#define IsHudson(Type)    ((Type & BASEMASK) == IMSGTYPE_HUDSON)
#define IsQbbs(Type)      ((Type & BASEMASK) == IMSGTYPE_QBBS)
#define IsPassth(Type)    ((Type & BASEMASK) == IMSGTYPE_PASSTH)
#define IsEcho(Type)      ((Type & TYPEMASK) == IMSGTYPE_ECHO)
#define IsLocal(Type)     ((Type & TYPEMASK) == IMSGTYPE_LOCAL)
#define IsNet(Type)       ((Type & TYPEMASK) == IMSGTYPE_NET)



/*
 *
 *  Exit Errorlevels & Error Messages
 *
 */

#define E_NOERR              0            // no error
#define E_CRDIR              238          // Error creating directory
#define E_ELOCK              239          // File locking error
#define E_AOPEN              240          // Error opening IMAIL.AX
#define E_BOPEN              241          // Error opening IMAIL.BX
#define E_NOIDX              242          // Index file missing/corrupt
#define E_NOCFG              243          // IMAIL.CF not found
#define E_NOARE              244          // IMAIL.AR not found
#define E_NONOD              245          // IMAIL.ND not found
#define E_BADCF              246          // Error in IMAIL.CF
#define E_BADVR              247          // Bad version of IMAIL.CF
#define E_EOPEN              248          // Error opening file
#define E_EREAD              249          // Error reading file
#define E_EWRIT              250          // Error writing file
#define E_CMDPR              251          // Command Line Parameter error
#define E_FILNF              252          // File not found
#define E_MEMRY              253          // Memory Allocation error
#define E_DISKF              254          // Insufficient disk space
#define E_UNKWN              255          // Unknown error

/* === ERRORLEVELs returned by TOSS and/or SCAN === */

#define T_NOMAIL             0x00         // no mail processed
#define T_NET                0x01         // Net mail
#define T_ECHO               0x02         // Echo mail
#define T_BAD                0x04         // Bad and/or dupe mail
#define T_QBBS               0x08         // Qbbs message base changed
#define T_MSG                0x10         // *.MSG message base changed
#define T_SQUISH             0x20         // Squish message base changed
#define T_PERS               0x40         // Mail to Sysop received

/*
 *
 *  Special values for 'ALL'
 *
 */

/*#define ZONE_ALL             56685u
#define NET_ALL              56685u
#define NODE_ALL             56685u
#define POINT_ALL            56685u
*/
/*
 *
 *  Misc other definitions required
 *
 */

#define FA_ANYFILE FA_RDONLY+FA_HIDDEN+FA_SYSTEM+FA_ARCH

#define TRUE	   	     1
#define FALSE	   	     0
#define BLANK	 	     ' '

typedef char str255[256];
typedef char str35[36];
typedef char str16[16];

typedef char pktdate[20];

typedef struct array512 {
  word         len;
  char         longstring[512];
} array512;

/* define some simple keyword replacements */

#define strdelete(s,p,num) strcpy(s+p-1,s+p+num)

#define STRSIZ               255                  // default string length

/*
 *
 *  In case your compiler doesn't have these ...
 *
 */

//#ifndef MAXPATH
//  #define MAXPATH            80
//#endif
#ifndef MAXDRIVE
  #define MAXDRIVE           3
#endif
#ifndef MAXDIR
  #define MAXDIR             66
#endif
#ifndef MAXFILE
  #define MAXFILE            9
#endif
#ifndef MAXEXT
  #define MAXEXT             5
#endif


/*
 *
 *                 QuickBBS Structures
 *
 */

typedef byte flagtype[4];

typedef struct userrecord    /*  Used in the USERS.BBS File  */
{
  char         name[36];
  char         city[26];
  char         pwd[16];
  char         dataphone[13];
  char         homephone[13];
  char         lasttime[6];
  char         lastdate[9];
  byte         attrib;
  flagtype     flags;
  sword      credit;
  sword      pending;
  sword      timesposted;
  sword      highmsgread;
  sword      seclvl;
  sword      times;
  sword      ups;
  sword      downs;
  sword      upk;
  sword      downk;
  sword      todayk;
  sword      elapsed;
  sword      len;
  byte         extraspace[8];
} userrecord;

/* --- Attrib: -----------------------------------------------------

    Bit 0: Deleted
    Bit 1: Screen Clear Codes
    Bit 2: More Prompt
    Bit 3: ANSI
    Bit 4: No-Kill
    Bit 5: Ignore Download Hours
    Bit 6: ANSI Full Screen Editor
    Bit 7: [ Reserved ]

    ------------------------------------------------------------- */

typedef struct userrecord_ra2     /*  Used in the RA 2.00 USERS.BBS File  */
{
  char  name[36];
  char  Location[26];
  char  Organisation[51];
  char  Address1[51];
  char  Address2[51];
  char  Address3[51];
  char  Handle[36];
  char  Comment[81];
  long  PasswordCRC;
  char  DataPhone[15];
  char  VoicePhone[15];
  char  LastTime[6];
  char  LastDate[9];
  byte  Attribute;
  byte  Attribute2;
  byte  Flags[4];
  long  Credit;
  long  Pending;
  word  MsgsPosted;
  word  Security;
  long  LastRead;
  long  NoCalls;
  long  Uploads;
  long  Downloads;
  long  UploadsK;
  long  DownloadsK;
  long  TodayK;
  short Elapsed;
  word  ScreenLength;
  byte  LastPwdChange;
  word  Group;
  word  CombinedInfo[200];
  char  FirstDate[9];
  char  BirthDate[9];
  char  SubDate[9];
  byte  ScreenWidth;
  byte  Language;
  byte  DateFormat;
  char  ForwardTo[36];
  word  MsgArea;
  word  FileArea;
  char  DefaultProtocol;
  word  FileGroup;
  byte  LastDOBCheck;
  byte  Sex;
  long  XIrecord;
  word  MsgGroup;
  byte  FreeSpace[48];
} userrecord_ra2;

/* --- Attrib RA2: -----------------------------------------------------

    Attribute

      Bit 0 : Deleted
      Bit 1 : Clear screen
      Bit 2 : More prompt
      Bit 3 : ANSI
      Bit 4 : No-kill
      Bit 5 : Xfer priority
      Bit 6 : Full screen msg editor
      Bit 7 : Quiet mode

    Attribute2

      Bit 0 : Hot-keys
      Bit 1 : AVT/0
      Bit 2 : Full screen message viewer
      Bit 3 : Hidden from userlist
      Bit 4 : Page priority
      Bit 5 : No echomail in mailbox scan
      Bit 6 : Guest account
      Bit 7 : Post bill enabled

    ------------------------------------------------------------- */

/*
 *
 *  Message Records
 *
 */

typedef struct inforecord {                   //  Used in the MSGINFO.BBS file
  word         lowmsg;                        //  Lowest Message in File
  word         highmsg;                       //  Highest Message in File
  word         totalactive;                   //  Total Active Messages
  word         activemsgs[200];
} inforecord;

typedef struct idxrecord {                    //  Used in the MSGIDX.BBS file
  word         msgnum;
  byte         board;
} idxrecord;

typedef struct hdrrecord {                    //  Used in the MSGHDR.BBS file
  word         msgnum, replyto, seealsonum, tread;
  word         startrec;
  word         numrecs, destnet, destnode, orignet, orignode;
  byte         destzone, origzone;
  word         cost;
  byte         msgattr, netattr, board;
  char         posttime[6];
  char         postdate[9];
  char         whoto[36], whofrom[36];
  char         subj[73];
} hdrrecord;


/* --- Msg Attributes: --------------------------------------------- */

#define QMSGDELTD   0x01         // deleted message,          0000 0001
#define QMSGOUTN    0x02         // outgoing net message      0000 0010
#define QMSGISNET   0x04         // Is a net message          0000 0100
#define QMSGPRIVATE 0x08         // Private                   0000 1000
#define QMSGREAD    0x10         // Received                  0001 0000
#define QMSGOUTE    0x20         // Outgoing echo message     0010 0000
#define QMSGLOCAL   0x40         // Local                     0100 0000
#define QMSGXX1     0x80         // [reserved]                1000 0000

/* --- Net Attributes: --------------------------------------------- */

#define QNETKILL    0x01         // Kill when sent            0000 0001
#define QNETSENT    0x02         // Sent                      0000 0010
#define QNETFILE    0x04         // File attach               0000 0100
#define QNETCRASH   0x08         // Crash Priority            0000 1000
#define QNETRRQ     0x10         // Request Receipt           0001 0000
#define QNETARQ     0x20         // Audit request             0010 0000
#define QNETCPT     0x40         // Is a return receipt       0100 0000
#define QNETXX1     0x80         // [reserved]                1000 0000

/* --------------------------------------------------------------------

    Each record in the MSGTOIDX.BBS file is of type String[35], each
    entry is simply the name of the person the corresponding message
    in MSGHDR.BBS is addressed to.

    Each record in the MSGTXT.BBS file is of type String[255], each
    entry is used to store blocks of message text.  The variable
    "StartRec" in the MSGHDR.BBS file contains the starting record
    position for the text, the variable "NumRecs" indicates how many
    consecutive blocks of text there are in the MSGTXT.BBS file.

   ----------------------------------------------------------------- */


/* *
 * *
 * *               Fido Packet Structures
 * *
 * */


/*
 *
 *  Normal Message Header  (*.MSG)
 *
 */

typedef struct f2normsghdr {
  char         fromwho[36];                           //  From username
  char         towho[36];                             //  To username
  char         subj[72];                              //  Subject
  char         date[20];                              //  date string
  word         times;                                 //  times read
  word         destnode;                              //  destination node
  word         orignode;                              //  origin node
  word         cost;                                  //  cost
  word         orignet;                               //  origin net
  word         destnet;                               //  destination net
  word         destzone;                              //  destination zone
  word         origzone;                              //  origin zone
  word         destpoint;                             //  destination point
  word         origpoint;                             //  origin point
  word         reply;                                 //  this is a reply to?
  word         attr;                                  //  Attribute
  word         nextreply;                             //  reply to this msg...
} f2normsghdr;



/*
 *
 * Message attributes
 *
 */

#define FMSGPRIVATE 0x0001   // private message,          0000 0000 0000 0001
#define FMSGCRASH   0x0002   // accept for forwarding     0000 0000 0000 0010
#define FMSGREAD    0x0004   // read by addressee         0000 0000 0000 0100
#define FMSGSENT    0x0008   // sent OK (remote)          0000 0000 0000 1000
#define FMSGFILE    0x0010   // file attached to msg      0000 0000 0001 0000
#define FMSGFWD     0x0020   // being forwarded           0000 0000 0010 0000
#define FMSGORPHAN  0x0040   // unknown dest node         0000 0000 0100 0000
#define FMSGKILL    0x0080   // kill after mailing        0000 0000 1000 0000
#define FMSGLOCAL   0x0100   // FidoNet vs. local         0000 0001 0000 0000
#define FMSGHOLD    0x0200   //                           0000 0010 0000 0000
#define FMSGXX1     0x0400   // STRIPPED by FidoNet<tm>   0000 0100 0000 0000
#define FMSGFRQ     0x0800   // file request              0000 1000 0000 0000
#define FMSGRRQ     0x1000   // receipt requested         0001 0000 0000 0000
#define FMSGCPT     0x2000   // is a return receipt       0010 0000 0000 0000
#define FMSGARQ     0x4000   // audit trail requested     0100 0000 0000 0000
#define FMSGURQ     0x8000   // update request            1000 0000 0000 0000

#define BUNDLE_VER 2         // IFNA Bundle Version
#define CAP_TYPE2   0x0001   // Can make Type 2 bundles
#define CAP_STONAGE 0x0000   // No particular capabailties, Type 2 assumed

/*
 *
 *  Header for Message Bundles (Bundle Type 2+)
 *
 */

typedef struct f2pktmsghdr {
   word orignode,              // originating node
        destnode,              // destination node
        year,                  // 1990 - nnnnn
        month,
        day,
        hour,
        minute,
        second,
        rate,                  // unused
        ver,                   // 2
        orignet,               // originating net
        destnet;               // destination net
    byte
        prod_lo,               // product code
        rev_major,             // revision level, major
        password[8];           // packet level password
    word
        qm_origzone,           // QMail wants orig.zone here
        qm_destzone;           // QMail wants dest.zone here
    byte
        trash[2];              // junk[8]
    word
        cap_valid;             // Cap. validity, byte swapped
    byte
        prod_hi,               // product code, hi byte
        rev_minor;             // revision level, minor
    word
        capability,            // bundling capability
        origzone,              // originating zone
        destzone,              // destination zone
        origpoint,             // originating point
        destpoint;             // destination point
    long
        junk;                  // junk
} f2pktmsghdr;

    /* --- Note ----------------------------------------------------

    (*) In the original FSC-0001 specification, these fields
    were part of the Filler, and so may not be recognized
    by all software.

    ------------------------------------------------------------- */


/*
 *
 *  Packeted Message Header (Bundle Type 2+)
 *
 */

typedef struct f2msghdr {
  word         ver;                //  Version (=2)
  word         orignode;           //  Originate Node
  word         destnode;           //  Destination Node
  word         orignet;            //  Originate Net
  word         destnet;            //  Destination Net
  word         attr;               //  Attribute
  word         cost;               //  Cost
  pktdate      date;               //  Date String
/*
  char         towho[36];             To username
  char         fromwho[36];           From username
  char         subject[72];           Subject
*/
} f2msghdr;

    /* --- Note ----------------------------------------------------

    The last three fields are variable length; they are
    shown above for reference, but are not part of the
    structure.

    ------------------------------------------------------------- */


/*
 *
 *  Structs used in IMAIL Configuration files
 *
 */

typedef struct naddress {        // std node number ...
  word         zone;             //  Zone Number
  word         net;              //  Net Number
  word         node;             //  Node Number
  word         point;            //  Point Number
} naddress;

typedef struct expt {
  naddress     dest;
  boolean      doexpt;
} expt;

typedef struct eaddress {                   // used in Area Manager ...
  struct naddress dstn;                     // node number
  unsigned             exp_only:1;               // export only flag
  unsigned             imp_only:1;               // import only flag
  unsigned             paused:1;                 // echo mail paused
  unsigned             rsvd1:5;                  // reserved
} eaddress;

typedef struct fwd_link {                   // used for forward request nodes ...
  char         areasfile[MAXFILE+MAXEXT];   // name of areas file
  char         toprogram[10];               // name of area manager
  char         password[21];                // area manager password
  struct naddress uplink;                   // address of uplink
} fwd_link;

typedef struct dom {
  char         domain[21];                  // name of domain
  char         outbound[80];           // root outbound path
  word         zones[ZONESDOM];             // Zones in this domain
  byte         akas[MAXAKAS];               // =my= AKAs in this domain
} dom;

typedef struct im_stats {                   // Used mainly by IUTIL
  word         th_day;                      // this day
  word         la_day;                      // last day
  word         th_week;                     // this week
  word         la_week;                     // last week
  word         th_month;                    // this month
  word         la_month;                    // last month
  word         th_year;                     // this year
  word         la_year;                     // last year
} im_stats;


/*
 *
 *  IMAIL.CF Structure
 *
 */

typedef struct im_config_type {
  byte         im_ver_maj;                    // Major Version
  byte         im_ver_min;                    // Minor Version
  byte         struct_maj;                    // reserved
  byte         struct_min;                    // reserved
  char         sysop[MAXNAME];                // name of sysop
  struct naddress aka[MAXAKAS];               // the AKAs
/*  struct dom   domains[MAXAKAS];              // domain names & zones
  byte         prod[10];                      // Type2+ product codes
  char         netmail[80];              // net mail subdirectory
  char         netfile[80];              // inbound files directory
  char         in_pkt[80];               // Directory for inbound PKTs
  char         out_pkt[80];              // Directory for outbound PKTs
  char         outbound[80];             // outbound directory
  char         quickbbs[80];             // QuickBBS system directory
  char         uns_netfile[80];          // unsecured inbound files
  char         rsvd8[80];                // reserved
  char         rsvd9[80];                // reserved
  char         semaphor[80];             // Semaphor directory
  char         logfilename[80];          // Log file name
  char         before_toss[80];          // call before proc. PKTs
  char         semaphor_net[MAXFILE+MAXEXT];  // Netmail rescan semaphor file
  char         alnk_help[MAXFILE+MAXEXT];     // AreaLink help text
  char         maint_help[MAXFILE+MAXEXT];    // Alnk Remote Maint. Helptext
  char         new_areas[MAXFILE+MAXEXT];     // name of file for new areas
  char         dflt_origin[MAXORIGIN];        // default origin line
  unsigned          rtnrecpt:1;                    // True if to send rtn recpt
  unsigned          del_empty_msg:1;               // delete empty netmails (TOSS)
  unsigned          ARCmail06:1;                   // ARCmail 0.6 compatibility
  unsigned          rsvd10:1;                      // reserved
  unsigned          auto_add:1;                    // Auto-add unknown areas
  unsigned          multi_tasking:1;               // TRUE if multi-tasking
  unsigned          ignore_unknown:1;              // ALNK ignores unknown systems
  unsigned          singleextract:1;               // extract 1 bundle at a time
  unsigned          trunc_sent:1;                  // 1 = Trunc 0 = Delete
  unsigned          keep_alnk_answ:1;              // keep arealink answer
  unsigned          prod_names:1;                  // use the FTSC product list
  unsigned          swap_ems:1;                    // swap to EMS
  unsigned          swap_ext:1;                    // swap to extended memory
  unsigned          forward_everything:1;          // forward req. not in fwd-lists
  unsigned          direct_video:1;                // use direct screen writing
  unsigned          forward_req:1;                 // Execute forward link req.
  unsigned          compr_after_pkt:1;             // compress after each PKT?
  unsigned          delete_bases:1;                // when removing an area,
                                              // delete also squish/msg-base
  unsigned          kill_on_fly:1;                 // use Squish 'kill on the fly'
  unsigned          rsvd2:5;                       // reserved
  long         last_run;                      // last run of IUTIL STAT
  byte         rsvd5;                         // reserved
  byte         rsvd6;                         // reserved
  word         rsvd7;                         // reserved
  word         max_arcmail_size;              // max size of arcmail bundles
  word         pwd_expire_days;               // days before pwd expr'd
  word         max_pkt_size;                  // max size of pkt to create
  word         rsvd3;                         // reserved
  byte         rsvd11;                        // reserved
  byte         max_msg_size;                  // max size of netmail (split)
  byte         via_line;                      // add Via Line to netmails
  byte         dupe_ring;                     // Check for possible d-rings
  byte         cpd_check;                     // circular path detection
  byte         pers_mail;                     // use personal mail feature
  byte         unlink_req;                    // Unlink areas without dlink
  byte         keep_alnk_req;                 // keep arealink request
  byte         log_level;                     // logging level
  char         no_import[MAXNOIMPT][38];      // ignore when importing
  struct fwd_link fwd[MAXFWDLINK];            // forward link requests
  long         max_dupes;                     // max dupes kept in dbase
  long         false_dupes;                   // false dupes found
  byte         max_add_pkt;                   // PKTs to compress in one run
  byte         pkt_not_for_us;                // how to handle PKTs not for us
  byte         environment;                   // FroDo, Binkley or Intermail
  char         bbs_system;                    // BBS software used
  char         filler[249];                   // reserved */
} im_config_type;

    /* --- Note ----------------------------------------------------

    The information about the badmail-area, the dupe-area
    and the personalmail-area has been moved to IMAIL.AR,
    see there for detailed information.

    ------------------------------------------------------------- */

/*
 *
 *  IMAIL.AR Structure
 *
 */

typedef struct areas_record_type {
  char         aname[MAXTAG];              // area name
  char         comment[61];                // area comment
  char         origin[MAXORIGIN];          // origin line to use
  char         group;                      // area group
  char         o_addr;                     // address for origin
  char         use_akas[MAXAKAS];          // addresses for seen-bys
  byte         msg_base_type;              // message base type
  byte         brd;                        // board number
  char         msg_path[80];               // MSG/Squish path
  unsigned          active:1;                   // flag area active
  unsigned          zone_gate:1;                // Zone-gate stripping
  unsigned          tiny_seen:1;                // tiny seen-by flag
  unsigned          secure:1;                   // secure flag
  unsigned          import_seen:1;              // import seen-by into base
  unsigned          deleted:1;                  // flag deleted area
  unsigned          auto_added:1;               // flag auto-added record
  unsigned          mandatory:1;                // area is mandatory
  unsigned          read_only:1;                // area is read only
  unsigned          unlinked:1;                 // area has been unlinked
  unsigned          ulnk_req:1;                 // perform unlinked requests?
  unsigned          hidden:1;                   // area is hidden
  unsigned          to_link:1;                  // should by processed by LINK
  unsigned          check_dup:1;                // check for dupes in this area?
  unsigned          no_pause:1;                 // %PAUSE not allowed in this echo?
  unsigned          hide_seen:1;                // Hide seens when importing
  byte         iutil_bits;                 // bits for IUTIL
  byte         user_bits;                  // 8 user-available bits
  byte         days;                       // days to keep messages
  word         msgs;                       // num messages to keep
  im_stats     t_stats;                    // TOSS stats
  im_stats     s_stats;                    // SCAN stats
  im_stats     d_stats;                    // dupe statistics
  time_t       creation;                   // date/time of creation
  time_t       update;                     // last update by STAT
  time_t       marked;                     // used by kill dead
  byte         kill_dead;                  // kill echos without traffic
  char         filler[9];
  struct eaddress export[MAXEXPORT];       // export list
} areas_record_type;

    /* --- Notes --------------------------------------------------

    1) The entries in 'o_addr' and 'use_akas' are indexes into
       the list of AKAs in IMAIL.CF, minus 1. eg:
          im_config.aka[o_addr-1]
       A value of 0 means 'no address'
    2) the 'user_bits' entry allows third-part software to store
       extra information in IMAIL.AR. Their meaning is program-
       specific, so be careful when making use of them!

    3) IMAIL.AR knows three predefined areatags:

        BADMAIL       for the IMAIL badmail area,
        DUPES         for the IMAIL dupe area and
        PERSMAIL      for the IMAIL personal mail area.

       All three boards are treated as local areas and
       the BADMAIL area must be present and non-passthru
       otherwise IMAIL will not run.

    ------------------------------------------------------------- */


/*
 *
 *  IMAIL.AX & IMAIL.BX Structures
 *
 */

