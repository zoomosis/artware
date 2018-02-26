sword SquishCloseArea(MSG * sq);
MSGH *SquishOpenMsg(MSG * sq, word mode, dword msgnum);
sword SquishCloseMsg(MSGH * msgh);
dword SquishReadMsg(MSGH * msgh, MIS * msg, dword offset, dword bytes,
                    byte * text, dword clen, byte * ctxt);
sword SquishWriteMsg(MSGH * msgh, word append, MIS * msg, byte * text,
                     dword textlen, dword totlen, dword clen, byte * ctxt);
sword SquishKillMsg(MSG * sq, dword msgnum);
sword SquishLock(MSG * sq);
sword SquishUnlock(MSG * sq);
sword SquishSetCurPos(MSGH * msgh, dword pos);
dword SquishGetCurPos(MSGH * msgh);
UMSGID SquishMsgnToUid(MSG * sq, dword msgnum);
dword SquishUidToMsgn(MSG * sq, UMSGID umsgid, word type);
dword SquishGetHighWater(MSG * mh);
sword SquishSetHighWater(MSG * sq, dword hwm);
dword SquishGetTextLen(MSGH * msgh);
dword SquishGetCtrlLen(MSGH * msgh);
sword _OpenSquish(MSG * sq, word * mode);
SQHDR *_SquishGotoMsg(MSG * sq, dword msgnum, FOFS * seek_frame,
                      SQIDX * idx, word updptrs);
MSGH *_SquishOpenMsgRead(MSG * sq, word mode, dword msgnum);
sword _SquishReadHeader(MSG * sq, dword ofs, SQHDR * hdr);
sword _SquishWriteHeader(MSG * sq, dword ofs, SQHDR * hdr);
sword _SquishUpdateHeaderNext(MSG * sq, dword ofs, SQHDR * hdr,
                              dword newval);
sword _SquishUpdateHeaderPrev(MSG * sq, dword ofs, SQHDR * hdr,
                              dword newval);
sword _SquishWriteSq(MSG * sq);
sword _SquishUpdateSq(MSG * sq, word force);
void Init_Hdr(SQHDR * sh);
void SqbaseToSq(struct _sqbase *sqbase, MSG * sq);
void SqToSqbase(MSG * sq, struct _sqbase *sqbase);
sword AddIndex(MSG * sq, SQIDX * ix, dword msgnum);
sword Add_To_Free_Chain(MSG * sq, FOFS killofs, SQHDR * killhdr);
sword _SquishReadIndex(MSG * sq);
sword _SquishWriteIndex(MSG * sq);
sword _SquishGetIdxFrame(MSG * sq, dword num, SQIDX * idx);
int _SquishLock(MSG * sq);
void _SquishUnlock(MSG * sq);
sword _SquishFindFree(MSG * sq, FOFS * this_frame, dword totlen,
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
