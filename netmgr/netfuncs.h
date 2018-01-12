void print_and_log (char *line, ...);

char  * MakeKludge       (char *curkludge, NETADDR *address);
dword   clean_end_message (char *msgbuf);

char * formaddress(NETADDR *this);
char * MakeT(dword t);
void   GetRawMsg(dword bytelimit);
int    SaveMessage(MSG *areahandle, MIS *mis, RAWBLOCK *blk, char *kludges, dword no, int intlforce);
int    IsCurrentArea(char *area);
int    matchaddress  (NETADDR *msg, NETADDR *mask);

void AddNote       (char * bouncefile, dword current, int replacebody);
void writemsg      (dword current, char * file, int writewhat);
void bounce        (char * bouncefile, dword current, NETADDR * address, int action, MASK *forwardmask, int extended, char *destarea);

