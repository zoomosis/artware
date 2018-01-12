/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#define NOVARS
#define NOVER
#define MSGAPI_HANDLERS
#define MSGAPI_PROC

#include <mem.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <stdlib.h>
#include <alloc.h>
#include <fcntl.h>
#include <share.h>
#include <sys\stat.h>

#include "dr.h"
#include "prog.h"
#include "alc.h"
#include "max.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_hmb.h"
#include "api_hmbp.h"
#include "apidebug.h"

static char *RECEIVED = "* Received *";   /* Used for MSGTOIDX */
static char *DELETED  = "* Deleted *";


/* Some HMB specific vars */

typedef struct _hmbapi
{

    int         info,           /* File handles for all files */
                idx,
                toidx,
                hdr,
                txt;

    int         locks;          /* How many locks? Have to lock?      */

    HMB_MSGINFO msginfo;        /* MSGINFO.BBS for this base          */

} HMBAPI;

HMBAPI HMBinfo;

typedef struct
{
    word msgno;
    word recno;

} API_HMBIDX;  // Used to keep an 'index' of the area in memory

typedef struct
{

    unsigned char board;
    API_HMBIDX    *msgs;
    word          array_size;

} HMBdata;     // Private data for area ( == areahandle->apidata )

// HMBdata *debugdata;  // Easier for debugging


static byte *fmpt  =  "FMPT";
static byte *topt  =  "TOPT";
static byte *flags =  "FLAGS";


/* Prototypes */

int   HMBOpenBase(void);
int   HMBCloseBase(void);
int   HMBScanArea(MSG *sq);
void  HMB2SQ(XMSG *msg, HMB_MSGHDR *hmbhdr, MSG *sq);
sword FindRecno(MSG *sq, word msgno);
int   ReadKludges(struct _msgh *msgh);
void  HMBHandleKludges(char *ctrl, XMSG *hdr);
int   HMBCalcTextLen(struct _msgh *msgh);
void  SQ2HMB(MSG *sq, struct _msgh *msgh, XMSG *msg, HMB_MSGHDR *hmbhdr, byte *kludges);
void  write_bbs_file(struct _msgh *msgh);

sword HMBReadInfo  (void);
sword HMBWriteInfo (void);

sword      HMBWriteIdx(sword msgnum, unsigned char board, sword recno);
HMB_MSGIDX *HMBReadEntireIndex(void);

sword HMBReadHeader (sword recno, HMB_MSGHDR *hdr);
sword HMBWriteHeader(sword recno, HMB_MSGHDR *hdr);


/*
     -----------------------------

      Function to open a HMB area.

     -----------------------------
*/


MSG * MSGAPI HMBOpenArea(byte *name, word mode, word type)
{
  MSG * sq = NULL;
  char tempname[120];

  NW(mode);

  if(mi.hudsonpath[0] == '\0')  // Is there a Hudson path? And open files?
     return NULL;

  if((sq=(MSG *)calloc(1, sizeof(MSG)))==NULL)
    return NULL;

  if((sq->apidata = calloc(1, sizeof(HMBdata))) == NULL)
    goto ErrorExit;

  if((sq->api=(struct _apifuncs *)malloc(sizeof(struct _apifuncs)))==NULL)
    goto ErrorExit;

  sq->id=MSGAPI_ID;
  sq->type = type;
  if (type & MSGTYPE_ECHO)
    sq->isecho=TRUE;

  sq->len=sizeof(MSG);
  sq->locked=0;
  sq->cur_msg=0;

  *sq->api=HMB_funcs;
  sq->sz_xmsg=sizeof(XMSG);

  HMBDATA->board = atoi(name);

//  Only for debugging purposes:
//  debugdata = (HMBdata *)sq->apidata;

  /* Reality check on the board number */

  if( (HMBDATA->board < 1) ||
      (HMBDATA->board > 200) )
      goto ErrorExit;


  /* Get inforecord.. (MSGINFO.BBS) */
  /* If we are running in locked mode, (any area locked), we shouldn't
     read it (it might have been updated in memory, and the update
     would then get lost!!!!   */

  if(HMBinfo.locks == 0)
    {
    if(HMBReadInfo() == -1)
       goto ErrorExit;
    }


  if(HMBScanArea(sq) == -1)  // Built in-memory index of area
      goto ErrorExit;

  // All went well, return areahandle

  return (MSG *)sq;

  ErrorExit:        // We only get here if things went wrong

  if(sq->api)     free(sq->api);
  if(sq->apidata) free(sq->apidata);
  if(sq)          free(sq);

  return NULL;

}

/*
      -----------------------------

      Function to close a HMB area.

      -----------------------------
*/

static sword EXPENTRY HMBCloseArea(MSG *sq)
{
  if (InvalidMh(sq))
    return -1;

  if(sq->locked)
     HMBUnlock(sq);

  free(sq->api);
  if(HMBDATA->msgs)
     free(HMBDATA->msgs);
  free((char *)sq->apidata);

  sq->id=0L;
  free(sq);

  return 0;
}


/*
      -------------------------------

      Function to open a HMB message.

      -------------------------------
*/


static MSGH * EXPENTRY HMBOpenMsg(MSG *sq, word mode, dword msgnum)
{
  struct _msgh *msgh;
  HMB_MSGHDR hdr;
  API_HMBIDX *tmpptr;

  if (InvalidMh(sq))
    return NULL;

  if(mode == MOPEN_WRITE) return NULL; // Not supported

  /* Translate special 'message numbers' to 'real' msg numbers */

  switch(msgnum)
    {
    case MSGNUM_CUR:
       msgnum = sq->cur_msg;
       break;

    case MSGNUM_PREV:
       msgnum = sq->cur_msg - 1;
       break;

    case MSGNUM_NEXT:
       msgnum = sq->cur_msg + 1;
       break;

    default:
       break;
    }


  /* Alloc space for handle, and initialize */

  if ((msgh=calloc(1, sizeof(struct _msgh)))==NULL)
    {
      msgapierr=MERR_NOMEM;
      return NULL;
    }

  msgh->sq = sq;
  msgh->id = MSGH_ID;
  msgh->mode = mode;

  /* If we're not creating a new msg, check existence */

  if( (mode != MOPEN_CREATE) || (msgnum != 0L) )
    {
    if((msgh->realno = HMBMsgnToUid(sq, msgnum)) == 0)
       goto ErrorExit;
    }


  if(msgnum != 0) /* Not a new creation, get info */
     {
     if( (msgh->recno = FindRecno(sq, msgnum)) == -1)
         goto ErrorExit;

     if(mode != MOPEN_CREATE)
        {
        if( HMBReadHeader(msgh->recno, &hdr) == -1 )
            goto ErrorExit;

        /* Sanity checks */

        if( (hdr.msgnum != msgh->realno) ||
            (hdr.board  != BNUM + 1) )
            goto ErrorExit;

        /* Check for deleted message */

        if(hdr.msg_attr & HMB_DELETED)
           goto ErrorExit;

        msgh->startblock = hdr.start_block;
        msgh->numblocks  = hdr.num_blocks;

        HMB2SQ((XMSG *)&msgh->hdr, &hdr, sq);

        // Don't read kludges for MOPEN_RW, can't read/write them anyway
        // Or don't need them for reading headers
        if( (mode!=MOPEN_RW) && (mode!=MOPEN_RDHDR) )
          {
          if(ReadKludges(msgh) != 0)
             goto ErrorExit;

          if(HMBCalcTextLen(msgh) == -1)
             goto ErrorExit;
          }
        }
      else /* mode = MOPEN_CREATE, msgnum != 0 */
        {
        /* New or re-created msg, always append TEXT at end */

        msgh->startblock = filelength(HMBinfo.txt) / sizeof(HMB_TXT);
        }

     sq->cur_msg = msgnum;
     }
   else  /* New creation, msgnum = 0 */
     {
     msgnum = ++sq->high_msg;                      // Squish/API number
     sq->num_msg++;
     msgh->realno = ++HMBinfo.msginfo.high_msg;    // Real HMB number
     HMBinfo.msginfo.total_msgs++;
     HMBinfo.msginfo.total_on_board[BNUM]++;

     /* New or re-created msg, always append TEXT at end */

     msgh->startblock = filelength(HMBinfo.txt) / sizeof(HMB_TXT);
     msgh->recno = filelength(HMBinfo.hdr) / sizeof(HMB_MSGHDR);

     /* If we only add one message, we can still make UID conversion work */

     if(sq->high_msg > HMBDATA->array_size) /* Space left to add? */
        {    // We need to realloc, pity, may go wrong

        // Try to realloc, making space for 100 extra (300 bytes extra)
        tmpptr = realloc(HMBDATA->msgs,
                         (HMBDATA->array_size + 100) * sizeof(API_HMBIDX));

        if(tmpptr == NULL) // Well, that failed, let MsgOpen fail then
            goto ErrorExit;

        HMBDATA->array_size += 100;
        HMBDATA->msgs = tmpptr; // Don't forget to use out newly alloced array
        }

     // Add new message to internal index
     HMBDATA->msgs[sq->high_msg-1].msgno = msgh->realno;
     HMBDATA->msgs[sq->high_msg-1].recno = msgh->recno;
     }

  msgh->msgnum = msgnum;
  sq->cur_msg = msgnum;

  return (MSGH *)msgh;

  ErrorExit:            // We only get here if things went wrong

  if(msgh) free(msgh);
  return NULL;
}


/*
      --------------------------------

      Function to close a HMB message.

      --------------------------------
*/


static sword EXPENTRY HMBCloseMsg(MSGH *msgh)
{
  if (InvalidMsgh(msgh))
     return -1;

  if(msgh->mode == MOPEN_CREATE)
    write_bbs_file((struct _msgh *)msgh);

  msgh->id=0L;

  pfree((struct _msgh *)msgh);

  return 0;
}



/*
      --------------------------------

      Function to read a HMB message.

      --------------------------------
*/

static dword EXPENTRY HMBReadMsg(MSGH *msgh, XMSG *msg,
                                  dword offset, dword bytes, byte *text,
                                  dword clen, byte *ctxt)
{
  long start, pos;
  char *thischar;
  unsigned len;
  int moves=0;


  if (InvalidMsgh(msgh))
    return -1;

  /* Copy header info */

  if(msg)
     memmove(msg, (XMSG *) &msgh->hdr, sizeof(XMSG));

  /* Copy kludges */

  if(ctxt && clen)
    {
    if(clen > msgh->clen)
       clen = msgh->clen;

    if(msgh->kludges)
       memmove(ctxt, msgh->kludges, clen);
    else *ctxt = '\0';
    }


  /* Read message text */

  if(text && bytes)
    {
    if(bytes > (msgh->cur_len - offset))
       bytes = (msgh->cur_len - offset);  /* No crazy len */

    start = (long) (((long)msgh->startblock * (long)sizeof(HMB_TXT)) + (long)msgh->rawclen + 1L + (long) offset);
    if(lseek(HMBinfo.txt, start, SEEK_SET) != start)
       return -1;
    if(read(HMBinfo.txt, text, (unsigned) bytes) != (int) bytes)
       return -1;

    start = (long) (msgh->rawclen + 1);

    /* Now we have to remove the length bytes from the Pascal strings.. */

    for(pos=256; pos <= (msgh->cur_len + msgh->rawclen); pos += 256)
       {
       if(pos <= msgh->rawclen)
          continue;         /* This offset was not read; still kludge */

       if( (pos - msgh->rawclen) > bytes) /* Outside of msg space */
          break;

       thischar = text + (pos - msgh->rawclen - 1);
       len = (unsigned) ((byte) thischar[0]);  // How long is this string
       if((pos+len) > (msgh->rawclen+bytes))   // Outside of text read this pass?
           len = (msgh->rawclen + bytes) - pos;
       memmove(thischar-moves, thischar+1, len);
       moves++;
       }

    if(moves)
       *(text+bytes-moves) = '\0'; // If we moved text backwards, we have
                                   // room for this..
    }


  return 1;
}


/*
      --------------------------------

      Function to write a HMB message.

      --------------------------------
*/




static sword EXPENTRY HMBWriteMsg(MSGH *msgh, word append, XMSG *msg, byte *text, dword textlen, dword totlen, dword clen, byte *ctxt)
{
  MSG *sq = msgh->sq;
  char temp[80];
  HMB_MSGHDR hmbhdr;
  HMB_MSGTOIDX toidx = "";
  HMB_TXT *txt;
  static word written, ctrlwritten;
  word blen, curblen;

  if (InvalidMsgh(msgh))
    return -1;

  if (! ctxt)
    clen=0L;

  if (!text)
    textlen=0L;

  if (textlen==0L)
    text=NULL;

  if (clen==0L)
    ctxt=NULL;

  if(!append)
    {
    written = ctrlwritten = 0;

    msgh->totlen = totlen;
    if(!msg) return -1; /* We need a header! */

    /* Convert the header */
    memset(&hmbhdr, '\0', sizeof(HMB_MSGHDR));
    SQ2HMB(sq, msgh, msg, &hmbhdr, ctxt);

    /* Write it out */

    if(HMBWriteHeader(msgh->recno, &hmbhdr) == -1)
       return -1;

    /* Write TOIDX file */
    memmove(toidx, hmbhdr.who_to, sizeof(HMB_MSGTOIDX));

    if(msg)
       {
       if(msg->attr & MSGREAD)    /* Received message! */
          {
          toidx[0] = strlen(RECEIVED);
          memmove(toidx+1, RECEIVED, strlen(RECEIVED));
          }
       }

    if(lseek(HMBinfo.toidx,
             (long) ((long)msgh->recno * sizeof(HMB_MSGTOIDX)),
             SEEK_SET) == -1)
         return -1;
    if(write(HMBinfo.toidx, &toidx, sizeof(HMB_MSGTOIDX)) != sizeof(HMB_MSGTOIDX))
         return -1;

    // If we are in MOPEN_RW mode, we quit here. We can NOT write
    // kludge/body in this mode! We rewrote TOIDX, because one
    // might have 'UNreceived' a message, so the name has to be
    // pute there again, instead of *RECEIVED* ...

    if(msgh->mode == MOPEN_RW) return 0;

    /* Write MSGIDX file  */
    if(HMBWriteIdx(msgh->realno, BNUM+1, msgh->recno) == -1)
       return -1;

    /* Write body */

    txt = calloc(1, sizeof(HMB_TXT));
    if(txt == NULL)
       return -1;

    if(lseek(HMBinfo.txt,
          (long) ((long)msgh->startblock * sizeof(HMB_TXT)),
          SEEK_SET) == -1)
      return -1;


    if(msgh->numblocks == 1) /* Easy come, easy go.. */
       {
       txt->str_len = msgh->rawclen + textlen;
       memmove(txt->str_txt, msgh->kludges, msgh->rawclen);
       memmove(txt->str_txt + msgh->rawclen, text, textlen);

       if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
         return -1;

       msgh->blocks_written++;
       msgh->bytes_written += txt->str_len;

       free(txt);
       }
    else    /* More than one block to write: trouble! */
       {
       while(written < (textlen + msgh->rawclen))
          {
          memset(txt, '\0', sizeof(HMB_TXT));
          blen = curblen = 0;
          if(ctrlwritten < msgh->rawclen)  /* We need to write kludges */
            {
            if((blen = (msgh->rawclen - ctrlwritten)) > 255)
               blen = 255;

            memmove(txt->str_txt, msgh->kludges + ctrlwritten, blen);
            ctrlwritten += blen;
            written += blen;
            }

          if(blen < 255)  /* Space left for more? */
            {
            if( (curblen = (textlen + msgh->rawclen - written)) > (255 - blen))
               curblen = 255 - blen;
            memmove(txt->str_txt + blen, text + (written - msgh->rawclen), curblen);
            written += curblen;
            }

          txt->str_len = (byte) (blen + curblen);
          if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
            return -1;
          msgh->bytes_written += txt->str_len;
          msgh->blocks_written++;
          msgh->lastblocklen = txt->str_len;
          }
       free(txt);
       }
    }
  else     /* we're appending, resync to last block, fill her up, write rest */
    {
    if(textlen)
      {
      txt = calloc(1, sizeof(HMB_TXT));
      if(txt == NULL)
          return -1;

      memset(txt, '\0', sizeof(HMB_TXT));
      written = 0;

      if(lseek(HMBinfo.txt,
          (long) ((long)(msgh->startblock + msgh->blocks_written - 1L) * sizeof(HMB_TXT)),
          SEEK_SET) == -1)
        return -1;

      if(read(HMBinfo.txt,
              txt,
              sizeof(HMB_TXT)) != sizeof(HMB_TXT))
        return -1;

      if(txt->str_len < 255)   /* First fill out this block! */
         {
         lseek(HMBinfo.txt, (long) (-1L * (long) sizeof(HMB_TXT)), SEEK_CUR); /* Back up, to rewrite this block */

         if(txt->str_len + textlen > 255) /* Can't all put it in this block */
            blen = 255 - txt->str_len;
         else
            blen = textlen;

         memmove(txt->str_txt + txt->str_len, text, blen);
         txt->str_len += blen;
         written = blen;
         msgh->bytes_written += blen;
         if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
            return -1;
         }

      while(written < textlen)    /* While there's left to write */
        {
        memset(txt, '\0', sizeof(HMB_TXT));
        if( (textlen-written) > 255 )
           blen = 255;
        else
           blen = textlen - written;
        memmove(txt->str_txt, text + written, blen);
        txt->str_len = blen;
        msgh->bytes_written += blen;
        if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
            return -1;
        msgh->blocks_written++;
        written += blen;
        }

      free(txt);
      }
    }

  return 0;

}



/*
      -------------------------------

      Function to kill a HMB message.

      -------------------------------
*/


static sword EXPENTRY HMBKillMsg(MSG *sq, dword msgnum)
{
  sword retval = -1;
  word  didlock = 0;
  HMB_MSGHDR hdr;
  int recno;
  HMB_MSGTOIDX toidx = "";

  if (InvalidMh(sq))
     return -1;

  if(!sq->locked)
    {
    if(HMBLock(sq) == -1)
       return -1;
    didlock = 1;
    }

  if( (recno = FindRecno(sq, msgnum)) == -1)
     goto Exit;

  /* Set deleted bit in message header */

  if(HMBReadHeader(recno, &hdr) == -1)
       goto Exit;

  hdr.msg_attr |= HMB_DELETED;

  if(HMBWriteHeader(recno, &hdr) == -1)
       goto Exit;

  /* Write IDX file */

  if(HMBWriteIdx(-1, BNUM + 1, recno) == -1)
     goto Exit;

  /* Write TOIDX file */

  toidx[0] = strlen(DELETED);
  memmove(toidx+1, DELETED, strlen(DELETED));

  if(lseek(HMBinfo.toidx, (long) ((long)recno * sizeof(HMB_MSGTOIDX)), SEEK_SET) == -1)
         goto Exit;

  if(write(HMBinfo.toidx, &toidx, sizeof(HMB_MSGTOIDX)) != sizeof(HMB_MSGTOIDX))
         goto Exit;

  HMBinfo.msginfo.total_msgs--;
  HMBinfo.msginfo.total_on_board[BNUM]--;

  // Now we have to remove that entry from our internal index

  memmove(&HMBDATA->msgs[msgnum-1],
           &HMBDATA->msgs[msgnum],
             (sq->num_msg - msgnum) * sizeof(API_HMBIDX) );

  memset(&HMBDATA->msgs[sq->num_msg-1], '\0', sizeof(API_HMBIDX));

  sq->num_msg--;
  sq->high_msg--;

  retval = 0;

  Exit:

  if(didlock)
     HMBUnlock(sq);

  return retval;
}


/*
      ----------------------------

      Function to lock a HMB area.

      ----------------------------
*/

static sword EXPENTRY HMBLock(MSG *sq)
{
  if (InvalidMh(sq))
    return -1;

  /* Don't do anything if already locked */

  if (sq->locked)
    return 0;


  if(mi.haveshare) // If not in Mtask env., no need to do this..
    {
    if(HMBinfo.locks == 0) // First lock. Actually lock file & read info
       {
       if(lock(HMBinfo.info, 407, 1) == -1)
          return -1;

       if(HMBReadInfo() == -1)
          {
          unlock(HMBinfo.info, 407, 1);
          return -1;
          }
       }
    }

  HMBinfo.locks += 1;

  sq->locked=TRUE;

  if(HMBScanArea(sq) == -1)  // Rescan area, so we are up-to-date..
    return -1;

  return 0;
}


/*
      ------------------------------

      Function to unlock a HMB area.

      ------------------------------
*/

/* Undo the above "lock" operation */

static sword EXPENTRY HMBUnlock(MSG *sq)
{
  if (InvalidMh(sq))
    return -1;


  if(HMBinfo.locks == 1) /* Last lock, update */
     {
     if(HMBWriteInfo() == -1)
        return -1;

     /* Flush all handles, so other task see all updated info in Mtask
        environments, and in non-shared env.'s for other file handles
        within the same app (fast scanning separate from API)           */

     flush_handle2(HMBinfo.info);
     flush_handle2(HMBinfo.idx);
     flush_handle2(HMBinfo.toidx);
     flush_handle2(HMBinfo.hdr);
     flush_handle2(HMBinfo.txt);
     }

  if (!sq->locked)
    return -1;

  if(mi.haveshare)
    {
    if(HMBinfo.locks == 1)
       {
       if(unlock(HMBinfo.info, 407, 1) == -1)
          return -1;
       }
    }

  HMBinfo.locks -= 1;

  sq->locked=FALSE;

  return 0;
}



/*
      --------------------------------

      Function to validate a HMB area.

      --------------------------------
*/

sword MSGAPI HMBValidate(byte *name)
{
  NW(name);
  return TRUE;
}


/*
      ----------------------------------------------

      Function to set current position in a HMB msg.

      ----------------------------------------------
*/


static sword EXPENTRY HMBSetCurPos(MSGH *msgh, dword pos)
{
   return msgh->cur_pos = pos;
}


/*
      ----------------------------------------------

      Function to get current position in a HMB msg.

      ----------------------------------------------
*/



static dword EXPENTRY HMBGetCurPos(MSGH *msgh)
{
  return (msgh->cur_pos);
}


/*
      ------------------------------------

      Function to convert # to UID (fake).

      ------------------------------------
*/


static UMSGID EXPENTRY HMBMsgnToUid(MSG *sq, dword msgnum)
{

  if (InvalidMh(sq))
    return 0L;

 // high_msg should be array_size in case of multiple writes to the area
 // 'in one lock', the in memory index won't be up-to-date in that case!

  if( (msgnum < 1) || (msgnum>sq->high_msg) ) 
    return 0L;

  return  ( (UMSGID) HMBDATA->msgs[msgnum-1].msgno );

}


/*
      -----------------------------------------------

      Function to convert UID to existing msg number.

      -----------------------------------------------
*/


static dword EXPENTRY HMBUidToMsgn(MSG *sq, UMSGID umsgid, word type)
{
  word wmsgid;
  word mn;

  if (InvalidMh(sq))
    return 0L;

  msgapierr=MERR_NONE;
  wmsgid=(word)umsgid;

  // Also here, num_msg should be array_size in case of multiple writes
  // 'in one lock' (see MsgnToUid).

  for (mn=0; (dword)mn < sq->num_msg; mn++)
    if (HMBDATA->msgs[mn].msgno==wmsgid ||
        (type==UID_NEXT && HMBDATA->msgs[mn].msgno >= wmsgid) ||
        (type==UID_PREV && HMBDATA->msgs[mn].msgno <= wmsgid &&
        ((dword)(mn+1) >= sq->num_msg || HMBDATA->msgs[mn+1].msgno > wmsgid)))
      return ((dword) mn + 1 );

  msgapierr=MERR_NOENT;
  return 0L;

}

/*
      ---------------------------

      Function to get msg lenght.

      ---------------------------
*/

static dword EXPENTRY HMBGetTextLen(MSGH *msgh)
{
  return msgh->cur_len;
}

/*
      ------------------------------

      Function to get kludge length.

      ------------------------------
*/

static dword EXPENTRY HMBGetCtrlLen(MSGH *msgh)
{
  return msgh->clen;
}

/*
      --------------------------------

      Function to get high water mark.

      --------------------------------
*/

dword EXPENTRY HMBGetHighWater(MSG *sq)
{
  return sq->high_water;
}

/*
      ---------------------------

      Function to set high water.

      ---------------------------
*/

sword EXPENTRY HMBSetHighWater(MSG *sq, dword hwm)
{
  sq->high_water = hwm;
  return 0;
}


/*
      --------------------------------------------------------------------

       Open all the files that are part of the Hudson message base.
       Return 0 on success, -1 on failure.

      --------------------------------------------------------------------
*/


int  HMBOpenBase(void)
{
    char temp[406];

    HMBinfo.info =
       HMBinfo.idx =
          HMBinfo.toidx =
             HMBinfo.hdr =
                HMBinfo.txt = -1;  // Set to -1 first, easy detection of
                                   // open files in case of error..


    sprintf(temp, "%s\\msginfo.bbs", Strip_Trailing(mi.hudsonpath, '\\'));

    if( (HMBinfo.info = open(temp, O_BINARY | O_RDWR | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
       return -1;

    if(filelength(HMBinfo.info)==0L)
       {
       memset(temp, '\0', sizeof(temp));
       if(write(HMBinfo.info, temp, sizeof(temp)) != sizeof(temp))
          goto ErrorExit;
       }

    sprintf(temp, "%s\\msgidx.bbs", mi.hudsonpath);

    if( (HMBinfo.idx = open(temp, O_BINARY | O_RDWR | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    sprintf(temp, "%s\\msgtoidx.bbs", mi.hudsonpath);

    if( (HMBinfo.toidx = open(temp, O_BINARY | O_RDWR | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    sprintf(temp, "%s\\msghdr.bbs", mi.hudsonpath);

    if( (HMBinfo.hdr = open(temp, O_BINARY | O_RDWR | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    sprintf(temp, "%s\\msgtxt.bbs", mi.hudsonpath);

    if( (HMBinfo.txt = open(temp, O_BINARY | O_RDWR | O_CREAT | SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    return 0;  // All was well!

    ErrorExit:  // At least one open failed, close all opened files..

    if(HMBinfo.info  != -1) close(HMBinfo.info );
    if(HMBinfo.idx   != -1) close(HMBinfo.idx  );
    if(HMBinfo.toidx != -1) close(HMBinfo.toidx);
    if(HMBinfo.hdr   != -1) close(HMBinfo.hdr  );
    if(HMBinfo.txt   != -1) close(HMBinfo.txt  );

    return -1;

}

/* ------------------------------------ */
/* Close all Hudson base files          */
/* ------------------------------------ */


int  HMBCloseBase(void)
{

       close(HMBinfo.info );
       close(HMBinfo.idx  );
       close(HMBinfo.toidx);
       close(HMBinfo.hdr  );
       close(HMBinfo.txt  );

       return 0;
}


/* ----------------------------------------- */
/* Scan a message area, build index for area */
/* ----------------------------------------- */

int HMBScanArea(MSG *sq)
{
    word howmany = HMBinfo.msginfo.total_on_board[BNUM];
    int l, current;
    HMB_MSGIDX *idx_array;
    static unsigned idxsize, idxcount;
    int testval = BNUM+1;

    if(HMBinfo.msginfo.total_on_board[BNUM] == 0)
       return 0;

    sq->num_msg = howmany;

    HMBDATA->array_size = howmany + 10;  // Reserve space for 10 extra, so
                                         // we can add msgs without realloc
                                         // immediately.. (could fail..)

    if(HMBDATA->msgs)
       free(HMBDATA->msgs);
    HMBDATA->msgs = calloc(HMBDATA->array_size, sizeof(API_HMBIDX));
    if(HMBDATA->msgs == NULL)
       {
       sq->num_msg = 0;
       return -1;
       }

    idxsize = filelength(HMBinfo.idx);
    idxcount = idxsize / sizeof(HMB_MSGIDX);

    if( (idx_array = HMBReadEntireIndex()) == NULL )
      {
      free(HMBDATA->msgs);
      HMBDATA->msgs = NULL;
      return -1;
      }

    for(l=0, current=0; (l<idxcount) && (current<howmany); l++)
       {
       if( (idx_array[l].board == testval) && (idx_array[l].msg_num != -1) )
          {
          HMBDATA->msgs[current].msgno = idx_array[l].msg_num;
          HMBDATA->msgs[current++].recno = l;
          }
       }

    sq->num_msg = sq->high_msg =  current;

    free(idx_array);

    return 0;

}

/* ------------------------------------------------------------------------ */
/* Find out the 'record number' in the index files for a certain msg number */
/* ------------------------------------------------------------------------ */


sword FindRecno(MSG *sq, word msgno)
{
    unsigned l;

    // num_msg should be array_size in locked/multiple writes!

    if( (msgno < 1) || (msgno>sq->num_msg) )
       return -1;

    msgno--;   /* Make zero based */

    return HMBDATA->msgs[msgno].recno;

}


/* -------------------------------------- */
/* Convert HMB header info to XMSG format */
/* -------------------------------------- */


void  HMB2SQ(XMSG *msg, HMB_MSGHDR *hmbhdr, MSG *sq)
{
    word m, d, y, h;

    msg->orig.zone = hmbhdr->orig_zone;
    msg->orig.node = hmbhdr->orig_node;
    msg->orig.net  = hmbhdr->orig_net ;

    msg->dest.zone = hmbhdr->dest_zone;
    msg->dest.node = hmbhdr->dest_node;
    msg->dest.net  = hmbhdr->dest_net ;

    if((unsigned char) hmbhdr->who_from[0] > (unsigned char) 35)
        hmbhdr->who_from[0] = (unsigned char) 35;
    memmove(&msg->from, hmbhdr->who_from+1, (size_t) hmbhdr->who_from[0]);

    if((unsigned char) hmbhdr->who_to[0] > (unsigned char) 35)
        hmbhdr->who_to[0] = (unsigned char) 35;
    memmove(&msg->to  , hmbhdr->who_to  +1, (size_t) hmbhdr->who_to[0]  );

    if((unsigned char) hmbhdr->subject[0] > (unsigned char) 71)
        hmbhdr->subject[0] = (unsigned char) 71;
    memmove(&msg->subj, hmbhdr->subject +1, (size_t) hmbhdr->subject[0] );

    msg->replyto = hmbhdr->prev_reply;
    msg->replies[0] = hmbhdr->next_reply;

    if(hmbhdr->msg_attr & HMB_PRIVATE)  msg->attr |= MSGPRIVATE;
    if(hmbhdr->msg_attr & HMB_RECEIVED) msg->attr |= MSGREAD;
    if(hmbhdr->msg_attr & HMB_LOCAL)    msg->attr |= MSGLOCAL;

    if( !(hmbhdr->msg_attr & HMB_UNSENT)  &&  // Not if unsent (netmail)
        !(hmbhdr->msg_attr & HMB_UNMOVED) &&  // Not if unmoved (echo)
         (hmbhdr->msg_attr & HMB_LOCAL)   &&  // Only if locally made
        !(sq->type & MSGTYPE_LOCAL) )         // Not if local area
        msg->attr |= MSGSENT;

    if(hmbhdr->net_attr & HMB_KILL)     msg->attr |= MSGKILL;
    if(hmbhdr->net_attr & HMB_SENT)     msg->attr |= MSGSENT;
    if(hmbhdr->net_attr & HMB_FILE)     msg->attr |= MSGFILE;
    if(hmbhdr->net_attr & HMB_CRASH)    msg->attr |= MSGCRASH;
    if(hmbhdr->net_attr & HMB_RECEIPT)  msg->attr |= MSGRRQ;
    if(hmbhdr->net_attr & HMB_RETURN)   msg->attr |= MSGCPT;

    if(hmbhdr->post_date[0] != 0)
       {
       sscanf(hmbhdr->post_date+1, "%2hd-%2hd-%2hd", &m, &d, &y);

       msg->date_written.date.mo = m;
       msg->date_written.date.da = d;
       msg->date_written.date.yr = y - 80;
       }

    if(hmbhdr->post_time[0] != 0)
       {
       sscanf(hmbhdr->post_time+1, "%2hd:%2hd", &h, &m);

       msg->date_written.time.mm = m;
       msg->date_written.time.hh = h;
       }


}


/* --------------------------------------------------------------- */
/* Extract the kludges from msg text, calc length, store in buffer */
/* Return 0 if all is well                                         */
/* --------------------------------------------------------------- */


int ReadKludges(struct _msgh *msgh)
{
    int howmany, error;
    byte *end;
    char *ctrlbuf;
    unsigned rawlen;

  // Read max of 2 blocks (512 bytes) for control info (kludges etc).

    howmany = (msgh->numblocks < 2) ? msgh->numblocks : 2;

    if(howmany == 0)  // No text, so no kludges..
       return 0;

    if(lseek(HMBinfo.txt, (long) ((long)msgh->startblock * sizeof(HMB_TXT)), SEEK_SET) !=
                          (long) ((long)msgh->startblock * sizeof(HMB_TXT)) )
        return -1;

    if(read(HMBinfo.txt, msgh->kludges, (howmany * sizeof(HMB_TXT)) ) !=
                                        (howmany * sizeof(HMB_TXT)) )
        return -1;


    if(howmany == 2)
       memmove(msgh->kludges+256, msgh->kludges+257, 255); /* Pascal strings! */

    ctrlbuf = (byte *) CopyToControlBuf(msgh->kludges + 1,
                                        &end,
                                        (unsigned *)NULL);

    msgh->rawclen = (dword) (end - (msgh->kludges+1));
    if(msgh->rawclen > 255)    // More than 1 Pascal string, discount
       msgh->rawclen += 1;

    memset(msgh->kludges, '\0', sizeof(msgh->kludges));
    if(ctrlbuf)
       {
       strcpy(msgh->kludges, ctrlbuf);
       free(ctrlbuf);
       }

    HMBHandleKludges(msgh->kludges, &msgh->hdr);

    msgh->clen = strlen(msgh->kludges) + 1;

    return 0;
}


/* ------------------------------------- */
/*  Extract FMPT, TOPT info from kludges */
/* ------------------------------------- */


void HMBHandleKludges(char *ctrl, XMSG *hdr)
{
  char *s;

  /* Handle the FMPT kludge */

  if ((s=GetCtrlToken(ctrl,fmpt)) != NULL)
  {
    hdr->orig.point=atoi(s+5);
    pfree(s);

    RemoveFromCtrl(ctrl,fmpt);
  }


  /* Handle TOPT too */

  if ((s=GetCtrlToken(ctrl,topt)) != NULL)
  {
    hdr->dest.point=atoi(s+5);
    pfree(s);

    RemoveFromCtrl(ctrl,topt);
  }


  if ((s=GetCtrlToken(ctrl,flags)) != NULL)
  {
    if(strstr(s+6, "HLD") != NULL)
       hdr->attr |= MSGHOLD;

    if(strstr(s+6, "FRQ") != NULL)
       hdr->attr |= MSGFRQ;

    if(strstr(s+6, "A/S") != NULL)
       hdr->attr |= ADD_AS;

    if(strstr(s+6, "DIR") != NULL)
       hdr->attr |= ADD_DIR;

    if(strstr(s+6, "IMM") != NULL)
       hdr->attr |= ADD_IMM;

    if(strstr(s+6, "KFS") != NULL)
       hdr->attr |= ADD_KFS;

    if(strstr(s+6, "TFS") != NULL)
       hdr->attr |= ADD_TFS;

    if(strstr(s+6, "LOK") != NULL)
       hdr->attr |= ADD_LOK;

    if(strstr(s+6, "CFM") != NULL)
       hdr->attr |= ADD_CFM;

    pfree(s);

    RemoveFromCtrl(ctrl,flags);
  }

}

/* ------------------------------------------------------------------- */
/* Calculate the length of the message text (minus kludges and slack!) */
/* ------------------------------------------------------------------- */

int HMBCalcTextLen(struct _msgh *msgh)
{
  long which;

  if(msgh->numblocks == 0)
     return 0;

  which = (long) msgh->startblock + (long) msgh->numblocks - 1L;

  if(lseek(HMBinfo.txt, (long) (which * sizeof(HMB_TXT)), SEEK_SET) !=
                        (long) (which * sizeof(HMB_TXT)) )
        return -1;

  if(read(HMBinfo.txt, &msgh->lastblocklen, (sizeof(byte)) ) !=
                                            (sizeof(byte)) )
        return -1;

  /* Underneath * 256 (not 255) cause we NEED the extra space for the */
  /* length byte of Pascal strings, when we read in the text (before  */
  /* removing the length bytes..                                      */

  msgh->cur_len = (msgh->numblocks * 256) -    /* Pascal string blocks */
                   msgh->rawclen -             /* Kludges              */
                   (255 - msgh->lastblocklen)  /* Slack                */
                   -1;

  return 0;

}

/* ------------------------------------------------------ */
/* Convert XMSG header to HMB header, add to control info */
/* ------------------------------------------------------ */


void SQ2HMB(MSG *sq, struct _msgh *msgh, XMSG *msg, HMB_MSGHDR *hmbhdr, byte *kludges)
{
    char temp[80];
    dword addmask = ADD_AS | ADD_DIR | ADD_IMM | ADD_KFS | ADD_TFS | ADD_CFM | ADD_LOK | MSGFRQ | MSGHOLD;
    byte *tempkludges;

    memset(msgh->kludges, '\0', sizeof(msgh->kludges));
    if(kludges)
       strcpy(msgh->kludges, kludges);

    hmbhdr->start_block = msgh->startblock;
    hmbhdr->msgnum = msgh->realno;

    hmbhdr->dest_net  = msg->dest.net;
    hmbhdr->dest_node = msg->dest.node;
    hmbhdr->dest_zone = msg->dest.zone;

    hmbhdr->orig_zone = msg->orig.zone;
    hmbhdr->orig_node = msg->orig.node;
    hmbhdr->orig_net  = msg->orig.net;

    hmbhdr->board = BNUM + 1;

    hmbhdr->who_to[0] = (byte) strlen(msg->to);
    memmove(hmbhdr->who_to+1, msg->to, hmbhdr->who_to[0]);

    hmbhdr->who_from[0] = (byte) strlen(msg->from);
    memmove(hmbhdr->who_from+1, msg->from, hmbhdr->who_from[0]);

    hmbhdr->subject[0] = (byte) strlen(msg->subj);
    memmove(hmbhdr->subject+1, msg->subj, hmbhdr->subject[0]);

    hmbhdr->prev_reply = msg->replyto ;
    hmbhdr->next_reply = msg->replies[0];

    if(msg->attr & MSGPRIVATE)   hmbhdr->msg_attr |= HMB_PRIVATE;
    if(msg->attr & MSGREAD)      hmbhdr->msg_attr |= HMB_RECEIVED;
    if(msg->attr & MSGLOCAL)     hmbhdr->msg_attr |= HMB_LOCAL;

    if( !(msg->attr & MSGSENT) && (msg->attr & MSGLOCAL) )
       {
       if(sq->type & MSGTYPE_ECHO)
          hmbhdr->msg_attr |= HMB_UNMOVED;
       else if(sq->type & MSGTYPE_NET)
          hmbhdr->msg_attr |= HMB_UNSENT;
       }

    if(sq->type & MSGTYPE_NET)
       hmbhdr->msg_attr |= HMB_NETMAIL;

    if(msg->attr & MSGKILL)   hmbhdr->net_attr |= HMB_KILL;
    if(msg->attr & MSGSENT)   hmbhdr->net_attr |= HMB_SENT;
    if(msg->attr & MSGFILE)   hmbhdr->net_attr |= HMB_FILE;
    if(msg->attr & MSGCRASH)  hmbhdr->net_attr |= HMB_CRASH;
    if(msg->attr & MSGRRQ)    hmbhdr->net_attr |= HMB_RECEIPT;
    if(msg->attr & MSGCPT)    hmbhdr->net_attr |= HMB_RETURN;


    sprintf(temp, "%02d-%02d-%02d", msg->date_written.date.mo,
                                    msg->date_written.date.da,
                                    msg->date_written.date.yr + 80);

    hmbhdr->post_date[0] = (byte) 8;
    memmove(hmbhdr->post_date+1, temp, 8);


    sprintf(temp, "%02d:%02d", msg->date_written.time.hh,
                               msg->date_written.time.mm);

    hmbhdr->post_time[0] = (byte) 5;
    memmove(hmbhdr->post_time+1, temp, 5);


    // Don't do all this for MOPEN_RW, can't write kludges anyway..
    if(msgh->mode != MOPEN_RW)
      {
      if( (msg->orig.point != 0) && (sq->type & MSGTYPE_NET) )
         {
         sprintf(temp, "\01FMPT %d", msg->orig.point);
         strcat(msgh->kludges, temp);
         }

      if( (msg->dest.point != 0) && (sq->type & MSGTYPE_NET) )
         {
         sprintf(temp, "\01TOPT %d", msg->dest.point);
         strcat(msgh->kludges, temp);
         }


      if((msg->attr & addmask) && (sq->type & MSGTYPE_NET) )
        {
        strcpy(temp,"\x01""FLAGS");

        if(msg->attr & MSGFRQ)
            strcat(temp, " FRQ");
         if(msg->attr & MSGHOLD)
            strcat(temp, " HLD");
        if(msg->attr & ADD_DIR)
            strcat(temp, " DIR");
        if(msg->attr & ADD_IMM)
            strcat(temp, " IMM");
        if(msg->attr & ADD_AS)
            strcat(temp, " A/S");
        if(msg->attr & ADD_KFS)
            strcat(temp, " KFS");
        if(msg->attr & ADD_TFS)
            strcat(temp, " TFS");
        if(msg->attr & ADD_LOK)
            strcat(temp, " LOK");
        if(msg->attr & ADD_CFM)
            strcat(temp, " CFM");

        strcat(msgh->kludges, temp);
        }

      if(msgh->kludges[0] != '\0')
         {
         tempkludges = CvtCtrlToKludge(msgh->kludges);
         strncpy(msgh->kludges, tempkludges, 512);
         free(tempkludges);
         }

      msgh->rawclen = strlen(msgh->kludges);

      // This WAS %256 and /256!! Stupid, stupid!!!!
      if( ((msgh->totlen + msgh->rawclen) % 255) == 0)
         msgh->numblocks = (msgh->totlen + msgh->rawclen) / 255;
      else
         msgh->numblocks = ((msgh->totlen + msgh->rawclen) / 255)+1;
      }

    // In case of MOPEN_RW, this info is OK already (read from header!)

    hmbhdr->num_blocks = msgh->numblocks;

}


/* ---------------------------------------------------------------- */
/* We write ECHOMAIL.BBS and NETMAIL.BBS here, because it need info */
/* about the record number in the Hudson base, info which is not    */
/* available at a higher level (app using the API)                  */
/* ---------------------------------------------------------------- */


void write_bbs_file(struct _msgh *msgh)
{
   int tosslog;
   char temp[120];
   sword data = (sword)msgh->recno;

   if(msgh->sq->type & MSGTYPE_LOCAL)
      return;

   if(msgh->sq->type & MSGTYPE_ECHO)
      sprintf(temp, "%s\\echomail.bbs", mi.hudsonpath);
   else
      sprintf(temp, "%s\\netmail.bbs", mi.hudsonpath);

   if( (tosslog = open(temp, O_BINARY | O_CREAT | SH_DENYWR | O_WRONLY, S_IWRITE | S_IREAD)) == -1)
         return;

   lseek(tosslog, 0L, SEEK_END);
   write(tosslog, &data, sizeof(sword));
   close(tosslog);

}


/* ------------------------------------------- */
/*  Function to read MSGINFO.BBS into memory.  */
/*  Returns 0 on success, -1 on failure.       */
/* ------------------------------------------- */


sword HMBReadInfo(void)
{

  if(lseek(HMBinfo.info, 0L, SEEK_SET) == -1)
     return -1;

  if(read(HMBinfo.info, &HMBinfo.msginfo, sizeof(HMB_MSGINFO))
                                                != sizeof(HMB_MSGINFO))
     return -1;

   return 0;

}

/* ------------------------------------------ */
/*  Function to write MSGINFO.BBS to disk.    */
/*  Returns 0 on success, -1 on failure.      */
/* ------------------------------------------ */


sword HMBWriteInfo(void)
{

  if(lseek(HMBinfo.info, 0L, SEEK_SET) == -1)
     return -1;

  if(write(HMBinfo.info, &HMBinfo.msginfo, sizeof(HMB_MSGINFO))
                                                != sizeof(HMB_MSGINFO))
     return -1;

   return 0;

}


/* --------------------------------------------- */
/*  Function to write an index entry to disk.    */
/*  Returns 0 on success, -1 on failure.         */
/* --------------------------------------------- */

sword HMBWriteIdx(sword msgnum, unsigned char board, sword recno)
{
    HMB_MSGIDX idx;

    idx.msg_num = msgnum;
    idx.board   = board;

    if(lseek(HMBinfo.idx,
             (long) ((long)recno * sizeof(HMB_MSGIDX)),
             SEEK_SET) == -1)
         return -1;

    if(write(HMBinfo.idx, &idx, sizeof(HMB_MSGIDX)) != sizeof(HMB_MSGIDX))
         return -1;

    return 0;

}

/* ---------------------------------------------- */
/*  Function to read the entire index in memory.  */
/*  Returns a pointer to the index on succes,     */
/*  and NULL on failure (no mem, or read error)   */
/* ---------------------------------------------- */

HMB_MSGIDX *HMBReadEntireIndex(void)
{
    unsigned idxsize = filelength(HMBinfo.idx);
    HMB_MSGIDX *idxptr;


    if( (idxptr = malloc(idxsize)) == NULL )
       return NULL;

    if((lseek(HMBinfo.idx, 0L, SEEK_SET) == -1) ||
       (read(HMBinfo.idx, idxptr, idxsize) != idxsize) )
       {
       free(idxptr);
       return NULL;
       }

    return idxptr;
}


/* -------------------------------------- */
/*  Function to read a header in memory.  */
/*  Returns 0 on succes, -1 on failure    */
/* -------------------------------------- */

sword HMBReadHeader(sword recno, HMB_MSGHDR *hdr)
{

  if(lseek(HMBinfo.hdr, (long) ((long)recno * sizeof(HMB_MSGHDR)), SEEK_SET) !=
                        (long) ((long)recno * sizeof(HMB_MSGHDR)))
       return -1;

  if(read(HMBinfo.hdr, hdr, sizeof(HMB_MSGHDR)) != sizeof(HMB_MSGHDR))
       return -1;

   return 0;

}

/* ------------------------------------ */
/*  Function to write header to disk.   */
/*  Returns 0 on succes, -1 on failure  */
/* ------------------------------------ */

sword HMBWriteHeader(sword recno, HMB_MSGHDR *hdr)
{

  if(lseek(HMBinfo.hdr, (long) ((long)recno * sizeof(HMB_MSGHDR)), SEEK_SET) !=
                        (long) ((long)recno * sizeof(HMB_MSGHDR)))
       return -1;

  if(write(HMBinfo.hdr, hdr, sizeof(HMB_MSGHDR)) != sizeof(HMB_MSGHDR))
       return -1;

   return 0;

}

