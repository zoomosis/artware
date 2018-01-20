#ifndef __VIDEO_H__
#define __VIDEO_H__

/*

                Direct video writing, based on a C_ECHO
                snippet by jim nutt and parts of Filelist2
                by Erik Vanriper...

*/

#include <stdlib.h>

/* Prototypes... */

void cls(void);
void clsw(unsigned char colour);
void video_init(void);
void video_deinit(void);
int  setlines(char lines);
void print(int x, int y, int attr, unsigned char *line);
void biprint(int x, int y, int attr1, int attr2, unsigned char *line, char sep);
void biprinteol(int x, int y, int attr1, int attr2, unsigned char *line, char sep);
void vprint(int x, int y, int col, char *fmt, ...);
void vbiprint(int x, int y, int col1, int col2, char sep, char *fmt, ...);
void printn(int x, int y, int attr, unsigned char *line, int len);
void printeol(int x, int y, int attr, unsigned char *line);
void printeoln(int x, int y, int attr, unsigned char *line, int len);
void tabprint(int x, int y, int attr, unsigned char *line, int tabsize);
void printc(int x, int y, int attr, unsigned char token);
void ClsWith(int attr, unsigned char token);
void ClsRectWith(int x1, int y1, int x2, int y2, int attr, unsigned char token);
void MoveXY(int col, int row);
size_t HLstrlen(char *s);

#if defined(__DOS__) && defined(__FLAT__)
#include <graph.h>
#else
void _settextcursor(short type);
#endif

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

#ifdef __DOS__
/* DOS is memory confined, so let's not waste it, and besides, we probably can't ever get more than 132 columns in DOS anyway */
#define MAX_SCREEN_WIDTH 250
#else
#define MAX_SCREEN_WIDTH 1024
#endif

#endif
