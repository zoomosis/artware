sword SquishCloseArea(MSGA * sq);
MSGH *SquishOpenMsg(MSGA * sq, word mode, dword msgnum);
sword SquishCloseMsg(MSGH * msgh);
dword SquishReadMsg(MSGH * msgh, MIS * msg, dword offset, dword bytes,
                    byte * text, dword clen, byte * ctxt);
sword SquishWriteMsg(MSGH * msgh, word append, MIS * msg, byte * text,
                     dword textlen, dword totlen, dword clen, byte * ctxt);
sword SquishKillMsg(MSGA * sq, dword msgnum);
sword SquishLock(MSGA * sq);
sword SquishUnlock(MSGA * sq);
sword SquishSetCurPos(MSGH * msgh, dword pos);
dword SquishGetCurPos(MSGH * msgh);
UMSGID SquishMsgnToUid(MSGA * sq, dword msgnum);
dword SquishUidToMsgn(MSGA * sq, UMSGID umsgid, word type);
dword SquishGetHighWater(MSGA * mh);
sword SquishSetHighWater(MSGA * sq, dword hwm);
dword SquishGetTextLen(MSGH * msgh);
dword SquishGetCtrlLen(MSGH * msgh);
sword _OpenSquish(MSGA * sq, word * mode);
SQHDR *_SquishGotoMsg(MSGA * sq, dword msgnum, FOFS * seek_frame,
                      SQIDX * idx, word updptrs);
MSGH *_SquishOpenMsgRead(MSGA * sq, word mode, dword msgnum);
sword _SquishReadHeader(MSGA * sq, dword ofs, SQHDR * hdr);
sword _SquishWriteHeader(MSGA * sq, dword ofs, SQHDR * hdr);
sword _SquishUpdateHeaderNext(MSGA * sq, dword ofs, SQHDR * hdr,
                              dword newval);
sword _SquishUpdateHeaderPrev(MSGA * sq, dword ofs, SQHDR * hdr,
                              dword newval);
sword _SquishWriteSq(MSGA * sq);
sword _SquishUpdateSq(MSGA * sq, word force);
void Init_Hdr(SQHDR * sh);
void SqbaseToSq(struct _sqbase *sqbase, MSGA * sq);
void SqToSqbase(MSGA * sq, struct _sqbase *sqbase);
sword AddIndex(MSGA * sq, SQIDX * ix, dword msgnum);
sword Add_To_Free_Chain(MSGA * sq, FOFS killofs, SQHDR * killhdr);
sword _SquishReadIndex(MSGA * sq);
sword _SquishWriteIndex(MSGA * sq);
sword _SquishGetIdxFrame(MSGA * sq, dword num, SQIDX * idx);
int _SquishLock(MSGA * sq);
void _SquishUnlock(MSGA * sq);
sword _SquishFindFree(MSGA * sq, FOFS * this_frame, dword totlen,
                      dword clen, SQHDR * freehdr,
                      FOFS * last_frame, SQHDR * lhdr, MSGH * msgh);


#define fop_wpb (O_CREAT | O_TRUNC | O_RDWR | O_BINARY)
#define fop_rpb (O_RDWR | O_BINARY)

#define Sqd ((struct _sqdata *)(sq->apidata))
#define MsghSqd ((struct _sqdata *)(((struct _msgh far *)msgh)->sq->apidata))


static struct _apifuncs sq_funcs = {
    SquishCloseArea,
    SquishOpenMsg,
    SquishCloseMsg,
    SquishReadMsg,
    SquishWriteMsg,
    SquishKillMsg,
    SquishLock,
    SquishUnlock,
    SquishSetCurPos,
    SquishGetCurPos,
    SquishMsgnToUid,
    SquishUidToMsgn,
    SquishGetHighWater,
    SquishSetHighWater,
    SquishGetTextLen,
    SquishGetCtrlLen
};


static byte *ss_sqd = "%s.sqd";
static byte *ss_sqi = "%s.sqi";

//static struct _sqdata * _junksq;
