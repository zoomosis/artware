#include "includes.h"

typedef struct
{

    long offset;
    long bytes;

} HELPIDX;


HELPIDX help[20];



void show_help(int cat)
{
    int helpfile;
    char *helptext, filename[100];
    LINE *firstline, *curline;  // , *last;
    int nolines = 0, maxlen = 0, line;
    BOX *helpbox;
    int clockstate = showclock;

    sprintf(filename, "%s" DIRSEP "timed.hlp", cfg.homedir);

    if ((helpfile = sopen(filename, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD)) == -1)
    {
        sprintf(msg, "Error opening helpfile (%s)!", filename);
        Message(msg, -1, 0, YES);
        return;
    }

    if (read(helpfile, &help, sizeof(help)) != sizeof(help))
    {
        Message("Error reading helpfile!", -1, 0, YES);
        close(helpfile);
        return;
    }

    helptext = mem_calloc(1, (unsigned)help[cat].bytes);

    lseek(helpfile, help[cat].offset, SEEK_SET);
    if (read(helpfile, helptext, (unsigned)help[cat].bytes) !=
        (int)help[cat].bytes)
    {
        Message("Error reading helpfile!", -1, 0, YES);
        close(helpfile);
        return;
    }

    close(helpfile);

    firstline = wraptext(helptext, maxx - 4, 1, 0);
    mem_free(helptext);

    for (curline = firstline; curline; curline = curline->next)
    {
        nolines++;
        maxlen = max(maxlen, curline->len);
    }

    clockoff();
    helpbox = initbox((maxy - nolines - 3) / 2,
                      (maxx - maxlen - 4) / 2,
                      (maxy - nolines - 3) / 2 + nolines + 3,
                      (maxx - maxlen - 4) / 2 + maxlen + 3,
                      cfg.col[Cpopframe], cfg.col[Cpoptext], SINGLE, YES,
                      ' ');
    drawbox(helpbox);
    titlewin(helpbox, TCENTER, "~ Help! ~", cfg.col[Cpoptitle]);

    for (curline = firstline, line = 0; curline;
         curline = curline->next, line++)
        boxwrite(helpbox, line + 1, 1, curline->ls);

    get_idle_key(1, GLOBALSCOPE);
    if (clockstate == 1)
        clockon();

    FreeLines(firstline);

    delbox(helpbox);

}
