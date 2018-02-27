#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <share.h>

#ifndef __WATCOMC__
#include <sys\stat.h>
#endif

#if defined(__386__) || defined(__FLAT__)
typedef unsigned bit;

typedef unsigned char byte;
typedef signed char sbyte;

typedef unsigned short word;
typedef signed short sword;

typedef unsigned int dword;
typedef signed int sdword;

typedef unsigned short ushort;
typedef signed short sshort;

typedef unsigned long ulong;
typedef signed long slong;

#else

typedef unsigned bit;

typedef unsigned char byte;
typedef signed char sbyte;

typedef unsigned int word;
typedef signed int sword;

typedef unsigned long dword;
typedef signed long sdword;

typedef unsigned short ushort;
typedef signed short sshort;

typedef unsigned long ulong;
typedef signed long slong;

#endif

#pragma pack(__push, 1)

typedef struct _diskarg
{

    char string[21];
    word options;
    word where;
    char next;

} DISKARG;

// Defines for 'next' above

#define LAST    0xFF
#define NEXTAND 0x01
#define NEXTOR  0x02


typedef struct _diskbase
{

    char sig[4];
    char version;
    char deleted;
    char thisareatag[60];
    word scope;
    word direction;
    word action;
    char target[120];
    char targetareaname[60];
    char hasargs;

} DISKBASE;

#define FDBVERSION 1

typedef struct
{

    dword crc;                  // CRC on areatag
    long offset;                // Offset of 'base record' for this search 
                                // stuff.

} FINDIDX;

#pragma pack(__pop)

static int idxin, datin, idxout, datout; // File handles for input/output


// Prototypes.

void OpenFindDatabase(void);
void CloseFindDatabase(void);
int GetNextIdx(void);
int ReadBase(void);
void WriteIdx(void);
void WriteBase(void);
void CopyRest(void);


// Some globals to hold te stuff in memory

static DISKBASE diskbase;
static FINDIDX findidx;
static DISKARG diskarg;


// ===========================================================

int main(void)
{
    puts("timEd find database packer, (c) 1994  Gerard van Essen (2:281/527)\n");

    OpenFindDatabase();

    while (GetNextIdx() != -1)
    {
        if (ReadBase() == -1)
            continue;           // Deleted record.
        printf("Area: %-15.15s\r", diskbase.thisareatag);
        fflush(stdout);
        WriteIdx();
        WriteBase();
        if (diskbase.hasargs > 0) // Any arguments to write out?
            CopyRest();
    }

    CloseFindDatabase();

    puts("\n\nDone!");

    return 0;

}

// ============================================================

int GetNextIdx(void)
{
    int result;

    result = read(idxin, &findidx, (unsigned)sizeof(FINDIDX));

    switch (result)
    {
    case (int)sizeof(FINDIDX):
        return 0;               // Read successfully.

    case (int)-1:
        puts("Error reading index!\n");
        exit(254);

    case (int)0:
        return -1;              // End of file reached.

    default:
        printf("Unexpected result from read() [%d]!\n", result);
        exit(254);
    }

    return 0;

}

// ============================================================

int ReadBase(void)
{

    lseek(datin, findidx.offset, SEEK_SET);

    if (read(datin, &diskbase, (unsigned)sizeof(DISKBASE)) !=
        (int)sizeof(DISKBASE))
    {
        puts("Error reading find database!\n");
        exit(254);
    }

    if (strcmp(diskbase.sig, "Art") != 0)
    {
        puts("Invalid signature in find database!\n");
        exit(254);
    }

    if (diskbase.version != FDBVERSION)
    {
        puts("Invalid version number in find database!\n");
        exit(254);
    }

    if (diskbase.deleted == 1)
        return -1;

    return 0;

}

// ============================================================

void WriteIdx(void)
{

    findidx.offset = tell(datout);

    if (write(idxout, &findidx, (unsigned)sizeof(FINDIDX)) !=
        (int)sizeof(FINDIDX))
    {
        puts("Error writing index!\n");
        exit(254);
    }

}

// ============================================================


void WriteBase(void)
{

    if (write(datout, &diskbase, (unsigned)sizeof(DISKBASE)) !=
        (int)sizeof(DISKBASE))
    {
        puts("Error writing find database!\n");
        exit(254);
    }

}

// ============================================================


void CopyRest(void)
{

    do
    {
        if (read(datin, &diskarg, (unsigned)sizeof(DISKARG)) !=
            (int)sizeof(DISKARG))
        {
            puts("Error reading find database!\n");
            exit(254);
        }

        if (write(datout, &diskarg, (unsigned)sizeof(DISKARG)) !=
            (int)sizeof(DISKARG))
        {
            puts("Error writing find database!\n");
            exit(254);
        }

    }
    while (diskarg.next != LAST);

}

// ============================================================


void OpenFindDatabase(void)
{
    if ((idxin =
         sopen("fdb.idx", O_BINARY | O_RDONLY, SH_DENYRW,
               S_IREAD | S_IWRITE)) == -1)
        goto error;

    if ((datin =
         sopen("fdb.dat", O_BINARY | O_RDONLY, SH_DENYRW,
               S_IREAD | S_IWRITE)) == -1)
        goto error;

    if ((idxout =
         sopen("$fdb.idx", O_BINARY | O_CREAT | O_TRUNC | O_WRONLY,
               SH_DENYWR, S_IREAD | S_IWRITE)) == -1)
        goto error;

    if ((datout =
         sopen("$fdb.dat", O_BINARY | O_CREAT | O_TRUNC | O_WRONLY,
               SH_DENYWR, S_IREAD | S_IWRITE)) == -1)
        goto error;

    return;

  error:

    puts("Error opening files!\n");
    exit(254);

}

// ==========================================================

void CloseFindDatabase(void)
{

    close(idxin);
    close(datin);
    close(idxout);
    close(datout);

    if ((unlink("fdb.dat") != 0) || (unlink("fdb.idx") != 0))
    {
        puts("Error deleting files!\n");
        exit(254);
    }

    if ((rename("$fdb.dat", "fdb.dat") != 0) ||
        (rename("$fdb.idx", "fdb.idx") != 0))
    {
        puts("Error renaming files!\n");
        exit(254);
    }

}
