#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __NetBSD__
#include <curses.h>
#else
#include <ncurses.h>
#endif

#include "video.h"

#define word unsigned int
#define dword unsigned long
#define sword int

#define F_NORMAL    0x000  /* Normal Font */
#define F_ALTERNATE 0x100  /* Alternate Font */

static char ansi2curses[8] =
{
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE
};

/* This maps PC colors to monochrome attributes for w/o color support */

static int mono_colors[128] =
{
    A_NORMAL, A_NORMAL, A_NORMAL, A_NORMAL,
    A_NORMAL, A_NORMAL, A_NORMAL, A_NORMAL,
    A_BOLD, A_BOLD, A_BOLD, A_BOLD,
    A_BOLD, A_BOLD, A_BOLD, A_BOLD,

    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,

    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,

    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,

    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,

    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,

    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,

    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_REVERSE, A_REVERSE, A_REVERSE, A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
        A_BOLD | A_REVERSE,
};

#ifdef STATIC_WIN
unsigned short maxx;            /* number of columns */
unsigned short maxy;            /* number of rows */
#else
#define maxx getmaxx(stdscr)
#define maxy getmaxy(stdscr)
#endif

static int posx, posy;
static int color;

/* ------------------------------------ */

#if 0
/* unused */
static char ch_ULCORNER[] = { 0xd6, 0xc9, 0xd5, 0xda, 0 };
#endif

static char ch_VLINE[] = { 0xba, 0xb3, 0 };

static chtype char2chtype(char ch)
{
    if (strchr(ch_VLINE, ch) != NULL)
    {
        return ACS_VLINE;
    }
    
    switch (ch)
    {
    case 0xd6:
    case 0xc9:
    case 0xd5:
    case 0xda:
        return ACS_ULCORNER;

    case 0xb7:
    case 0xbb:
    case 0xb8:
    case 0xbf:
        return ACS_URCORNER;

    case 0xd3:
    case 0xc8:
    case 0xd4:
    case 0xc0:
        return ACS_LLCORNER;
    case 0xbd:
    case 0xbc:
    case 0xbe:
    case 0xd9:
        return ACS_LRCORNER;
    case 0xcd:
    case 0xc4:
        return ACS_HLINE;
    case 0xba:
    case 0xb3:
        return ACS_VLINE;
    case 0xdb:
        return ACS_BLOCK;
    case 0xc2:
        return ACS_TTEE;
    case 0xc1:
        return ACS_BTEE;
    case 0xc3:
        return ACS_LTEE;
    case 0xb4:
        return ACS_RTEE;
    case 0xb0:
        return ACS_CKBOARD;
    case 0xb1:
        return ACS_CKBOARD;
    case 0xfb:
        return ACS_DIAMOND;
    case 0xfe:
        return ACS_DIAMOND;
    default:
        return ch & 0xff;
    }
    
    return ch;
}

unsigned int mkcolor(unsigned int acolor)
{
    unsigned int attr = 0;

    if (!has_colors())
    {
        attr = mono_colors[acolor & 0x7F];
    }
    else
    {
#if 0
        /* this causes erroneous underlines in Linux for some reason */
        if (acolor & 0x08)
            attr |= A_DIM | A_BOLD;
#else
        if (acolor & 0x08)
            attr |= A_BOLD;
#endif
        if (acolor & 0x80)
            attr |= A_BLINK;
        attr |= COLOR_PAIR(((acolor & 0x07) | ((acolor & 0x70) >> 1)));
    }
/*
#ifdef A_ALTCHARSET
    if (acolor & F_ALTERNATE)
    {
        attr |= A_ALTCHARSET;
    }
#endif
*/
    return attr;
}


unsigned int setcolor(unsigned int newcolor)
{
    unsigned int attr = 0;

    if ((unsigned int) color == newcolor)
        return mkcolor(color);

    color = newcolor;

    attr = mkcolor(newcolor);

    attrset(attr);
    bkgdset(attr & (~A_ALTCHARSET));

    return attr;
}

void video_escdelay(int ms)
{
#ifndef __NetBSD__
    ESCDELAY = ms;
#endif
}

void video_init(void)
{
    atexit(video_deinit);

    initscr();

#ifdef STATIC_WIN
    maxx = getmaxx(stdscr);
    maxy = getmaxy(stdscr);
#endif

    if (has_colors())
    {
        int i = 0;

        start_color();

        while (i < COLOR_PAIRS)
        {
            init_pair(i, ansi2curses[i & 0x07], ansi2curses[(i & 0x38) >> 3]);
            i++;
        }
    }

    nonl();
    noecho();
    cbreak();
    nodelay(stdscr, FALSE);
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    intrflush(stdscr, FALSE);
    raw();
    color = 0;
    posx = posy = 0;
    video_escdelay(50);
    cls();
}

void video_deinit(void)
{
    endwin();
}

static void slamlenbiline(int x, int y, char *line, chtype attr1, chtype attr2, char sep)
{
    chtype ch;

    attr1 = mkcolor(attr1);
    attr2 = mkcolor(attr2);

    move(x, y);

    while (*line)
    {
        if (*line == sep)
        {
            ch = attr1;
            attr1 = attr2;
            attr2 = ch;
        }
        else
        {
            addch(attr1 | char2chtype(*line));
            y++;
        }

        line++;
    }

    setcolor(attr1);
    move(x, y);
}

void biprint(int x, int y, int attr1, int attr2, unsigned char *line, char sep)
{
    slamlenbiline(x, y, line, attr1, attr2, sep);
    move(posx, posy);
}

void biprinteol(int x, int y, int attr1, int attr2, unsigned char *line, char sep)
{
    slamlenbiline(x, y, line, attr1, attr2, sep);
    clrtoeol();
    move(posx, posy);
}

/* Now a function to write a string directly to the screen */

void print(int x, int y, int attr, unsigned char *line)
{
    chtype color = mkcolor(attr);

    while (*line)
    {
        mvaddch(x, y, color | char2chtype(*line));
        line++;
        y++;
    }

    move(posx, posy);
    refresh();
}

/* Print function that takes a variable number of args like printf  */

void vprint(int x, int y, int attr, char *fmt, ...)
{
    va_list argptr;
    char temp[1024];
    int howmuch;

    va_start(argptr, fmt);
    howmuch = vsprintf(temp, fmt, argptr);
    va_end(argptr);

    printn(x, y, attr, temp, howmuch);
    refresh();
}

/* Print function that takes a variable number of args like printf  */

void vbiprint(int x, int y, int attr1, int attr2, char sep, char *fmt, ...)
{
    va_list argptr;
    char temp[1024];
    int howmuch;

    va_start(argptr, fmt);
    howmuch = vsprintf(temp, fmt, argptr);
    va_end(argptr);

    biprint(x, y, attr1, attr2, temp, sep);
    refresh();
}

/* Now a function to write a string directly to the screen */

void printn(int x, int y, int attr, unsigned char *line, int len)
{
    chtype color = mkcolor(attr);

    while (len--)
    {
        mvaddch(x, y++, color | char2chtype(*line++));
    }

    move(posx, posy);
    refresh();
}

void printeol(int x, int y, int attr, unsigned char *line)
{
    chtype color = mkcolor(attr);

    while (*line)
    {
        mvaddch(x, y++, color | char2chtype(*line++));
    }

    move(x, y);
    setcolor(attr);
    clrtoeol();
    move(posx, posy);
    refresh();
}

void printeoln(int x, int y, int attr, unsigned char *line, int len)
{
    chtype color = mkcolor(attr);

    while (len--)
    {
        mvaddch(x, y++, color | char2chtype(*line++));
    }

    move(x, y);
    setcolor(attr);
    clrtoeol();
    move(posx, posy);
    refresh();
}

/* A function to write a character directly to the screen */

void printc(int x, int y, int attr, unsigned char token)
{
    chtype ch;
    ch = char2chtype(token) | mkcolor(attr);
    mvaddch(x, y, ch);
    move(posx, posy);
    refresh();
}

void cls(void)
{
    setcolor(7);
    clear();
    move(0, 0);
    posx = 0;
    posy = 0;
    refresh();
}

void clsw(unsigned char colour)
{
    setcolor(colour);
    clear();
    move(posx, posy);
    refresh();
}

void ClsRectWith(int y1, int x1, int y2, int x2, int attr, unsigned char token)
{
    int x, y;

    chtype tk = ((chtype) mkcolor(attr)) | char2chtype(token);

    move(0, 0);
    refresh();

    for (y = y1; y <= y2; y++)
    {
        mvaddch(y, x1, tk);

        for (x = x1 + 1; x <= x2; x++)
        {
            addch(tk);
        }
    }

    move(posx, posy);
    refresh();
}

void MoveXY(int col, int row)
{
    posy = col - 1;
    posx = row - 1;
    move(row - 1, col - 1);
    refresh();
}

int setlines(char lines)
{
    return (int) lines;
}

/* Calculate length of string, exluding ~ characters (for highlighting) */

size_t HLstrlen(char *s)
{
    size_t i = 0;

    if (s == NULL)
    {
	return 0;
    }

    while (*s)
    {
        if (*s != '~')
        {
            i++;
        }

        s++;
    }

    return i;
}
