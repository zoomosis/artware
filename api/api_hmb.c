/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#define MSGAPI_HANDLERS

#ifndef __GNUC__
#include <mem.h>
#include <io.h>
#include <dos.h>
#include <share.h>
#else
#include <unistd.h>
#endif

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

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

extern char *fmpt;
extern char *topt;
extern char *flags;

/* Some HMB specific vars */

typedef struct _hmbapi
{

    int         info,           /* File handles for all files */
                idx,
                toidx,
                hdr,
                txt;

    int         locks;          /* How many locks? Have to lock?      */
    int         opens;

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


typedef struct
{

   dword api_attr;
   word  hmb_attr;

}  ATTR_AH;

#define HMBATTRNUM 11

ATTR_AH Attr_AH_Table[HMBATTRNUM] = {

    {aPRIVATE, HMB_PRIVATE},
    {aREAD,    HMB_RECEIVED},
    {aLOCAL,   HMB_LOCAL},
    {aKILL,    HMB_KILL},
    {aSENT,    HMB_SENT},
    {aFILE,    HMB_FILE},
    {aCRASH,   HMB_CRASH},
    {aRRQ,     HMB_RECEIPT},
    {aCPT,     HMB_RETURN},
    {aDEL,     HMB_DELETED},
    {aARQ,     HMB_AUDIT}

};


/* Prototypes */

int   HMBOpenBase(void);
int   HMBCloseBase(void);
int   near HMBScanArea(MSG *sq);
//void  near HMB2SQ(XMSG *msg, HMB_MSGHDR *hmbhdr, MSG *sq);
sword near FindRecno(MSG *sq, word msgno);
int   near ReadKludges(struct _msgh *msgh);
void  near HMBHandleKludges(char *ctrl, MIS *mis);
int   near HMBCalcTextLen(struct _msgh *msgh);
//void  near SQ2HMB(MSG *sq, struct _msgh *msgh, XMSG *msg, HMB_MSGHDR *hmbhdr, byte *kludges);
void  near write_bbs_file(struct _msgh *msgh);

sword near HMBReadInfo  (void);
sword near HMBWriteInfo (void);

sword        near HMBWriteIdx(sword msgnum, unsigned char board, sword recno);
HMB_MSGIDX * HMBReadEntireIndex(void);

sword near HMBReadHeader (sword recno, HMB_MSGHDR *hdr);
sword near HMBWriteHeader(sword recno, HMB_MSGHDR *hdr);

void HMB2MIS(MIS *mis, HMB_MSGHDR *hmbhdr, MSG *sq);
void MIS2HMB(MSG *sq, struct _msgh *msgh, MIS *mis, HMB_MSGHDR *hmbhdr, byte *kludges);
/*
     -----------------------------

      Function to open a HMB area.

     -----------------------------
*/


MSG * HMBOpenArea(byte *name, word mode, word type)
{
  MSG * sq = NULL;

  msgapierr=MERR_NONE;

  NW(mode);

  if(mi.hudsonpath[0] == '\0')  // Is there a Hudson path? And open files?
    {
    msgapierr=MERR_NOHMB;
    return NULL;
    }

  if(HMBOpenBase() == -1)
    return NULL;

  if((sq=CreateAreaHandle(type))==NULL)
    return NULL;

  if((sq->apidata = calloc(1, sizeof(HMBdata))) == NULL)
    {
    msgapierr=MERR_NOMEM;
    goto ErrorExit;
    }

  *sq->api=HMB_funcs;

  HMBDATA->board = atoi(name);

//  Only for debugging purposes:
//  debugdata = (HMBdata *)sq->apidata;

  /* Reality check on the board number */

  if( (HMBDATA->board < 1) ||
      (HMBDATA->board > 200) )
      {
      msgapierr=MERR_NOENT;
      goto ErrorExit;
      }


  /* Get inforecord.. (MSGINFO.BBS) */
  /* If we are running in locked mode, (any area locked), we shouldn't
     read it (it might have been updated in memory, and the update
     would then get lost!!!!   */

  if(HMBinfo.locks == 0)
    {
    if(HMBReadInfo() == -1)
       goto ErrorExit;
    }

  if(HMBScanArea(sq) == -1)  // Build in-memory index of area
      goto ErrorExit;

  // All went well, return areahandle..

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

sword HMBCloseArea(MSG *sq)
{
  msgapierr=MERR_NONE;

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

  HMBCloseBase();

  return 0;
}


/*
      -------------------------------

      Function to open a HMB message.

      -------------------------------
*/


MSGH *  HMBOpenMsg(MSG *sq, word mode, dword msgnum)
{
  struct _msgh *msgh;
  HMB_MSGHDR hdr;
  API_HMBIDX *tmpptr;

  msgapierr=MERR_NONE;

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

        if(hdr.msgnum != msgh->realno)
          {
          msgapierr=MERR_MSGNO;
          goto ErrorExit;
          }

        if(hdr.board != BNUM + 1)
          {
          msgapierr=MERR_BOARD;
          goto ErrorExit;
          }

        /* Check for deleted message */

        if(hdr.hmbattr & HMB_DELETED)
          {
          msgapierr=MERR_DELETED;
          goto ErrorExit;
          }

        msgh->startblock = hdr.start_block;
        msgh->numblocks  = hdr.num_blocks;

        HMB2MIS(&msgh->mis, &hdr, sq);

        // Don't read kludges for MOPEN_RW, can't read/write them anyway
        // Or don't need them for reading headers
        if( (mode!=MOPEN_RW) && (mode!=MOPEN_RDHDR) )
          {
          if(ReadKludges(msgh) != 0)
             goto ErrorExit;

          // Conversion of attach/request is done here. We needed the
          // kludges (for FRQ attribute!) first..

          if(msgh->mis.attr1 & (aFRQ|aURQ))  // extract requested files from subject.
            Extract_Requests(&msgh->mis);

          if(msgh->mis.attr1 & aFILE)      // extract attached files from subject.
            Extract_Attaches(&msgh->mis);

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
           {
           msgapierr=MERR_NOMEM;
           sq->high_msg--;
           sq->num_msg--;
           HMBinfo.msginfo.high_msg--;
           HMBinfo.msginfo.total_msgs--;
           HMBinfo.msginfo.total_on_board[BNUM]--;
           goto ErrorExit;
           }

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


sword  HMBCloseMsg(MSGH *msgh)
{
  msgapierr=MERR_NONE;

  if (InvalidMsgh(msgh))
     return -1;

  if(msgh->mode == MOPEN_CREATE)
    {
    // Now we need to write trailing stuff like SB, Path etc.
    HMBWriteMsg(msgh, 1, NULL, msgh->trail, msgh->trailsize, msgh->trailsize, 0L, NULL);
    write_bbs_file((struct _msgh *)msgh);
    }

  msgh->id=0L;

  FreeMIS(&msgh->mis);

  if(msgh->trail)
     free(msgh->trail);
  free((struct _msgh *)msgh);

  return 0;
}



/*
      --------------------------------

      Function to read a HMB message.

      --------------------------------
*/

dword  HMBReadMsg(MSGH *msgh, MIS *mis,
                                  dword offset, dword bytes, byte *text,
                                  dword clen, byte *ctxt)
{
  long start, pos;
  char *thischar;
  unsigned len;
  int moves=0;
  int startlbytes=0;   // How many Pascal length bytes to be counted
  dword bytesread=0L;

  msgapierr=MERR_NONE;

  if (InvalidMsgh(msgh))
    return -1;

  /* Copy header info */

  if(mis)
     CopyMIS(&msgh->mis, mis);

  /* Copy kludges */

  if(ctxt && clen)
    {
    if(clen > msgh->clen)
       clen = msgh->clen;

    if(msgh->kludges)
       memcpy(ctxt, msgh->kludges, clen);
    else *ctxt = '\0';
    }


  /* Read message text */

  if(text && bytes)
    {
    if(bytes > (msgh->cur_len - offset))
       bytes = (msgh->cur_len - offset);  /* No crazy len */

    startlbytes = (msgh->rawclen + offset) / 256;
//    endlbytes = (msgh->rawclen + offset + bytes) / 256;
    startlbytes = 0;

    start = (long) (((long)msgh->startblock * (long)sizeof(HMB_TXT)) + (long)msgh->rawclen + 1L + (long) startlbytes + (long) offset);

    if(lseek(HMBinfo.txt, start, SEEK_SET) != start)
      {
      msgapierr=MERR_SEEK;
      return -1;
      }

    if(read(HMBinfo.txt, text, (unsigned) bytes) != (int) bytes)
       {
       msgapierr=MERR_READ;
       return -1;
       }

    bytesread += bytes;

    start = (long) (msgh->rawclen + 1);

    /* Now we have to remove the length bytes from the Pascal strings.. */

    for(pos=256; pos <= (msgh->cur_len + msgh->rawclen); pos += 256)
       {
       if(pos <= (msgh->rawclen + offset))
          continue;         /* This offset was not read; still kludge */

       if( (pos - msgh->rawclen - offset) > bytes) /* Outside of msg space */
          break;

       thischar = text + (pos - msgh->rawclen - offset - 1);
       len = (unsigned) ((byte) thischar[0]);  // How long is this string
       if((pos+len) > (msgh->rawclen+bytes+offset))   // Outside of text read this pass?
           len = (msgh->rawclen + offset + bytes) - pos;
       memcpy(thischar-moves, thischar+1, len);
       moves++;
       }

    if(moves)
       *(text+bytes-moves) = '\0'; // If we moved text backwards, we have
                                   // room for this..
    }

  return (bytesread - moves);  // Return exactly what we read.
}

/*
      --------------------------------

      Function to write a HMB message.

      --------------------------------
*/

sword  HMBWriteMsg(MSGH *msgh, word append, MIS *mis, byte *text, dword textlen, dword totlen, dword clen, byte *ctxt)
{
  MSG *sq = msgh->sq;
  HMB_MSGHDR hmbhdr;
  HMB_MSGTOIDX toidx = "";
  HMB_TXT *txt;
  static dword written, ctrlwritten;
  dword blen, curblen;

  msgapierr=MERR_NONE;

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
    if(!mis) return -1; /* We need a header! */

    /* Convert the header */
    memset(&hmbhdr, '\0', sizeof(HMB_MSGHDR));
    MIS2HMB(sq, msgh, mis, &hmbhdr, ctxt);

    /* Write it out */

    if(HMBWriteHeader(msgh->recno, &hmbhdr) == -1)
       return -1;

    /* Write TOIDX file */
    memcpy(toidx, hmbhdr.who_to, sizeof(HMB_MSGTOIDX));

    if(mis)
       {
       if(mis->attr1 & aREAD)    /* Received message! */
          {
          toidx[0] = strlen(RECEIVED);
          memcpy(toidx+1, RECEIVED, strlen(RECEIVED));
          }
       }

    if(lseek(HMBinfo.toidx,
             (long) ((long)msgh->recno * sizeof(HMB_MSGTOIDX)),
             SEEK_SET) == -1)
         {
         msgapierr=MERR_SEEK;
         return -1;
         }

    if(write(HMBinfo.toidx, &toidx, sizeof(HMB_MSGTOIDX)) != sizeof(HMB_MSGTOIDX))
         {
         msgapierr=MERR_WRITE;
         return -1;
         }

    // If we are in MOPEN_RW mode, we quit here. We can NOT write
    // kludge/body in this mode! We rewrote TOIDX, because one
    // might have 'UNreceived' a message, so the name has to be
    // put there again, instead of *RECEIVED* ...

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
      {
      msgapierr=MERR_SEEK;
      return -1;
      }


    if(msgh->numblocks == 1) /* Easy come, easy go.. */
       {
       txt->str_len = msgh->rawclen + textlen;
       memcpy(txt->str_txt, msgh->kludges, msgh->rawclen);
       memcpy(txt->str_txt + msgh->rawclen, text, textlen);

       if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
         {
         msgapierr=MERR_WRITE;
         return -1;
         }

       msgh->blocks_written++;
       msgh->bytes_written += txt->str_len;

       free(txt);
       }
    else    /* More than one block to write: trouble! */
       {
       while(written < (textlen + msgh->rawclen))   // While there's left to write..
          {
          memset(txt, '\0', sizeof(HMB_TXT));
          blen = curblen = 0;
          if(ctrlwritten < msgh->rawclen)  /* We still need to write kludges */
            {
            if((blen = (msgh->rawclen - ctrlwritten)) > 255)
               blen = 255;

            memcpy(txt->str_txt, msgh->kludges + ctrlwritten, blen);
            ctrlwritten += blen;
            written += blen;
            }

          if(blen < 255)  /* Space left for more in this block? */
            {
            if( (curblen = (textlen + msgh->rawclen - written)) > (255 - blen))
               curblen = 255 - blen;
            memcpy(txt->str_txt + blen, text + (written - msgh->rawclen), curblen);
            written += curblen;
            }

          txt->str_len = (byte) (blen + curblen);
          if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
            {
            msgapierr=MERR_WRITE;
            return -1;
            }
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

      /* Seek back to last block till now, and read it */

      if(lseek(HMBinfo.txt,
          (long) ((long)(msgh->startblock + msgh->blocks_written - 1L) * sizeof(HMB_TXT)),
          SEEK_SET) == -1)
        {
        msgapierr=MERR_SEEK;
        return -1;
        }

      if(read(HMBinfo.txt,
              txt,
              sizeof(HMB_TXT)) != sizeof(HMB_TXT))
        {
        msgapierr=MERR_READ;
        return -1;
        }

      if(txt->str_len < 255)   /* First fill out this block! */
         {
         lseek(HMBinfo.txt, (long) (-1L * (long) sizeof(HMB_TXT)), SEEK_CUR); /* Back up, to rewrite this block */

         if(txt->str_len + textlen > 255) /* Can't all put it in this block */
            blen = 255 - txt->str_len;
         else
            blen = textlen;

         memcpy(txt->str_txt + txt->str_len, text, blen);
         txt->str_len += blen;
         written = blen;
         msgh->bytes_written += blen;
         if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
            {
            msgapierr=MERR_WRITE;
            return -1;
            }
         }

      /* We filled out the last block of previous writes, now add rest in new blocks */

      while(written < textlen)    /* While there's left to write */
        {
        memset(txt, '\0', sizeof(HMB_TXT));
        if( (textlen-written) > 255 )
           blen = 255;
        else
           blen = textlen - written;
        memcpy(txt->str_txt, text + written, blen);
        txt->str_len = blen;
        msgh->bytes_written += blen;
        if(write(HMBinfo.txt, txt, sizeof(HMB_TXT)) != sizeof(HMB_TXT))
            {
            msgapierr=MERR_WRITE;
            return -1;
            }
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


sword  HMBKillMsg(MSG *sq, dword msgnum)
{
  sword retval = -1;
  word  didlock = 0;
  HMB_MSGHDR hdr;
  int recno;
  HMB_MSGTOIDX toidx = "";

  msgapierr=MERR_NONE;

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

  hdr.hmbattr |= HMB_DELETED;

  if(HMBWriteHeader(recno, &hdr) == -1)
       goto Exit;

  /* Write IDX file */

  if(HMBWriteIdx(-1, BNUM + 1, recno) == -1)
     goto Exit;

  /* Write TOIDX file */

  toidx[0] = strlen(DELETED);
  memcpy(toidx+1, DELETED, strlen(DELETED));

  if(lseek(HMBinfo.toidx, (long) ((long)recno * sizeof(HMB_MSGTOIDX)), SEEK_SET) == -1)
      {
      msgapierr=MERR_SEEK;
      goto Exit;
      }

  if(write(HMBinfo.toidx, &toidx, sizeof(HMB_MSGTOIDX)) != sizeof(HMB_MSGTOIDX))
     {
     msgapierr=MERR_WRITE;
     goto Exit;
     }

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

sword  HMBLock(MSG *sq)
{
  int result, i;
  char temp[120];

  msgapierr=MERR_NONE;

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
          {
          sprintf(temp, "%s/mbunlock.now", mi.hudsonpath);
          if(creat_touch(temp) == -1)
            {
            msgapierr=MERR_NOLOCK;
            return -1;
            }

          for(i=0; i<12; i++)  // we retry 12 times (12 secs)
            {
            sleep(1);
            if( (result=lock(HMBinfo.info, 407, 1)) != -1)
               break;
            }

          if(result == -1)
            {
            msgapierr=MERR_NOLOCK;
            return -1;
            }
          // else we go on below..
          }
       
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

sword  HMBUnlock(MSG *sq)
{
  char temp[120];

  msgapierr=MERR_NONE;

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
          {
          msgapierr=MERR_NOUNLOCK;
          return -1;
          }

       sprintf(temp, "%s/mbunlock.now", mi.hudsonpath);
       unlink(temp);
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

sword HMBValidate(byte *name)
{
  NW(name);
  return TRUE;
}


/*
      ----------------------------------------------

      Function to set current position in a HMB msg.

      ----------------------------------------------
*/


sword  HMBSetCurPos(MSGH *msgh, dword pos)
{
   return msgh->cur_pos = pos;
}


/*
      ----------------------------------------------

      Function to get current position in a HMB msg.

      ----------------------------------------------
*/



dword  HMBGetCurPos(MSGH *msgh)
{
  return (msgh->cur_pos);
}


/*
      ------------------------------------

      Function to convert # to UID (fake).

      ------------------------------------
*/


UMSGID  HMBMsgnToUid(MSG *sq, dword msgnum)
{
  msgapierr=MERR_NONE;

  if (InvalidMh(sq))
    return 0L;

 // high_msg should be array_size in case of multiple writes to the area
 // 'in one lock', the in memory index won't be up-to-date in that case!

  if( (msgnum < 1) || (msgnum>sq->high_msg) )
    {
    msgapierr=MERR_NOENT;
    return 0L;
    }

  return  ( (UMSGID) HMBDATA->msgs[msgnum-1].msgno );

}


/*
      -----------------------------------------------

      Function to convert UID to existing msg number.

      -----------------------------------------------
*/


dword  HMBUidToMsgn(MSG *sq, UMSGID umsgid, word type)
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

dword  HMBGetTextLen(MSGH *msgh)
{
  return msgh->cur_len;
}

/*
      ------------------------------

      Function to get kludge length.

      ------------------------------
*/

dword  HMBGetCtrlLen(MSGH *msgh)
{
  return msgh->clen;
}

/*
      --------------------------------

      Function to get high water mark.

      --------------------------------
*/

dword  HMBGetHighWater(MSG *sq)
{
  return sq->high_water;
}

/*
      ---------------------------

      Function to set high water.

      ---------------------------
*/

sword  HMBSetHighWater(MSG *sq, dword hwm)
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

   if(HMBinfo.opens)
      {
      HMBinfo.opens++;
      return 0;
      }

    HMBinfo.info =
       HMBinfo.idx =
          HMBinfo.toidx =
             HMBinfo.hdr =
                HMBinfo.txt = -1;  // Set to -1 first, easy detection of
                                   // open files in case of error..


    #ifndef __GNUC__
    sprintf(temp, "%s\\msginfo.bbs", Strip_Trailing(mi.hudsonpath, '\\'));
    #else
    sprintf(temp, "%s/msginfo.bbs", Strip_Trailing(mi.hudsonpath, '/'));    
    #endif

    if( (HMBinfo.info = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
       {
       msgapierr=MERR_OPENFILE;
       return -1;
       }

    if(filelength(HMBinfo.info)==0L)
       {
       memset(temp, '\0', sizeof(temp));
       if(write(HMBinfo.info, temp, sizeof(temp)) != sizeof(temp))
          goto ErrorExit;
       }

    sprintf(temp, "%s/msgidx.bbs", mi.hudsonpath);

    if( (HMBinfo.idx = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    sprintf(temp, "%s/msgtoidx.bbs", mi.hudsonpath);

    if( (HMBinfo.toidx = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    sprintf(temp, "%s/msghdr.bbs", mi.hudsonpath);

    if( (HMBinfo.hdr = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    sprintf(temp, "%s/msgtxt.bbs", mi.hudsonpath);

    if( (HMBinfo.txt = sopen(temp, O_BINARY | O_RDWR | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE)) == -1)
        goto ErrorExit;

    HMBinfo.opens++;

    return 0;  // All was well!

    ErrorExit:  // At least one open failed, close all opened files..

    if(HMBinfo.info  != -1) close(HMBinfo.info );
    if(HMBinfo.idx   != -1) close(HMBinfo.idx  );
    if(HMBinfo.toidx != -1) close(HMBinfo.toidx);
    if(HMBinfo.hdr   != -1) close(HMBinfo.hdr  );
    if(HMBinfo.txt   != -1) close(HMBinfo.txt  );

    msgapierr=MERR_OPENFILE;

    return -1;

}

/* ------------------------------------ */
/* Close all Hudson base files          */
/* ------------------------------------ */


int  HMBCloseBase(void)
{
   HMBinfo.opens--;

   if(HMBinfo.opens) return 0;

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

int near HMBScanArea(MSG *sq)
{
    word howmany = HMBinfo.msginfo.total_on_board[BNUM];
    int l, current;
    HMB_MSGIDX *idx_array;
    static unsigned idxsize, idxcount;
    int testval = BNUM+1;
    int retval=0;

    if(howmany == 0)
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
       msgapierr=MERR_NOMEM;
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

    for(l=0, current=0; (l<idxcount) && (current<=howmany); l++)
       {
       if( (idx_array[l].board == testval) && (idx_array[l].msg_num != -1) )
          {
          HMBDATA->msgs[current].msgno = idx_array[l].msg_num;
          HMBDATA->msgs[current++].recno = l;
          }
       }

    if(current > howmany)
      {
      retval = -1;
      msgapierr=MERR_BADMSGINFO;
      }

    sq->num_msg = sq->high_msg =  current;

    free(idx_array);

    return retval;

}

/* ------------------------------------------------------------------------ */
/* Find out the 'record number' in the index files for a certain msg number */
/* ------------------------------------------------------------------------ */


sword near FindRecno(MSG *sq, word msgno)
{
    // num_msg should be array_size in locked/multiple writes!

    if( (msgno < 1) || (msgno>sq->num_msg) )
       {
       msgapierr=MERR_NOENT;
       return -1;
       }

    msgno--;   /* Make zero based */

    return HMBDATA->msgs[msgno].recno;

}


/* -------------------------------------- */
/* Convert HMB header info to MIS format  */
/* -------------------------------------- */

void HMB2MIS(MIS *mis, HMB_MSGHDR *hmbhdr, MSG *sq)
{
    word m, d, y, h;
    int l;
    struct JAMtm TM;

    memset( &TM, '\0', sizeof( TM ));

    mis->origfido.zone = hmbhdr->orig_zone;
    mis->origfido.node = hmbhdr->orig_node;
    mis->origfido.net  = hmbhdr->orig_net ;

    mis->destfido.zone = hmbhdr->dest_zone;
    mis->destfido.node = hmbhdr->dest_node;
    mis->destfido.net  = hmbhdr->dest_net ;

    memcpy(mis->from, hmbhdr->who_from+1, (size_t) min(hmbhdr->who_from[0], 35));
    memcpy(mis->to, hmbhdr->who_to+1, (size_t) min(hmbhdr->who_to[0], 35));
    memcpy(mis->subj, hmbhdr->subject+1, (size_t) min(hmbhdr->subject[0], 71));

    mis->replyto = hmbhdr->prev_reply;
    mis->replies[0] = hmbhdr->next_reply;

    mis->msgno    = hmbhdr->msgnum;
    mis->origbase = sq->type;

    // Attribute conversion using conv. table...

    for (l=0; l<HMBATTRNUM; l++)
      {
      if(hmbhdr->hmbattr & Attr_AH_Table[l].hmb_attr)
         mis->attr1 |= Attr_AH_Table[l].api_attr;
      }

    if( !(hmbhdr->hmbattr & HMB_UNSENT)  &&  // Not if unsent (netmail)
        !(hmbhdr->hmbattr & HMB_UNMOVED) &&  // Not if unmoved (echo)
         (hmbhdr->hmbattr & HMB_LOCAL)   &&  // Only if locally made
        !(sq->type & MSGTYPE_LOCAL) )        // Not if local area
        mis->attr1 |= aSENT;


    if(hmbhdr->post_date[0] != 0 && hmbhdr->post_time[0] != 0)
       {
       sscanf(hmbhdr->post_date+1, "%2hd-%2hd-%2hd", &m, &d, &y);
       TM.tm_year = y;
       TM.tm_mon  = m-1;
       TM.tm_mday = d;

       sscanf(hmbhdr->post_time+1, "%2hd:%2hd", &h, &m);
       TM.tm_hour = h;
       TM.tm_min  = m;

       mis->msgwritten = JAMsysMkTime(&TM);
       }

}


/* --------------------------------------------------------------- */
/* Extract the kludges from msg text, calc length, store in buffer */
/* Return 0 if all is well                                         */
/* --------------------------------------------------------------- */


int near ReadKludges(struct _msgh *msgh)
{
    int howmany;
    byte *end, *begin;
    char *ctrlbuf;
    char *dest, *src;
    int movesleft;

  // Read max of 2 blocks (512 bytes) for control info (kludges etc).
  // 10 aug 94: upped to 8 blocks...

    howmany = (msgh->numblocks < 8) ? msgh->numblocks : 8;

    if(howmany == 0)  // No text, so no kludges..
       return 0;

    if(lseek(HMBinfo.txt, (long) ((long)msgh->startblock * sizeof(HMB_TXT)), SEEK_SET) !=
                          (long) ((long)msgh->startblock * sizeof(HMB_TXT)) )
        {
        msgapierr=MERR_SEEK;
        return -1;
        }

    if(read(HMBinfo.txt, msgh->kludges, (howmany * sizeof(HMB_TXT)) ) !=
                                        (howmany * sizeof(HMB_TXT)) )
        {
        msgapierr=MERR_READ;
        return -1;
        }


    if(howmany > 1)  // Get rid op Pascal lenght bytes.
      {
      movesleft = howmany - 1;
      dest = msgh->kludges + 256;
      src  = msgh->kludges + 257;

      while(movesleft)
         {
         memcpy(dest, src, 255);         /* Pascal strings! */
         src  += 256;
         dest += 255;
         movesleft--;
         }

      *dest = '\0';                      // NULL terminate.
      }

    ctrlbuf = (byte *) CopyToControlBuf(msgh->kludges + 1,
                                        &end,
                                        (unsigned *)NULL);

    begin = msgh->kludges + 1;
    msgh->rawclen = (dword) (end - begin);
    if(msgh->rawclen > 255)    // More than 1 Pascal string, discount
       msgh->rawclen += (msgh->rawclen/255);

    memset(msgh->kludges, '\0', sizeof(msgh->kludges));
    if(ctrlbuf)
       {
       strcpy(msgh->kludges, ctrlbuf);
       free(ctrlbuf);
       }

    HMBHandleKludges(msgh->kludges, &msgh->mis);

    msgh->clen = strlen(msgh->kludges) + 1;

    return 0;
}


/* ------------------------------------- */
/*  Extract FMPT, TOPT info from kludges */
/* ------------------------------------- */


void near HMBHandleKludges(char *ctrl, MIS *mis)
{
  char *s;

  /* Handle the FMPT kludge */

  if ((s=GetCtrlToken(ctrl,fmpt)) != NULL)
  {
    mis->origfido.point=atoi(s+5);
    free(s);

    RemoveFromCtrl(ctrl,fmpt);
  }


  /* Handle TOPT too */

  if ((s=GetCtrlToken(ctrl,topt)) != NULL)
  {
    mis->destfido.point=atoi(s+5);
    free(s);

    RemoveFromCtrl(ctrl,topt);
  }

  Convert_Flags(ctrl, &mis->attr1, &mis->attr2);

}


/* ------------------------------------------------------------------- */
/* Calculate the length of the message text (minus kludges and slack!) */
/* ------------------------------------------------------------------- */

int near HMBCalcTextLen(struct _msgh *msgh)
{
  long which;
  int howmany, movesleft;
  unsigned readlen;
  char *txtptr, *dest, *src;

  if(msgh->numblocks == 0)
     return 0;

  howmany = (msgh->numblocks < 16) ? msgh->numblocks : 16;

  which = (long) msgh->startblock + (long) msgh->numblocks - (long) howmany;

  if(lseek(HMBinfo.txt, (long) (which * sizeof(HMB_TXT)), SEEK_SET) !=
                        (long) (which * sizeof(HMB_TXT)) )
        {
        msgapierr=MERR_SEEK;
        return -1;
        }

  readlen = howmany * 256;

  if((txtptr = calloc(1, readlen+1)) == NULL)
     return -1;

  if(read(HMBinfo.txt, txtptr, readlen) != (int) readlen)
    {
    msgapierr=MERR_READ;
    free(txtptr);
    return -1;
    }

  // Now we fiddle with readlen, in order to get the right number back
  // which is wrong due to the presence of Pascal length bytes.

  readlen -= howmany;

  // And we want to know the lenght of the last block (as it is filled
  // out with slack to reach 256 bytes total pascal string length).

  msgh->lastblocklen = *(txtptr + ((howmany-1)*256));

  // NULL terminate the last block at the point where the Pascal string
  // ends. There may be trailing junk!

  *(txtptr + ((howmany-1)*256) + msgh->lastblocklen + 1) = '\0';

  if(howmany > 1)  // Get rid of Pascal lenght bytes.
    {
    movesleft = howmany - 1;
    dest = txtptr + 256;
    src  = txtptr + 257;

    while(movesleft)
       {
       memcpy(dest, src, 255);         /* Pascal strings! */
       src  += 256;
       dest += 255;
       movesleft--;
       }

    *dest = '\0';                      // NULL terminate.
    }

  // text now starts at txtptr+1 and is NULL terminated;

  // If this junk is not even longer than the rawclen, then we obviously have
  // a small message without any text. We then actually only have the kludges
  // over here! (+5 : must at least be 'VIA <something>' or PATH:, SEEN-BY)

  if(strlen(txtptr+1) <= (msgh->rawclen+5))
    msgh->trailsize = 0;
  else
    msgh->trailsize = AnalyseTrail(txtptr+1, readlen, &msgh->mis);

  free(txtptr);

  if(msgh->trailsize == 0) // No SB, Path etc found. Only count slack!
     msgh->trailsize = 255 - msgh->lastblocklen;
  else                     // We need to recalc to discount for pascal strings
    {
    if(msgh->trailsize > 255)    // More than 1 Pascal string, discount
       msgh->trailsize += (msgh->trailsize/255);
    }

  /* Underneath * 256 (not 255) cause we NEED the extra space for the */
  /* length byte of Pascal strings, when we read in the text (before  */
  /* removing the length bytes..)                                     */

  msgh->cur_len = (msgh->numblocks * 256) -    /* Pascal string blocks */
                   msgh->rawclen -             /* Kludges              */
                   msgh->trailsize -           /* Slack + SB etc.      */
                   1;

  return 0;

}

/* ----------------------------------------------------- */
/* Convert MIS header to HMB header, add to control info */
/* ----------------------------------------------------- */


void MIS2HMB(MSG *sq, struct _msgh *msgh, MIS *mis, HMB_MSGHDR *hmbhdr, byte *kludges)
{
  char temp[150];
  byte *tempkludges;
  int l;
  JAMTM *jamtm;
  dword trailmax;
  STRINGLIST *current;

  #define HMBMASK1 (aAS|aDIR|aIMM|aKFS|aTFS|aCFM|aLOK|aFRQ|aHOLD|aZGT)
  #define HMBMASK2 (aHUB|aXMA|aHIR|aCOV|aSIG|aLET|aFAX)


  memset(msgh->kludges, '\0', sizeof(msgh->kludges));
  if(kludges)
     strcpy(msgh->kludges, kludges);

  hmbhdr->start_block = msgh->startblock;
  hmbhdr->msgnum = msgh->realno;

  hmbhdr->dest_net  = mis->destfido.net;
  hmbhdr->dest_node = mis->destfido.node;
  hmbhdr->dest_zone = mis->destfido.zone;

  hmbhdr->orig_zone = mis->origfido.zone;
  hmbhdr->orig_node = mis->origfido.node;
  hmbhdr->orig_net  = mis->origfido.net;

  hmbhdr->board = BNUM + 1;

  hmbhdr->who_to[0] = (byte) strlen(mis->to);
  memcpy(hmbhdr->who_to+1, mis->to, hmbhdr->who_to[0]);

  hmbhdr->who_from[0] = (byte) strlen(mis->from);
  memcpy(hmbhdr->who_from+1, mis->from, hmbhdr->who_from[0]);

  hmbhdr->subject[0] = (byte) strlen(mis->subj);
  memcpy(hmbhdr->subject+1, mis->subj, hmbhdr->subject[0]);

  hmbhdr->prev_reply = mis->replyto;
  hmbhdr->next_reply = mis->replies[0];

  // Now convert attribute values, using our conversion table..

  for (l=0; l<HMBATTRNUM; l++)
    {
    if(mis->attr1 & Attr_AH_Table[l].api_attr)
       hmbhdr->hmbattr |= Attr_AH_Table[l].hmb_attr;
    }

  if(!(mis->attr1 & aSENT) && (mis->attr1 & aLOCAL) )
     {
     if(sq->type & (MSGTYPE_ECHO|MSGTYPE_NEWS))
        hmbhdr->hmbattr |= HMB_UNMOVED;
     else if(sq->type & (MSGTYPE_NET|MSGTYPE_MAIL))
        hmbhdr->hmbattr |= HMB_UNSENT;
     }

  if(sq->type & (MSGTYPE_NET|MSGTYPE_MAIL))
     hmbhdr->hmbattr |= HMB_NETMAIL;

  jamtm = JAMsysLocalTime(&mis->msgwritten);

  sprintf(temp, "%02d-%02d-%02d", jamtm->tm_mon+1,
                                  jamtm->tm_mday,
                                  jamtm->tm_year);

  hmbhdr->post_date[0] = (byte) 8;
  memcpy(hmbhdr->post_date+1, temp, 8);


  sprintf(temp, "%02d:%02d", jamtm->tm_hour,
                             jamtm->tm_min);

  hmbhdr->post_time[0] = (byte) 5;
  memcpy(hmbhdr->post_time+1, temp, 5);

  memset(temp, '\0', sizeof(temp));
  Files2Subject(mis, temp);
  if(temp[0] != '\0')           // Anything done?
    {
    hmbhdr->subject[0] = (byte) strlen(temp);
    memcpy(hmbhdr->subject+1, temp, strlen(temp));
    }

  // Don't do all this for MOPEN_RW, can't write kludges anyway..
  if(msgh->mode != MOPEN_RW)
    {
    if( (mis->origfido.point != 0) && (sq->type & (MSGTYPE_NET|MSGTYPE_MAIL)) )
       {
       sprintf(temp, "\01FMPT %hu", mis->origfido.point);
       strcat(msgh->kludges, temp);
       }

    if( (mis->destfido.point != 0) && (sq->type & (MSGTYPE_NET|MSGTYPE_MAIL)) )
       {
       sprintf(temp, "\01TOPT %hu", mis->destfido.point);
       strcat(msgh->kludges, temp);
       }

    if( ((mis->attr1 & HMBMASK1) || (mis->attr2 & HMBMASK2)) &&
        (sq->type & (MSGTYPE_NET|MSGTYPE_MAIL)) )
      {
      Attr2Flags(temp, (mis->attr1 & HMBMASK1), (mis->attr2 & HMBMASK2));
      strcat(msgh->kludges, temp);
      }

    if(msgh->kludges[0] != '\0')
       {
       tempkludges = CvtCtrlToKludge(msgh->kludges);
       strncpy(msgh->kludges, tempkludges, 2048);
       free(tempkludges);
       }

    msgh->rawclen = strlen(msgh->kludges);

    // Now the trailing stuff.

    if(mis->seenby || mis->path || mis->via)
      {
      trailmax  = 512;
      msgh->trailsize = 0;
      msgh->trail = calloc(1, 512);

      for(current=mis->seenby; current; current=current->next)
        {
        if(!(sq->type & (MSGTYPE_ECHO|MSGTYPE_NEWS)))
           break;
        AddToString(&msgh->trail, &trailmax, &msgh->trailsize, "\rSEEN-BY: ");
        AddToString(&msgh->trail, &trailmax, &msgh->trailsize, current->s);
        }

      for(current=mis->path; current; current=current->next)
        {
        if(!(sq->type & (MSGTYPE_ECHO|MSGTYPE_NEWS)))
           break;
        AddToString(&msgh->trail, &trailmax, &msgh->trailsize, "\r\01PATH: ");
        AddToString(&msgh->trail, &trailmax, &msgh->trailsize, current->s);
        }

      for(current=mis->via; current; current=current->next)
        {
        if(!(sq->type & (MSGTYPE_NET|MSGTYPE_MAIL)))
           break;
        AddToString(&msgh->trail, &trailmax, &msgh->trailsize, "\r\01Via ");
        AddToString(&msgh->trail, &trailmax, &msgh->trailsize, current->s);
        }
      }

    // This WAS %256 and /256!! Stupid, stupid!!!!
    if( ((msgh->totlen + msgh->rawclen + msgh->trailsize) % 255) == 0 )
       msgh->numblocks = (msgh->totlen + msgh->rawclen + msgh->trailsize) / 255;
    else
       msgh->numblocks = ((msgh->totlen + msgh->rawclen + msgh->trailsize) / 255)+1;
    }

  // In case of MOPEN_RW, this info is OK already (read from header!)

  hmbhdr->num_blocks = msgh->numblocks;

}



/* ---------------------------------------------------------------- */
/* We write ECHOMAIL.BBS and NETMAIL.BBS here, because it need info */
/* about the record number in the Hudson base, info which is not    */
/* available at a higher level (app using the API)                  */
/* ---------------------------------------------------------------- */


void near write_bbs_file(struct _msgh *msgh)
{
   int tosslog;
   char temp[120];
   sword data = (sword)msgh->recno;

   if(msgh->sq->type & MSGTYPE_LOCAL)
      return;

   if(msgh->sq->type & (MSGTYPE_ECHO|MSGTYPE_NEWS))
      sprintf(temp, "%s/echomail.bbs", mi.hudsonpath);
   else
      sprintf(temp, "%s/netmail.bbs", mi.hudsonpath);

   if( (tosslog = sopen(temp, O_BINARY | O_CREAT | O_WRONLY, SH_DENYWR, S_IWRITE | S_IREAD)) == -1)
         return;

   lseek(tosslog, 0L, SEEK_END);
   write(tosslog, &data, sizeof(sword));
   close(tosslog);

}


/* ------------------------------------------- */
/*  Function to read MSGINFO.BBS into memory.  */
/*  Returns 0 on success, -1 on failure.       */
/* ------------------------------------------- */


sword near HMBReadInfo(void)
{

  if(lseek(HMBinfo.info, 0L, SEEK_SET) == -1)
     {
     msgapierr=MERR_SEEK;
     return -1;
     }

  if(read(HMBinfo.info, &HMBinfo.msginfo, sizeof(HMB_MSGINFO))
                                                != sizeof(HMB_MSGINFO))
     {
     msgapierr=MERR_READ;
     return -1;
     }

   return 0;

}

/* ------------------------------------------ */
/*  Function to write MSGINFO.BBS to disk.    */
/*  Returns 0 on success, -1 on failure.      */
/* ------------------------------------------ */


sword near HMBWriteInfo(void)
{

  if(lseek(HMBinfo.info, 0L, SEEK_SET) == -1)
     {
     msgapierr=MERR_SEEK;
     return -1;
     }

  if(write(HMBinfo.info, &HMBinfo.msginfo, sizeof(HMB_MSGINFO))
                                                != sizeof(HMB_MSGINFO))
     {
     msgapierr=MERR_WRITE;
     return -1;
     }

   return 0;

}


/* --------------------------------------------- */
/*  Function to write an index entry to disk.    */
/*  Returns 0 on success, -1 on failure.         */
/* --------------------------------------------- */

sword near HMBWriteIdx(sword msgnum, unsigned char board, sword recno)
{
    HMB_MSGIDX idx;

    idx.msg_num = msgnum;
    idx.board   = board;

    if(lseek(HMBinfo.idx,
             (long) ((long)recno * sizeof(HMB_MSGIDX)),
             SEEK_SET) == -1)
         {
         msgapierr=MERR_SEEK;
         return -1;
         }

    if(write(HMBinfo.idx, &idx, sizeof(HMB_MSGIDX)) != sizeof(HMB_MSGIDX))
         {
         msgapierr=MERR_WRITE;
         return -1;
         }

    return 0;

}

/* ---------------------------------------------- */
/*  Function to read the entire index in memory.  */
/*  Returns a pointer to the index on succes,     */
/*  and NULL on failure (no mem, or read error)   */
/* ---------------------------------------------- */

HMB_MSGIDX * HMBReadEntireIndex(void)
{
    unsigned idxsize;     HMB_MSGIDX *idxptr;


    if(HMBOpenBase() == -1)
      return NULL;

    idxsize = filelength(HMBinfo.idx);

    if( (idxptr = malloc(idxsize)) == NULL )
       {
       msgapierr=MERR_NOMEM;
       HMBCloseBase();
       return NULL;
       }


    if((lseek(HMBinfo.idx, 0L, SEEK_SET) == -1) ||
       (read(HMBinfo.idx, idxptr, idxsize) != idxsize) )
       {
       msgapierr=MERR_READ;
       free(idxptr);
       HMBCloseBase();
       return NULL;
       }

    HMBCloseBase();

    return idxptr;
}


/* -------------------------------------- */
/*  Function to read a header in memory.  */
/*  Returns 0 on succes, -1 on failure    */
/* -------------------------------------- */

sword near HMBReadHeader(sword recno, HMB_MSGHDR *hdr)
{

  if(lseek(HMBinfo.hdr, (long) ((long)recno * sizeof(HMB_MSGHDR)), SEEK_SET) !=
                        (long) ((long)recno * sizeof(HMB_MSGHDR)))
       {
       msgapierr=MERR_SEEK;
       return -1;
       }

  if(read(HMBinfo.hdr, hdr, sizeof(HMB_MSGHDR)) != sizeof(HMB_MSGHDR))
       {
       msgapierr=MERR_READ;
       return -1;
       }

   return 0;

}

/* ------------------------------------ */
/*  Function to write header to disk.   */
/*  Returns 0 on succes, -1 on failure  */
/* ------------------------------------ */

sword near HMBWriteHeader(sword recno, HMB_MSGHDR *hdr)
{

  if(lseek(HMBinfo.hdr, (long) ((long)recno * sizeof(HMB_MSGHDR)), SEEK_SET) !=
                        (long) ((long)recno * sizeof(HMB_MSGHDR)))
       {
       msgapierr=MERR_SEEK;
       return -1;
       }

  if(write(HMBinfo.hdr, hdr, sizeof(HMB_MSGHDR)) != sizeof(HMB_MSGHDR))
       {
       msgapierr=MERR_WRITE;
       return -1;
       }

   return 0;

}


/* ------------------------------------ */
/*  Function to create or touch a file  */
/*  (semaphore file)                    */
/*  Return 0 if OK, -1 on error         */
/* ------------------------------------ */

#ifndef __GNUC__

int creat_touch(char *filename)
{
   int retval = 0;
   time_t    tnow;
   struct tm tmnow;
#ifndef __WATCOMC__
   struct ftime ft;
#else
union {

    struct  mftime   {
        unsigned    ft_tsec  : 5;   /* Two second interval */
        unsigned    ft_min   : 6;   /* Minutes */
        unsigned    ft_hour  : 5;   /* Hours */
        unsigned    ft_day   : 5;   /* Days */
        unsigned    ft_month : 4;   /* Months */
        unsigned    ft_year  : 7;   /* Year */
    } gvebits;

    struct shorts   {
        unsigned short gvetime;
        unsigned short gvedate;
    } gveshorts;

} ft;

unsigned short xt, xd;

#endif
   FILE *f;


   tnow  = time(NULL);
   tmnow = *localtime(&tnow);

#ifndef __WATCOMC__
   ft.ft_year  = tmnow.tm_year - 80;
   ft.ft_month = tmnow.tm_mon + 1;
   ft.ft_day   = tmnow.tm_mday;
   ft.ft_hour  = tmnow.tm_hour;
   ft.ft_min   = tmnow.tm_min;
   ft.ft_tsec  = tmnow.tm_sec/2;
#else

   ft.gvebits.ft_year  = tmnow.tm_year - 80;
   ft.gvebits.ft_month = tmnow.tm_mon + 1;
   ft.gvebits.ft_day   = tmnow.tm_mday;
   ft.gvebits.ft_hour  = tmnow.tm_hour;
   ft.gvebits.ft_min   = tmnow.tm_min;
   ft.gvebits.ft_tsec  = tmnow.tm_sec/2;
   xd = ft.gveshorts.gvedate;
   xt = ft.gveshorts.gvetime;
#endif

   if ((f = fopen(filename, "r+b")) != NULL)
#ifndef __WATCOMC__
         setftime(fileno(f), &ft);
#else
         _dos_setftime(fileno(f), xd, xt);
#endif
   else if ((f = fopen(filename, "w")) != NULL)
#ifndef __WATCOMC__
         setftime(fileno(f), &ft);
#else
         _dos_setftime(fileno(f), xd, xt);
#endif
   else  retval = -1;

   if (f)
         fclose(f);

   return retval;

}

#else

int creat_touch(char *filename)
{
  int handle;

  unlink(filename);	
  handle = creat(filename, S_IREAD | S_IWRITE);
  
  if(!handle) return -1;
  
  close(handle);
  
  return 0;
}


#endif
