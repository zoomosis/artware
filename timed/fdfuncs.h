
int FDinit(int syslookup);
int FDclose(void);
short int FDcurr(ADDRLIST * found);
short int FDhead(ADDRLIST * found);
short int FDtail(ADDRLIST * found);
short int FDmark(void);
short int FDfindmark(ADDRLIST * found);
short int FDsearch_name(char *name, ADDRLIST * found);
short int FDsearch_node(NETADDR * address, ADDRLIST * found);
int get_node(unsigned blockno, void *node, int size);
int FDnextname(void);
int FDnextnode(void);
int FDprevname(void);
int FDprevnode(void);

void FDupper(char *in);
