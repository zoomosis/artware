void *mem_malloc(unsigned n);
void *mem_calloc(unsigned n, unsigned t);
void *mem_realloc(void *org, unsigned n);
char *mem_strdup(char *s);
void mem_free(void *p);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __NetBSD__
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "video.h"
#include "scrnutil.h"
#include "unused.h"

void drawrect(int x1, int y1, int x2, int y2, int style, int color);
void xgettext(unsigned short x1, unsigned short y1, unsigned short x2,
              unsigned short y2, char *s);
void xputtext(unsigned short x1, unsigned short y1, unsigned short x2,
              unsigned short y2, char *s);
unsigned int mkcolor(unsigned int acolor);

typedef struct _saved_screen
{
    WINDOW *thisscreen;
    struct _saved_screen *next;
}
SCRNLIST;

static SCRNLIST *scrnstack = NULL;

void drawbox(BOX * data)
{
    /* Should the screen "underneath" be saved? */

    if (data->save == YES)      
    {
        data->blockptr =
            mem_malloc((data->x2 - data->x1 + 1) * (data->y2 - data->y1 +
                                                    1) * sizeof(chtype));
        saveblock(data->y1, data->x1, data->y2, data->x2, data->blockptr);
    }

    drawrect(data->x1,
             data->y1,
             data->x2, data->y2, data->borderstyle, data->ColBorder);


    if (data->fill != (char)-1)
        ClsRectWith(data->x1 + 1, data->y1 + 1, data->x2 - 1, data->y2 - 1,
                    data->ColInside, data->fill);

}


BOX *initbox(int x1, int y1, int x2, int y2,
             int border, int inside, int bstyle, YesNo save, char fill)
{
    BOX *box;

    box = mem_malloc(sizeof(BOX));

    box->x1 = x1;
    box->y1 = y1;
    box->x2 = x2;
    box->y2 = y2;
    box->ColBorder = border;
    box->ColInside = inside;
    box->save = save;
    box->fill = fill;
    box->borderstyle = bstyle;
    return (box);
}


void delbox(BOX * data)
{
    if (data->save == YES)
    {
        restoreblock(data->y1,
                     data->x1,
                     data->y2 - data->y1 + 1,
                     data->x2 - data->x1 + 1, data->blockptr);

        free(data->blockptr);
    }

    free(data);

}


void boxwrite(BOX * box, int x, int y, char *line)
{
    print(box->x1 + 1 + x, box->y1 + 1 + y, box->ColInside, line);
}



void savescreen(void)
{
    SCRNLIST *scrptr, *temp = scrnstack;

    scrptr = mem_calloc(1, sizeof(SCRNLIST));
    scrptr->next = NULL;

    scrptr->thisscreen = dupwin(stdscr);

    if (!scrnstack)
        scrnstack = scrptr;
    else
    {
        while (temp->next)
            temp = temp->next;  /* Walk to end of stack */
        temp->next = scrptr;    /* Link this in */
    }
}


void putscreen(void)
{
    SCRNLIST *temp = scrnstack, *prev = NULL;

    if (temp)                   /* Any stack already? */
    {
        while (temp->next)
        {
            prev = temp;
            temp = temp->next;
        }
    }
    else
        return;


    overwrite(temp->thisscreen, stdscr);

    delwin(temp->thisscreen);
    free(temp);

    if (prev)
        prev->next = NULL;      /* If there was a previous screen, end
                                   linked list there */
    else
        scrnstack = NULL;       /* Else this was the first, so stack is
                                   empty */
}


/*------------------------------------------------------------------------------*/

void bios_scroll_up(int count, int sr, int sc, int er, int ec, int attr)
{
    WINDOW *sub;

	unused(attr);
    sub = subwin(stdscr, er - sr, ec - sc, er, sr);
    idlok(sub, TRUE);
    scrollok(sub, TRUE);
    wscrl(sub, count);
    touchwin(stdscr);
    wmove(sub, 0, ec - sc);
    wclrtoeol(sub);
    // move(posx,posy);
    touchwin(stdscr);
    wrefresh(sub);
    delwin(sub);
}

/*------------------------------------------------------------------------------*/


void saveblock(int y1, int x1, int y2, int x2, char * b)
{
    int x, y;

    for (x = x1; x <= x2; x++)
        for (y = y1; y <= y2; y++)
        {
            *b++ = mvinch(x, y);
        }
}



void restoreblock(int y1, int x1, int y2, int x2, char * b)
{
    int x, y;

    for (x = 0; x < x2; x++)
        for (y = 0; y < y2; y++)
        {
            mvaddch(x1 + x, y1 + y, *b++);
        }

    refresh();
}


void drawrect(int x1, int y1, int x2, int y2, int style, int color)
{
    chtype attr;
    int i;
	
	unused(style);

#if 0
    int ofs = ((x1 * maxx) + y1) * 2; /* Start offset, upper left */
#endif

    /* First we check out the borderstyle... */
/*
   if (style==SINGLE)
      strcpy(lines, "ڿ��ĳ");

   else if (style==S_VERT)
      strcpy(lines, "ոԾͳ");

   else if (style==DOUBLE)
      strcpy(lines, "ɻȼͺ");

   else if (style==S_HOR)
      strcpy(lines, "ַӽĺ");
*/
    attr = mkcolor(color);

    mvaddch(x1, y1, attr | ACS_ULCORNER);
    for (i = y1 + 1; i < y2; i++)
        addch(attr | ACS_HLINE);
    addch(attr | ACS_URCORNER);

    mvaddch(x2, y1, attr | ACS_LLCORNER);
    for (i = y1 + 1; i < y2; i++)
        addch(attr | ACS_HLINE);
    addch(attr | ACS_LRCORNER);

    for (x1++; x1 < x2; x1++)
    {
        mvaddch(x1, y1, attr | ACS_VLINE);
        mvaddch(x1, y2, attr | ACS_VLINE);
    }

    refresh();
}


void titlewin(BOX * win, int where, char *s, int hlcol)
{
    char fmt[6];
    char x;

    if (hlcol == 0)
    {
        hlcol = win->ColBorder;
    }

    strcpy(fmt, "%s");

    switch (where)
    {
    case TLEFT:
        x = win->y1 + 1;
        break;

    case TRIGHT:
        x = win->y2 - 2 - HLstrlen(s);
        break;

    case TCENTER:
        x = win->y1 + ((win->y2 - win->y1 - HLstrlen(s)) / 2);
        break;
    }

    if (s != NULL)
    {
        vbiprint(win->x1, x, win->ColBorder, hlcol, '~', fmt, s);
    }
    
}
