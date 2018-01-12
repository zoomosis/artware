/* #pragma inline */

#include <dos.h>
#include <string.h>

#ifdef __OS2__
    #define near
    #define far
    #define INCL_SUB
    #define INCL_NOPMAPI
    #include <os2.h>
    #include <conio.h>
#endif

#ifdef __OS2__

   #define word      unsigned short
   #define sword     short

#else

   #define word      unsigned int
   #define sword     int

#endif


void cls(void);
void clsw(unsigned char colour);
void video_init(void);
void print(int x, int y, int attr, unsigned char far *line);
void printn(int x, int y, int attr, unsigned char far *line, int len);
void tabprint(int x, int y, int attr, unsigned char far *line, int tabsize);
void printc(int x, int y, int attr, unsigned char token);
void printeol(int x, int y, int attr, unsigned char far *line);
void printeoln(int x, int y, int attr, unsigned char far *line, int len);
void MoveXY(int col, int row);

void ClsWith(int attr, unsigned char token);
void ClsRectWith(int x1, int y1, int x2, int y2, int attr, unsigned char token);

#ifndef __OS2__
unsigned int far *screen;    /* pointer to screen memory */
unsigned int vbase;      /* segment of video base */
unsigned int scrnsize;
#else
ULONG screen;
USHORT scrnsize;
VIOMODEINFO vmi;
#endif

int maxx;       /* number of columns */
int maxy;       /* number of rows */
int desqview;

#ifndef __OS2__
 union REGS r;
 struct SREGS sr;
 int vmode = 0;
#endif


/* ------------------------------------ */

void video_init(void)

{

/* VIOopen(); */

/* get display mode */

#ifndef __OS2__

r.h.ah = 0x0f;
int86(0x10,&r,&r);

vmode = r.h.al;

/* guess what!?
   it returns the number of columns too! */

if (maxx == 0)
        maxx = (int) r.h.ah;

/* number of rows is harder */

if (maxy == 0)
{
        /* try using a ega/vga function */

        r.x.ax = 0x1130;
        r.x.dx = maxy;
        int86(0x10,&r,&r);

        /* if it fails, it returns 0, so we set
           the screen to 25 lines, otherwise
           it returns the actual number of lines
           of text on the screen */

        maxy = (r.x.dx == 0) ? 25 : (r.x.dx + 1);
}

if (vbase == 0)

        /* if you are in mode 7 the video is
           most likely based at 0xB000, otherwise
           it will be at 0xB800.  This fails for
           a small number of cards, like the
           genius mono displays (although, i've
           never had a failure reported... */

        if (vmode == 0x07)
                vbase = 0xb000;
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

screen = (unsigned int far *) MK_FP(vbase,r.x.di);

scrnsize = maxx * maxy;

#else

  vmi.cb = sizeof(VIOMODEINFO);
  VioGetMode(&vmi, 0);
  maxx = vmi.col;
  maxy = vmi.row;

/*  VioGetBuf(&screen, &scrnsize,0); */

#endif
}


/* Now a function to write a string directly to the screen */

void print(int x, int y, int attr, unsigned char far *line)

{

   #ifndef __OS2__

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

   #else

   VioWrtCharStrAtt(line, (int) strlen(line), x, y,(PBYTE) &attr, 0);

   #endif

 }


/* Now a function to write a string directly to the screen */

void printn(int x, int y, int attr, unsigned char far *line, int len)

{

   #ifndef __OS2__

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

   #else

   VioWrtCharStrAtt(line, (int) len, x, y,(PBYTE) &attr, 0);

   #endif

 }


void printeol(int x, int y, int attr, unsigned char far *line)

{

   #ifndef __OS2__

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

 #else
   int len;
   USHORT cell = 32 | (attr<<8);

   len = (int) strlen(line);

   VioWrtCharStrAtt(line, len, x, y, (PBYTE) &attr, 0);

   VioWrtNCell(&cell, (int) maxx-len, x, y+len, 0);


 #endif
}



void printeoln(int x, int y, int attr, unsigned char far *line, int len)

{

   #ifndef __OS2__

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

 #else

   USHORT cell = 32 | (attr<<8);

   VioWrtCharStrAtt(line, len, x, y, (PBYTE) &attr, 0);

   VioWrtNCell(&cell, (int) maxx-len, x, y+len, 0);


 #endif
}


void tabprint(int x, int y, int attr, unsigned char far *line, int tabsize)

{
   #ifndef __OS2__

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
   asm cmp al,9
   asm je dotab
   asm stosw
   asm loop again
   asm jmp endstring

dotab:

   asm mov dx,cx           /* Bewaar CX               */
   asm mov cx,tabsize      /* Stel counter op tabsize */
   asm mov al,32           /* Spatie schrijven        */
   asm rep stosw           /* Schrijf spaties         */
   asm mov cx,dx           /* Herstel cx              */
   asm sub cx,tabsize      /* Pas cx aan              */
   asm cmp cx,0            /* Zijn we al einde regel? */
   asm jg again

   asm jmp getout

endstring:

   asm jcxz getout
   asm mov al,32
   asm rep stosw

getout:

   asm pop ds

 #else
   char temp[160], *inptr, *outptr;
   int l, len;

   memset(temp, ' ', sizeof(temp));
   inptr  = line;
   outptr = temp;

   while(*inptr != '\0')
      {
      if(*inptr == '\t')
          {
           for(l=0; l<tabsize; l++)
               *outptr++ = ' ';
           inptr++;
          }
      else
         *outptr++ = *inptr++;
      }

   VioWrtCharStrAtt(temp, (int) maxx, x, y, (PBYTE) &attr, 0);

 #endif
 }



/* A function to write a character directly to the screen */


void printc(int x, int y, int attr, unsigned char token)

{
  #ifndef __OS2__
   *(screen + (x * maxx) + y) = token | (attr<<8);
  #else
   USHORT cell = token | (attr<<8);

   VioWrtNCell(&cell, 1, x, y, 0);
  #endif
}


void cls(void)
{

   #ifndef __OS2__
   asm push ds                   /* Save DS */

   asm mov cx,scrnsize

   asm mov es,vbase
   asm mov di,0

   asm mov ah,7
   asm mov al,32

   asm cld

   asm rep stosw

   asm pop ds
   #else
      USHORT cell = 0x0720;

      VioScrollUp(0,0,-1,-1,-1,&cell,0);
   #endif

}

void clsw(unsigned char colour)
{

   #ifndef __OS2__
   asm push ds                   /* Save DS */

   asm mov cx,scrnsize

   asm mov es,vbase
   asm mov di,0

   asm mov ah,colour
   asm mov al,32

   asm cld

   asm rep stosw

   asm pop ds
   #else
//      USHORT cell = 0x0720;
      USHORT cell = 0x0020 | (colour << 8);

      VioScrollUp(0,0,-1,-1,-1,&cell,0);
   #endif

}


/* And one to clear the screen w/ a given attribute & character! */


/*void ClsWith(int attr, unsigned char token)

{
   int   l,
   val = token | (attr<<8);

   for(l=0; l<25*maxx; l++)
            *(screen + l) = val;
} */

void ClsRectWith(int y1, int x1, int y2, int x2,
                                       int attr, unsigned char token)

{
   #ifndef __OS2__
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

   #else

   USHORT  cell = token | (attr<<8);

/*    BYTE Cell[2] = {'*', 0x07); */

   VioScrollUp(y1,x1,y2,x2,-1,&cell,0);

   #endif
}



void MoveXY(int col, int row)
{
#ifndef __OS2__

    union REGS regs;

    regs.h.dh = (unsigned) (row-1);
    regs.h.dl = (unsigned) (col-1);
    regs.h.bh = 0;
    regs.h.ah = 2;
    int86(0x10, &regs, &regs);

#else

//    gotoxy(col,row);
     VioSetCurPos(row-1, col-1, 0);

#endif
}

