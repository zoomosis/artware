#ifndef __VERSION_H__
#define __VERSION_H__

#define PROGNAME "timEd"
#define PROGYEAR "2018"

#if RELEASE
#define VERSION "1.31+"
#else
#define VERSION "1.31.g1+"
#endif

#ifdef __DOS__
#ifdef __FLAT__
#define PROGSUFFIX "/386"
#else
#define PROGSUFFIX ""
#endif
#endif

#ifdef __OS2__
#define PROGSUFFIX "/2"
#endif

#ifdef __NT__
#define PROGSUFFIX "/NT"
#endif

#ifdef __UNIX__
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
