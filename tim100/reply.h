
char *MakeKludge   (MMSG *curmsg, XMSG *header, int netmail);
void MakeMessage   (MMSG *curmsg, AREA *area, MSG *areahandle, word reply, UMSGID reply_to_id, char *add_to_top);
void ChangeMessage (MMSG *curmsg, AREA *area, MSG *areahandle, int bodytoo);
int  pascal IsQuote       (char *line);
void MakeText      (MMSG *curmsg);
void ReplyOther    (MMSG *curmsg, AREA *area);
int  check_alias   (XMSG *hdr, char *usenet_address);
char *make_origin  (int aka);
void address_expand(char *line, NETADDR *fa, int aka);
void zonegate       (XMSG *header, char *kludges);
void check_direct   (XMSG *header, char *kludges);
