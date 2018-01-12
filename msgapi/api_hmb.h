/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/


struct _msgh
{
  MSG *sq;
  dword id;  /* Must always equal MSGH_ID */

  dword bytes_written;
  dword cur_pos;

  /* For HMB only! */

  dword clen;
  dword rawclen;
  dword cur_len;
  dword msgnum;
  dword realno;
  dword totlen;

  sword recno;
  word  startblock;
  word  numblocks;
  byte  lastblocklen;
  word  blocks_written;

  word mode;

  /*   Data that is 'buffered' 'cause we need to analyse it to   */
  /*   find out lengths anyway.                                  */

  char kludges[2049]; /* + 1 for trailing '\0'! */

  MIS mis;
  dword trailsize;
  char *trail;

};


/* Easy access to the board number */

#define HMBDATA ((HMBdata *)sq->apidata)
#define BNUM    (((HMBdata *)sq->apidata)->board - 1)



/* Hudson base structures */

#define HMB_DELETED      0x0001           /* Message attributes               */
#define HMB_UNSENT       0x0002
#define HMB_NETMAIL      0x0004
#define HMB_PRIVATE      0x0008
#define HMB_RECEIVED     0x0010
#define HMB_UNMOVED      0x0020
#define HMB_LOCAL        0x0040

#define HMB_KILL         0x0100           /* Hudson netmail attributes        */
#define HMB_SENT         0x0200
#define HMB_FILE         0x0400
#define HMB_CRASH        0x0800
#define HMB_RECEIPT      0x1000
#define HMB_AUDIT        0x2000
#define HMB_RETURN       0x4000


typedef struct {                       /* MSGINFO.BBS structure definition */
  word low_msg;                /* Lowest msg # in message base     */
  word high_msg;               /* Highest msg # in message base    */
  word total_msgs;             /* Total # of messages in base      */
  word total_on_board [200];   /* Number of messages / board       */
} HMB_MSGINFO;

typedef struct {                       /* MSGIDX.BBS structure definiton   */
  sword msg_num;                         /* Message #                        */
  unsigned char board;                 /* Board # where msg is stored      */
} HMB_MSGIDX;

typedef char HMB_MSGTOIDX[36];     /* MSGTOIDX.BBS structure def.      */

typedef struct {                       /* MSGHDR.BBS structure definition  */
  sword msgnum;                          /* Message number                   */
  word prev_reply;             /* Msg # of previous reply, 0 if no */
  word next_reply;             /* Msg # of next reply, 0 if none   */
  word times_read;             /* # of times msg was read, UNUSED  */
  word start_block;            /* Record # of msg in MSGTXT.BBS    */
  word num_blocks;             /* # of records in MSGTXT.BBS       */
  word dest_net;               /* Destination net                  */
  word dest_node;              /* Destination node                 */
  word orig_net;               /* Origin net                       */
  word orig_node;              /* Origin node                      */
  unsigned char dest_zone;             /* Destination zone                 */
  unsigned char orig_zone;             /* Origin zone                      */
  word cost;                   /* Cost (Netmail)                   */
  word hmbattr;
//  unsigned char msg_attr;              /* Msg attributes. Bits as follows: */
                                       /* 0 : Deleted                      */
                                       /* 1 : Unsent                       */
                                       /* 2 : Netmail                      */
                                       /* 3 : Private                      */
                                       /* 4 : Received                     */
                                       /* 5 : Unmoved outgoing echo        */
                                       /* 6 : Local                        */
                                       /* 7 : RESERVED                     */
//  unsigned char net_attr;              /* Netmail attributes. Bits follow: */
                                       /* 0 : Kill/Sent                    */
                                       /* 1 : Sent                         */
                                       /* 2 : File attach                  */
                                       /* 3 : Crash                        */
                                       /* 4 : Receipt request              */
                                       /* 5 : Audit request                */
                                       /* 6 : Is a return receipt          */
                                       /* 7 : RESERVED                     */
  unsigned char board;                 /* Message board #                  */
  char post_time [6];                  /* Time message was posted          */
  char post_date [9];                  /* Date message was posted          */
  char who_to [36];                    /* Recipient to whom msg is sent    */
  char who_from [36];                  /* Sender who posted message        */
  char subject [73];                   /* Subject line of message          */
} HMB_MSGHDR;

typedef struct {                       /* MSGTXT.BBS structure definition  */
  unsigned char str_len;               /* This string is stored in memory  */
  char str_txt [255];                  /*  in Pascal format to reduce      */
} HMB_TXT;                       /*  overhead, so take care!         */


