#include <io.h>

#include "compiler.h"
#include "typedefs.h"

sword far pascal farread(sword handle,byte far *buf,word len)
{
        return((sword) read(handle, buf, len));
}

sword far pascal farwrite(sword handle,byte far *buf,word len)
{
        return((sword) write(handle, buf, len));
}

