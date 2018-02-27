
#include "includes.h"

int AttemptLock(MSGA * areahandle)
{

    while (1)
    {
        if (MsgLock(areahandle) == -1)
        {
            Message("Can't lock area! Wait for retry, <ESC> to abort", 10,
                    0, YES);
            if (xkbhit() && (get_idle_key(1, GLOBALSCOPE) == 27))
            {
                showerror();
                return -1;
            }
        }
        else
            break;
    }

    return 0;
}
