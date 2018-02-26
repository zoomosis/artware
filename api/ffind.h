#ifndef __FFIND_H_DEFINED
#define __FFIND_H_DEFINED

#include "compiler.h"
#include "typedefs.h"
#include "stamp.h"
#include "prog.h"

struct _ffind;
typedef struct _ffind FFIND;

  /* Layout of the DOS Disk Transfer Area */

struct _dosdta
{
    char achReserved[21];
    byte bAttr;
    word usTime;
    word usDate;
    dword ulSize;
    char achName[13];
};

unsigned far pascal __dfindfirst(char far * name, unsigned attr,
                                 struct _dosdta far * dta);
unsigned far pascal __dfindnext(struct _dosdta far * dta);


struct _ffind
{
    word usAttr;
    SCOMBO scCdate;
    SCOMBO scAdate;
    SCOMBO scWdate;
    dword ulSize;

    char szName[13];
    struct _dosdta __dta;

#ifdef __FLAT__                 /* OS/2 2.0 or NT */
    unsigned long hdir;
#else
    unsigned short hdir;        /* directory handle from DosFindFirst */
#endif

};

FFIND *_fast FindOpen(char *filespec, unsigned short attribute);
FFIND *_fast FindInfo(char *filespec); /* PLF Thu 10-17-1991 18:03:09 */
int _fast FindNext(FFIND * ff);

void _fast FindClose(FFIND * ff);

#define ATTR_READONLY  0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME    0x08
#define ATTR_SUBDIR    0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_RSVD1     0x40
#define ATTR_RSVD2     0x80

#define MSDOS_READONLY  ATTR_READONLY
#define MSDOS_HIDDEN    ATTR_HIDDEN
#define MSDOS_SYSTEM    ATTR_SYSTEM
#define MSDOS_VOLUME    ATTR_VOLUME
#define MSDOS_SUBDIR    ATTR_SUBDIR
#define MSDOS_ARCHIVE   ATTR_ARCHIVE
#define MSDOS_RSVD1     ATTR_RSVD1
#define MSDOS_RSVD2     ATTR_RSVD2

#endif                          /* __FFIND_H_DEFINED */
