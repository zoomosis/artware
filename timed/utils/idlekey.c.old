#include <stdlib.h>
#include <conio.h>

#ifdef __OS2__

   #define INCL_SUB
   #define INCL_NOPMAPI
   #include <os2.h>

#endif


#define MAXSTUFF 10
#define ZF 0x0040                   /* Mask for zero-flag in regs.x.flags */


int keybuf[MAXSTUFF];
static int curkey=0;
static int enhanced = 0;

int  get_idle_key(char allowstuff);
void stuffkey(int key);
void kbflush(void);
void check_enhanced(void);
int xkbhit(void);

#if defined (__WATCOMC__) && !defined(__FLAT__)

#include <i86.h>

extern unsigned short asmkbhit(void);

#pragma aux asmkbhit = \
   " mov ah, 11h "     \
   " cmp enhanced, 1 " \
   " je go_on        " \
   " mov ah, 1h      " \
   " go_on: int 16h  " \
   " jnz goback      " \
   " mov ax, 0       " \
   " goback:         " \
   modify [ax]         \
   value [ax]          ;
#else
#ifndef __OS2__
int asmkbhit(void)
{
    asm mov ah, 11h
    asm cmp enhanced, 1
    asm je go_on
    asm mov ah, 1h
go_on:
    asm  int 16h
    asm jnz goback
    asm mov ax, 0

goback:
}
#endif
#endif


void check_enhanced(void)
{

   #ifndef __FLAT__

   #ifdef __WATCOMC__
   union REGS regs;
   unsigned short ret;
   #else
   union REGS regs;
   unsigned short ret;
   #endif

   regs.h.ah = 5;    // Stuff character if keyboard
   regs.h.cl = 32;   // ASCII code for space
   regs.h.ch = 57;   // Scan code for space

   int86(0x16, &regs, &regs);

   enhanced = 1;  // So asmkbhit() will call right f'tion

   if( (ret = asmkbhit()) != 0 )  // ZF is clear when key available
     {
     if(ret == 0x3920)
       enhanced = 1;
     else
       enhanced = 0;

     if(xkbhit())  // If no enhanced keyboard (no keystroke waiting),
        getch();   // this won't cause prg to wait for key..
                   // If it IS enhanced, it will clear keypress..
     }
   else
     enhanced = 0;

   #else

   #ifdef __OS2__

   KBDINFO keystat;

   keystat.cb = sizeof(KBDKEYINFO);
   KbdGetStatus(&keystat, 0);
   keystat.fsMask |= 0x0004;
   keystat.fsMask &= 0xFFF7;
   KbdSetStatus(&keystat, 0);

   #else
     enhanced = 1;
   #endif

   #endif

}


int get_idle_key(char allowstuff)
{
   int i;
   #ifndef __OS2__
   union REGS regs;
   #else
   KBDKEYINFO kbd;
   #endif

   if(allowstuff && (curkey>0))
      return keybuf[--curkey];


   #ifndef __FLAT__
   if(!xkbhit() && showclock && (cfg.usr.status & CLOCK))
     {
//     update_clock();   // Take this out!
     while(!xkbhit())
       {
       for(i=0; i<100; i++)
         {
         if(xkbhit()) break;
//         update_clock();   // Take this out!
         give_slice();
         }
       if(!xkbhit()) update_clock(0);
       }
     }

   #ifndef __OS2__
   #ifndef __WATCOMC__
   if (_osmajor > 9)    /* OS/2 DOS box time slice release */
      {
      while (!xkbhit())
         asm {
            mov ax,1680h
            int 2fh
         }
      }
   #endif
   #endif


   #endif

   #ifdef __OS2__

   KbdCharIn(&kbd, 0, 0);

   if( (i=kbd.chChar) == 0 )
      i = 256 + kbd.chScan;

   else if(i == 0xE0)
      {
      if(kbd.chScan != 0)     // So NOT ASCII 224!
         i = 256 + kbd.chScan ;
      }

   #else          // DOS shit
   #ifndef __FLAT__
   if(cfg.usr.status & LOWLEVELKB)
     {
     if(enhanced)
       regs.h.ah = 0x10;
     else
       regs.h.ah = 0x00;

     int86(0x16, &regs, &regs);

     if(regs.h.al == 0)
       {
       i = 256 + regs.h.ah;
       }
     else if(regs.h.al == 0xE0)
       {
       if(regs.h.ah == 0)  // ASCII 224
          i = 0xE0;
       else
          i = 256 + regs.h.ah;
       }
     else
        i = regs.h.al;
     }
   else        // Not low level keyboard routines...
   #endif  // !FLAT
     {
     i = getch();
     if(i == 0)
        i = 256 + getch();
     }

   #endif

   return i;

}

void stuffkey(int key)
{
   if(curkey <= MAXSTUFF)
      keybuf[curkey++] = key;
}

/* Flush the keyboard buffer */

void kbflush(void)
{
   while(xkbhit()) get_idle_key(0);
}


int xkbhit(void)
{
   #ifndef __OS2__


   #ifndef __FLAT__
   #ifdef __WATCOMC__
   union REGPACK regs;
   #else
   union REGS regs;
   #endif

   if(cfg.usr.status & LOWLEVELKB)
     {
     return(asmkbhit());     // ZF is clear when key available
     }
   else
   #endif  // !FLAT
     return kbhit();

   #else

   return kbhit();

   #endif
}
