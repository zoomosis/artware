#ifndef __VERSION_H__
#define __VERSION_H__

#define PROGNAME "timEd"
#define VERSION "1.12"
#define PROGYEAR "2018"

#ifdef DOS
#ifdef __FLAT__
#define PROGSUFFIX "/386"
#else
#define PROGSUFFIX ""
#endif
#endif

#ifdef OS2
#define PROGSUFFIX "/2"
#endif

#ifdef WIN32
#define PROGSUFFIX "/Win32"
#endif

#ifdef UNIX
#if defined(__FreeBSD__)
#define PROGSUFFIX "/FreeBSD"
#elif defined(__NetBSD__)
#define PROGSUFFIX "/NetBSD"
#elif defined(__OpenBSD__)
#define PROGSUFFIX "/OpenBSD"
#elif defined(__linux__)
#define PROGSUFFIX "/Linux"
#elif defined(__BEOS__)
#define PROGSUFFIX "/BeOS"
#elif defined(__CYGWIN__)
#define PROGSUFFIX "/Cygwin"
#elif defined(__APPLE__)
#define PROGSUFFIX "/OSX"
#else
#define PROGSUFFIX "/UNIX"
#endif
#endif

#endif
