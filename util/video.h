/*

                Direct video writing, based on a C_ECHO
                snippet by jim nutt and parts of Filelist2
                by Erik Vanriper...

*/

/* Prototypes... */
#ifdef __OS2__

#define near
#define far

#endif


void cls(void);
void clsw(unsigned char colour);
void video_init(void);
void print(int x, int y, int attr, unsigned char far *line);
void printn(int x, int y, int attr, unsigned char far *line, int len);
void printeol(int x, int y, int attr, unsigned char far *line);
void printeoln(int x, int y, int attr, unsigned char far *line, int len);
void tabprint(int x, int y, int attr, unsigned char far *line, int tabsize);
void printc(int x, int y, int attr, unsigned char token);
void ClsWith(int attr, unsigned char token);
void ClsRectWith(int x1, int y1, int x2, int y2, int attr, unsigned char token);
void MoveXY(int col, int row);

/* Some color defines.. */
/*
#define B_BLACK      0
#define B_BLUE       16
#define B_GREEN      32
#define B_CYAN       48
#define B_RED        64
#define B_MAGENTA    80
#define B_BROWN      96
#define B_GRAY       112
#define F_BLACK      0
#define F_BLUE       1
#define F_GREEN      2
#define F_CYAN       3
#define F_RED        4
#define F_MAGENTA    5
#define F_BROWN      6
#define F_GRAY       7
#define F_LBLACK     8
#define F_LBLUE      9
#define F_LGREEN     10
#define F_LCYAN      11
#define F_LRED       12
#define F_LMAGENTA   13
#define F_YELLOW     14
#define F_WHITE      15
*/

extern int maxx;
extern int maxy;
