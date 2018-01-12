#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <share.h>

#include <video.h>
#include <scrnutil.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "idx.h"
#include "message.h"

void clean_index(AREA *area);



void clean_index(AREA *area)
{
   int index;
   char idxname[120];


   sprintf(idxname,  "%s.SQI", area->dir);

   if ( (index = open(idxname, O_RDWR | O_BINARY | SH_DENYNO)) == -1)
      return;

   if(chsize(index, (long) ((long) area->highest * (long) sizeof(struct my_idx))) == -1)
      Message("Error cleaning index!", -1, 0, YES);

   close(index);

}