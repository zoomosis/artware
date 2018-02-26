#include "spack.h"

#define SQNUMATTR1 26

#ifdef __FLAT__
#define MAXTRAILSIZE 8096
#else
#define MAXTRAILSIZE 4096
#endif


/* The eXtended message structure.  Translation between this structure, and *
 * the structure used by the individual message base formats, is done       *
 * on-the-fly by the API routines.                                          */

typedef struct
{
    dword attr;

    /* Bitmasks for 'attr' */

#define MSGPRIVATE 0x0001
#define MSGCRASH   0x0002
#define MSGREAD    0x0004
#define MSGSENT    0x0008
#define MSGFILE    0x0010
#define MSGFWD     0x0020
#define MSGORPHAN  0x0040
#define MSGKILL    0x0080
#define MSGLOCAL   0x0100
#define MSGHOLD    0x0200
#define MSGXX2     0x0400
#define MSGFRQ     0x0800
#define MSGRRQ     0x1000
#define MSGCPT     0x2000
#define MSGARQ     0x4000
#define MSGURQ     0x8000

#define MSGSCANNED  0x00010000L
#define MSGUID      0x00020000L

    /* GvE: */

#define XMSG_FROM_SIZE  36
#define XMSG_TO_SIZE    36
#define XMSG_SUBJ_SIZE  72

    byte from[XMSG_FROM_SIZE];
    byte to[XMSG_TO_SIZE];
    byte subj[XMSG_SUBJ_SIZE];

    NETADDR orig;               /* Origination and destination addresses */
    NETADDR dest;

    struct _stamp date_written; /* When user wrote the msg (UTC) */
    struct _stamp date_arrived; /* When msg arrived on-line (UTC) */
    sword utc_ofs;              /* Offset from UTC of message writer, in *
                                   minutes.  */

#define MAX_REPLY 10            /* Max number of stored replies to one msg 
                                 */

    UMSGID replyto;
    UMSGID replies[MAX_REPLY];

    byte ftsc_date[20];         /* Obsolete date information.  If it
                                   weren't for the * fact that FTSC
                                   standards say that one cannot * modify
                                   an in-transit message, I'd be VERY *
                                   tempted to axe this field entirely, and 
                                   recreate * an FTSC-compatible date
                                   field using * the information in
                                   'date_written' upon * export.  Nobody
                                   should use this field, except *
                                   possibly for tossers and scanners.  All 
                                   others * should use one of the two
                                   binary datestamps, * above.  */
} XMSG;

#define SQADDMASK1 (aAS|aDIR|aIMM|aKFS|aTFS|aLOK|aCFM)
#define SQADDMASK2 (aHUB|aXMA|aHIR|aCOV|aSIG|aLET|aFAX)

#include "spop.h"
