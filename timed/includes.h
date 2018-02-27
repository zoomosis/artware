#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "scrnutil.h"
#include "video.h"

#ifdef __WATCOMC__
#include <malloc.h>
#include <conio.h>
#include <dos.h>
#include <stddef.h>
#include <io.h>
#include <process.h>
#include <share.h>
#include <direct.h>
#endif

#ifdef __UNIX__
#include <unistd.h>
#endif

#include <msgapi.h>
#include <progprot.h>

#include "tstruct.h"
#include "tprotos.h"

#ifndef __FLAT__
#ifndef __WATCOMC__
#include "dvaware.h"
#endif
#include <exec.h>
#endif


extern CFG cfg;
extern CUSTOM custom;
extern char msg[80];
extern char myname[25];
extern int showclock;

extern char months_ab[][4];     // From MSGAPI
extern char weekday_ab[][4];    // From MSGAPI

extern sword AreaSelectKeys[256];
extern sword ReadMessageKeys[256];
extern sword EditorKeys[256];
extern sword GlobalKeys[256];
extern sword ListKeys[256];

extern KEYMACRO *KeyMacros;
extern word NumMacros;

extern MENUENTRY ReaderMenu[MAXMENUENTRIES];
extern word NumMenuEntries;
