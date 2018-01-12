#include "includes.h"

int MaySee(MSG *areahandle, dword no)
{
  MSGH *msghandle;
  MIS mis;
  int l;

  if ((msghandle = MsgOpenMsg(areahandle, MOPEN_RDHDR, no)) == NULL)
     {
     showerror();
     return 0;
     }

  if(MsgReadMsg(msghandle, &mis, 0L, 0L, NULL, 0L, NULL) == -1)
     Message("Error reading message in MaySee()!", -1, 0, 1);

  if(MsgCloseMsg(msghandle) == -1)
     Message("Error closing message in MaySee()!", -1, 0, 1);

  // Check private bit.

  if(!(mis.attr1 & aPRIVATE))
    return 1;

  // Check for personal message (from/to you)
  for(l=0; (cfg.usr.name[l].name[0] != '\0') && (l<10); l++)
    {
    if(strcmpi(mis.to, cfg.usr.name[l].name)==0)
      return 1;
    else if(strcmpi(mis.from, cfg.usr.name[l].name)==0)
      return 1;
    }

  return 0;
}

// ==============================================================

int ScanMaySee(AREA *area, MSG *areahandle)
{
   dword uidtoscan;


   if(area->mayseelist)
     DumpMarkList(area->mayseelist);

   area->mayseelist = InitMarkList();

   // Get the lowest UID to start with...
   uidtoscan = MsgMsgnToUid(areahandle, area->lowest);

   while(uidtoscan != 0 && uidtoscan != -1)
     {
     // Check this particular msg, if we may see it then add to the list.
     if(MaySee(areahandle, MsgUidToMsgn(areahandle, uidtoscan, UID_EXACT)))
        AddMarkList(area->mayseelist, uidtoscan);

     // Find out the next message, using UID_NEXT and convert back to uid
     uidtoscan = MsgUidToMsgn(areahandle, uidtoscan+1, UID_NEXT);
     if(uidtoscan)
        uidtoscan = MsgMsgnToUid(areahandle, uidtoscan);
     }

   return 0;

}

// ==============================================================

int BePrivate(AREA * area)
{

   if( (area->type != NEWS && area->type != ECHOMAIL) &&
       (cfg.usr.status & RESPECTPRIVATE) )
      return 1;

   return 0;


}

// ==============================================================

int CurrentIsNotLast(AREA *area, MSG *areahandle)
{
   dword uid;

   if(BePrivate(area))
     {
     uid = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));
     if(NextMarked(area->mayseelist, uid) != 0)
       return 1;
     }
   else
     {
     if(MsgGetCurMsg(areahandle) < MsgGetHighMsg(areahandle))
       return 1;
     }

   return 0;

}
// ==============================================================

dword GetNextPrivate(AREA *area, MSG *areahandle)
{
   dword uid;

   uid = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));
   if(NextMarked(area->mayseelist, uid) != 0)
     return MsgUidToMsgn(areahandle,
                              NextMarked(area->mayseelist, uid), UID_NEXT);

   // If nothing came out of (shouldn't happen!), give some valid number..

   return area->highest;

}

// ==============================================================

dword GetPrevPrivate(AREA *area, MSG *areahandle)
{
   dword uid;

   uid = MsgMsgnToUid(areahandle, MsgGetCurMsg(areahandle));
   if(PrevMarked(area->mayseelist, uid) != 0)
     return MsgUidToMsgn(areahandle,
                              PrevMarked(area->mayseelist, uid), UID_EXACT);

   // If nothing came out of it (shouldn't happen!), give some valid number..

   return area->lowest;

}

// ==============================================================



