/*
** DOW.H - day-of-week macro.  From the FidoNet CECHO - by
** Paul Schlyter.
*/
 
/*#ifdef ISO		/* International Monday-Sunday calendars	*/

 #define dow(y,m,d)  \
        ( ( ( 3*(y) - (7*((y)+((m)+9)/12))/4 + (23*(m))/9 + (d) + 2    \
        + (((y)-((m)<3))/100+1) * 3 / 4 - 16 ) % 7 ) )

#else			/* Sunday-Saturday (i.e. U.S.) calendars	*/
*/
 #define dow(y,m,d)  \
        ( ( ( 3*(y) - (7*((y)+((m)+9)/12))/4 + (23*(m))/9 + (d) + 2    \
        + (((y)-((m)<3))/100+1) * 3 / 4 - 15 ) % 7 ) )
/* #endif */
