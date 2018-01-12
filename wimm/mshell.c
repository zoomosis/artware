/*
  MSHELL.C contains routines to protect the C-programmer from errors
  in calling memory allocation/free routines. The standard library
  calls supported are : malloc, realloc, strdup, free

  The interface header MSHELL.H redfines these standard library
  calls as macros. When your code is compiled, the macros expand
  to calls to this memory shell. This module then calls the system
  memory routines, with additional error checking.

  The allocation routines in this module add a data structure to the
  top of the allocated memory blocks which tag them as legal memory blocks.

  When the free routine is called, the memory block to be freed is
  checked for legality. If the block is not legal, the memory list
  is dumped to stderr and the program is terminated.

  Compilation options :

  MEM_LIST   link all allocated memory blocks onto an internal list.
         the list can be displayed using Mem_Display().
  MEM_WHERE  Save the file/linenumber of allocated blocks in the header
         Compiler must support __FILE__ and __LINE__ preprocessor
         directives and __FILE__ string must have static or global scope

  Pointer Alignment Size
  ----------------------
  Define  ALIGN_SIZE  value :

  processor    minimum size   suggested size
  8088              1                1
  80286         1        2
  80386         1        4
  80386sx       1        2
  68000         2        2
  68020         1        4
  SPARC         8       16
  VAX           1       16

  How to use the MSHELL module
  ----------------------------
  #include "MSHELL.C"     for total compilation, or
  #include "MSHELL.H"  and  -a  compile MSHELL.C and link in your library
                -b  place MSHELL.C in your project file

  at the end of your program, call Check_Mem (FILE *fp)
  So, it is possible to direct output to screen by
  calling Check_Mem(stdout), or placing  output
  into a file, like this :

  . #include "mshell.h"
  .
  . void main (void)
  . {
  .  FILE *fp;
  .  if ((fp= fopen("MSHELL.ERR", "w+") == NULL) printf("cannot open file\n");
  .
  .       your program
  .
  .  Check_Mem(fp);
  .  fclose(fp);
  . }

*/

#define __MSHELL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "mshell.h"


                /* constants */
#define MEMTAG  0xa55a      /* value for mh_tag  */

                /* structures */
typedef struct memnod       /* memory block header info */
{
   unsigned int   mh_tag;     /* special ident tag */
   size_t     mh_size;    /* size of allocation block */

   struct memnod  *mh_next; /* next memory block */
   struct memnod  *mh_prev; /* previous memory block */

   char          *mh_file; /* file allocation was from */
   unsigned int  mh_line; /* line allocation was from */

} MEMHDR;


/* Alignment macros -- The macro ALIGNE_SIZE  defines size of the largest
   object that must be aligned on your processor. The RESERVE_SIZE macro
   calculates the nearest multiple of ALIGNE_SIZE that is larger that the
   memory block header. Change ALIGN_SIZE to match alignment requirements
   of your processor  (see table above)  */

#define ALIGN_SIZE  sizeof(double)

#define HDR_SIZE        sizeof(MEMHDR)
#define RESERVE_SIZE    (((HDR_SIZE+(ALIGN_SIZE-1))/ALIGN_SIZE) * ALIGN_SIZE)

/* conversion macros-- these macros convert the internal pointer
   to a memory block header to/from the pointer used by the client code */

#define CLIENT_2_HDR(a) ((MEMHDR *) (((char *)(a)) - RESERVE_SIZE))
#define HDR_2_CLIENT(a) ((void *) (((char *)(a)) + RESERVE_SIZE))

                    /* local variables */

static unsigned long mem_size= 0;   /* amount of memory used */

static MEMHDR   *memlist= NULL; /* list of memory blocks */

                    /* local functions */

void    mem_tag_err(void *, char *, int, char *);   /* tag error */


    void mem_list_add(MEMHDR *);        /* add block to list */
    void mem_list_delete(MEMHDR *);     /* delete block from list */
    #define Mem_Tag_Err(a,b) mem_tag_err(a,fil,lin,b)


/* ------------- functions accessed only through macros -------------------*/
/*   mem_alloc()

     usage        :    void *mem_alloc(size_t size)
     parameters   :    size  Size of block in bytes to allocate
     return value :    pointer to allocated block, NULL not enough memory
     description  :    mem_alloc() makes a protected call to malloc()
               this routine is acccessed through the malloc macro.
*/

void *mem_alloc(
      size_t    size,
      char      *fil,
      int       lin
      #else
      size_t    size
   )

{
  MEMHDR    *p;
                  /* allocate memory block */
  p= malloc(RESERVE_SIZE + size);
  if (p== NULL)  return NULL;

                   /* init header */
  p->mh_tag  = MEMTAG;
  p->mh_size = size;
  mem_size  += size;

  p->mh_file = fil;
  p->mh_line = lin;

  mem_list_add(p);

                  /* return pointer to client data */
  return HDR_2_CLIENT(p);
}


/*   mem_realloc()

     usage        :    void *mem_realloc(void *ptr, size_t size)
     parameters   :    ptr -  pointer to current block
               size-  size to adjust block to
     return value :    pointer to new block, NULL if memory cannot be relocated
     description  :    mem_alloc() makes a protected call to realloc().
               this routine is acccessed through the realloc macro.
*/

void *mem_realloc(
     void   *ptr,
     size_t size,
     char   *fil,
     int    lin
   )
{
  MEMHDR    *p;

                  /* convert client pointer to header pointer */
  p= CLIENT_2_HDR(ptr);
                  /* check for valid block */
  if (p->mh_tag != MEMTAG) {
     Mem_Tag_Err(p, "attempt to re-allocate non-existing memory block");
     return NULL;
  }
                  /* invalidate header */
  p->mh_tag = ~MEMTAG;
  mem_size -= p->mh_size;

  mem_list_delete(p);    /* remove block from list */

                 /* reallocate memory block */
  p = (MEMHDR *) realloc(p, RESERVE_SIZE + size);
  if (p== NULL) return NULL;
                 /* update header */

  p->mh_tag  = MEMTAG;
  p->mh_size = size;
  mem_size  += size;

  p->mh_file  = fil;
  p->mh_line  = lin;


  mem_list_add(p);      /* add block to list */

                /* return pointer to client data */

  return HDR_2_CLIENT(p);
}



/*  mem_strdup()

     usage        :    char *mem_strdup(char *str)
     parameters   :    str- string to save
     return value :    pointer to allocated string, NULL if not enough memory
     description  :    saves specified string in dynamic memory
               acces this routines using the strdup() macro

*/

char *mem_strdup(
      char  *str,
      char  *fil,
      int   lin
   )
{

  char *s;

  s= mem_alloc(strlen(str)+1, fil, lin);

  if (s != NULL) strcpy(s, str);
  return s;
}



/*  mem_free()

     usage        :    void mem_free(void *ptr)
     parameters   :    ptr- pointer to memory to free
     return value :    none
     description  :    frees specified memory block. Block must be allocated
               by mem_alloc(), mem_realloc() or mem_strdup()
               access through using free() macro
*/


void mem_free(
     void   *ptr,
     char   *fil,
     int    lin
   )
{
  MEMHDR    *p;

             /* convert client pointer to header pointer */
  p= CLIENT_2_HDR(ptr);
             /* check for valid block */
  if (p->mh_tag != MEMTAG) {
     Mem_Tag_Err(p, "attemp to free non-existing memory block");
     return;
  }
             /* invalidate header */
  p->mh_tag  = ~MEMTAG;
  mem_size  -= p->mh_size;

  mem_list_delete(p);     /* remove block from list */

                    /* free memory block */
  free(p);
}


/* --------------------functions accessed directly ------------------------*/
/*  Mem_Used()

     usage        :    unsigned long Mem_Used (void)
     parameters   :    none
     return value :    number of bytes currently allocated by the
               memory shell. Does not reflect the space used
               by the overhead
*/
unsigned long Mem_Used (void)
{
  return mem_size;
}


/*   Mem_Display()

     usage        :    void Mem_Display (FILE *fp)
     parameters   :    fp- File to output data to
     return value :    none
     description  :    display contents of memory allocation list
               if MEM_LIST is defined.

*/

void Mem_Display (FILE *fp)
{
      MEMHDR    *p;
      int   idx;
     if (mem_size)
        fprintf(fp, "Memory leaks  %lu bytes (memory not freed)\n", mem_size);
     else
        fprintf(fp, "All allocated memory correctly freed.\n");
     fprintf(fp, "\nIndex    Size  File  -  Line - \n");

      idx= 0;
      p = memlist;
      while (p != NULL) {
     fprintf(fp, "%-5d %6u", idx++, p->mh_size);

     fprintf(fp, "   %s line: %d", p->mh_file, p->mh_line);

     if (p->mh_tag != MEMTAG)   fprintf(fp, " Invalid");
     fprintf(fp, "\n");
     p= p->mh_next;
      }
}




void Mem_Check (FILE *fp)
{
 if (Mem_Used() != 0) {
    fprintf(fp,"   Memory allocation error     \n");
    fprintf(fp, "Memory list not empty : \n");
    Mem_Display(fp);
 }
}


/*------------memory list manipulation functions--------------------------*/
/*  mem_list_add() -- add block to list */

static void mem_list_add(MEMHDR *p)
{
  p->mh_next= memlist;
  p->mh_prev= NULL;
  if (memlist != NULL) memlist->mh_prev = p;
  memlist= p;

  #if defined (DEBUG_LIST)
    printf("mem_list_add()\n");
    Mem_Display(stdout);
  #endif
}


/* mem_list_delete()  -- delete block from list */

static void mem_list_delete (MEMHDR *p)
{
  if (p->mh_next != NULL)  p->mh_next->mh_prev = p->mh_prev;

  if (p->mh_prev != NULL)  p->mh_prev->mh_next = p->mh_next;
  else
     memlist = p->mh_next;

  #if defined (DEBUG_LIST)
      printf("mem_list_delete()\n");
      Mem_Display(stdout);
  #endif
}


/* ----------------------error display------------------------------------*/
/* mem_tag_err() --- display memory tag error  */

static void mem_tag_err (void *p, char *fil, int lin, char *errmsg)
{
  _AX= 0x0003;
  geninterrupt(0x10);      /* place in textmode to show error message */

  fprintf(stderr, "Memory tag error - %p in file %s line: %d \n", p, fil, lin);
  fprintf(stderr, "%s \n", errmsg);

  Mem_Display(stderr);
  exit(1);
}

