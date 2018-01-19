#ifndef __OS2__
#pragma inline
#endif


#ifdef __TEST__
//  #define mem_malloc malloc
//  #define mem_calloc calloc
#endif

// Prototypes for mem_ functions:

void * mem_malloc(unsigned n);
void * mem_calloc(unsigned n, unsigned t);
void * mem_realloc(void * org, unsigned n);
char * mem_strdup(char *s);
void mem_free(void *p);

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "video.h"
#include "scrnutil.h"
//#include "memchk.h"
//#include "..\mshell.h"

#ifdef __OS2__
     #define INCL_SUB
     #include <os2.h>
//     #define far
//     #define near
#endif

#ifndef __OS2__

#ifndef __FLAT
extern unsigned int far *screen;
extern unsigned int vbase;
//#define mem_calloc calloc
//#define mem_malloc malloc
#else
extern unsigned short * screen;
extern unsigned short vbase;
#endif

#else
extern ULONG screen;
#endif

void drawrect(int x1, int y1, int x2, int y2, int style, int color);
void xgettext(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, char *s);
void xputtext(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, char *s);



#ifndef __OS2__
typedef struct _saved_screen
{

   char                 *thisscreen;
   struct _saved_screen *next;

} SCRNLIST;
#else
typedef struct _saved_screen
{

   USHORT               *thisscreen;
   struct _saved_screen *next;

} SCRNLIST;

#endif

#ifdef __NT__

#define WIN32_LEAN_AND_MEAN
   #include <windows.h>
   extern HANDLE ScreenHandle;
   USHORT VioWrtCellStr (PCH pchCellString, USHORT cbCellString, USHORT usRow, USHORT usColumn, USHORT hvio);

#endif

static SCRNLIST *scrnstack = NULL;



void drawbox(BOX *data)
{

/*   if(heapcheck() == -1)
     {
     cls();
     printf("Heap error!"); getch();
     }
*/

   if (data->save==YES)    /* Should the screen "underneath" be saved? */
      {
      /* ozzmosis 2018-01-18 hack fix, replace "sizeof(unsigned short)" with "4" */
      /* TODO: investigate further */
      data->blockptr=mem_malloc( (data->x2-data->x1+1) * (data->y2-data->y1+1) * 4);
      saveblock(data->y1, data->x1, data->y2, data->x2, data->blockptr);
      }


   /* Draw the box.... */

   drawrect(data->x1,
            data->y1,
            data->x2,
            data->y2,
            data->borderstyle,
            data->ColBorder);

   /* First we clear the screen w/ the given character... */


   if (data->fill != (char) -1)
      ClsRectWith(data->x1+1, data->y1+1, data->x2-1, data->y2-1,
                                             data->ColInside, data->fill);

/*      if(heapcheck() == -1)
     {
     cls();
     printf("Heap error!"); getch();
     }*/

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
   return(box);
}


void delbox(BOX *data)

{
   if (data->save==YES)
      {
      restoreblock(data->y1,
                   data->x1,
                   data->y2-data->y1+1,
                   data->x2-data->x1+1,
                   data->blockptr);

      free(data->blockptr);
      }

   free(data);

}


void boxwrite(BOX *box, int x, int y, char *line)

{
   print(box->x1 + 1 + x, box->y1 + 1 + y, box->ColInside, line);
}



void savescreen()

{
   SCRNLIST *scrptr, *temp = scrnstack;
   unsigned short len = maxx*maxy*2;

#ifdef __NT__
   /* brute force bug fix */
   /* TODO: investigate further */
   return;
   
   if (screen == NULL)
   {
	   /* bugfix 2018-01-18 ozzmosis */
	   /* TODO: why is screen == NULL in the NT build? */
	   return;
   }
#endif 
   
   scrptr = mem_calloc(1, sizeof(SCRNLIST));

   scrptr->thisscreen = mem_malloc(len);      /* Get the mem */
   scrptr->next = NULL;

   #ifndef __OS2__

   memcpy(scrptr->thisscreen,
          (char *)screen,
          len); /* Save it */

   #else

   /* OS2 */
   
   //   gettext(1,1,maxx,maxy,scrptr->thisscreen);
   VioReadCellStr((PBYTE) scrptr->thisscreen, &len, 0, 0, 0);

   #endif

   if(!scrnstack) scrnstack = scrptr;
   else
       {
       while(temp->next) temp=temp->next;     /* Walk to end of stack */
       temp->next = scrptr;                   /* Link this in */
       }
}


void putscreen()

{
   SCRNLIST *temp = scrnstack, *prev=NULL;

   if(temp)       /* Any stack already? */
      {
      while(temp->next)
         {
         prev = temp;
         temp = temp->next;
         }
      }
   else return;

   #ifndef __OS2__

   memcpy((char *)screen,
          temp->thisscreen,
          maxx*maxy*2); /* Restore it */

   #else
//   puttext(1,1,maxx,maxy,temp->thisscreen);
   VioWrtCellStr((PBYTE) temp->thisscreen, maxx*maxy*2, 0, 0, 0);
   #endif

   free(temp->thisscreen);
   free(temp);

   if(prev) prev->next = NULL;      /* If there was a previous screen, end linked list there */
   else scrnstack = NULL;           /* Else this was the first, so stack is empty */
}


/*------------------------------------------------------------------------------*/

void bios_scroll_up(int count, int sr, int sc, int er, int ec, int attr)
{
  #if !defined(__OS2__) && !defined(__NT__)
    union REGS regs;
    regs.h.al = count;
    regs.h.ch = sr;
    regs.h.cl = sc;
    regs.h.dh = er;
    regs.h.dl = ec;
    regs.h.bh = attr;
    regs.h.ah = 6;
  #ifndef __FLAT__
    int86(0x10,&regs,&regs);
  #else
    int386(0x10,&regs,&regs);
  #endif

  #else

     #ifdef __NT__

     // Do scrolling for NT here!

     #else

     USHORT cell = 32 | (attr<<8);
     VioScrollUp(sr, sc, er, ec, count, (PBYTE) &cell, 0);

     #endif

  #endif
}

/*------------------------------------------------------------------------------*/



void saveblock(int x1, int y1, int x2, int y2, char * b)

{

   #ifndef __WATCOMC__

   asm push ds

   asm mov ax,y1               /* y1                    */
   asm mov bx,maxx
   asm mul bl                  /* y1 * width of screen  */
   asm add ax,x1
   asm shl ax,1                /* x1 * 2                */
   asm shl bx,1                /* width * 2             */
   asm mov cx,x2
   asm sub cx,x1               /* x2 - x1               */
   asm mov dx,y2
   asm sub dx,y1               /* y2 - y1               */
   asm inc dx
   asm inc cx
   asm les di,b
   asm mov ds,vbase
   asm push cx

gr1:
   asm pop cx
   asm push cx
   asm mov si,ax
   asm rep movsw
   asm add ax,bx
   asm dec dx
   asm jnz gr1

   asm pop cx

   asm pop ds

   #else
       xgettext(x1+1,
                y1+1,
                x2+1,
                y2+1,
                b);
   #endif

}



void restoreblock(int x, int y, int w, int h, char * b)
{
   #ifndef __WATCOMC__
   asm push ds

   asm mov ax,y              /* y                                 */
   asm mov bx,maxx
   asm mul bl                /* y * width                         */
   asm add ax,x              /* + x = base of block (upper left)  */
   asm shl ax,1              /* * 2 = no of bytes (screen = ints) */
   asm shl bx,1              /* y * width in bytes (bytes 1 line) */

   asm mov es,vbase

   asm lds si,b              /* b --> point to memblock to put on screen */
   asm mov dx,w              /* w                                        */

pr1:
   asm mov cx,dx             /* width?                                   */
   asm mov di,ax             /* point to base of block on screen         */
   asm rep movsw             /* restore one line                         */
   asm add ax,bx             /* go one line down                         */
   asm dec h                 /* height? More lines to go?                */
   asm jnz pr1               /* yes, jump to pr1                         */

   asm pop ds

   #else
   xputtext(x+1,y+1,x+w,y+h,b);
   #endif
}


void drawrect(int x1, int y1, int x2, int y2, int style, int color)
{

   char  lines[7];
#ifndef __WATCOMC__
   int ofs = ( (x1 * maxx) + y1) * 2;    /* Start offset, upper left */
#endif
   char temp[133];
#ifndef __OS2__
   int l;
#endif

   #ifdef __OS2__
   USHORT  cell;
   #endif

   /* First we check out the borderstyle... */

   if (style==DOUBLE)
      strcpy(lines, "É»È¼Íº");
   else
      strcpy(lines, "Ú¿ÀÙÄ³");

   #ifndef __WATCOMC__

   asm push ds

   asm mov es,vbase
   asm mov di,ofs

   asm mov ax,color
   asm mov cl,8
   asm shl ax,cl        /* Put color byte in place == ah */

   asm mov al,lines[0]  /* Upper left Ú                  */

   asm cld
   asm stosw

   asm mov cx,y2
   asm sub cx,y1
   asm dec cx           /* Number of 'horizontal positions for box Ä  */
   asm mov dx,cx        /* Save it in DX..                            */
   asm shl dx,1         /* .. in number of bytes                      */
   asm mov al,lines[4]  /* Put 'Ä' char in..                          */
   asm rep stosw
   asm mov al,lines[1]  /* Draw a '¿'                                 */
   asm stosw

   asm mov cx,x2
   asm sub cx,x1        /* Number of lines to draw ('³')              */
   asm dec cx
   asm mov al,lines[5]  /* Put a '³' in al                            */

   asm mov bx,maxx
   asm dec bx           /* Adjust one pos down, for stosw inc's       */
   asm dec bx           /* And another one..                          */
   asm shl bx,1         /* Number of bytes on one line..              */
   asm sub bx,dx        /* .. minus width of box                      */

loop_vert:

   asm add di,bx        /* Down one line                              */
   asm stosw            /* Draw left vertical side                    */
   asm add di,dx        /* Move to right vertical side                */
   asm stosw            /* Draw it                                    */

   asm loop loop_vert   /* For enough lines..                         */

   asm add di,bx        /* Down one line                              */

   asm mov al,lines[2]  /* Put a 'À' in..                             */
   asm stosw
   asm mov al,lines[4]  /* And a 'Ä'                                  */
   asm mov cx,dx        /* Width of box                               */
   asm shr cx,1         /* Make it number of words again              */
   asm rep stosw
   asm mov al,lines[3]  /* Put a 'Ù' in                               */
   asm stosw

   asm pop ds

   #else

/*       strcpy(lines, "Ö·Ó½Äº"); */

       memset(temp, lines[4], sizeof(temp));
       temp[y2-y1+1] = '\0';
       temp[y2-y1] = lines[1];
       temp[0] = lines[0];

       print(x1,y1,color,temp);

       temp[y2-y1] = lines[3];
       temp[0] = lines[2];
       print(x2,y1,color,temp);


  #ifndef __OS2__

       for(l=1; l<(x2-x1) ; l++)
         {
         printc(x1+l, y1, color, lines[5]);
         printc(x1+l, y2, color, lines[5]);
         }

   #else

   cell = lines[5] | (color<<8);

   VioScrollDn(x1+1, y1, x2-1, y1, x2-x1-1, (PBYTE)&cell, 0);
   VioScrollDn(x1+1, y2, x2-1, y2, x2-x1-1, (PBYTE)&cell, 0);

   #endif

   #endif
}


void titlewin(BOX *win, int where, char *s, int hlcol)
{
   char fmt[6];
   char x;

   if(hlcol == 0) hlcol = win->ColBorder;

   if(win->borderstyle == SINGLE)
      strcpy(fmt, "´%sÃ");
   else
      strcpy(fmt, "¹%sÌ");

   switch(where)
     {
     case TLEFT:
        x = win->y1+1;
        break;
     case TRIGHT:
        x = win->y2-2-HLstrlen(s);
        break;
     case TCENTER:
        x = win->y1 + ((win->y2 - win->y1 - HLstrlen(s))/2);
        break;
     }

   vbiprint(win->x1, x, win->ColBorder, hlcol, '~', fmt, s);

}

#ifdef __WATCOMC__

void xputtext(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, char *s)
{
   #ifdef __NT__
   COORD coordsize;
   COORD coordloc;
   SMALL_RECT writerect;
   BOOL result;
   DWORD errorcode;

   coordsize.X = x2-x1+1;
   coordsize.Y = y2-y1+1;
   coordloc.X  = 0;
   coordloc.Y  = 0;
   writerect.Top    = y1 - 1;
   writerect.Bottom = y2 - 1;
   writerect.Right  = x2 - 1;
   writerect.Left   = x1 - 1;

   result = WriteConsoleOutput(ScreenHandle, (CHAR_INFO *)s, coordsize, coordloc, &writerect);
   if(result == FALSE)
      errorcode = GetLastError();

   #else

   int i;
   int regellen = (x2-x1+1)*2;
//   unsigned int ofs;
//   short *out;
#ifndef __OS2__
   char * whereto;
#endif

   for(i=y1-1; i<y2; i++)
     {
     #if !defined(__OS2__)

       whereto = (char *)screen;
       whereto += ( (i*maxx*2) + ((x1 - 1)*2) );
       memcpy(whereto, s, regellen);

     #else
       VioWrtCellStr(s, regellen, i, x1-1, 0);
     #endif
     s += regellen;
     }

   #endif
}


void xgettext(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, char *s)
{
   #ifdef __NT__
   COORD coordsize;
   COORD coordloc;
   SMALL_RECT readrect;
   BOOL result;
   DWORD errorcode;


   coordsize.X = x2-x1+1;
   coordsize.Y = y2-y1+1;
   coordloc.X  = 0;
   coordloc.Y  = 0;
   readrect.Top    = y1 - 1;
   readrect.Bottom = y2 - 1;
   readrect.Right  = x2 - 1;
   readrect.Left   = x1 - 1;
   result = ReadConsoleOutput(ScreenHandle, (CHAR_INFO *)s, coordsize, coordloc, &readrect);

   if(result == FALSE)
      errorcode = GetLastError();

   #else
   int i;
#ifdef __OS2__
   USHORT regellen;
#else
   int regellen;
#endif

#ifndef __OS2__
   char *fromwhere;
#endif

   regellen = (x2-x1+1)*2;

   for(i=y1-1; i<y2; i++)
     {
     #if !defined(__OS2__) && !defined(__NT__)
       fromwhere = (char *)screen;
       fromwhere += ( (i*maxx*2) + ((x1 - 1)*2) );
       memcpy(s, fromwhere, regellen);
     #else
       VioReadCellStr(s, &regellen, i, x1-1, 0);
     #endif

     s += regellen;
     }
   #endif
}

#endif
