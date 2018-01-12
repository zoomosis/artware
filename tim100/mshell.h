/*
  from Dr. Dobb's Journal
  august 1990 page 24
  by Jim Schimandle

*/

                        /* compilation options */
#define MEM_LIST        /* build internal list */
#define MEM_WHERE       /* keep track of memory block source */

                           /* interface functions */

unsigned long   Mem_Used(void);
void            Mem_Display(FILE *);
void            Mem_Check(FILE *);


            /* interface functions to access only through macros */

#if defined (MEM_WHERE)
    void    *mem_alloc(size_t, char *, int);
    void    *mem_realloc(void *, size_t, char *, int);
    void    *mem_calloc(int, size_t, char *, int);
    void     mem_free(void *, char *, int);
    char    *mem_strdup(char *, char *, int);
#else
    void    *mem_alloc(size_t);
    void    *mem_realloc(void *, size_t);
    void     mem_free(void *);
    char    *mem_strdup(char *);
#endif

                     /* interface macro's */

#if !defined(__MSHELL__)
  #if defined(MEM_WHERE)
    #define malloc(a)      mem_alloc((a), __FILE__, __LINE__)
    #define calloc(a,b)    mem_calloc((a),(b), __FILE__, __LINE__)
    #define realloc(a,b)   mem_realloc((a),(b),__FILE__,__LINE__)
    #define free(a)        mem_free((a),__FILE__, __LINE__)
    #define strdup(a)      mem_strdup((a),__FILE__, __LINE__)
    #define xcalloc(a,b)   mem_calloc((a),(b), __FILE__, __LINE__)
    #define xmalloc(a)     mem_alloc((a), __FILE__, __LINE__)
    #define xrealloc(a,b)  mem_realloc((a),(b),__FILE__,__LINE__)
    #define xstrdup(a)     mem_strdup((a),__FILE__, __LINE__)


  #else
    #define malloc(a)      mem_alloc(a)
    #define realloc(a,b)   mem_realloc((a),(b))
    #define free(a)        mem_free(a)
    #define strdup(a)      mem_strdup(a)
  #endif
#endif
