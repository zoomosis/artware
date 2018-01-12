#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgapi.h>
#include "wstruct.h"


long GetLast(AREA *area, MSG *areahandle);


long GetLast(AREA *area, MSG *areahandle)

{
	int lrfile, bytes_read=0, lastint;
	long last=0, lr=0;
	char temp[130];


	sprintf(temp, "%s\\lastread", area->dir);

   if ((lrfile = sopen(temp, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
      return 0;

	bytes_read = read(lrfile, &lastint, sizeof(int));
   close(lrfile);

	if (bytes_read == sizeof(int))
      last = (long)lastint;
   else return 0;

	lr = MsgUidToMsgn(areahandle, last, UID_PREV);

   if (lr == 0)
      {
      lr = MsgUidToMsgn(areahandle, last, UID_NEXT);
      return (lr>0) ? lr-1 : 0;
      }
   else return lr;

}
