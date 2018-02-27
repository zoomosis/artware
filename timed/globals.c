#include <stdio.h>
#include <msgapi.h>
#include "tstruct.h"

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
