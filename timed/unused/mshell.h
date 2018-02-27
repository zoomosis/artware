/*
  from Dr. Dobb's Journal
  august 1990 page 24
  by Jim Schimandle

*/

                        /* compilation options */
#define MEM_LIST                /* build internal list */
#define MEM_WHERE               /* keep track of memory block source */

                           /* interface functions */

unsigned long Mem_Used(void);
void Mem_Display(FILE *);
void Mem_Check(FILE *);


            /* interface functions to access only through macros */

void *debug_alloc(size_t, char *, int);
void *debug_calloc(size_t n, size_t, char *, int);
void *debug_realloc(void *, size_t, char *, int);
void debug_free(void *, char *, int);
char *debug_strdup(char *, char *, int);

                     /* interface macro's */

#if !defined(__MSHELL__)

#define mem_malloc(a)      debug_alloc((a), __FILE__, __LINE__)
#define mem_calloc(a, b)   debug_calloc((a), (b), __FILE__, __LINE__)
#define xmalloc(a)         debug_alloc((a), __FILE__, __LINE__)
#define xcalloc(a, b)      debug_calloc((a), (b), __FILE__, __LINE__)
#define mem_realloc(a,b)   debug_realloc((a),(b),__FILE__,__LINE__)
#define mem_free(a)        debug_free((a),__FILE__, __LINE__)
#define mem_strdup(a)      debug_strdup((a),__FILE__, __LINE__)

#define malloc(a)      debug_alloc((a), __FILE__, __LINE__)
#define calloc(a, b)   debug_calloc((a), (b), __FILE__, __LINE__)
#define realloc(a,b)   debug_realloc((a),(b),__FILE__,__LINE__)
#define free(a)        debug_free((a),__FILE__, __LINE__)
#define strdup(a)      debug_strdup((a),__FILE__, __LINE__)

#endif
