// Raw character block functions

RAWBLOCK * InitRawblock(unsigned startlen, unsigned delta, unsigned maxlen);
void       AddToBlock (RAWBLOCK *blk, char *s, unsigned len);
char       GetLastChar(RAWBLOCK *blk);
void       StripLastChar(RAWBLOCK *blk);
void       SetLastChar(RAWBLOCK *blk, char c);
RAWBLOCK * JoinLastBlocks(RAWBLOCK *first, int minsize);
void       FreeBlocks(RAWBLOCK *blk);

