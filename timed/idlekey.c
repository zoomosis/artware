#ifdef __NT__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

#ifndef __OS2__
  #include <bios.h>
#endif

#ifdef __OS2__

   #define INCL_SUB
   #define INCL_NOPMAPI
   #include <os2.h>

#endif


#define MAXSTUFF 10
#define ZF 0x0040                   /* Mask for zero-flag in regs.x.flags */


int keybuf[MAXSTUFF];
static int curkey=0;
#ifndef __OS2__
static int enhanced = 0;
#endif

// Variables to control macro execution.

static int curmacro = 0;
static int macropos = 0;

#include "includes.h"


#ifdef __NT__

/*
 *  Important: The following code will break in unpredictable ways if
 *  structure alignment isn't set to Watcom's default.
 *
 *  So don't use wcl386 -zp1 !!!
 *
 *  - ozzmosis 2018-01-19
 */

#include "video.h"

static HANDLE HInput = INVALID_HANDLE_VALUE;

static unsigned long key_hit = 0xFFFFFFFFUL;

static int nt_kbhit(void)
{
    int iKey = 0;
    INPUT_RECORD irBuffer;
    DWORD pcRead;

    if (key_hit != 0xFFFFFFFFUL)
    {
        return (int)key_hit;
    }

    memset(&irBuffer, 0, sizeof irBuffer);

	if (HInput == INVALID_HANDLE_VALUE)
	{
        HInput = GetStdHandle(STD_INPUT_HANDLE);
	}
	
	if (HInput == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: Unable to get input handle with GetStdHandle()\n");
		return 0;
	}
	
    if (WaitForSingleObject(HInput, 0L) == 0)
    {
        ReadConsoleInput(HInput, &irBuffer, 1, &pcRead);

        if (irBuffer.EventType == KEY_EVENT && irBuffer.Event.KeyEvent.bKeyDown != 0 && irBuffer.Event.KeyEvent.wRepeatCount <= 1)
        {
            WORD vk, vs, uc;
            BOOL fShift, fAlt, fCtrl;

            vk = irBuffer.Event.KeyEvent.wVirtualKeyCode;
            vs = irBuffer.Event.KeyEvent.wVirtualScanCode;
            uc = irBuffer.Event.KeyEvent.uChar.AsciiChar;

            fShift = (irBuffer.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED);
            fAlt = (irBuffer.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED + LEFT_ALT_PRESSED));
            fCtrl = (irBuffer.Event.KeyEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED + LEFT_CTRL_PRESSED));

			if (fCtrl)
			{
				iKey = uc;
			}
			else if (fAlt)
			{
				iKey = 256 + vs;
			}
			else if (uc == 0)
            {
				/* function & arrow keys */
                iKey = 256 + vs;
			}
			else
			{
				iKey = uc;
			}
        }
    }

    if (iKey != 0)
    {
        key_hit = iKey;
    }

    return (int)iKey;
}

static int nt_getch(void)
{
    int iKey;
	
    while (key_hit == 0xFFFFFFFFUL)
    {
        nt_kbhit();
    }
    iKey = key_hit;
    key_hit = 0xFFFFFFFFUL;
    return (int)iKey;
}

#endif

#if defined(__WATCOMC__) && !defined(__OS2__)

extern void os2slice(void);

#pragma aux os2slice = \
   " mov ax, 1680h "   \
   " int 2fh       "   \
   modify [ax]         ;

#endif

#if !defined(__OS2__) && !defined(__NT__)

int asmkbhit(int enhanced)
{
  int i;

  if(enhanced)
    i = _bios_keybrd(_NKEYBRD_READY);
  else
    i = _bios_keybrd(_KEYBRD_READY);

  return i;
}


#endif

#if 0
#if defined (__WATCOMC__) // && !defined(__FLAT__)

extern int asmkbhit(int enhanced);

#pragma aux asmkbhit =  \
   " mov ah, 11h      " \
   " cmp bx, 1        " \
   " je go_on         " \
   " mov ah, 1h       " \
   " go_on: int 16h   " \
   " jz nokey         " \
   " mov ax, 1        " \
   " jmp goback       " \
   " nokey: mov ax, 0 " \
   " goback:          " \
   parm   [bx]          \
   modify [ax bx]       \
   value  [ax]          ;

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

#endif /* 0 */

// =============================================================

void check_enhanced(void)
{

   #ifndef __FLAT__

   #ifdef __WATCOMC__
   union REGS regs;
   unsigned short ret;
   #else
   union REGS regs;
   unsigned short ret;
   #endif    // __WATCOMC__

   regs.h.ah = 5;    // Stuff character in keyboard
   regs.h.cl = 32;   // ASCII code for space
   regs.h.ch = 57;   // Scan code for space

   int86(0x16, &regs, &regs);

   enhanced = 1;  // So asmkbhit() will call right f'tion

   if( (ret = asmkbhit(enhanced)) != 0 )  // ZF is clear when key available
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

   #else       // __FLAT__

   #ifdef __OS2__

   KBDINFO keystat;

   keystat.cb = sizeof(KBDKEYINFO);
   KbdGetStatus(&keystat, 0);
   keystat.fsMask |= 0x0004;
   keystat.fsMask &= 0xFFF7;
   KbdSetStatus(&keystat, 0);

   #else             // NOT OS2
     enhanced = 1;
   #endif            // __OS2__

   #endif      // End FLAT

}


int get_idle_key(char allowstuff, int scope)
{
   int i;
   #ifndef __OS2__
   #ifndef __NT__
   union REGS regs;
   #endif
   #else
   KBDKEYINFO kbd;
   #endif

   if(allowstuff && (curkey>0))
      return keybuf[--curkey];

   if(curmacro != 0)
     {
     i = KeyMacros[curmacro-1].start[macropos];
     if(++macropos >= KeyMacros[curmacro-1].len)
       curmacro = 0;
     return i;
     }


   #ifndef __FLAT__
   if(!xkbhit() && showclock && (cfg.usr.status & CLOCK))
     {
     while(!xkbhit())
       {
       for(i=0; i<100; i++)
         {
//         vprint(3,67,7, "Slicing: %d ", i);
         if(xkbhit())
            break;
//         vprint(0,20,7, "Slicing...", i);
         give_slice();
//         vprint(0,20,7, "          ", i);
         }

       if(!xkbhit())
         {
         update_clock(0);
         }

       }
     }

   #ifndef __OS2__
   if (_osmajor > 9)    /* OS/2 DOS box time slice release */
      {
      while (!xkbhit())
      #ifndef __WATCOMC__
         asm {
            mov ax,1680h
            int 2fh
         }
      #else
         os2slice();
      #endif
      }
   #endif    // __OS2__

   #endif    // ! __FLAT__

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
#ifdef __NT__
     i = nt_getch();
#else
     i = getch();
     if(i == 0)
	 {
        i = 256 + getch();
     }
#endif
     }

   #endif

   // Now we check for macro's and keyboard mappings

   // First we check global mappings that are always active

   if(GlobalKeys[i] != 0)
     i = GlobalKeys[i];
   else
     {
     // Check other scopes, if command/key is defined: return mapping
     // Otherwise, we simply return the keycode.

     switch(scope)
       {
       case READERSCOPE:
         if(ReadMessageKeys[i] != 0)
           i = ReadMessageKeys[i];
         break;

       case AREASCOPE:
         if(AreaSelectKeys[i] != 0)
           i = AreaSelectKeys[i];
         break;

       case EDITORSCOPE:
         if(EditorKeys[i] != 0)
           i = EditorKeys[i];
         break;

       case LISTSCOPE:
         if(ListKeys[i] != 0)
           i = ListKeys[i];
         break;
        }
     }

   if(i > 512)    // Macro
     {
     curmacro = (i - 512);
     if(NumMacros < curmacro)   // check if it exists..
        Message("Invalid macro called!", 10, 0, YES);
     macropos = 1;     // start at 1, we'll return 0 now..
     i = KeyMacros[curmacro-1].start[0];

     if(KeyMacros[curmacro-1].len == 1)  // Only 1 char? Stop now!
       curmacro = 0;
     }

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
   while(xkbhit()) get_idle_key(0, GLOBALSCOPE);
}


int xkbhit(void)
{
#ifdef __NT__
   return nt_kbhit();
#elif !defined(__OS2__)

   #ifndef __FLAT__

   if(cfg.usr.status & LOWLEVELKB)
     {
     return(asmkbhit(enhanced));     // ZF is clear when key available
     }
   else
   #endif  // !FLAT
     {
     return kbhit();
     }

   #else

   return kbhit();

   #endif
}

// ==============================================================

void MacroStart(sword i)
{

  if(i > 512)    // Macro
    {
    curmacro = (i - 512);
    macropos = 0;     // start at 0, start of macro
    }
  else   // This is not a macro. Simply stuff this code
    {
    stuffkey(i);
    }

}

// ==============================================================

