
void *xmalloc(unsigned size);
void *xcalloc(unsigned n, unsigned size);
char *xstrdup(char *s);


void *dmalloc(unsigned size, int line, char *file);
void *dcalloc(unsigned n, unsigned size, int line, char *file);
void dfree(void *ptr, int line, char *file);


#ifdef DEBUG
#define xmalloc(a) dmalloc( (a), __LINE__, __FILE__)
#define xcalloc(a,b) dcalloc( (a), (b), __LINE__, __FILE__)
#define free(a) dfree( (a), __LINE__, __FILE__)
#endif


/* #include "mshell.h" */


