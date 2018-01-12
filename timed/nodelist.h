
typedef struct
{

   byte typesearch;     // 0 == name, 1 == node
   byte typenodelist;   // 0 == V7,   1 == Frodo

} NODEHANDLE;


NODEHANDLE * NodeListOpen(int sysop);
short int NodeListSearch(NODEHANDLE *nhandle, NETADDR *addr, char *sysop, int mode, ADDRLIST *found);
short int NodeListCurr(NODEHANDLE *nhandle, ADDRLIST *found);
short int NodeListNext(NODEHANDLE *nhandle, ADDRLIST *found);
short int NodeListPrev(NODEHANDLE *nhandle, ADDRLIST *found);
short int NodeListHead(NODEHANDLE *nhandle, ADDRLIST *found);
short int NodeListTail(NODEHANDLE *nhandle, ADDRLIST *found);
short int NodeListMark(NODEHANDLE *nhandle);
short int NodeListFindMark(NODEHANDLE *nhandle, ADDRLIST *found);
short int NodeListClose(NODEHANDLE *nhandle);

