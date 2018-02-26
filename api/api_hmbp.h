/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/


sword HMBCloseArea(MSG * sq);
MSGH *HMBOpenMsg(MSG * sq, word mode, dword msgnum);
sword HMBCloseMsg(MSGH * msgh);
dword HMBReadMsg(MSGH * msgh, MIS * mis, dword offset, dword bytes,
                 byte * text, dword clen, byte * ctxt);
sword HMBWriteMsg(MSGH * msgh, word append, MIS * mis, byte * text,
                  dword textlen, dword totlen, dword clen, byte * ctxt);
sword HMBKillMsg(MSG * sq, dword msgnum);
sword HMBLock(MSG * sq);
sword HMBUnlock(MSG * sq);
sword HMBSetCurPos(MSGH * msgh, dword pos);
dword HMBGetCurPos(MSGH * msgh);
UMSGID HMBMsgnToUid(MSG * sq, dword msgnum);
dword HMBUidToMsgn(MSG * sq, UMSGID umsgid, word type);
dword HMBGetHighWater(MSG * mh);
sword HMBSetHighWater(MSG * sq, dword hwm);
dword HMBGetTextLen(MSGH * msgh);
dword HMBGetCtrlLen(MSGH * msgh);


static struct _apifuncs HMB_funcs = {
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
