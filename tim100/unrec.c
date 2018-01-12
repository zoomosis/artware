#include <stdio.h>
#include <video.h>
#include <scrnutil.h>
#include <msgapi.h>
#include <video.h>

#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "showhdr.h"
#include "message.h"
#include "unrec.h"
#include "showmail.h"
#include "getmsg.h"
#include "readarea.h"

void unreceive(MMSG *curmsg, MSG *areahandle, AREA *area)
{
   char temp[100];

   if(MarkReceived(areahandle, MsgGetCurMsg(areahandle), 1) == -1)
      Message("Error updating message", -1, 0, YES);

   curmsg->hdr.attr &= ~MSGREAD;
   paint_header(&curmsg->hdr, (area->type == NETMAIL), (curmsg->status & PERSONAL));
   sprintf(temp, "%s", MakeRep(curmsg, areahandle));
   print(0,26,cfg.col.msglinks,temp);

}