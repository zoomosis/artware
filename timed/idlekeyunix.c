#include <stdio.h>
#include <sys/time.h>

#ifndef __NetBSD__
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "includes.h"

#define Key_C_F1   0x15e
#define Key_C_F2   0x15f
#define Key_C_F3   0x160
#define Key_C_F4   0x161
#define Key_C_F5   0x162
#define Key_C_F6   0x163
#define Key_C_F7   0x164
#define Key_C_F8   0x165
#define Key_C_F9   0x166
#define Key_C_F10  0x167
#define Key_C_Lft  0x173
#define Key_C_Rgt  0x174
#define Key_C_End  0x175
#define Key_C_PgDn 0x176
#define Key_C_Home 0x177
#define Key_C_PgUp 0x184
#define Key_C_F11  0x189
#define Key_C_F12  0x18a
#define Key_C_Up   0x18d
#define Key_C_Dwn  0x191
#define Key_C_Ins  0x192
#define Key_C_Del  0x193
#define Key_C_Tab  0x194
#define Key_C_2    0x103
#define Key_C_A    0x0001
#define Key_C_B    0x0002
#define Key_C_C    0x0003
#define Key_C_D    0x0004
#define Key_C_E    0x0005
#define Key_C_F    0x0006
#define Key_C_G    0x0007
#define Key_C_H    0x0008
#define Key_C_I    0x0009
#define Key_C_Ent  0x000a
#define Key_C_J    0x000a
#define Key_C_K    0x000b
#define Key_C_L    0x000c
#define Key_C_M    0x000d
#define Key_C_N    0x000e
#define Key_C_O    0x000f
#define Key_C_P    0x0010
#define Key_C_Q    0x0011
#define Key_C_R    0x0012
#define Key_C_S    0x0013
#define Key_C_T    0x0014
#define Key_C_U    0x0015
#define Key_C_V    0x0016
#define Key_C_W    0x0017
#define Key_C_X    0x0018
#define Key_C_Y    0x0019
#define Key_C_Z    0x001a
#define Key_C_6    0x000e
#define Key_C_BS   0x007f
#define Key_C_Brk  0x0003
#define Key_S_Tab  0x10f
#define Key_S_F1   0x154
#define Key_S_F2   0x155
#define Key_S_F3   0x156
#define Key_S_F4   0x157
#define Key_S_F5   0x158
#define Key_S_F6   0x159
#define Key_S_F7   0x15a
#define Key_S_F8   0x15b
#define Key_S_F9   0x15c
#define Key_S_F10  0x15d
#define Key_S_F11  0x187
#define Key_S_F12  0x188
#define Key_S_Q    0x0051
#define Key_S_W    0x0057
#define Key_S_E    0x0045
#define Key_S_R    0x0052
#define Key_S_T    0x0054
#define Key_S_Y    0x0059
#define Key_S_U    0x0055
#define Key_S_I    0x0049
#define Key_S_O    0x004f
#define Key_S_P    0x0050
#define Key_S_A    0x0041
#define Key_S_S    0x0053
#define Key_S_D    0x0044
#define Key_S_F    0x0046
#define Key_S_G    0x0047
#define Key_S_H    0x0048
#define Key_S_J    0x004a
#define Key_S_K    0x004b
#define Key_S_L    0x004c
#define Key_S_Z    0x005a
#define Key_S_X    0x0058
#define Key_S_C    0x0043
#define Key_S_V    0x0056
#define Key_S_B    0x0042
#define Key_S_N    0x004e
#define Key_S_M    0x004d
#define Key_A_Esc  0x101
#define Key_A_BS   0x10e
#define Key_A_Q    0x110
#define Key_A_W    0x111
#define Key_A_E    0x112
#define Key_A_R    0x113
#define Key_A_T    0x114
#define Key_A_Y    0x115
#define Key_A_U    0x116
#define Key_A_I    0x117
#define Key_A_O    0x118
#define Key_A_P    0x119
#define Key_A_Ent  0x11c
#define Key_A_A    0x11e
#define Key_A_S    0x11f
#define Key_A_D    0x120
#define Key_A_F    0x121
#define Key_A_G    0x122
#define Key_A_H    0x123
#define Key_A_J    0x124
#define Key_A_K    0x125
#define Key_A_L    0x126
#define Key_A_Z    0x12c
#define Key_A_X    0x12d
#define Key_A_C    0x12e
#define Key_A_V    0x12f
#define Key_A_B    0x130
#define Key_A_N    0x131
#define Key_A_M    0x132
#define Key_A_F1   0x168
#define Key_A_F2   0x169
#define Key_A_F3   0x16a
#define Key_A_F4   0x16b
#define Key_A_F5   0x16c
#define Key_A_F6   0x16d
#define Key_A_F7   0x16e
#define Key_A_F8   0x16f
#define Key_A_F9   0x170
#define Key_A_F10  0x171
#define Key_A_1    0x178
#define Key_A_2    0x179
#define Key_A_3    0x17a
#define Key_A_4    0x17b
#define Key_A_5    0x17c
#define Key_A_6    0x17d
#define Key_A_7    0x17e
#define Key_A_8    0x17f
#define Key_A_9    0x180
#define Key_A_0    0x181
#define Key_A_F11  0x18b
#define Key_A_F12  0x18c
#define Key_F1     0x13b
#define Key_F2     0x13c
#define Key_F3     0x13d
#define Key_F4     0x13e
#define Key_F5     0x13f
#define Key_F6     0x140
#define Key_F7     0x141
#define Key_F8     0x142
#define Key_F9     0x143
#define Key_F10    0x144
#define Key_Home   0x147
#define Key_Up     0x148
#define Key_PgUp   0x149
#define Key_Lft    0x14b
#define Key_Cent   0x14c
#define Key_Rgt    0x14d
#define Key_End    0x14f
#define Key_Dwn    0x150
#define Key_PgDn   0x151
#define Key_Ins    0x152
#define Key_Del    0x153
#define Key_F11    0x185
#define Key_F12    0x186
#define Key_BS     0x0008
#define Key_Tab    0x0009
#define Key_Ent    0x000d
#define Key_Esc    0x001b
#define Key_Dot    0x002e
#define Key_Spc    0x0020
#define Key_Q      0x0071
#define Key_W      0x0077
#define Key_E      0x0065
#define Key_R      0x0072
#define Key_T      0x0074
#define Key_Y      0x0079
#define Key_U      0x0075
#define Key_I      0x0069
#define Key_O      0x006f
#define Key_P      0x0070
#define Key_A      0x0061
#define Key_S      0x0073
#define Key_D      0x0064
#define Key_F      0x0066
#define Key_G      0x0067
#define Key_H      0x0068
#define Key_J      0x006a
#define Key_K      0x006b
#define Key_L      0x006c
#define Key_Z      0x007a
#define Key_X      0x0078
#define Key_C      0x0063
#define Key_V      0x0076
#define Key_B      0x0062
#define Key_N      0x006e
#define Key_M      0x006d

#define MAXSTUFF 10

int keybuf[MAXSTUFF];
static int curkey = 0;

/* Variables to control macro execution. */

static int curmacro = 0;
static int macropos = 0;

int get_idle_key(char allowstuff, int scope);
void stuffkey(int key);
void kbflush(void);

int kbhit(void)
{
#ifndef __BEOS__
    /* not portable to BeOS */

    fd_set select_set;
    struct timeval timeout;
    FD_ZERO(&select_set);
    FD_SET(0, &select_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    select(FD_SETSIZE, &select_set, 0, 0, &timeout);
    return FD_ISSET(0, &select_set);
#else
    int rc;
    nodelay(stdscr, 1);
    rc = getch();
    nodelay(stdscr, 0);
    if (rc == ERR)
    {
        return 0;
    }
    else
    {
        return rc;
    }
#endif
}

int xkbhit(void)
{
    return kbhit();
}

int tgetkey(void)
{
    int ch;

    ch = ERR;

    while (ch == ERR)
    {
        ch = getch();
    }

    switch (ch)
    {
    case KEY_LEFT:
        return Key_Lft;

    case KEY_RIGHT:
        return Key_Rgt;

    case KEY_UP:
        return Key_Up;

    case KEY_DOWN:
        return Key_Dwn;

    case 127:
    case KEY_BACKSPACE:
        return Key_BS;

    case KEY_NPAGE:
        return Key_PgDn;

    case KEY_PPAGE:
        return Key_PgUp;

    case KEY_HOME:
        return Key_Home;

    case KEY_END:
    case KEY_LL:
        return Key_End;

    case KEY_DC:
        return Key_Del;

    case KEY_IC:
        return Key_Ins;

    case KEY_F(1):
        return Key_F1;

    case KEY_F(2):
        return Key_F2;

    case KEY_F(3):
        return Key_F3;

    case KEY_F(4):
        return Key_F4;

    case KEY_F(5):
        return Key_F5;

    case KEY_F(6):
        return Key_F6;

    case KEY_F(7):
        return Key_F7;

    case KEY_F(8):
        return Key_F8;

    case KEY_F(9):
        return Key_F9;

    case KEY_F(10):
        return Key_F10;

    case KEY_F(11):
        return 0x1b;            /* DEC mode ... <grin> */

    case 0x1b:
#ifndef __NetBSD__
        halfdelay(1);
#endif
        ch = getch();

        nocbreak();
        cbreak();
        switch (tolower(ch))
        {
        case ERR:
        case 0x1b:
            return 0x1b;

        case 'a':
            return Key_A_A;

        case 'b':
            return Key_A_B;

        case 'c':
            return Key_A_C;

        case 'd':
            return Key_A_D;

        case 'e':
            return Key_A_E;

        case 'f':
            return Key_A_F;

        case 'g':
            return Key_A_G;

        case 'h':
            return Key_A_H;

        case 'i':
            return Key_A_I;

        case 'j':
            return Key_A_J;

        case 'k':
            return Key_A_K;

        case 'l':
            return Key_A_L;

        case 'm':
            return Key_A_M;

        case 'n':
            return Key_A_N;

        case 'o':
            return Key_A_O;

        case 'p':
            return Key_A_P;

        case 'q':
            return Key_A_Q;

        case 'r':
            return Key_A_R;

        case 's':
            return Key_A_S;

        case 't':
            return Key_A_T;

        case 'u':
            return Key_A_U;

        case 'v':
            return Key_A_V;

        case 'w':
            return Key_A_W;

        case 'x':
            return Key_A_X;

        case 'y':
            return Key_A_Y;

        case 'z':
            return Key_A_Z;

        case '1':
            return Key_F1;

        case '2':
            return Key_F2;

        case '3':
            return Key_F3;

        case '4':
            return Key_F4;

        case '5':
            return Key_F5;

        case '6':
            return Key_F6;

        case '7':
            return Key_F7;

        case '8':
            return Key_F8;

        case '9':
            return Key_F9;

        case '0':
            return Key_F10;

	case 70:
	    return Key_Home;

	case 72:
	    return Key_End;
        }
    }

    return ch;
}

int get_idle_key(char allowstuff, int scope)
{
    int i;

    if (allowstuff && (curkey > 0))
    {
        return keybuf[--curkey];
    }

    if (curmacro != 0)
    {
        i = KeyMacros[curmacro - 1].start[macropos];

        if (++macropos >= KeyMacros[curmacro - 1].len)
        {
            curmacro = 0;
        }

        return i;
    }

    i = tgetkey();

    if (GlobalKeys[i] != 0)
    {
        i = GlobalKeys[i];
    }
    else
    {
        /*
         *  Check other scopes, if command/key is defined: return mapping
         *  Otherwise, we simply return the keycode.
         */

        switch (scope)
        {
        case READERSCOPE:
            if (ReadMessageKeys[i] != 0)
            {
                i = ReadMessageKeys[i];
            }
            break;

        case AREASCOPE:
            if (AreaSelectKeys[i] != 0)
            {
                i = AreaSelectKeys[i];
            }
            break;

        case EDITORSCOPE:
            if (EditorKeys[i] != 0)
            {
                i = EditorKeys[i];
            }
            break;

        case LISTSCOPE:
            if (ListKeys[i] != 0)
            {
                i = ListKeys[i];
            }
            break;
        }
    }

    /* Macro */

    if (i > 512)
    {
        curmacro = (i - 512);

        if (NumMacros < curmacro)
        {
            /* check if it exists.. */

            Message("Invalid macro called!", 10, 0, YES);
        }

        /* start at 1, we'll return 0 now.. */

        macropos = 1;

        i = KeyMacros[curmacro - 1].start[0];

        if (KeyMacros[curmacro - 1].len == 1)
        {
            /* Only 1 char? Stop now! */

            curmacro = 0;
        }
    }

    return i;
}

void stuffkey(int key)
{
    if (curkey <= MAXSTUFF)
    {
        keybuf[curkey++] = key;
    }
}

/* Flush the keyboard buffer */

void kbflush(void)
{
#ifndef __BEOS__
    /* not portable to BeOS */

    while (kbhit())
    {
        get_idle_key(0, GLOBALSCOPE);
    }
#endif
}

void MacroStart(short i)
{
    if (i > 512)                // Macro
    {
        curmacro = (i - 512);
        macropos = 0;           // start at 0, start of macro
    }
    else                        // This is not a macro. Simply stuff this
                                // code
    {
        stuffkey(i);
    }
}
