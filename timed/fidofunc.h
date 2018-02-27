
int FIDOinit(void);
int FIDOsearch(char *name, ADDRLIST * found);
int FIDOcurr(ADDRLIST * found);
int FIDOnext(ADDRLIST * found);
int FIDOprev(ADDRLIST * found);
int FIDOhead(ADDRLIST * found);
int FIDOtail(ADDRLIST * found);
int FIDOmark(void);
int FIDOfindmark(ADDRLIST * found);
int FIDOclose(void);
