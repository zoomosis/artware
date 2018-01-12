/*
 *   bmg.c ->    Boyer-Moore-Gosper search routines
 *
 *   Adapted from:
 *   Boyer/Moore/Gosper-assisted 'egrep' search, with delta0 table as in
 *   original paper (CACM, October, 1977).  No delta1 or delta2.  According  
 *   to experiment (Horspool, Soft. Prac. Exp., 1982), delta2 is of minimal
 *   practical value.  However, to improve for worst case input, integrating
 *   the improved Galil strategies (Apostolico/Giancarlo, Siam. J. Comput.,
 *   February 1986) deserves consideration.
 *
 *   James A. Woods                 Copyright (c) 1986
 *   NASA Ames Research Center
 *
 *   29 April 1986    Jeff Mogul  Stanford
 *   Adapted as a set of subroutines for use by a program. No
 *   regular-expression support.
 *
 *   29 August 1986   Frank Whaley    Beyond Words
 *   Trimmed for speed and other dirty tricks.
 */

#include "includes.h"

/*
 *  bmgCompile() -> compile Boyer-Moore delta table
 *
 *  bmgCompile() compiles the delta table for the given search string, and
 *   initializes the search argument structure.  This structure must be
 *   passed to the bmgSearch() function described below.
 */

void bmgCompile(pat, arg, ignore, whole)
    char    *pat;       /*  the pattern         */
    bmgARG  *arg;       /*  argument data       */
    int ignore;         /*  TRUE to ignore case */
    int whole;          /*  Whole words only    */
    {
    int i,          /*  general ctr     */
        patlen;     /*  pattern length  */

    patlen = strlen(pat);

    strcpy(arg->pat, pat);
    if ((arg->ignore = ignore) != 0)
        strlwr(arg->pat);

    arg->wholeword = (char) whole;

    memset(arg->delta, patlen, 256);

    for (i = 0; i < patlen; ++i)
        arg->delta[arg->pat[i]] = patlen - i - 1;

    if (ignore) /*  tweak upper case if ignore on  */
        for (i = 0; i < patlen; ++i)
            arg->delta[toupper(pat[i])] = patlen - i - 1;
    }


/*
 *  bmgSearch() ->  scan for match
 *
 *  bmgSearch() performs a Boyer-Moore-Gosper search of the given buffer
 *   for the string described by the given search argument structure.  The
 *   match action function "action" will be called for each match found.
 *   This function should return non-zero to continue searching, or 0 to
 *   terminate the search.  bmgSearch() returns the total number of
 *   matches found.
 */

char *bmgSearch(buffer, buflen, arg, howstrict)
    char    *buffer;    /*  buffer to search    */
    int buflen;         /*  length of buffer    */
    bmgARG  *arg;       /*  search argument     */
    char howstrict;
    {
    char    *s;     /*  temp ptr for comparisons    */
    int inc,        /*  position increment          */
        k,          /*  current buffer index        */
        patlen;     /*  pattern length              */

    int beginok, endok;

    #define SEPS " .,[]{}:;'`!?<>\\/%\"-=+|$#@~^&_()"

    k = (patlen = strlen(arg->pat)) - 1;

    if (k >= buflen)
        return NULL;

    for (;;)
        {
        while (((inc = arg->delta[buffer[k]]) != 0) &&
            ((k += inc) < buflen))
            ;
        if (k >= buflen)
            break;

        s = buffer + (k++ - (patlen - 1));

        if(!(arg->ignore ? strncmpi(s, arg->pat, patlen) : strncmp(s, arg->pat, patlen)))
          {
          // Match on string! Now check whole words...
          if(arg->wholeword == 0)
             return (s);

          if(
              ((howstrict & NOTFIRST) && (s == buffer)) || // Is begin of buffer?
              ((howstrict & NOTLAST) && (s == (buffer+buflen-patlen))) // end
            )
            /* nothing */ ;
          else
            {
            beginok = endok = 0;
            if(s == buffer || (strchr(SEPS, *(s-1)) != NULL) )
               beginok=1;

            if( ((s+patlen) == (buffer+buflen)) ||
                (strchr(SEPS, *(s+patlen)) != NULL) )
               endok=1;

            if(beginok && endok) return (s);
            }
          }
        }

    return (NULL);
    }
