/* Non-DOS systems... */

#ifdef __OS2__
#define __FARCODE__
#define __FARDATA__
#define __LARGE__
#endif

#ifndef __UNIX__

#if defined(__unix__) || \
  defined(__CYGWIN__) || \
  defined(__linux__) || \
  defined(__FreeBSD__) || \
  defined (__BEOS__) || \
  defined(__NetBSD__) || \
  defined(__APPLE__)

#define __UNIX__ 1

#endif
#endif


#ifdef __UNIX__

#include <time.h>

#define __FARCODE__
#define __FARDATA__
#define __LARGE__

long filelength(int handle);
int lock(int handle, unsigned long offset, unsigned long nbytes);
int unlock(int handle, unsigned long offset, unsigned long nbytes);

#define SH_DENYNO 0x0001
#define SH_DENYWR 0x0002
#define SH_DENYRW 0x0004

// Binary on Linux? Where?

#define O_BINARY  0x0000
#define O_TEXT    0x0000

int sopen(const char *filename, int access, int share, int permission);

#define tell(a) 	 lseek(a, 0L, SEEK_CUR)
#define chsize(a, b) 	 ftruncate(a, b)
#define _fsopen(a, b, c) fopen(a, b)

void fnsplit(const char *path, char *drive, char *dir, char *name,
             char *ext);

#define _splitpath(a, b, c, d, e)	fnsplit(a, b, c, d, e)

#define MAXEXT 20
#define MAXFILE 255
#define MAXDRIVE 1
#define MAXDIR MAXFILE
#define MAXPATH MAXDIR
#define _MAX_PATH MAXPATH

struct tm *_localtime(const time_t * timer, struct tm *tmbuf);
void _fullpath(char *out, char *in, int maxlen);

char *strupr(char *s);
char *strlwr(char *s);

#define strnicmp(a, b, c) strncasecmp(a, b, c)
#define strncmpi(a, b, c) strncasecmp(a, b, c)
#define stricmp(a, b)  strcasecmp(a, b)
#define strcmpi(a, b)  strcasecmp(a, b)

#else

/* not __UNIX__ */

#define MAXEXT   _MAX_EXT
#define MAXFILE  _MAX_FNAME
#define MAXDRIVE 1
#define MAXDIR   _MAX_FNAME
#define MAXPATH  _MAX_PATH

#endif


#ifndef __COMPILER_H_DEFINED
#define __COMPILER_H_DEFINED

#ifndef __WATCOMC__             /* WATCOM has both M_I86xxx and
                                   __modeltype__ macros */

#if (defined(M_I86SM) || defined(M_I86MM)) || (defined(M_I86CM) || defined(M_I86LM) || defined(M_I86HM))
#define __MSC__
#endif

#ifdef M_I86SM
#define __SMALL__
#endif

#ifdef M_I86MM
#define __MEDIUM__
#endif

#ifdef M_I86CM
#define __COMPACT__
#endif

#ifdef M_I86LM
#define __LARGE__
#endif

#ifdef M_I86HM
#define __HUGE__
#endif

#endif                          /* ! __WATCOMC__ */


/* Handle 386 "flat" memory model */

#ifdef __FLAT__

  /* Other macros may get defined by braindead compilers */

#ifdef __SMALL__
#undef __SMALL_
#endif

#ifdef __TINY__
#undef __TINY__
#endif

#ifdef __MEDIUM__
#undef __MEDIUM__
#endif

#ifdef __COMPACT__
#undef __COMPACT__
#endif

#ifdef __LARGE__
#undef __LARGE__
#endif

#ifdef __HUGE__
#undef __HUGE__
#endif

  /* Code is really "near", but "far" in this context means that we want *
     a 32 bit ptr (vice 16 bit).  */

#define __FARCODE__
#define __FARDATA__

  /* Everything should be "near" in the flat model */

#ifdef far
#undef far
#endif

#ifdef near
#undef near
#endif

#ifdef huge
#undef huge
#endif

#define far
#define near
#define huge
#endif


#if defined(__SMALL__) || defined(__TINY__)
#define __NEARCODE__
#define __NEARDATA__
#endif

#ifdef __MEDIUM__
#define __FARCODE__
#define __NEARDATA__
#endif

#ifdef __COMPACT__
#define __NEARCODE__
#define __FARDATA__
#endif

#if defined(__LARGE__) || defined(__HUGE__)
#define __FARCODE__
#define __FARDATA__
#endif



//#if !defined(OS_2) && !defined(__MSDOS__)
//  #define __MSDOS__
//#endif

/* Compiler-specific stuff:                                                 *
 *                                                                          *
 *  _stdc - Standard calling sequence.  This should be the type of          *
 *          function required for function ptrs for qsort() et al.          *
 *  _fast - Fastest calling sequence supported.  If the default             *
 *          calling sequence is the fastest, or if your compiler            *
 *          only has one, define this to nothing.                           *
 *  _intr - For defining interrupt functions.  For some idiotic             *
 *          reason, MSC requires that interrupts be declared                *
 *          as "cdecl interrupt", instead of just "interrupt".              */

#if defined(__TURBOC__)

#define _stdc     cdecl
#define _intr     interrupt far
#define _intcast  void (_intr *)()
#define _veccast  _intcast
#define _fast     _fastcall     // pascal
#define _loadds

#define NW(var) (void)var
  /* structs are packed in TC by default, accd to TURBOC.CFG */

#elif defined(__MSC__)

#define _stdc     cdecl
#define _intr     cdecl interrupt far
#define _intcast  void (_intr *)()
#define _veccast  _intcast

#if _MSC_VER >= 600
#define _fast _fastcall
#else
#define _fast pascal
#endif

#pragma pack(1)                 /* Structures should NOT be padded */
#define NW(var)  var = var      /* NW == No Warning */

#elif defined(__WATCOMC__)

#define _stdc
#define _intr     cdecl interrupt __far
#define _intcast  void (_intr *)()
#define _veccast  void (__interrupt __far *)()
#define _fast

//  #define farcalloc hcalloc

#pragma pack(1)                 /* Structures should NOT be padded */
#define NW(var)   (void)var

#elif defined(__GNUC__)

#define _stdc
#define _fast
#define __FLAT__
#define far
#define near
#define huge

#define NW(var)  (void)var

#define pascal

#else
#error Unknown compiler!

#define _stdc
#define _intr     interrupt
#define _intcast  void (_intr *)()
#define _veccast  _intr
#define _fast _fastcall         // was nothing
#define NW(var)   (void)var
#define __MSDOS__
#endif

#endif                          /* ! __COMPILER_H_DEFINED */
