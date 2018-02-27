/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

/* $Id: api_sqp.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

sword JAMCloseArea(MSGA * sq);
MSGH *JAMOpenMsg(MSGA * sq, word mode, dword msgnum);
sword JAMCloseMsg(MSGH * msgh);
dword JAMReadMsg(MSGH * msgh, MIS * mis, dword offset, dword bytes,
                 byte * text, dword clen, byte * ctxt);
sword JAMWriteMsg(MSGH * msgh, word append, MIS * mis, byte * text,
                  dword textlen, dword totlen, dword clen, byte * ctxt);
sword JAMKillMsg(MSGA * sq, dword msgnum);
sword JAMLock(MSGA * sq);
sword JAMUnlock(MSGA * sq);
sword JAMSetCurPos(MSGH * msgh, dword pos);
dword JAMGetCurPos(MSGH * msgh);
UMSGID JAMMsgnToUid(MSGA * sq, dword msgnum);
dword JAMUidToMsgn(MSGA * sq, UMSGID umsgid, word type);
dword JAMGetHighWater(MSGA * mh);
sword JAMSetHighWater(MSGA * sq, dword hwm);
dword JAMGetTextLen(MSGH * msgh);
dword JAMGetCtrlLen(MSGH * msgh);


static struct _apifuncs JAM_funcs = {
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
