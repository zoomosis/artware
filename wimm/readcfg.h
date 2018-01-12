/* Prototype */

void GetConfig(char *cfgname);

/* Some variables */

extern NAMELIST          *firstname;
extern AREA              *firstarea;
extern AREALIST          *exclude_first, *force_first;
extern char              LocalArea[100];
extern char              LogFile[100];
extern int               LocalType;
extern int               mode;
extern int               scanfrom;
extern int               markreceived;
extern dword             attr;
extern int               nonotes;
extern int               addareakludge;