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
#include <fcntl.h>
#include <sys\stat.h>
#include <share.h>
#include <time.h>
#include <dos.h>

#include "dr.h"
#include "prog.h"
#include "alc.h"
#include "max.h"
#include "old_msg.h"
#include "msgapi.h"
#include "jam.h"
#include "api_jam.h"
#include "api_jamp.h"
#include "apidebug.h"


/* Prototypes */

static void JAM2SQ(struct _msgh *msgh, byte *Subfields);
static void tinyJAM2SQ(struct _msgh *msgh, byte *Subfields);
void addkludge(struct _msgh *msgh, char *text, int txtlen, char *data, dword len, word *cursize);
void addtext(struct _msgh *msgh, char *stattext, int statlen, char *text, word len, word *cursize);
void Init_JAMheader(MSG *sq);
void SQ2JAM(XMSG *msg, char *ctxt, MSG *sq, word mode, byte** SubFieldPtr);

byte JAMMsgExists(MSG *sq, UMSGID umsgid);

int JAMmbOpen (MSG *sq, byte *name);
int JAMmbClose(MSG *sq);

int JAMReadIdx   (MSG *sq, dword number, JAMIDXREC *idx);
int JAMReadHeader(MSG *sq, long offset, JAMHDR *hdr);

int JAMWriteIdx   (MSG *sq, dword number, JAMIDXREC *idx);
int JAMWriteHeader(MSG *sq, long offset, JAMHDR *hdr);

/* Functions stolen from the JAM API (but modified) */

void JAMmbAddField(byte *start,
                   dword WhatField,
                   size_t  DatLen,
                   word * Position,
                   byte * Data);

int JAMmbUpdateHeaderInfo(MSG *sq, int WriteIt);

dword   JAMsysTime     (dword * pTime);
dword   JAMsysMkTime   (JAMTM * pTm);
JAMTM * JAMsysLocalTime(dword * pt);

int    JAMmbStoreLastRead(MSG *sq, dword last, dword CRC);
dword  JAMmbFetchLastRead(MSG *sq, dword UserCRC);


static JAMDATA *jamdata;

/*
     ----------------------------

      Function to open a JAM area.

      ----------------------------
*/


MSG * MSGAPI JAMOpenArea(byte *name, word mode, word type)
{
  MSG * sq;

  NW(mode);

  if ((sq=calloc(1, sizeof(MSG)))==NULL)
    return NULL;

  sq->id=MSGAPI_ID;
  sq->type = type;

  if (type & MSGTYPE_ECHO)
    sq->isecho=TRUE;

  if ((sq->api=(struct _apifuncs *)calloc(1, sizeof(struct _apifuncs)))==NULL)
  {
    pfree(sq);
    msgapierr=MERR_NOMEM;
    return NULL;
  }


  if ((sq->apidata=(JAMDATA *)calloc(1, sizeof(JAMDATA)))==NULL)
  {
    pfree(sq->api);
    pfree(sq);
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  /* For debugging : */
  jamdata = sq->apidata;

  sq->len=sizeof(MSG);

//  sq->locked=0;     Will be 0 already..
//  sq->cur_msg=0;

  *sq->api=JAM_funcs;
  sq->sz_xmsg=sizeof(XMSG);

  /* Now try to open the message area, using JAM_API functions */

  if( JAMmbOpen(sq, name) == -1)  /* -1 == failed! */
    {
    pfree(sq->api);
    pfree(sq);
    pfree(sq->apidata);
    return NULL;
    }

  sq->num_msg  = JamData->HdrInfo.ActiveMsgs;
  sq->high_msg = JAMUidToMsgn(sq,
                 (dword) ((filelength(JamData->IdxHandle) / sizeof(JAMIDXREC))
                                           + JamData->HdrInfo.BaseMsgNum - 1),
                  UID_PREV);

  return sq;
}


/*
      -----------------------------

      Function to close a JAM area.

      -----------------------------
*/


static sword EXPENTRY JAMCloseArea(MSG *sq)
{
  if (InvalidMh(sq))
    return -1;

  if(sq->locked)
     JAMUnlock(sq);

  JAMmbClose(sq);

  pfree(sq->api);
  pfree((JAMDATA *)sq->apidata);
  sq->id=0L;
  pfree(sq);

  return 0;
}


/*
      -------------------------------

      Function to open a JAM message.

      -------------------------------
*/


static MSGH * EXPENTRY JAMOpenMsg(MSG *sq, word mode, dword msgnum)
{
  struct _msgh *msgh;
  byte *SubFieldPtr = NULL;

  if (InvalidMh(sq))
    return NULL;


  if(mode == MOPEN_WRITE) return NULL;  // Not supported

  if( ((mode == MOPEN_CREATE) || (mode == MOPEN_RW)) && (!(sq->locked)) )
     return NULL;                     // We must be locked if we write!

  /* Translate special 'message numbers' to 'real' msg numbers */

  switch(msgnum)
    {
    case MSGNUM_CUR:
       msgnum = sq->cur_msg;
       break;

    case MSGNUM_PREV:
       msgnum = JAMUidToMsgn(sq, sq->cur_msg - 1, UID_PREV);
       if(msgnum == 0)
          return NULL;
       break;

    case MSGNUM_NEXT:
       msgnum = JAMUidToMsgn(sq, sq->cur_msg + 1, UID_NEXT);
       if(msgnum == 0)
          return NULL;
       break;

    default:
       break;
    }

  /* If we're not creating a new msg, check existence */

  if( (mode != MOPEN_CREATE) || (msgnum != 0L) )
    {
    if(
        (msgnum < JamData->HdrInfo.BaseMsgNum) ||
        (msgnum > sq->high_msg)
      )
       return NULL;
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


  /* Now get message index, to learn about msg */

  if(msgnum != 0)   /* Not a new addition, so we want info */
    {
    sq->cur_msg = msgnum;

    // Read Index

    if( JAMReadIdx(sq, msgnum, &JamData->Idx) == -1 )
       goto ErrorExit;

    if( JamData->Idx.HdrOffset == -1 )  // Deleted message!
        goto ErrorExit;

    // Get header

    if(JAMReadHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
       goto ErrorExit;

    if(JamData->Hdr.Attribute & MSG_DELETED)
       goto ErrorExit;

    if(JamData->Hdr.MsgNum != msgnum)       // Weird! Corrupt?
       goto ErrorExit;

    // Don't read subfields if we are in MOPEN_RW mode.

    if( (mode == MOPEN_READ) || (mode == MOPEN_RDHDR) )
      {
      if(JamData->Hdr.SubfieldLen != 0L)
        {
        if( (SubFieldPtr = calloc(1, JamData->Hdr.SubfieldLen)) == NULL )
            goto ErrorExit;

        // No seek here, we just read the header..
        if(read(JamData->HdrHandle,
                 SubFieldPtr,
                   (unsigned) JamData->Hdr.SubfieldLen) !=
                                          (int) JamData->Hdr.SubfieldLen )
           goto ErrorExit;
        }
      }


    if(mode == MOPEN_CREATE) /* Re-create existing msg, zap out old */
       {
       JamData->Hdr.Attribute |= MSG_DELETED;
       JamData->Hdr.TxtLen = 0L;

       if(JAMWriteHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
           goto ErrorExit;
       }

    msgh->hdroffset = JamData->Idx.HdrOffset;
    msgh->txtoffset = JamData->Hdr.TxtOffset;
    }


  if(mode == MOPEN_CREATE)   /* New addition to the message base */
    {
    /* Point to end, to add to msgbase */
    msgh->hdroffset = lseek(JamData->HdrHandle, 0L, SEEK_END);
    msgh->txtoffset = lseek(JamData->TxtHandle, 0L, SEEK_END);

    if(msgnum == 0)
      {
      msgnum = (filelength(JamData->IdxHandle) / sizeof(JAMIDXREC)) + JamData->HdrInfo.BaseMsgNum;
      sq->high_msg = msgnum;
      sq->num_msg++;
      JamData->HdrInfo.ActiveMsgs += 1; /* New addition, inc # msgs */
      }

    sq->cur_msg = msgnum;
    }

  if(mode != MOPEN_CREATE) /* So we need to know lengths etc. */
    {
    if(mode != MOPEN_RDHDR)
       JAM2SQ(msgh, SubFieldPtr);
    else
       tinyJAM2SQ(msgh, SubFieldPtr);
    }

  msgh->msgnum = msgnum;

  if(SubFieldPtr) free(SubFieldPtr);

  return (MSGH *)msgh;

  // We only get here if we had an error!

  ErrorExit:

  if(SubFieldPtr) free(SubFieldPtr);
  free(msgh);
  return NULL;

}


/*
      --------------------------------

      Function to close a JAM message.

      --------------------------------
*/


static sword EXPENTRY JAMCloseMsg(MSGH *msgh)
{
  if (InvalidMsgh(msgh))
     return -1;

  msgh->id=0L;
  if(msgh->kludges) free(msgh->kludges);
  if(msgh->addtext) free(msgh->addtext);

  pfree((struct _msgh *)msgh);

  return 0;
}



/*
      --------------------------------

      Function to read a JAM message.

      --------------------------------
*/

static dword EXPENTRY JAMReadMsg(MSGH *msgh, XMSG *msg,
                                  dword offset, dword bytes, byte *text,
                                  dword clen, byte *ctxt)
{
  MSG *sq = msgh->sq;


  if (InvalidMsgh(msgh))
    return -1;

  /* Copy header info */

  if(msg)
    memmove(msg, &msgh->hdr, sizeof(XMSG));

  /* Copy kludges */

  if(ctxt && clen)
    {
    if(clen > msgh->clen)
       clen = msgh->clen;

    if(msgh->kludges)
       memmove(ctxt, msgh->kludges, (size_t) clen);
    else *ctxt = '\0';
    }


  /* Read message text */

  if(text && bytes)
    {
    if(bytes > (msgh->cur_len - offset))
       bytes = (msgh->cur_len - offset);  /* No crazy len */

    if(offset >= (msgh->cur_len - msgh->add_size)) /* Return added text (SB + Path) */
       {
       memmove(text, msgh->addtext + (offset - (msgh->cur_len - msgh->add_size)), bytes);
       }
    else
       {
       if(offset + bytes >= (msgh->cur_len - msgh->add_size)) /* Disk + added text */
         {
         if(lseek(JamData->TxtHandle,
                 (long) (msgh->txtoffset + offset),
                 SEEK_SET) == -1) return -1;

         if (read(JamData->TxtHandle, text, (msgh->cur_len - msgh->add_size) - offset) == -1)
            return -1;

         memmove(text + ((msgh->cur_len - msgh->add_size) - offset),
                 msgh->addtext,
                 bytes - ((msgh->cur_len - msgh->add_size) - offset) );
         }
       else /* Only disk */
         {
         if(lseek(JamData->TxtHandle,
                 (long) (msgh->txtoffset + offset),
                 SEEK_SET) == -1) return -1;

         if (read(JamData->TxtHandle, text, (size_t) bytes) == -1)
            return -1;
         }
       }
    }

  return 1;
}


/*
      --------------------------------

      Function to write a JAM message.

      --------------------------------
*/


static sword EXPENTRY JAMWriteMsg(MSGH *msgh, word append, XMSG *msg, byte *text, dword textlen, dword totlen, dword clen, byte *ctxt)
{
  MSG *sq = msgh->sq;
  byte temp[80];
  byte *SubFieldPtr = NULL;

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
    if(!msg) return -1; /* We need a header! */

    if(msgh->mode != MOPEN_RW) // if MOPEN_RW we retain old header for update!
      {
      Init_JAMheader(sq);

      /* Set index values */
      JamData->Idx.HdrOffset = msgh->hdroffset;

      strcpy(temp, msg->to);
      strlwr(temp);
      JamData->Idx.UserCRC = JAMsysCrc32(temp, strlen(temp), -1L);

      /* Write out the index */

      if(JAMWriteIdx(sq, msgh->msgnum, &JamData->Idx) == -1)
         return -1; /* Write failed? */

      /* Convert header + kludges */

      if(totlen)
         {
         msgh->totlen = totlen;
         JamData->Hdr.TxtLen = totlen;
         }
      else JamData->Hdr.TxtLen = msgh->cur_len - msgh->add_size;

      JamData->Hdr.TxtOffset = msgh->txtoffset;

      if(clen)
         msgh->clen = clen;

      JamData->Hdr.MsgNum = msgh->msgnum;
      }

    // Convert XMSG structure, for MOPEN_RW only partially (attribs, links)

    SQ2JAM(msg, ctxt, sq, msgh->mode, &SubFieldPtr);

    /* Write out the header */

    if(JAMWriteHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
       {
       if(SubFieldPtr) free(SubFieldPtr);
       return -1;
       }

    if(msgh->mode == MOPEN_RW) // No Subfield/Text write then..
       return 0;

    /* Subfields */

    if(SubFieldPtr)
      {
      if(write(JamData->HdrHandle,
                 SubFieldPtr,
                 (unsigned) JamData->Hdr.SubfieldLen) !=
                                             (int) JamData->Hdr.SubfieldLen)
         {
         free(SubFieldPtr);
         return -1;
         }
      free(SubFieldPtr);
      }

    if(textlen)
      {
      if( (msgh->bytes_written + textlen) > totlen)  /* They want too much */
         return -1;
      if(write(JamData->TxtHandle, text, (size_t) textlen) != (size_t) textlen)
         return -1;
      msgh->bytes_written += textlen;
      }
    }
  else     /* append */
    {
    if(textlen)
      {
      if( (msgh->bytes_written + textlen) > msgh->totlen)  /* They want too much */
         return -1;
      if(write(JamData->TxtHandle, text, (size_t) textlen) != (size_t) textlen)
         return -1;
      msgh->bytes_written += textlen;
      }
    }

  return 0;

}



/*
      -------------------------------

      Function to kill a JAM message.

      -------------------------------
*/


static sword EXPENTRY JAMKillMsg(MSG *sq, dword msgnum)
{
  sword retval = -1;
  word  didlock = 0;

  if (InvalidMh(sq))
     return -1;

  if(!sq->locked)
    {
    if(JAMLock(sq) == -1)
       return -1;
    didlock = 1;
    }

  if( JAMReadIdx(sq, msgnum, &JamData->Idx) == -1 )
     goto Exit;

  if( JamData->Idx.HdrOffset == -1 )  // Deleted message!
      goto Exit;

  // Get header

  if(JAMReadHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
     goto Exit;

  if(JamData->Hdr.Attribute & MSG_DELETED)
     goto Exit;

  if(JamData->Hdr.MsgNum != msgnum)       // Weird! Corrupt?
     goto Exit;

  JamData->Hdr.Attribute |= MSG_DELETED;
  JamData->Hdr.TxtLen = 0;

  if(JAMWriteHeader(sq, JamData->Idx.HdrOffset, &JamData->Hdr) == -1)
       goto Exit;

  JamData->Idx.UserCRC = 0xFFFFFFFFL;

  if(JAMWriteIdx(sq, msgnum, &JamData->Idx) == -1)
       goto Exit;

  sq->num_msg--;

  if(msgnum == sq->high_msg) /* Last Msg deleted? New high_msg! */
    {
    sq->high_msg = JAMUidToMsgn(sq,
                 (dword) ((filelength(JamData->IdxHandle) / sizeof(JAMIDXREC))
                                           + JamData->HdrInfo.BaseMsgNum - 1),
                  UID_PREV);
    }

  JamData->HdrInfo.ActiveMsgs--;

  retval = 0;

  Exit:

  if(didlock)
     {
     if(JAMUnlock(sq) == -1)
        return -1;
     }

  return retval;
}


/*
      ----------------------------

      Function to lock a JAM area.

      ----------------------------
*/

static sword EXPENTRY JAMLock(MSG *sq)
{
  dword oldcounter = JamData->HdrInfo.ModCounter;

  if (InvalidMh(sq))
    return -1;

  /* Don't do anything if already locked */

  if (sq->locked)
    return 0;

  /* Set the flag in the _sqdata header */

  if(JAMmbUpdateHeaderInfo(sq, 0) == -1)
     return -1;

  if(mi.haveshare)
     {
     if(lock(JamData->HdrHandle, 0L, 1L) == -1)
        return -1;
     }

  sq->locked=TRUE;

  if(JamData->HdrInfo.ModCounter != oldcounter) /* Area was updated in meantime */
     {
     sq->num_msg  = JamData->HdrInfo.ActiveMsgs;
     sq->high_msg = JAMUidToMsgn(sq,
                 (dword) ((filelength(JamData->IdxHandle) / sizeof(JAMIDXREC))
                                           + JamData->HdrInfo.BaseMsgNum - 1),
                  UID_PREV);
     }

  return 0;
}


/*
      ------------------------------

      Function to unlock a JAM area.

      ------------------------------
*/

/* Undo the above "lock" operation */

static sword EXPENTRY JAMUnlock(MSG *sq)
{
  if (InvalidMh(sq))
    return -1;

  if (!sq->locked)
    return -1;

  if(JAMmbUpdateHeaderInfo(sq, 1) == -1)
     return -1;

  flush_handle2(JamData->IdxHandle);
  flush_handle2(JamData->TxtHandle);
  flush_handle2(JamData->HdrHandle);

  if(mi.haveshare)
     {
     if(unlock(JamData->HdrHandle, 0L, 1L) == -1)
        return -1;
     }

  sq->locked=FALSE;

  return 0;
}



/*
      --------------------------------

      Function to validate a JAM area.

      --------------------------------
*/

sword MSGAPI JAMValidate(byte *name)
{
   NW(name);

  return TRUE;
}


/*
      ----------------------------------------------

      Function to set current position in a JAM msg.

      ----------------------------------------------
*/


static sword EXPENTRY JAMSetCurPos(MSGH *msgh, dword pos)
{
   return msgh->cur_pos = pos;
}


/*
      ----------------------------------------------

      Function to get current position in a JAM msg.

      ----------------------------------------------
*/



static dword EXPENTRY JAMGetCurPos(MSGH *msgh)
{
  if (InvalidMsgh(msgh))
    return -1;

  return (msgh->cur_pos);
}


/*
      ------------------------------------

      Function to convert # to UID (fake).

      ------------------------------------
*/


static UMSGID EXPENTRY JAMMsgnToUid(MSG *sq, dword msgnum)
{

  if (InvalidMh(sq))
    return -1;

  return (UMSGID)msgnum;

}


/*
      -----------------------------------------------

      Function to convert UID to existing msg number.

      -----------------------------------------------
*/


static dword EXPENTRY JAMUidToMsgn(MSG *sq, UMSGID umsgid, word type)
{
  sdword size;
  dword curpos;
  dword found=0;


  if (InvalidMh(sq))
    return 0L;

  /* If number doesn't exist anymore, start at first existing msg */

  if(umsgid < JamData->HdrInfo.BaseMsgNum)
    {
    if(type != UID_NEXT)
       return 0L;
    else
       umsgid = JamData->HdrInfo.BaseMsgNum;
    }

   if( (size = filelength(JamData->IdxHandle) / sizeof(JAMIDXREC)) == 0L)
       return 0L;

   /* Maybe it is too high? */

   if(umsgid > (JamData->HdrInfo.BaseMsgNum + size - 1))
     {
     if(type != UID_PREV)
        return 0L;
     else
        umsgid = JamData->HdrInfo.BaseMsgNum + size - 1;
     }

   /* An exact match is easy to check */

   if(type == UID_EXACT)
     {
     if(JAMMsgExists(sq, umsgid))
        return umsgid;
     else
        return 0L;
     }

   /* Otherwise, we walk through the index until we find a non-deleted msg.. */

   curpos = umsgid;

   if(type == UID_NEXT) /* Look forward */
      {
      for( ; curpos < (JamData->HdrInfo.BaseMsgNum + size); curpos++)
        {
        if(JAMMsgExists(sq, curpos))
            {
            found = curpos;
            break;
            }
        }
      }

   else if(type == UID_PREV) /* Look backward.. */
      {
      for( ; curpos >= JamData->HdrInfo.BaseMsgNum; curpos--)
        {
        if(JAMMsgExists(sq, curpos))
            {
            found = curpos;
            break;
            }
        }
      }

   return (dword) found;
}

/*
      ---------------------------

      Function to get msg lenght.

      ---------------------------
*/

static dword EXPENTRY JAMGetTextLen(MSGH *msgh)
{
  return msgh->cur_len;
}

/*
      ------------------------------

      Function to get kludge length.

      ------------------------------
*/

static dword EXPENTRY JAMGetCtrlLen(MSGH *msgh)
{
  return msgh->clen;
}

/*
      --------------------------------

      Function to get high water mark.

      --------------------------------
*/

dword EXPENTRY JAMGetHighWater(MSG *sq)
{
  NW(sq);
  return 0;
}

/*
      ---------------------------

      Function to set high water.

      ---------------------------
*/

sword EXPENTRY JAMSetHighWater(MSG *sq, dword hwm)
{
  NW(sq);
  NW(hwm);
  return -1;
}


/*
      --------------------------------------------------------------------

      Function to convert JAM subfields to XMSG struct + 'normal' kludges.

      --------------------------------------------------------------------
*/

static void JAM2SQ(struct _msgh *msgh, byte *Subfields)
{
   JAMSUBFIELD *SFptr;
   char *bufptr;
   MSG *sq = msgh->sq;
   JAMTM *tmdate;
   word cursize = 1024, addsize = 512, copylen;
   sword SFleft = (sword) JamData->Hdr.SubfieldLen;


   if(msgh->mode == MOPEN_RW)      // Don't convert Subfields for MOPEN_RW,
     SFleft = 0;                   // can't write them anyway!

   msgh->hdr.replyto    = JamData->Hdr.ReplyTo;
   msgh->hdr.replies[0] = JamData->Hdr.Reply1st;
   msgh->hdr.replies[1] = JamData->Hdr.ReplyNext; // This needs work from the high level app!!

   msgh->cur_len = JamData->Hdr.TxtLen;

   /* Convert message attributes to XMSG standard */

   if(JamData->Hdr.Attribute & MSG_LOCAL)       msgh->hdr.attr |= MSGLOCAL;
   if(JamData->Hdr.Attribute & MSG_INTRANSIT)   msgh->hdr.attr |= MSGFWD;
   if(JamData->Hdr.Attribute & MSG_PRIVATE)     msgh->hdr.attr |= MSGPRIVATE;
   if(JamData->Hdr.Attribute & MSG_READ)        msgh->hdr.attr |= MSGREAD;
   if(JamData->Hdr.Attribute & MSG_SENT)        msgh->hdr.attr |= MSGSENT;
   if(JamData->Hdr.Attribute & MSG_KILLSENT)    msgh->hdr.attr |= MSGKILL;
   if(JamData->Hdr.Attribute & MSG_HOLD)        msgh->hdr.attr |= MSGHOLD;
   if(JamData->Hdr.Attribute & MSG_CRASH)       msgh->hdr.attr |= MSGCRASH;
   if(JamData->Hdr.Attribute & MSG_FILEREQUEST) msgh->hdr.attr |= MSGFRQ;
   if(JamData->Hdr.Attribute & MSG_FILEATTACH)  msgh->hdr.attr |= MSGFILE;
   if(JamData->Hdr.Attribute & MSG_ORPHAN)      msgh->hdr.attr |= MSGORPHAN;
   if(JamData->Hdr.Attribute & MSG_RECEIPTREQ)  msgh->hdr.attr |= MSGRRQ;

   if(JamData->Hdr.Attribute & MSG_CONFIRMREQ)  msgh->hdr.attr |= ADD_CFM;
   if(JamData->Hdr.Attribute & MSG_ARCHIVESENT) msgh->hdr.attr |= ADD_AS;
   if(JamData->Hdr.Attribute & MSG_DIRECT)      msgh->hdr.attr |= ADD_DIR;
   if(JamData->Hdr.Attribute & MSG_IMMEDIATE)   msgh->hdr.attr |= ADD_IMM;
   if(JamData->Hdr.Attribute & MSG_KILLFILE)    msgh->hdr.attr |= ADD_KFS;
   if(JamData->Hdr.Attribute & MSG_TRUNCFILE)   msgh->hdr.attr |= ADD_TFS;
   if(JamData->Hdr.Attribute & MSG_LOCKED)      msgh->hdr.attr |= ADD_LOK;

   tmdate = JAMsysLocalTime(&JamData->Hdr.DateWritten);
   TmDate_to_DosDate(tmdate, &msgh->hdr.date_written);

   tmdate = JAMsysLocalTime(&JamData->Hdr.DateProcessed);
   TmDate_to_DosDate(tmdate, &msgh->hdr.date_arrived);


   if (SFleft == 0) return; /* No subfields to process */

   if( (msgh->kludges = calloc(1, cursize) ) == NULL) return;
   msgh->clen = 1;

   if( (msgh->addtext = calloc(1, addsize) ) == NULL) return;

   SFptr = (JAMSUBFIELD *)Subfields;

   while(SFleft > 0)
     {
     bufptr = (char *)SFptr + sizeof(JAMBINSUBFIELD);
     switch(SFptr->LoID)
       {
       case JAMSFLD_OADDRESS:
         sscanf(bufptr, "%hd:%hd/%hd.%hd",
                        &msgh->hdr.orig.zone,
                        &msgh->hdr.orig.net,
                        &msgh->hdr.orig.node,
                        &msgh->hdr.orig.point);
         break;

       case JAMSFLD_DADDRESS:
         sscanf(bufptr, "%hd:%hd/%hd.%hd",
                        &msgh->hdr.dest.zone,
                        &msgh->hdr.dest.net,
                        &msgh->hdr.dest.node,
                        &msgh->hdr.dest.point);
         break;

       case JAMSFLD_SENDERNAME:
         copylen = min(35, SFptr->DatLen);
         memmove(&msgh->hdr.from, bufptr, copylen);
         break;

       case JAMSFLD_RECVRNAME:
         copylen = min(35, SFptr->DatLen);
         memmove(&msgh->hdr.to, bufptr, copylen);
         break;

       case JAMSFLD_SUBJECT:
         copylen = min(71, SFptr->DatLen);
         memmove(&msgh->hdr.subj, bufptr, copylen);
         break;

       case JAMSFLD_MSGID:
         addkludge(msgh, "\01MSGID: ", 8, bufptr, SFptr->DatLen, &cursize);
         break;

       case JAMSFLD_REPLYID:
         addkludge(msgh, "\01REPLY: ", 8, bufptr, SFptr->DatLen, &cursize);
         break;

       case JAMSFLD_PID:
         addkludge(msgh, "\01PID: ", 6, bufptr, SFptr->DatLen, &cursize);
         break;

       case JAMSFLD_FLAGS:
         addkludge(msgh, "\01FLAGS: ", 8, bufptr, SFptr->DatLen, &cursize);
         break;

       case JAMSFLD_TZUTCINFO:
         addkludge(msgh, "\01TZUTC: ", 8, bufptr, SFptr->DatLen, &cursize);
         break;

       case JAMSFLD_FTSKLUDGE:
         addkludge(msgh, "\01", 1, bufptr, SFptr->DatLen, &cursize);
         break;

       case JAMSFLD_SEENBY2D:
         addtext(msgh, "SEEN-BY: ", 9, bufptr, SFptr->DatLen, &addsize);
         break;

       case JAMSFLD_PATH2D:
         addtext(msgh, "\01PATH: ", 7, bufptr, SFptr->DatLen, &addsize);
         break;

       case JAMSFLD_TRACE:
         addtext(msgh, "\01Via: ", 6, bufptr, SFptr->DatLen, &addsize);
         break;

       case JAMSFLD_ENCLFILE:
       case JAMSFLD_ENCLFREQ:
       case JAMSFLD_ENCLFILEWC:

         if( (strlen(msgh->hdr.subj) + SFptr->DatLen + 1) < 72)
           {
           if(strlen(msgh->hdr.subj) > 0)
              strcat(msgh->hdr.subj, " ");
           strncat(msgh->hdr.subj, bufptr, SFptr->DatLen);
           }
         break;

       default:
           addkludge(msgh, "\01", 1, bufptr, SFptr->DatLen, &cursize);
           break;

       }

     SFleft -= (sizeof(JAMSUBFIELD) + SFptr->DatLen); /* How much left? */
     SFptr = (JAMSUBFIELD *) (bufptr + SFptr->DatLen); /* Point to next
                                                              SubField */

     }

}


/*
      --------------------------------------------------------------------

      Function to convert JAM subfields to XMSG struct + 'normal' kludges.
      Tiny version when only reading the header (timEd LIST mode)

      --------------------------------------------------------------------
*/

static void tinyJAM2SQ(struct _msgh *msgh, byte *Subfields)
{
   JAMSUBFIELD *SFptr;
   char *bufptr;
   MSG *sq = msgh->sq;
   JAMTM *tmdate;
   word cursize = 1024, addsize = 512, copylen;
   sword SFleft = (sword) JamData->Hdr.SubfieldLen;


   if (SFleft == 0) return; /* No subfields to process */

   SFptr = (JAMSUBFIELD *)Subfields;

   while(SFleft > 0)
     {
     bufptr = (char *)SFptr + sizeof(JAMBINSUBFIELD);
     switch(SFptr->LoID)
       {
       case JAMSFLD_SENDERNAME:
         copylen = min(35, SFptr->DatLen);
         memmove(&msgh->hdr.from, bufptr, copylen);
         break;

       case JAMSFLD_RECVRNAME:
         copylen = min(35, SFptr->DatLen);
         memmove(&msgh->hdr.to, bufptr, copylen);
         break;

       case JAMSFLD_SUBJECT:
         copylen = min(71, SFptr->DatLen);
         memmove(&msgh->hdr.subj, bufptr, copylen);
         break;

       case JAMSFLD_ENCLFILE:
       case JAMSFLD_ENCLFREQ:
       case JAMSFLD_ENCLFILEWC:

         if( (strlen(msgh->hdr.subj) + SFptr->DatLen + 1) < 72)
           {
           if(strlen(msgh->hdr.subj) > 0)
              strcat(msgh->hdr.subj, " ");
           strncat(msgh->hdr.subj, bufptr, SFptr->DatLen);
           }
         break;

       default:
           /* A big NOTHING! */
           break;

       }

     SFleft -= (sizeof(JAMSUBFIELD) + SFptr->DatLen); /* How much left? */
     SFptr = (JAMSUBFIELD *) (bufptr + SFptr->DatLen); /* Point to next
                                                              SubField */

     }

}


/*
      -----------------------------------------------------------------

      Function to add a kludge line to the kludges that are 'buffered'.

      -----------------------------------------------------------------
*/

void addkludge(struct _msgh *msgh, char *text, int txtlen, char *data, dword len, word *cursize)
{
   int thislen;
   char *oldkludges;

   if(len > 79) len = 79;

   thislen = txtlen + len;
   msgh->clen += thislen;
   if(msgh->clen > (*cursize - 10))
     {
     *cursize += 512;
     oldkludges = msgh->kludges;
     msgh->kludges = realloc(msgh->kludges, *cursize);
     if(msgh->kludges == NULL)        /* Out of memory!! */
       {
       msgh->kludges = oldkludges;
       msgh->clen -= thislen;
       return;
       }
     }

   memmove(msgh->kludges + msgh->clen - thislen - 1, text, txtlen);
   memmove(msgh->kludges + msgh->clen - thislen + txtlen - 1, data, len);
   *(msgh->kludges + msgh->clen - 1) = '\0';

}



/*
      -------------------------------------------------------

      Function to add text (seen-by + path) to msgh->addtext.

      -------------------------------------------------------
*/

void addtext(struct _msgh *msgh, char *stattext, int statlen, char *text, word len, word *cursize)
{
   int thislen;
   char *old;


   thislen = statlen + len + 1; /* +1 for '\r' */
   msgh->add_size += thislen;
   if(msgh->add_size > (*cursize-10))
     {
     *cursize += 512;
     old = msgh->addtext;
     msgh->addtext = realloc(msgh->addtext, *cursize);
     if(msgh->addtext == NULL)        /* Out of memory!! */
       {
       msgh->addtext = old;
       msgh->add_size -= thislen;
       *cursize -= 512;
       return;
       }
     }

   memmove(msgh->addtext + msgh->add_size - thislen, stattext, statlen);
   memmove(msgh->addtext + msgh->add_size - thislen + statlen, text, len);
   *(msgh->addtext + msgh->add_size - 1) = '\r';
   *(msgh->addtext + msgh->add_size) = '\0';

   msgh->cur_len += thislen;

}

/*
      ---------------------------------------------

      Function to initialize a JAM header (JAMHDR).

      ---------------------------------------------
*/


void Init_JAMheader(MSG *sq)
{

   memset(&JamData->Hdr, '\0', sizeof(JAMHDR));

   strcpy(JamData->Hdr.Signature, "JAM");
   JamData->Hdr.Revision    = CURRENTREVLEV;
   JamData->Hdr.MsgIdCRC    = -1L;
   JamData->Hdr.ReplyCRC    = -1L;
   JamData->Hdr.PasswordCRC = -1L;

}


/*
      ------------------------------------------------------

      Function to convert XMSG and kludges to JAM subfields.

      ------------------------------------------------------
*/

void SQ2JAM(XMSG *msg, char *ctxt, MSG *sq, word mode, byte **SubFieldPtr)
{
   struct tm tmdate;
   char temp[80];
   word position;
   char *kludgeptr, *endptr;
   word curlen = 1024;  // Current length of Subfields

   *SubFieldPtr = NULL;

   /* We start by converting XMSG header info.. */

   JamData->Hdr.ReplyTo   = msg->replyto;
   JamData->Hdr.Reply1st  = msg->replies[0];
   JamData->Hdr.ReplyNext = msg->replies[1];

   /* Convert message attributes to XMSG standard */

   JamData->Hdr.Attribute = 0L;  // Clean out first, for MOPEN_RW

   if(msg->attr & MSGLOCAL)   JamData->Hdr.Attribute |= MSG_LOCAL;
   if(msg->attr & MSGFWD)     JamData->Hdr.Attribute |= MSG_INTRANSIT;
   if(msg->attr & MSGPRIVATE) JamData->Hdr.Attribute |= MSG_PRIVATE;
   if(msg->attr & MSGREAD)    JamData->Hdr.Attribute |= MSG_READ;
   if(msg->attr & MSGSENT)    JamData->Hdr.Attribute |= MSG_SENT;
   if(msg->attr & MSGKILL)    JamData->Hdr.Attribute |= MSG_KILLSENT;
   if(msg->attr & MSGHOLD)    JamData->Hdr.Attribute |= MSG_HOLD;
   if(msg->attr & MSGCRASH)   JamData->Hdr.Attribute |= MSG_CRASH;
   if(msg->attr & MSGFRQ)     JamData->Hdr.Attribute |= MSG_FILEREQUEST;
   if(msg->attr & MSGFILE)    JamData->Hdr.Attribute |= MSG_FILEATTACH;
   if(msg->attr & MSGORPHAN)  JamData->Hdr.Attribute |= MSG_ORPHAN;
   if(msg->attr & MSGRRQ)     JamData->Hdr.Attribute |= MSG_RECEIPTREQ;

   if(msg->attr & ADD_AS)     JamData->Hdr.Attribute |= MSG_ARCHIVESENT;
   if(msg->attr & ADD_DIR)    JamData->Hdr.Attribute |= MSG_DIRECT;
   if(msg->attr & ADD_IMM)    JamData->Hdr.Attribute |= MSG_IMMEDIATE;
   if(msg->attr & ADD_KFS)    JamData->Hdr.Attribute |= MSG_KILLFILE;
   if(msg->attr & ADD_TFS)    JamData->Hdr.Attribute |= MSG_TRUNCFILE;
   if(msg->attr & ADD_LOK)    JamData->Hdr.Attribute |= MSG_LOCKED;
   if(msg->attr & ADD_CFM)    JamData->Hdr.Attribute |= MSG_CONFIRMREQ;

   if(sq->type & MSGTYPE_ECHO)
       JamData->Hdr.Attribute |= MSG_TYPEECHO;
   else if(sq->type & MSGTYPE_NET)
       JamData->Hdr.Attribute |= MSG_TYPENET;
   else
       JamData->Hdr.Attribute |= MSG_TYPELOCAL;

   DosDate_to_TmDate(&msg->date_written, &tmdate);
   JamData->Hdr.DateWritten = JAMsysMkTime(&tmdate);

   DosDate_to_TmDate(&msg->date_arrived, &tmdate);
   JamData->Hdr.DateProcessed = JAMsysMkTime(&tmdate);

   if(mode == MOPEN_RW) return;  // No need to convert Subfields..

   position = 0L;

   // We start with 1024 bytes - enough to hold items below. When we
   // start adding kludges, we'll check if it's enough every time

   if( (*SubFieldPtr = calloc(1, curlen)) == NULL )
      return;

   JAMmbAddField(*SubFieldPtr, JAMSFLD_SENDERNAME, strlen(msg->from), &position, msg->from);
   JAMmbAddField(*SubFieldPtr, JAMSFLD_RECVRNAME,  strlen(msg->to), &position, msg->to);

   if(sq->type & MSGTYPE_NET)
     {
     if(msg->orig.point == 0)
        sprintf(temp, "%d:%d/%d", msg->orig.zone,
                                  msg->orig.net,
                                  msg->orig.node);
     else
        sprintf(temp, "%d:%d/%d.%d", msg->orig.zone,
                                     msg->orig.net,
                                     msg->orig.node,
                                     msg->orig.point);

     JAMmbAddField(*SubFieldPtr, JAMSFLD_OADDRESS, strlen(temp), &position, temp);


     if(msg->dest.point == 0)
        sprintf(temp, "%d:%d/%d", msg->dest.zone,
                                  msg->dest.net,
                                  msg->dest.node);
     else
        sprintf(temp, "%d:%d/%d.%d", msg->dest.zone,
                                     msg->dest.net,
                                     msg->dest.node,
                                     msg->dest.point);

     JAMmbAddField(*SubFieldPtr, JAMSFLD_DADDRESS, strlen(temp), &position, temp);
     }

   if( (!(msg->attr & MSGFILE)) && (!(msg->attr & MSGFRQ)) )
      JAMmbAddField(*SubFieldPtr, JAMSFLD_SUBJECT, strlen(msg->subj), &position, msg->subj);
   else
      {
      if(msg->attr & MSGFILE)
         JAMmbAddField(*SubFieldPtr, JAMSFLD_ENCLFILE, strlen(msg->subj), &position, msg->subj);
      else
         JAMmbAddField(*SubFieldPtr, JAMSFLD_ENCLFREQ, strlen(msg->subj), &position, msg->subj);
      }


   /* Update SubFieldLen, just in case there are no kludges */

   JamData->Hdr.SubfieldLen = position;

   if(ctxt == NULL) return;

   kludgeptr = ctxt;
   while( (kludgeptr = strchr(kludgeptr, '\01')) != NULL )
     {
     // First we check the size of the memory block we use.
     // Realloc if too small, take 100 chars as a MAX for
     // Subfields to add. Add 10 bytes for LoID, HiID etc.

     if( (position + 110) > curlen)
        {
        curlen += 512;
        if( (*SubFieldPtr = realloc(*SubFieldPtr, curlen)) == NULL )
           return; // Out of mem :-(
        }

     endptr = strchr(kludgeptr+1, '\01');

     if(endptr == NULL) endptr = strchr(kludgeptr+1, '\0');

     if(!strncmp(kludgeptr+1, "MSGID: ", 7))
        {
        JAMmbAddField(*SubFieldPtr, JAMSFLD_MSGID, endptr-(kludgeptr+8),
                                                   &position, kludgeptr+8);
        memset(temp, '\0', sizeof(temp));
        memmove(temp, kludgeptr+8, endptr-(kludgeptr+8));
        strlwr(temp);
        JamData->Hdr.MsgIdCRC = JAMsysCrc32(temp, strlen(temp), -1L);
        }
     else if(!strncmp(kludgeptr+1, "REPLY: ", 7))
        {
        JAMmbAddField(*SubFieldPtr, JAMSFLD_REPLYID, endptr-(kludgeptr+8),
                                                   &position, kludgeptr+8);
        memset(temp, '\0', sizeof(temp));
        memmove(temp, kludgeptr+8, endptr-(kludgeptr+8));
        strlwr(temp);
        JamData->Hdr.ReplyCRC = JAMsysCrc32(temp, strlen(temp), -1L);
        }
     else if(!strncmp(kludgeptr+1, "PID: ", 5))
        {
        JAMmbAddField(*SubFieldPtr, JAMSFLD_PID, endptr-(kludgeptr+6),
                                                   &position, kludgeptr+6);
        }
     else if(!strncmp(kludgeptr+1, "TZUTC: ", 7))
        {
        JAMmbAddField(*SubFieldPtr, JAMSFLD_TZUTCINFO, endptr-(kludgeptr+8),
                                                   &position, kludgeptr+8);
        }
/*     else if(!strncmp(kludgeptr+1, "INTL: ", 6))
        {
            /* Do nothing, no INTL kludges in JAM base! But what about
               zonegating? */
        } */
     else if(!strncmp(kludgeptr+1, "FLAGS: ", 7))
        {
        JAMmbAddField(*SubFieldPtr, JAMSFLD_FLAGS, endptr-(kludgeptr+8),
                                                   &position, kludgeptr+8);
        }
     else JAMmbAddField(*SubFieldPtr, JAMSFLD_FTSKLUDGE, endptr-(kludgeptr+1),
                                                     &position, kludgeptr+1);

     kludgeptr++;
     }

   JamData->Hdr.SubfieldLen = position;
}

/*
   ---------------------------------------------------------------------

    Check if a certain message exists (non deleted), return 1 if Exists

   ---------------------------------------------------------------------
*/


byte JAMMsgExists(MSG *sq, UMSGID umsgid)
{
   JAMHDR hdr;

   /* Read index entry.. */

   if(JAMReadIdx(sq, umsgid, &JamData->Idx) == -1)
      return 0;

   /* If both are 0xFFFFFFFF there is no corresponding header.. */

   if( (JamData->Idx.UserCRC   == 0xFFFFFFFFL) &&
       (JamData->Idx.HdrOffset == 0xFFFFFFFFL) )
       return 0;

   if(JAMReadHeader(sq, JamData->Idx.HdrOffset, &hdr) == -1)
      return 0;

   if(hdr.Attribute & MSG_DELETED)
      return 0;
   else
      return 1;

}


/* ----------------------------------------------------------- */
/* - Function that opens all JAM files, creates if necessary - */
/* - Returns 0 on success, -1 on error.                      - */
/* ----------------------------------------------------------- */


int JAMmbOpen(MSG *sq, byte *tempname)
{
  char FileName[120], name[100];

  int      access = O_RDWR | O_BINARY | SH_DENYNO | O_CREAT;
  unsigned mode   = S_IREAD | S_IWRITE;


  strcpy(name, tempname);
  Strip_Trailing(name, '\\');

  JamData->HdrHandle =
   JamData->IdxHandle =
    JamData->TxtHandle =
     JamData->LrdHandle = -1;

  /* .JHR file */

  sprintf(FileName, "%s%s", name, EXT_HDRFILE);
  if((JamData->HdrHandle=open(FileName, access, mode)) == -1)
      return -1;

  if(filelength(JamData->HdrHandle) == 0L)  // Zero length, create new header
    {
    memset(&JamData->HdrInfo, 0, sizeof(JAMHDRINFO));
    strcpy(JamData->HdrInfo.Signature, HEADERSIGNATURE);
    JamData->HdrInfo.DateCreated=JAMsysTime(NULL);
    JamData->HdrInfo.PasswordCRC=0xffffffffL;
    JamData->HdrInfo.BaseMsgNum=1L;

    if (write(JamData->HdrHandle, &JamData->HdrInfo, sizeof(JAMHDRINFO))
                                                        !=sizeof(JAMHDRINFO))
        goto ErrorExit;
    }
  else
    if(JAMmbUpdateHeaderInfo(sq, 0) == -1)
        goto ErrorExit;

  /* .JDT file */

  sprintf(FileName, "%s%s", name, EXT_TXTFILE);
  if( (JamData->TxtHandle=open(FileName, access, mode)) == -1)
      goto ErrorExit;

  /* .JDX file */

  sprintf(FileName, "%s%s", name, EXT_IDXFILE);
  if( (JamData->IdxHandle=open(FileName, access, mode)) == -1)
      goto ErrorExit;

  /* .JLR file */

  sprintf(FileName, "%s%s", name, EXT_LRDFILE);
  if( (JamData->LrdHandle=open(FileName, access, mode)) == -1)
      goto ErrorExit;

  return 0;  // Success!

  ErrorExit:

  if(JamData->HdrHandle != -1) close(JamData->HdrHandle);
  if(JamData->IdxHandle != -1) close(JamData->IdxHandle);
  if(JamData->TxtHandle != -1) close(JamData->TxtHandle);
  if(JamData->LrdHandle != -1) close(JamData->LrdHandle);

  return -1;  // Something wrong :-(

}


/* -------------------------------------- */
/* - Function that closes all JAM files - */
/* -------------------------------------- */


int JAMmbClose(MSG *sq)
{
  /* Close all handles */

  close(JamData->HdrHandle);
  close(JamData->TxtHandle);
  close(JamData->IdxHandle);
  close(JamData->LrdHandle);

  return 0;
}


/* --------------------------------------------------- */
/* - Read a JAM header, check signature and revision - */
/* - Returns -1 on error, 0 if all went well.        - */
/* --------------------------------------------------- */

int JAMReadHeader(MSG *sq, long offset, JAMHDR *hdr)
{

    /* Fetch header */
    if(lseek(JamData->HdrHandle, offset, SEEK_SET) != offset)
        return -1;

    if(read(JamData->HdrHandle, hdr, sizeof(JAMHDR)) != sizeof(JAMHDR))
        return -1;

    /* Check header */
    if(strcmp(hdr->Signature, HEADERSIGNATURE) != 0)
        return -1;

    if(hdr->Revision != CURRENTREVLEV)
        return -1;

    return 0;
}


/* -------------------------------------------- */
/* - Write out a JAM header                   - */
/* - Returns -1 on error, 0 if all went well. - */
/* -------------------------------------------- */

int JAMWriteHeader(MSG *sq, long offset, JAMHDR *hdr)
{

    if(lseek(JamData->HdrHandle, offset, SEEK_SET) != offset)
        return -1;

    if(write(JamData->HdrHandle, hdr, sizeof(JAMHDR)) != sizeof(JAMHDR))
        return -1;

    return 0;
}


/* ------------------------------------------------------ */
/* - Read a JAM index entry, use 'cache' to speed up    - */
/* - repeated, sequential access (UidToMsgn for example - */
/* - Returns -1 on error, 0 if all went well.           - */
/* ------------------------------------------------------ */


int JAMReadIdx(MSG *sq, dword number, JAMIDXREC *idx)
{
   long WhatOffset;
   int  BytesRead;


    if( (JamData->IdxCache.active != 0)     &&
        (number <= JamData->IdxCache.high) &&
        (number >= JamData->IdxCache.base) &&
        (time(NULL) < (JamData->IdxCache.birth+5)) )      /* Cache hit? */

        {
        memmove(idx,
                &JamData->IdxCache.contents[number - JamData->IdxCache.base],
                sizeof(JAMIDXREC));

        return 0;
        }

    /* No cache hit, fill cache */

    JamData->IdxCache.active = 0; /* Invalidate contents */
    JamData->IdxCache.high   = 0;

    JamData->IdxCache.base = number - (IDXBUFSIZE/2);

    if( (JamData->IdxCache.base < JamData->HdrInfo.BaseMsgNum) ||
        ((IDXBUFSIZE/2) > number) )
       JamData->IdxCache.base = JamData->HdrInfo.BaseMsgNum;

    /* Fetch index record */

    WhatOffset=(JamData->IdxCache.base - JamData->HdrInfo.BaseMsgNum) * (long) sizeof(JAMIDXREC);
    if (lseek(JamData->IdxHandle, WhatOffset, SEEK_SET) != WhatOffset)
        return -1;

    if ((BytesRead = read(JamData->IdxHandle,
                          &JamData->IdxCache.contents[0],
                          sizeof(JAMIDXREC) * IDXBUFSIZE )) == -1)
        return -1;

    JamData->IdxCache.high = JamData->IdxCache.base +
                             (BytesRead/sizeof(JAMIDXREC)) - 1;

    if(BytesRead < sizeof(JAMIDXREC))
       return -1;

    JamData->IdxCache.birth = time(NULL);
    JamData->IdxCache.active = 1;

    memmove(idx,
            &JamData->IdxCache.contents[number - JamData->IdxCache.base],
            sizeof(JAMIDXREC));

    return 0;
}


/* -------------------------------------------- */
/* - Write a JAM index entry.                 - */
/* - Returns -1 on error, 0 if all went well. - */
/* -------------------------------------------- */


int JAMWriteIdx(MSG *sq, dword number, JAMIDXREC *idx)
{
   long offset;

    JamData->IdxCache.active = 0; /* Invalidate contents of cache! */
    JamData->IdxCache.high   = 0;

    /* Fetch index record */

    offset = (number - JamData->HdrInfo.BaseMsgNum) * (long) sizeof(JAMIDXREC);

    if(lseek(JamData->IdxHandle, offset, SEEK_SET) != offset)
        return -1;

    if(write(JamData->IdxHandle, idx, sizeof(JAMIDXREC)) != sizeof(JAMIDXREC))
        return -1;

    return 0;
}





/* =======================================================

       Here we add some functions stolen from JAM API

   ======================================================= */


/* ---------------------- */
/* - Add a JAM Subfield - */
/* ---------------------- */

/*
**  Add specified field to WorkBuf. It requires that POSITION has been
**  reset to zero (0) prior to the first call. The POSITION parameter is
**  updated to point to the first position after the newly added field.
*/

void JAMmbAddField(byte *start, dword WhatField,
                   size_t DatLen,
                   word * Position, byte * Data)
{
   JAMSUBFIELD *SubFieldPtr;

   if(DatLen > 100) return;

   SubFieldPtr=(JAMSUBFIELD *)(start + *Position);
   SubFieldPtr->LoID=(word)WhatField;
   SubFieldPtr->HiID=0;
   SubFieldPtr->DatLen=0L;
   *Position+= sizeof(JAMBINSUBFIELD);

   memcpy( (start + *Position), Data, DatLen);
   SubFieldPtr->DatLen+=DatLen;
   *Position+= DatLen;
}




int JAMmbUpdateHeaderInfo(MSG *sq, int WriteIt)
{

    /* Seek to beginning of file */
    if (lseek(JamData->HdrHandle, 0L, SEEK_SET)!=0L)
        return -1;

    /* Update ModCounter if told to*/
    if (WriteIt)
        {
        JamData->HdrInfo.ModCounter++;

        if (JamData->HdrInfo.BaseMsgNum == 0L)
            JamData->HdrInfo.BaseMsgNum = 1L;

        /* Update header info record */
        if (write(JamData->HdrHandle, &JamData->HdrInfo, sizeof(JAMHDRINFO))!= sizeof(JAMHDRINFO))
            {
            JamData->HdrInfo.ModCounter--;
            return -1;
            }
        }
    else
        /* Fetch header info record */
        {
        if (read(JamData->HdrHandle, &JamData->HdrInfo, sizeof(JAMHDRINFO))!= sizeof(JAMHDRINFO))
            return -1;

        if (JamData->HdrInfo.BaseMsgNum == 0L)
            JamData->HdrInfo.BaseMsgNum = 1L;
        }

    return 0;
}


/*
**  JAMsysTime
**
**  Calculates the number of seconds that has passed from January 1, 1970
**  until the current date and time.
**
**      Parameters:   UINT32ptr pTime     - Ptr to buffer where the number
**                                          number of seconds will be stored
**
**         Returns:   Number of seconds since 1970 to specified date/time
*/
dword JAMsysTime(dword * pTime)
{
        struct date   d;
        struct time   t;
        struct JAMtm  m;
        dword        ti;

        getdate(&d);
        gettime(&t);

        m.tm_year = d.da_year - 1900;
        m.tm_mon  = d.da_mon - 1;
        m.tm_mday = d.da_day;
        m.tm_hour = t.ti_hour;
        m.tm_min  = t.ti_min;
        m.tm_sec  = t.ti_sec;

    ti = JAMsysMkTime(&m);
    if(pTime)
        *pTime = ti;

    return(ti);
}

/*
**  JAMsysMkTime
**
**  Calculates the number of seconds that has passed from January 1, 1970
**  until the specified date and time.
**
**      Parameters:   JAMTMptr pTm        - Ptr to structure containing time
**
**         Returns:   Number of seconds since 1970 to specified date/time
*/
dword JAMsysMkTime(JAMTM * pTm)
{
    dword  Days;
    int     Years;

    /*Get number of years since 1970*/
    Years = pTm->tm_year - 70;

    /*Calculate number of days during these years,*/
    /*including extra days for leap years         */
    Days = Years * 365 + ((Years + 1) / 4);

    /*Add the number of days during this year*/
    Days += _mdays [pTm->tm_mon] + pTm->tm_mday - 1;
    if((pTm->tm_year & 3) == 0 && pTm->tm_mon > 1)
        Days++;

    /*Convert to seconds, and add the number of seconds this day*/
    return(((dword) Days * 86400L) + ((dword) pTm->tm_hour * 3600L) +
           ((dword) pTm->tm_min * 60L) + (dword) pTm->tm_sec);
}

/*
**  JAMsysLocalTime
**
**  Converts the specified number of seconds since January 1, 1970, to
**  the corresponding date and time.
**
**      Parameters:   dword * pt        - Number of seconds since Jan 1, 1970
**
**         Returns:   Ptr to struct JAMtm area containing date and time
*/

JAMTM * JAMsysLocalTime(dword * pt)
{
    static struct JAMtm   m;
    sdword                 t = *pt;
    int                   LeapDay;

    m.tm_sec  = (int) (t % 60); t /= 60;
    m.tm_min  = (int) (t % 60); t /= 60;
    m.tm_hour = (int) (t % 24); t /= 24;
    m.tm_wday = (int) ((t + 4) % 7);

    m.tm_year = (int) (t / 365 + 1);
    do
        {
        m.tm_year--;
        m.tm_yday = (int) (t - m.tm_year * 365 - ((m.tm_year + 1) / 4));
        }
    while(m.tm_yday < 0);
    m.tm_year += 70;

    LeapDay = ((m.tm_year & 3) == 0 && m.tm_yday >= _mdays [2]);

    for(m.tm_mon = m.tm_mday = 0; m.tm_mday == 0; m.tm_mon++)
        if(m.tm_yday < _mdays [m.tm_mon + 1] + LeapDay)
            m.tm_mday = m.tm_yday + 1 - (_mdays [m.tm_mon] + (m.tm_mon != 1 ? LeapDay : 0));

    m.tm_mon--;

    m.tm_isdst = -1;

    return(&m);
}


/*
**  Fetch LastRead for passed UserID. Returns 1 upon success and 0 upon
**  failure.
*/

dword JAMmbFetchLastRead(MSG *sq, dword UserCRC)
{
    sdword ReadCount;
    dword LastReadRec;
    JAMLREAD new;

    begin:

    /* Seek to beginning of file */

    if (lseek(JamData->LrdHandle, 0L, SEEK_SET) != 0L)
        return -1;

    /* Read file from top to bottom */

    LastReadRec=0L;
    while (1)
        {
        ReadCount=read(JamData->LrdHandle, &new, (sdword)sizeof(JAMLREAD));
        if (ReadCount!=(sdword)sizeof(JAMLREAD))
            {
            if (!ReadCount)
                /* End of file */
                {
                memset(&new, '\0', sizeof(JAMLREAD));
                new.UserCRC = UserCRC;
                new.UserID  = UserCRC;
                write(JamData->LrdHandle, &new, sizeof(JAMLREAD));
                goto begin;
                }
            else
                /* Read error */
                return -1;
            }

        /* See if it matches what we want */
        if(new.UserCRC == UserCRC)
          {
          JamData->lastread = LastReadRec;
          return new.HighReadMsg;
          }

        /* Next record number */
        LastReadRec++;
        }/*while*/

}

/*
**  Store LastRead record and if successful, optionally updates the header
**  info block (and its ModCounter) at the beginning of the header file.
**  Returns 1 upon success and 0 upon failure.
*/

int JAMmbStoreLastRead(MSG *sq, dword last, dword CRC)
{
    sdword UserOffset;
    JAMLREAD new;


    /* Seek to the appropriate position */

    UserOffset=(sdword)(JamData->lastread * (sdword)sizeof(JAMLREAD));
    if (lseek(JamData->LrdHandle, UserOffset, SEEK_SET)!=UserOffset)
        return -1;

    memset(&new, '\0', sizeof(JAMLREAD));
    new.UserCRC = CRC;
    new.UserID  = CRC;
    new.HighReadMsg = last;
    new.LastReadMsg = last;

    /* Write record */
    if (write(JamData->LrdHandle, &new, (sdword)sizeof(JAMLREAD))!=(sdword)sizeof(JAMLREAD))
        return -1;

    return 0;
}

