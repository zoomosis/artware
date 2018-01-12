/*
 *  bmg.h ->    Boyer-Moore-Gosper search definitions
 *
 *  see bmg.c for documentation
 */

#define bmgMAXPAT   22      /*  max pattern length  */
typedef struct
    {
    char    delta[256];     /*  ASCII only deltas   */
    char    pat[bmgMAXPAT + 1]; /*  the pattern     */
    char    ignore;         /*  ignore case flag    */
    }
    bmgARG;

void bmgCompile(char *s, bmgARG *pattern, int ignore_case);
char *bmgSearch(char *buffer, int buflen, bmgARG *pattern);
