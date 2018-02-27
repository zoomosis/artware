#include "includes.h"


void clean_index(AREA * area);



void clean_index(AREA * area)
{
    int index;
    char idxname[120];


    sprintf(idxname, "%s.sqi", area->dir);

    if ((index = sopen(idxname, O_RDWR | O_BINARY, SH_DENYWR)) == -1)
        return;

    if (chsize
        (index,
         (long)((long)area->highest * (long)sizeof(struct my_idx))) == -1)
        Message("Error cleaning index!", -1, 0, YES);

    close(index);

}
