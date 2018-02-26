/***************************************************************************
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

/* $Id: api_sqp.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

sword JAMCloseArea(MSG * sq);
MSGH *JAMOpenMsg(MSG * sq, word mode, dword msgnum);
sword JAMCloseMsg(MSGH * msgh);
dword JAMReadMsg(MSGH * msgh, MIS * mis, dword offset, dword bytes,
                 byte * text, dword clen, byte * ctxt);
sword JAMWriteMsg(MSGH * msgh, word append, MIS * mis, byte * text,
                  dword textlen, dword totlen, dword clen, byte * ctxt);
sword JAMKillMsg(MSG * sq, dword msgnum);
sword JAMLock(MSG * sq);
sword JAMUnlock(MSG * sq);
sword JAMSetCurPos(MSGH * msgh, dword pos);
dword JAMGetCurPos(MSGH * msgh);
UMSGID JAMMsgnToUid(MSG * sq, dword msgnum);
dword JAMUidToMsgn(MSG * sq, UMSGID umsgid, word type);
dword JAMGetHighWater(MSG * mh);
sword JAMSetHighWater(MSG * sq, dword hwm);
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
