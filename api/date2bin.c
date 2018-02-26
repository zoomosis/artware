#include <stdio.h>
#include <string.h>
#include <time.h>
#include "prog.h"

static void near StandardDate(union stamp_combo *d_written);

void _fast ASCII_Date_To_Binary(char *msgdate,
                                union stamp_combo *d_written)
{
    char temp[80];

    int dd, yy, mo, hh, mm, ss, x;

    time_t timeval;
    struct tm *tim;

    timeval = time(NULL);
    tim = localtime(&timeval);

    if (*msgdate == '\0')       /* If no date... */
    {
        /* Insert today's date */
        strftime(msgdate, 19, "%d %b %y %H:%M:%S", tim);

        StandardDate(d_written);
        return;
    }

    if (sscanf(msgdate, "%d %s %d %d:%d:%d", &dd, temp, &yy, &hh, &mm, &ss)
        == 6)
        x = 1;
    else if (sscanf(msgdate, "%d %s %d %d:%d", &dd, temp, &yy, &hh, &mm) ==
             5)
    {
        ss = 0;
        x = 1;
    }
    else if (sscanf
             (msgdate, "%*s %d %s %d %d:%d", &dd, temp, &yy, &hh,
              &mm) == 5)
        x = 2;
    else if (sscanf
             (msgdate, "%d/%d/%d %d:%d:%d", &mo, &dd, &yy, &hh, &mm,
              &ss) == 6)
        x = 3;
    else
        x = 0;

    if (x == 0)
    {
        StandardDate(d_written);
        return;
    }

    if (x == 1 || x == 2)       /* Formats one and two have ASCII date, so 
                                   compare to list */
    {
        for (x = 0; x < 12; x++)
        {
            if (eqstri(temp, months_ab[x]))
            {
                d_written->msg_st.date.mo = x + 1;
                break;
            }
        }

        if (x == 12)            /* Invalid month, use January instead. */
            d_written->msg_st.date.mo = 1;
    }
    else
        d_written->msg_st.date.mo = mo; /* Format 3 don't need no ASCII
                                           month */

    /* Use sliding window technique to interprete the year number */

    while (yy <= tim->tm_year - 50)
    {
        yy += 100;
    }

    while (yy > tim->tm_year + 50)
    {
        yy -= 100;
    }

    d_written->msg_st.date.yr = yy - 80;
    d_written->msg_st.date.da = dd;

    d_written->msg_st.time.hh = hh;
    d_written->msg_st.time.mm = mm;
    d_written->msg_st.time.ss = ss >> 1;
}



/* Date couldn't be determined, so set it to Jan 1st, 1980 */

static void near StandardDate(union stamp_combo *d_written)
{
    d_written->msg_st.date.yr = 0;
    d_written->msg_st.date.mo = 1;
    d_written->msg_st.date.da = 1;

    d_written->msg_st.time.hh = 0;
    d_written->msg_st.time.mm = 0;
    d_written->msg_st.time.ss = 0;
}
