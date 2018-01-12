#include "mstruct.h"


/* defines for 'displaytype' in ShowMsg function */

#define NORMAL_DISPLAY 0
#define SCAN_DISPLAY   1

int ShowMsg(MMSG *msg, AREA *area, MSG *areahandle, int displaytype);
int showbody(MMSG *curmsg, MSG *areahandle, AREA *area, int displaytype);

char *MakeT(struct _stamp *t);
char *MakeRep  (MMSG *msg, MSG *areahandle);
