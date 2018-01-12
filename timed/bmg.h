/*
 *  bmg.h ->    Boyer-Moore-Gosper search definitions
 *
 *  see bmg.c for documentation
 */

#define bmgMAXPAT   22      /*  max pattern length  */
typedef struct
    {
    char    delta[256];         /*  ASCII only deltas      */
    char    pat[bmgMAXPAT + 1]; /*  the pattern            */
    char    ignore;             /*  ignore case flag       */
    char    wholeword;          /*  match whole words only */
    }
    bmgARG;

void bmgCompile(char *s, bmgARG *pattern, int ignore_case, int whole);
char *bmgSearch(char *buffer, int buflen, bmgARG *pattern, char howstrict);

// defines for 'howstrict' above..

#define ALWAYS    0x00
#define NOTFIRST  0x01
#define NOTLAST   0x02
