#ifdef __GCC__

#include <time.h>

long filelength(int handle);
int lock(int handle, unsigned long offset, unsigned long nbytes);
int unlock(int handle, unsigned long offset, unsigned long nbytes);

#define SH_DENYNO 0x0001
#define SH_DENYWR 0x0002
#define SH_DENYRW 0x0004

// Binary on Linux? Where?

#define O_BINARY  0x0000
#define O_TEXT    0x0000

int sopen(const char *filename, int access, int share, int permission);

#define tell(a) 	 lseek(a, 0L, SEEK_CUR)
#define chsize(a, b) 	 ftruncate(a, b)
#define _fsopen(a, b, c) fopen(a, b)

void fnsplit(const char *path, char *drive, char *dir, char *name,
             char *ext);

#define _splitpath(a, b, c, d, e)	fnsplit(a, b, c, d, e)

#define MAXEXT 20
#define MAXFILE 255
#define MAXDRIVE 1
#define MAXDIR 255
#define MAXPATH 255
#define _MAX_PATH 255

struct tm *_localtime(const time_t * timer, struct tm *tmbuf);
void _fullpath(char *out, char *in, int maxlen);

char *strupr(char *s);
char *strlwr(char *s);

#endif
