#include "includes.h"
#include "commcrc.h"


#define NUMSPECIALKEYS 28

char *specialnames[] = {

    "del",
    "ins",
    "home",
    "end",
    "pageup",
    "pagedown",
    "cursorup",
    "cursordown",
    "cursorleft",
    "cursorright",
    "esc",
    "backspace",
    "tab",
    "enter",
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    "f9",
    "f10",
    "f11",
    "f12",
    "space",
    "greyminus"
};

int altspecialcodes[] = {

    419,
    418,
    0,
    415,
    409,
    417,
    408,
    416,
    411,
    413,
    0,
    270,
    421,
    284,
    360,
    361,
    362,
    363,
    364,
    365,
    366,
    367,
    368,
    369,
    395,
    396,
    0,
    330
};

int ctrlspecialcodes[] = {

    403,
    402,
    375,
    373,
    388,
    374,
    397,
    401,
    371,
    372,
    0,
    127,
    409,
    10,
    350,
    351,
    352,
    353,
    354,
    355,
    356,
    357,
    358,
    359,
    393,
    394,
    0,
    398
};


int shiftspecialcodes[] = {

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    271,
    13,
    340,
    341,
    342,
    343,
    344,
    345,
    346,
    347,
    348,
    349,
    391,
    392,
    0,
    45
};



int specialcodes[] = {

    339,
    338,
    327,
    335,
    329,
    337,
    328,
    336,
    331,
    333,
    27,
    8,
    9,
    13,
    315,
    316,
    317,
    318,
    319,
    320,
    321,
    322,
    323,
    324,
    389,
    390,
    32,
    45
};

int AltKeys[] = {

    286,
    304,
    302,
    288,
    274,
    289,
    290,
    291,
    279,
    292,
    293,
    294,
    306,
    305,
    280,
    281,
    272,
    275,
    287,
    276,
    278,
    303,
    273,
    301,
    277,
    300
};


int AltNumber[] = {

    385,
    376,
    377,
    378,
    379,
    380,
    381,
    382,
    383,
    384,

};

// =============================================================

typedef struct
{

    sword command;
    dword crc;

} COMMANDCRC;

#define NUMCOMMANDS 153

COMMANDCRC cTable[] = {

    {cAStag, crcASTAG},
    {cASsetview, crcASSETVIEW},
    {cASup, crcASUP},
    {cASdown, crcASDOWN},
    {cAShome, crcASHOME},
    {cASend, crcASEND},
    {cASpageup, crcASPAGEUP},
    {cASpagedown, crcASPAGEDOWN},
    {cASshell, crcASSHELL},
    {cASscanthorough, crcASSCANTHOROUGH},
    {cASscan, crcASSCAN},
    {cASscanpersonal, crcASSCANPERSONAL},
    {cAStagsetwrite, crcASTAGSETWRITE},
    {cAStagsetread, crcASTAGSETREAD},
    {cASexit, crcASEXIT},
    {cASlist, crcASLIST},
    {cASenter, crcASENTER},
    {cASquit, crcASQUIT},
    {cAShelp, crcASHELP},
    {cEDITbegline, crcEDITBEGLINE},
    {cEDITendline, crcEDITENDLINE},
    {cEDITbegtext, crcEDITBEGTEXT},
    {cEDITendtext, crcEDITENDTEXT},
    {cEDITbegpage, crcEDITBEGPAGE},
    {cEDITendpage, crcEDITENDPAGE},
    {cEDITup, crcEDITUP},
    {cEDITdown, crcEDITDOWN},
    {cEDITright, crcEDITRIGHT},
    {cEDITleft, crcEDITLEFT},
    {cEDITenter, crcEDITENTER},
    {cEDITdel, crcEDITDEL},
    {cEDITback, crcEDITBACK},
    {cEDITtab, crcEDITTAB},
    {cEDITbacktab, crcEDITBACKTAB},
    {cEDITpageup, crcEDITPAGEUP},
    {cEDITpagedown, crcEDITPAGEDOWN},
    {cEDITtoggleinsert, crcEDITTOGGLEINSERT},
    {cEDITtogglehcr, crcEDITTOGGLEHCR},
    {cEDITabort, crcEDITABORT},
    {cEDITsave, crcEDITSAVE},
    {cEDITshell, crcEDITSHELL},
    {cEDITimportfile, crcEDITIMPORTFILE},
    {cEDITdeltoeol, crcEDITDELTOEOL},
    {cEDITmarkblock, crcEDITMARKBLOCK},
    {cEDITunmarkblock, crcEDITUNMARKBLOCK},
    {cEDITdelblock, crcEDITDELBLOCK},
    {cEDITcopyblock, crcEDITCOPYBLOCK},
    {cEDITmoveblock, crcEDITMOVEBLOCK},
    {cEDITzapquotes, crcEDITZAPQUOTES},
    {cEDITdupline, crcEDITDUPLINE},
    {cEDITeditheader, crcEDITEDITHEADER},
    {cEDITdelwordright, crcEDITDELWORDRIGHT},
    {cEDITdelwordleft, crcEDITDELWORDLEFT},
    {cEDITunerase, crcEDITUNERASE},
    {cEDITjumpwordright, crcEDITJUMPWORDRIGHT},
    {cEDITjumpwordleft, crcEDITJUMPWORDLEFT},
    {cEDITdelline, crcEDITDELLINE},
    {cEDIThelp, crcEDITHELP},
    {cEDITsavemenu, crcEDITSAVEMENU},
    {cEDITrunexternal, crcEDITRUNEXTERNAL},
    {cEDITwriteraw, crcEDITWRITERAW},
    {cEDITwritefmt, crcEDITWRITEFMT},
    {cEDITwriterawblock, crcEDITWRITERAWBLOCK},
    {cEDITwritefmtblock, crcEDITWRITEFMTBLOCK},
    {cEDITfiledelete, crcEDITFILEDELETE},

    {cREADup, crcREADUP},
    {cREADdown, crcREADDOWN},
    {cREADbegtext, crcREADBEGTEXT},
    {cREADendtext, crcREADENDTEXT},
    {cREADpageup, crcREADPAGEUP},
    {cREADpagedown, crcREADPAGEDOWN},
    {cREADnext, crcREADNEXT},
    {cREADprevious, crcREADPREVIOUS},
    {cREADfirstmsg, crcREADFIRSTMSG},
    {cREADlastmsg, crcREADLASTMSG},
    {cREADfind, crcREADFIND},
    {cREADlist, crcREADLIST},
    {cREADbroadlist, crcREADBROADLIST},
    {cREADedithello, crcREADEDITHELLO},
    {cREADexit, crcREADEXIT},
    {cREADdelete, crcREADDELETE},
    {cREADwrite, crcREADWRITE},
    {cREADprint, crcREADPRINT},
    {cREADmove, crcREADMOVE},
    {cREADmarkchain, crcREADMARKCHAIN},
    {cREADreply, crcREADREPLY},
    {cREADfollowup, crcREADFOLLOWUP},
    {cREADturboreply, crcREADTURBOREPLY},
    {cREADshell, crcREADSHELL},
    {cREADgoback, crcREADGOBACK},
    {cREADfreqfiles, crcREADFREQFILES},
    {cREADenter, crcREADENTER},
    {cREADreplyother, crcREADREPLYOTHER},
    {cREADchange, crcREADCHANGE},
    {cREADchangeheader, crcREADCHANGEHEADER},
    {cREADchangeattributes, crcREADCHANGEATTRIBUTES},
    {cREADunreceive, crcREADUNRECEIVE},
    {cREADbouncereply, crcREADBOUNCEREPLY},
    {cREADmark, crcREADMARK},
    {cREADinfo, crcREADINFO},
    {cREADchangeaddress, crcREADCHANGEADDRESS},
    {cREADchangename, crcREADCHANGENAME},
    {cREADchangecharset, crcREADCHANGECHARSET},
    {cREADtogglekludges, crcREADTOGGLEKLUDGES},
    {cREADmaintenance, crcREADMAINTENANCE},
    {cREADnextareanewmail, crcREADNEXTAREANEWMAIL},
    {cREADprevareanewmail, crcREADPREVAREANEWMAIL},
    {cREADnextmarked, crcREADNEXTMARKED},
    {cREADprevmarked, crcREADPREVMARKED},
    {cREADnextmsgorpage, crcREADNEXTMSGORPAGE},
    {cREADgotoreply, crcREADGOTOREPLY},
    {cREADgotooriginal, crcREADGOTOORIGINAL},
    {cREADnextreply, crcREADGOTONEXTREPLY},
    {cREADsetbookmark, crcREADSETBOOKMARK},
    {cREADreturnbookmark, crcREADRETURNBOOKMARK},
    {cREADlookuporigaddress, crcREADLOOKUPORIGINADDRESS},
    {cREADlookuptoname, crcREADLOOKUPTONAME},
    {cREADlookupfromname, crcREADLOOKUPFROMNAME},
    {cREADhelp, crcREADHELP},
    {cREADrunexternal, crcREADRUNEXTERNAL},
    {cREADfiltermemory, crcREADFILTERMEMORY},
    {cREADfiltermsg, crcREADFILTERMSG},
    {cREADfilterrealbody, crcREADFILTERREALBODY},
    {cREADwritebody, crcREADWRITEBODY},
    {cREADwriterealbody, crcREADWRITEREALBODY},
    {cREADsqundelete, crcREADSQUNDELETE},
    {cREADsdmrenumber, crcREADSDMRENUMBER},
    {cREADwriteheader, crcREADWRITEHEADER},
    {cREADfiledelete, crcREADFILEDELETE},
    {cREADsearchcurmsg, crcREADSEARCHCURMSG},

    {cLISTabort, crcLISTABORT},
    {cLISTcopy, crcLISTCOPY},
    {cLISTdelete, crcLISTDELETE},
    {cLISTdetails, crcLISTDETAILS},
    {cLISTdown, crcLISTDOWN},
    {cLISTend, crcLISTEND},
    {cLISTexit, crcLISTEXIT},
    {cLISThelp, crcLISTHELP},
    {cLISThome, crcLISTHOME},
    {cLISTmove, crcLISTMOVE},
    {cLISTpagedown, crcLISTPAGEDOWN},
    {cLISTpageup, crcLISTPAGEUP},
    {cLISTprint, crcLISTPRINT},
    {cLISTreadmsg, crcLISTREADMSG},
    {cLISTshell, crcLISTSHELL},
    {cLISTswitch, crcLISTSWITCH},
    {cLISTtag, crcLISTTAG},
    {cLISTtagall, crcLISTTAGALL},
    {cLISTtagrange, crcLISTTAGRANGE},
    {cLISTuntagall, crcLISTUNTAGALL},
    {cLISTuntagrange, crcLISTUNTAGRANGE},
    {cLISTup, crcLISTUP},
    {cLISTwrite, crcLISTWRITE}

};


// =============================================================

int GetSpecialIndex(char *s);
sword GetCommandCode(char *s, int lineno);
sword GetKeyCode(char *s);
void AddToMacro(sword ** s, int *len, int *maxlen, sword code);
int GetNextElement(char *s, sword * code, char **string);
sword MakeNewMacro(sword * newmacro, int len);
char *GetMenuDescription(char **charptr);

// =============================================================

int ReadKeyFile(void)
{
    FILE *keyfile;
    char filename[120];
    char line[2048];
    char *key, *value;
    int scope = GLOBALSCOPE;
    sword keycode, commandcode;
    int lineno = 0;


    if (cfg.homedir[0] != '\0')
        sprintf(filename, "%s" DIRSEP "timkeys.cfg", cfg.homedir);
    else
        strcpy(filename, "timkeys.cfg");

    if ((keyfile = fopen(filename, "r")) == NULL)
        return -1;

    if (strlen(filename) > 25)
    {
        filename[25] = '\0';
        filename[24] = '.';
        filename[23] = '.';
    }
    vprint(12, 42, 7, "(%s)", filename);

    while (fgets(line, sizeof line, keyfile) != NULL)
    {
        StripCR(line);

        if (!(lineno++ % 5))
            working(12, 7, 7);

        if ((key = strtok(line, " \t\r\n")) == NULL)
            continue;

        if (strcmpi(key, "386") == 0 ||
            strcmpi(key, "DOS") == 0 || strcmpi(key, "OS2") == 0)
        {
#ifdef __OS2__

            // OS/2 version
            if (strcmpi(key, "OS2") != 0)
                continue;

#elif defined(__FLAT__)

            // DOS/386 version
            if (strcmpi(key, "386") != 0)
                continue;

#else

            // DOS 16 bit version
            if (strcmpi(key, "DOS") != 0)
                continue;

#endif

            key = strtok(NULL, " \n\r\t"); /* Get real keyword after '386' 
                                              etc */
            if (key == NULL)
                continue;
        }


        if (key[0] == ';')
            continue;
        if (key[0] == '[')
        {
            if (strcmpi(key + 1, "AreaSelection]") == 0)
                scope = AREASCOPE;
            else if (strcmpi(key + 1, "Editor]") == 0)
                scope = EDITORSCOPE;
            else if (strcmpi(key + 1, "MessageReader]") == 0)
                scope = READERSCOPE;
            else if (strcmpi(key + 1, "Global]") == 0)
                scope = GLOBALSCOPE;
            else if (strcmpi(key + 1, "List]") == 0)
                scope = LISTSCOPE;
            else if (strcmpi(key + 1, "Menu]") == 0)
                scope = MENUSCOPE;
            else
            {
                sprintf(msg, "Unknow section: %s (timkeys.cfg, line %d)",
                        key, lineno);
                Message(msg, -1, 0, YES);
            }
            continue;
        }

        if ((value = strtok(NULL, "\r\n")) == NULL)
            continue;

        if ((keycode = GetKeyCode(key)) == -1)
        {
            sprintf(msg, "Unknown key: %s (timkey.cfg, line %d)", key,
                    lineno);
            Message(msg, -1, 0, YES);
            continue;
        }

        if (keycode < 0)
        {
            if (scope != MENUSCOPE)
            {
                sprintf(msg,
                        "Menu item in non-menu section! (timkey.cfg, line %d)",
                        lineno);
                Message(msg, -1, 0, YES);
                continue;
            }
            else                // We have to parse the literal string to
                                // be used as menu entry
            {
                if (NumMenuEntries >= (MAXMENUENTRIES - 1))
                {
                    sprintf(msg, "Can't have more than %d menu entries!",
                            MAXMENUENTRIES);
                    Message(msg, -1, 0, YES);
                    continue;
                }

                ReaderMenu[NumMenuEntries].where = keycode; // Is now
                                                            // MENUJAM or
                                                            // similar.
                ReaderMenu[NumMenuEntries].desc = GetMenuDescription(&value); // Value 
                                                                              // is 
                                                                              // updated

                if (ReaderMenu[NumMenuEntries].desc == NULL)
                {
                    sprintf(msg,
                            "Invalid menu entry in line %d (timkeys.cfg)!",
                            lineno);
                    Message(msg, -1, 0, YES);
                    continue;
                }
            }
        }                       // keycode < 0

        if ((commandcode = GetCommandCode(value, lineno)) == 0)
        {
            sprintf(msg, "Unknown command: %s (timkeys.cfg, line %d)",
                    value, lineno);
            Message(msg, -1, 0, YES);
            continue;
        }

        switch (scope)
        {
        case GLOBALSCOPE:
            GlobalKeys[keycode] = commandcode;
            break;

        case EDITORSCOPE:
            EditorKeys[keycode] = commandcode;
            break;

        case READERSCOPE:
            ReadMessageKeys[keycode] = commandcode;
            break;

        case AREASCOPE:
            AreaSelectKeys[keycode] = commandcode;
            break;

        case LISTSCOPE:
            ListKeys[keycode] = commandcode;
            break;

        case MENUSCOPE:
            ReaderMenu[NumMenuEntries].macro = commandcode;
            NumMenuEntries++;
            break;
        }
    }

    fclose(keyfile);

    print(12, 7, 9, "û");

    return 0;
}

// =============================================================

sword GetCommandCode(char *s, int lineno)
{
    int i;
    sword code = -1, newcode;
    sword *newmacro = NULL;
    int newmaclen = -1;
    int macmaxlen = -1;
    char quotechar;


    while ((i = GetNextElement(s, &newcode, &s)) != -1)
    {
        if (i == 0)             // Command code returned.
        {
            if (newmaclen == -1) // If this is the first element
            {
                code = newcode;
                newmaclen = 0;
            }
            else                // This is not the first element
            {
                if (newmaclen == 0) // There is already one element
                    AddToMacro(&newmacro, &newmaclen, &macmaxlen, code); // Add 
                                                                         // it.
                AddToMacro(&newmacro, &newmaclen, &macmaxlen, newcode); // And 
                                                                        // the 
                                                                        // new 
                                                                        // one.
            }
        }
        else                    // Literal string
                                // -------------------------
        {
            if (newmaclen == 0) // There is already one element
                AddToMacro(&newmacro, &newmaclen, &macmaxlen, code); // Add 
                                                                     // it.

            quotechar = *s++;   // Single or double quotes?
            while (*s && *s != quotechar)
            {
                AddToMacro(&newmacro, &newmaclen, &macmaxlen, (sword) * s); // Add 
                                                                            // it.
                s++;
            }

            if (*s != quotechar) // We didn't end with a quote?
            {
                sprintf(msg,
                        "String without ending quote (timkeys.cfg, line %d)",
                        lineno);
                Message(msg, -1, 0, YES);
                break;
            }
            else
                s++;
        }
    }

    if (newmaclen > 0)          // This is indeed a macro. Work on it.
    {
        code = 513 + MakeNewMacro(newmacro, newmaclen); // First macro ==
                                                        // 513
        mem_free(newmacro);
    }

    return code;

}

// =============================================================

void AddToMacro(sword ** s, int *len, int *maxlen, sword code)
{

    if (*len < 1)               // Set up a new one
    {
        *s = mem_malloc(50 * sizeof(sword));
        *maxlen = 50;
        *len = 0;
    }

    if (*maxlen < (*len + 1))   // Expand?
    {
        *s = mem_realloc(*s, ((*maxlen + 50) * sizeof(sword)));
        *maxlen += 50;
    }

    (*s)[*len] = code;
    (*len)++;

}

// =============================================================
//    Return -1 if no more elements
//    Return  0 if commandcode (returned in 'code')
//    Return  1 if literal string (ptr returned in 'string')
// =============================================================

int GetNextElement(char *s, sword * code, char **string)
{
    char temp[100];
    char *out;
    int len, i;
    dword crc;

    while (isspace(*s))
        s++;                    // Skip leading space

    if (*s == '\0')
        return -1;

    if (*s == '\'' || *s == '"') // Literal string
    {
        *string = s;
        return 1;
    }

    // Copy the command name.

    for (out = temp, len = 0; *s && !isspace(*s) && len < 99; len++)
        *out++ = toupper(*s++);
    *out = '\0';

    crc = JAMsysCrc32(temp, strlen(temp), -1L);

    for (i = 0; i < NUMCOMMANDS; i++)
    {
        if (cTable[i].crc == crc)
        {
            *string = s;
            *code = cTable[i].command;
            return 0;
        }
    }

    // It was not a command. In that case we can try to find the keycode
    // And send that through!

    *code = GetKeyCode(temp);
    if (*code != -1)
    {
        *string = s;
        return 0;               // Yep, it was a normal key! Cool.
    }

    sprintf(msg, "Unknown command: %0.50s!", temp);
    Message(msg, -1, 0, YES);

    return -1;

}

// =============================================================

sword MakeNewMacro(sword * newmacro, int len)
{
    static int maxmacros = 0;

    if (maxmacros < (NumMacros + 1))
    {
        maxmacros += 10;
        KeyMacros = mem_realloc(KeyMacros, maxmacros * sizeof(KEYMACRO));
    }

    KeyMacros[NumMacros].start = mem_malloc(len * sizeof(sword));
    memcpy(KeyMacros[NumMacros].start, newmacro, len * sizeof(sword));
    KeyMacros[NumMacros].len = len;

    NumMacros++;

    return (NumMacros - 1);     // Return index number.
}


// =============================================================

sword GetKeyCode(char *s)
{
    int idx;

    if (!s || *s == '\0')
        return -1;


    if (strlen(s) == 1)         // One character, a simple char
        return (int)(*s);

    // ------- First check for MENUsquish etc. entries.

    if ((*s == 'm' || *s == 'M') && (strncmpi(s, "menu", 4) == 0))
    {
        s += 4;                 // Skip 'menu'

        if (strncmpi(s, "all", 3) == 0)
            return MENUALL;
        if (strncmpi(s, "jam", 3) == 0)
            return MENUJAM;
        if (strncmpi(s, "sdm", 3) == 0)
            return MENUSDM;
        if (strncmpi(s, "hmb", 3) == 0)
            return MENUHMB;
        if (strncmpi(s, "squish", 6) == 0)
            return MENUSQUISH;
    }

    if (*s == '#')              // Shifted key ------------------
    {
        s++;
        if (*s == '\0')
            return -1;

        if (strlen(s) == 1)     // Shifted, one character
            return (int)(toupper(*s)); // Weird, make upper case?!

        idx = GetSpecialIndex(s);
        if (idx == -1)
            return -1;
        return shiftspecialcodes[idx];
    }

    if (*s == '^')              // Control key ------------------
    {
        s++;
        if (*s == '\0')
            return -1;

        if (strlen(s) == 1)     // Ctrl, one character
        {
            *s = toupper(*s);
            if (*s < 'A' || *s > 'Z')
                return -1;
            return ((int)(*s - 'A' + 1));
        }

        idx = GetSpecialIndex(s);
        if (idx == -1)
            return -1;
        return ctrlspecialcodes[idx];
    }

    if (*s == '@')              // ALT key ------------------
    {
        s++;
        if (*s == '\0')
            return -1;

        if (strlen(s) == 1)     // Alt, one character
        {
            if (strchr("0123456789", *s) != NULL) // Alt-'0' .. Alt-'9';
                return AltNumber[*s - '0'];

            if (*s == '+')
                return 334;
            if (*s == '-')
                return 386;
            if (*s == '=')
                return 387;

            *s = toupper(*s);
            if (*s < 'A' || *s > 'Z')
                return -1;
            return ((int)AltKeys[*s - 'A']);
        }

        idx = GetSpecialIndex(s);
        if (idx == -1)
            return -1;
        return altspecialcodes[idx];
    }

    // Not ctrl, shift or alt and not 1 char. So this must be a special
    // name, like <esc> or anything..

    idx = GetSpecialIndex(s);
    if (idx == -1)
        return -1;
    return specialcodes[idx];

}

// ==============================================================

int GetSpecialIndex(char *s)
{
    int i;

    for (i = 0; i < NUMSPECIALKEYS; i++)
    {
        if (strcmpi(s, specialnames[i]) == 0)
            return i;
    }

    return -1;                  // Not found..

}


// ===================================================================
//
//  Get a description from between quotes. Update charptr, so higher
//  level routine can continue parsing the string.
//
// ===================================================================

char *GetMenuDescription(char **charptr)
{
    char *desc;
    char temp[100];
    char *s = *charptr;
    char quotestring;

    while (*s == ' ')
        s++;
    desc = temp;
    if (*s != '\'' && *s != '"')
        return NULL;

    quotestring = *s++;
    while (*s && *s != quotestring)
        *desc++ = *s++;

    if (*s == 0)                // No end of quote found
        return NULL;
    else
        s++;                    // Skip last quotechar

    *desc = '\0';               // NULL terminate temp
    *charptr = s;               // Update passed ptr for higher level
                                // f'ion

    return mem_strdup(temp);

}

// ==============================================================
