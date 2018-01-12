#include <io.h>
#include <stdio.h>
#include <share.h>
#include <fcntl.h>

#include <msgapi.h>

#include "nstruct.h"
#include "netfuncs.h"


long get_last_sdm(char *dir)
{
	int lrfile;
	long retval;
   dword last = 0L;
	char temp[80];


   sprintf(temp, "%s\\lastread", dir);

   if ((lrfile = sopen(temp, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
      return 0L;

	if(read(lrfile, &last, sizeof(word)) != sizeof(word))
      last = 0L;

   close(lrfile);

   retval = (long) last;

   return retval;

}

// ==============================================================

void put_last_sdm(char *dir, long last)
{
	char temp[80];
	int lrfile;
   word lastint;


   sprintf(temp, "%s\\lastread", dir);

   lastint = (word) last;

   if ((lrfile = sopen(temp, O_CREAT|O_WRONLY|O_TRUNC|O_BINARY, SH_DENYWR, S_IREAD|S_IWRITE)) == -1)
		{
      print_and_log("Can't open lastread file!\n");
      return;
		}

	if (write(lrfile, &lastint, sizeof(word)) != sizeof(word) )
         print_and_log("Can't write! Disk full?\n");

	close(lrfile);

}

// ==============================================================

long AdjustLastread(long MemLastRead, MSG *areahandle)
{
   long newval;

   if(MemLastRead < 2)     // If it's 1 or 0, we always end with 0 anyway..
     return 0L;

   newval = MsgUidToMsgn(areahandle, MemLastRead-1, UID_PREV);

   return (newval == -1) ? 0L : newval;
}

// ==============================================================

long RenumArea(MSG *areahandle, long lr)
{
   long newlr;
   MSGH *msghandle;
   long curno;

   if(MsgGetNumMsg(areahandle) == 0)
     return 0L;  // No msgs? No renumbering, and set lastread to 0!

   print_and_log("Renumbering area..\n");

   // First we need to let the MSGAPI 'active' message point to the lastread
   // message, or (if that one doesn't exist anymore) to a message that will
   // have to be the lastread message. So we first fiddle with the lastread.

   // Below, the UID == real number, since we only do this for SDM...

   newlr = MsgUidToMsgn(areahandle, lr, UID_PREV);  // same or prev msg?

   // newlr could be 0.

   // Set API to our lastread, if it's not 0..

   if(newlr != 0)
     {
     if( (msghandle=MsgOpenMsg(areahandle, MOPEN_READ, newlr)) != NULL)
        MsgCloseMsg(msghandle);
     }

   if((curno = (dword) SDMRenumber(areahandle)) == -1)
      print_and_log("There was an error while renumbering!\n");

   if(newlr > 0)
     newlr = curno;

   return newlr;

}

// ==============================================================

