
void  CopyMsg(MSG *areahandle, MSG *toareahandle, AREA *toarea, dword msgno, char kill);
void  MoveMessage(MSG *areahandle, AREA *area, MMSG *curmsg);
dword clean_end_message(char *msgbuf);
int   clean_origin(char *msgbuf);
void  invalidate_origin(char *msg);