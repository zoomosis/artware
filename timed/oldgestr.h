/*
**  gestruct.h
**
**  System data files definitions for GEcho 1.01
**
**  Copyright (C) 1993 Gerard J. van der Land. All rights reserved.
**
**  All information in this document is subject to change at any time
**  without prior notice.
**
**  Last revision: 17-Apr-93
**
**  Strings are NUL padded and NUL terminated arrays of char type.
**  Path names are back slash ('\') terminated.
*/

#define GE_THISREV 0x0002       /* System file revision level */
#define GE_MAJOR   1;           /* GEcho major revision version */
#define GE_MINOR   01;          /* GEcho minor revision version */

#define AKAS          11        /* Main + AKAs */
#define UPLINKS       10        /* AreaMgr uplinks */
#define USERS         10        /* User names */
#define MAXAREAS    6500        /* Area records */
#define MAXSYSTEMS   350        /* Systems per area */
#define MAXNODES    5000        /* Node records */

/*
**  Datatypes:
**
**  typedef unsigned char   byte;   ( 8 bits)
**  typedef unsigned short  word;   (16 bits)
**  typedef unsigned long   dword;  (32 bits)
*/

/* --- Address type */

struct _ge_address
{
    word zone;
    word net;
    word node;
    word point;
};

typedef struct _ge_address GE_ADDRESS;

/* --- Log levels */

#define LOG_INBOUND     0x0001
#define LOG_OUTBOUND    0x0002
#define LOG_PACKETS     0x0004
#define LOG_UNEXPECT    0x0008
#define LOG_AREAMGR     0x0010
#define LOG_EXTPKTINFO  0x0040
#define LOG_NETEXPORT   0x0100
#define LOG_NETIMPORT   0x0200
#define LOG_NETPACK     0x0400
#define LOG_NETMOVED    0x0800
#define LOG_STATISTICS  0x2000
#define LOG_MBUTIL      0x4000
#define LOG_DEBUG       0x8000

/* --- Log styles */

#define LOG_FD       0          /* FrontDoor */
#define LOG_BINK     1          /* BinkleyTerm */
#define LOG_QUICK    2          /* QuickBBS */
#define LOG_DBRIDGE  3          /* D'Bridge */

/* --- Setup option bits */

#define NOKILLNULL 0x0001       /* Don't kill null netmail messages while
                                   tossing */
#define RESCANOK   0x0002       /* Allow %RESCAN */
#define KEEPREQS   0x0004       /* Keep AreaMgr requests */
#define NONODEADD  0x0008       /* Don't automatically add NodeMgr records 
                                 */
#define USEJAMBUF  0x0010       /* Use JAM buffer - mandatory for JAM! */
#define USEHMBBUF  0x0020       /* Use Hudson buffers */
#define KEEPNET    0x0040       /* Don't use Kill/Sent on exported netmail 
                                 */
#define KEEPMGR    0x0080       /* Don't use Kill/Sent on MGR receipts */
/* added GE_ to avoid probs w/ timEd KILLDUPES */
#define GE_KILLDUPES  0x0200    /* Kill duplicate messages */
//#define ARCMAIL    0x0400  /* Use ARCmail 0.60 naming for out-of-zone mail */
#define NOCRSTRIP  0x0800       /* Don't strip Soft-CRs */
#define NOAUTODISC 0x2000       /* Don't automatically disconnect empty PT 
                                   areas */
#define NOCHECKEND 0x4000       /* Don't check for valid end of archives */
#define SETPVT     0x8000       /* Set Pvt on imported netmail messages */

/* --- Extra option bits */

#define NOCHKDEST  0x0001       /* Don't check packet destination */
#define AREASBBS   0x0002       /* Use AREAS.BBS for MBUTIL */
#define PAUSEOK    0x0004       /* Allow %PAUSE */
#define NOTIFYOK   0x0008       /* Allow %NOTIFY OFF */
#define ADDALLOK   0x0010       /* Allow +* */
#define PWDOK      0x0020       /* Allow %PWD */
#define PKTPWDOK   0x0040       /* Allow %PKTPWD */
#define NOBADPKTS  0x0080       /* Don't notify sysop about BAD/DST/LOC
                                   packets */
#define PKTPRGONCE 0x0100       /* Run PKT program only before the first
                                   PKT */
#define CREATEBUSY 0x0200       /* Create busy flags */
#define COMPRESSOK 0x0400       /* Allow %COMPRESS */
#define FROMOK     0x0800       /* Allow %FROM */
#define REDIR2NUL  0x1000       /* Redirect output of external utilities
                                   to NUL */
#define NOEXPAND   0x2000       /* Don't expand filenames of file attaches 
                                 */
#define LOCALEXPT  0x4000       /* Export netmail to our own AKA */
#define OPUSDATES  0x8000       /* Use Opus style binary date/time stamps */

/* --- Compression types */

#define PR_ARC  0               /* For compressed mail files created by
                                   ARC or PKPAK */
#define PR_ARJ  1               /* For compressed mail files created by
                                   ARJ */
#define PR_LZH  2               /* For compressed mail files created by
                                   LHA */
#define PR_PAK  3               /* For compressed mail files created by
                                   PAK */
#define PR_ZIP  4               /* For compressed mail files created by
                                   PKZIP */
#define PR_ZOO  5               /* For compressed mail files created by
                                   ZOO */
#define PR_SQZ  6               /* For compressed mail files created by
                                   SQZ */

/* --- Locking method */

#define LOCK_OFF    0           /* Deny Write (Exclusive) */
#define LOCK_RA101  1           /* RemoteAccess 1.01 (SHARE) */
#define LOCK_RA111  2           /* RemoteAccess 1.11 (SHARE) */

/* --- Semaphore mode */

#define SEMPAHORE_OFF  0        /* Don't use semaphores */
#define SEMAPHORE_FD   1        /* FrontDoor 2.1x */
#define SEMAPHORE_IM   2        /* InterMail 2.2x */
#define SEMAPHORE_DB   3        /* D'Bridge 1.3x */
#define SEMAPHORE_BT   4        /* BinkleyTerm 2.5x */

/* --- Check user name */

#define CHECK_NOT        0      /* Don't check if user name exists */
#define CHECK_USERFILE   1      /* User file (USERS.BBS) */
#define CHECK_USERINDEX  2      /* User index (USERSIDX.BBS / NAMEIDX.BBS) 
                                 */

/* --- Mailer type */

#define MAILER_FD  0            /* FrontDoor */
#define MAILER_DB  1            /* D'Bridge */
#define MAILER_BT  2            /* BinkleyTerm */

/* --- BBS type */

#define BBS_RA111     0         /* RemoteAccess 1.1x */
#define BBS_RA200     1         /* RemoteAccess 2.0x */
#define BBS_QUICK275  2         /* QuickBBS 2.7x */
#define BBS_SBBS116   3         /* SuperBBS 1.16 */

/* --- Change PID */

#define PID_NO       0
#define PID_REPLACE  1
#define PID_REMOVE   2

/* --- Change tear line */

#define TEAR_NO       0
#define TEAR_DEFAULT  1
#define TEAR_CUSTOM   2
#define TEAR_EMPTY    3
#define TEAR_REMOVE   4

/* --- Uplink option bits */

#define UPLINK_DIRECT   0x01
#define UPLINK_ALWAYS   0x02
#define UPLINK_ADDPLUS  0x04

typedef struct
{
    GE_ADDRESS address;         /* Uplink address */
    char areafix[9];            /* AreaFix program */
    char password[17];          /* AreaFix password */
    char filename[13];          /* "Forward List" filename */
    byte unused[6];
    byte options;               /* See --- Uplink options bits */
    byte filetype;              /* 0 = Random, 1 = "<areaname>
                                   <description>" */
    dword groups;               /* Nodes must have one of these groups */
    byte origin;                /* Origin AKA */
}
UPLINK;

typedef struct
{
    word zone;
    word net;
    byte aka;
}
AKAMATCH;

typedef struct
{
    byte bg_char;
    byte headerframe;
    byte headertext;
    byte background;
    byte bottomline;
    byte bottomtext;
    byte bottomkey;
    byte errorframe;
    byte errortext;
    byte helpframe;
    byte helptitle;
    byte helptext;
    byte helpfound;
    byte winframe;
    byte wintitle;
    byte winline;
    byte wintext;
    byte winkey;
    byte windata;
    byte winselect;
    byte inputdata;
    byte exportonly;
    byte importonly;
    byte lockedout;
}
COLORSET;

/* --- SETUP.GE structure */

typedef struct
{
    word sysrev;                /* Must contain GE_THISREV */
    word options;               /* Options bits, see --- Setup option bits 
                                 */
    word autorenum;             /* Auto renumber value */
    word maxpktsize;            /* Maximum packet size, 0 = unlimited */
    byte logstyle;              /* See --- Log styles */
    byte oldnetmailboard;       /* Netmail board, must be zero now */
    byte badboard;              /* Where bad echomail is stored (0 = path) 
                                 */
    byte dupboard;              /* Where duplicates are stored (0 = path) */
    byte recoveryboard;         /* Recovery board (1-200, 0 = delete) */
    byte filebuffer;            /* Size (in KB) of MBU file I/O buffer */
    byte days;                  /* Days to keep old mail around */
    byte swapping;              /* Swapping method */
    byte compr_default;         /* Default compresion type */
    byte pmcolor[15];           /* See --- Personal Mail colors */
    GE_ADDRESS aka[AKAS];       /* Main address and AKAs */
    word pointnet[AKAS];        /* Pointnets for all addresses */
    dword gekey;                /* GEcho registration key */
    dword mbukey;               /* MBUTIL registration key */
    char geregto[51];           /* Text used to generate the GEcho key */
    char mburegto[51];          /* Text used to generate the MBUTIL key */
    char username[USERS][36];   /* User names */
    char mbpath[53];            /* Hudson message base path */
    char mailpath[53];          /* Netmail path */
    char inbound_path[53];      /* Where incoming compressed mail is
                                   stored */
    char outbound_path[53];     /* Where outgoing compressed mail is
                                   stored */
    char editor[65];            /* External full screen editor used by PM */
    char nodepath[53];          /* Nodelist directory used by PMNC and PM */
    char areasfile[65];         /* AREAS.BBS style file */
    char logfile[65];           /* GEcho/MBUTIL log file */
    char mgrlogfile[65];        /* AreaMgr log file */
    char swap_path[53];         /* Swap path */
    char tear_line[31];         /* Tearline to be placed by MBUTIL Export */
    char originline[20][61];    /* Origin lines */
    char compr_prog[10][13];    /* Compression program filenames */
    char compr_switches[10][20]; /* Compression program switches */
    char decompr_prog[10][13];  /* Decompression program filenames */
    char decompr_switches[10][20]; /* Decompression program switches */
    char groups[26][21];        /* Descriptions of area groups */
    byte lockmode;              /* See --- Locking method */
    char secure_path[53];       /* From which secure PKTs are tossed */
    char rcvdmailpath[53];      /* Directory to which Rcvd netmail is
                                   moved */
    char sentmailpath[53];      /* Directory to which Sent netmail is
                                   moved */
    char semaphorepath[53];     /* Where FD rescan files are stored */
    byte version_major;         /* Major GEcho version */
    byte version_minor;         /* Minor GEcho version */
    byte semaphore_mode;        /* See --- Semaphore modes */
    char badecho_path[53];      /* Where sec. violating and unknown mail
                                   is stored */
    byte mailer_type;           /* See --- Mailer type */
    word loglevel;              /* See --- Log level */
    AKAMATCH akamatch[20];      /* AKA matching table */
    char mbulogfile[65];        /* MBUTIL log file */
    word maxqqqs;               /* Max. number of QQQ info stored in
                                   memory */
    byte maxqqqopen;            /* Not used */
    byte maxhandles;            /* Max. number of files used by GEcho */
    word maxarcsize;            /* Max. archive size, 0 = unlimited */
    word delfuture;             /* Days to delete messages in the future,
                                   0 = disable */
    word extraoptions;          /* See --- Extra option bits */
    byte firstboard;            /* First available new board (0 = *.MSG) */
    word reserved1;             /* Reserved */
    word copy_persmail;         /* Copy personal mail for user (bits 0-9) */
    byte persmailboard[USERS];  /* Personal mail board (0 = path) */
    dword public_groups;        /* Public groups (bits 0-25) */
    word dupentries;            /* Number of duplicate entries in
                                   ECHODUPE.GE */
    byte rcvdboard;             /* Where Rcvd netmail is moved to (0 =
                                   path) */
    byte sentboard;             /* Where Sent netmail is moved to (0 =
                                   path) */
    byte akaboard[AKAS];        /* Netmail boards for AKAs */
    byte userboard[USERS];      /* Netmail boards for system users, 255 =
                                   use AKA board */
    byte reserved2;             /* Reserved */
    UPLINK uplink[UPLINKS];     /* Uplink manager information */
    char persmail_path[53];     /* Path to store copies of personal
                                   echomail */
    char outpkts_path[53];      /* Where outbound packets are temp. stored 
                                 */
    word compr_mem[10];         /* Memory needed for compression programs */
    word decompr_mem[10];       /* Memory needed for decompression
                                   programs */
    dword pwdcrc;               /* CRC-32 of access password, -1L = no
                                   password */
    word default_maxmsgs;       /* Maximum number of messages (Purge) */
    word default_maxdays;       /* Maximum age of non-Rcvd messages
                                   (Purge) */
    char gus_prog[13];          /* General Unpack Shell program filename */
    char gus_switches[20];      /* GUS switches */
    word gus_mem;               /* Memory needed for GUS */
    word default_maxrcvddays;   /* Maximum age of Rcvd messages (Purge) */
/* ------------------------------------------------------------------------- */
    byte checkname;             /* See --- Check user name */
    byte maxareacachesize;      /* Area cache size, 0 .. 64 KB */
    char inpkts_path[53];       /* Where inbound mail packets should be
                                   stored */
    char pkt_prog[13];          /* Called before each tossed mail packet */
    char pkt_switches[20];      /* Command line switches */
    word pkt_mem;               /* Memory needed */
    word maxareas;              /* Maximum number of areas */
    word maxsystems;            /* Maximum number of systems per area */
    word maxnodes;              /* Maximum number of nodes */
    word default_minmsgs;       /* Minimum number of messages (Purge) */
    byte bbs_type;              /* See --- BBS type */
    byte decompress_ext;        /* 0 = 0-9, 1 = 0-F, 2 = 0-Z */
    byte change_PID;            /* See --- Change PID */
    byte change_tearline;       /* See --- Change tear line */
    word prog_notavail;         /* Bit 0-9, 1 = program not available */
    byte gs_background;         /* Character to use as background */
    byte gscolor[32];           /* GSETUP color see, See COLORSET
                                   structure */
}
SETUP_GE;

/***************************************************************************/

/* --- Area option bits */

#define IMPORTSB  0x0001        /* Import SEEN-BY lines to message base */
#define SECURITY  0x0002        /* Only accept mail from systems in the
                                   exportlist */
#define PASSTHRU  0x0004        /* Mail is not imported, only forwarded */
#define VISIBLE   0x0008        /* Area is visible for anyone in AreaMgr's 
                                   %LIST */
#define REMOVED   0x0010        /* Area should be removed by GSETUP Pack */
#define NOUNLINK  0x0020        /* Do not allow users to unlink this area */
#define TINYSB    0x0040        /* Tiny SEEN-BYs without systems not in
                                   exportlist */
#define PVT       0x0080        /* Private bits are preserved and are not
                                   stripped */
#define CHECKSB   0x0100        /* Use SEEN-BYs for duplicate prevention */
#define NOPAUSE   0x0200        /* Do not allow users to pause this area */
#define SDM       0x0400        /* Area is stored in *.MSG format */
#define HIDESB    0x0800        /* Hide imported SEEN-BY lines behind ^a */
#define NOIMPORT  0x1000        /* AreaMgr will set new nodes to
                                   Export-Only */
#define DELFUTURE 0x2000        /* Del messages dated in the future */
#define NOTIFIED  0x4000        /* Sysop notified that area was
                                   disconnected */
#define UPLDISC   0x8000        /* Disconnected from uplink (only for PT
                                   areas) */

/* --- Extra area option bits */

#define NODUPECHK 0x01          /* Don't do duplicate checking for this
                                   area */
#define NOLINKING 0x02          /* Don't do reply chain linking for this
                                   area */
#define HIDDEN    0x04          /* Area is hidden for everyone */

/* --- Area type */

#define GE_ECHOMAIL  0
#define GE_NETMAIL   1
#define GE_LOCAL     2
#define GE_BADECHO   3 */
/* Added GE_... */
#define GE_PERSONAL  4

/* --- Area format */

#define FORMAT_PT   0           /* Passthru */
#define FORMAT_HMB  1           /* Hudson Message Base */
#define FORMAT_SDM  2           /* *.MSG base */
#define FORMAT_JAM  3           /* Joaquim Andrew Mats message base */
#define FORMAT_SQ   4           /* Squish base */

/* --- AREAFILE.GE header */

typedef struct
{
    word hdrsize;               /* sizeof(AREAFILE_HDR) */
    word recsize;               /* sizeof(AREAFILE_GE) */
    word systems;               /* Number of entries in the export list
                                   (1-maxsystems) */
}
AREAFILE_HDR;

/* --- AREAFILE.GE record */

typedef struct
{
    char name[51];              /* AREA tag, must be uppercase, no spaces */
    char comment[61];           /* Description of the topics discussed in
                                   area */
    char path[51];              /* Path where the *.MSG files are stored */
    char originline[61];        /* Custom origin line, used if origlinenr
                                   = 0 */
    word areanumber;            /* Area number (1-200 = Hudson) */
    char group;                 /* Group (A-Z) */
    word options;               /* See --- Area option bits */
    byte originlinenr;          /* Origin line (1-20, 0 = custom) */
    byte pkt_origin;            /* Address for the packet/Origin line
                                   (0-10) */
    word seenbys;               /* Addresses (bits 0-10) to add to the
                                   SEEN-BY */
    word reserved;
    word maxmsgs;               /* Maximum number of messages (MBUTIL
                                   Purge) */
    word maxdays;               /* Maximum age of non-Rcvd messages
                                   (MBUTIL Purge) */
    word maxrcvddays;           /* Maximum age of Rcvd messages (MBUTIL
                                   Purge) */
    byte areatype;              /* See --- Area type */
    byte areaformat;            /* See --- Area format */
    byte extraoptions;
}
AREAFILE_GE;

/* --- Export entry status bits */

#define EXPT_NOIMPORT  0x01     /* Don't accept mail from this system */
#define EXPT_NOEXPORT  0x02     /* Don't forward mail to this system */
#define EXPT_PAUSE     0x04     /* Temporary don't send this area to this
                                   system */
#define EXPT_NOUNLINK  0x08     /* Don't allow system to disconnect */

/* --- Export list entry */

typedef struct
{
    GE_ADDRESS address;
    byte status;
}
EXPORTENTRY;

/* --- AREAFILE.GEX index entry */

typedef struct
{
    long crc32;                 /* CRC-32 on areaname */
    word areanumber;            /* Area number (1-200 = Hudson) */
    long offset;                /* File offset of record in AREAFILE.GE */
}
AREAFILE_GEX;

/***************************************************************************/

/* --- Status:
   0x0000 = None
   0x0002 = Crash
   0x0200 = Hold
   0xFFFF = Removed entry
*/

/* --- Node option bits */

#define REMOTEMAINT  0x0001     /* Allow node to use %FROM */
/* Added GE_ */
#define GE_ALLOWRESCAN  0x0002  /* Allow node to use %RESCAN */
#define FORWARDREQ   0x0004     /* Allow node to forward AreaMgr requests */
/* Added GE_ */
#define GE_DIRECT       0x0008  /* Use Direct status */
#define NONOTIFY     0x0010     /* Don't send Notify list */
#define PACKNETMAIL  0x0020     /* Pack netmail for this node */

/* --- NODEFILE.GE header */

typedef struct
{
    word hdrsize;               /* sizeof(NODEFILE_HDR) */
    word recsize;               /* sizeof(NODEFILE_GE) */
}
NODEFILE_HDR;

/* --- NODEFILE.GE record */

typedef struct
{
    GE_ADDRESS address;         /* Address of the system */
    char sysop[36];             /* Name of the sysop or point */
    char pktpwd[9];             /* Packet (session) password */
    char mgrpwd[17];            /* AreaMgr password */
    dword groups;               /* Read/write groups (bits 0-25) */
    word options;               /* See --- Node option bits */
    byte comprtype;             /* Compression type (0 - 9, 10 = PKT) */
    word status;                /* File attach status. See above */
    GE_ADDRESS route_to;        /* Address to route mail files to */
/* ------------------------------------------------------------------------- */
    dword readgroups;           /* Read/write groups (bits 0-25) */
}
NODEFILE_GE;

/* --- NODEFILE.GEX index entry */

typedef struct
{
    GE_ADDRESS address;         /* Address of the system */
    long offset;                /* File offset of record in NODEFILE.GE */
}
NODEFILE_GEX;

/***************************************************************************/

/* --- Routed node entry */

typedef struct
{
    GE_ADDRESS node;
    byte status;
    byte via;
}
ROUTE;

/* --- PACKFILE.GE structure */

typedef struct
{
    GE_ADDRESS via[60];
    ROUTE route[640];
}
PACKFILE_GE;

/***************************************************************************/

/* --- ECHODUPE.GE structure */

typedef struct
{
    word pointer;               /* Next offset */
    word entries;               /* Number of entries in the database */
/*
   word crc32_high[entries];     32 bit CRCs on msg headers, high portions
   word crc32_low[entries];      32 bit CRCs on msg headers, low portions
*/
}
ECHODUPE_GE;

/***************************************************************************/

/* --- FTSCPROD.GE record */

#define ZONE_ALL  0x01
#define NET_ALL   0x02
#define NODE_ALL  0x04

typedef struct
{
    byte cap;                   /* Capability, 0 = Type 2.0, 1 = Type 2.1, 
                                   2 = Type 2+ */
    char name[30];              /* Name of product */
}
FTSCPROD_GE;

/***************************************************************************/

typedef struct
{
    word old_msgnum;
    word new_msgnum;
}
MBUTIL_RNX;

/* --- Personal Mail colors (same as used by FrontDoor editor FM) */

#define pm_top_line               pmcolor[ 0]
#define pm_status_line            pmcolor[ 1]
#define pm_error                  pmcolor[ 2]
#define pm_text                   pmcolor[ 3]
#define pm_text_quotes            pmcolor[ 4]
#define pm_text_reverse           pmcolor[ 5]
#define pm_text_hardCRs           pmcolor[ 6]
#define pm_header                 pmcolor[ 7]
#define pm_header_data            pmcolor[ 8]
#define pm_header_data_highlight  pmcolor[ 9]
#define pm_data_entry             pmcolor[10]
#define pm_window_frame           pmcolor[11]
#define pm_window_text            pmcolor[12]
#define pm_window_select          pmcolor[13]
#define pm_window_highlight       pmcolor[14]

/* end of file "gestruct.h" */
