
int check_attach (MIS *mis, char *mask, int copyfiles);
void     writefa(MIS *mis, MSG *areahandle, AREA *area, int exceptfirst);

int AttemptLock(MSG *areahandle);

void   bmgCompile(char *s, bmgARG *pattern, int ignore_case, int whole);
char * bmgSearch(char *buffer, int buflen, bmgARG *pattern, char howstrict);


void check_cc(AREA *area, MSG *areahandle, MIS *mis, RAWBLOCK **first);
int  choose_name(void);
int  choose_address(void);
void clean_index(AREA *area);void readconfig(void);
void CopyMsg(MSG *areahandle, MSG *toareahandle, AREA *toarea, dword msgno, char kill);
void CopyMarked(AREA *area, AREA *toarea, MSG *areahandle, MSG *toareahandle, int kill);
void DeleteMarked(AREA *area, MSG *areahandle);

void  MoveMessage(MSG *areahandle, AREA *area, MMSG *curmsg);
dword clean_end_message(char *msgbuf);
int   clean_origin(char *msgbuf, char **startorig);
void  invalidate_origin(char *msg);

int get_quote_string(char *line, char *quote);

// Nodelist seraching stuff

ADDRLIST * fido_lookup(char *name);
ADDRLIST * NodeLookup(char *name, int sysop, int prompt);


void FindMessage (MSG *areahandle, AREA *area, long last);

RAWBLOCK * GetBody(AREA *area, MSG *areahandle, MMSG *curmsg, char *origline, LINE *firstline, int checkchange, int startline);
int WriteRawBody(RAWBLOCK *blk, char *curfile);

LINE *edittext(LINE *begin, AREA *area, MSG *areahandle, MMSG *curmsg, int curtxtline);
int  ShowEditMenu(MMSG *curmsg, int escallowed);


MMSG * GetMsg       (long curno, MSG *areahandle, int doformat, AREA *area);
MMSG * GetFmtMsg    (long curno, MSG *areahandle, AREA *area);
int    MarkReceived (MSG *areahandle, AREA *area, dword no, int not, int hasCFM);
MMSG * GetRawMsg    (long curno, MSG *areahandle, int what, int convert);
void   checklines   (LINE *first);

int  MakeHeader    (MSG *areahandle, MMSG *curmsg, int reply, AREA *area, UMSGID reply_to_id, char *subject, MMSG *newmmsg);
int  EditHeader    (MSG *areahandle, MMSG *curmsg, int address, int aka, int domatch, AREA *area);
int  SetAttributes (MMSG *curmsg, word base, int fullscreen);

void statusbar     (char *s);
int  matchaka      (MIS *mis);
void show_help     (int cat);
int  addrcmp       (NETADDR *one, NETADDR *two);

int  get_idle_key   (char allowstuff, int scope);
void stuffkey       (int key);
void kbflush        (void);
void check_enhanced (void);
int  xkbhit         (void);
void MacroStart     (sword i);

int   getstring(int row, int col, char *answer, int len, int maxlen, char *legalset, word colour, word exitcol);
dword MsgList(MSG *areahandle, AREA *area, dword curno, char wide);

int  maint_menu (AREA *area);
void Message    (char *text, int pause, int stop, YesNo save);
int  confirm    (char *text);


int check_node(MIS *mis, int aka, int prompt);

/* Should timEd ask confirmation if only 1 address found? */

int  pickone(char **choices, int y1, int x1, int y2, int x2);
void free_picklist(char **choices);
int picklist(char **choices, char **help, char *title, int y1, int x1, int y2, int x2);


void PrintMessage(MMSG *curmsg, AREA *area, MSG *areahandle, int hardcopy, int tagged, int what);
FILE * OpenExist(char *default_name, AREA *area, MSG *areahandle, MMSG *curmsg);
void do_print(FILE *outfile, AREA *area, MMSG *curmsg, int what);

void inst_newcrit(void);
void inst_oldcrit(void);

int ReadArea(AREA *area);


long GetLast(AREA *area, MSG *areahandle, int raw);
void ReleaseMsg(MMSG *thismsg, int allofit);
void UpdateLastread(AREA *area, long last, dword highest, MSG *areahandle);
long MsgGetLowMsg(MSG *areahandle);
void beep (void);
void showmem(void);
void ScanArea(AREA *area, MSG *areahandle, int raw);
dword anchor(int direction, MSG *areahandle);
void check_registration(void);
int read_key(void);
void edit_hello_strings(AREA *area);
void get_custom_info(AREA *area);
long get_last_squish(AREA *area);


char *MakeKludge   (MMSG *curmsg, MIS *mis, int netmail);
void MakeMessage   (MMSG *curmsg, AREA *area, MSG *areahandle, word reply, UMSGID reply_to_id, char *add_to_top);
void ChangeMessage (MMSG *curmsg, AREA *area, MSG *areahandle, int bodytoo);
#ifndef __WATCOMC__
int  pascal IsQuote       (char *line);
#else
int  IsQuote        (char *line);
#endif
void MakeText       (MMSG *curmsg, FILE *parameterfile);
void ReplyOther     (MMSG *curmsg, AREA *area);
int  check_alias    (MIS *mis);
char *make_origin   (int aka);
void address_expand (char *line, NETADDR *fa, int aka);
void zonegate       (MIS *mis, char *kludges, int showinfo, dword base);
//void check_direct (MIS *mis, char *kludges);
int SaveMessage     (MSG *areahandle, AREA *area, MIS *mis, RAWBLOCK *blk, char *kludges, dword no, int preserve);
char *expand        (char *line, MIS *mis, MIS *newmis);


int get_request (MMSG *curmsg, AREA *area, MSG *areahandle);

AREA *SelectArea (AREA *first, int pickonly, AREA *area_to_start);
int  AreaVisible (AREA *curptr);
int  fastscan    (AREA *curarea, int personal);

int    shell_to_DOS (void);
int    runexternal  (AREA *area, MSG **areahandle, MMSG *curmsg, long lastorhigh, dword highest);
char * BuildCommandLine(char *charptr, AREA *area, MSG *areahandle, MMSG *curmsg, char *curfile, char *newfile);
int    runaprog(char *prog, char *parms, int waitforkey);
int    editrunexternal(AREA *area, MSG *areahandle, MMSG *curmsg);
int    FileDelete(void);

void   paint_header (MMSG *curmsg, word type);
char * attr_to_txt  (dword attr1, dword attr2);
char * FormAddress  (NETADDR *address);
char * MakeT        (dword t, int type);
void   SumAttachesRequests(MIS *mis, char *temp, int maxlen, int what);

void showinfo     (MMSG *curmsg, AREA *area, MSG *areahandle);

dword ShowMsg  (MMSG *msg, AREA *area, MSG *areahandle, int displaytype);
dword showbody (MMSG *curmsg, MSG *areahandle, AREA *area, int displaytype);

char *MakeRep  (MMSG *msg, MSG *areahandle, AREA *area);

void add_tosslog(AREA *area, MSG *areahandle);

void unreceive(MMSG *curmsg, MSG *areahandle, AREA *area);

void working(int y, int x, int col);

LINE *FormatText(char *txt, int ll);
LINE *fastFormatText(char *txt, int column);

// Fast, buffered textfile reading

XFILE * xopen(char const *);
void    xclose(XFILE *);
char  * xgetline(XFILE *);

// Safe memory allocations, exit on failure

//void * xmalloc(unsigned size);
//void * xcalloc(unsigned n, unsigned size);
//char * xstrdup(char *s);
//void   dumpmem(void);

#ifndef __OS2__
#ifdef __WATCOMC__
dword coreleft(void);
#endif
#endif

// JAM lastread routines from MSGAPI

int    JAMmbStoreLastRead(MSG *sq, dword last, dword highest, dword CRC);
dword  JAMmbFetchLastRead(MSG *sq, dword UserCRC, int getlast);

int    write_echolog(void);

// Compare for list sorting routine

int    comp_flist(const void *one, const void *two);

// Some prototypes for the dirlist stuff.

char **     dirlist(char *filespec, char tagging);
int         showfiles(FILELIST *list, char tagging);
void        free_flist(FILELIST * list);
char **     build_filelist(FILELIST *list, char *path);
void        free_filelist(char **filelist);


// Raw character block functions

RAWBLOCK * InitRawblock(unsigned startlen, unsigned delta, unsigned maxlen);
void       AddToBlock (RAWBLOCK *blk, char *s, unsigned len);
char       GetLastChar(RAWBLOCK *blk);
void       SetLastChar(RAWBLOCK *blk, char c);
void       StripLastChar(RAWBLOCK *blk);
RAWBLOCK * JoinLastBlocks(RAWBLOCK *first, int minsize);
void       FreeBlocks(RAWBLOCK *blk);

// Line wrapping..

LINE *wraptext(char *txt, int llen, int end, int savelast);
void FreeLines(LINE *line);


MARKLIST * InitMarkList(void);
void       DumpMarkList(MARKLIST *lst);
void       AddMarkList(MARKLIST * lst, dword no);
void       RemoveMarkList(MARKLIST *lst, dword no);
int        IsMarked(MARKLIST *lst, dword no);
dword      NextMarked(MARKLIST *lst, dword start);
dword      PrevMarked(MARKLIST *lst, dword start);

// Function concerning tagged areas

void SaveTags(void);
void LoadTags(void);
int  ReadTagFile(int choice);

// Show extended error information from msgbase code..

void showerror(void);

// Clock stuff

void clockon     (void);
void clockoff    (void);
void update_clock(int forced);
#ifdef __OS2__
void killclock(void);
#endif

void check_mtask(void);
void give_slice(void);

// From checkcc:

AREA * FindArea(char *tag);

int PrinterReady(char *printer);

void MarkReplyChain(AREA *area, MSG *areahandle, dword startmsg);

ATTACHLIST * MakeAttachList (STRINGLIST *slist);
void FreeAttachList (ATTACHLIST *l);

#pragma aux ins09  "_*" parm caller [] modify [ax bx cx dx es]
#pragma aux undo09 "_*" parm caller [] modify [ax bx cx dx es]

extern void ins09(void);
extern void undo09(void);

int ReadKeyFile(void);

// Character translation

char * TranslateBlocks (char *destset, RAWBLOCK *blk, char *kludges, MIS *mis);
int    LoadCharMap     (char *name, sword level, int readmap);
char * TranslateChars  (char *s, unsigned *len, sword level);
sword  CheckForCharset (char *kludges);
void   TranslateHeader (MIS *mis, sword level);
void   PickCharMap     (void);


// Filtering messages

void FilterMessage(MMSG *curmsg, AREA *area, MSG *areahandle, int realbody);
void FilterMemory(MMSG *curmsg, int realbody);
RAWBLOCK *ReadBodyFromDisk(char *filename);

// Prototypes for mem_ functions:

void * mem_malloc(unsigned n);
void * mem_calloc(unsigned n, unsigned t);
void * mem_realloc(void * org, unsigned n);
char * mem_strdup(char *s);
void mem_free(void *p);

void RemoveTOline(RAWBLOCK *rawbody, MIS *mis);

// private.c

int   MaySee(MSG *areahandle, dword no);
int   ScanMaySee(AREA *area, MSG *areahandle);
int   BePrivate(AREA * area);
int   CurrentIsNotLast(AREA *area, MSG *areahandle);
dword GetNextPrivate(AREA *area, MSG *areahandle);






