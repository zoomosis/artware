/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

/* $Id: api_sqp.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

static sword EXPENTRY HMBCloseArea(MSG *sq);
static MSGH * EXPENTRY HMBOpenMsg(MSG *sq,word mode,dword msgnum);
static sword EXPENTRY HMBCloseMsg(MSGH *msgh);
static dword EXPENTRY HMBReadMsg(MSGH *msgh,XMSG *msg,dword offset,dword bytes,byte *text,dword clen,byte *ctxt);
static sword EXPENTRY HMBWriteMsg(MSGH *msgh,word append,XMSG *msg,byte *text,dword textlen,dword totlen,dword clen,byte *ctxt);
static sword EXPENTRY HMBKillMsg(MSG *sq,dword msgnum);
static sword EXPENTRY HMBLock(MSG *sq);
static sword EXPENTRY HMBUnlock(MSG *sq);
static sword EXPENTRY HMBSetCurPos(MSGH *msgh,dword pos);
static dword EXPENTRY HMBGetCurPos(MSGH *msgh);
static UMSGID EXPENTRY HMBMsgnToUid(MSG *sq,dword msgnum);
static dword EXPENTRY HMBUidToMsgn(MSG *sq,UMSGID umsgid,word type);
static dword EXPENTRY HMBGetHighWater(MSG *mh);
static sword EXPENTRY HMBSetHighWater(MSG *sq,dword hwm);
static dword EXPENTRY HMBGetTextLen(MSGH *msgh);
static dword EXPENTRY HMBGetCtrlLen(MSGH *msgh);



static struct _apifuncs HMB_funcs=
{
  HMBCloseArea,
  HMBOpenMsg,
  HMBCloseMsg,
  HMBReadMsg,
  HMBWriteMsg,
  HMBKillMsg,
  HMBLock,
  HMBUnlock,
  HMBSetCurPos,
  HMBGetCurPos,
  HMBMsgnToUid,
  HMBUidToMsgn,
  HMBGetHighWater,
  HMBSetHighWater,
  HMBGetTextLen,
  HMBGetCtrlLen
};


