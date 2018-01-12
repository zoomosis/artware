#include <conio.h>
#include <stdio.h>
#include <dos.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "global.h"


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


void check_enhanced(void)
{

   #ifndef __OS2__

   union REGS regs;

   regs.h.ah = 5;    // Stuff character if keyboard
   regs.h.cl = 32;   // ASCII code for space
   regs.h.ch = 57;   // Scan code for space

   int86(0x16, &regs, &regs);

   regs.h.ah = 0x11; // Check for enhanced keystroke.

   int86(0x16, &regs, &regs);

   if(!(regs.x.flags & ZF))     // ZF is clear when key available
     {
     if( (regs.h.al == 32) && (regs.h.ah == 57) )
        enhanced = 1;

     if(xkbhit())  // If no enhanced keyboard (no keystroke waiting),
        getch();  // this won't cause prg to wait for key..
                  // If it IS enhanced, it will clear keypress..
     }

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

   #ifndef __OS2__
   if (_osmajor > 9)    /* OS/2 DOS box time slice release */
      {
      while (!xkbhit())

         asm {
            mov ax,1680h
            int 2fh
         }
      }
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

   union REGS regs;

   if(cfg.usr.status & LOWLEVELKB)
     {
     if(enhanced)
       regs.h.ah = 0x11; // Check for enhanced keystroke (enh kbd only).
     else
       regs.h.ah = 0x01; // Check for 'normal' keystroke

     int86(0x16, &regs, &regs);

     return(!(regs.x.flags & ZF));     // ZF is clear when key available
     }
   else
     return kbhit();

   #else

   return kbhit();

   #endif
}