#define MSGAPI_HANDLERS

#ifndef __GNUC__
#include <mem.h>
#include <io.h>
#include <dos.h>
#include <share.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "dr.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "prog.h"
#include "alc.h"
#include "max.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sq.h"
#include "api_sqp.h"
#include "apidebug.h"
#include "sqsdm.h"


// Some function added by GvE

int  BuildUidlist(MSG *sq);
int  RemoveFromIndex(MSG *sq, dword msgnum);
int  SquishCalcTrailing(MSGH *msgh);
//int  AddToString(char **start, unsigned *curmax, unsigned *cursize, char *string);
void MIS2SQ(MIS *mis, XMSG *xmsg);
void SQ2MIS(MSGH *msgh, XMSG *xmsg, MIS *mis);


extern dword SQAttrTable1[SQNUMATTR1][2];

// End added funcs


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


MSG * SquishOpenArea(byte *name, word mode, word type)
{
  struct _sqbase sqbase;
  MSG * sq;


  if ((sq=CreateAreaHandle(type))==NULL)
    return NULL;

  *sq->api=sq_funcs;

  if ((sq->apidata=(void *)calloc(1, sizeof(struct _sqdata)))==NULL)
  {
    free(sq->api);
    free(sq);
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  Sqd->uid=1L;
  strcpy(Sqd->base, name);
  if (! _OpenSquish(sq, &mode))
  {
    free(sq->api);
    free((char *)sq->apidata);
    free(sq);
    return NULL;
  }

  if (mode==MSGAREA_CREATE)
  {
    memset(&sqbase, '\0', sizeof(struct _sqbase));

    sqbase.len=sizeof(struct _sqbase);
    sqbase.end_frame=sqbase.len;

    sqbase.sz_sqhdr=sizeof(SQHDR);
    sqbase.uid=1L;
  }
  else
  {
    if (read(Sqd->sfd, (char *)&sqbase, sizeof sqbase) != sizeof sqbase)
    {
      /* The base must be locked or corrupted, so return an error */

      close(Sqd->sfd);
      close(Sqd->ifd);

      free(sq->api);
      free((char *)sq->apidata);
      free(sq);

      msgapierr=MERR_BADF;
      return NULL;
    }
  }


  Sqd->delta=sqbase;

  strcpy(sqbase.base,name);
  SqbaseToSq(&sqbase,sq);

  Sqd->next_frame=Sqd->begin_frame;
  Sqd->prev_frame=NULL_FRAME;
  Sqd->cur_frame=NULL_FRAME;

  #ifndef __FLAT__
  if(sq->num_msg > 16380 || BuildUidlist(sq) == -1)
  #else
  if(BuildUidlist(sq) == -1)
  #endif
    {
    #ifndef __FLAT__
    if(sq->num_msg > 16380)
       msgapierr = MERR_TOOMANYMSG;
    #endif
    close(Sqd->sfd);
    close(Sqd->ifd);

    free(sq->api);
    free((char *)sq->apidata);
    free(sq);

    return NULL;
    }

  if (mode==MSGAREA_CREATE)
    _SquishWriteSq(sq);

  return sq;
}



/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword  SquishCloseArea(MSG *sq)
{
  if (InvalidMh(sq))
    return -1;

  if (sq->locked)
    SquishUnlock(sq);

  _SquishUpdateSq(sq, TRUE);

  if (Sqd->msgs_open)
  {
    msgapierr=MERR_EOPEN;
    return -1;
  }

  if(Sqd->uidlist) free(Sqd->uidlist);

  close(Sqd->sfd);
  close(Sqd->ifd);

  free(sq->api);
  free((char *)sq->apidata);
  sq->id=0L;
  free(sq);

  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */



MSGH *  SquishOpenMsg(MSG *sq, word mode, dword msgnum)
{
  struct _msgh *msgh;


  if (InvalidMh(sq))
    return NULL;

  if (mode==MOPEN_CREATE)
  {
    #ifndef __FLAT__
    if(sq->num_msg >= 16380)
      {
      msgapierr = MERR_TOOMANYMSG;
      return NULL;
      }
    #endif

    if ((sdword)msgnum < 0 || msgnum > sq->num_msg)
    {
      msgapierr=MERR_NOENT;
      return NULL;
    }

    if ((msgh=calloc(1, sizeof(struct _msgh)))==NULL)
    {
      msgapierr=MERR_NOMEM;
      return NULL;
    }

    msgh->sq=sq;
    msgh->msgnum=msgnum;
  }
  else if (msgnum==0)
  {
    msgapierr=MERR_NOENT;
    return NULL;
  }
  else
    {
    if((msgh=_SquishOpenMsgRead(sq, mode, msgnum))==NULL)
      return NULL;

    msgh->relnum = msgnum;

    if(mode != MOPEN_RDHDR)
      SquishCalcTrailing(msgh);
    }

  msgh->mode=mode;
  msgh->id=MSGH_ID;

  MsghSqd->msgs_open++;

  return (MSGH *)msgh;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

sword  SquishCloseMsg(MSGH *msgh)
{
  int retval = 0;
  char ch='\0';


  if (InvalidMsgh(msgh))
    return -1;

  if(msgh->mode==MOPEN_CREATE)
    {
  
    lseek(MsghSqd->sfd,
          msgh->seek_frame + (dword)sizeof(SQHDR) + sizeof(XMSG) +
          msgh->clen + (dword)msgh->bytes_written,
          SEEK_SET);


     if(msgh->trail != NULL)
      {
      if(write(MsghSqd->sfd, msgh->trail, msgh->trailsize+1) == -1)
         retval = -1;
      }
    else
      {
      if(msgh->totlen != 0L)           // Only if not empty message..
         write(MsghSqd->sfd, &ch, 1);
      }

    }

  MsghSqd->msgs_open--;

  if(msgh->trail) free(msgh->trail);

  if (msgh->hdr)
    free(msgh->hdr);

  FreeMIS(&msgh->mis);

  msgh->id=0L;
  free(msgh);

  return retval;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


dword  SquishReadMsg(MSGH *msgh, MIS *mis, dword offset, dword bytes,
                                           byte *text, dword clen, byte *ctxt)
{
  dword bytesread=0L;
  XMSG xmsg;


  if (InvalidMsgh(msgh))
    return -1;

  if (mis)
    {
    if(lseek(MsghSqd->sfd,
             (long)((long)msgh->seek_frame+(dword)MsghSqd->sz_sqhdr),
             SEEK_SET) != (long)((long)msgh->seek_frame+(dword)MsghSqd->sz_sqhdr))
             return -1;

    if(read(MsghSqd->sfd, (char *)&xmsg, (unsigned) sizeof(XMSG))
               != (int) sizeof(XMSG))
             return -1;

    SQ2MIS(msgh, &xmsg, &msgh->mis);

    // Now copy the result to the given MIS structure..
    CopyMIS(&msgh->mis, mis);

    /* If the _xmsg struct has been expanded, then seek past the junk we    *
     * don't yet know about.                                                */

    if(msgh->sq->sz_xmsg != sizeof(XMSG))
      lseek(MsghSqd->sfd, (long) ((long)msgh->sq->sz_xmsg - (long)sizeof(XMSG)), SEEK_CUR);
    }

  if (bytes > msgh->hdr->msg_length - sizeof(XMSG) - offset - msgh->clen)
  {
    bytes=msgh->hdr->msg_length - sizeof(XMSG) - offset - msgh->clen;
  }

  if (! (text && bytes) && clen==0)
    return 0;

  switch (msgh->hdr->frame_type)
  {
    case FRAME_normal:
      msgh->cur_pos=offset;

      if (clen && ctxt)
      {
        if (lseek(MsghSqd->sfd,
                  (long) ((long)msgh->seek_frame+(long)MsghSqd->sz_sqhdr+
                  (long)msgh->sq->sz_xmsg),
                  SEEK_SET) == -1L)
        {
          msgapierr=MERR_BADF;
          return -1;
        }

        if(read(MsghSqd->sfd, ctxt, (unsigned)min(msgh->clen, clen)) !=
                            (int)min(msgh->clen, clen))
               {
               return -1;
               }

        if(mis && ctxt)
           Convert_Flags(ctxt, &mis->attr1, &mis->attr2);

        /* Skip over rest of control info */

        if (clen < msgh->clen && bytes && text)
          lseek(MsghSqd->sfd, (long)((long)msgh->clen-(long)clen), SEEK_CUR);

        bytesread=0;
      }
      else if (lseek(MsghSqd->sfd,
                     (long)(
                     (long)msgh->seek_frame + (dword)MsghSqd->sz_sqhdr +
                     (dword)msgh->sq->sz_xmsg + (long)msgh->clen+
                     (long)msgh->cur_pos
                     ),
                     SEEK_SET) == -1L)
      {
        msgapierr=MERR_BADF;
        return -1;
      }

      if (bytes && text)
      {
        bytesread=(dword)read(MsghSqd->sfd,text,(unsigned)bytes);
        msgh->cur_pos += (dword)bytesread;
      }

      return bytesread;

    default:
      msgapierr=MERR_BADF;
      return -1;
  }
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword  SquishWriteMsg(MSGH *msgh, word append, MIS *mis, byte *text, dword textlen, dword totlen, dword clen, byte *ctxt)
{
  MSG *sq = msgh->sq;

  SQIDX idxe, here;

  XMSG  xmsg;

  SQHDR freehdr;
  SQHDR newhdr;
  SQHDR save, lhdr, *ohdr = NULL;

  FOFS oldofs = 0L;
  FOFS this_frame, last_frame;
  FOFS seek;

  STRINGLIST *current;
  dword trailmax;

  word insert=FALSE;
  word ctlen;

  char temp[150];
  char *newkludges = NULL;


  if (InvalidMsgh(msgh))
    return -1;

  if(!sq->locked)
    return -1;

  Init_Hdr(&freehdr);
  Init_Hdr(&lhdr);
  Init_Hdr(&newhdr);

  memset(&xmsg, '\0', sizeof(XMSG));

  if (!ctxt)
    clen=0L;

  if (!text)
    textlen=0L;

  if (textlen==0L)
    text=NULL;

  if (clen==0L)
    ctxt=NULL;

  if (msgh->mode != MOPEN_CREATE)
  {
    msgh->bytes_written=0L;
    append=TRUE;
  }


  /* First we check for any 'FLAGS' kludges to be written.. */

  if((sq->type & (MSGTYPE_NET|MSGTYPE_MAIL)) && mis && (clen || !append) )  /* Do we have to do it? */
    {
    if((mis->attr1 & SQADDMASK1) || (mis->attr2 & SQADDMASK2))
      {
      if(!mi.useflags) /* We can only simulate Direct :-( */
         {
         if(mis->attr1 & aDIR)
            xmsg.attr |= (MSGHOLD | MSGCRASH);
         }
      else
         {
         Attr2Flags(temp, (mis->attr1 & SQADDMASK1), (mis->attr2 & SQADDMASK2));

         if(clen == 0)
            clen = 1; /* For trailing '\0'! */

         newkludges = calloc(1, clen + strlen(temp));
         if(newkludges)
            {
            if(ctxt)
               strcpy(newkludges, ctxt);
            strcat(newkludges, temp);
            }
         }
      }
    }

  /* Make sure that we don't write any more than we have to. */

  if (clen && (dword)(ctlen=strlen(ctxt)) < clen)
    clen=ctlen+1;

   /* Change clen if we have new kludges with FLAGS, leave 0 if RW */
  if( (newkludges) && (msgh->mode != MOPEN_RW) )
    clen = strlen(newkludges)+1;

  // Now we have to prepare the trailing stuff (SEEN-BY, PATH, VIA).
  // It is passed separately by the higher level app, but in Squish
  // areas it is part of the body, so we have to add that...

  if(mis && !append && (mis->seenby || mis->path || mis->via))
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
      if(!(sq->type & (MSGTYPE_MAIL|MSGTYPE_NET)))
         break;
      AddToString(&msgh->trail, &trailmax, &msgh->trailsize, "\r\01Via ");
      AddToString(&msgh->trail, &trailmax, &msgh->trailsize, current->s);
      }

    totlen += msgh->trailsize;
    }

  // Convert the MIS structure to a XMSG header.

  if( (mis && !append) || (msgh->mode == MOPEN_RW))
    MIS2SQ(mis, &xmsg);

  if (append)
  {
    /* The control info can only be written on the first pass, so blank     *
     * it out if we're appending.                                           */

    if (clen)
      clen=0L;

    if (ctxt)
      ctxt=NULL;
  }
  else
  {
    sword res;

    if(totlen != 0L) totlen++;   // For trailing '\0'.
    msgh->totlen=totlen;

    if ((res=_SquishFindFree(sq, &this_frame, totlen, clen, &freehdr,
                             &last_frame, &lhdr, msgh)) != 0)
      {
      if(newkludges)
         free(newkludges);
      return res;
      }

  /**************************************************************************
                     Update the Message Pointer chain:
   **************************************************************************/

    /* 0 means automatically use last message number */

    if (msgh->msgnum==0)
      msgh->msgnum=sq->num_msg+1;

/**/


/**/

    if (msgh->msgnum==sq->num_msg+1 ||
        (ohdr=_SquishGotoMsg(sq, msgh->msgnum, &oldofs, &here, FALSE))==NULL)
    {
      insert=FALSE; /* we're just appending */

      msgh->msgnum=sq->num_msg+1;

      /* There is no next frame, since this is the last message... */

      newhdr.next_frame=NULL_FRAME;

      /* ...and the previous frame should be the former last message. */

      newhdr.prev_frame=Sqd->last_frame;

      /* Now update the former-last-message's frame, to say that we come    *
       * after it.                                                          */

      _SquishUpdateHeaderNext(sq, newhdr.prev_frame, &lhdr, this_frame);
    }
    else /* we're rewriting an 'old' message */
    {
      insert=TRUE; /* inserting a msg in middle of area */

      here.ofs=this_frame;

      /* Update the 'to' field, if necessary */

      if (mis)
      {
        here.hash=SquishHash(xmsg.to);

        if (xmsg.attr & MSGREAD)
          here.hash |= 0x80000000Lu;

        // Add UID to msg header (Sq 1.10 - GvE).
        xmsg.attr |= MSGUID;
        xmsg.replies[9] = here.umsgid;
      }

      newhdr.next_frame=ohdr->next_frame;    // We'll link in this frame in
      newhdr.prev_frame=ohdr->prev_frame;    // the location of the old frame

      if (AddIndex(sq, &here, msgh->msgnum-1)==-1)   // !!!!!  ||
// !!!!!         Add_To_Free_Chain(sq, oldofs, ohdr)==-1)
      {
        free(ohdr);

        if(newkludges)
           free(newkludges);

        return -1;
      }

      _SquishUpdateHeaderNext(sq, newhdr.prev_frame, &save, this_frame);
      _SquishUpdateHeaderPrev(sq, newhdr.next_frame, &save, this_frame);

      if (Sqd->begin_frame==oldofs)
        Sqd->begin_frame=this_frame;

      if (Sqd->last_frame==oldofs)
        Sqd->last_frame=this_frame;

//  !!!!!      free(ohdr);

      if (Sqd->cur_frame==oldofs)
        Sqd->cur_frame=this_frame;
    }


/**/

  /**************************************************************************
                       Update the Free Pointer chain:
   **************************************************************************/

    /* If this was the first frame (ie. the first one pointed to by        *
     * sq->free_frame, which means that the first frame found was long     *
     * enough to hold the message), then set the free pointer to the       *
     * start of the new free chain.                                        */

    if (this_frame==Sqd->free_frame)
      Sqd->free_frame=freehdr.next_frame;

    if (this_frame==Sqd->last_free_frame)
      Sqd->last_free_frame=freehdr.prev_frame;

    /* Now update the linked list of free frames, to remove the current    *
     * frame from the free-frame list, if necessary.  We only need to do   *
     * this if the current frame wasn't just being appended to the .SQD    *
     * file, since there would be no links to update in that case.         */

    if (this_frame != Sqd->end_frame)
    {
      _SquishUpdateHeaderNext(sq, freehdr.prev_frame, &lhdr,
                              freehdr.next_frame);

      _SquishUpdateHeaderPrev(sq, freehdr.next_frame, &lhdr,
                              freehdr.prev_frame);
    }

/**/

// GvE: If we were rewriting an old message, we only NOW add the old frame
//      to the list of free frames. The MSGAPI did that much earlier, at
//      a moment where it was also manipulating the linked list of free
//      frames in case it was re-using space of older deleted messages.
//      It kept info of a 'rescued' frame in memory, while possibly mani-
//      pulating that same frame (!) on disk. Oops.

   if(oldofs != 0L && ohdr != NULL)
      {
      Add_To_Free_Chain(sq, oldofs, ohdr);
      free(ohdr);
      ohdr = NULL;
      oldofs = 0L;
      }


// End GvE

    /* If we're writing into the middle of the file, then we shouldn't     *
     * update the frame length, as the frame may be longer than the actual *
     * message, and we'd lose some space that way.  However, if we're at   *
     * the end of the file, then we're creating this frame, so it is       *
     * by default the length of the message.                               */

    if (this_frame != Sqd->end_frame)
      newhdr.frame_length=freehdr.frame_length;
    else
    {
      newhdr.frame_length=totlen + clen + (dword)sizeof(XMSG);

      /* While we're at it, since we're writing at EOF, state              *
       * where the new EOF is.                                             */

      Sqd->end_frame=this_frame + (dword)MsghSqd->sz_sqhdr +
                     (dword)sizeof(XMSG) + clen + totlen;
    }


    /* Tell the system that this is now the current last message */

    if (!append && !insert)
      Sqd->last_frame=this_frame;

    newhdr.msg_length=(dword)sizeof(XMSG) + clen + totlen;
    newhdr.clen=clen;

    newhdr.frame_type=FRAME_normal;

    _SquishWriteHeader(sq, this_frame, &newhdr);

    /* If no messages exist, point the head of the message chain to         *
     * this one.                                                            */

    if (Sqd->begin_frame==NULL_FRAME)
      Sqd->begin_frame=this_frame;

    if (Sqd->next_frame==NULL_FRAME)
      Sqd->next_frame=this_frame;

    msgh->seek_frame=this_frame;
  }


  seek=msgh->seek_frame + MsghSqd->sz_sqhdr;


  /* Now with the pointer manipulation over with, write the message         *
   * header...                                                              */

  if(mis)
  {
    if (tell(Sqd->sfd) != seek)
      lseek(Sqd->sfd, seek, SEEK_SET);

    // ---- Hacked on later to fill out UID added in Squish v1.10!  GvE

    if(!append && !insert)
      {
      xmsg.attr       |= MSGUID;
      xmsg.replies[9]  = Sqd->uid;
      }

    write(Sqd->sfd, (char *)&xmsg, sizeof(XMSG));
  }

  seek += sizeof(XMSG);

  if (!append)
  {
    if ((ctxt && clen) || (clen && newkludges))
    {
      if (tell(Sqd->sfd) != seek)
        lseek(Sqd->sfd, seek, SEEK_SET);

      if(!newkludges)
         write(Sqd->sfd, (char *)ctxt, (unsigned)clen);
      else
         {
         write(Sqd->sfd, (char *)newkludges, (unsigned)clen);
         free(newkludges);
         newkludges = NULL;
         }
    }
    else clen=0L;

    msgh->clen=clen;
  }

  seek += msgh->clen;

  if (!append)
    msgh->bytes_written=0L;

  if (text)
  {
    dword howmuch;

    if (append)
      seek += msgh->bytes_written;

    if (tell(Sqd->sfd) != seek)
      lseek(Sqd->sfd, seek, SEEK_SET);

    /* And write the message text itself!  Just don't let the app write     *
     * any more than it said that it was going to.                          */

    howmuch=(dword)(msgh->totlen - msgh->bytes_written);
    howmuch=min(howmuch, (dword)textlen);

    if (howmuch)
      if (write(Sqd->sfd, (char *)text, (unsigned)howmuch) != (int)howmuch)
      {
        msgapierr=MERR_NODS;
        return -1;
      }

    msgh->bytes_written += howmuch;

    seek += textlen;
  }

  if (!append)
  {
    /* Inc number of messages, and write into right spot in index file.   *
     * (Frame location of msg #1 is stored at offset 0*sizeof(dword) in   *
     * the index, msg #2's offset is stored at offset 1*sizeof(dword),    *
     * etc.                                                               */

    memset(&idxe, '\0', sizeof(SQIDX));

    idxe.hash=SquishHash(xmsg.to);
    idxe.ofs=msgh->seek_frame;

    if (xmsg.attr & MSGREAD)
      idxe.hash |= 0x80000000Lu;

    idxe.umsgid=Sqd->uid++;

    /* Add the message to the .SQI file, as long as we didn't do it         *
     * when we were re-writing the message links...                         */

    if (msgh->msgnum==sq->num_msg+1 && AddIndex(sq, &idxe, sq->num_msg)==-1)
    {
      return -1;
    }


    /* If we just created a new message, increment the lastread pointer */

    if (!insert)
    {
      sq->num_msg++;
      sq->high_msg++;
    }


    /* Update the header in the SQD file, too */

    _SquishUpdateSq(sq, FALSE);
  }

  return 0;
}



/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */



sword  SquishKillMsg(MSG *sq, dword msgnum)
{
  SQIDX killframe;
  FOFS killofs;
  SQHDR killhdr, lhdr, *hd;
  int didlock = 0;


  if (InvalidMh(sq))
    return -1;

  if(!sq->locked)
    {
    if(SquishLock(sq) == -1)
       {
       msgapierr = MERR_NOLOCK;
       return -1;
       }
    didlock = 1;
    }

  Init_Hdr(&killhdr);
  Init_Hdr(&lhdr);

  /* Make the message number zero-based, so we can index into msgoffs[] */
  msgnum--;

  /* If the seek fails, the index isn't read correctly, or the index   *
   * doeesn't point to a valid message number, then report error.  Use *
   * special-case code for the first message, to make things faster.   */

  if (msgnum==0L)
    killofs=Sqd->begin_frame;
  else
  {
    if (_SquishGetIdxFrame(sq, msgnum, &killframe)==-1)
    {
      msgapierr=MERR_BADF;
      if(didlock) SquishUnlock(sq);
      return -1;
    }

    killofs=killframe.ofs;
  }

  if (_SquishReadHeader(sq, killofs, &killhdr)==-1)
  {
    msgapierr=MERR_BADF;
    if(didlock) SquishUnlock(sq);
    return -1;
  }

  /* If we don't have the index in memory already, grab it from disk */

//  if (!sq->locked &&
//      _SquishReadIndex(sq)==-1)
//  {
//    return -1;
//  }



  /*************************************************************************
                   Update the Message Pointer chain:
   *************************************************************************/

  /* Fix the back/previous link */

  _SquishUpdateHeaderNext(sq, killhdr.prev_frame, &lhdr, killhdr.next_frame);

  /* Fix the forward/next link */

  _SquishUpdateHeaderPrev(sq, killhdr.next_frame, &lhdr, killhdr.prev_frame);

  /* If we delete the 1st msg in area, update the begin pointer */

  if (Sqd->begin_frame==killofs)
     Sqd->begin_frame=killhdr.next_frame;

  if (Sqd->last_frame==killofs)
    Sqd->last_frame=killhdr.prev_frame;

  if (Sqd->next_frame==killofs)
    Sqd->next_frame=killhdr.next_frame;

  if (Sqd->prev_frame==killofs)
    Sqd->prev_frame=killhdr.prev_frame;

  if (Sqd->cur_frame==killofs)
    Sqd->cur_frame=killhdr.next_frame;

  /*************************************************************************
                   Update the Free Pointer chain:
   *************************************************************************/

  if (Add_To_Free_Chain(sq,killofs,&killhdr)==-1)
  {
    if(didlock) SquishUnlock(sq);
    return -1;
  }

  if(RemoveFromIndex(sq, msgnum) == -1)
  {
    if(didlock) SquishUnlock(sq);
    return -1;
  }

  /* Deduct one from the number of msgs */

  sq->num_msg--;
  sq->high_msg--;

  /* Adjust the current message number if we're above, since things    *
   * will have shifted...                                              */

  if (sq->cur_msg==msgnum+1)    /* If we killed the msg we're on */
  {
    /* Then jump back to the prior message, and refresh pointers */

    if ((hd=_SquishGotoMsg(sq, --sq->cur_msg, NULL, NULL, TRUE))==NULL)
    {
      Sqd->prev_frame=Sqd->cur_frame=NULL_FRAME;
      Sqd->next_frame=Sqd->begin_frame;
    }
    else free(hd);
  }
  else if (sq->cur_msg > msgnum)  /* Decrement counter appropriately */
    sq->cur_msg--;

  _SquishUpdateSq(sq, FALSE);

  if(didlock)
     {
     if(SquishUnlock(sq) == -1)
        {
        msgapierr = MERR_NOUNLOCK;
        return -1;
        }
     }

  return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

/* "Lock" the message base.  This means that no other applications will be   *
 * able to access the message base during the duration it is locked,         *
 * however, message writing will go MUCHMUCHMUCH quicker.                    */

sword  SquishLock(MSG *sq)
{

  if (InvalidMh(sq))
    return -1;

  /* Don't do anything if already locked */

  if (sq->locked)
    return 0;

  /* And lock the file */

  if (! _SquishLock(sq))
  {
    return -1;
  }

  /* Added by GvE, 30-01-93 */

  if(_SquishUpdateSq(sq, TRUE) == -1)
    return -1;

  // Free internal index and rescan area.

  if(Sqd->uidlist) free(Sqd->uidlist);

  Sqd->uidlist = NULL;

  if(BuildUidlist(sq) == -1)
     {
     SquishUnlock(sq);
     return -1;
     }

  /* End added by GvE */

  /* Set the flag in the _sqdata header */

  sq->locked=TRUE;

  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


/* Undo the above "lock" operation */

sword  SquishUnlock(MSG *sq)
{
  if (InvalidMh(sq))
    return -1;

  if (!sq->locked)
    return -1;

  /* Added by GvE, 30-01-93 */

  if(_SquishUpdateSq(sq, TRUE) == -1)
    return -1;

  /* End added by GvE */

  sq->locked=FALSE;

  if (mi.haveshare)
    unlock(Sqd->sfd, 0L, 1L);

  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword SquishValidate(byte *name)
{
  byte temp[PATHLEN];


  sprintf(temp, ss_sqd, name);

  if (! fexist(temp))
    return FALSE;

  sprintf(temp, ss_sqi, name);

  if (! fexist(temp))
    return FALSE;

  return TRUE;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword  SquishSetCurPos(MSGH *msgh, dword pos)
{
  if (InvalidMsgh(msgh))
    return -1;

  msgh->cur_pos=pos;
  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


dword  SquishGetCurPos(MSGH *msgh)
{
  if (InvalidMsgh(msgh))
    return -1;

  return (msgh->cur_pos);
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


UMSGID SquishMsgnToUid(MSG *sq, dword msgnum)
{

  if(InvalidMh(sq))
    return 0L;

  if(msgnum == 0L)
    return (UMSGID) 0L;

  if(Sqd->uidlist == NULL)
    return (UMSGID) 0L;

  return(Sqd->uidlist[msgnum-1]);

}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


dword  SquishUidToMsgn(MSG *sq, UMSGID umsgid, word type)
{
  UMSGID answer;
  dword mn;

  if (InvalidMh(sq))
    return 0L;

  if (umsgid==0L)
    return ((dword)0L);

  if(Sqd->uidlist == NULL)
    return ((dword)0L);

  answer=(UMSGID)0L;

  for (mn=0; mn < sq->num_msg; mn++)
  {
    /* Scan each element, and if we find a match, return the right value */

    if (Sqd->uidlist[mn] == umsgid ||
        (type==UID_NEXT && Sqd->uidlist[mn] >= umsgid) ||
        (type==UID_PREV && Sqd->uidlist[mn] <= umsgid &&
         Sqd->uidlist[mn] &&
         (mn+1 >= sq->num_msg || Sqd->uidlist[mn+1] > umsgid)))
    {
      answer=(UMSGID)(mn+1);
      break;
    }
  }

  return answer;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


dword  SquishGetTextLen(MSGH *msgh)
{
  return msgh->cur_len;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


dword  SquishGetCtrlLen(MSGH *msgh)
{
  return msgh->clen;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


dword  SquishGetHighWater(MSG *sq)
{
  return (SquishUidToMsgn(sq, sq->high_water, UID_PREV));
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword  SquishSetHighWater(MSG *sq, dword hwm)
{
  sq->high_water=SquishMsgnToUid(sq, hwm);
  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

// Extra function hacked on by Gerard van Essen to undelete

dword SquishUndelete(MSG *sq)
{
   SQHDR sqheader;
   dword maxmsg     = Sqd->max_msg;
   MIS  *mis        = NULL;
   XMSG *hdr        = NULL;
   char *body       = NULL;
   char *memptr     = NULL;
   char *tottext    = NULL;
   char *kludges    = NULL;
//   char *endkludges = NULL;
   MSGH *msghandle  = NULL;
   dword retval     = (dword) -1;
   dword totlen     = 0L,
         bodylen    = 0L,
         clen       = 0L,
         undeleted  = 0L,
         lastframe  = 0L;


   if (InvalidMh(sq))
     return (dword) -1;

   if (!sq->locked)
     return (dword) -1;

   Sqd->max_msg = 0; // No dynamic deleting now!

   if( (hdr = malloc(sizeof(XMSG))) == NULL)
      {
      msgapierr=MERR_NOMEM;
      goto close_up;
      }

   if( (mis = calloc(1, sizeof(MIS))) == NULL)
      {
      msgapierr=MERR_NOMEM;
      goto close_up;
      }

   while(Sqd->free_frame && Sqd->free_frame != lastframe)  // prevent endless loops.
     {
     lastframe = Sqd->free_frame;

     // Read the deleted message, as if it were not..
     if(_SquishReadHeader(sq, Sqd->free_frame, &sqheader) == -1)
        goto close_up;

//     bodylen = sqheader.msg_length - sqheader.clen - sizeof(XMSG);
     totlen = sqheader.frame_length - sizeof(XMSG);

     if(totlen && (tottext=calloc(1, (unsigned)totlen)) == NULL )
        {
        msgapierr=MERR_NOMEM;
        goto close_up;
        }

     memptr = tottext;

     if(read(Sqd->sfd, hdr, (unsigned) sizeof(XMSG)) != (int) sizeof(XMSG))
        {
        msgapierr=MERR_READ;
        goto close_up;
        }

     if(read(Sqd->sfd, tottext, (unsigned) totlen) != (int) totlen)
        {
        msgapierr=MERR_READ;
        goto close_up;
        }

     // Make sure it is NULL terminated. My routines will add a NULL if re-
     // quired, but that increases the length of the message, which is
     // highly undesirable, because we want to reuse the space that is
     // now occupied by this deleted message for the undeleted message.
     // If the last char is not a NULL, we'll make it one.

     if(totlen && *(tottext+totlen-1) != '\0')
       *(tottext+totlen-1) = '\0';

     if(totlen && *tottext == '\01')
       {
       clen = strlen(tottext)+1;   // How long is it?
       kludges = tottext;          // This is begin of kludges.
       tottext += clen;            // The body starts here.
       }
     else
       {
       if(totlen > 3 && *tottext == '\0')
          tottext++;
       }

     if(clen && ((long) (sqheader.frame_length - sizeof(XMSG) - clen)) <= 0)
       {
       clen = sqheader.frame_length - sizeof(XMSG);  // No body?! Strange..
       bodylen = 0;
       }
     else
       {
       bodylen = strlen(tottext);
       body = tottext;
       if((long) (sqheader.frame_length - sizeof(XMSG) - clen - bodylen) <= 0)
          bodylen = sqheader.frame_length - sizeof(XMSG) - clen;
       }

     // Write it out as a new message..
     if( (msghandle = SquishOpenMsg(sq, MOPEN_CREATE, 0L)) == NULL )
        goto close_up;

     SQ2MIS(msghandle, hdr, mis);

     mis->attr1 = (aPRIVATE|aSCANNED|aSENT);  // Prevent re-export!
     mis->attr2 = 0;

     if(SquishWriteMsg(msghandle,
                       0,
                       mis,
                       body,
                       bodylen,
                       bodylen,
                       clen,
                       kludges) == -1)
                       goto close_up;

     if(SquishCloseMsg(msghandle) == -1)
        goto close_up;

     undeleted++;

     if(memptr) free(memptr);
     memptr = NULL;
     FreeMIS(mis);
     }

   retval = undeleted;

   close_up:

   Sqd->max_msg = maxmsg;  // Reset dynamic deleting
   if(hdr)       free(hdr);
   FreeMIS(mis);
   if(mis)       free(mis);
   if(memptr)    free(memptr);
   if(msghandle) SquishCloseMsg(msghandle);

   return retval;
}


// End extra and now deleted stuff..


/****************************************************************************/
/****************** END OF EXTERNALLY-VISIBLE FUNCTIONS *********************/
/****************************************************************************/

sword _OpenSquish(MSG *sq, word *mode)
{
  byte temp[PATHLEN];

  sprintf(temp, ss_sqd, Sqd->base);

  if ((Sqd->sfd=sopen(temp, *mode==MSGAREA_CREATE ? fop_wpb : (O_RDWR | O_BINARY),
                      SH_DENYNO,
                      S_IREAD | S_IWRITE))==-1)
  {
    if (*mode != MSGAREA_CRIFNEC)
    {
      msgapierr=MERR_NOENT;
      return 0;
    }

    *mode=MSGAREA_CREATE;

    if ((Sqd->sfd=sopen(temp, fop_wpb | O_EXCL, SH_DENYNO,
                        S_IREAD | S_IWRITE))==-1)
    {
      msgapierr=MERR_NOENT;
      return 0;
    }
  }

  sprintf(temp,ss_sqi,Sqd->base);

  if ((Sqd->ifd=sopen(temp,*mode==MSGAREA_CREATE ? fop_wpb : fop_rpb,
                     SH_DENYNO,S_IREAD | S_IWRITE))==-1)
  {
    if (*mode != MSGAREA_CRIFNEC)
    {
      close(Sqd->sfd);
      msgapierr=MERR_NOENT;
      return 0;
    }

    *mode=MSGAREA_CREATE;

    if ((Sqd->ifd=sopen(temp, fop_wpb | O_EXCL, SH_DENYNO,
                        S_IREAD | S_IWRITE))==-1)
    {
      close(Sqd->sfd);
      msgapierr=MERR_NOENT;
      return 0;
    }
  }

  return 1;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

SQHDR * _SquishGotoMsg(MSG *sq, dword msgnum,
                                       FOFS *seek_frame, SQIDX *idx,
                                       word updptrs)
{
  SQIDX idxe;
  SQIDX *ip;
  FOFS ofs;
  SQHDR *hdr;


  /* In case the caller didn't provide a SQIDX to write to, use our own */

  if (idx)
    ip=idx;
  else ip=&idxe;


  /* Get the SQIDX record from the index file */

  if (_SquishGetIdxFrame(sq, msgnum-1, ip)==-1)
    return NULL;

  ofs=ip->ofs;

  if (seek_frame)
    *seek_frame=ofs;

  /* Read the frame header from the data file */

  if ((hdr=(SQHDR *)malloc(Sqd->sz_sqhdr))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  if (_SquishReadHeader(sq,ofs,hdr) != 0)
  {
    free(hdr);
    return NULL;
  }

  if (updptrs)
  {
    sq->cur_msg=msgnum;

    /* Now update all the in-memory pointers */

    Sqd->cur_frame=ofs;
    Sqd->next_frame=hdr->next_frame;
    Sqd->prev_frame=hdr->prev_frame;
  }

  return hdr;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

MSGH * _SquishOpenMsgRead(MSG *sq, word mode, dword msgnum)
{
  struct _msgh *msgh;

  SQHDR *hdr;

  FOFS this_frame;
  FOFS seek_frame;


  NW(mode);

  this_frame=Sqd->cur_frame;

  if ((this_frame==Sqd->end_frame || this_frame==NULL_FRAME) &&
      msgnum==MSGNUM_current)
  {
    msgapierr=MERR_NOENT;
    return NULL;
  }

  /* Figure out which way to seek */

  hdr=NULL;

  if (msgnum==MSGNUM_current || msgnum==sq->cur_msg)
  {
    seek_frame=Sqd->cur_frame;
    msgnum=MSGNUM_current;
  }
  else if (msgnum==MSGNUM_next || msgnum==sq->cur_msg+1L)
  {
    seek_frame=Sqd->next_frame;
    msgnum=MSGNUM_next;
  }
  else if (msgnum==MSGNUM_previous || msgnum==sq->cur_msg-1L)
  {
    seek_frame=Sqd->prev_frame;
    msgnum=MSGNUM_previous;
  }
  else if ((hdr=_SquishGotoMsg(sq, msgnum, &seek_frame, NULL, TRUE))==NULL)
    return NULL;


  /* If we go to beginning, we're at message #0 */

  if (msgnum==MSGNUM_previous && seek_frame==NULL_FRAME)
  {
    Sqd->next_frame=Sqd->begin_frame;
    Sqd->prev_frame=NULL_FRAME;
    Sqd->cur_frame=NULL_FRAME;
    sq->cur_msg=0;

    if (hdr)
      free(hdr);

    msgapierr=MERR_NOENT;
    return NULL;
  }
  else if (seek_frame==NULL_FRAME)
  {
    /* else if that message doesn't exist */

    if (hdr)
      free(hdr);

    msgapierr=MERR_NOENT;
    return NULL;
  }
  else if (seek_frame != this_frame)
  {
    /* If we moved to another msg, then update pointer appropriately */

    switch ((int)msgnum)
    {
      case MSGNUM_next:
        sq->cur_msg++;
        break;

      case MSGNUM_previous:
        sq->cur_msg--;
        break;

    /* default: _SquishGotoMsg() will set sq->cur_msg by itself */
    }
  }
  /* If the pointer didn't move, then error, unless we specifically asked*
   * to read the same msg number,                                        */
  else if (msgnum != MSGNUM_current && msgnum != sq->cur_msg)
  {
    if (hdr)
      free(hdr);

    msgapierr=MERR_BADA;
    return NULL;
  }

  /* _SquishGotoMsg() will have already set these */

  if (msgnum==MSGNUM_current || msgnum==MSGNUM_previous ||
      msgnum==MSGNUM_next)
  {
    /* Update the current pointer */
    Sqd->cur_frame=seek_frame;

    if ((hdr=malloc(Sqd->sz_sqhdr))==NULL)
    {
      msgapierr=MERR_NOMEM;
      return NULL;
    }

    /* Now grab the header of the frame we're trying to read */

    if (_SquishReadHeader(sq,seek_frame,hdr) != 0)
    {
      free(hdr);
      return NULL;
    }

    /* Copy the "next frame" pointer into memory */

    Sqd->next_frame=hdr->next_frame;
    Sqd->prev_frame=hdr->prev_frame;
  }


  if ((msgh=calloc(1, sizeof(struct _msgh)))==NULL)
  {
    free(hdr);
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  msgh->hdr=hdr;
  /*msgh->this_frame=this_frame;*/
  msgh->seek_frame=seek_frame;
  msgh->sq=sq;
  msgh->cur_pos=0L;
  msgh->clen=hdr->clen;
  msgh->cur_len=hdr->msg_length-msgh->clen-sizeof(XMSG);
  msgh->msgnum=msgnum;

  return (MSGH *)msgh;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword _SquishReadHeader(MSG *sq, dword ofs, SQHDR *hdr)
{
  if (ofs==NULL_FRAME)
    return 0;

  if(lseek(Sqd->sfd, ofs, SEEK_SET) == -1L ||
     read(Sqd->sfd, (char *)hdr, sizeof(SQHDR)) != sizeof(SQHDR) ||
     hdr->id != SQHDRID)
  {
    msgapierr=MERR_BADF;
    return -1;
  }

  return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword _SquishWriteHeader(MSG *sq,dword ofs,SQHDR *hdr)
{
  if (ofs==NULL_FRAME)
    return 0;

  hdr->id=SQHDRID;

  if (lseek(Sqd->sfd, ofs, SEEK_SET)==-1L ||
      write(Sqd->sfd, (char *)hdr, sizeof(SQHDR)) != sizeof(SQHDR))
  {
    msgapierr=MERR_BADF;
    return -1;
  }

  return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword _SquishUpdateHeaderNext(MSG *sq, dword ofs,
                                            SQHDR *hdr, dword newval)
{
  if (ofs==NULL_FRAME)
    return 0;

  if (_SquishReadHeader(sq, ofs, hdr) != 0)
    return -1;

  hdr->next_frame=newval;

  #ifdef DEBUG
  printf("Update header next at %lx to %lx\n", ofs, newval);
  #endif


  if (_SquishWriteHeader(sq, ofs, hdr) != 0)
    return -1;

  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


sword _SquishUpdateHeaderPrev(MSG *sq, dword ofs, SQHDR *hdr, dword newval)
{
  if (ofs==NULL_FRAME)
    return 0;

  if (_SquishReadHeader(sq, ofs, hdr) != 0)
    return -1;

  hdr->prev_frame=newval;

  #ifdef DEBUG
  printf("Update header prev at %lx to %lx\n", ofs, newval);
  #endif

  if (_SquishWriteHeader(sq, ofs, hdr) != 0)
    return -1;

  return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */
/* Rewrite the first "struct _sq" portion of the file (which contains the   *
 * global data)                                                             */

sword _SquishWriteSq(MSG *sq)
{
  struct _sqbase sqbase;


  SqToSqbase(sq, &sqbase);

  lseek(Sqd->sfd, 0L, SEEK_SET);

  if (write(Sqd->sfd, (char *)&sqbase, sizeof(struct _sqbase))
                                            != sizeof(struct _sqbase))
  {
    return -1;
  }

  Sqd->delta=sqbase;

  /* Make sure it gets written, so other tasks see it */

//  #ifndef __OS2__
  flush_handle2(Sqd->sfd);
//  #endif

  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

sword _SquishReadSq(MSG *sq, struct _sqbase *sqb)
{
  lseek(Sqd->sfd, 0L, SEEK_SET);

  if (read(Sqd->sfd, (char *)sqb, sizeof(struct _sqbase))
                                           != sizeof(struct _sqbase))
  {
    return -1;
  }

  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

sword _SquishUpdateSq(MSG *sq, word force)
{
  struct _sqbase now; /* The status of the header in the SQD */
  struct _sqbase new; /* The new, to-be-written header */
  struct _sqbase upd;
  struct _sqbase *delta=&Sqd->delta;


  /* No need to do this if the squishfile is locked */

  if (sq->locked && !force)
    return 0;

  SqToSqbase(sq, &upd);

  /* Read the Squish header, as it currently is now */

  if (_SquishReadSq(sq, &now)==-1)
    return -1;

  /* Let the new header be equal to what it is now, but apply any changes   *
   * that this task may have made.                                          */

  new=now;

  /* Now make changes as necessary to the squishfile's links */

  if (! eqstri(delta->base, upd.base))
    strcpy(new.base, upd.base);

  if (delta->num_msg != upd.num_msg)
    new.num_msg += (upd.num_msg - delta->num_msg);

  if (delta->high_msg != upd.high_msg)
    new.high_msg += (upd.high_msg - delta->high_msg);

  if (delta->max_msg != upd.max_msg)
    new.max_msg=upd.max_msg;

  if (delta->keep_days != upd.keep_days)
    new.keep_days=upd.keep_days;

  if (delta->skip_msg != upd.skip_msg)
    new.skip_msg=upd.skip_msg;

  if (delta->high_water != upd.high_water)
    new.high_water=upd.high_water;

  if (delta->uid != upd.uid)
    new.uid=upd.uid;

  if (delta->begin_frame != upd.begin_frame)
    new.begin_frame=upd.begin_frame;

  if (delta->last_frame != upd.last_frame)
    new.last_frame=upd.last_frame;

  if (delta->free_frame != upd.free_frame)
    new.free_frame=upd.free_frame;

  if (delta->last_free_frame != upd.last_free_frame)
    new.last_free_frame=upd.last_free_frame;

  if (delta->end_frame != upd.end_frame)
    new.end_frame=upd.end_frame;

  SqbaseToSq(&new, sq);

  if (_SquishWriteSq(sq)==-1)
    return -1;

  return 0;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

void Init_Hdr(SQHDR *sh)
{
  memset(sh, '\0', sizeof(SQHDR));

  sh->id=SQHDRID;
  sh->frame_type=FRAME_normal;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

 void SqToSqbase(MSG *sq,struct _sqbase *sqbase)
{
  memset(sqbase,'\0',sizeof(struct _sqbase));

  sqbase->len=Sqd->len;
  sqbase->num_msg=sq->num_msg;
  sqbase->high_msg=sq->high_msg;
  sqbase->max_msg=Sqd->max_msg;
  sqbase->keep_days=Sqd->keep_days;
  sqbase->skip_msg=Sqd->skip_msg;
/*sqbase->zero_ofs=Sqd->zero_ofs;*/
  sqbase->high_water=sq->high_water;
  sqbase->uid=Sqd->uid;

  strcpy(sqbase->base,Sqd->base);

  sqbase->begin_frame=Sqd->begin_frame;
  sqbase->free_frame=Sqd->free_frame;
  sqbase->last_free_frame=Sqd->last_free_frame;
  sqbase->end_frame=Sqd->end_frame;
  sqbase->last_frame=Sqd->last_frame;
/*sqbase->sz_sqidx=Sqd->sz_sqidx;*/
  sqbase->sz_sqhdr=Sqd->sz_sqhdr;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


 void SqbaseToSq(struct _sqbase *sqbase,MSG *sq)
{
  Sqd->len=sqbase->len;
  sq->num_msg=sqbase->num_msg;
  sq->high_msg=sqbase->high_msg;
  Sqd->max_msg=sqbase->max_msg;
  Sqd->keep_days=sqbase->keep_days;
  Sqd->skip_msg=sqbase->skip_msg;
/*Sqd->zero_ofs=sqbase->zero_ofs;*/
  sq->high_water=sqbase->high_water;
  Sqd->uid=sqbase->uid;

  strcpy(Sqd->base,sqbase->base);

  Sqd->begin_frame=sqbase->begin_frame;
  Sqd->free_frame=sqbase->free_frame;
  Sqd->last_free_frame=sqbase->last_free_frame;
  Sqd->end_frame=sqbase->end_frame;
  Sqd->last_frame=sqbase->last_frame;
/*Sqd->sz_sqidx=sqbase->sz_sqidx;*/
  Sqd->sz_sqhdr=sqbase->sz_sqhdr;

  if (Sqd->max_msg && Sqd->skip_msg >= Sqd->max_msg-1)
  {
    Sqd->skip_msg=0;
    Sqd->max_msg=0;
  }
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

sword AddIndex(MSG *sq, SQIDX *ix, dword num)
{
  dword * tmpptr;

  // First we do it on disk..

  if (lseek(Sqd->ifd, (long)num*(long)sizeof(SQIDX), SEEK_SET)==-1 ||
      write(Sqd->ifd, (char *)ix, sizeof(SQIDX)) != sizeof(SQIDX))
  {
    msgapierr=MERR_BADF;
    return -1;
  }

  // Then we need to add it to the memory index of UID's

  if(num < Sqd->maxlen)       // Fits in alloced mem, or do we need realloc?
    {
    Sqd->uidlist[num] = ix->umsgid;
    }
  else                        // Get more memory..
    {
    tmpptr = (dword *)realloc(Sqd->uidlist, (Sqd->maxlen + 100) * sizeof(dword));

    if(tmpptr == NULL)           // No mem left :-(
      {
      msgapierr=MERR_NOMEM;
      return -1;
      }
    Sqd->maxlen += 100;
    Sqd->uidlist = tmpptr;
    Sqd->uidlist[num] = ix->umsgid;
    }

  return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

sword Add_To_Free_Chain(MSG *sq, FOFS killofs, SQHDR *killhdr)
{
  SQHDR lhdr;

  /* If there are no other free frames, then this is simple... */

  if (Sqd->free_frame==NULL_FRAME || Sqd->last_free_frame==NULL_FRAME)
  {
    Sqd->free_frame=Sqd->last_free_frame=killofs;

    killhdr->prev_frame=killhdr->next_frame=NULL_FRAME;
  }
  else
  {
    /* Insert this frame into the chain */

    killhdr->next_frame=NULL_FRAME;
    killhdr->prev_frame=Sqd->last_free_frame;

    if (_SquishUpdateHeaderNext(sq, Sqd->last_free_frame, &lhdr, killofs)==-1)
      return -1;

    Sqd->last_free_frame=killofs;
  }


  /* Now write the fixed header of the killed message back to the file */

  killhdr->frame_type=FRAME_free;

  #ifdef DEBUG
  fprintf(stdout,"* Killed frame at %08lx (%ld bytes)\n\n",killofs,
          killhdr->frame_length);
  #endif

  if (_SquishWriteHeader(sq, killofs, killhdr)==-1)
    return -1;

  /* And write to the SquishFile, to update the FREE pointer */

  _SquishUpdateSq(sq, FALSE);

  return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

sword _SquishGetIdxFrame(MSG *sq, dword num, SQIDX *idx)
{
  dword ofs;

  ofs = (dword)num * (dword)sizeof(SQIDX);

  if (lseek(Sqd->ifd, ofs, SEEK_SET)==-1L ||
      read(Sqd->ifd, (char *)idx, sizeof(SQIDX)) != sizeof(SQIDX))
  {
    msgapierr=MERR_BADF;
    return -1;
  }

  if (idx->ofs==NULL_FRAME)
    return -1;
  else return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

/* Finds a free frame big enough to hold the specified message */

sword _SquishFindFree(MSG *sq, FOFS *this_frame, dword totlen,
                               dword clen, SQHDR *freehdr,
                               FOFS *last_frame, SQHDR *lhdr, MSGH *msgh)
{
  /* First grab the latest `free' frame from the file, so we don't       *
   * overwrite ourselves, in a multi-tasking environment.                */

  *this_frame=Sqd->free_frame;


  /* Kill enough msgs to make room for this one, as long as there         *
   * is a limit set, we're set up for dynamic renumbering, AND            *
   * we're writing to the last message in the base.                       */

  if (MsghSqd->max_msg && msgh->msgnum==0L &&
      (MsghSqd->flag & SF_STATIC)==0)
  {
    while (msgh->sq->num_msg+1 > MsghSqd->max_msg)
    {
      if (SquishKillMsg(msgh->sq, MsghSqd->skip_msg+1L)==-1)
      {
        return -1;
      }
    }
  }

   /* Now walk the list of free pointers, and see if we can find one which *
   * is big enough to hold everything.                                    */

  #ifdef DEBUG
  fprintf(stdout,
          "Msg #%ld; need %ld bytes:\n",
          msgh->sq->num_msg,
          totlen+clen+(dword)sizeof(XMSG));
  #endif

  for (;;)
  {
    /* If we're at EOF, then there's always enough room. */
    if (*this_frame==NULL_FRAME)
    {
      #ifdef DEBUG
      fprintf(stdout,"!!! No go.  Allocating new frame.\n\n");
      #endif

      *this_frame=Sqd->end_frame;
      freehdr->next_frame=NULL_FRAME;
      freehdr->prev_frame=NULL_FRAME;
      freehdr->frame_length=0L;
      freehdr->msg_length=0L;
      freehdr->frame_type=FRAME_normal;
      freehdr->clen=clen;
      break;
    }

    if (_SquishReadHeader(sq, *this_frame, freehdr)==-1)
    {
      *this_frame=NULL_FRAME;
      continue;
    }

    #ifdef DEBUG
    fprintf(stdout,
            "  Frame at %08lx: %ld\n",
            *this_frame,
            freehdr->frame_length);
    #endif

    if (freehdr->frame_length >= totlen+clen+(dword)sizeof(XMSG))
    {
      #ifdef DEBUG
      fprintf(stdout,"... Got it!\n\n");
      #endif

      break;
    }
    else
    {
      /* If two frames are adjacent, try to merge them to make more       *
       * room for the current message.                                    */

      *last_frame=NULL_FRAME;

      while (freehdr->next_frame==
                 *this_frame + MsghSqd->sz_sqhdr + freehdr->frame_length &&
             freehdr->frame_length < totlen + clen + (dword)sizeof(XMSG))
      {
        if (_SquishReadHeader(sq, freehdr->next_frame, lhdr)==-1)
          break;

        freehdr->frame_length += MsghSqd->sz_sqhdr+lhdr->frame_length;

        /* Don't bother writing the frame to disk -- if a message is      *
         * being written, then it'll get updated anyway.  If not, then    *
         * we want to keep these chunks separated, in case we have        *
         * smaller messages in the future.                                */

        #ifdef DEBUG
        fprintf(stdout,"  Compact of %08lx and %08lx (new len=%ld)\n",
                *this_frame, freehdr->next_frame, freehdr->frame_length);
        #endif

        *last_frame=freehdr->next_frame;
        freehdr->next_frame=lhdr->next_frame;
      }

      if (freehdr->frame_length >= totlen+clen+(dword)sizeof(XMSG))
      {
        /* If one of te frames in our chain was the last free frame,      *
         * set the last free frame to the one we've merged it into,       *
         * for later a clean up effort.                                   */

        if (*last_frame==Sqd->last_free_frame)
          Sqd->last_free_frame=*this_frame;

        #ifdef DEBUG
        fprintf(stdout,"  Rescued frame at %08lx (%ld bytes)\n",
                *this_frame, freehdr->frame_length);
        #endif
        break;
      }

      *this_frame=freehdr->next_frame;
    }
  }

  return 0;
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */

/* #define PRIME 65521*/

dword SquishHash(byte *f)
{
  dword hash=0, g;
  char *p;

  for (p=f; *p; p++)
  {
    hash=(hash << 4) + tolower(*p);

    if ((g=(hash & 0xf0000000L)) != 0L)
    {
      hash |= g >> 24;
      hash |= g;
    }
  }


  /* Strip off high bit */

  return (hash & 0x7fffffffLu);
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */



int _SquishLock(MSG *sq)
{
  return !(mi.haveshare && lock(Sqd->sfd, 0L, 1L)==-1);
}

/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


void _SquishUnlock(MSG *sq)
{
  if (mi.haveshare)
    unlock(Sqd->sfd, 0L, 1L);
}


// -------------------------------------------------------------
// --   Build a list of UID's in memory (index of area).      --
// --       Will be used for UID <-> Msgn conversion.         --
// -------------------------------------------------------------

int BuildUidlist(MSG *sq)
{
   #define CHUNKSIZE 1300    // 1300 index entries at a time, < 16 Kb
   SQIDX *bufptr = NULL, *curidx;
   dword left, maxlen;
   unsigned whatnow, entry=0, i;

   Sqd->maxlen  = 0; // Start with 0, in case mem alloc fails..

   if(sq->num_msg == 0)
      {
      Sqd->uidlist = NULL;
      return 0;
      }

   maxlen = sq->num_msg + 100;   // Room for 100 additions..

   if((Sqd->uidlist =
   #if defined (__WATCOMC__) && !defined (__FLAT__)
          (dword *)calloc(1, maxlen * sizeof(dword))) == NULL)
   #else
          (dword *)malloc(maxlen * sizeof(dword))) == NULL)
   #endif
          {
          msgapierr = MERR_NOMEM;
          return -1;
          }

   Sqd->maxlen = maxlen;

   if((bufptr = malloc(CHUNKSIZE * sizeof(SQIDX))) == NULL)
     {
     msgapierr=MERR_NOMEM;
     goto errorexit;
     }

   if(lseek(Sqd->ifd, 0L, SEEK_SET) == -1)
     {
     msgapierr=MERR_SEEK;
     goto errorexit;
     }

   left = sq->num_msg;

   while(left > 0)
     {
     whatnow = min(CHUNKSIZE, left);
     left -= whatnow;
     if(read(Sqd->ifd, bufptr, (unsigned) (whatnow * sizeof(SQIDX))) !=
                                             (int) (whatnow * sizeof(SQIDX)))
       {
       msgapierr=MERR_READ;
       goto errorexit;
       }

     for(i=0, curidx=bufptr; i<whatnow; i++, curidx++, entry++)
       {
       Sqd->uidlist[entry] = curidx->umsgid;
       }
     }

   if(bufptr) free(bufptr);

   return 0;

   errorexit:

   if(Sqd->uidlist) free(Sqd->uidlist);
   Sqd->uidlist = NULL;
   if(bufptr)       free(bufptr);
   return -1;
}


/* ---------------------------------------------------------------
   --                                                           --
   --------------------------------------------------------------- */


// Remove an entry from the index (both disk & memory index).

int RemoveFromIndex(MSG *sq, dword msgnum)
{
  byte * pcBuf;
  int got;
  SQIDX sqi;

  msgnum++;       // Make 1-based

  sqi.umsgid = sqi.hash = -1L;
  sqi.ofs    = 0L;

  #ifndef __OS2__
    #define SHIFT_SIZE 16384
  #else
    #define SHIFT_SIZE 65536
  #endif

  // First, let update it on disk.

  (void)lseek(Sqd->ifd, (long)msgnum * (long)sizeof(SQIDX), SEEK_SET);

  if ( (pcBuf=malloc(SHIFT_SIZE)) == NULL )
  {
    msgapierr=MERR_NOMEM;
    return -1;
  }

  while ((got=read(Sqd->ifd, pcBuf, SHIFT_SIZE)) > 0)
  {
    /* Skip back to one position before this index entry */

    (void)lseek(Sqd->ifd, -(long)got - sizeof(SQIDX), SEEK_CUR);

    if (write(Sqd->ifd, pcBuf, (unsigned)got) != got)
    {
      msgapierr=MERR_BADF;
      free(pcBuf);
      return -1;
    }

    (void)lseek(Sqd->ifd, (long)sizeof(SQIDX), SEEK_CUR);
  }

  free(pcBuf);

  /* Now write the last entry to stomp over the index element that is at    *
   * the end of the file.                                                   */

//  (void)lseek(Sqd->ifd, -(long)sizeof(SQIDX), SEEK_CUR);

//  if (write(Sqd->ifd, (char *)&sqi, sizeof(SQIDX)) != (int)sizeof(SQIDX))
//  {
//    msgapierr=MERR_BADF;
//    return -1;
//  }

  // No we don't, we just change the size. This will matter if we delete
  // a whole bunch of messages in one pass: the index will get smaller
  // and smaller, and speed will go up because there's not that much shifting
  // and rewriting going on.

  // Remember, num_msg is not yet decreased by one by SquishDelete!

  if(chsize(Sqd->ifd, (long) (((long)sq->num_msg-1L) * (long) sizeof(SQIDX)) ) == -1)
    return -1;

  // Fine. And now we update it in memory as well

  if(Sqd->uidlist == NULL)
    return -1;

  memcpy(Sqd->uidlist + msgnum - 1,
         Sqd->uidlist + msgnum,
         (sq->num_msg - msgnum) * sizeof(dword));

  return 0;

}


// ------------------------------------------------------------------

int SquishCalcTrailing(MSGH *msgh)
{
   unsigned readlen = MAXTRAILSIZE;
   long start, bodylen;
   char *txtptr;


   bodylen = msgh->hdr->msg_length - sizeof(XMSG) - msgh->clen;

   if(bodylen < readlen) readlen = bodylen;

   if(readlen == 0) return 0;  // Nothing to do, no body.

   // Where to start reading message.
   start = (long)msgh->seek_frame + (dword)MsghSqd->sz_sqhdr +
               (dword)msgh->sq->sz_xmsg + (long)msgh->clen + bodylen - readlen;

   if(lseek(MsghSqd->sfd, start, SEEK_SET) != start) return -1;

   if((txtptr = calloc(1, readlen+1)) == NULL)
     return -1;

   if(read(MsghSqd->sfd, txtptr, (unsigned)readlen) != (int) readlen)
     {
     free(txtptr);
     return -1;
     }

   msgh->trailsize = AnalyseTrail(txtptr, readlen, &msgh->mis);
   msgh->cur_len -= msgh->trailsize;

   free(txtptr);
   return 0;

}

// --------------------------------------------------------------
// Returns possible extra kludges that area needed

void MIS2SQ(MIS *mis, XMSG *xmsg)
{
   int i;
   JAMTM *tmdate;
   struct tm *timestamp;
//   STRINGLIST *current;
//   char temp[72];
   dword curtime;

   memcpy(xmsg->from, mis->from, min(35, strlen(mis->from)));
   memcpy(xmsg->to, mis->to, min(35, strlen(mis->to)));
   memcpy(xmsg->subj, mis->subj, min(71, strlen(mis->subj)));

   xmsg->orig = mis->origfido;
   xmsg->dest = mis->destfido;

   for(i=0; i<SQNUMATTR1; i++)
    {
    if(mis->attr1 & SQAttrTable1[i][0])
       xmsg->attr |= SQAttrTable1[i][1];
    }

   xmsg->replyto = mis->replyto;
   for(i=0; i<9; i++)
      xmsg->replies[i] = mis->replies[i];

   tmdate = JAMsysLocalTime((dword *)&mis->msgwritten);
   TmDate_to_DosDate(tmdate, &xmsg->date_written);

   if((dword) mis->msgprocessed != 0L)
      tmdate = JAMsysLocalTime((dword *)&mis->msgprocessed);
   else
      {
      curtime = (dword) JAMsysTime(NULL);
      tmdate = JAMsysLocalTime(&curtime); // We need something here for Squish..
      }
   TmDate_to_DosDate(tmdate, &xmsg->date_arrived);

   timestamp = JAMsysLocalTime(&mis->msgwritten);
   sprintf(xmsg->ftsc_date, "%02d %s %02d  %02d:%02d:%02d",
       timestamp->tm_mday, months_ab[timestamp->tm_mon],
       timestamp->tm_year, timestamp->tm_hour,
       timestamp->tm_min, timestamp->tm_sec);

   xmsg->utc_ofs = mis->utc_ofs;

   Files2Subject(mis, xmsg->subj);

}


// ===================================================================

void SQ2MIS(MSGH *msgh, XMSG *xmsg, MIS *mis)
{
  int l;
  MSG *sq = msgh->sq;
  struct tm tmdate;

  memcpy(mis->to,   xmsg->to, 36);
  memcpy(mis->from, xmsg->from, 36);
  memcpy(mis->subj, xmsg->subj, 72);
  memcpy(mis->ftsc_date, xmsg->ftsc_date, 20);

  for(l=0; l<SQNUMATTR1; l++)
   {
   if(xmsg->attr & SQAttrTable1[l][1])
      mis->attr1 |= SQAttrTable1[l][0];
   }

  if( (mis->attr1 & (aCRASH|aHOLD)) == (aCRASH|aHOLD) )  // Direct for squish!
    {
    if(mi.useflags == 0)   // We're set for a Squish environment
      {
      mis->attr1  |= aDIR;              // Make it 'direct', and..
      mis->attr1 &= ~(aCRASH|aHOLD);    // toggle off the Crash and Hold.
      }
    }

  mis->origfido = xmsg->orig;
  mis->destfido = xmsg->dest;

  mis->utc_ofs = xmsg->utc_ofs;

  mis->replyto = xmsg->replyto;
  for(l=0; l<9; l++)
    mis->replies[l] = xmsg->replies[l];

  mis->msgno    = SquishMsgnToUid(sq, msgh->relnum);
  mis->origbase = sq->type;

  DosDate_to_TmDate(&xmsg->date_written, &tmdate);
  mis->msgwritten = JAMsysMkTime(&tmdate);

  DosDate_to_TmDate(&xmsg->date_arrived, &tmdate);
  mis->msgprocessed = JAMsysMkTime(&tmdate);

  // Now we have to fiddle with file attaches and requests..

  if(xmsg->attr & (MSGFRQ|MSGURQ))  // extract requested files from subject.
    Extract_Requests(mis);

  if(xmsg->attr & MSGFILE)          // extract attached files from subject.
    Extract_Attaches(mis);

}
