#define VFN "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:\\._|=+-)(&^%$#@![]{}<>~`0123456789"

void PrintMessage(MMSG *curmsg, AREA *area, int hardcopy);
FILE * OpenExist(char *default_name);
void do_print(FILE *outfile, AREA *area, MMSG *curmsg);

void inst_newcrit(void);
void inst_oldcrit(void);