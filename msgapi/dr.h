
/* For chdir(), getcurdir(), setdisk(), getdisk() etc. calls */

#include "ffind.h"

#ifdef __TURBOC__

#include <dir.h>

#define _A_SUBDIR MSDOS_SUBDIR
#define _A_SYSTEM MSDOS_SYSTEM

#else
#include <direct.h>
#endif


