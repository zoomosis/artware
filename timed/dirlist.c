#include "includes.h"

void add_file(FILELIST * list, FLIST * thisone);


// Show list of files, allow selection/tagging, return final list
// with (one or more) tagged files.
// Returns NULL if unsuccessful or if user pressed ESC

#ifdef __WATCOMC__

#define MAXDRIVE _MAX_DRIVE
#define MAXDIR   _MAX_DIR
#define MAXFILE  _MAX_FNAME
#define MAXEXT   _MAX_EXT
#define MAXPATH  _MAX_PATH

#endif

char **dirlist(char *filespec, char tagging)
{
#ifndef __WATCOMC__
    struct ffblk ffblk;
#else
    struct find_t ffblk;
#endif

    FLIST *cf;
    FILELIST *list;
    int done, ret, howmany;
    char temp[120],
        drive[MAXDRIVE],
        dir[MAXDIR], file[MAXFILE], ext[MAXEXT], *lastbslash;
    char **filelist;
    char location[MAXPATH];
    unsigned i;

    list = mem_calloc(1, sizeof(FILELIST));

    memset(location, '\0', MAXPATH);
    if (strncmpi(filespec, "\\\\", 2) != 0)
        _fullpath(location, filespec, MAXPATH);
    fnsplit(location, drive, dir, file, ext);

    savescreen();

    while (1)
    {
        howmany = 0;
        memset(list, '\0', sizeof(FILELIST));

        if ((strcmp(file, "*") == 0) && // Add 'updir?'
            (strcmp(ext, ".*") == 0) && (strcmp(dir, "\\") != 0))
        {
            cf = mem_calloc(1, sizeof(FLIST));
            strcpy(cf->name, "\\..");
            cf->size = 0;
            cf->status |= UPDIR;
            add_file(list, cf);
        }

        sprintf(temp, " þ Reading directory..", location);
        printeol(0, 0, cfg.col[Cmsgbar], temp);

#ifndef __WATCOMC__
        done = findfirst(location, &ffblk, FA_DIREC);
#else
        done = _dos_findfirst(location, _A_SUBDIR, &ffblk);
#endif
        while (!done)
        {
            if (!(++howmany % 25))
                working(0, 23, cfg.col[Cmsgbar]);

#ifndef __WATCOMC__
            if (ffblk.ff_name[0] != '.') // We don't want these
#else
            if (ffblk.name[0] != '.') // We don't want these
#endif
            {
                cf = mem_calloc(1, sizeof(FLIST));

#ifndef __WATCOMC__
                strlwr(ffblk.ff_name);

                if (ffblk.ff_attrib & FA_DIREC)
#else
                strlwr(ffblk.name);

                if (ffblk.attrib & _A_SUBDIR)
#endif
                {
#ifndef __WATCOMC__
                    sprintf(cf->name, "\\%-0.89s", ffblk.ff_name);
#else
                    sprintf(cf->name, "\\%-0.89s", ffblk.name);
#endif
                    cf->status |= FDIR;
                }
                else
                {
#ifndef __WATCOMC__
                    strncpy(cf->name, ffblk.ff_name, 89);
#else
                    strncpy(cf->name, ffblk.name, 89);
#endif
                }

#ifndef __WATCOMC__
                cf->size = ffblk.ff_fsize;
#else
                cf->size = ffblk.size;
#endif
                add_file(list, cf);
            }
#ifdef __WATCOMC__
            done = _dos_findnext(&ffblk);
#else
            done = findnext(&ffblk);
#endif
        }

#ifdef __WATCOMC__
        _dos_findclose(&ffblk);
#endif

        if (list->curnum == 0)
        {
            Message("No files found..", -1, 0, YES);
            putscreen();
            return NULL;
        }

        sprintf(temp, " þ Sorting directory..", location);
        printeol(0, 0, cfg.col[Cmsgbar], temp);

        qsort(list->liststart, list->curnum, sizeof(void *), comp_flist);

        sprintf(temp, " þ Path: %s", location);
        printeol(0, 0, cfg.col[Cmsgbar], temp);
        if (tagging)
            statusbar
                ("Press <enter> or <space> to tag, ctrl-<enter> to accept.");
        else
            statusbar("Press <enter> to select.");

        ret = showfiles(list, tagging);

        switch (ret)
        {
        case 0:                // OK
            putscreen();
            sprintf(temp, "%s%s", drive, dir);
            filelist = build_filelist(list, temp);
            free_flist(list);
            mem_free(list);
            return filelist;

        case -1:               // ESC
            free_flist(list);
            mem_free(list);
            putscreen();
            return NULL;

        case 1:                // Change directory
            i = 0;
            while ((i < list->curnum) &&
                   !((list->liststart[i]->status & FTAGGED) &&
                     (list->liststart[i]->status & (FDIR | UPDIR))))
                i++;

            if (list->liststart[i]->status & FDIR) // Go 'down' in
                                                   // directory
            {
                sprintf(temp, "%s%s", dir, list->liststart[i]->name + 1);
                fnmerge(location, drive, temp, file, ext);
            }
            else
            {
                Strip_Trailing(dir, '\\');
                if ((lastbslash = strrchr(dir, '\\')) != NULL)
                    *lastbslash = '\0';
                if (*dir == '\0')
                    strcpy(dir, "\\");
                fnmerge(location, drive, dir, file, ext);
            }

            fnsplit(location, drive, dir, file, ext);

            free_flist(list);
            memset(list, 0, sizeof(FILELIST));
            break;
        }
    }

}

// Show directory listing
// Return values: -1 == ESC, 0 == OK, 1 == DIR change

int showfiles(FILELIST * list, char tagging)
{
    BOX *filebox;

    FLIST *curptr;
    int maxlines = 25, rows = 0, l = 0;
    int curline = 0, start = 0, highlighted = 0, key;
    char temp[81];
    int i, found;
    char movedown = 1;


    rows = list->curnum > maxlines - 4 ? maxlines - 4 : list->curnum;

    filebox =
        initbox(1, 0, rows + 2, 26, cfg.col[Casframe], cfg.col[Castext],
                SINGLE, YES, ' ');
    drawbox(filebox);

    while (1)                   /* Exit is w/ return statement that gives
                                   command back */
    {
        /* display them.. */

        for (l = start; l < start + rows; l++)
        {
            curptr = list->liststart[l];
            if (!(curptr->status & (FDIR | UPDIR)))
                sprintf(temp, "  %-12.12s   %5d K ", curptr->name,
                        curptr->size / 1024);
            else
                sprintf(temp, "  %-12.12s           ", curptr->name,
                        curptr->size / 1024);

            if (curptr->status & FTAGGED)
                temp[0] = 'þ';

            if (l == curline)
            {
                print(l - start + 2, 1, cfg.col[Cashigh], temp);
                highlighted = l;
                MoveXY(2, l - start + 3);
            }
            else
                print(l - start + 2, 1, cfg.col[Castext], temp);
        }

        /* Now check for commands... */

        switch ((key = get_idle_key(1, GLOBALSCOPE)))
        {
        case 328:              /* up */
            movedown = 0;
            if (curline)
            {
                curline--;
                if (curline < start)
                    start = curline;
            }
            else
            {
                curline = list->curnum - 1;
                start = list->curnum - rows;
            }
            break;

        case 336:              /* down */
            movedown = 1;
            if (curline < list->curnum - 1)
            {
                curline++;
                if (curline >= start + rows)
                    start++;
            }
            else
                start = curline = 0;
            break;

        case 327:              /* home */
            curline = start = 0;
            movedown = 1;
            break;

        case 335:              /* end */
            curline = list->curnum - 1;
            start = list->curnum - rows;
            break;

        case 329:              /* page up */

            movedown = 0;
            if (start == 0)
            {
                curline = 0;
                break;
            }

            start = curline - rows;

            if (start < 0)
                start = 0;

            if (curline > start + rows - 1)
                curline = start;

            break;

        case 337:              /* page down */

            movedown = 1;
            if (start == list->curnum - rows)
            {
                curline = list->curnum - 1;
                break;
            }

            start = curline + rows;

            if (start > list->curnum - rows)
                start = list->curnum - rows;

            if (curline < start)
                curline = start;

            break;

        case 27:
            delbox(filebox);
            return -1;

        case 10:               /* ctrl-enter : accept */
            if (tagging)        // This does nothing if not in tagging
                                // mode.
            {
                delbox(filebox);
                return 0;
            }
            // else we intentionally fall through!!

        case 13:               /* <CR> */

            list->liststart[highlighted]->status ^= FTAGGED;

            if (list->liststart[highlighted]->status & (FDIR | UPDIR))
            {
                delbox(filebox);
                return 1;
            }

            if (!tagging)
            {
                delbox(filebox);
                return 0;
            }
            else
            {
                if (movedown)
                    stuffkey(336);
                else
                    stuffkey(328);
            }
            break;

        case 32:               /* Space */

            if (list->liststart[highlighted]->status & (FDIR | UPDIR))
                break;
            if (tagging)
            {
                list->liststart[highlighted]->status ^= FTAGGED;
                if (movedown)
                    stuffkey(336);
                else
                    stuffkey(328);

            }
            break;

        default:

            if (key > 30 && key < 127)
            {
                found = -1;
                for (i = 0; i < list->curnum; i++)
                {
                    if (list->liststart[i]->name[0] == (char)key)
                    {
                        found = i;
                        break;
                    }
                }

                if (found != -1) // Not NULL, so no match found..
                {
                    curline = found;

                    start = found - (rows / 2);
                    if (start < 0)
                        start = 0;

                    if (start > list->curnum - rows)
                        start = list->curnum - rows;
                }

            }
            break;

        }                       /* switch */

    }                           /* while(1) */

}



int comp_flist(const void *one, const void *two)
{
    char temp[250];


    strcpy(temp, ((FLIST *) (*(FLIST **) one))->name);

    if ((((FLIST *) (*(FLIST **) one))->name[0] == '\\') &&
        (((FLIST *) (*(FLIST **) two))->name[0] != '\\'))
        return -1;

    if ((((FLIST *) (*(FLIST **) one))->name[0] != '\\') &&
        (((FLIST *) (*(FLIST **) two))->name[0] == '\\'))
        return 1;

    return (strcmp
            (((FLIST *) (*(FLIST **) one))->name,
             ((FLIST *) (*(FLIST **) two))->name));
}



// Build a list of files that area tagged in the directory listing.

char **build_filelist(FILELIST * list, char *path)
{
    int howmany = 0, top = 10;

    int i = 0;

    char **filelist;
    char temp[MAXPATH];

    filelist = mem_calloc(top, sizeof(char *));

    while (i < list->curnum)
    {
        if (list->liststart[i]->status & FTAGGED)
        {
            if (howmany > (top - 2))
            {
                top += 10;
                filelist = mem_realloc(filelist, top * sizeof(char *));
                if (filelist == NULL)
                    return NULL;
            }

            sprintf(temp, "%s%s", path, list->liststart[i]->name);
            filelist[howmany++] = mem_strdup(temp);
            filelist[howmany] = NULL;
        }
        i++;
    }

//  filelist[howmany] = NULL;
    filelist = mem_realloc(filelist, (howmany + 1) * sizeof(char *));
    if (filelist == NULL)
        return NULL;

    return filelist;

}


// Add a file to a list of files for display.

void add_file(FILELIST * list, FLIST * thisone)
{
    FLIST **tmp;

    // Set up new list if necessary..

    if (list->liststart == NULL)
    {
        list->liststart = mem_calloc(25, sizeof(void *));
        list->max = 10;
        list->curnum = 1;
    }
    else
        list->curnum++;

    if (list->curnum > list->max) // List full, so expand?
    {
        tmp =
            mem_realloc(list->liststart,
                        ((list->max) + 25) * sizeof(void *));
        if (!tmp)
        {
            Message("Out of memory!", -1, 0, YES);
            return;
        }
        list->max += 25;
        list->liststart = tmp;
    }

    list->liststart[list->curnum - 1] = thisone;

}


// List of display files (for dirlist display)..

void free_flist(FILELIST * filelist)
{
    int i;

    if (filelist == NULL)
        return;

    for (i = 0; i < filelist->curnum; i++)
        mem_free(filelist->liststart[i]);

    if (filelist->liststart)
        mem_free(filelist->liststart);

}


// mem_free a list of files.... (that were returned by 'dirlist').

void free_filelist(char **filelist)
{
    int i;

    if (filelist == NULL)
        return;

    for (i = 0; filelist[i] != NULL; i++)
        mem_free(filelist[i]);

    mem_free(filelist);

}
