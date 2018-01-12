
int V7init(int syslookup);
int V7close(void);
short int V7search(char *searchitem, int len, ADDRLIST *found);
short int V7curr(ADDRLIST *found);
short int V7next(ADDRLIST *found);
short int V7prev(ADDRLIST *found);
short int V7head(ADDRLIST *found);
short int V7tail(ADDRLIST *found);
short int V7mark(void);
short int V7findmark(ADDRLIST *found);

