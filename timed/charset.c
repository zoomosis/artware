#include "includes.h"

static CHARREC CharRec;

static CHARREC ReadRecBuffer[2], WriteRecBuffer[2];
static int LastReadLoaded = 0,  // Whether index 0 or 1 of above was last
    LastWriteLoaded = 0;        // loaded. Used to dump LRU

#define UNKNOWNBUFFER 10

char ReadUnknown[9][UNKNOWNBUFFER];
sword ReadUnknownLevel[UNKNOWNBUFFER];
char WriteUnknown[9][UNKNOWNBUFFER];
//sword WriteUnknownLevel[UNKNOWNBUFFER];

int IsLevelOne(char *s);
sword GetLevel(char *s);


// ====================================================================
// Load a Character Translation Table, for either reading or writing.
// name is the charset name to convert to (writing) or from (reading),
// like ASCII, LATIN-1 or something like that.
//
// Return -1 for error, 0 if not found, 1 if found.
//
// ====================================================================

int LoadCharMap(char *name, sword level, int readmap)
{
    FILE *in;
    char filename[_MAX_PATH];
    CHARIDENTIFY chid;
    int found = 0, i;


    // First check the buffers...
    if (readmap)
    {
        // First check for CharSets that are known to be not present.
        for (i = 0; i < UNKNOWNBUFFER; i++)
        {
            if (strcmpi(ReadUnknown[i], name) == 0
                && ReadUnknownLevel[i] == level)
                return 0;
        }

        if ((strcmpi((const char *)ReadRecBuffer[0].from_set, name) == 0)
            && (ReadRecBuffer[0].level == level))
        {
            memcpy(&CharRec, &ReadRecBuffer[0], sizeof(CHARREC));
            LastReadLoaded = 0;
            return 1;
        }

        if ((strcmpi((const char *)ReadRecBuffer[1].from_set, name) == 0)
            && (ReadRecBuffer[1].level == level))
        {
            memcpy(&CharRec, &ReadRecBuffer[1], sizeof(CHARREC));
            LastReadLoaded = 1;
            return 1;
        }
    }
    else
    {
        // First check for CharSets that are known to be not present.
        for (i = 0; i < UNKNOWNBUFFER; i++)
        {
            if (strcmpi(WriteUnknown[i], name) == 0)
                return 0;
        }

        if (strcmpi(WriteRecBuffer[0].to_set, name) == 0)
//   &&     (WriteRecBuffer[0].level == level) )
        {
            memcpy(&CharRec, &WriteRecBuffer[0], sizeof(CHARREC));
            LastWriteLoaded = 0;
            return 1;
        }

        if (strcmpi(WriteRecBuffer[1].to_set, name) == 0)
//  &&        (WriteRecBuffer[1].level == level) )
        {
            memcpy(&CharRec, &WriteRecBuffer[1], sizeof(CHARREC));
            LastWriteLoaded = 1;
            return 1;
        }
    }

//   Message("DiskRead for CharSet!", -1, 0, YES);

    sprintf(filename, "%s\\%smaps.dat", cfg.homedir,
            readmap ? "read" : "writ");
    if ((in = fopen(filename, "rb")) == NULL)
    {
        sprintf(msg, "Error opening %s!", filename);
        Message(msg, -1, 0, YES);
        return -1;
    }

    if (fread(&chid, sizeof(CHARIDENTIFY), 1, in) != 1)
    {
        sprintf(msg, "Error reading %d!", filename);
        Message(msg, -1, 0, YES);
        fclose(in);
        return -1;
    }

    while (fread(&CharRec, sizeof(CHARREC), 1, in) == 1)
    {
        if (readmap == 1)
        {
            if ((strcmpi((const char *)CharRec.from_set, name) == 0) &&
                (CharRec.level == level))
            {
                found = 1;
                LastReadLoaded = LastReadLoaded ? 0 : 1;
                memcpy(&ReadRecBuffer[LastReadLoaded], &CharRec,
                       sizeof(CHARREC));
                break;
            }
        }
        else                    // writemap
        {
            if (strcmpi(CharRec.to_set, name) == 0)
            {
                found = 1;
                LastWriteLoaded = LastWriteLoaded ? 0 : 1;
                memcpy(&WriteRecBuffer[LastWriteLoaded], &CharRec,
                       sizeof(CHARREC));
                break;
            }
        }
    }

    fclose(in);

    if (!found)
    {
//     sprintf(msg, "Charset %s (level %hd) not found", name, level);
//     Message(msg, -1, 0, YES);

        if (readmap)
        {
            for (i = 0; i < UNKNOWNBUFFER; i++)
            {
                if (ReadUnknown[i][0] == '\0')
                {
                    strcpy(ReadUnknown[i], name);
                    ReadUnknownLevel[i] = level;
                    break;
                }
            }
        }
        else
        {
            for (i = 0; i < UNKNOWNBUFFER; i++)
            {
                if (WriteUnknown[i][0] == '\0')
                {
                    strcpy(WriteUnknown[i], name);
                    break;
                }
            }
        }
    }
//   else
//     {
//     sprintf(msg, "Charset %s (level %hd) loaded", name, level);
//     Message(msg, -1, 0, YES);
//     }

    return found;

}

// ==============================================================
//
// len is input and output: input contains current length, output
// contains the new length.
//
// ==============================================================

char *TranslateChars(char *s, unsigned *len, sword level)
{
    char *out, *outptr;
    int addedlen = 512;
    int nowadded = 0;
    unsigned left = *len, pos;
    int idx;

    if (!s)
        return NULL;

    if (!len || !(*len))        // Empty or zero len. Give something back,
    {                           // to prevent NULL pointers..
        out = mem_malloc(1);
        *out = '\0';
        *len = 0;
        return out;
    }

    out = outptr = mem_malloc(*len + addedlen); // addedlen for extra
                                                // chars á = ss etc

    while (left)
    {
        if (*s <= 127 && level == 2) // Normal char, copy if level 2.
        {
            *outptr++ = *s++;
            left--;
        }
        else                    // Extended char, translate. Or: level 1,
                                // so remap char
        {
            idx = *s;
            if (idx > 127)
                idx -= 128;     // Table runs from 128-255, not 0-255

            // First check for a simple 1-char replacement.
            if (CharRec.lookup_table[idx][0] == 0 ||
                CharRec.lookup_table[idx][0] == 1)
            {
                if (CharRec.lookup_table[idx][1] != 0)
                    *outptr++ = CharRec.lookup_table[idx][1];
                s++;
            }
            else                // not a standard one char replacement..
            {
                // Is it a two char replacement then?
                if (CharRec.lookup_table[idx][0] >= 32 &&
                    CharRec.lookup_table[idx][0] <= 127 &&
                    CharRec.lookup_table[idx][1] >= 32)
                {
                    // We need to replace one char with two chars. First
                    // check if we
                    // have enough extra space to expand it to two chars.
                    if (nowadded >= (addedlen - 2)) // extra also for
                                                    // trailing zero that
                                                    // we add!
                    {
                        pos = outptr - out; // Save position within output 
                                            // block,
                        // realloc might move the block to another
                        // memory location entirely!
                        addedlen += 512;
                        out = mem_realloc(out, *len + addedlen);
                        outptr = out + pos; // Set outptr back where we
                                            // left off.
                    }
                    nowadded++; // Update number of EXTRA chars we added.

                    // Output the two replacing characters.
                    *outptr++ = CharRec.lookup_table[idx][0];
                    *outptr++ = CharRec.lookup_table[idx][1];
                    s++;
                }
                else            // Weird, strange, undefined or whatever,
                                // make it '?'
                {
                    *outptr++ = '?';
                    s++;
                }
            }
            left--;
        }
    }

    *outptr = '\0';             // Make sure it's NULL terminated
    // Now set the new length
    *len = (outptr - out);

    return out;

}

// ==============================================================
// Returns new kludges
//

char *TranslateBlocks(char *destset, RAWBLOCK * blk, char *kludges,
                      MIS * mis)
{
    char add[30];
    char *newkludges;
    char *tmp;
    int kludgelen = 0;

    // Strip old CHRS and CHARSET kludges, add a new (right) one from us.

    if (kludges)
    {
        RemoveFromCtrl(kludges, "CHRS");
        RemoveFromCtrl(kludges, "CHARSET");
        kludgelen = strlen(kludges);
    }

    if (strcmpi(destset, "ASCII") != 0) // Do not add CHRS: ASCII, that's
                                        // nonsense
    {
        if (IsLevelOne(destset))
            sprintf(add, "\01CHRS: %s 1", destset);
        else
            sprintf(add, "\01CHRS: %s 2", destset);
    }
    else
        add[0] = '\0';
    newkludges = mem_calloc(1, kludgelen + strlen(add) + 1);

    if (kludges)
        strcpy(newkludges, kludges);
    strcat(newkludges, add);

    if (strcmpi(destset, LOCALCHARSET) == 0) // This is the same as local, 
                                             // so
        return newkludges;      // no action required, write it!

    if (LoadCharMap(destset, 2, 0) != 1) // 0 == WRITEmap, 2 == level 2
    {
        LoadCharMap("ASCII", 2, 0); // Not found, so map to ASCII
        RemoveFromCtrl(newkludges, "CHRS");
    }

    // Header is first!

    TranslateHeader(mis, 2);

    while (blk)
    {
        if (blk->txt)
        {
            tmp = TranslateChars(blk->txt, &(blk->curlen), 2);
            blk->curend = tmp + blk->curlen;
            if (blk->txt)
                mem_free(blk->txt);
            blk->txt = tmp;
        }
        blk = blk->next;
    }

    return newkludges;
}

// ==============================================================

sword CheckForCharset(char *kludges)
{
    sword level = 0;
    char *s;
    char charset[30] = "";
    int result;

    if (kludges)
    {
        if ((s = GetCtrlToken(kludges, "CHRS")) != NULL)
        {
            if (sscanf(s, "CHRS: %s %hd", charset, &level) != 2)
                level = 0;
            mem_free(s);
        }
        else if ((s = GetCtrlToken(kludges, "CHARSET")) != NULL)
        {
            result = sscanf(s, "CHARSET: %s %hd", charset, &level);
            switch (result)
            {
            case 0:
                level = 0;
                break;
            case 1:
                level = GetLevel(charset);
                break;
            case 2:
                // nothing, level was correctly parsed.
                break;
            }
            mem_free(s);
        }
        else                    // No kludge, take default for read.
        {
            strcpy(charset, custom.csread);
            if (custom.csreadlevel == 0)
                custom.csreadlevel = GetLevel(custom.csread);
            level = custom.csreadlevel;
        }
    }

    if (strcmpi(charset, LOCALCHARSET) == 0)
        level = 0;

    if (strcmpi(charset, "ASCII") == 0)
        level = 0;

    if (level != 0)
    {
        if (LoadCharMap(charset, level, 1) != 1) // 1 == found.
            level = 0;          // Then no translation.
    }

    return level;
}

// ==============================================================

void TranslateHeader(MIS * mis, sword level)
{
    unsigned hdrlen;
    char *hdrbuf;

    hdrlen = strlen(mis->from);
    hdrbuf = TranslateChars(mis->from, &hdrlen, level);
    if (hdrbuf)
    {
        memcpy(mis->from, hdrbuf, min(99, hdrlen));
        mem_free(hdrbuf);
    }

    hdrlen = strlen(mis->to);
    hdrbuf = TranslateChars(mis->to, &hdrlen, level);
    if (hdrbuf)
    {
        memcpy(mis->to, hdrbuf, min(99, hdrlen));
        mem_free(hdrbuf);
    }

    hdrlen = strlen(mis->subj);
    hdrbuf = TranslateChars(mis->subj, &hdrlen, level);
    if (hdrbuf)
    {
        memcpy(mis->subj, hdrbuf, min(99, hdrlen));
        mem_free(hdrbuf);
    }

}

// ==============================================================

void PickCharMap(void)
{
    FILE *in;
    char filename[_MAX_PATH];
    CHARIDENTIFY chid;
    STRINGLIST *strlist = NULL, *current;
    int tot = 0, l = 0, ret;
    char **picklist;


    sprintf(filename, "%s\\readmaps.dat", cfg.homedir);
    if ((in = fopen(filename, "rb")) == NULL)
    {
        sprintf(msg, "Error opening %s!", filename);
        Message(msg, -1, 0, YES);
        return;
    }

    if (fread(&chid, sizeof(CHARIDENTIFY), 1, in) != 1)
    {
        sprintf(msg, "Error reading %d!", filename);
        Message(msg, -1, 0, YES);
        fclose(in);
        return;
    }

    strlist = AddToStringList(strlist, "IBMPC", "2", 0);

    while (fread(&CharRec, sizeof(CHARREC), 1, in) == 1)
    {
        if (strcmpi(CharRec.to_set, "IBMPC") == 0)
        {
            if (CharRec.level == 1)
                strlist =
                    AddToStringList(strlist, (char *)CharRec.from_set, "1",
                                    0);
            else
                strlist =
                    AddToStringList(strlist, (char *)CharRec.from_set, "2",
                                    0);
        }
    }

    fclose(in);

    if (!strlist)
        return;

    for (current = strlist; current; current = current->next)
        tot++;

    picklist = mem_calloc(tot + 1, sizeof(char *));
    for (l = 0, current = strlist; l < tot; l++, current = current->next)
    {
        picklist[l] = (char *)mem_calloc(1, 13);
        sprintf(picklist[l], " %-8.8s %-1.1s ", current->s, current->pw);
    }

    ret = pickone(picklist, 5, 19, 23, 78);

    free_picklist(picklist);

    if (ret != -1)
    {
        for (current = strlist, l = 0; l < ret;
             l++, current = current->next)
            ;                   /* nothing */
        strcpy(custom.csread, current->s);
        custom.csreadlevel = 0;
    }

    FreeStringList(strlist);

}

// ==============================================================

int IsLevelOne(char *s)
{
    STRINGLIST *current;

    for (current = cfg.FirstLevelOne; current; current = current->next)
    {
        if (strcmpi(current->s, s) == 0) // Found it in list, so it's
                                         // level 1!
            return 1;
    }

    return 0;                   // If we got here, we didn't find it in
                                // the list. So it's level 2.
}

// ==============================================================

sword GetLevel(char *s)
{

    if (LoadCharMap(s, 1, 1) == 1) // ret == 1 : found.
        return 1;
    else if (LoadCharMap(s, 2, 1) == 1) // ret == 1 : found.
        return 2;
    else
        return 0;

}

// ==============================================================
