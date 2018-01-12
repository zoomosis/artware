
XMSG *MakeHeader   (MSG *areahandle, MMSG *curmsg, int reply, AREA *area, UMSGID reply_to_id, char *subject, char *usenet_address);
int  EditHeader    (MSG *areahandle, XMSG *hdr, int address, int aka, int domatch, char *usenet_address, AREA *area);
int  SetAttributes (XMSG *hdr);
void statusbar     (char *s);
int  matchaka      (XMSG *header);