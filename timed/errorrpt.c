#include "includes.h"


void showerror(void)
{
   if(msgapierr == 0)
     return;

   sprintf(msg, "Extended info: %s", errmsgs[msgapierr]);

   Message(msg, -1, 0, YES);
}