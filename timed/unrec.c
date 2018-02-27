#include "includes.h"


void unreceive(MMSG * curmsg, MSGA * areahandle, AREA * area)
{
    char temp[100];

    if (MarkReceived(areahandle, area,
                     (dword) MsgGetCurMsg(areahandle), (int)1, 0) == -1)
        Message("Error updating message", -1, 0, YES);

    curmsg->mis.attr1 &= ~aREAD;
    paint_header(curmsg, area->type);
    sprintf(temp, "%s", MakeRep(curmsg, areahandle, area));
#ifndef __SENTER__
    print(0, 26, cfg.col[Cmsglinks], temp);
#else
    print(1, 26, cfg.col[Cmsglinks], temp);
#endif


}
