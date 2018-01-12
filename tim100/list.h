
dword MsgList(MSG *areahandle, AREA *area, dword curno, char wide);


typedef struct _msglist
{

   char              from[21];
   char              to[21];
   char              subj[28];
   int               tagged;
   dword             n;
   struct _msglist   *next;

} MSGLIST;

