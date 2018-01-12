

int ReadArea(AREA *area);


long GetLast(AREA *area, MSG *areahandle);
void ReleaseMsg(MMSG *thismsg, int allofit);
void UpdateLastread(AREA *area, long last, MSG *areahandle);
long MsgGetLowMsg(MSG *areahandle);
void beep (void);
void showmem(void);
void ScanArea(AREA *area, MSG *areahandle);
dword anchor(int direction, MSG *areahandle);