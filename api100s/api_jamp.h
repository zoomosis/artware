/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

/* $Id: api_sqp.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

static sword EXPENTRY JAMCloseArea(MSG *sq);
static MSGH * EXPENTRY JAMOpenMsg(MSG *sq,word mode,dword msgnum);
static sword EXPENTRY JAMCloseMsg(MSGH *msgh);
static dword EXPENTRY JAMReadMsg(MSGH *msgh,XMSG *msg,dword offset,dword bytes,byte *text,dword clen,byte *ctxt);
static sword EXPENTRY JAMWriteMsg(MSGH *msgh,word append,XMSG *msg,byte *text,dword textlen,dword totlen,dword clen,byte *ctxt);
static sword EXPENTRY JAMKillMsg(MSG *sq,dword msgnum);
static sword EXPENTRY JAMLock(MSG *sq);
static sword EXPENTRY JAMUnlock(MSG *sq);
static sword EXPENTRY JAMSetCurPos(MSGH *msgh,dword pos);
static dword EXPENTRY JAMGetCurPos(MSGH *msgh);
static UMSGID EXPENTRY JAMMsgnToUid(MSG *sq,dword msgnum);
static dword EXPENTRY JAMUidToMsgn(MSG *sq,UMSGID umsgid,word type);
static dword EXPENTRY JAMGetHighWater(MSG *mh);
static sword EXPENTRY JAMSetHighWater(MSG *sq,dword hwm);
static dword EXPENTRY JAMGetTextLen(MSGH *msgh);
static dword EXPENTRY JAMGetCtrlLen(MSGH *msgh);



static struct _apifuncs JAM_funcs=
{
  JAMCloseArea,
  JAMOpenMsg,
  JAMCloseMsg,
  JAMReadMsg,
  JAMWriteMsg,
  JAMKillMsg,
  JAMLock,
  JAMUnlock,
  JAMSetCurPos,
  JAMGetCurPos,
  JAMMsgnToUid,
  JAMUidToMsgn,
  JAMGetHighWater,
  JAMSetHighWater,
  JAMGetTextLen,
  JAMGetCtrlLen
};


