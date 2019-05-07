#include <stdio.h>
#include <msgapi.h>
#include "tstruct.h"
#include "unused.h"

CFG cfg;                        // Holds the config
CUSTOM custom;                  // Custom area configuration
char msg[80];                   // For the Message() function
int showclock = 0;

char myname[25] = PROGNAME PROGSUFFIX " " VERSION;

sword AreaSelectKeys[512];
sword ReadMessageKeys[512];
sword EditorKeys[512];
sword GlobalKeys[512];
sword ListKeys[512];

KEYMACRO *KeyMacros;
word NumMacros = 0;

MENUENTRY ReaderMenu[MAXMENUENTRIES];
word NumMenuEntries = 0;

#ifdef __UNIX__

char *itoa(int i, char *a, int n)
{                                              
    unused(n);                                 
    sprintf(a, "%d", i);
    return a;
}                                              

char *ltoa(long i, char *a, int n)
{   
    unused(n);
    sprintf(a, "%ld", i);
    return a;
}

int fnmerge(char *fname, char *drive, char *dir, char *name, char *ext)
{   
    unused(drive);
    sprintf(fname, "%s" DIRSEP "%s%s", dir, name, ext);
    return 1;
}

char *strrev(char *txt)
{
    char *buf = strdup(txt);
    char *h = buf;
    char *h2 = txt;

    while (*h)
    {
        h++;
    }
                                                     
    h--;

    while (h >= buf)
    {
        *h2 = *h;

        h2++;
        h--;
    }
                                                     
    free(buf);
    return txt;
}

#endif

