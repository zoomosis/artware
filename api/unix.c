#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>

#include "compiler.h"

// A function to return the size of an open file.

long filelength(int handle)
{
    struct stat mystat;

    fstat(handle, &mystat);

    return (long)mystat.st_size;

}

// Convert a string to all lower case. Return (original) string.

char *strlwr(char *s)
{
    char *s1;

    for (s1 = s; s1 && *s1; s1++)
        *s1 = tolower(*s1);

    return s;

}


// Convert a string to upper case

char *strupr(char *s)
{
    char *s1;

    for (s1 = s; s1 && *s1; s1++)
        *s1 = toupper(*s1);

    return s;

}


// Lock a file. Ignore region supplied for locking, as Linux
// doesn't seem to support locking part of a file.
// Returns 0 on success, -1 on error.

int lock(int handle, unsigned long offset, unsigned long nbytes)
{
    return (flock(handle, LOCK_EX | LOCK_NB));
}

// Unlock a file.

int unlock(int handle, unsigned long offset, unsigned long nbytes)
{
    return (flock(handle, LOCK_UN | LOCK_NB));
}

// Open a file with sharing support. Don't know what to do with
// sharing support here. Ignore for now..

int sopen(const char *filename, int access, int share, int permission)
{
    return (open(filename, access, permission));
}

// Split a path into dir, name, ext. Ignore 'drive' here, by
// making it '\0' always.

void fnsplit(const char *path, char *drive, char *dir, char *name,
             char *ext)
{
    char temp[512];
    char *charptr;

    if (drive)
        *drive = '\0';
    if (dir)
        *dir = '\0';
    if (name)
        *name = '\0';
    if (ext)
        *ext = '\0';

    if (!path)
        return;

    strncpy(temp, path, 511);
    temp[511] = '\0';

    // First we look for an etension 
    charptr = strrchr(temp, '.');
    if (ext)
        strncpy(ext, charptr, MAXEXT);
    if (charptr)
        *charptr = '\0';

    // Then we look for the filename
    charptr = strrchr(temp, '/');
    // If we found a slash, copy the part after the last slash..
    if (name && charptr)
        strncpy(name, charptr, MAXFILE);
    // If we didn't, we just got a filename, no path! Copy and return.
    if (!charptr)
    {
        if (name)
            strncpy(name, temp, MAXFILE);
        return;
    }

    if (charptr)
        *charptr = '\0';

    // Then what's left must be the the dir..

    if (dir)
        strcpy(dir, temp);

}

// A localtime() lookalike, for compatibility with Watcom

struct tm *_localtime(const time_t * timer, struct tm *tmbuf)
{
    struct tm *mytm;

    mytm = localtime(timer);

    memcpy(tmbuf, mytm, sizeof(struct tm));

    return mytm;

}

// Fake the _fullpath function..

void _fullpath(char *out, char *in, int maxlen)
{
    strncpy(out, in, maxlen - 1);
}
