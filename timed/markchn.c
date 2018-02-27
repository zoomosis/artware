#include "includes.h"

// Prototypes

void WalkChain(MARKLIST * marklist, MARKLIST * history, MSG * areahandle,
               dword curno);

// ====================================================================
// Mark all messages belonging to a reply chain (tree).
// ====================================================================

void MarkReplyChain(AREA * area, MSG * areahandle, dword startmsg)
{
    MARKLIST *history;

    history = InitMarkList();

    savescreen();

    WalkChain(area->mlist, history, areahandle, startmsg);

    sprintf(msg, " þ %d messages were marked. Press any key.",
            history->active);
    statusbar(msg);
    get_idle_key(1, GLOBALSCOPE);

    putscreen();

    DumpMarkList(history);
}


// ====================================================================
// Walk element by element, recursing calls for every forward and backward
// link...
// ====================================================================

void WalkChain(MARKLIST * marklist, MARKLIST * history, MSG * areahandle,
               dword curno)
{
    MSGH *msghandle;
    dword number;
    MIS *mis;
    int i;

    // Prevent lockups dus to lack of stackspace, because this is a
    // recursive
    // function!

    if (stackavail() < 1500)
        return;

    if (IsMarked(history, curno)) // We've been here before...
        return;

    if ((number = MsgUidToMsgn(areahandle, curno, UID_EXACT)) == 0)
        return;

    if ((msghandle = MsgOpenMsg(areahandle, MOPEN_READ, number)) == NULL)
    {
        sprintf(msg, "Cannot open msg#%lu!", number);
        Message(msg, -1, 0, YES);
        return;
    }

    sprintf(msg, "Marking message #%lu..", number);
    statusbar(msg);

    AddMarkList(marklist, curno); // Mark it.
    AddMarkList(history, curno); // And add it to history list.

    mis = mem_calloc(1, sizeof(MIS));
    if (MsgReadMsg(msghandle, mis, 0L, 0L, NULL, 0L, NULL) == -1)
    {
        sprintf(msg, "Error reading msg#%lu!", number);
        Message(msg, -1, 0, YES);
        mem_free(mis);
        MsgCloseMsg(msghandle);
        return;
    }

    MsgCloseMsg(msghandle);

    if (mis->replyto != 0L)
        WalkChain(marklist, history, areahandle, mis->replyto);

    if (mis->nextreply != 0L)
        WalkChain(marklist, history, areahandle, mis->nextreply);

    for (i = 0; i < 9; i++)
    {
        if (mis->replies[i] != 0L)
            WalkChain(marklist, history, areahandle, mis->replies[i]);
    }

    FreeMIS(mis);
    mem_free(mis);

}

// ==============
