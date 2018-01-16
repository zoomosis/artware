#define MSGAPI_HANDLERS

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #include <os2.h>
#endif

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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include "prog.h"
#include "dr.h"
#include "alc.h"
#include "max.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sdm.h"
#include "api_sdmp.h"
#include "apidebug.h"
#include "sqsdm.h"


// Prototypes

void near Convert_Fmsg_To_Xmsg(struct _omsg *fmsg,XMSG *msg,word def_zone);
void near Convert_Xmsg_To_Fmsg(XMSG *msg,struct _omsg *fmsg);
void near Init_Xmsg(XMSG *msg);
sword near _SdmRescanArea(MSG *mh);
sword near _Grab_Clen(MSGH *msgh);
void WriteToFd(byte *str);
void near Get_Binary_Date(struct _stamp *todate,struct _stamp *fromdate,byte *asciidate);

// End protos, added by Gve in this file instead of api_sdmp.h

#define SDM_BLOCK 2048
#define Mhd ((struct _sdmdata *)(mh->apidata))
#define MsghMhd ((struct _sdmdata *)(((struct _msgh *)msgh)->sq->apidata))

static byte *hwm_from="-=|ÿSquishMailÿ|=-";

void SDM2MIS(struct _omsg *sdmhdr, MIS *mis);
char * MIS2SDM(MSGH *msgh, MIS *mis, struct _omsg *sdmhdr, char *kludges);

extern dword SQAttrTable1[SQNUMATTR1][2];

void CheckReadOnly(char *filename);

// ====================================================================

MSG * SdmOpenArea(byte *name, word mode, word type)
{
  MSG *mh;

//  NW(_junksqd); /* to shut up wcc */

  if ((mh=CreateAreaHandle(type))==NULL)
     return NULL;

  *mh->api=sdm_funcs;

  if ((mh->apidata=(void *)calloc(1, sizeof(struct _sdmdata)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    goto ErrOpen;
  }

  strcpy(Mhd->base,name);
  #ifdef __GNUC__
  Add_Trailing(Mhd->base,'/');
  #else
  Add_Trailing(Mhd->base,'\\');
  #endif
  Mhd->hwm=(dword)-1L;

  mh->high_water=(dword)-1L;

  if (! direxist(name) )
     {
     if(mode!=MSGAREA_NORMAL)
        {
        #ifndef __GNUC__
        if(mkdir(name)==-1) /* Try to make it */
        #else
        if(mkdir(name,S_IREAD|S_IWRITE)==-1)
        #endif
         {
         /* Give error back! */
         msgapierr=MERR_NOENT;
         goto ErrOpen;
         }
        }
     else
        {
        /* Give error back! */
        msgapierr=MERR_NOENT;
        goto ErrOpen;
        }
      }

  if (! _SdmRescanArea(mh))
    goto ErrOpen;


  msgapierr=0;
  return mh;

ErrOpen:

  if (mh)
  {
    if (mh->api)
    {
      if (mh->apidata)
        free((char *)mh->apidata);

      free(mh->api);
    }

    free(mh);
  }

  return NULL;
}


// ====================================================================


sword  SdmCloseArea(MSG *mh)
{
  static byte *msgbody="NOECHO\r\rPlease ignore.  This message is only used "
                       "by the SquishMail system to store\r"
                       "the high water mark for each conference area.\r\r"
                       "\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r"
                       "(Elvis was here!)\r\r\r";
  MIS mis;
  MSGH *msgh;


  if (InvalidMh(mh))
    return -1;

  if (Mhd->hwm_chgd)
    if ((msgh=SdmOpenMsg(mh, MOPEN_CREATE, 1L)) != NULL)
    {
      memset(&mis, '\0', sizeof(MIS));

      mis.msgwritten = mis.msgprocessed = JAMsysTime(NULL);

      /* Use high-bit chars in the to/from field, so that (l)users          *
       * can't log on as this userid and delete the HWM.                    */

      strcpy(mis.from,hwm_from);
      strcpy(mis.to,mis.from);
      strcpy(mis.subj,"High wadda' mark");

      /* To prevent "intl 0:0/0 0:0/0" kludges */
      mis.origfido.zone=mis.destfido.zone=mi.def_zone;

      mis.replyto=mh->high_water;
      mis.attr1 = aPRIVATE | aREAD | aLOCAL | aSENT;

      SdmWriteMsg(msgh,FALSE,&mis,msgbody,strlen(msgbody),
                  strlen(msgbody),0L,NULL);

      SdmCloseMsg(msgh);
    }

  if (Mhd->msgs_open)
  {
    msgapierr=MERR_EOPEN;
    return -1;
  }

  if (Mhd->msgnum)
    {
    free(Mhd->msgnum);
    Mhd->msgnum = NULL;
    }

  free((char *)mh->apidata);
  free(mh->api);

  mh->id=0L;
  free(mh);

  msgapierr=MERR_NONE;
  return 0;
}


// ====================================================================


MSGH * SdmOpenMsg(MSG *mh, word mode, dword msgnum)
{
  byte msgname[PATHLEN];
  int handle, retval;
  int filemode, sharemode=SH_DENYNO;
  word mn, owrite=FALSE;
  size_t newnumber;

  MSGH *msgh;


  if (InvalidMh(mh))
    return NULL;

//  if(mode == MOPEN_RDHDR)
//     mode = MOPEN_RDHDR;

  if (msgnum==MSGNUM_CUR)
    msgnum=mh->cur_msg;
  else if (msgnum==MSGNUM_PREV)
  {
    for (mn=(word)mh->num_msg-1; (sdword)mn < (sdword)mh->high_msg; mn--)
      if ((dword)Mhd->msgnum[mn] < mh->cur_msg)
      {
        msgnum=mh->cur_msg=Mhd->msgnum[mn];
        break;
      }

    /* If mn==-1, no message to go to */

    if (mn==(word)-1)
    {
      msgapierr=MERR_NOENT;
      return NULL;
    }
  }
  else if (msgnum==MSGNUM_NEXT)
  {
    for (mn=0; mn < (word)mh->num_msg; mn++)
      if ((dword)Mhd->msgnum[mn] > mh->cur_msg)
      {
        msgnum=mh->cur_msg=Mhd->msgnum[mn];
        break;
      }

    /* If mn==Mhd->msgnum_len, we can't go to any message */

    if ((dword)mn==mh->num_msg)
    {
      msgapierr=MERR_NOENT;
      return NULL;
    }
  }
  else if (mode != MOPEN_CREATE)
  {
    /* If we're not creating, make sure that the specified msg# can         *
     * be found.                                                            */

    for (mn=0; mn < (word)mh->num_msg; mn++)
      if (msgnum==Mhd->msgnum[mn])
        break;

    if ((dword)mn==mh->num_msg)
    {
      msgapierr=MERR_NOENT;
      return NULL;
    }
  }


  if (mode==MOPEN_CREATE)
  {
    /* If we're creating a new message... */

    if (msgnum==0L)
    {
      /* If the base isn't locked, make sure that we avoid conflicts... */

      if (! mh->locked)
      {
        /* Check to see if the msg we're writing already exists */

        sprintf(msgname, sd_msg, Mhd->base, (int)mh->high_msg+1);

        if (fexist(msgname))
        {
          /* If so, rescan the base, to find out which msg# it is. */

          if (Mhd->msgnum && Mhd->msgnum_len)
            {
            free(Mhd->msgnum);
            Mhd->msgnum = NULL;
            }

          _SdmRescanArea(mh);
        }
      }

      msgnum=++mh->high_msg;

      /* Make sure that we don't overwrite the high-water mark, unless      *
       * we call with msgnum != 0L (a specific number).                     */

      if (mh->isecho && msgnum==1)
        msgnum=mh->high_msg=2;
    }
    else
    {
      /* Otherwise, we're overwriting an existing msg */

    owrite=TRUE;
    sprintf(msgname,sd_msg,Mhd->base,(int)msgnum);
    CheckReadOnly(msgname);
    }

    filemode=O_CREAT | O_TRUNC | O_RDWR;
    sharemode = SH_DENYWR;
  }
  else if ( (mode==MOPEN_READ) || (mode==MOPEN_RDHDR) )
    filemode=O_RDONLY;
  else if (mode==MOPEN_WRITE)
    filemode=O_WRONLY;
  else filemode=O_RDWR;

  sprintf(msgname,sd_msg,Mhd->base,(int)msgnum);

  if(mode == MOPEN_READ || mode == MOPEN_RDHDR)
    {
    if ( (handle=sopen(msgname, (O_RDONLY | O_BINARY), SH_DENYNO, S_IREAD|S_IWRITE)) == -1 )
      {
      msgapierr=MERR_NOENT;
      return NULL;
      }
    }
  else
    {
    if ((handle=sopen(msgname,filemode | O_BINARY,sharemode,
                      S_IREAD | S_IWRITE))==-1)
      {
      if (filemode & O_CREAT)
        msgapierr=MERR_BADF;
      else msgapierr=MERR_NOENT;
      return NULL;
      }
    }

  mh->cur_msg=msgnum;

  if ((msgh=calloc(1, sizeof(MSGH)))==NULL)
  {
    close(handle);
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  msgh->fd=handle;
  msgh->mode = mode;
  msgh->msgnum = msgnum;

  if (mode==MOPEN_CREATE)
    {
    if (mh->num_msg+1 >= Mhd->msgnum_len)
      {
      Mhd->msgnum=realloc(Mhd->msgnum,
                          (Mhd->msgnum_len += SDM_BLOCK)*sizeof(word));

      if (!Mhd->msgnum)
      {
        free(msgh);
        close(handle);
        msgapierr=MERR_NOMEM;
        return NULL;
      }
    }

    /* If we're writing a new msg, this is easy -- just add to end of list */

    if (!owrite /*msgnum==mh->high_msg || mh->num_msg==0*/)
      {
      newnumber = (size_t)(mh->num_msg);
      Mhd->msgnum[newnumber]=(word)msgnum;
      mh->num_msg++;
      }
    else
    {
      for (mn=0; (dword)mn < mh->num_msg; mn++)
        if ((dword)Mhd->msgnum[mn] >= msgnum)
          break;

      /* If this message is already in the list then do nothing -- simply   *
       * overwrite it, keeping the same message number, so no action is     *
       * required.                                                          */

      if ((dword)Mhd->msgnum[mn]==msgnum)
        ;
      else
      {
        /* Otherwise, we have to shift everything up by one since we're     *
         * adding this new message inbetween two others.                    */

        memmove(Mhd->msgnum+mn+1,
                Mhd->msgnum+mn,
                ((size_t)mh->num_msg-mn)*sizeof(word));

        Mhd->msgnum[mn]=(word)msgnum;
        mh->num_msg++;
      }
    }
  }

  msgh->cur_pos=0L;

  if (mode==MOPEN_CREATE)
    msgh->msg_len=0;
  else msgh->msg_len=(dword)-1;

  msgh->sq=mh;
  msgh->id=MSGH_ID;
  msgh->clen=-1;
  msgh->zplen=0;

  if(mode!=MOPEN_CREATE)
    {
    ReadHdrKludges(msgh);
    SDMCalcTrailing(msgh);
    }

  msgapierr=MERR_NONE;

  /* Keep track of how many messages were opened for this area */

  MsghMhd->msgs_open++;

  return msgh;
}


// ====================================================================

sword  SdmCloseMsg(MSGH *msgh)
{
  sword retval = 0;
  char ch='\0';


  if (InvalidMsgh(msgh))
    return -1;

  if(msgh->mode==MOPEN_CREATE)
    {
    lseek(msgh->fd,
          0L,
          SEEK_END);

    if(msgh->trail != NULL)
      {
      if(write(msgh->fd, msgh->trail, msgh->trailsize+1) == -1)
         retval = -1;
      }
    else
      write(msgh->fd, &ch, 1);

    }

  MsghMhd->msgs_open--;

  close(msgh->fd);

  if(msgh->trail) free(msgh->trail);
  if(msgh->kludges) free(msgh->kludges);

  FreeMIS(&msgh->mis);

  msgh->id=0L;
  free(msgh);

  msgapierr=MERR_NONE;
  return retval;
}

// ====================================================================


dword SdmReadMsg(MSGH *msgh, MIS *mis, dword offset, dword bytes, byte *text, dword clen, byte *ctxt)
{
  word got = 0;


  if (InvalidMsgh(msgh))
    return -1L;

  if (! (clen && ctxt))
  {
    clen=0L;
    ctxt=NULL;
  }

  if (! (text && bytes))
  {
    bytes=0L;
    text=NULL;
  }

  if(mis)
    {
    CopyMIS(&msgh->mis, mis);

    if(msgh->mode == MOPEN_RDHDR)
       return 0L;
    }


  if(text)
    {
    lseek(msgh->fd, (dword)sizeof(struct _omsg) + msgh->rawclen + offset,
            SEEK_SET);

    msgh->cur_pos=offset;

    got=read(msgh->fd, text, (word)bytes);

    // Update counter only if we got some text

    if (got > 0)
      msgh->cur_pos += got;
    }

  /* And if the app requested ctrlinfo, put it in its place. */

  if(ctxt)
    strcpy(ctxt,msgh->kludges);

  msgapierr=MERR_NONE;

  return (dword) got;
}

// ====================================================================

sword  SdmWriteMsg(MSGH *msgh, word append, MIS *mis, byte *text, dword textlen, dword totlen, dword clen, byte *ctxt)
{
  struct _omsg fmsg;
  byte *s;
  char *newkludges=NULL;
  dword trailmax;
  STRINGLIST *current;
  MSG *sq=msgh->sq;


  NW(totlen);

  if (clen==0L || ctxt==NULL)
  {
    ctxt=NULL;
    clen=0L;
  }

  if (InvalidMsgh(msgh))
    return -1;

  lseek(msgh->fd,0L,SEEK_SET);

  if(mis)
    {
    newkludges = MIS2SDM(msgh, mis, &fmsg, ctxt);

    // Now we have to prepare the trailing stuff (SEEN-BY, PATH, VIA).
    // It is passed separately by the higher level app, but in SDM
    // areas it is part of the body, so we have to add that...

    if(!append && (mis->seenby || mis->path || mis->via))
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

      totlen += msgh->trailsize;
      }

    if(write(msgh->fd,(char *)&fmsg,
             sizeof(struct _omsg)) != sizeof(struct _omsg))
      {
      msgapierr=MERR_NODS;
      if(newkludges) free(newkludges);
      return -1;
      }

    if(msgh->mode == MOPEN_RW)
      {
      if(newkludges) free(newkludges);
      return 0;
      }
    }

  /* Now write the control info / kludges */

  if(ctxt || newkludges)
    {
    if(newkludges)
       s=CvtCtrlToKludge(newkludges);
    else
       s=CvtCtrlToKludge(ctxt);

    if(newkludges) free(newkludges);

    if(s)
      {
      write(msgh->fd,s,strlen(s));
      free(s);
      }
    }

  if(append)
    lseek(msgh->fd,0L,SEEK_END);

  if(text)
    {
    if(write(msgh->fd, text, (int)textlen) != (unsigned)textlen)
      {
      msgapierr=MERR_NODS;
      return -1;
      }
    }

  msgapierr=MERR_NONE;

  return 0;

}

// ====================================================================

sword SdmKillMsg(MSG *mh, dword msgnum)
{
  dword hwm;
  byte temp[PATHLEN];
  word mn;

  if (InvalidMh(mh))
    return -1;


  /* Remove the message number from our private index */

  for (mn=0; (dword)mn < mh->num_msg; mn++)
    if ((dword)Mhd->msgnum[mn]==msgnum)
    {
      memmove(Mhd->msgnum+mn,
              Mhd->msgnum+mn+1,
              (int)(mh->num_msg-mn-1)*sizeof(int));
      break;
    }

  /* If we couldn't find it, return an error message */

  if (mn==(word)mh->num_msg)
  {
    msgapierr=MERR_NOENT;
    return -1;
  }

  sprintf(temp,sd_msg,Mhd->base,(unsigned int)msgnum);

  CheckReadOnly(temp);

  if (unlink(temp)==-1)
  {
    msgapierr=MERR_NOENT;
    return -1;
  }

  mh->num_msg--;

  /* Adjust the high message number */

  if (msgnum==mh->high_msg)
    if (mh->num_msg)
      mh->high_msg=(dword)Mhd->msgnum[(int)mh->num_msg-1];
    else mh->high_msg=0;

  /* Now adjust the high-water mark, if necessary */

  hwm=SdmGetHighWater(mh);

  if (hwm != (dword)-1 && hwm > 0 && hwm >= msgnum)
    SdmSetHighWater(mh,msgnum-1);

  msgapierr=MERR_NONE;
  return 0;
}


// ====================================================================

sword SdmLock(MSG *mh)
{

  if (InvalidMh(mh))
    return -1;

  if (! _SdmRescanArea(mh))
    return -1;

  msgapierr=MERR_NONE;

  return 0;
}

// ====================================================================

sword  SdmUnlock(MSG *mh)
{
  if (InvalidMh(mh))
    return -1;

  msgapierr=MERR_NONE;
  return 0;
}

// ====================================================================

dword SdmGetTextLen(MSGH *msgh)
{
   return (msgh->bodylen - msgh->trailsize);
}

// ====================================================================

dword SdmGetCtrlLen(MSGH *msgh)
{
   return msgh->kludges ? strlen(msgh->kludges)+1 : 0L;
}

// ====================================================================

sword SdmValidate(byte *name)
{
  msgapierr=MERR_NONE;
  return (direxist(name) != FALSE);
}

// ====================================================================

sword SdmSetCurPos(MSGH *msgh, dword pos)
{
  if (InvalidMsgh(msgh))
    return 0;

  lseek(msgh->fd,msgh->cur_pos=pos,SEEK_SET);
  msgapierr=MERR_NONE;
  return 0;
}

// ====================================================================

dword  SdmGetCurPos(MSGH *msgh)
{
  if (InvalidMsgh(msgh))
    return -1L;

  msgapierr=MERR_NONE;
  return msgh->cur_pos;
}

// ====================================================================

UMSGID  SdmMsgnToUid(MSG *mh, dword msgnum)
{
  if (InvalidMh(mh))
    return (UMSGID)0L;

  msgapierr=MERR_NONE;
  return (UMSGID)msgnum;
}

// ====================================================================

dword  SdmUidToMsgn(MSG *mh, UMSGID umsgid, word type)
{
  word wmsgid;
  word mn;

  if (InvalidMh(mh))
    return 0L;

  msgapierr=MERR_NONE;
  wmsgid=(word)umsgid;

  for (mn=0; (dword)mn < mh->num_msg; mn++)
    if (Mhd->msgnum[mn]==wmsgid ||
        (type==UID_NEXT && Mhd->msgnum[mn] >= wmsgid) ||
        (type==UID_PREV && Mhd->msgnum[mn] <= wmsgid &&
        ((dword)(mn+1) >= mh->num_msg || Mhd->msgnum[mn+1] > wmsgid)))
      return ((dword)Mhd->msgnum[mn]);

  msgapierr=MERR_NOENT;
  return 0L;
}

// ====================================================================

dword  SdmGetHighWater(MSG *mh)
{
  MSGH *msgh;
  MIS mis;

  if (InvalidMh(mh))
    return -1;

  /* If we've already fetched the highwater mark... */

  if (mh->high_water != (dword)-1L)
    return (mh->high_water);

  if ((msgh=SdmOpenMsg(mh, MOPEN_READ, 1L))==NULL)
    return 0L;

  if (SdmReadMsg(msgh,&mis,0L,0L,NULL,0L,NULL)==-1 ||
      !eqstr(mis.from,hwm_from))
    mh->high_water=0L;
  else mh->high_water=(dword)mis.replyto;

  FreeMIS(&mis);
  SdmCloseMsg(msgh);

  return (mh->high_water);
}

// ====================================================================

sword  SdmSetHighWater(MSG *mh,dword hwm)
{
  if (InvalidMh(mh))
    return -1;

  /* Only write it to memory for now.  We'll do a complete update of        *
   * the real HWM in 1.MSG only when doing a MsgCloseArea(), to save        *
   * time.                                                                  */

  if (hwm != mh->high_water)
    Mhd->hwm_chgd=TRUE;

  mh->high_water=hwm;
  return 0;
}

// ====================================================================

// First we have the GNU C version

#ifdef __GNUC__

sword near _SdmRescanArea(MSG *mh)
{
  DIR *mydir;
  struct dirent *mydirent;
  size_t len;
  
//  byte temp[_MAX_PATH] = "";
  word mn, thismsg;
	

  mh->num_msg=mh->high_msg=0;
  if(Mhd->msgnum) free(Mhd->msgnum);

  if ((Mhd->msgnum=malloc(SDM_BLOCK*sizeof(int)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  Mhd->msgnum_len=SDM_BLOCK;

//  sprintf(temp,"%s*.msg",Mhd->base);

  if ((mydir=opendir(Mhd->base)) != NULL)
  {
    mn=0;
    
    while ((mydirent=readdir(mydir)) != NULL)
    {
      /* Don't count zero-length or invalid messages */

//      if (ff->ulSize < sizeof(struct _omsg))
//        continue;

      len = strlen(mydirent->d_name);
      if(len < 5 || stricmp(&mydirent->d_name[len-4], ".msg"))
         continue;	// Must end in .MSG!

      if (mn >= Mhd->msgnum_len)
      {
        Mhd->msgnum=realloc(Mhd->msgnum,
                             (Mhd->msgnum_len += SDM_BLOCK) * sizeof(unsigned));

        if (!Mhd->msgnum)
        {
          msgapierr=MERR_NOMEM;
          return FALSE;
        }
      }

      if ((thismsg=(unsigned)atoi(mydirent->d_name)) != 0)
      {
        Mhd->msgnum[mn++]=thismsg;

        if ((dword)thismsg > mh->high_msg)
          mh->high_msg=(dword)thismsg;
        
        mh->num_msg=(dword)mn;
      }

    } 

  closedir(mydir);

  qksort((int *)Mhd->msgnum, (word)mh->num_msg);
  }

  return TRUE;
}


#endif

#if defined(__MSDOS__) || defined(__DOS__)

sword near _SdmRescanArea(MSG *mh)
{
//  #ifndef __WATCOMC__
//  struct ffblk myffblk;
//  #else
  struct find_t myffblk;
//  #define findfirst(a, b, c) _dos_findfirst(a, c, b)
//  #define findnext  _dos_findnext
//  #endif

  byte temp[_MAX_PATH] = "";
  word mn, thismsg;
  FFIND *ff;


  mh->num_msg=mh->high_msg=0;
  if(Mhd->msgnum) free(Mhd->msgnum);

  if ((Mhd->msgnum=malloc(SDM_BLOCK*sizeof(int)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  Mhd->msgnum_len=SDM_BLOCK;

  sprintf(temp,"%s*.msg",Mhd->base);

  if ((ff=FindOpen(temp, 0)) != 0)
  {
    mn=0;

    do
    {
      /* Don't count zero-length or invalid messages */

      if (ff->ulSize < sizeof(struct _omsg))
        continue;

      if (mn >= Mhd->msgnum_len)
      {
        Mhd->msgnum=realloc(Mhd->msgnum,
                             (Mhd->msgnum_len += SDM_BLOCK) * sizeof(unsigned));

        if (!Mhd->msgnum)
        {
          msgapierr=MERR_NOMEM;
          return FALSE;
        }
      }

      if ((thismsg=(unsigned)atoi(ff->szName)) != 0)
      {
        Mhd->msgnum[mn++]=thismsg;

        if ((dword)thismsg > mh->high_msg)
          mh->high_msg=(dword)thismsg;
        
        mh->num_msg=(dword)mn;
      }

      #ifdef OS_2
      if((mn % 128)==127)
        DosSleep(1L);
      #endif
    }
    while (FindNext(ff)==0);

    FindClose(ff);

    qksort((int *)Mhd->msgnum, (word)mh->num_msg);
  }

  return TRUE;
}
#endif
#ifdef __NT__

// NT Version of this.

sword near _SdmRescanArea(MSG *mh)
{
  struct find_t fileinfo;
  unsigned rc;
  byte temp[_MAX_PATH] = "";
  word mn, thismsg;


  mh->num_msg=mh->high_msg=0;
  if(Mhd->msgnum) free(Mhd->msgnum);

  if ((Mhd->msgnum=malloc(SDM_BLOCK*sizeof(int)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  Mhd->msgnum_len=SDM_BLOCK;

  sprintf(temp,"%s*.msg",Mhd->base);

  if (_dos_findfirst(temp, _A_NORMAL, &fileinfo) == 0)
  {
    mn=0;

    do
    {
      /* Don't count zero-length or invalid messages */

      if (fileinfo.size < sizeof(struct _omsg))
        continue;

      if (mn >= Mhd->msgnum_len)
      {
        Mhd->msgnum=realloc(Mhd->msgnum,
                             (Mhd->msgnum_len += SDM_BLOCK) * sizeof(unsigned));

        if (!Mhd->msgnum)
        {
          msgapierr=MERR_NOMEM;
          return FALSE;
        }
      }

      if ((thismsg=(unsigned)atoi(fileinfo.name)) != 0)
      {
        Mhd->msgnum[mn++]=thismsg;

        if ((dword)thismsg > mh->high_msg)
          mh->high_msg=(dword)thismsg;
        
        mh->num_msg=(dword)mn;
      }
    }
    while (_dos_findnext(&fileinfo)==0);

    _dos_findclose(&fileinfo);

    qksort((int *)Mhd->msgnum, (word)mh->num_msg);
  }

  return TRUE;
}

// End of NT version

#endif

#ifdef __OS2__

/* Updated OS/2 version of this stuff */

sword near _SdmRescanArea(MSG *mh)
{
  byte temp[PATHLEN];
  word mn, thismsg;
  HDIR          FindHandle;
  FILEFINDBUF3  FindBuffer;
  ULONG         FindCount;
  APIRET        rc;          /* Return code */

  FindHandle = 0x0001;
  FindCount = 1;


  mh->num_msg=mh->high_msg=0;

  if(Mhd->msgnum) free(Mhd->msgnum);

  if ((Mhd->msgnum=malloc(SDM_BLOCK*sizeof(int)))==NULL)
    {
    msgapierr=MERR_NOMEM;
    return FALSE;
    }

  Mhd->msgnum_len=SDM_BLOCK;

  sprintf(temp,"%s*.msg",Mhd->base);


  rc = DosFindFirst(temp,                  /* File pattern */
                    &FindHandle,           /* Directory search handle */
                    0,                     /* Search attribute */
                    (PVOID) &FindBuffer,   /* Result buffer */
                    sizeof(FindBuffer),    /* Result buffer length */
                    &FindCount,            /* Number of entries to find */
                    FIL_STANDARD);         /* Return level 1 file info */

  if (rc == 0)
    {
    mn=0;

    do
     {
     if (FindBuffer.cbFile < sizeof(struct _omsg))
        continue;

     if (mn >= Mhd->msgnum_len)
       {
       Mhd->msgnum=realloc(Mhd->msgnum,
                             (Mhd->msgnum_len += SDM_BLOCK)*sizeof(int));

       if (!Mhd->msgnum)
         {
         msgapierr=MERR_NOMEM;
         return FALSE;
         }
       }

     if ((thismsg=atoi(FindBuffer.achName)) != 0)
       {
       Mhd->msgnum[mn++]=thismsg;

       if ((dword)thismsg > mh->high_msg)
         mh->high_msg=(dword)thismsg;

       mh->num_msg=(dword)mn;
       }

     }
     while (DosFindNext(FindHandle,           /* Directory handle */
                       (PVOID) &FindBuffer,  /* Result buffer */
                       sizeof(FindBuffer),   /* Result buffer length */
                       &FindCount) == 0);    /* Number of entries to find */


    /* Now sort the list of messages */

    qksort((int *)Mhd->msgnum, (word)mh->num_msg);
    }

  rc = DosFindClose(FindHandle);

  return TRUE;
}

#endif

// ==================================================

int SDMRenumber(MSG *mh)
{
   int mn;
   char oldname[120], newname[120];
   int newposition;

   if (Mhd->msgnum && Mhd->msgnum_len)
     {
     free(Mhd->msgnum);
     Mhd->msgnum = NULL;
     }

   if(_SdmRescanArea(mh) != TRUE)
     return -1;

   for(mn=0; mn < mh->num_msg; mn++)
     {
     if(mh->cur_msg == Mhd->msgnum[mn])
        newposition = mn+1;

     if(Mhd->msgnum[mn] != mn+1)
       {
       sprintf(oldname,"%s%d.msg",Mhd->base, Mhd->msgnum[mn]);
       sprintf(newname,"%s%d.msg",Mhd->base, mn+1);
       if(rename(oldname, newname) != 0)
         break;
       }
     }

   if (Mhd->msgnum && Mhd->msgnum_len)
     {
     free(Mhd->msgnum);
     Mhd->msgnum = NULL;
     }

   if(_SdmRescanArea(mh) != TRUE)
     return -1;

   return newposition;
}

// ====================================================================

void near Get_Binary_Date(struct _stamp *todate,struct _stamp *fromdate,byte *asciidate)
{
  if (fromdate->date.da==0 ||
      fromdate->date.da > 31 ||
      fromdate->date.mo == 0 ||
      fromdate->date.mo > 12 ||
      fromdate->date.yr > 20 ||
      fromdate->date.yr < 10 ||
      fromdate->time.hh > 23 ||
      fromdate->time.mm > 59 ||
      fromdate->time.ss > 59 ||
      ((union stamp_combo *)&fromdate)->ldate==0)
  {
    ASCII_Date_To_Binary(asciidate,(union stamp_combo *)todate);
  }
  else *todate=*fromdate;
}

// ======================================================================

int ReadHdrKludges(MSGH *msgh)
{
  struct _omsg *fmsg;
  unsigned readlen;
  char *buffer, *ctl;
  char *end, *ctrlbuf=NULL;


  if(msgh->mode == MOPEN_RDHDR)
     readlen = sizeof(struct _omsg);
  else
     readlen = sizeof(struct _omsg) + MAX_SDM_CLEN;

  if( (buffer=calloc(1, readlen+1)) == NULL)
    {
    msgapierr = MERR_NOMEM;
    return -1;
    }

  if(read(msgh->fd, buffer, (unsigned) readlen) < (int) sizeof(struct _omsg))
    {
    msgapierr=MERR_BADF;
    free(buffer);
    return -1;
    }

  fmsg = (struct _omsg *)buffer;
  ctl  = buffer + sizeof(struct _omsg);

  // --- Now convert the header

  SDM2MIS((struct _omsg *)buffer, &msgh->mis);

  msgh->mis.msgno    = msgh->msgnum;
  msgh->mis.origbase = msgh->sq->type;

  if(msgh->mode == MOPEN_RDHDR)
    {
    free(buffer);
    return 0;
    }

  // --- Then analyse the kludges (length, flags)

  ctrlbuf = (char *) CopyToControlBuf(ctl,
                                      &end,
                                      (unsigned *)NULL);

  msgh->rawclen = (dword) (end - ctl);

  if(ctrlbuf)
    {
    msgh->kludges = strdup(ctrlbuf);
    free(ctrlbuf);
    }

  ConvertControlInfo(msgh->kludges, &msgh->mis);

  msgh->clen = strlen(msgh->kludges) + 1;

  free(buffer);

  return 0;

}

// ======================================================================

void SDM2MIS(struct _omsg *sdmhdr, MIS *mis)
{
  struct _stamp tempdate;
  struct tm tmdate;
  int l;

  memset(mis, '\0', sizeof(mis));

  for(l=0; l<SQNUMATTR1; l++)
   {
   if(sdmhdr->attr & SQAttrTable1[l][1])
      mis->attr1 |= SQAttrTable1[l][0];
   }

  if( (!mi.useflags) &&
      (sdmhdr->attr & MSGCRASH) &&
      (sdmhdr->attr & MSGHOLD)  )
    {
    mis->attr1 |= aDIR;
    mis->attr1 &= ~(aCRASH|aHOLD);
    }

  mis->origfido.zone = mis->destfido.zone = mi.def_zone;
  mis->origfido.net  = sdmhdr->orig_net;
  mis->origfido.node = sdmhdr->orig;
  mis->destfido.net  = sdmhdr->dest_net;
  mis->destfido.node = sdmhdr->dest;

  /* Convert 4d pointnets !? */

  if (sdmhdr->times==~sdmhdr->cost && sdmhdr->times)
    mis->origfido.point=sdmhdr->times;

  memcpy(mis->to,   sdmhdr->to, 36);
  memcpy(mis->from, sdmhdr->from, 36);
  memcpy(mis->subj, sdmhdr->subj, 72);
  memcpy(mis->ftsc_date, sdmhdr->date, 20);

  Get_Binary_Date(&tempdate, &sdmhdr->date_written, sdmhdr->date);
  DosDate_to_TmDate(&tempdate, &tmdate);
  mis->msgwritten = JAMsysMkTime(&tmdate);

  Get_Binary_Date(&tempdate, &sdmhdr->date_arrived, sdmhdr->date);
  DosDate_to_TmDate(&tempdate, &tmdate);
  mis->msgprocessed = JAMsysMkTime(&tmdate);

  mis->replyto    = sdmhdr->reply;
  mis->replies[0] = sdmhdr->up;
  mis->timesread  = sdmhdr->times;

  // Now we have to fiddle with file attaches and requests..

  if(sdmhdr->attr & (MSGFRQ|MSGURQ))  // extract requested files from subject.
    Extract_Requests(mis);

  if(sdmhdr->attr & MSGFILE)          // extract attached files from subject.
    Extract_Attaches(mis);
}

// ======================================================================

int SDMCalcTrailing(MSGH *msgh)
{
   unsigned readlen = MAXTRAILSIZE;
   long start, filelen;
   char *txtptr;

   filelen = filelength(msgh->fd);
   msgh->bodylen = filelen - sizeof(struct _omsg) - msgh->rawclen;

   if(msgh->bodylen < readlen) readlen = msgh->bodylen;

   if(readlen == 0) return 0;  // Nothing to do, no body.

   // Where to start reading message.
   start = filelen - readlen;

   if(lseek(msgh->fd, start, SEEK_SET) != start) return -1;

   if((txtptr = calloc(1, readlen+1)) == NULL)
     return -1;

   if(read(msgh->fd, txtptr, (unsigned)readlen) != (int) readlen)
     {
     free(txtptr);
     return -1;
     }

   msgh->trailsize = AnalyseTrail(txtptr, readlen, &msgh->mis);

   free(txtptr);
   return 0;

}

// ======================================================================

char * MIS2SDM(MSGH *msgh, MIS *mis, struct _omsg *sdmhdr, char *kludges)
{
   int i;
   dword maxlen, curlen;
   MSG *sq = msgh->sq;
   JAMTM *tmdate;
   struct tm *timestamp;
   char temp[150];
   dword curtime;
   char *newkludges = NULL;

   // We reserve extra space, *and* copy over the old kludges. We can
   // easily add any info now (FLAGS, INTL etc), without changing the
   // original kludges passed to us by the calling app..

   maxlen = kludges ? strlen(kludges) : 0;
   maxlen += 200;
   if( (newkludges = calloc(1, maxlen)) == NULL)
     return NULL;

   if(kludges)
     strcpy(newkludges, kludges);

   curlen = strlen(newkludges);

   memset(sdmhdr, '\0', sizeof(struct _omsg));

   memcpy(sdmhdr->from, mis->from, min(35, strlen(mis->from)));
   memcpy(sdmhdr->to, mis->to, min(35, strlen(mis->to)));
   memcpy(sdmhdr->subj, mis->subj, min(71, strlen(mis->subj)));

   sdmhdr->orig = mis->origfido.node;
   sdmhdr->dest = mis->destfido.node;
   sdmhdr->orig_net = mis->origfido.net;
   sdmhdr->dest_net = mis->destfido.net;

   for(i=0; i<SQNUMATTR1; i++)
    {
    if(mis->attr1 & SQAttrTable1[i][0])
       sdmhdr->attr |= SQAttrTable1[i][1];
    }

   // ---

   /* First we check for any 'FLAGS' kludges to be written.. */

   if(sq->type & (MSGTYPE_NET|MSGTYPE_MAIL))  /* Do we have to do it? */
     {
     if((mis->attr1 & SQADDMASK1) || (mis->attr2 & SQADDMASK2))
       {
       if(!mi.useflags) /* We can only simulate Direct :-( */
          {
          if(mis->attr1 & aDIR)
             sdmhdr->attr |= (MSGHOLD | MSGCRASH);
          }
       else
          {
          Attr2Flags(temp, (mis->attr1 & SQADDMASK1), (mis->attr2 & SQADDMASK2));
          AddToString(&newkludges, &maxlen, &curlen, temp);
          }
       }

     // Now check for addition of INTL, FMPT, TOPT..
     if ((mis->destfido.zone != mi.def_zone || mis->origfido.zone != mi.def_zone) &&
         !stristr(newkludges,"\x01INTL"))
        {
        sprintf(temp,"\x01INTL %hu:%hu/%hu %hu:%hu/%hu",
                mis->destfido.zone,mis->destfido.net,mis->destfido.node,
                mis->origfido.zone,mis->origfido.net,mis->origfido.node);
        AddToString(&newkludges, &maxlen, &curlen, temp);
        }

     if(mis->origfido.point && !strstr(newkludges,"\x01""FMPT"))
       {
       sprintf(temp,"\x01""FMPT %hu",mis->origfido.point);
       AddToString(&newkludges, &maxlen, &curlen, temp);
       }

     if(mis->destfido.point && !strstr(newkludges,"\x01""TOPT"))
       {
       sprintf(temp,"\x01""TOPT %hu",mis->destfido.point);
       AddToString(&newkludges, &maxlen, &curlen, temp);
       }
     }

   // ---

   sdmhdr->reply = mis->replyto;
   sdmhdr->up    = mis->replies[0];
   sdmhdr->times = mis->timesread;

   tmdate = JAMsysLocalTime((dword *)&mis->msgwritten);
   TmDate_to_DosDate(tmdate, &sdmhdr->date_written);

   if((dword) mis->msgprocessed != 0L)
      tmdate = JAMsysLocalTime((dword *)&mis->msgprocessed);
   else
      {
      curtime = (dword) JAMsysTime(NULL);
      tmdate = JAMsysLocalTime(&curtime); // We need something here for Squish..
      }
   TmDate_to_DosDate(tmdate, &sdmhdr->date_arrived);

   timestamp = JAMsysLocalTime(&mis->msgwritten);
   sprintf(sdmhdr->date, "%02d %s %02d  %02d:%02d:%02d",
       timestamp->tm_mday, months_ab[timestamp->tm_mon],
       timestamp->tm_year, timestamp->tm_hour,
       timestamp->tm_min, timestamp->tm_sec);

   Files2Subject(mis, sdmhdr->subj);

   return (newkludges ? newkludges : NULL);

}

// ======================================================================

#ifdef __NEVER__

void CheckReadOnly(char *filename)
{
  HFILE       FileHandle;      /* File handle */
  ULONG       FileInfoLevel;   /* File info data required */
  FILESTATUS  FileInfoBuf;     /* File info buffer */
  ULONG       FileInfoBufSize; /* File info buffer size */
  APIRET      rc;              /* Return code */
  ULONG       Action;

  #define OPEN_FILE 0x01
  #define FILE_EXISTS OPEN_FILE
  #define SHARE_FLAG 0x10
//  #define ACCESS_FLAG 0x02
  #define ACCESS_FLAG 0x00


  rc = DosOpen(filename,                /* File path name */
               &FileHandle,             /* File handle */
               &Action,                 /* Action taken */
               0L,                      /* File primary allocation */
               0L,                      /* File attribute */
               FILE_EXISTS,             /* Open function type */
               SHARE_FLAG | ACCESS_FLAG,
               0L);                     /* No extended attributes */

  if(rc != 0)
      return;

  FileInfoLevel = 1;    /* Indicate that Level 1 information */
                        /*   is desired                      */

  FileInfoBufSize = sizeof(FILESTATUS);
                        /* Size of the buffer that will      */
                        /*   receive the Level 1 information */

  rc = DosQueryFileInfo(FileHandle, FileInfoLevel,
                        &FileInfoBuf, FileInfoBufSize);
                        /* Obtain a copy of the Level 1 */
                        /*   file information           */

  if(rc != 0)
    {
    DosClose(FileHandle);
    return;
    }

  if(FileInfoBuf.attrFile & FILE_READONLY)
    {
    FileInfoBuf.attrFile &= ~FILE_READONLY;

    rc = DosSetFileInfo(FileHandle, FileInfoLevel,
                      &FileInfoBuf, FileInfoBufSize);
    }

  DosClose(FileHandle);

}

void CheckReadOnly(char *filename)
{
   unsigned attribs;

   if(_dos_getfileattr(filename, &attribs) != 0)
      return;

   if(attribs & _A_RDONLY)
     {
     attribs &= ~_A_RDONLY;
     dos_setfileattr(filename, &attribs);
     }
}
#else

void CheckReadOnly(char *filename)
{
   chmod(filename, S_IRWXO | S_IRWXU);
}

#endif


// ======================================================================

