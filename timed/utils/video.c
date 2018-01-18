/* #pragma inline */

#include <dos.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#ifdef __OS2__

    #define INCL_SUB
    #define INCL_NOPMAPI
    #include <os2.h>
    #include <stddef.h>

#endif

#ifdef __NT__

#define WIN32_LEAN_AND_MEAN

   #include <windows.h>
 
#endif

#ifdef __FLAT__

   #define word      unsigned short
   #define dword     unsigned int
   #define sword     short

#else

   #define word      unsigned int
   #define dword     unsigned long
   #define sword     int

#endif


#ifdef __OS2__

   void _settextcursor(word type);

#endif

#ifdef __NT__

   HANDLE ScreenHandle = ERROR_INVALID_HANDLE;
   USHORT VioWrtCellStr (PCH pchCellString, USHORT cbCellString, USHORT usRow, USHORT usColumn, USHORT hvio);
   void _settextcursor(word type);

#endif


#include "video.h"

void ClsWith(int attr, unsigned char token);
void ClsRectWith(int x1, int y1, int x2, int y2, int attr, unsigned char token);

#ifndef __OS2__

#ifndef __FLAT__
unsigned short * screen;    /* pointer to screen memory */
unsigned int vbase=0;      /* segment of video base */
unsigned int scrnsize;
#else
unsigned short *screen;    /* pointer to screen memory */
unsigned short vbase=0;      /* segment of video base */
unsigned short scrnsize;

#ifndef __NT__
   #include <graph.h>
#endif

#include <i86.h>

#endif
#else
ULONG screen;
USHORT scrnsize;
VIOMODEINFO origvio;
#endif

#ifdef __NT__

CONSOLE_SCREEN_BUFFER_INFO OrigScreenInfo;
USHORT scrnsize;

#endif

int maxx;       /* number of columns */
int maxy;       /* number of rows */
int desqview;

#ifndef __OS2__
 union REGS r;
 struct SREGS sr;
 sword vmode     =  0;
 sword origmode  = -1;
 sword origlines = -1;
 int mono        =  0;
#endif


/* ------------------------------------ */

void video_init(void)

{

/* get display mode */

// ===================== DOS Stuff ===========================
#ifndef __FLAT__

r.h.ah = 0x0f;
int86(0x10,&r,&r);

vmode = r.h.al;
if(origmode == -1)
   origmode = vmode;

/* guess what!?
   it returns the number of columns too! */

//if (maxx == 0)
        maxx = (int) r.h.ah;

/* number of rows is harder */

//if (maxy == 0)
//{
        /* try using a ega/vga function */

        r.x.ax = 0x1130;
        r.x.dx = 0;
        int86(0x10,&r,&r);

        /* if it fails, it returns 0, so we set
           the screen to 25 lines, otherwise
           it returns the actual number of lines
           of text on the screen */

        maxy = (r.x.dx == 0) ? 25 : (r.x.dx + 1);

        if(origlines == -1) origlines = maxy;
//}

if (vbase == 0)
   {
   /* if you are in mode 7 the video is
      most likely based at 0xB000, otherwise
      it will be at 0xB800.  This fails for
      a small number of cards, like the
      genius mono displays (although, i've
      never had a failure reported... */

   if (vmode == 0x07)
       {
       vbase = 0xb000;
       mono=1;
       }
   else    vbase = 0xb800;

   r.h.ah = 0xfe;
   sr.es = vbase;

   /* if we are running desqview or topview,
      this function returns the segment of a video
      buffer to use, otherwise it returns the
      value in es unchanged */

   r.x.di = 0;
   int86x(0x10,&r,&r,&sr);

   desqview = (vbase != sr.es);
   vbase = sr.es;

   /* make my screen pointer */

   screen = (unsigned short *) MK_FP(vbase,r.x.di);
   }

scrnsize = maxx * maxy;

// =========================== OS/2 stuff ========================
#elif defined(__OS2__)

  VIOMODEINFO vmi;

  vmi.cb = sizeof(VIOMODEINFO);
  VioGetMode(&vmi, 0);
  if(origvio.cb == 0)
     {
     memcpy(&origvio, &vmi, sizeof(VIOMODEINFO));
     origvio.cb = sizeof(VIOMODEINFO);
     }
  maxx = vmi.col;
  maxy = vmi.row;


// ============================= WIN NT/95 stuff ====================

#elif defined(__NT__)

  CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;

  if(ScreenHandle == ERROR_INVALID_HANDLE)
    {
    if( (ScreenHandle = GetStdHandle(STD_OUTPUT_HANDLE)) == ERROR_INVALID_HANDLE)
      return;
    }

  if(GetConsoleScreenBufferInfo(ScreenHandle, &ScreenInfo) == FALSE)
    return;

  memcpy(&OrigScreenInfo, &ScreenInfo, sizeof(CONSOLE_SCREEN_BUFFER_INFO));

  maxx = ScreenInfo.dwSize.X;
  maxy = ScreenInfo.dwSize.Y;
  scrnsize = maxx * maxy;

// ============================= DOS Extended stuff ====================

#else

r.h.ah = 0x0f;
int386(0x10,&r,&r);

vmode = r.h.al;
if(origmode == -1)
   origmode = vmode;


/* guess what!?
   it returns the number of columns too! */

//if (maxx == 0)
        maxx = (sword) r.h.ah;

/* number of rows is harder */

//if (maxy == 0)
//{
        /* try using a ega/vga function */

        r.w.ax = 0x1130;
        r.w.dx = maxy;
        int386(0x10,&r,&r);

        /* if it fails, it returns 0, so we set
           the screen to 25 lines, otherwise
           it returns the actual number of lines
           of text on the screen */

        maxy = (r.w.dx == 0) ? 25 : (r.w.dx + 1);
        if(origlines == -1) origlines = maxy;

//}

if (vbase == 0)
        {
        /* if you are in mode 7 the video is
           most likely based at 0xB000, otherwise
           it will be at 0xB800.  This fails for
           a small number of cards, like the
           genius mono displays (although, i've
           never had a failure reported... */

        if (vmode == 0x07)
           {
           vbase  = 0xb000;
           screen = (unsigned short *) ((0xb000) << 4);
           mono = 1;
           }
        else
           {
           vbase  = 0xb800;
           screen = (unsigned short *) ((0xb800) << 4);
           }

        memset(&sr, '\0', sizeof(sr));
        sr.es = sr.ds = sr.gs = sr.cs = FP_SEG(&sr);
        memset(&r, '\0', sizeof(r));

        r.h.ah = 0xfe;
        sr.es = FP_SEG(screen);

        /* if we are running desqview or topview,
           this function returns the segment of a video
           buffer to use, otherwise it returns the
           value in es unchanged */

        r.w.di = 0;
//        printf("screen: %p  -  sr.es: %p  -  sr.di: %p  \n", screen, sr.es, r.w.di);
//        printf("\Are you ready?\n"); getch();
        int386x(0x10,&r,&r,&sr);
//        printf("\Done!\n"); getch();
//        printf("screen: %p  -  sr.es: %p  -  sr.di: %p  \n", screen, sr.es, r.w.di); getch();

        desqview = (FP_SEG(screen) != sr.es);
        if(desqview)
           screen = MK_FP(vbase, r.w.di);

//        printf("screen: %p  -  sr.es: %p  -  sr.di: %p  \n", screen, sr.es, r.w.di); getch();
        }

scrnsize = maxx * maxy;

#endif
}


// =================================================================

void video_deinit(void)
{

#if !defined(__OS2__) && !defined(__NT__)

r.h.ah = 0x00;
r.h.al = origmode;
#ifdef __FLAT__
int386(0x10,&r,&r);
#else
int86(0x10,&r,&r);
#endif
setlines(origlines);
#else
  #ifdef __OS2__
    origvio.cb = sizeof(VIOMODEINFO);
    VioSetMode(&origvio, 0);
  #else

    // Reset screen in NT environment.

  #endif
#endif

}

// ================================================================

void biprint(int x, int y, int attr1, int attr2, unsigned char *line, char sep)
{
   #if defined (__OS2__) || defined(__NT__)
   char tmpline[300];
   word len = 0;

   extern word slamlenbiline(unsigned char * d, char * s, short mask1, short mask2, char sep);
   #pragma aux slamlenbiline = \
      "       mov cx, 0    " \
      "       cld          " \
      "begin: lodsb        " \
      "       or al, al    " \
      "       jz getout    " \
      "       cmp al, bl   " \
      "       jz switch    " \
      "       stosw        " \
      "       inc cx       " \
      "       inc cx       " \
      "       jmp begin    " \
      " switch:            " \
      "       xchg ax, dx  " \
      "       jmp begin    " \
      " getout:            " \
      "       stosw        " \
      parm [edi] [esi] [ax] [dx] [bl] \
      value [cx] \
      modify [ax dx esi edi bl]   ;

   len = slamlenbiline(tmpline, line, (attr1 << 8), (attr2 << 8), sep);

   VioWrtCellStr(tmpline, (int) len, x, y, 0);

   #else

   unsigned short * out = screen + (x*maxx) + y;

   #ifndef __FLAT__

   extern void slambiline(unsigned short * d, char * s, short mask1, short mask2, char sep);
   #pragma aux slambiline = \
      "       cld         " \
      "begin: lodsb       " \
      "       or al, al   " \
      "       jz getout   " \
      "       cmp al, bl  " \
      "       jz switch   " \
      "       stosw       " \
      "       jmp begin   " \
      " switch:           " \
      "       xchg ax, dx " \
      "       jmp begin   " \
      " getout:           " \
      parm [es di] [ds si] [ax] [dx] [bl] \
      modify [ax dx es di ds si bl]   ;

   #else             // FLAT

   extern void slambiline(unsigned short * d, char * s, short mask1, short mask2, char sep);
   #pragma aux slambiline = \
      "       cld         " \
      "begin: lodsb       " \
      "       or al, al   " \
      "       jz getout   " \
      "       cmp al, bl  " \
      "       jz switch   " \
      "       stosw       " \
      "       jmp begin   " \
      " switch:           " \
      "       xchg ax, dx " \
      "       jmp begin   " \
      " getout:           " \
      parm [edi] [esi] [eax] [edx] [bl] \
      modify [eax edx esi edi bl] ;

   #endif

   slambiline(out, line, (attr1 << 8), (attr2 << 8), sep);

   #endif

}


// =====================================================================

void biprinteol(int x, int y, int attr1, int attr2, unsigned char *line, char sep)
{
   #if defined (__OS2__) || defined(__NT__)
   char tmpline[300];
   word len;
   USHORT cell = 32 | (attr1<<8);

   // slamlenbiline, see function above..

   len = slamlenbiline(tmpline, line, (attr1 << 8), (attr2 << 8), sep);

   VioWrtCellStr(tmpline, (int) len, x, y, 0);

   len >>= 1;

   #ifdef __OS2__

     VioWrtNCell((PBYTE) &cell, (int) (maxx-len-y), x, y+len, 0);

   #else    // NT

     memset(tmpline, 32, 132);
     printn(x, y+len, attr1, tmpline, (int)(maxx-len-y));

   #endif

   #else

   unsigned short * out = screen + (x*maxx) + y;

//   while(*line)
//     *out++ = ((attr << 8) | *line++);

   #ifndef __386__

   extern void slamleftbiline(unsigned short * d, char * s, short mask1, short mask2, short left, char sep);
   #pragma aux slamleftbiline = \
      "       cld          " \
      "begin: lodsb        " \
      "       or al, al    " \
      "       jz getout    " \
      "       cmp al, bl   " \
      "       jz switch    " \
      "       stosw        " \
      "       loop begin   " \
      " switch:            " \
      "       xchg ax, dx  " \
      "       jmp begin    " \
      " getout:            " \
      "       mov al, ' '  " \
      "       jcxz ende    " \
      "       rep stosw    " \
      " ende:              " \
      parm [es di] [ds si] [ax] [dx] [cx] [bl] \
      modify [ax bx cx dx es di ds si bl]   ;

   #else

   extern void slamleftbiline(unsigned short * d, char * s, short mask1, short mask2, short left, char sep);
   #pragma aux slamleftbiline = \
      "       cld          " \
      "begin: lodsb        " \
      "       or al, al    " \
      "       jz getout    " \
      "       cmp al, bl   " \
      "       jz switch    " \
      "       stosw        " \
      "       loop begin   " \
      " switch:            " \
      "       xchg ax, dx  " \
      "       jmp begin    " \
      " getout:            " \
      "       mov al, ' '  " \
      "       jcxz ende    " \
      "       rep stosw    " \
      " ende:              " \
      parm [edi] [esi] [ax] [dx] [ecx] [bl] \
      modify [ax bx ecx dx esi edi bl]   ;

    #endif

    slamleftbiline(out, line, (attr1 << 8), (attr2 << 8), maxx-y, sep);

   #endif

}


// =================================================================


/* Now a function to write a string directly to the screen */

void print(int x, int y, int attr, unsigned char *line)
{

   #ifndef __WATCOMC__

   unsigned int ofs = ((x*maxx) + y) << 1;

   asm push ds

   asm mov es,vbase
   asm mov di,ofs

   asm mov ax,attr
   asm mov cl,8
   asm shl ax,cl

   asm lds si,line
   asm cld

repeat:

   asm lodsb
   asm or al,al
   asm jz end
   asm stosw
   asm jmp repeat

end:
   asm pop ds

   #elif defined (__OS2__)

   VioWrtCharStrAtt(line, (int) strlen(line), x, y,(PBYTE) &attr, 0);

   #elif defined (__NT__)

   printn(x, y, attr, line, strlen(line));

   #else

   unsigned short *out = screen + (x*maxx) + y;
//   short contents = attr << 8;

//   while(*line)
//     *out++ = ((attr << 8) | *line++);

   #ifndef __FLAT__
   extern void slamline(unsigned short * d, char * s, int mask);
   #pragma aux slamline = \
      "       cld       " \
      "begin: lodsb     " \
      "       or al, al " \
      "       jz getout " \
      "       stosw     " \
      "       jmp begin " \
      " getout:         " \
      parm [es di] [ds si] [ax] \
      modify [ax es di ds si]   ;
   #else
   extern void slamline(unsigned short * d, char * s, int mask);
   #pragma aux slamline = \
      "       cld       " \
      "begin: lodsb     " \
      "       or al, al " \
      "       jz getout " \
      "       stosw     " \
      "       jmp begin " \
      " getout:         " \
      parm [edi] [esi] [ax] \
      modify [ax edi esi]   ;
   #endif

   slamline(out, line, (attr << 8));

   #endif

 }


// =============================================================

/* Print function that takes a variable number of args like printf  */

void vprint(int x, int y, int attr, char *fmt, ...)
{
   va_list argptr;
   char temp[200];
   int howmuch;

   va_start(argptr, fmt);
   howmuch = vsprintf(temp, fmt, argptr);
   va_end(argptr);

   printn(x, y, attr, temp, howmuch);

}

// =================================================================
/* Print function that takes a variable number of args like printf  */
// =================================================================

void vbiprint(int x, int y, int attr1, int attr2, char sep, char *fmt, ...)
{
   va_list argptr;
   char temp[200];
   int howmuch;

   va_start(argptr, fmt);
   howmuch = vsprintf(temp, fmt, argptr);
   va_end(argptr);

   biprint(x, y, attr1, attr2, temp, sep);

}


// ==================================================================

/* Now a function to write a string directly to the screen */

void printn(int x, int y, int attr, unsigned char *line, int len)
{
   #ifndef __WATCOMC__

   unsigned int ofs = ((x*maxx) + y) << 1;


   asm push ds

   asm mov es,vbase
   asm mov di,ofs

   asm mov ax,attr
   asm mov cl,8
   asm shl ax,cl
   asm mov cx,len

   asm lds si,line
   asm cld

repeat:

   asm jcxz end
   asm lodsb
   asm stosw
   asm loop repeat

end:
   asm pop ds

   #elif defined (__OS2__)

   VioWrtCharStrAtt(line, (int) len, x, y,(PBYTE) &attr, 0);

   #elif defined (__NT__)

   static WORD vattrs[133];
   WORD *q;
	COORD dwCursorPosition;
   unsigned long dwWritten;
   int i;
   BOOL fResult = TRUE;


   dwCursorPosition.X = y;
   dwCursorPosition.Y = x;

   for (q = vattrs, i = 0; i < len; i++)
	{
      *q++ = (WORD) attr;
	}

	fResult = WriteConsoleOutputCharacter (
      ScreenHandle,
      line,
      len,
		dwCursorPosition,
		&dwWritten);

   if (!fResult) return;

   WriteConsoleOutputAttribute (
         ScreenHandle,
			vattrs,
         len,
			dwCursorPosition,
			&dwWritten);

   // End of NT stuff

   #else

   unsigned short *out = screen + (x*maxx) + y;
//   short contents = attr << 8;

//   while(*line)
//     *out++ = ((attr << 8) | *line++);

   #ifndef __FLAT__
   extern void slamnline(unsigned short * d, char * s, int mask, int len);
   #pragma aux slamnline =  \
      "       jcxz getout " \
      "       cld         " \
      "begin: jcxz getout " \
      "       lodsb       " \
      "       stosw       " \
      "       loop begin  " \
      " getout:           " \
      parm [es di] [ds si] [ax] [cx] \
      modify [ax es di ds si cx]     ;
   #else
   extern void slamnline(unsigned short * d, char * s, int mask, int len);
   #pragma aux slamnline =  \
      "       jcxz getout " \
      "       cld         " \
      "begin: jcxz getout " \
      "       lodsb       " \
      "       stosw       " \
      "       loop begin  " \
      " getout:           " \
      parm [edi] [esi] [ax] [ecx] \
      modify [ax esi edi ecx]     ;
   #endif

   slamnline(out, line, (attr << 8), len);

   #endif

 }


// =================================================================


void printeol(int x, int y, int attr, unsigned char *line)
{

   #ifndef __WATCOMC__

   unsigned int ofs = ((x*maxx) + y) << 1;

   asm push ds

   asm mov es,vbase
   asm mov di,ofs

   asm mov ax,attr
   asm mov cl,8
   asm shl ax,cl

   asm mov cx,maxx
   asm sub cx,y

   asm lds si,line

   asm cld

again:

   asm lodsb
   asm or al,al
   asm jz endstring
   asm stosw
   asm loop again

endstring:

   asm jcxz finito
   asm mov al,32
   asm rep stosw

finito:

   asm pop ds

 #elif defined (__OS2__) 
   int len;
   USHORT cell = 32 | (attr<<8);

   len = (int) strlen(line);

   VioWrtCharStrAtt(line, len, x, y, (PBYTE) &attr, 0);

   VioWrtNCell((PBYTE)&cell, (int) maxx-len-y, x, y+len, 0);
 
 #elif defined(__NT__)

   char tmpline[133];
   int len = strlen(line);

   printn(x, y, attr, line, len);
   memset(tmpline, 32, 132);
   printn(x, y+len, attr, tmpline, (int)(maxx-len-y));


 #else

   unsigned short *out = screen + (x*maxx) + y;
//   short contents = attr << 8;

//   while(*line)
//     *out++ = ((attr << 8) | *line++);


   #ifndef __FLAT__
   extern void slameol(unsigned short * d, char * s, int mask, int max);
   #pragma aux slameol   =   \
      "       cld          " \
      "begin: lodsb        " \
      "       or al, al    " \
      "       jz clear     " \
      "       stosw        " \
      "       loop begin   " \
      "clear: jcxz getout  " \
      "       mov al, 32   " \
      "       cld          " \
      "       rep stosw    " \
      "getout:             " \
      parm [es di] [ds si] [ax] [cx] \
      modify [ax es di ds si cx] ;
   #else
   extern void slameol(unsigned short * d, char * s, int mask, int max);
   #pragma aux slameol   =   \
      "       cld          " \
      "begin: lodsb        " \
      "       or al, al    " \
      "       jz clear     " \
      "       stosw        " \
      "       loop begin   " \
      "clear: jcxz getout  " \
      "       mov al, 32   " \
      "       cld          " \
      "       rep stosw    " \
      "getout:             " \
      parm [edi] [esi] [ax] [ecx] \
      modify [ax edi esi ecx] ;
   #endif

   slameol(out, line, (attr << 8), maxx);



// =============
//   int len, left;

//   word ofs = ((x*maxx) + y);
//   unsigned short *out = screen + ofs;
//   unsigned short contents = attr << 8;

//   len = (int) strlen(line);
//   left = len;

//   while(left--)
//     {
//     *out = (contents | *line);
//     out++;
//     line++;
//     }

//   left = maxx-len;
//   contents = (attr << 8) + 32;

//   while(left--)
//     {
//     *out++ = contents;
//     }

 #endif
}


// =================================================================


void printeoln(int x, int y, int attr, unsigned char *line, int len)

{

   #ifndef __WATCOMC__

   unsigned int ofs = ((x*maxx) + y) << 1;
   unsigned int left = maxx - y - len;

   asm push ds

   asm mov es,vbase
   asm mov di,ofs

   asm mov ax,attr
   asm mov cl,8
   asm shl ax,cl

   asm mov cx,len

   asm lds si,line

   asm cld

again:

   asm jcxz rest
   asm lodsb
   asm stosw
   asm loop again

rest:

   asm mov cx,left
   asm jcxz finito
   asm mov al,32
   asm rep stosw

finito:

   asm pop ds

 #elif defined (__OS2__)

   USHORT cell = 32 | (attr<<8);

   VioWrtCharStrAtt(line, len, x, y, (PBYTE) &attr, 0);

   VioWrtNCell((PBYTE)&cell, (int) maxx-len-y, x, y+len, 0);

 #elif defined (__NT__)

   char tmpline[133];

   printn(x, y, attr, line, len);
   memset(tmpline, 32, 132);
   printn(x, y+len, attr, tmpline, (int)(maxx-len-y));

 #else
   unsigned short *out = screen + (x*maxx) + y;
//   short contents = attr << 8;

//   while(*line)
//     *out++ = ((attr << 8) | *line++);

   #ifndef __FLAT__
   extern void slamneol(unsigned short * d, char * s, int mask, int len, int max);
   #pragma aux slamneol  =   \
      "       cld          " \
      "       sub bx, cx   " \
      "begin: jcxz clear   " \
      "       lodsb        " \
      "       stosw        " \
      "       loop begin   " \
      "clear: mov cx, bx   " \
      "       jcxz getout  " \
      "       mov al, 32   " \
      "       cld          " \
      "       rep stosw    " \
      "getout:             " \
      parm [es di] [ds si] [ax] [cx] [bx] \
      modify [ax es di ds si cx bx] ;
   #else
   extern void slamneol(unsigned short * d, char * s, int mask, int len, int max);
   #pragma aux slamneol  =   \
      "       cld          " \
      "       sub bx, cx   " \
      "begin: jcxz clear   " \
      "       lodsb        " \
      "       stosw        " \
      "       loop begin   " \
      "clear: mov cx, bx   " \
      "       jcxz getout  " \
      "       mov al, 32   " \
      "       cld          " \
      "       rep stosw    " \
      "getout:             " \
      parm [edi] [esi] [ax] [ecx] [ebx] \
      modify [ax edi esi ecx ebx] ;
   #endif

   slamneol(out, line, (attr << 8), len, maxx);





//   int left;

//   word ofs = ((x*maxx) + y);
//   unsigned short *out = screen + ofs;
//   short contents = attr << 8;
//   left = len;

//   while(left--)
//     {
//     *out = (contents | *line);
//     out++;
//     line++;
//     }

//   left = maxx-len;
//   contents = (attr << 8) + 32;
//
//   while(left--)
//     {
//     *out++ = contents;
//     }

 #endif
}

// =================================================================

/* A function to write a character directly to the screen */

// =================================================================


void printc(int x, int y, int attr, unsigned char token)

{
  #ifndef __OS2__

   #ifdef __NT__

      char s[2];

      s[0] = token;
      s[1] = '\0';
      printn(x,y,attr,s,1);

   #else

      *(screen + (x * maxx) + y) = token | (attr<<8);

   #endif

  #else
   USHORT cell = token | (attr<<8);

   VioWrtNCell((PBYTE)&cell, 1, x, y, 0);
  #endif

}


// =================================================================


void cls(void)
{

   #ifndef __WATCOMC__
   asm push ds                   /* Save DS */

   asm mov cx,scrnsize

   asm mov es,vbase
   asm mov di,0

   asm mov ah,7
   asm mov al,32

   asm cld

   asm rep stosw

   asm pop ds

   #elif defined (__OS2__)

      USHORT cell = 0x0720;

      VioScrollUp(0,0,-1,-1,-1,(PBYTE)&cell,0);

   #elif defined(__NT__)

      COORD coord;
      dword dwWritten;

      coord.X = 0;
      coord.Y = 0;

      FillConsoleOutputAttribute(
         ScreenHandle,
         7,
         scrnsize,
         coord,
         &dwWritten);

      FillConsoleOutputCharacter(
         ScreenHandle,
         0x20,
         scrnsize,
         coord,
         &dwWritten);

   #else

//   unsigned short *where = screen;
//   int howmuch = scrnsize;

//   while(howmuch--)
//     *where++ = 0x0720;

   #ifndef __FLAT__
   extern void asmcls(unsigned short * d, int len);
   #pragma aux asmcls    =    \
      "       jcxz getout   " \
      "       cld           " \
      "       mov ax, 0720h " \
      "       rep stosw     " \
      "getout:              " \
      parm [es di] [cx]       \
      modify [ax es di cx]    ;
   #else
   extern void asmcls(unsigned short far * d, int len);
   #pragma aux asmcls    =    \
      "       jcxz getout   " \
      "       cld           " \
      "       mov ax, 0720h " \
      "       rep stosw     " \
      "getout:              " \
      parm [edi] [ecx]       \
      modify [ax edi ecx]    ;
   #endif

   asmcls(screen, scrnsize);

   #endif

}

// =================================================================


void clsw(unsigned char colour)
{

   #ifndef __WATCOMC__
   asm push ds                   /* Save DS */

   asm mov cx,scrnsize

   asm mov es,vbase
   asm mov di,0

   asm mov ah,colour
   asm mov al,32

   asm cld

   asm rep stosw

   asm pop ds

   #elif defined (__OS2__)

//      USHORT cell = 0x0720;
      USHORT cell = 0x0020 | (colour << 8);

      VioScrollUp(0,0,-1,-1,-1,(PBYTE)&cell,0);

   #elif defined(__NT__)

      COORD coord;
      dword dwWritten;

      coord.X = 0;
      coord.Y = 0;

      FillConsoleOutputAttribute(
         ScreenHandle,
         colour,
         scrnsize,
         coord,
         &dwWritten);

      FillConsoleOutputCharacter(
         ScreenHandle,
         0x20,
         scrnsize,
         coord,
         &dwWritten);

   #else

//   unsigned short *where = screen;
//   int howmuch = scrnsize;
   unsigned short cell = 0x0020 | (colour << 8);

//   while(howmuch--)
//     *where++ = cell;

   #ifndef __FLAT__
   extern void asmclsw(unsigned short * d, int len, int cell);
   #pragma aux asmclsw   =    \
      "       jcxz getout   " \
      "       cld           " \
      "       rep stosw     " \
      "getout:              " \
      parm [es di] [cx] [ax]  \
      modify [ax es di cx]    ;
   #else
   extern void asmclsw(unsigned short * d, int len, int cell);
   #pragma aux asmclsw   =    \
      "       jcxz getout   " \
      "       cld           " \
      "       rep stosw     " \
      "getout:              " \
      parm [edi] [ecx] [ax]  \
      modify [ax edi ecx]    ;
   #endif

   asmclsw(screen, scrnsize, 0x0020 | (colour << 8));

   #endif

}

// =================================================================

/* And one to clear the screen w/ a given attribute & character! */

// =================================================================


/*void ClsWith(int attr, unsigned char token)

{
   int   l,
   val = token | (attr<<8);

   for(l=0; l<25*maxx; l++)
            *(screen + l) = val;
} */


// =================================================================


void ClsRectWith(int y1, int x1, int y2, int x2,
                                       int attr, unsigned char token)

{
   #ifndef __WATCOMC__

   int val = token | (attr<<8);

   asm push ds                   /* Save DS */


   asm mov es,vbase

   asm mov ax,y1
   asm mov bx,x1
   asm mov cx,maxx
   asm mul cl           /* Begin regel * aantal kolommen        */
   asm add ax,bx        /* Bereken begin offset upper left in.. */
   asm shl ax,1         /* .. integer waarden                   */
   asm mov si,ax        /* Source index points to begin offset  */

   asm mov bx,x2
   asm sub bx,x1
   asm inc bx           /* BX is aantal kolommen + 1            */

   asm mov dx,y2
   asm sub dx,y1
   asm inc dx           /* DX is aantal rijen + 1               */

   asm mov ax,val

   asm cld

loop:

   asm mov di,si        /* Point to begin of 1 line in window    */
   asm mov cx,bx        /* Aantal kolommen om op te werken       */
   asm rep stosw        /* Fill this line                        */
   asm mov cx,maxx
   asm shl cx,1         /* Aantal bytes op 1 regel               */
   asm add si,cx        /* Zak effectief 1 regel in videomem     */
   asm dec dx           /* Verlaag aantal regels om nog te doen  */
   asm jne loop         /* Loop als we er nog 1 of meer moeten   */

   asm pop ds

   #elif defined (__OS2__)

   USHORT  cell = token | (attr<<8);

/*    BYTE Cell[2] = {'*', 0x07); */

   VioScrollUp(y1,x1,y2,x2,-1,(PBYTE)&cell,0);

   #elif defined (__NT__)
   COORD      coord;
   dword      dwWritten;
   int        i;

   coord.X = x1;

   for(i=y1; i<=y2; i++)
      {
      coord.Y = i;
      FillConsoleOutputAttribute(
         ScreenHandle,
         attr,
         x2-x1+1,
         coord,
         &dwWritten);

      FillConsoleOutputCharacter(
         ScreenHandle,
         token,
         x2-x1+1,
         coord,
         &dwWritten);
      }

   #else

   int i, j;
   int regellen = (x2-x1+1);
//   unsigned int ofs;
//   short *out;
   unsigned short *out;
   unsigned short contents = token | (attr<<8);

   for(i=y1; i<y2+1; i++)
     {
     out = screen + (i*maxx) + x1;
     j = regellen;
     while(j--)
       *out++ = contents;
     }

   #endif
}


// =================================================================


void MoveXY(int col, int row)
{
#if !defined(__OS2__) && !defined(__NT__)

    union REGS regs;

    regs.h.dh = (unsigned) (row-1);
    regs.h.dl = (unsigned) (col-1);
    regs.h.bh = 0;
    regs.h.ah = 2;

#ifdef __386__
    int386(0x10, &regs, &regs);
#else
    int86(0x10, &regs, &regs);
#endif

#else

   #ifdef __OS2__
     VioSetCurPos(row-1, col-1, 0);
   #else

   COORD dwCursorPosition;

   dwCursorPosition.X = col-1;
   dwCursorPosition.Y = row-1;

   (void) SetConsoleCursorPosition (ScreenHandle, dwCursorPosition);

   #endif

#endif
}

// =================================================================


#ifdef __OS2__
int setlines(char lines)
{
 int rc;
 VIOMODEINFO vmi;

 vmi.cb = sizeof(VIOMODEINFO);
 VioGetMode(&vmi, 0);
//  maxx = vmi.col;
 vmi.row = lines;
 if(lines==30 || lines==60)
    vmi.vres=480;
 else
    vmi.vres=400;

 rc = VioSetMode(&vmi, 0);

 video_init();
 #ifdef __WATCOMC__
 _settextcursor(0x2000);
 #else
 _setcursortype(_NOCURSOR);
 #endif


 return rc == 0 ? 0 : -1;

}

#elif defined (__NT__)

int setlines(char lines)
{
  COORD coord;
  SMALL_RECT rect;
  BOOL rc;
 
  coord.X = maxx;
  coord.Y = lines;

  if(SetConsoleScreenBufferSize(ScreenHandle, coord) == FALSE)
    return -1;

  rect.Left   = 0;
  rect.Top    = 0;
  rect.Right  = maxx-1;
  rect.Bottom = lines-1;

  rc = SetConsoleWindowInfo(ScreenHandle, TRUE, &rect);

  video_init();
  #ifdef __WATCOMC__
  _settextcursor(0x2000);
  #else
  _setcursortype(_NOCURSOR);
  #endif

  return rc == TRUE ? 0 : -1;

}


#else

int setlines(char lines)
{
   union REGS cpu;

   if(lines != 25 && lines != 28 && lines != 50)
     return 0;

	cpu.h.ah=0x12;
	cpu.h.al=(unsigned char)400;
#ifndef __386__
   int86(0x10, &cpu, &cpu);
#else
   int386(0x10, &cpu, &cpu);
#endif

	 // set 25 lines mode..
	 cpu.h.ah=0;
	 cpu.h.al=(unsigned short)3;

    #ifndef __386__
    int86(0x10, &cpu, &cpu);
    #else
    int386(0x10, &cpu, &cpu);
    #endif

#ifdef __WATCOMC__
	 switch (lines) {
		 case 25:	 break; /* already 25 */
       case 28:    cpu.w.ax=0x1111; break;
       case 50:    cpu.w.ax=0x1112; break;
	 }
#else
    switch (lines) {
		 case 25:	 break; /* already 25 */
       case 28:    cpu.x.ax=0x1111; break;
       case 50:    cpu.x.ax=0x1112; break;
	 }
#endif

    cpu.h.bl=0;
    #ifndef __386__
    int86(0x10, &cpu, &cpu);
    #else
    int386(0x10, &cpu, &cpu);
    #endif

    vbase=maxx=maxy=0;
    video_init();
//    maxy=lines;
//    _crtinit();
    #ifdef __WATCOMC__
    _settextcursor(0x2000);
    #else
    _setcursortype(_NOCURSOR);
    #endif

    return 0;
}
#endif

// =================================================================


#ifdef __WATCOMC__

#ifdef __OS2__
void _settextcursor(word type)
{
   VIOCURSORINFO ci;

   VioGetCurType(&ci, 0);

   switch(type)
     {
     case 0x2000: ci.attr = -1; break;
     case 0x0607: ci.attr =  1; ci.yStart = -80; ci.cEnd = -90; break;  // normal
     case 0x0407: ci.attr =  1; ci.yStart = -50; ci.cEnd = -90; break;  // block
     }

   VioSetCurType(&ci, 0);

}

#elif defined(__NT__)

void _settextcursor(word type)
{
   CONSOLE_CURSOR_INFO ci;

   switch(type)
     {
     case 0x2000: ci.bVisible = 0; break;
     case 0x0607: ci.bVisible = 1; ci.dwSize = 20; break;  // normal
     case 0x0407: ci.bVisible = 1; ci.dwSize = 50; break;  // block
     }

   SetConsoleCursorInfo(ScreenHandle, &ci);
}

#else

#ifndef __FLAT__
void _settextcursor(word type)
{
   union REGS regs;

   regs.h.ah = 1;

   regs.w.cx = type;

   if(mono)
     {
     switch(type)
       {
       case 0x0607 : regs.h.ch = 11; regs.h.cl = 12; break;
       case 0x0407 : regs.h.ch =  8; regs.h.cl = 12; break;
       default     : break;
       }
     }

//   #ifndef __FLAT__
   int86(0x10, &regs, &regs);
//   #else
//   int386(0x10, &regs, &regs);
//   #endif

}
#endif

#endif

#endif

// ==============================================================
// Calc len of string, exlusing ~ characters (for highlighting)
// ==============================================================


size_t HLstrlen(char *s)
{
   size_t reallen = 0;

   while(*s)
     {
     if(*s != '~') reallen++;
     s++;
     }

   return reallen;
}

// ==============================================================
//
//    This function emulates this OS/2 VIO call for WIN NT/95
//    It is a quick hack to get stuff working in NT...
//
// ==============================================================

#ifdef __NT__

USHORT VioWrtCellStr (PCH pchCellString, USHORT cbCellString, USHORT usRow, USHORT usColumn, USHORT hvio)
{
	DWORD dwWritten;
	BOOL fResult = TRUE;
	static char vchars[500];
	static WORD vattrs[500];
	COORD dwCursorPosition;

	int i, count;
	char *p;
	unsigned char *s = (unsigned char *) pchCellString;
	WORD *q;

	if (cbCellString > 1000)
		return 1;

	dwCursorPosition.X = usColumn;
	dwCursorPosition.Y = usRow;

	count = cbCellString >> 1;
	for (p = vchars, q = vattrs, i = 0; i < count; i++)
	{
		*p++ = (char) (*s++);
		*q++ = (WORD) (*s++);
	}

	fResult = WriteConsoleOutputCharacter (
      ScreenHandle,
		vchars,
		count,
		dwCursorPosition,
		&dwWritten);

	if (fResult)
	{
		fResult = WriteConsoleOutputAttribute (
         ScreenHandle,
			vattrs,
			count,
			dwCursorPosition,
			&dwWritten);
	}
	return (fResult != TRUE);
}

#endif

