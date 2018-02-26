#include <stdio.h>
#include <time.h>
#include "prog.h"

union stamp_combo *_fast Get_Dos_Date(union stamp_combo *st)
{
    time_t timeval;
    struct tm *tim;

    timeval = time(NULL);
    tim = localtime(&timeval);

    return (TmDate_to_DosDate(tim, st));
}
