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
int setlines(char lines);
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
void ClsRectWith(int x1, int y1, int x2, int y2, int attr,
                 unsigned char token);
void MoveXY(int col, int row);
size_t HLstrlen(char *s);

#if defined(__DOS__) && defined(__FLAT__)
#include <graph.h>
#else
void _settextcursor(short type);
#endif

#ifndef _NOCURSOR
#define _NOCURSOR     0
#endif

#ifndef _NORMALCURSOR
#define _NORMALCURSOR 1
#endif

#ifndef _SOLIDCURSOR
#define _SOLIDCURSOR  1
#endif

extern int maxx;
extern int maxy;

#ifdef __DOS__
/* DOS is memory confined, so let's not waste it, and besides, we probably can't ever get more than 132 columns in DOS anyway */
#define MAX_SCREEN_WIDTH 250
#else
#define MAX_SCREEN_WIDTH 1024
#endif

#endif
