// Proto from binkpack

char * fancy_address(NETADDR *this);
char * OutboundName(NETADDR *address);
int    MakeFlo(char *filename, int CreateOnly, char *newsubject);
int    MakeReq(char *filename, int update, char *newsubject);
int    MakeBusyFlag  (char *location);
void   RemoveBusyFlag(char *location);
int    SkipPacketHeader(int handle, int SeekToEnd);

int PackMail(NETADDR *orig, NETADDR *dest, dword current, char *password);
int MoveMail(NETADDR *orig, NETADDR *dest, char *dir, dword current, char *password);
int Truncate(char *filename);
int CopyFile(char *filename, char *todir, int delete);


