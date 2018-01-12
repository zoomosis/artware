
sword  SdmCloseArea(MSG *mh);
MSGH *  SdmOpenMsg(MSG *mh,word mode,dword msgnum);
sword  SdmCloseMsg(MSGH *msgh);
dword  SdmReadMsg(MSGH *msgh,MIS *msg,dword offset,dword bytes,byte *text,dword clen,byte *ctxt);
sword  SdmWriteMsg(MSGH *msgh,word append,MIS *msg,byte *text,dword textlen,dword totlen,dword clen,byte *ctxt);
sword  SdmKillMsg(MSG *mh,dword msgnum);
sword  SdmLock(MSG *mh);
sword  SdmUnlock(MSG *mh);
sword  SdmSetCurPos(MSGH *msgh,dword pos);
dword  SdmGetCurPos(MSGH *msgh);
UMSGID  SdmMsgnToUid(MSG *mh,dword msgnum);
dword  SdmUidToMsgn(MSG *mh,UMSGID umsgid,word type);
dword  SdmGetHighWater(MSG *mh);
sword  SdmSetHighWater(MSG *sq,dword hwm);
dword  SdmGetTextLen(MSGH *msgh);
dword  SdmGetCtrlLen(MSGH *msgh);

static byte *sd_msg="%s%u.msg";

/* Pointer to 'struct _sdmdata' so we can get Turbo Debugger to use         *
 * the _sdmdata structure...                                                */

// static struct _sdmdata *_junksqd;

static struct _apifuncs sdm_funcs=
{
  SdmCloseArea,
  SdmOpenMsg,
  SdmCloseMsg,
  SdmReadMsg,
  SdmWriteMsg,
  SdmKillMsg,
  SdmLock,
  SdmUnlock,
  SdmSetCurPos,
  SdmGetCurPos,
  SdmMsgnToUid,
  SdmUidToMsgn,
  SdmGetHighWater,
  SdmSetHighWater,
  SdmGetTextLen,
  SdmGetCtrlLen,
};
