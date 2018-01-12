#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>

#include <msgapi.h>
#include "nstruct.h"
#include "netfuncs.h"

// ==============================================================

//#undef mem_malloc
//#undef mem_calloc
//#undef mem_strdup
//#undef mem_realloc
//#undef mem_free

// ==============================================================

#if defined(__WATCOMC__) && defined(__DOS__)

dword coreleft(void)
{
   dword dosfree;

   union REGS regs;

   _heapmin();

   regs.h.ah = 0x48;
   regs.w.bx = 0xFFFF;
   int86(0x21, &regs, &regs);
   dosfree = (dword)regs.w.bx * 16L;

   return (dosfree + (dword) _memavl());
}

#endif
#if !defined(__OS2__) && !defined(__NT__) && defined(__FLAT__)

dword coreleft(void)
{

struct meminfo {
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
    memset( &sregs, 0, sizeof(sregs) );
    sregs.es = FP_SEG( &MemInfo );
    regs.x.edi = FP_OFF( &MemInfo );

    int386x( DPMI_INT, &regs, &regs, &sregs );

    return MemInfo.LargestBlockAvail;
 

}
#endif


void * mem_malloc(unsigned n)
{
   void *p;

   if( (p=malloc(n)) == NULL )
     {
     print_and_log("\nAttempt to allocate memory failed (malloc)!\n");
     print_and_log("Amount requested : %u bytes\n", n);
     #ifndef __FLAT__
     printf("Free memory      : %lu bytes\n", coreleft());
     #endif
     print_and_log("Further execution impossible.\n");
     exit(254);
     }

//   memset(p, 0xFF, n);  // Make memory 'dirty'

   return p;
}

// ================================================

void * mem_calloc(unsigned n, unsigned t)
{
   void *p;

   if( (p=calloc(n, t)) == NULL)
     {
     print_and_log("\nAttempt to allocate memory failed (calloc)!\n");
     print_and_log("Amount requested : %u * %u bytes\n", n, t);
     #ifndef __FLAT__
     printf("Free memory      : %lu bytes\n", coreleft());
     #endif
     print_and_log("Further execution impossible.\n");
     exit(254);
     }

   return p;

}

// =====================================================

void * mem_realloc(void * org, unsigned n)
{
   void *p=NULL;

   if(0 == n)  // Free block
     {
     if(NULL != org)
        mem_free(org);
     }
   else
     {
     if( (p = realloc(org, n)) == NULL)
       {
       print_and_log("\nAttempt to re-allocate memory failed (realloc)!\n");
       print_and_log("Amount requested : %u bytes\n", n);
       #ifndef __FLAT__
       printf("Free memory      : %lu bytes\n", coreleft());
       #endif
       print_and_log("Further execution impossible.\n");
       exit(254);
       }
     }

   return p;
}

// ===================================================

char * mem_strdup(char *s)
{
   char *p;

   if( (p=strdup(s)) == NULL)
     {
     print_and_log("\nAttempt to duplicate string failed (strdup)!\n");
     print_and_log("Amount requested : %u bytes\n", strlen(s));
     #ifndef __FLAT__
     printf("Free memory      : %lu bytes\n", coreleft());
     #endif
     print_and_log("Further execution impossible.\n");
     exit(254);
     }

   return p;
}

// ====================================================

void mem_free(void *p)
{

  if(NULL == p)
     {
     print_and_log("Attempt to free a NULL pointer\n");
     return;
     }

//  memset(p, 0xFF, _msize(p));

  free(p);
}

// ==========================================================

