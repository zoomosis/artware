#ifndef __TPROTOS_H__
#define __TPROTOS_H__

int check_attach(MIS * mis, char *mask, int copyfiles);
void writefa(MIS * mis, MSGA * areahandle, AREA * area, int exceptfirst);

int AttemptLock(MSGA * areahandle);

void check_cc(AREA * area, MSGA * areahandle, MIS * mis, RAWBLOCK ** first);
int choose_name(void);
int choose_address(void);
void clean_index(AREA * area);
void readconfig(void);
void CopyMsg(MSGA * areahandle, MSGA * toareahandle, AREA * toarea,
             dword msgno, char kill);
void CopyMarked(AREA * area, AREA * toarea, MSGA * areahandle,
                MSGA * toareahandle, int kill);
void DeleteMarked(AREA * area, MSGA * areahandle);

void MoveMessage(MSGA * areahandle, AREA * area, MMSG * curmsg);
dword clean_end_message(char *msgbuf);
int clean_origin(char *msgbuf, char **startorig);
void invalidate_origin(char *msg);

int get_quote_string(char *line, char *quote);

// Nodelist searching stuff

ADDRLIST *fido_lookup(char *name);
ADDRLIST *fido_nodelist_lookup(char *name);
ADDRLIST *NodeLookup(char *name, int sysop);


void FindMessage(MSGA * areahandle, AREA * area, long last);

RAWBLOCK *GetBody(AREA * area, MSGA * areahandle, MMSG * curmsg,
                  char *origline, LINE * firstline, int checkchange,
                  int startline);
int WriteRawBody(RAWBLOCK * blk, char *curfile);

LINE *edittext(LINE * begin, AREA * area, MSGA * areahandle, MMSG * curmsg,
               int curtxtline);
int ShowEditMenu(MMSG * curmsg, int escallowed);


MMSG *GetMsg(long curno, MSGA * areahandle, int doformat, AREA * area);
MMSG *GetFmtMsg(long curno, MSGA * areahandle, AREA * area);
int MarkReceived(MSGA * areahandle, AREA * area, dword no, int not,
                 int hasCFM);
MMSG *GetRawMsg(long curno, MSGA * areahandle, int what, int convert);
void checklines(LINE * first);

int MakeHeader(MSGA * areahandle, MMSG * curmsg, int reply, AREA * area,
               UMSGID reply_to_id, char *subject, MMSG * newmmsg);
int EditHeader(MSGA * areahandle, MMSG * curmsg, int address, int aka,
               int domatch, AREA * area);
int SetAttributes(MMSG * curmsg, word base, int fullscreen);

void statusbar(char *s);
int matchaka(MIS * mis);
void show_help(int cat);
int addrcmp(NETADDR * one, NETADDR * two);

int getstring(int row, int col, char *answer, int len, int maxlen,
              char *legalset, word colour, word exitcol);
dword MsgList(MSGA * areahandle, AREA * area, dword curno, char wide);

int maint_menu(AREA * area);
void Message(char *text, int pause, int stop, YesNo save);
int confirm(char *text);


int check_node(MIS * mis, int aka, int prompt);

/* Should timEd ask confirmation if only 1 address found? */

int pickone(char **choices, int y1, int x1, int y2, int x2);
void free_picklist(char **choices);
//int picklist(char **choices, char **help, char *title, int y1, int x1, int y2, int x2);
int picklist(char **choices, char **help, int y1, int x1, int y2, int x2);


void PrintMessage(MMSG * curmsg, AREA * area, MSGA * areahandle,
                  int hardcopy, int tagged, int what);
FILE *OpenExist(char *default_name, AREA * area, MSGA * areahandle,
                MMSG * curmsg);
void do_print(FILE * outfile, AREA * area, MMSG * curmsg, int what);

void inst_newcrit(void);
void inst_oldcrit(void);

int ReadArea(AREA * area);


long GetLast(AREA * area, MSGA * areahandle, int raw);
void ReleaseMsg(MMSG * thismsg, int allofit);
void UpdateLastread(AREA * area, long last, dword highest,
                    MSGA * areahandle);
long MsgGetLowMsg(MSGA * areahandle);
void showmem(void);
void ScanArea(AREA * area, MSGA * areahandle, int raw);
dword anchor(int direction, MSGA * areahandle);
void check_registration(void);
int read_key(void);
void edit_hello_strings(AREA * area);
void get_custom_info(AREA * area);
long get_last_squish(AREA * area);

#ifdef __WATCOMC__
int beep(void);
#endif

char *MakeKludge(MMSG * curmsg, MIS * mis, int netmail);
void MakeMessage(MMSG * curmsg, AREA * area, MSGA * areahandle, word reply,
                 UMSGID reply_to_id, char *add_to_top);
void ChangeMessage(MMSG * curmsg, AREA * area, MSGA * areahandle,
                   int bodytoo);
#ifndef __WATCOMC__
int pascal IsQuote(char *line);
#else
int IsQuote(char *line);
#endif
void MakeText(MMSG * curmsg, FILE * parameterfile);
void ReplyOther(MMSG * curmsg, AREA * area);
int check_alias(MIS * mis);
char *make_origin(int aka);
void address_expand(char *line, NETADDR * fa, int aka);
void zonegate(MIS * mis, char *kludges, int showinfo, dword base);
//void check_direct (MIS *mis, char *kludges);
int SaveMessage(MSGA * areahandle, AREA * area, MIS * mis, RAWBLOCK * blk,
                char *kludges, dword no, int preserve);
char *expand(char *line, MIS * mis, MIS * newmis);


int get_request(MMSG * curmsg, AREA * area, MSGA * areahandle);

AREA *SelectArea(AREA * first, int pickonly, AREA * area_to_start);
int AreaVisible(AREA * curptr);
int fastscan(AREA * curarea, int personal);

int shell_to_DOS(void);
int runexternal(AREA * area, MSGA ** areahandle, MMSG * curmsg,
                long lastorhigh, dword highest);
char *BuildCommandLine(char *charptr, AREA * area, MSGA * areahandle,
                       MMSG * curmsg, char *curfile, char *newfile);
int runaprog(char *prog, char *parms, int waitforkey);
int editrunexternal(AREA * area, MSGA * areahandle, MMSG * curmsg);
int FileDelete(void);

void paint_header(MMSG * curmsg, word type);
char *attr_to_txt(dword attr1, dword attr2);
char *FormAddress(NETADDR * address);
char *MakeT(dword t, int type);
void SumAttachesRequests(MIS * mis, char *temp, int maxlen, int what);

void showinfo(MMSG * curmsg, AREA * area, MSGA * areahandle);

dword ShowMsg(MMSG * msg, AREA * area, MSGA * areahandle, int displaytype);
dword showbody(MMSG * curmsg, MSGA * areahandle, AREA * area,
               int displaytype);

char *MakeRep(MMSG * msg, MSGA * areahandle, AREA * area);

void add_tosslog(AREA * area, MSGA * areahandle);

void unreceive(MMSG * curmsg, MSGA * areahandle, AREA * area);

void working(int y, int x, int col);

LINE *FormatText(char *txt, int ll);
LINE *fastFormatText(char *txt, int column);

// Safe memory allocations, exit on failure

//void * xmalloc(unsigned size);
//void * xcalloc(unsigned n, unsigned size);
//char * xstrdup(char *s);
//void   dumpmem(void);

#if defined(__WATCOMC__) && !defined(__OS2__) && !defined(__NT__)
dword coreleft(void);
#endif

// JAM lastread routines from MSGAPI

int JAMmbStoreLastRead(MSGA * sq, dword last, dword highest, dword CRC);
dword JAMmbFetchLastRead(MSGA * sq, dword UserCRC, int getlast);

int write_echolog(void);

// Compare for list sorting routine

int comp_flist(const void *one, const void *two);

// Some prototypes for the dirlist stuff.

char **dirlist(char *filespec, char tagging);
int showfiles(FILELIST * list, char tagging);
void free_flist(FILELIST * list);
char **build_filelist(FILELIST * list, char *path);
void free_filelist(char **filelist);


// Raw character block functions

RAWBLOCK *InitRawblock(unsigned startlen, unsigned delta, unsigned maxlen);
void AddToBlock(RAWBLOCK * blk, char *s, unsigned len);
char GetLastChar(RAWBLOCK * blk);
void SetLastChar(RAWBLOCK * blk, char c);
void StripLastChar(RAWBLOCK * blk);
RAWBLOCK *JoinLastBlocks(RAWBLOCK * first, int minsize);
void FreeBlocks(RAWBLOCK * blk);

// Line wrapping..

LINE *wraptext(char *txt, int llen, int end, int savelast);
void FreeLines(LINE * line);


MARKLIST *InitMarkList(void);
void DumpMarkList(MARKLIST * lst);
void AddMarkList(MARKLIST * lst, dword no);
void RemoveMarkList(MARKLIST * lst, dword no);
int IsMarked(MARKLIST * lst, dword no);
dword NextMarked(MARKLIST * lst, dword start);
dword PrevMarked(MARKLIST * lst, dword start);

// Function concerning tagged areas

void SaveTags(void);
void LoadTags(void);
int ReadTagFile(int choice);

// Show extended error information from msgbase code..

void showerror(void);
void debug(const char *format, ...);

// Clock stuff

void clockon(void);
void clockoff(void);
void update_clock(int forced);
#ifdef __OS2__
void killclock(void);
#endif

void check_mtask(void);
void give_slice(void);

// From checkcc:

AREA *FindArea(char *tag);

int PrinterReady(char *printer);

void MarkReplyChain(AREA * area, MSGA * areahandle, dword startmsg);

ATTACHLIST *MakeAttachList(STRINGLIST * slist);
void FreeAttachList(ATTACHLIST * l);

#if 1
#ifdef __WATCOMC__
#pragma aux ins09  "_*" parm caller [] modify [ax bx cx dx es]
#pragma aux undo09 "_*" parm caller [] modify [ax bx cx dx es]
#endif
#endif

extern void ins09(void);
extern void undo09(void);

int ReadKeyFile(void);

// Character translation

char *TranslateBlocks(char *destset, RAWBLOCK * blk, char *kludges,
                      MIS * mis);
int LoadCharMap(char *name, sword level, int readmap);
char *TranslateChars(char *s, unsigned *len, sword level);
sword CheckForCharset(char *kludges);
void TranslateHeader(MIS * mis, sword level);
void PickCharMap(void);


// Filtering messages

void FilterMessage(MMSG * curmsg, AREA * area, MSGA * areahandle,
                   int realbody);
void FilterMemory(MMSG * curmsg, int realbody);
RAWBLOCK *ReadBodyFromDisk(char *filename);

// Prototypes for mem_ functions:

void *mem_malloc(unsigned n);
void *mem_calloc(unsigned n, unsigned t);
void *mem_realloc(void *org, unsigned n);
char *mem_strdup(char *s);
void mem_free(void *p);

void RemoveTOline(RAWBLOCK * rawbody, MIS * mis);

// private.c

int MaySee(MSGA * areahandle, dword no);
int ScanMaySee(AREA * area, MSGA * areahandle);
int BePrivate(AREA * area);
int CurrentIsNotLast(AREA * area, MSGA * areahandle);
dword GetPrevPrivate(AREA * area, MSGA * areahandle);
dword GetNextPrivate(AREA * area, MSGA * areahandle);

/* find.c */

void bottom(char *s);

/* idlekey.c */

int get_idle_key(char allowstuff, int scope);
void stuffkey(int key);
void kbflush(void);
void check_enhanced(void);
int xkbhit(void);
void MacroStart(sword i);

/* edit.c */

void WriteToFile(int raw, int block, AREA * area, MSGA * areahandle,
                 MMSG * curmsg);

/* version7.c */

char *fancy_str(char *string);

/* config.c */

char *StripCR(char *str);

#endif
