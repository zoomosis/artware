#include "includes.h"

#ifdef __WATCOMC__
#include <malloc.h>
#endif

// ==============================================================

//#undef mem_malloc
//#undef mem_calloc
//#undef mem_strdup
//#undef mem_realloc
//#undef mem_free

// ===============================

#if defined(__WATCOMC__) && defined(__DOS__) && !defined(__FLAT__)

dword coreleft(void)
{
    dword dosfree;

    union REGS regs;

    regs.h.ah = 0x48;
    regs.w.bx = 0xFFFF;
    int86(0x21, &regs, &regs);
    dosfree = (dword) regs.w.bx * 16L;

    return (dosfree + (dword) _memavl());
}

#endif

#if !defined(__OS2__) && !defined(__NT__) && defined(__FLAT__)

dword coreleft(void)
{

    struct meminfo
    {
        unsigned LargestBlockAvail;
        unsigned MaxUnlockedPage;
        unsigned LargestLockablePage;
        unsigned LinAddrSpace;
        unsigned NumFreePagesAvail;
        unsigned NumPhysicalPagesFree;
        unsigned TotalPhysicalPages;
        unsigned FreeLinAddrSpace;
        unsigned SizeOfPageFile;
        unsigned Reserved[3];
    } MemInfo;

#define DPMI_INT	0x31

    union REGS regs;
    struct SREGS sregs;

    regs.x.eax = 0x00000500;
    memset(&sregs, 0, sizeof(sregs));
    sregs.es = FP_SEG(&MemInfo);
    regs.x.edi = FP_OFF(&MemInfo);

    int386x(DPMI_INT, &regs, &regs, &sregs);

    return MemInfo.LargestBlockAvail;


}
#endif

//#if defined(__WATCOMC__) && defined(__FLAT__)

//dword coreleft(void)
//{
//   return (100000L);
//}

//#endif




void *mem_malloc(unsigned n)
{
    void *p;

    if ((p = malloc(n)) == NULL)
    {
        cls();
        MoveXY(1, 1);
        printf("Attempt to allocate memory failed (malloc)!\n");
        printf("Amount requested : %u bytes\n", n);
#if !defined(__OS2__) && !defined(__NT__)
        printf("Free memory      : %lu bytes\n", coreleft());
#endif
        printf
            ("Further execution impossible. Press a key twice to exit\n");
        kbflush();
        get_idle_key(1, GLOBALSCOPE);
        get_idle_key(1, GLOBALSCOPE);
        exit(254);
    }

    memset(p, 0xFF, n);         // Make memory 'dirty'

    return p;
}

// ==========================

void *mem_calloc(unsigned n, unsigned t)
{
    void *p;

    if ((p = calloc(n, t)) == NULL)
    {
        cls();
        MoveXY(1, 1);
        printf("Attempt to allocate memory failed (calloc)!\n");
        printf("Amount requested : %u * %u bytes\n", n, t);
#if !defined(__OS2__) && !defined(__NT__)
        printf("Free memory      : %lu bytes\n", coreleft());
#endif
        printf
            ("Further execution impossible. Press a key twice to exit\n");
        kbflush();
        get_idle_key(1, GLOBALSCOPE);
        get_idle_key(1, GLOBALSCOPE);
        exit(254);
    }

    return p;

}

// ================================

void *mem_realloc(void *org, unsigned n)
{
    void *p = NULL;

    if (0 == n)                 // Free block
    {
        if (NULL != org)
            mem_free(org);
    }
    else
    {
        if ((p = realloc(org, n)) == NULL)
        {
            cls();
            MoveXY(1, 1);
            printf("Attempt to re-allocate memory failed (realloc)!\n");
            printf("Amount requested : %u bytes\n", n);
#if !defined(__OS2__) && !defined(__NT__)
            printf("Free memory      : %lu bytes\n", coreleft());
#endif
            printf
                ("Further execution impossible. Press a key twice to exit\n");
            kbflush();
            get_idle_key(1, GLOBALSCOPE);
            get_idle_key(1, GLOBALSCOPE);
            exit(254);
        }
    }

    return p;
}

// ========================

char *mem_strdup(char *s)
{
    char *p;

    if ((p = strdup(s)) == NULL)
    {
        cls();
        MoveXY(1, 1);
        printf("Attempt to duplicate string failed (strdup)!\n");
        printf("Amount requested : %u bytes\n", strlen(s));
#if !defined(__OS2__) && !defined(__NT__)
        printf("Free memory      : %lu bytes\n", coreleft());
#endif
        printf
            ("Further execution impossible. Press a key twice to exit\n");
        kbflush();
        get_idle_key(1, GLOBALSCOPE);
        get_idle_key(1, GLOBALSCOPE);
        exit(254);
    }

    return p;
}

// =========================

void mem_free(void *p)
{
//   struct heapinfo hi;

    if (NULL == p)
    {
        Message("Attempt to free a NULL pointer!", -1, 0, YES);
        return;
    }

//   ok:

//   #ifdef __WATCOM__
    memset(p, 0xFF, _msize(p));
//   #else
//   memset(p, 0xFF, 1);
//   #endif

    free(p);
}

// =====
