#define __MSHELL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#include <errno.h>
#include <malloc.h>
#include "mshell.h"


                              /* constants */
#define MEMTAG  0xa55a        /* value for mh_tag  */

                              /* structures */
typedef struct memnod         /* memory block header info */
{
   unsigned int    mh_tag;    /* special ident tag */
   size_t          mh_size;   /* size of allocation block */

   struct memnod * mh_next;   /* next memory block */
   struct memnod * mh_prev;   /* previous memory block */

   char          * mh_file;   /* file allocation was from */
   unsigned int    mh_line;   /* line allocation was from */

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
/*   debug_alloc()

     usage        :    void *debug_alloc(size_t size)
     parameters   :    size  Size of block in bytes to allocate
     return value :    pointer to allocated block, NULL not enough memory
     description  :    debug_alloc() makes a protected call to malloc()
               this routine is acccessed through the malloc macro.
*/

void *debug_alloc(
      size_t    size,
      char      *fil,
      int       lin
   )

{
  MEMHDR    *p;
  char temp[120];


//  sprintf(temp, "Alloc    : %d bytes (%s, line %d)", size, fil, lin);
//  LOG(temp);

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

  memset(HDR_2_CLIENT(p), 0xFF, size);

                  /* return pointer to client data */
  return HDR_2_CLIENT(p);
}


void *debug_calloc(
      size_t    n,
      size_t    size,
      char      *fil,
      int       lin
   )

{
  MEMHDR    *p;
  char temp[120];
                  /* allocate memory block */

//  sprintf(temp, "Calloc   : %d bytes (%s, line %d)", size, fil, lin);
//  LOG(temp);

  p= calloc(n, RESERVE_SIZE + size);
  if (p== NULL)  return NULL;

                   /* init header */
  p->mh_tag  = MEMTAG;
  p->mh_size = size*n;
  mem_size  += (size*n);

  p->mh_file = fil;
  p->mh_line = lin;

  mem_list_add(p);

                  /* return pointer to client data */
  return HDR_2_CLIENT(p);
}


/*   debug_realloc()

     usage        :    void *debug_realloc(void *ptr, size_t size)
     parameters   :    ptr -  pointer to current block
               size-  size to adjust block to
     return value :    pointer to new block, NULL if memory cannot be relocated
     description  :    debug_alloc() makes a protected call to realloc().
               this routine is acccessed through the realloc macro.
*/

void *debug_realloc(
     void   *ptr,
     size_t size,
     char   *fil,
     int    lin
   )
{
  MEMHDR    *p;
  char temp[120];

//  sprintf(temp, "ReAlloc  : %d bytes (%s, line %d)", size, fil, lin);
//  LOG(temp);

  if(!ptr)
     return (debug_alloc(size, fil, lin));

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



/*  debug_strdup()

     usage        :    char *debug_strdup(char *str)
     parameters   :    str- string to save
     return value :    pointer to allocated string, NULL if not enough memory
     description  :    saves specified string in dynamic memory
               acces this routines using the strdup() macro

*/

char *debug_strdup(
      char  *str,
      char  *fil,
      int   lin
   )
{

  char *s;
  char temp[120];

//  sprintf(temp, "StrDup   : (%s, line %d)", fil, lin);
//  LOG(temp);

  s= debug_alloc(strlen(str)+1, fil, lin);

  if (s != NULL) strcpy(s, str);
  return s;
}



/*  debug_free()

     usage        :    void debug_free(void *ptr)
     parameters   :    ptr- pointer to memory to free
     return value :    none
     description  :    frees specified memory block. Block must be allocated
               by debug_alloc(), debug_realloc() or debug_strdup()
               access through using free() macro
*/


void debug_free(
     void   *ptr,
     char   *fil,
     int    lin
   )
{
  MEMHDR    *p;
  char temp[120];
  

//  sprintf(temp, "Free     : (%s, line %d)", fil, lin);
//  LOG(temp);


               /* convert client pointer to header pointer */
  p= CLIENT_2_HDR(ptr);

  memset(ptr, 0xFF, p->mh_size);

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
        fprintf(fp, "Memory leaks  %d bytes (memory not freed)\n", mem_size);
     else
        fprintf(fp, "All allocated memory correctly freed.\n");
     fprintf(fp, "\nIndex    Size  File  -  Line - \n");

      idx= 0;
      p = memlist;
      while (p != NULL)
         {
         if( strcmpi(p->mh_file, "..\\config.c") != 0 &&
             strcmpi(p->mh_file, "..\\macro.c") != 0 )
             {
             fprintf(fp, "%-5d %6u", idx++, p->mh_size);
             fprintf(fp, "   %s line: %d", p->mh_file, p->mh_line);
             if (p->mh_tag != MEMTAG)   fprintf(fp, " Invalid");
             fprintf(fp, "\n");
             }
         p= p->mh_next;
         }

}


// ==============================================================

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


// ==============================================================
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

  fprintf(stderr, "Memory tag error - %p in file %s line: %d \n", p, fil, lin);
  fprintf(stderr, "%s \n", errmsg);

  Mem_Display(stderr);
  exit(1);
}

// ==============================================================

void direct_free(void *s)
{
   free(s);
}

// ==============================================================


