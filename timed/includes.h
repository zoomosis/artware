#include <malloc.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <process.h>
#include <scrnutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <time.h>
#include <share.h>
#include <direct.h>

#ifdef __WATCOMC__

#include <stddef.h>

#ifndef __OS2__
//  #include <graph.h>
//  #define coreleft _memmax
#endif

#endif                          // __WATCOMC__

#include <video.h>

#include <msgapi.h>
#include <progprot.h>

#include "tstruct.h"
#include "tprotos.h"

#ifdef __DEBUGGING__

#define MEM_DEBUG 1

#endif

//#include "mshell.h"

//#include <memchk.h>

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
