/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/


sword HMBCloseArea(MSGA * sq);
MSGH *HMBOpenMsg(MSGA * sq, word mode, dword msgnum);
sword HMBCloseMsg(MSGH * msgh);
dword HMBReadMsg(MSGH * msgh, MIS * mis, dword offset, dword bytes,
                 byte * text, dword clen, byte * ctxt);
sword HMBWriteMsg(MSGH * msgh, word append, MIS * mis, byte * text,
                  dword textlen, dword totlen, dword clen, byte * ctxt);
sword HMBKillMsg(MSGA * sq, dword msgnum);
sword HMBLock(MSGA * sq);
sword HMBUnlock(MSGA * sq);
sword HMBSetCurPos(MSGH * msgh, dword pos);
dword HMBGetCurPos(MSGH * msgh);
UMSGID HMBMsgnToUid(MSGA * sq, dword msgnum);
dword HMBUidToMsgn(MSGA * sq, UMSGID umsgid, word type);
dword HMBGetHighWater(MSGA * mh);
sword HMBSetHighWater(MSGA * sq, dword hwm);
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
