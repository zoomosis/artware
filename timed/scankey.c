#include <stdio.h>
#include <conio.h>

#ifdef __NT__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 *  Important: The following code will break in unpredictable ways if
 *  structure alignment isn't set to Watcom's default.
 *
 *  So don't use wcl386 -zp1 !!!
 *
 *  - ozzmosis 2018-01-19
 */

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

#if 0			
			printf("uc: %d  vk: %d  vs: %d  fShift: %d  fAlt: %d  fCtrl: %d\n", uc, vk, vs, fShift, fAlt, fCtrl);
#endif

			if (fCtrl)
			{
				if (uc == 0)
				{
					if (vk == 36 && vs == 71)
					{
						iKey = 375;  /* Ctrl+Home */
					}
					else if (vk == 35 && vs == 79)
					{
						iKey = 373;  /* Ctrl+End */
					}
					else
					{
						iKey = uc;
					}
				}
				else
				{
				    iKey = uc;
				}
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

#define getch nt_getch

#endif

int main(void)
{
   int ch;

   printf("By all means, start typing:\n\n");

   do {

     ch = getch();
     if(ch == 0)
        ch = 256 + getch();
     printf("ResultCode: %d\n", ch);

   } while(ch != 27);

   return 0;
}

