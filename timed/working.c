#include <stdlib.h>

#include "video.h"
#include "scrnutil.h"


char wchars[] = "|/-\\";


void working(int y, int x, int col)
{
    static int n = 0;

    printc(y, x, col, wchars[n]);

    n = n < 3 ? n + 1 : 0;

}
