#include "includes.h"

#include "fe130.h"
#include "fe141.h"
#include "fe142.h"
#include "gestruct.h"
//#include "im_struc.h"
#include "fmail120.h"
#include "fmail098.h"
#include "xmail.h"
#include "wtrgate.h"
#include "crc.h"


char *xgstr(char *line);

void readconfig(void);
void analyse(char *line, int number);
void parseaddress(char *address);
void addname(char *name);
void addarea(char *value, word type);
char *getquoted(char *line);

AREA *init_area(void);
void check_aka(AREA * current, char *line);
void addmacro(char *name, int usenet);
void startup(char *what);
int one_of_three(char *what);

void AddEchoArea(char *line, int type);
void AddAreasArea(char *line);
void ReadSquishCfg(char *filename);
void ReadAreasBBS(void);
void ReadFECFG(void);
void ReadNewFECFG(int cfgfile);
void ana_FE_area(Area * area, SysAddress * aka_array);
void ana_FE141_area(FE141Area * area, SysAddress * aka_array);
void ana_FE142_area(FENEW_Area * area, FENEW_SysAddress * aka_array);
void ReadGEchoCFG(void);
void ana_GECHO_area(AREAFILE_GE * area, GE_ADDRESS * aka_array);
//void ReadImailCFG(void);
//void ana_Imail_area(areas_record_type *area, naddress *aka_array);
void ReadxMailCFG(void);
void ana_xMail_area(xMailArea * area, word areano);
void ReadFmailCFG(void);
void old_ana_Fmail_area(rawEchoTypeOld * area, nodeFakeType * aka_array);
void ana_Fmail_area(rawEchoType * area, nodeFakeType * aka_array);
void ReadWtrCFG(void);
void ana_wtr_area(WTRAREA * area, WTRAKA * akalist);
void add_to_ptlist(char *tag);
int checkpt(char *tag);
void mem_freeptlist(void);
char *xpstrdup(char *str);
void ReadConfigFile(char *temp, int showname);
void AddLinkedArea(AREA * thisarea);
void startupmode(char *s);
void freqattribs(char *s);
int near AreaCompare(AREA * one, AREA * two);
void uucpaddress(char *address);
void ReadSoup2SQCFG(void);

void akaforce(char *s);
int near filladdress(NETADDR * thisaddress, char *adr);
void addwritename(char *filename);
void MapDrive(char *mapping, int line);
char *AllocRemapPath(char *s);
void V7Addflag(char *data, char type);
int BoardNotUsed(byte boardno);
void AddLevelOne(char *s);


// --------------------------------------------------------------
// Some stuff to mark areas for character translation (FSC-0054)

#define MAXCHARSETLEN 8

char DefaultOutput[MAXCHARSETLEN + 1] = "ASCII";
char DefaultInput[MAXCHARSETLEN + 1] = "IBMPC";

typedef struct _cslist
{
    char *tag;
    char charset[MAXCHARSETLEN];
    struct _cslist *next;

} CHARSETLIST;

CHARSETLIST *CharSetList = NULL, *AssumeList = NULL;

// --------------------------------------------------------------

void AddConvert(char *s);
void AddAssume(char *s);
void ProcessCharsets(void);
void DoOneCharset(CHARSETLIST * current, int isAssume);
int inlist(char *charptr, char *tag);



typedef struct _ptlist
{

    char *tag;
    struct _ptlist *next;

} PTLIST;

/* -----------  Some Global Variables :  --------------  */

char *AreasBBS = NULL;          /* Path and name of areas.bbs file */
char *SquishCfg = NULL;
char *FEcfg = NULL;
char *GEchocfg = NULL;
//char           *Imailcfg   = NULL;
char *xMailcfg = NULL;
char *Fmailcfg = NULL;
char *Wtrgatecfg = NULL;
char *Soup2SQcfg = NULL;


typedef struct _cfgtable
{
    dword crc;
    char **cfg;
    void (*fnptr) ();

} CFGTABLE;



#define MAXCFG 7
CFGTABLE cfgtable[MAXCFG] = {
//   { crcSQUISHCFG  , &SquishCfg , ReadSquishCfg  },
    {crcAREASBBS, &AreasBBS, ReadAreasBBS},
    {crcFASTECHOCFG, &FEcfg, ReadFECFG},
    {crcGECHOCFG, &GEchocfg, ReadGEchoCFG},
//   { crcIMAILCFG   , &Imailcfg  , ReadImailCFG   },
    {crcXMAILCFG, &xMailcfg, ReadxMailCFG},
    {crcFMAILCFG, &Fmailcfg, ReadFmailCFG},
    {crcSOUP2SQCFG, &Soup2SQcfg, ReadSoup2SQCFG},
    {crcWTRGATECFG, &Wtrgatecfg, ReadWtrCFG}
};

typedef struct oneargtable
{
    dword crc;
    void (*fnptr) ();
    char oneword;

} ONEARGTABLE;


#define MAXONEARG 10
ONEARGTABLE oneargtable[MAXONEARG] = {
    {crcADDRESS, parseaddress, 1},
    {crcUUCPADDRESS, uucpaddress, 1},
    {crcFREQATTRIBUTES, freqattribs, 1},
    {crcAKAFORCE, akaforce, 0},
    {crcSTARTUP_SCAN, startup, 1},
    {crcSTARTUP_MODE, startupmode, 1},
    {crcWRITENAME, addwritename, 1},
    {crcCONVERTOUTPUT, AddConvert, 0},
    {crcASSUMECHARSET, AddAssume, 0},
    {crcLEVELONEOUTPUT, AddLevelOne, 0}

};

typedef struct _pathstringtable
{
    dword crc;
    char *string;
    char maxlen;

} PATHSTRINGTABLE;

#define MAXPATHSTRING 17

PATHSTRINGTABLE pathstringtable[MAXPATHSTRING] = {
    {crcNODELIST, cfg.usr.nodelist, 99},
    {crcFDNODELIST, cfg.usr.fdnodelist, 99},
    {crcFIDOUSER, cfg.usr.fidouser, 99},
    {crcECHOLOG, cfg.usr.echolog, 99},
    {crcJAMLOG, cfg.usr.jamlogpath, 99},
    {crcHUDSONPATH, cfg.usr.hudsonpath, 99},
    {crcEDITOR, cfg.usr.editor, 99},
    {crcPRINTER, cfg.usr.printer, 19},
    {crcLASTREAD, cfg.usr.lr, 19},
    {crcNETMAIL_SEMAPHORE, cfg.usr.netsema, 99},
    {crcUUCPNAME, cfg.usr.uucpname, 35},
    {crcAREALISTSORT, cfg.usr.alistsort, 5},
    {crcCFMFILE, cfg.usr.cfmfile, 119},
    {crcLOCALFILES, cfg.usr.localfiles, 99},
//   {crcEXESIGN          , cfg.usr.exesign,    99 },
//   {crcEXECRYPT         , cfg.usr.execrypt,   99 },
//   {crcEXESPELL         , cfg.usr.exespell,   99 },
    {crcDEFAULTOUTPUT, DefaultOutput, MAXCHARSETLEN - 1},
    {crcDEFAULTINPUT, DefaultInput, MAXCHARSETLEN - 1},
    {crcFIDONODELIST, cfg.usr.fidonodelist, 99}

};



typedef struct _toggletable
{
    dword crc;
    dword mask;

} TOGGLETABLE;

#define MAXTOGGLE 31
TOGGLETABLE toggletable[MAXTOGGLE] = {

    {crcSHOWKLUDGES, SHOWKLUDGES},
    {crcJUMPY_EDIT, JUMPYEDIT},
    {crcARCMAILATTACH, ARCMAIL},
    {crcEMPTY_TEARLINE, EMPTYTEAR},
    {crcCONFIRM_DELETE, CONFIRMDELETE},
    {crcCONFIRM_EXIT, CONFIRMEXIT},
    {crcCONFIRM_EDIT_EXIT, CONFIRMEDITEXIT},
    {crcSHOW_EDIT_HCR, SHOWEDITHCR},
    {crcCCVERBOSE, CCVERBOSE},
    {crcPERSONAL_SKIP_RECEIVED, SKIPRECEIVED},
    {crcINTLFORCE, INTLFORCE},
    {crcLOWLEVELKB, LOWLEVELKB},
    {crcNETMAIL_TEARLINE, NETTEAR},
    {crcMOVE_NOTES, SHOWNOTES},
    {crcSWAP_ON_EDIT, SWAPEDIT},
    {crcSWAP_ON_SHELL, SWAPSHELL},
    {crcEND_OF_AREA_MENU, ENDAREAMENU},
    {crcCLOCK, CLOCK},
    {crcREADNETAREAS, READNET},
    {crcREADLOCALAREAS, READLOCAL},
    {crcAKAMATCHING, AKAMATCHING},
    {crcENTERMARKS, ENTERMARKS},
    {crcJAMZGTBIT, JAMZGTBIT},
    {crcJAMGETLAST, JAMGETLAST},
    {crcEDITSAVEMENU, EDITSAVEMENU},
    {crcSPELLCHECK_DEFAULT, SPELLCHECKDFLT},
    {crcINTERNET_EMPTY_LINE, INTERNETEMPTY},
    {crcNOSPACE_PASSWORD, NOSPACEPASSWORD},
    {crcREPLYTOCFM, REPLYTOCFM},
    {crcRESPECTPRIVATE, RESPECTPRIVATE},
    {crcAREALISTWRAPAROUND, AREALISTWRAPAROUND}

};

typedef struct _colourtable
{
    dword crc;
    int index;
    char stdcol;

} COLOURTABLE;


#define MAXCOLOUR 35
COLOURTABLE colourtable[MAXCOLOUR] = {

    {crcCOLOR_ASBAR, Casbar, 112},
    {crcCOLOR_ASFRAME, Casframe, 7},
    {crcCOLOR_ASTEXT, Castext, 7},
    {crcCOLOR_ASHIGH, Cashigh, 112},
    {crcCOLOR_ASSPECIAL, Casspecial, 14},
    {crcCOLOR_ASACCENT, Casaccent, 14},
    {crcCOLOR_ASHIGHACCENT, Cashighaccent, 14},
    {crcCOLOR_MSGHEADER, Cmsgheader, 7},
    {crcCOLOR_MSGDATA, Cmsgdata, 7},
    {crcCOLOR_MSGLINKS, Cmsglinks, 7},
    {crcCOLOR_MSGATTRIBS, Cmsgattribs, 7},
    {crcCOLOR_MSGDATE, Cmsgdate, 7},
    {crcCOLOR_MSGLINE, Cmsgline, 7},
    {crcCOLOR_MSGQUOTE, Cmsgquote, 14},
    {crcCOLOR_MSGTEXT, Cmsgtext, 7},
    {crcCOLOR_MSGKLUDGE, Cmsgkludge, 14},
    {crcCOLOR_MSGORIGIN, Cmsgorigin, 14},
    {crcCOLOR_MSGBAR, Cmsgbar, 112},
    {crcCOLOR_MSGBARACCENT, Cmsgbaraccent, 112},
    {crcCOLOR_MSGSPECIAL, Cmsgspecial, 112},
    {crcCOLOR_ENTRY, Centry, 112},
    {crcCOLOR_EDITHCR, Cedithcr, 7},
    {crcCOLOR_EDITCURNORMAL, Ceditcurnormal, 14},
    {crcCOLOR_EDITCURQUOTE, Ceditcurquote, 7},
    {crcCOLOR_EDITBLOCK, Ceditblock, 112},
    {crcCOLOR_EDITCURBLOCK, Ceditcurblock, 112},
    {crcCOLOR_POPFRAME, Cpopframe, 112},
    {crcCOLOR_POPTEXT, Cpoptext, 112},
    {crcCOLOR_POPTITLE, Cpoptitle, 112},
    {crcCOLOR_FINDACTIVE, Cfindactive, 14},
    {crcCOLOR_FINDPASSIVE, Cfindpassive, 7},
    {crcCOLOR_FINDTEXT, Cfindtext, 7},
    {crcCOLOR_FINDDATA, Cfinddata, 14},
    {crcCOLOR_FINDHIGH, Cfindhigh, 112},
    {crcCOLOR_FINDACCENT, Cfindaccent, 14}

};

int stuff = 0, globalline = 0;
PTLIST *firstpt = NULL;


char *StripCR(char *str)
{
    Strip_Trailing(str, '\n');
    Strip_Trailing(str, '\r');
    return str;
}


AREA *last = NULL;

/* Here goes... */

void readconfig(void)
{
    char temp[100];
    int i;

    strcpy(temp, cfg.homedir);

    strcpy(cfg.homedir, temp);

    /* now set the default colors */

    for (i = 0; i < MAXCOLOUR; i++)
    {
        cfg.col[colourtable[i].index] = colourtable[i].stdcol;
    }

    sprintf(cfg.usr.writename, "%s\\timed.prn", cfg.homedir);
    strcpy(cfg.usr.lr, "lastread");
    strcpy(cfg.usr.printer, "PRN");
    cfg.usr.status |=
        (CONFIRMDELETE | NETTEAR | SHOWNOTES | ENDAREAMENU |
         CONFIRMEDITEXIT | LOWLEVELKB | JUMPYEDIT | CLOCK | JAMGETLAST |
         INTERNETEMPTY);

    strcat(temp, "\\timed.cfg");

    print(10, 7, 9, "û");
    print(10, 35, 7, "(timEd.cfg)");

    ReadConfigFile(temp, 0);

    if (SquishCfg != NULL)
        ReadSquishCfg(NULL);

    for (i = 0; i < MAXCFG; i++)
    {
        if (*(cfgtable[i].cfg) != NULL)
        {
            (*(cfgtable[i].fnptr)) ();

            if (cfgtable[i].crc != crcSQUISHCFG)
                mem_free(*(cfgtable[i].cfg));
        }
    }

    if (cfg.first == NULL)
        Message("No areas defined!", -1, 254, YES);

    ProcessCharsets();

    mem_freeptlist();

    if (strcmpi(cfg.usr.editor, "INTERNAL") == 0)
        cfg.usr.internal_edit = 1;

    if (stuff != 0)
        stuffkey(stuff);

#ifdef __OS2__
    cfg.usr.status |= CLOCK;
    cfg.usr.status |= (LOWLEVELKB);
//   #else
//     cfg.usr.status &= ~LOWLEVELKB;
#endif

}


/* Analyse one line from the config file */


void analyse(char *line, int number)
{
    char *keyword;
    dword crc;
    int i;

    globalline = number;

    keyword = strtok(line, " \n\r\t\0"); /* Get keyword */

    if ((keyword == NULL) || (keyword[0] == ';')) /* It's a comment! */
        return;

    if (strcmpi(keyword, "386") == 0 ||
        strcmpi(keyword, "DOS") == 0 || strcmpi(keyword, "OS2") == 0)
    {
#ifdef __OS2__

        // OS/2 version
        if (strcmpi(keyword, "OS2") != 0)
            return;

#elif defined(__FLAT__)

        // DOS/386 version
        if (strcmpi(keyword, "386") != 0)
            return;

#else

        // DOS 16 bit version
        if (strcmpi(keyword, "DOS") != 0)
            return;

#endif

        keyword = strtok(NULL, " \n\r\t\0"); /* Get real keyword after
                                                '386' etc */
    }

    strupr(keyword);

    crc = JAMsysCrc32(keyword, strlen(keyword), -1L);

    // Check for a config file being defined
    for (i = 0; i < MAXCFG; i++)
    {
        if (crc == cfgtable[i].crc)
        {
            *(cfgtable[i].cfg) = mem_strdup(xgstr(strtok(NULL, " \t\n")));
            return;
        }
    }

    for (i = 0; i < MAXONEARG; i++)
    {
        if (crc == oneargtable[i].crc)
        {
            if (oneargtable[i].oneword)
                (*(oneargtable[i].fnptr)) (xgstr(strtok(NULL, " \t\n\r")));
            else
                (*(oneargtable[i].fnptr)) (xgstr(strtok(NULL, "\n\r")));
            return;
        }
    }


    for (i = 0; i < MAXPATHSTRING; i++)
    {
        if (crc == pathstringtable[i].crc)
        {
            strncpy(pathstringtable[i].string,
                    Strip_Trailing(xgstr(strtok(NULL, " \t\n")), '\\'),
                    pathstringtable[i].maxlen);
            return;
        }
    }


    for (i = 0; i < MAXTOGGLE; i++)
    {
        if (crc == toggletable[i].crc)
        {
            cfg.usr.status =
                (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ?
                cfg.usr.status | toggletable[i].mask :
                cfg.usr.status & ~toggletable[i].mask;
            return;
        }
    }


    for (i = 0; i < MAXCOLOUR; i++)
    {
        if (crc == colourtable[i].crc)
        {
            cfg.col[colourtable[i].index] =
                atoi(xgstr(strtok(NULL, " \t\n")));
            return;
        }
    }

    if (crc == crcV7MODEMTYPEBIT)
        V7Addflag(strtok(NULL, "\n"), MODEMBIT);

    else if (crc == crcV7MODEMTYPEVALUE)
        V7Addflag(strtok(NULL, "\n"), MODEMVALUE);

    else if (crc == crcV7NODEFLAGBIT)
        V7Addflag(strtok(NULL, "\n"), NODEBIT);

    else if (strcmp(keyword, "ECHOAREA") == 0)
        addarea(strtok(NULL, "\n"), ECHOMAIL);

    else if (strcmp(keyword, "NEWSAREA") == 0)
        addarea(strtok(NULL, "\n"), NEWS);

    else if (strcmp(keyword, "MAILAREA") == 0)
        addarea(strtok(NULL, "\n"), MAIL);

    else if (strcmp(keyword, "NETAREA") == 0)
        addarea(strtok(NULL, "\n"), NETMAIL);

    else if (strcmp(keyword, "LOCALAREA") == 0)
        addarea(strtok(NULL, "\n"), LOCAL);

    else if (strcmp(keyword, "MACRO") == 0)
        addmacro(xgstr(strtok(NULL, "\n")), 0);

    else if (strcmp(keyword, "USENETMACRO") == 0)
        addmacro(xgstr(strtok(NULL, "\n")), 1);

    else if (strcmp(keyword, "NAME") == 0)
        addname(getquoted(strtok(NULL, "\n")));

    else if (strcmp(keyword, "ORIGIN") == 0)
        strncpy(cfg.usr.origin, getquoted(strtok(NULL, "\n")), 65);

    else if (strcmp(keyword, "SIGNOFF") == 0)
        strncpy(cfg.usr.signoff, getquoted(strtok(NULL, "\n")), 300);

    else if (strcmp(keyword, "INCLUDE") == 0)
        ReadConfigFile(xgstr(strtok(NULL, " \t\n")), 1);

    else if (strcmp(keyword, "HELLO") == 0)
        strncpy(cfg.usr.hello, getquoted(strtok(NULL, "\n")), 300);

    else if (strcmp(keyword, "REPHELLO") == 0)
        strncpy(cfg.usr.rephello, getquoted(strtok(NULL, "\n")), 300);

    else if (strcmp(keyword, "FOLLOWHELLO") == 0)
        strncpy(cfg.usr.followhello, getquoted(strtok(NULL, "\n")), 300);

    else if (strcmp(keyword, "STARTUP_LINES") == 0)
        cfg.usr.lines = (char)atoi(xgstr(strtok(NULL, " \t\n")));

    else if (strcmp(keyword, "EXESIGN") == 0)
        strncpy(cfg.usr.exesign, xgstr(strtok(NULL, "\r\n")), 99);

    else if (strcmp(keyword, "EXECRYPT") == 0)
        strncpy(cfg.usr.execrypt, xgstr(strtok(NULL, "\r\n")), 99);

    else if (strcmp(keyword, "EXESPELL") == 0)
        strncpy(cfg.usr.exespell, xgstr(strtok(NULL, "\r\n")), 99);

    else if (strcmp(keyword, "EXECRYPTSIGN") == 0)
        strncpy(cfg.usr.execryptsign, xgstr(strtok(NULL, "\r\n")), 99);

    else if (strcmp(keyword, "SQUISHCFG") == 0)
        SquishCfg = mem_strdup(xgstr(strtok(NULL, " \r\n")));

    else if (strcmp(keyword, "KILL_ORIGINAL") == 0)
        cfg.usr.delreply = one_of_three(xgstr(strtok(NULL, " \t\n")));

    else if (strcmp(keyword, "ZONEGATE") == 0)
        cfg.usr.zonegate = one_of_three(xgstr(strtok(NULL, " \t\n")));

    else if (strcmp(keyword, "SQUISH_OFFSET") == 0)
        cfg.usr.sqoff = (word) atoi(xgstr(strtok(NULL, " \t\n")));

    else if (strcmp(keyword, "HUDSON_OFFSET") == 0)
        cfg.usr.hudsonoff = (word) atoi(xgstr(strtok(NULL, " \t\n")));

    else if (strcmp(keyword, "DRIVEREPLACE") == 0)
        MapDrive(xgstr(strtok(NULL, "\n")), globalline);

    else
    {
        sprintf(msg, "Unknown keyword: %-0.30s on line %d!", keyword,
                globalline);
        Message(msg, -1, 0, YES);
    }
}


// =========================================================

void parseaddress(char *address)
{
    static int n = 0;

    if (n > (tMAXAKAS - 1))
    {
        sprintf(msg, "Too many AKA's defined! Skipping %s", address);
        Message(msg, -1, 0, YES);
        return;
    }

    sscanf(address, "%hu:%hu/%hu.%hu", &cfg.usr.address[n].zone,
           &cfg.usr.address[n].net, &cfg.usr.address[n].node,
           &cfg.usr.address[n].point);
    n++;
}


// =========================================================

void addname(char *name)
{
    static int n = 0;

    if (name == NULL)
        return;

    if (n > (tMAXNAMES - 1))
    {
        sprintf(msg, "Too many Names defined! Skipping %s", name);
        Message(msg, -1, 0, YES);
        return;
    }

    strncpy(cfg.usr.name[n].name, name, 39);
    cfg.usr.name[n].hash = SquishHash(name);

    strlwr(name);
    cfg.usr.name[n].crc = JAMsysCrc32(name, strlen(name), -1L);

    n++;
}


// =========================================================

AREA *init_area()
{
    AREA *thisarea;

    thisarea = mem_calloc(1, sizeof(AREA));

    thisarea->highest = thisarea->lr = -1;
    thisarea->stdattr |= aLOCAL;
    strcpy(thisarea->cswrite, DefaultOutput);
    strcpy(thisarea->csread, DefaultInput);

    return thisarea;
}

// =========================================================

void check_aka(AREA * current, char *rest)
{
    char *charptr, *aka;
    char temp[80];
    NETADDR tempaddr;
    int n;


    if (rest == NULL)
        return;

    if (((charptr = strstr(rest, "-P")) != NULL)
        || ((charptr = strstr(rest, "-p")) != NULL))
    {
        charptr += 2;

        if ((aka = strtok(charptr, " \t\n\r")) != NULL)
            strcpy(temp, aka);
        else
        {
            sprintf(msg, "Can't figure out -P switch in line %d!",
                    globalline);
            Message(msg, -1, 0, YES);
            return;
        }

        memset(&tempaddr, '\0', sizeof(NETADDR));
        sscanf(temp, "%hu:%hu/%hu.%hu", &tempaddr.zone, &tempaddr.net,
               &tempaddr.node, &tempaddr.point);

        for (n = 0; n < (tMAXAKAS - 1); n++)
        {
            if (addrcmp(&cfg.usr.address[n], &tempaddr) == 0)
            {
                current->aka = n;
                break;
            }
        }

        if (n == (tMAXAKAS - 1))
        {
            sprintf(msg,
                    "Unknown AKA (%hu:%hu/%hu.%hu) referenced (line %d)!",
                    tempaddr.zone, tempaddr.net, tempaddr.node,
                    tempaddr.point, globalline);
            Message(msg, -1, 0, YES);
        }

    }


}

// ============================================================
/* Add an area found in timEd.cfg */

void addarea(char *value, word type)
{
    AREA *current;

    char temp[101], *tempptr, *tagptr, *pathptr, *rest, *charptr;


    if (value == NULL)
        return;

    current = init_area();

    strncpy(temp, getquoted(value), 59); /* First get the (quoted)
                                            description */
    temp[59] = 0;

    current->desc = mem_strdup(temp);

    tempptr = value;
    while ((*tempptr != '"') && (*tempptr != '\0'))
        tempptr++;

    if (tempptr != '\0')
        tempptr++;              /* skip over first " */

    while ((*tempptr != '"') && (*tempptr != '\0'))
        tempptr++;


    if (*tempptr == '\0')
    {
        mem_free(current);      /* fail */
        return;
    }

    value = tempptr + 1;        /* Skip over second " */

    tagptr = strtok(value, " \t\n\r");
    pathptr = strtok(NULL, " \t\n\r");

    if ((tagptr == NULL) || (pathptr == NULL))
    {
        mem_free(current);
        return;
    }

    current->tag = mem_strdup(tagptr);

    memset(temp, '\0', sizeof(temp));
    strncpy(temp, pathptr, 79);
    Strip_Trailing(temp, '\\');
    current->dir = mem_strdup(temp);

    current->type = type;

    if ((rest = strtok(NULL, "\n")) != NULL)
    {
        if (strstr(rest, "-$") != NULL)
            current->base |= MSGTYPE_SQUISH;

        else if ((strstr(rest, "-J") != NULL) ||
                 (strstr(rest, "-j") != NULL))
            current->base |= MSGTYPE_JAM;

        else if ((strstr(rest, "-H") != NULL) ||
                 (strstr(rest, "-h") != NULL))
            current->base |= MSGTYPE_HMB;

        else
            current->base |= MSGTYPE_SDM;


        if (((charptr = strstr(rest, "-A")) != NULL)
            || ((charptr = strstr(rest, "-a")) != NULL))
        {
            charptr += 2;
            while (*charptr != ' ' && *charptr != '-' && *charptr != '\t'
                   && *charptr != '\n' && *charptr != '\0'
                   && *charptr != '\r')
            {
                switch (*charptr)
                {
                case 'p':
                case 'P':
                    current->stdattr |= aPRIVATE;
                    break;

                case 'c':
                case 'C':
                    current->stdattr |= aCRASH;
                    break;

                case 'k':
                case 'K':
                    current->stdattr |= aKILL;
                    break;

                case 'd':
                case 'D':
                    current->stdattr |= aDIR;
                    break;

                case 'i':
                case 'I':
                    current->stdattr |= aIMM;
                    break;

                case 'r':
                case 'R':
                    current->readonly = 1;
                    break;

                default:
                    sprintf(msg, "Junk attribute in %s (line %d)", rest,
                            globalline);
                    Message(msg, -1, 0, YES);
                    break;
                }
                charptr++;
            }
        }

        check_aka(current, rest); /* See if an AKA is used.. */
    }
    else                        /* rest was NULL, so *.MSG */
        current->base |= MSGTYPE_SDM;

    /* Set local, echomail, netmail bits */

    if (type == ECHOMAIL)
        current->base |= MSGTYPE_ECHO;
    else if (type == LOCAL)
        current->base |= MSGTYPE_LOCAL;
    else if (type == MAIL)
        current->base |= MSGTYPE_MAIL;
    else if (type == NEWS)
        current->base |= MSGTYPE_NEWS;
    else
        current->base |= MSGTYPE_NET;

    AddLinkedArea(current);

}


/* ------------------------------- */
/* Extract text from within quotes */
/* ------------------------------- */


char *getquoted(char *line)
{
    static char temp[301];
    char *charptr, *tempptr;

    memset(temp, '\0', sizeof(temp));
    tempptr = temp;

    if ((charptr = line) == NULL)
        return temp;

    if (strchr(line, '"') == NULL
        || strchr(line, '"') == strrchr(line, '"'))
    {
        sprintf(msg, "Illegal line format in %0.30s (line %d)", line,
                globalline);
        Message(msg, -1, 0, YES);
        return temp;
    }

    while (*charptr++ != '"') /* nothing */ ; /* skip leading space */

    while (*charptr != '"' && (strlen(temp) < 300)) /* Copy chars */
        *tempptr++ = *charptr++;

    return temp;

}


// =========================================================

char *xgstr(char *line)
{
    static char temp[5] = "";


    if (line != NULL)
        return line;
    else
        return temp;
}


/* ------------------------------------------------------ */
/* --  Add an area from Squish.cfg to the linked list  -- */
/* ------------------------------------------------------ */


void AddEchoArea(char *line, int type)
{
    AREA *thisarea;
    char *rest_of_line, *tagptr, *pathptr;
    char temp[100];


    if (line == NULL)
        return;

    thisarea = init_area();

    tagptr = strtok(line, " \t\n\r");
    pathptr = strtok(NULL, " \t\n\r");

    if ((tagptr == NULL) || (pathptr == NULL))
    {
        mem_free(thisarea);
        return;
    }

    thisarea->tag = mem_strdup(tagptr);
    thisarea->desc = thisarea->tag;

    memset(temp, '\0', sizeof(temp));
    strncpy(temp, pathptr, 79);
    Strip_Trailing(temp, '\\');
    thisarea->dir = AllocRemapPath(temp);

    if ((rest_of_line = strtok(NULL, "\n")) != NULL)
    {
        if (strchr(rest_of_line, '$') != NULL) /* Squish area */
            thisarea->base = MSGTYPE_SQUISH;
        else
            thisarea->base = MSGTYPE_SDM;

        if (strstr(rest_of_line, "-0") != NULL) /* passthru */
        {
            /* Add to list of passthru area */
            add_to_ptlist(thisarea->tag);

            mem_free(thisarea->tag);
            mem_free(thisarea->dir);
            mem_free(thisarea);
            return;
        }

        check_aka(thisarea, rest_of_line);
    }
    else                        /* SDM area */
        thisarea->base = MSGTYPE_SDM;

    thisarea->type = type;

    switch (type)
    {
    case ECHOMAIL:
        thisarea->base |= MSGTYPE_ECHO;
        break;
    case LOCAL:
        thisarea->base |= MSGTYPE_LOCAL;
        break;
    case NETMAIL:
        thisarea->base |= MSGTYPE_NET;
        thisarea->stdattr |= aPRIVATE;
        break;
    }

    AddLinkedArea(thisarea);

}


/* ------------------------------------------------------------- */
/* --  Here we add an area from a areas.bbs line to the list  -- */
/* ------------------------------------------------------------- */


void AddAreasArea(char *line)   /* Here we add an area to the list */
{
    AREA *thisarea, *curarea;
    char *tagptr, *pathptr, *rest;
    char temp[100];

    thisarea = init_area();

    if (line == NULL)
        return;


    while (line[0] == '$' || line[0] == '#' || line[0] == 'p'
           || line[0] == 'P' || line[0] == '!')
    {
        if (line[0] == '$')     /* Squish area */
        {
            thisarea->base = MSGTYPE_SQUISH | MSGTYPE_ECHO;
            line++;             /* Skip first char ($) */
        }

        else if (line[0] == '!') /* Squish area */
        {
            thisarea->base = MSGTYPE_JAM | MSGTYPE_ECHO;
            line++;             /* Skip first char ($) */
        }

        else                    // Passthru area
        {
            mem_free(thisarea);
            return;
        }
    }

    if (!(thisarea->base & (MSGTYPE_SQUISH | MSGTYPE_JAM)))
    {
        if (isdigit(*line))
            thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;
        else
            thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;
    }

    pathptr = strtok(line, " \t\n\r");
    tagptr = strtok(NULL, " \t\n\r");

    if ((pathptr == NULL) || (tagptr == NULL))
    {
        mem_free(thisarea);
        return;
    }

    if (checkpt(tagptr) != 0)
    {
        mem_free(thisarea);
        return;
    }

    thisarea->tag = mem_strdup(tagptr);

    memset(temp, '\0', sizeof(temp));
    strncpy(temp, pathptr, 79);
    Strip_Trailing(temp, '\\');
    thisarea->dir = AllocRemapPath(temp);

    thisarea->desc = thisarea->tag;

    if ((rest = strtok(NULL, "\n\r")) != NULL)
        check_aka(thisarea, rest);

    curarea = cfg.first;
    while (curarea)
    {
        if (strcmpi(curarea->tag, thisarea->tag) == 0) /* Dupe! */
        {
            if (thisarea->base & MSGTYPE_SQUISH) /* See if it's finally
                                                    squish type */
            {
                curarea->base |= MSGTYPE_SQUISH;
                curarea->base &= ~MSGTYPE_SDM; /* Switch off SDM bit */
            }

            mem_free(thisarea->tag);
            mem_free(thisarea->dir);
            mem_free(thisarea);
            return;
        }
        curarea = curarea->next;
    }

    thisarea->type = ECHOMAIL;

    AddLinkedArea(thisarea);
}



/* ------------------------------------------------------------ */
/* --  Read the squish.cfg file and get Echoareas out of it  -- */
/* ------------------------------------------------------------ */

// If filename == NULL, use the SquishCFG. Otherwise read the 'filename'.

void ReadSquishCfg(char *filename)
{

    XFILE *squishfile;
    char *line, temp[80];
    char *keyword;
    unsigned lineno = 0;


    if (filename == NULL)
    {
        if ((squishfile = xopen(SquishCfg)) == NULL)
        {
            sprintf(msg, "Can't open squish.cfg! (%s)", SquishCfg);
            Message(msg, -1, 254, YES);
        }
    }
    else
    {
        if ((squishfile = xopen(filename)) == NULL)
        {
            sprintf(msg, "Can't open squish include file! (%s)", filename);
            Message(msg, -1, 254, YES);
        }
    }

    if (!filename)
    {
        sprintf(temp, "(%s)", SquishCfg);
        if (strlen(temp) > 33)
        {
            temp[33] = '\0';
            temp[32] = '.';
            temp[31] = '.';
        }
        print(14, 36, 7, temp);
    }

    while ((line = xgetline(squishfile)) != NULL)
    {
        if (!(lineno++ % 5))
            working(14, 7, 7);


        if (line[0] == ';' || strlen(line) < 11) /* Quick check for speed */
            continue;

        if ((keyword = strtok(line, " \t\r")) != NULL)
        {
            if (strcmpi(keyword, "EchoArea") == 0)
                AddEchoArea(strtok(NULL, "\n"), ECHOMAIL);
            else if ((strcmpi(keyword, "LocalArea") == 0)
                     && (cfg.usr.status & READLOCAL))
                AddEchoArea(strtok(NULL, "\n"), LOCAL);
            else if ((strcmpi(keyword, "BadArea") == 0)
                     && (cfg.usr.status & READLOCAL))
                AddEchoArea(strtok(NULL, "\n"), LOCAL);
            else if ((strcmpi(keyword, "DupeArea") == 0)
                     && (cfg.usr.status & READLOCAL))
                AddEchoArea(strtok(NULL, "\n"), LOCAL);
            else if ((strcmpi(keyword, "NetArea") == 0)
                     && (cfg.usr.status & READNET))
                AddEchoArea(strtok(NULL, "\n"), NETMAIL);
            else if (strcmpi(keyword, "AreasBBS") == 0)
                AreasBBS = mem_strdup(xgstr(strtok(NULL, " \t\r\n")));
            else if (strcmpi(keyword, "include") == 0)
                ReadSquishCfg(xgstr(strtok(NULL, " \t\r\n")));
            else if (strcmpi(keyword, "ArcMailAttach") == 0)
                cfg.usr.status |= ARCMAIL;
        }
    }

    xclose(squishfile);

    print(14, 7, 9, "û");

}


/* ------------------------------------------------------- */
/* --  Read a Areas.bbs file and get the defined areas  -- */
/* ------------------------------------------------------- */

void ReadAreasBBS()
{

    XFILE *areasfile;
    char *line, drive[_MAX_DRIVE], dir[_MAX_DIR], temppath[256];
    int skip = 0;               /* To skip first line */
    char *p, temp[80];
    unsigned lineno = 0;

    areasfile = xopen(AreasBBS);

    if (areasfile == NULL)
    {
        if (SquishCfg)
        {
            fnsplit(SquishCfg, drive, dir, NULL, NULL);
            sprintf(temppath, "%s%s%s", drive, dir, AreasBBS);
        }
        if ((areasfile = xopen(temppath)) == NULL)
        {
            sprintf(msg, "Can't open areas.bbs file! (%s)", AreasBBS);
            Message(msg, -1, 0, YES);
            return;
        }
    }

    sprintf(temp, "(%s)", AreasBBS);
    if (strlen(temp) > 44)
    {
        temp[44] = '\0';
        temp[43] = '.';
        temp[42] = '.';
    }
    print(16, 25, 7, temp);


    while ((line = xgetline(areasfile)) != NULL)
    {
        if (!(lineno++ % 5))
            working(16, 7, 7);
        p = line;
        while (p && isspace(*p))
            p++;
        strcpy(line, p);

        if ((strlen(line) > 4) && (line[0] != ';') && (line[0] != '-')
            && skip++)
            AddAreasArea(line);
    }

    xclose(areasfile);

    print(16, 7, 9, "û");

}

// =========================================================


void ReadFECFG()
{
    CONFIG *fecfg;
    int l, extraleft, n, left, toread;
    long areaoffset = 0L;
    int cfgfile;
    Area *areas = NULL;
    FE141Area *FE141areas = NULL;
    char temp[100];
    ExtensionHeader eh;
    SysAddress *aka_array;
    int got;
    AREA *thisarea;
    word revision = 0;


    if ((cfgfile = sopen(FEcfg, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
    {
        sprintf(msg, "Can't open %s!", FEcfg);
        Message(msg, -1, 0, YES);
        sprintf(msg, "Errno is: %d", errno);
        Message(msg, -1, 0, YES);
        return;
    }

    sprintf(temp, "(%s)", FEcfg);
    if (strlen(temp) > 33)
    {
        temp[33] = '\0';
        temp[32] = '.';
        temp[31] = '.';
    }
    print(14, 36, 7, temp);

    if (read(cfgfile, &revision, sizeof(word)) != sizeof(word))
    {
        sprintf(msg, "Error reading %s!", FEcfg);
        Message(msg, -1, 0, YES);
        close(cfgfile);
        return;
    }

    lseek(cfgfile, 0L, SEEK_SET);

    if ((revision < 4) || (revision > 6))
    {
        Message("Unsupported Fastecho version!", -1, 0, YES);
        close(cfgfile);
        return;
    }

    if (revision == 6)
    {
        ReadNewFECFG(cfgfile);
        return;
    }

    fecfg = mem_calloc(1, sizeof(CONFIG));

    if (read(cfgfile, fecfg, sizeof(CONFIG)) != sizeof(CONFIG))
    {
        sprintf(msg, "Error reading %s!", FEcfg);
        Message(msg, -1, 0, YES);
        close(cfgfile);
        mem_free(fecfg);
        return;
    }

    extraleft = fecfg->offset;
    if (fecfg->AkaCnt == 0)
        fecfg->AkaCnt = 1;
    aka_array = mem_calloc(fecfg->AkaCnt, sizeof(SysAddress));
    if (fecfg->AkaCnt == 1)
        memmove(aka_array, fecfg->oldakas, sizeof(SysAddress));

    for (n = 0; extraleft > 0; n++)
    {
        if (read(cfgfile, &eh, sizeof(ExtensionHeader)) !=
            sizeof(ExtensionHeader))
        {
            sprintf(msg, "Error reading %s!", FEcfg);
            Message(msg, -1, 0, YES);
            close(cfgfile);
            mem_free(fecfg);
            mem_free(aka_array);
            return;
        }

        if (eh.type == EH_AKAS)
        {
            read(cfgfile, aka_array, eh.offset);
            break;
        }

        extraleft -= eh.offset;
        lseek(cfgfile, eh.offset, SEEK_CUR);
    }

    // Add the netmail area from the config.
    if (cfg.usr.status & READNET)
    {
        thisarea = init_area();
        thisarea->base |= (MSGTYPE_SDM | MSGTYPE_NET);
        thisarea->type = NETMAIL;
        thisarea->tag = mem_strdup("Netmail");
        thisarea->stdattr |= aPRIVATE;
        thisarea->desc = thisarea->tag;
        thisarea->dir = AllocRemapPath(fecfg->NetMPath);
        AddLinkedArea(thisarea);
    }

    if (fecfg->revision == 4)
    {
        areaoffset =
            (long)sizeof(CONFIG) + (long)fecfg->offset +
            (long)(fecfg->NodeCnt * sizeof(Node));

        lseek(cfgfile, areaoffset, SEEK_SET);

        areas = mem_calloc(25, sizeof(Area)); /* Read buffer to read disk */

        for (l = 0, left = fecfg->AreaCnt; left > 0; left -= 25)
        {
            toread = min(left, 25);
            if (toread == 0)
                break;

            if (read(cfgfile, areas, (unsigned)(toread * sizeof(Area))) !=
                (int)(toread * sizeof(Area)))
            {
                sprintf(msg, "Error reading %s!", FEcfg);
                Message(msg, -1, 0, YES);
                close(cfgfile);
                mem_free(fecfg);
                mem_free(aka_array);
                mem_free(areas);
                return;
            }

            for (l = 0; l < toread; l++)
            {
                if (!(l % 5))
                    working(14, 7, 7);
                ana_FE_area(&areas[l], aka_array);
            }
        }
    }
// ==============
    else
    {
        areaoffset =
            (long)sizeof(CONFIG) + (long)fecfg->offset +
            (long)(fecfg->NodeCnt * sizeof(FE141Node));
        lseek(cfgfile, (long)areaoffset, SEEK_SET);

        FE141areas = mem_calloc(25, sizeof(FE141Area)); /* Read buffer to
                                                           read disk */

        for (l = 0, left = fecfg->AreaCnt; left > 0; left -= 25)
        {
            toread = min(left, 25);
            if (toread == 0)
                break;

            if ((got =
                 read(cfgfile, FE141areas,
                      (unsigned)(toread * sizeof(FE141Area)))) !=
                (int)(toread * sizeof(FE141Area)))
            {
                sprintf(msg, "Error reading %s! (got %d, wanted %d)",
                        FEcfg, got, toread * sizeof(FE141Area));
                Message(msg, -1, 0, YES);
                close(cfgfile);
                mem_free(fecfg);
                mem_free(aka_array);
                mem_free(FE141areas);
                return;
            }

            for (l = 0; l < toread; l++)
            {
                if (!(l % 5))
                    working(14, 7, 7);
                ana_FE141_area(&FE141areas[l], aka_array);
            }
        }
    }

// ==============

    close(cfgfile);
    if (areas)
        mem_free(areas);
    if (FE141areas)
        mem_free(FE141areas);
    mem_free(aka_array);

    print(14, 7, 9, "û");

}

// =========================================================


void ReadNewFECFG(int cfgfile)
{
    FENEW_CONFIG *fecfg;
    int l, extraleft, n, left, toread;
    long areaoffset = 0L;
    Area *areas = NULL;
    FENEW_Area *FE142areas = NULL;
    ExtensionHeader eh;
    FENEW_SysAddress *aka_array;
    int got;
    AREA *thisarea;



    fecfg = mem_calloc(1, sizeof(FENEW_CONFIG));

    if (read(cfgfile, fecfg, sizeof(FENEW_CONFIG)) != sizeof(FENEW_CONFIG))
    {
        sprintf(msg, "Error reading %s!", FEcfg);
        Message(msg, -1, 0, YES);
        close(cfgfile);
        mem_free(fecfg);
        return;
    }

    extraleft = fecfg->offset;
    if (fecfg->AkaCnt == 0)
        fecfg->AkaCnt = 1;
    aka_array = mem_calloc(fecfg->AkaCnt, sizeof(FENEW_SysAddress));

    for (n = 0; extraleft > 0; n++)
    {
        if (read(cfgfile, &eh, sizeof(ExtensionHeader)) !=
            sizeof(ExtensionHeader))
        {
            sprintf(msg, "Error reading %s!", FEcfg);
            Message(msg, -1, 0, YES);
            close(cfgfile);
            mem_free(fecfg);
            mem_free(aka_array);
            return;
        }

        if (eh.type == EH_AKAS)
        {
            read(cfgfile, aka_array, eh.offset);
            break;
        }

        extraleft -= eh.offset;
        lseek(cfgfile, eh.offset, SEEK_CUR);
    }

    // Add the netmail area from the config.
    if (cfg.usr.status & READNET)
    {
        thisarea = init_area();
        thisarea->base |= (MSGTYPE_SDM | MSGTYPE_NET);
        thisarea->type = NETMAIL;
        thisarea->tag = mem_strdup("Netmail");
        thisarea->stdattr |= aPRIVATE;
        thisarea->desc = thisarea->tag;
        thisarea->dir = AllocRemapPath(fecfg->NetMPath);
        AddLinkedArea(thisarea);
    }

    areaoffset =
        (long)sizeof(CONFIG) + (long)fecfg->offset +
        (long)(fecfg->NodeCnt * fecfg->NodeRecSize);
    lseek(cfgfile, (long)areaoffset, SEEK_SET);

    FE142areas = mem_calloc(25, sizeof(FENEW_Area)); /* Read buffer to
                                                        read disk */

    for (l = 0, left = fecfg->AreaCnt; left > 0; left -= 25)
    {
        toread = min(left, 25);
        if (toread == 0)
            break;

        if ((got =
             read(cfgfile, FE142areas,
                  (unsigned)(toread * sizeof(FENEW_Area)))) !=
            (int)(toread * sizeof(FENEW_Area)))
        {
            sprintf(msg, "Error reading %s! (got %d, wanted %d)", FEcfg,
                    got, toread * sizeof(FENEW_Area));
            Message(msg, -1, 0, YES);
            close(cfgfile);
            mem_free(fecfg);
            mem_free(aka_array);
            mem_free(FE142areas);
            return;
        }

        for (l = 0; l < toread; l++)
        {
            if (!(l % 5))
                working(14, 7, 7);
            ana_FE142_area(&FE142areas[l], aka_array);
        }
    }

    close(cfgfile);
    if (areas)
        mem_free(areas);
    if (FE142areas)
        mem_free(FE142areas);
    mem_free(aka_array);

    print(14, 7, 9, "û");

}

// =========================================================


void ana_FE_area(Area * area, SysAddress * aka_array)
{
    AREA *thisarea;
    char tempaka[100];


    if (area->flags.type == PT_BOARD)
        return;

    thisarea = init_area();

    if (area->flags.type == SQUISH) /* Squish area */
        thisarea->base = MSGTYPE_SQUISH;
    else if (area->flags.type == JAM)
        thisarea->base = MSGTYPE_JAM;
    else if (area->flags.type == QBBS)
        thisarea->base = MSGTYPE_HMB;
    else
        thisarea->base = MSGTYPE_SDM;

    switch (area->type)
    {
    case AREA_ECHOMAIL:
        thisarea->base |= MSGTYPE_ECHO;
        thisarea->type = ECHOMAIL;
        break;
    case AREA_NETMAIL:
        if (!(cfg.usr.status & READNET))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_NET;
        thisarea->type = NETMAIL;
        thisarea->stdattr |= aPRIVATE;
        break;
    default:
        if (!(cfg.usr.status & READLOCAL))
        {
            mem_free(thisarea);
            return;
        }

        thisarea->base |= MSGTYPE_LOCAL;
        thisarea->type = LOCAL;
        break;
    }

    thisarea->group = area->flags.group;

    strlwr(area->name);

    thisarea->tag = mem_strdup(area->name);
    if (area->desc[0] != '\0')
        thisarea->desc = mem_strdup(area->desc);
    else
        thisarea->desc = thisarea->tag;

    if (!(thisarea->base & MSGTYPE_HMB))
    {
        Strip_Trailing(area->path, '\\');
        thisarea->dir = AllocRemapPath(area->path);
    }
    else
    {
        sprintf(tempaka, "%d", area->board);
        thisarea->dir = mem_strdup(tempaka);
    }

    sprintf(tempaka, "-P%hu:%hu/%hu.%hu",
            aka_array[area->flags.aka].main.zone,
            aka_array[area->flags.aka].main.net,
            aka_array[area->flags.aka].main.node,
            aka_array[area->flags.aka].main.point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);

}


// =========================================================

void ana_FE142_area(FENEW_Area * area, FENEW_SysAddress * aka_array)
{
    AREA *thisarea;
    char tempaka[100];

    if (area->flags.storage == FENEW_PASSTHRU)
        return;

    thisarea = init_area();

    if (area->flags.storage == FENEW_SQUISH) /* Squish area */
        thisarea->base = MSGTYPE_SQUISH;
    else if (area->flags.storage == FENEW_JAM)
        thisarea->base = MSGTYPE_JAM;
    else if (area->flags.storage == FENEW_QBBS)
        thisarea->base = MSGTYPE_HMB;
    else if (area->flags.storage == FENEW_FIDO)
        thisarea->base = MSGTYPE_SDM;
    else
    {
        Message("Unknown storage type found in Fastecho area!", -1, 0,
                YES);
        mem_free(thisarea);
        return;
    }

    switch (area->flags.atype)
    {
    case FENEW_AREA_ECHOMAIL:
        thisarea->base |= MSGTYPE_ECHO;
        thisarea->type = ECHOMAIL;
        break;
    case FENEW_AREA_NETMAIL:
        if (!(cfg.usr.status & READNET))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_NET;
        thisarea->type = NETMAIL;
        thisarea->stdattr |= aPRIVATE;
        break;
    default:
        if (!(cfg.usr.status & READLOCAL))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_LOCAL;
        thisarea->type = LOCAL;
        break;
    }

    thisarea->group = area->info.group;

    strlwr(area->name);

    thisarea->tag = mem_strdup(area->name);
    if (area->desc[0] != '\0')
        thisarea->desc = mem_strdup(area->desc);
    else
        thisarea->desc = thisarea->tag;

    if (!(thisarea->base & MSGTYPE_HMB))
    {
        Strip_Trailing(area->path, '\\');
        thisarea->dir = AllocRemapPath(area->path);
    }
    else
    {
        sprintf(tempaka, "%d", area->board);
        thisarea->dir = mem_strdup(tempaka);
    }

    sprintf(tempaka, "-P%hu:%hu/%hu.%hu",
            aka_array[area->info.aka].main.zone,
            aka_array[area->info.aka].main.net,
            aka_array[area->info.aka].main.node,
            aka_array[area->info.aka].main.point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);

}

// =========================================================

void ana_FE141_area(FE141Area * area, SysAddress * aka_array)
{
    AREA *thisarea;
    char tempaka[100];

    if (area->flags.type == PT_BOARD)
        return;

    thisarea = init_area();

    if (area->flags.type == SQUISH) /* Squish area */
        thisarea->base = MSGTYPE_SQUISH;
    else if (area->flags.type == JAM)
        thisarea->base = MSGTYPE_JAM;
    else if (area->flags.type == QBBS)
        thisarea->base = MSGTYPE_HMB;
    else
        thisarea->base = MSGTYPE_SDM;

    switch (area->type)
    {
    case AREA_ECHOMAIL:
        thisarea->base |= MSGTYPE_ECHO;
        thisarea->type = ECHOMAIL;
        break;
    case AREA_NETMAIL:
        if (!(cfg.usr.status & READNET))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_NET;
        thisarea->type = NETMAIL;
        thisarea->stdattr |= aPRIVATE;
        break;
    default:
        if (!(cfg.usr.status & READLOCAL))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_LOCAL;
        thisarea->type = LOCAL;
        break;
    }

    thisarea->group = area->flags.group;

    strlwr(area->name);

    thisarea->tag = mem_strdup(area->name);
    if (area->desc[0] != '\0')
        thisarea->desc = mem_strdup(area->desc);
    else
        thisarea->desc = thisarea->tag;

    if (!(thisarea->base & MSGTYPE_HMB))
    {
        Strip_Trailing(area->path, '\\');
        thisarea->dir = AllocRemapPath(area->path);
    }
    else
    {
        sprintf(tempaka, "%d", area->board);
        thisarea->dir = mem_strdup(tempaka);
    }

    sprintf(tempaka, "-P%hu:%hu/%hu.%hu",
            aka_array[area->flags.aka].main.zone,
            aka_array[area->flags.aka].main.net,
            aka_array[area->flags.aka].main.node,
            aka_array[area->flags.aka].main.point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);

}



/* ------------------------------------------ */


void addmacro(char *name, int usenet)
{

    char temp[40], *usenetptr, *macroptr, *toptr, *toaddrptr, *subjectptr;
    MACROLIST *thismacro;
    static MACROLIST *lastmacro;


    if (name == NULL)
        return;

    memset(temp, '\0', sizeof(temp));

    macroptr = strtok(name, " ,\t");
    if (usenet)
    {
        if ((usenetptr = strtok(NULL, " ,\t")) == NULL)
        {
            sprintf(msg, "Illegal macro format in %s", name);
            Message(msg, -1, 0, YES);
            return;
        }
    }

    toptr = strtok(NULL, ",\n\t\r");
    toaddrptr = strtok(NULL, ",\n\t\r");
    subjectptr = strtok(NULL, "\n\t\r");

    if ((!macroptr) || (!toptr))
    {
        sprintf(msg, "Illegal macro format in %s (line %d)", name,
                globalline);
        Message(msg, -1, 0, YES);
        return;
    }

    thismacro = mem_calloc(1, sizeof(MACROLIST));

    if (usenet)
        strcpy(thismacro->usenet, usenetptr);

    strncpy(thismacro->macro, macroptr, 19);
    strncpy(thismacro->toname, toptr, 99);

    if (toaddrptr)
    {
        address_expand(toaddrptr, &thismacro->toaddress, -1);
        if (subjectptr)
        {
            strncpy(thismacro->subject, subjectptr, 99);
        }
    }

    thismacro->next = NULL;

    if (cfg.firstmacro == NULL) /* first name */
        cfg.firstmacro = thismacro;
    else                        /* establish link */
        lastmacro->next = thismacro;

    lastmacro = thismacro;

}


// ==========================================


void startup(char *what)
{

    if (what == NULL)
        return;


    if (strcmpi(what, "yes") == 0)
        stuff = cASscan;
    else if (strcmpi(what, "personal") == 0)
        stuff = cASscanpersonal;
    else if (strcmpi(what, "no") != 0)
    {
        sprintf(msg, "Invalid value (%s) for Startup_Scan keyword!", what);
        Message(msg, -1, 0, YES);
    }

}


/* Choose from Yes, No or Ask */

int one_of_three(char *what)
{

    if (what != NULL)
    {
        if (strcmpi(what, "no") == 0)
            return 0;
        if (strcmpi(what, "yes") == 0)
            return 1;
        if (strcmpi(what, "ask") == 0)
            return 2;
    }

    sprintf("Error: %s should be 'Yes', 'No' or 'Ask' (line %d)!", what,
            globalline);
    Message(msg, -1, 0, YES);

    return 0;
}


/* Add an entry to the list of passthru tags from squish.cfg */

void add_to_ptlist(char *tag)
{
    PTLIST *curtag;
    static PTLIST *last = NULL;

    curtag = mem_calloc(1, sizeof(PTLIST));

    curtag->tag = mem_strdup(tag);
    if (firstpt == NULL)
        firstpt = curtag;
    else
        last->next = curtag;

    last = curtag;

}


/* See if a tag is in the Passthru list from Squish.cfg */

int checkpt(char *tag)
{
    PTLIST *curtag = firstpt;

    while (curtag)
    {
        if (strcmpi(curtag->tag, tag) == 0)
            return 1;
        curtag = curtag->next;
    }

    return 0;

}

/* mem_free mem allocated for ptlist */

void mem_freeptlist(void)
{
    PTLIST *current;

    while (firstpt)
    {
        current = firstpt->next;
        mem_free(firstpt->tag);
        mem_free(firstpt);
        firstpt = current;
    }

}

/* Read GEcho config files */

void ReadGEchoCFG(void)
{
    SETUP_GE *sys;
    AREAFILE_HDR AreaHdr;
    AREAFILE_GE AreaFile;
    int SetupGE, AreaFileGE;
    word result, records, arearecsize, counter;
    char drive[3], dir[100];
    char temp[120];
    AREA *thisarea;
    int i, new = 0;             // New == newer than GEcho 1.02+

    sys = mem_calloc(1, sizeof(SETUP_GE));

    SetupGE = sopen(GEchocfg, O_RDONLY | O_BINARY, SH_DENYNO);
    if (SetupGE == -1)
    {
        Message("Unable to open SETUP.GE", -1, 0, YES);
        mem_free(sys);
        return;
    }
    result = read(SetupGE, sys, sizeof(SETUP_GE));
    close(SetupGE);

    if (result == (word) - 1)
    {
        Message("Error reading SETUP.GE", -1, 0, YES);
        mem_free(sys);
        return;
    }

    if ((sys->sysrev != GE_THISREV) || (sys->version_major != 1))
    {
        Message("System file revision level mismatch", -1, 0, YES);
        mem_free(sys);
        return;
    }

    if (sys->version_minor < 1)
    {
        Message("This is an older, unsupported version of GEcho.", -1, 0,
                YES);
        mem_free(sys);
        return;
    }

    if (sys->version_minor > 2)
        new = 1;

/*
**  Opening AREAFILE.GE and checking the header
*/
    fnsplit(GEchocfg, drive, dir, NULL, NULL);
    fnmerge(temp, drive, dir, "areafile", "ge");
    mem_free(GEchocfg);
    GEchocfg = mem_strdup(temp);

    AreaFileGE = sopen(GEchocfg, O_RDONLY | O_BINARY, SH_DENYNO);
    if (AreaFileGE == -1)
    {
        Message("Unable to open AREAFILE.GE", -1, 0, YES);
        return;
    }
    result = read(AreaFileGE, &AreaHdr, sizeof(AREAFILE_HDR));
    if (result < sizeof(AREAFILE_HDR))
    {
        Message("Error reading AREAFILE header", -1, 0, YES);
        return;
    }

    sprintf(temp, "(%s)", GEchocfg);
    if (strlen(temp) > 33)
    {
        temp[33] = '\0';
        temp[32] = '.';
        temp[31] = '.';
    }
    print(14, 36, 7, temp);


    if (cfg.usr.status & READNET)
    {
        thisarea = init_area();
        thisarea->base = MSGTYPE_SDM | MSGTYPE_NET;
        thisarea->type = NETMAIL;
        thisarea->stdattr |= aPRIVATE;
        thisarea->tag = thisarea->desc = mem_strdup("Netmail");
        thisarea->dir = AllocRemapPath(sys->mailpath);
        AddLinkedArea(thisarea);

        if (!new)
        {
            for (i = 0; i < OLDAKAS; i++)
            {
                if (sys->oldakaboard[i] != 0)
                {
                    thisarea = init_area();
                    thisarea->base = MSGTYPE_HMB | MSGTYPE_NET;
                    thisarea->type = NETMAIL;
                    thisarea->stdattr |= aPRIVATE;
                    sprintf(temp, "-P%hu:%hu/%hu.%hu", sys->oldaka[i].zone,
                            sys->oldaka[i].net,
                            sys->oldaka[i].node, sys->oldaka[i].point);

                    check_aka(thisarea, temp);
                    sprintf(temp, "Netmail (%hu:%hu/%hu.%hu)",
                            sys->oldaka[i].zone, sys->oldaka[i].net,
                            sys->oldaka[i].node, sys->oldaka[i].point);

                    thisarea->desc = mem_strdup(temp);
                    sprintf(temp, "net%d", i);
                    thisarea->tag = mem_strdup(temp);
                    itoa(sys->oldakaboard[i], temp, 10);
                    thisarea->dir = AllocRemapPath(temp);
                    AddLinkedArea(thisarea);
                }
            }

            if ((sys->rcvdmailpath[0] != '\0') || (sys->oldrcvdboard != 0))
            {
                thisarea = init_area();
                if (sys->oldrcvdboard != 0)
                {
                    thisarea->base = MSGTYPE_HMB | MSGTYPE_NET;
                    itoa(sys->oldrcvdboard, temp, 10);
                    thisarea->dir = mem_strdup(temp);
                }
                else
                {
                    thisarea->base = MSGTYPE_SDM | MSGTYPE_NET;
                    thisarea->dir = AllocRemapPath(sys->rcvdmailpath);
                }

                thisarea->type = NETMAIL;
                thisarea->tag = thisarea->desc =
                    mem_strdup("Received_Netmail");
                AddLinkedArea(thisarea);
            }

            if ((sys->sentmailpath[0] != '\0') || (sys->oldsentboard != 0))
            {
                thisarea = init_area();
                if (sys->oldsentboard != 0)
                {
                    thisarea->base = MSGTYPE_HMB | MSGTYPE_NET;
                    itoa(sys->oldsentboard, temp, 10);
                    thisarea->dir = mem_strdup(temp);
                }
                else
                {
                    thisarea->base = MSGTYPE_SDM | MSGTYPE_NET;
                    thisarea->dir = AllocRemapPath(sys->sentmailpath);
                }

                thisarea->type = NETMAIL;
                thisarea->tag = thisarea->desc =
                    mem_strdup("Sent_Netmail");
                AddLinkedArea(thisarea);
            }
        }
        else                    // GEcho 1.10 of hoger
        {
            // nothing..
        }
    }

    if (cfg.usr.status & READLOCAL)
    {
        if (!new)
        {
            if ((sys->badecho_path[0] != '\0') || (sys->oldbadboard != 0))
            {
                thisarea = init_area();
                if (sys->oldbadboard != 0)
                {
                    thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
                    itoa(sys->oldbadboard, temp, 10);
                    thisarea->dir = mem_strdup(temp);
                }
                else
                {
                    thisarea->base = MSGTYPE_SDM | MSGTYPE_LOCAL;
                    thisarea->dir = AllocRemapPath(sys->badecho_path);
                }

                thisarea->type = LOCAL;
                thisarea->tag = thisarea->desc =
                    mem_strdup("Bad_Messages");
                AddLinkedArea(thisarea);
            }

            if (sys->olddupboard != 0)
            {
                thisarea = init_area();
                thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
                itoa(sys->olddupboard, temp, 10);
                thisarea->dir = mem_strdup(temp);

                thisarea->type = LOCAL;
                thisarea->tag = thisarea->desc = mem_strdup("Dupes");
                AddLinkedArea(thisarea);
            }

            if (sys->recoveryboard != 0)
            {
                thisarea = init_area();
                thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
                itoa(sys->recoveryboard, temp, 10);
                thisarea->dir = mem_strdup(temp);

                thisarea->type = LOCAL;
                thisarea->tag = thisarea->desc = mem_strdup("Recovery");
                AddLinkedArea(thisarea);
            }


            if ((sys->persmail_path[0] != '\0')
                || (sys->oldpersmailboard[0] != 0))
            {
                thisarea = init_area();
                if (sys->oldpersmailboard[0] != 0)
                {
                    thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
                    itoa(sys->oldpersmailboard[0], temp, 10);
                    thisarea->dir = mem_strdup(temp);
                }
                else
                {
                    thisarea->base = MSGTYPE_SDM | MSGTYPE_LOCAL;
                    thisarea->dir = AllocRemapPath(sys->persmail_path);
                }

                thisarea->type = LOCAL;
                thisarea->tag = thisarea->desc =
                    mem_strdup("Personal_Messages");
                AddLinkedArea(thisarea);
            }
        }
        else
        {
            // We do this later, after we read areafile.ge, to see if the
            // areas are already defined there..
        }
    }


/*   if (AreaHdr.recsize < sizeof(AREAFILE_GE))
   {
      Message("Incompatible AREAFILE record size", -1, 0, YES);
      return;
   } */

/*
**  Reading AREAFILE.GE
**
**  Method 2: Sequentially reading
**  This will read the area records in the order in which they area stored.
**  You will have to check each record to see if it has been removed.
*/

    arearecsize =
        AreaHdr.maxconnections * sizeof(CONNECTION) + AreaHdr.recsize;
    records = (filelength(AreaFileGE) - AreaHdr.hdrsize) / arearecsize;

    for (counter = 0; counter < records; counter++)
    {
        if (!(counter % 5))
            working(14, 7, 7);
        lseek(AreaFileGE,
              (long)AreaHdr.hdrsize + (long)arearecsize * counter,
              SEEK_SET);
        result = read(AreaFileGE, &AreaFile, sizeof(AREAFILE_GE));
        if (result < sizeof(AREAFILE_GE))
            break;
        if ((AreaFile.options & REMOVED) == 0)
            ana_GECHO_area(&AreaFile,
                           new ? (GE_ADDRESS *) & sys->
                           aka : (GE_ADDRESS *) & sys->oldaka);
    }

    close(AreaFileGE);

    // Now we check for the bad messages. If the boardnumber is not yet
    // used,
    // we add it ourself. If it _is_ used, it was apparently already
    // present
    // in areafile.ge, so we don't have to add it ourself.


    if (cfg.usr.status & READLOCAL)
    {
        if (new)
        {
            if (sys->badarea > 0 && sys->badarea < 201
                && BoardNotUsed(sys->badarea))
            {
                thisarea = init_area();
                thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
                itoa(sys->badarea, temp, 10);
                thisarea->dir = mem_strdup(temp);

                thisarea->type = LOCAL;
                thisarea->tag = thisarea->desc =
                    mem_strdup("Bad_Messages");
                AddLinkedArea(thisarea);
            }

            if (sys->recoveryboard != 0
                && BoardNotUsed(sys->recoveryboard))
            {
                thisarea = init_area();
                thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
                itoa(sys->recoveryboard, temp, 10);
                thisarea->dir = mem_strdup(temp);

                thisarea->type = LOCAL;
                thisarea->tag = thisarea->desc = mem_strdup("Recovery");
                AddLinkedArea(thisarea);
            }
        }
    }


    mem_free(sys);
    print(14, 7, 9, "û");

}

/* Analyse area found in GEcho areafile */

void ana_GECHO_area(AREAFILE_GE * area, GE_ADDRESS * aka_array)
{
    AREA *thisarea;
    char tempaka[100];

    if (area->areaformat == FORMAT_PT || area->areaformat == FORMAT_PCB
        || area->areaformat > (NUM_FORMATS - 1))
        return;

    thisarea = init_area();

    thisarea->group = area->group;

    strlwr(area->name);

    thisarea->tag = mem_strdup(area->name);
    if (area->comment[0] != '\0')
        thisarea->desc = mem_strdup(area->comment);
    else
        thisarea->desc = thisarea->tag;

    if (area->areaformat == FORMAT_SQUISH) /* Squish area */
        thisarea->base = MSGTYPE_SQUISH;
    else if (area->areaformat == FORMAT_JAM)
        thisarea->base = MSGTYPE_JAM;
    else if (area->areaformat == FORMAT_HMB)
        thisarea->base = MSGTYPE_HMB;
    else
        thisarea->base = MSGTYPE_SDM;

    switch (area->areatype)
    {
    case GE_ECHOMAIL:
        thisarea->base |= MSGTYPE_ECHO;
        thisarea->type = ECHOMAIL;
        break;

    case GE_NETMAIL:
        if (!(cfg.usr.status & READNET))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_NET;
        thisarea->type = NETMAIL;
        thisarea->stdattr |= aPRIVATE;
        break;

    default:
        if (!(cfg.usr.status & READLOCAL))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_LOCAL;
        thisarea->type = LOCAL;
        break;
    }

    if (thisarea->base & MSGTYPE_HMB)
    {
        itoa(area->areanumber, tempaka, 10);
        thisarea->dir = mem_strdup(tempaka);
    }
    else
    {
        Strip_Trailing(area->path, '\\');
        thisarea->dir = AllocRemapPath(area->path);
    }

    sprintf(tempaka, "-P%hu:%hu/%hu.%hu", aka_array[area->pkt_origin].zone,
            aka_array[area->pkt_origin].net,
            aka_array[area->pkt_origin].node,
            aka_array[area->pkt_origin].point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);
}



// void ReadImailCFG(void)
// {
//    im_config_type *imcfg;
//    int cfgfile;
//    areas_record_type area;
//    char temp[120];
//    char drive[MAXDRIVE], dir[MAXDIR];
//    int counter=0;
//    AREA *thisarea;
//
//    if( (cfgfile=sopen(Imailcfg, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
//       {
//       sprintf(msg, "Can't open %s!", Imailcfg);
//       Message(msg, -1, 0, YES);
//       return;
//       }
//
//    sprintf(temp, "(%s)", Imailcfg);
//    if(strlen(temp) > 33)
//       {
//       temp[33]='\0';
//       temp[32]='.';
//       temp[31]='.';
//       }
//    print(14,36,7,temp);
//
//    imcfg = mem_calloc(1, sizeof(im_config_type));
//
//    if(read(cfgfile, imcfg, sizeof(im_config_type)) != sizeof(im_config_type))
//       {
//       sprintf(msg, "Error reading %s!", Imailcfg);
//       Message(msg, -1, 0, YES);
//       close(cfgfile);
//       mem_free(imcfg);
//       return;
//       }
//
//    close(cfgfile);
//
//    if(cfg.usr.status & READNET)
//      {
//      if(imcfg->netmail[0] != '\0')
//        {
//        thisarea = init_area();
//        thisarea->base = MSGTYPE_SDM | MSGTYPE_NET;
//        thisarea->type = NETMAIL;
//        thisarea->stdattr |= aPRIVATE;
//        thisarea->tag = thisarea->desc = mem_strdup("Netmail");
//        thisarea->dir = AllocRemapPath(imcfg->netmail);
//        AddLinkedArea(thisarea);
//        }
//      }
//
//
//    fnsplit(Imailcfg, drive, dir, NULL, NULL);
//    fnmerge(temp, drive, dir, "imail", "ar");
//    mem_free(Imailcfg);
//    Imailcfg = mem_strdup(temp);
//
//    if( (cfgfile=sopen(Imailcfg, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
//       {
//       sprintf(msg, "Can't open %s!", Imailcfg);
//       Message(msg, -1, 0, YES);
//       return;
//       }
//
//
//    while(read(cfgfile, &area, sizeof(area)) == sizeof(area) )
//         {
//         if(!(counter++ % 5)) working(14,7,7);
//         ana_Imail_area(&area, &imcfg->aka);
//         }
//
//
//    close(cfgfile);
//    mem_free(imcfg);
//
//    print(14,7,9,"û");
//
// }
//
//
// /* ------------------------------------------ */
//
// void ana_Imail_area(areas_record_type *area, naddress *aka_array)
// {
//    AREA  *thisarea;
//    char tempaka[100];
//
//
//    if( (area->deleted) || (!area->active) )
//         return;
//
//    if( IsPassth(area->msg_base_type) )
//        return;
//
//    thisarea = init_area();
//
//    if (IsSquish(area->msg_base_type))    /* Squish area */
//         thisarea->base = MSGTYPE_SQUISH;
//    else if(IsJam(area->msg_base_type))
//         thisarea->base = MSGTYPE_JAM;
//    else if( (IsQbbs(area->msg_base_type)) ||
//             (IsHudson(area->msg_base_type)) )
//         thisarea->base = MSGTYPE_HMB;
//    else
//         thisarea->base = MSGTYPE_SDM;
//
//    thisarea->group = area->group;
//
//    if(area->msg_base_type & IMSGTYPE_ECHO)
//       {
//       thisarea->base |= MSGTYPE_ECHO;
//       thisarea->type = ECHOMAIL;
//       }
//    else if(area->msg_base_type & IMSGTYPE_NET)
//       {
//       if(cfg.usr.status & READNET)
//          {
//          thisarea->base |= MSGTYPE_NET;
//          thisarea->type = NETMAIL;
//          thisarea->stdattr |= aPRIVATE;
//          }
//       else
//         {
//         mem_free(thisarea);
//         return;
//         }
//       }
//    else
//       {
//       if(cfg.usr.status & READLOCAL)
//          {
//          thisarea->type = LOCAL;
//          thisarea->base |= MSGTYPE_LOCAL;
//          }
//       else
//         {
//         mem_free(thisarea);
//         return;
//         }
//       }
//
//
//    strlwr(area->aname);
//
//    thisarea->tag  = mem_strdup(area->aname);
//    if(area->comment[0] != '\0')
//         thisarea->desc = mem_strdup(area->comment);
//    else
//         thisarea->desc = thisarea->tag;
//
//    if(!(thisarea->base & MSGTYPE_HMB))  // We need a path.
//      {
//      Strip_Trailing(area->msg_path, '\\');
//      thisarea->dir = AllocRemapPath(area->msg_path);
//      }
//    else   // We need a board.
//      {
//      itoa(area->brd, tempaka, 10);
//      thisarea->dir = mem_strdup(tempaka);
//      }
//
//
//    sprintf(tempaka, "-P%hu:%hu/%hu.%hu", aka_array[area->o_addr-1].zone,
//                                      aka_array[area->o_addr-1].net,
//                                      aka_array[area->o_addr-1].node,
//                                      aka_array[area->o_addr-1].point);
//
//    check_aka(thisarea, tempaka);
//
//    AddLinkedArea(thisarea);
// }
//
// ==============================================================

void ReadxMailCFG()
{
    FILE *cfgfile;
    xMailArea area;
    char temp[100];
    word areano = 0;
    int counter = 0;


    if ((cfgfile = _fsopen(xMailcfg, "rb", SH_DENYNO)) == NULL)
    {
        sprintf(msg, "Can't open %s!", xMailcfg);
        Message(msg, -1, 0, YES);
        return;
    }

    setvbuf(cfgfile, NULL, _IOFBF, 4096);

    sprintf(temp, "(%s)", xMailcfg);
    if (strlen(temp) > 33)
    {
        temp[33] = '\0';
        temp[32] = '.';
        temp[31] = '.';
    }
    print(14, 36, 7, temp);

    while (fread(&area, sizeof(area), 1, cfgfile) == 1)
    {
        if (!(counter++ % 5))
            working(14, 7, 7);
        ana_xMail_area(&area, ++areano);
    }

    fclose(cfgfile);

    print(14, 7, 9, "û");


}


// ==============================================================

void ReadFmailCFG()
{
    configType *fmcfg = NULL;
    FM12configType *fm12cfg = NULL;
    int cfgfile;
    rawEchoTypeOld oldarea;
    rawEchoType area;
    char temp[120];
    char drive[_MAX_DRIVE], dir[_MAX_DIR];
    int counter = 0;
    headerType hdr;
    AREA *thisarea;
    int i, n;
    long pos;
    size_t oldcfgsize = sizeof(configType);
    size_t fm12cfgsize = sizeof(FM12configType);
    int isFmail120 = 0;

    if ((cfgfile = sopen(Fmailcfg, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
    {
        sprintf(msg, "Can't open %s!", Fmailcfg);
        Message(msg, -1, 0, YES);
        return;
    }

    sprintf(temp, "(%s)", Fmailcfg);
    if (strlen(temp) > 33)
    {
        temp[33] = '\0';
        temp[32] = '.';
        temp[31] = '.';
    }
    print(14, 36, 7, temp);

    if (filelength(cfgfile) > oldcfgsize)
    {
        fm12cfg = mem_calloc(1, fm12cfgsize);
        isFmail120 = 1;
    }

    fmcfg = mem_calloc(1, oldcfgsize);

    if (isFmail120)
    {
        if (read(cfgfile, fm12cfg, fm12cfgsize) != fm12cfgsize)
        {
            sprintf(msg, "Error reading %s!", Fmailcfg);
            Message(msg, -1, 0, YES);
            close(cfgfile);
            mem_free(fm12cfg);
            mem_free(fmcfg);
            return;
        }
        memcpy(fmcfg, fm12cfg, oldcfgsize); // We make a copy for the old
                                            // stuff..
    }
    else
    {
        if (read(cfgfile, fmcfg, oldcfgsize) != oldcfgsize)
        {
            sprintf(msg, "Error reading %s!", Fmailcfg);
            Message(msg, -1, 0, YES);
            close(cfgfile);
            mem_free(fmcfg);
            return;
        }
    }

    close(cfgfile);

    if (cfg.usr.status & READNET)
    {
        if (fmcfg->netPath[0] != '\0')
        {
            thisarea = init_area();
            thisarea->base = MSGTYPE_SDM | MSGTYPE_NET;
            thisarea->dir = AllocRemapPath(fmcfg->netPath);

            thisarea->type = NETMAIL;
            thisarea->stdattr |= aPRIVATE;
            thisarea->tag = thisarea->desc = mem_strdup("Netmail");
            AddLinkedArea(thisarea);
        }

        for (i = 0; i < FMMAX_AKAS; i++)
        {
            if (fmcfg->netmailBoard[i] != 0)
            {
                thisarea = init_area();
                thisarea->base = MSGTYPE_HMB | MSGTYPE_NET;
                thisarea->type = NETMAIL;
                thisarea->stdattr |= aPRIVATE;
                sprintf(temp, "-P%hu:%hu/%hu.%hu",
                        fmcfg->akaList[i].nodeNum.zone,
                        fmcfg->akaList[i].nodeNum.net,
                        fmcfg->akaList[i].nodeNum.node,
                        fmcfg->akaList[i].nodeNum.point);

                check_aka(thisarea, temp);
                sprintf(temp, "Netmail (%hu:%hu/%hu.%hu)",
                        fmcfg->akaList[i].nodeNum.zone,
                        fmcfg->akaList[i].nodeNum.net,
                        fmcfg->akaList[i].nodeNum.node,
                        fmcfg->akaList[i].nodeNum.point);

                thisarea->desc = mem_strdup(temp);
                sprintf(temp, "net%d", i);
                thisarea->tag = mem_strdup(temp);
                itoa(fmcfg->netmailBoard[i], temp, 10);
                thisarea->dir = mem_strdup(temp);
                AddLinkedArea(thisarea);
            }
        }


        if (fmcfg->rcvdPath[0] != '\0')
        {
            thisarea = init_area();
            thisarea->base = MSGTYPE_SDM | MSGTYPE_NET;
            thisarea->dir = AllocRemapPath(fmcfg->rcvdPath);

            thisarea->type = NETMAIL;
            thisarea->tag = thisarea->desc =
                mem_strdup("Received_Netmail");
            AddLinkedArea(thisarea);
        }

        if (fmcfg->sentPath[0] != '\0')
        {
            thisarea = init_area();
            thisarea->base = MSGTYPE_SDM | MSGTYPE_NET;
            thisarea->dir = AllocRemapPath(fmcfg->sentPath);

            thisarea->type = NETMAIL;
            thisarea->tag = thisarea->desc = mem_strdup("Sent_Netmail");
            AddLinkedArea(thisarea);
        }
    }

    if (cfg.usr.status & READLOCAL)
    {
        if (fmcfg->pmailPath[0] != '\0')
        {
            thisarea = init_area();
            thisarea->base = MSGTYPE_SDM | MSGTYPE_LOCAL;
            thisarea->dir = AllocRemapPath(fmcfg->pmailPath);

            thisarea->type = LOCAL;
            thisarea->tag = thisarea->desc = mem_strdup("Personal_Mail");
            AddLinkedArea(thisarea);
        }

        if (fmcfg->recBoard != 0)
        {
            thisarea = init_area();
            thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
            thisarea->type = LOCAL;
            thisarea->tag = thisarea->desc = mem_strdup("Recovery");
            itoa(fmcfg->recBoard, temp, 10);
            thisarea->dir = mem_strdup(temp);
            AddLinkedArea(thisarea);
        }

        if (fmcfg->badBoard != 0)
        {
            thisarea = init_area();
            thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
            thisarea->type = LOCAL;
            thisarea->tag = thisarea->desc = mem_strdup("Bad_Messages");
            itoa(fmcfg->badBoard, temp, 10);
            thisarea->dir = mem_strdup(temp);
            AddLinkedArea(thisarea);
        }

        if (fmcfg->dupBoard != 0)
        {
            thisarea = init_area();
            thisarea->base = MSGTYPE_HMB | MSGTYPE_LOCAL;
            thisarea->type = LOCAL;
            thisarea->tag = thisarea->desc = mem_strdup("Dupes");
            itoa(fmcfg->dupBoard, temp, 10);
            thisarea->dir = mem_strdup(temp);
            AddLinkedArea(thisarea);
        }

    }

//   if( (fmcfg->versionMajor == 0) && (fmcfg->versionMinor < 97) )
//       oldtype = 1;   Didin't work on .97 beta, identified as 0.94!

    fnsplit(Fmailcfg, drive, dir, NULL, NULL);
    fnmerge(temp, drive, dir, "fmail", "ar");
    mem_free(Fmailcfg);
    Fmailcfg = mem_strdup(temp);

    if ((cfgfile = sopen(Fmailcfg, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
    {
        sprintf(msg, "Can't open %s!", Fmailcfg);
        Message(msg, -1, 0, YES);
        return;
    }

    if (read(cfgfile, &hdr, sizeof(headerType)) != sizeof(headerType))
    {
        sprintf(msg, "Error reading %s!", Fmailcfg);
        Message(msg, -1, 0, YES);
        close(cfgfile);
        return;
    }

    if (strncmpi(hdr.versionString, "fmail", 5) != 0) // Old type?
    {
        lseek(cfgfile, 0L, SEEK_SET);

        while (read(cfgfile, &oldarea, sizeof(oldarea)) == sizeof(oldarea))
        {
            if (!(counter++ % 5))
                working(14, 7, 7);
            old_ana_Fmail_area(&oldarea, &fmcfg->akaList);
        }
    }
    else
    {
        n = 0;
        while (n < hdr.totalRecords)
        {
            pos =
                (long)sizeof(headerType) +
                (long)((long)n * (long)hdr.recordSize);
            lseek(cfgfile, pos, SEEK_SET);
            if (read(cfgfile, &area, sizeof(area)) != sizeof(area))
            {
                Message("Error reading Fmail area file!", -1, 0, YES);
                break;
            }
            if (!(counter++ % 5))
                working(14, 7, 7);

            if (isFmail120)
                ana_Fmail_area(&area, &fm12cfg->akaList);
            else
                ana_Fmail_area(&area, &fmcfg->akaList);
            n++;
        }
    }

    close(cfgfile);
    mem_free(fmcfg);
    if (fm12cfg)
        mem_free(fm12cfg);

    print(14, 7, 9, "û");

}

// ==============================================================


void old_ana_Fmail_area(rawEchoTypeOld * area, nodeFakeType * aka_array)
{
    AREA *thisarea;
    char tempaka[100];


    if ((!area->options.active) || (area->options.disconnected))
        return;

    if (area->options.local && (!(cfg.usr.status & READLOCAL)))
        return;

    if (area->board == 0)
        return;                 /* Passthru */

    /* Passthru? */

    thisarea = init_area();

    strlwr(area->areaName);

    thisarea->tag = mem_strdup(area->areaName);
    if (area->comment[0] != '\0')
        thisarea->desc = mem_strdup(area->comment);
    else
        thisarea->desc = thisarea->tag;

    itoa(area->board, tempaka, 10);
    thisarea->dir = mem_strdup(tempaka);

    thisarea->base = MSGTYPE_HMB;
    if (area->options.local)
    {
        thisarea->base |= MSGTYPE_LOCAL;
        thisarea->type = LOCAL;
    }
    else
    {
        thisarea->type = ECHOMAIL;
        thisarea->base |= MSGTYPE_ECHO;
    }

    thisarea->group = (byte) area->group;

    sprintf(tempaka, "-P%hu:%hu/%hu.%hu",
            aka_array[area->address].nodeNum.zone,
            aka_array[area->address].nodeNum.net,
            aka_array[area->address].nodeNum.node,
            aka_array[area->address].nodeNum.point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);
}

// ==============================================================


void ana_Fmail_area(rawEchoType * area, nodeFakeType * aka_array)
{
    AREA *thisarea;
    char tempaka[100];


    if ((!area->options.active) || (area->options.disconnected))
        return;

    if (area->options.local && (!(cfg.usr.status & READLOCAL)))
        return;

    if ((area->board == 0) && (area->msgBasePath[0] == '\0'))
        return;                 /* Passthru */

    /* Passthru? */

    thisarea = init_area();

    strlwr(area->areaName);

    thisarea->tag = mem_strdup(area->areaName);
    if (area->comment[0] != '\0')
        thisarea->desc = mem_strdup(area->comment);
    else
        thisarea->desc = thisarea->tag;

    if ((area->board != 0) && (area->msgBasePath[0] == '\0'))
    {
        itoa(area->board, tempaka, 10);
        thisarea->dir = mem_strdup(tempaka);
        thisarea->base = MSGTYPE_HMB;
    }
    else
    {
        thisarea->dir = AllocRemapPath(area->msgBasePath);
        thisarea->base = MSGTYPE_JAM;
    }

    if (area->options.local)
    {
        thisarea->type = LOCAL;
        thisarea->base |= MSGTYPE_LOCAL;
    }
    else
    {
        thisarea->type = ECHOMAIL;
        thisarea->base |= MSGTYPE_ECHO;
    }

    thisarea->group = (byte) area->group;


    sprintf(tempaka, "-P%hu:%hu/%hu.%hu",
            aka_array[area->address].nodeNum.zone,
            aka_array[area->address].nodeNum.net,
            aka_array[area->address].nodeNum.node,
            aka_array[area->address].nodeNum.point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);
}


// ==============================================================

void ReadWtrCFG()
{
    int cfgfile;
    WTRAREA area;
    char temp[100];
    int counter = 0;
    char drive[_MAX_DRIVE], dir[_MAX_DIR];
    WTRAKA akalist[20];

    if ((cfgfile =
         sopen(Wtrgatecfg, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
    {
        sprintf(msg, "Can't open %s!", Wtrgatecfg);
        Message(msg, -1, 0, YES);
        return;
    }

    sprintf(temp, "(%s)", Wtrgatecfg);
    if (strlen(temp) > 33)
    {
        temp[33] = '\0';
        temp[32] = '.';
        temp[31] = '.';
    }
    print(14, 36, 7, temp);

    lseek(cfgfile, 333, SEEK_SET);

    if (read(cfgfile, &akalist, sizeof(akalist)) != sizeof(akalist))
    {
        sprintf(msg, "Error reading %s!", Wtrgatecfg);
        Message(msg, -1, 0, YES);
        close(cfgfile);
        return;
    }

    close(cfgfile);

    fnsplit(Wtrgatecfg, drive, dir, NULL, NULL);
    fnmerge(temp, drive, dir, "areabase", "tdb");
    Wtrgatecfg = mem_strdup(temp);

    if ((cfgfile =
         sopen(Wtrgatecfg, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
    {
        sprintf(msg, "Can't open %s!", Fmailcfg);
        Message(msg, -1, 0, YES);
        return;
    }

    lseek(cfgfile, 26L, SEEK_SET);

    while (read(cfgfile, &area, sizeof(area)) == sizeof(area))
    {
        if (!(counter++ % 5))
            working(14, 7, 7);
        ana_wtr_area(&area, &akalist);
    }

    close(cfgfile);

    print(14, 7, 9, "û");


}


// ==============================================================

void ana_wtr_area(WTRAREA * area, WTRAKA * akalist)
{
    AREA *thisarea;
    char tempaka[100];
    char *tmp;

    if (area->deleted || area->fidoname[0] == '\0'
        || area->base == WTRPASSTHRU)
        return;

    /* Passthru? */

    thisarea = init_area();

    if (area->base == WTRSQUISH)
        thisarea->base = MSGTYPE_SQUISH;
    else if (area->base == WTRJAM)
        thisarea->base = MSGTYPE_JAM;
    else if (area->base == WTRSDM)
        thisarea->base = MSGTYPE_SDM;

    switch (area->type)
    {
    case WTRECHO:
        thisarea->base |= MSGTYPE_ECHO;
        thisarea->type = ECHOMAIL;
        break;
    case WTRNET:
        if (!(cfg.usr.status & READNET))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_NET;
        thisarea->type = NETMAIL;
        thisarea->stdattr |= aPRIVATE;
        break;
    default:
        if (!(cfg.usr.status & READLOCAL))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_LOCAL;
        thisarea->type = LOCAL;
        break;
    }

    thisarea->group = area->group;

    thisarea->tag = xpstrdup(area->fidoname);
    strlwr(thisarea->tag);
    if (area->desc[0] != '\0')
        thisarea->desc = xpstrdup(area->desc);
    else
        thisarea->desc = thisarea->tag;

    tmp = xpstrdup(area->path);
    thisarea->dir = AllocRemapPath(tmp);
    mem_free(tmp);

    sprintf(tempaka, "-P%hu:%hu/%hu.%hu",
            akalist[area->OriginAka - 1].zone,
            akalist[area->OriginAka - 1].net,
            akalist[area->OriginAka - 1].node,
            akalist[area->OriginAka - 1].point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);
}





// ==============================================================

void ana_xMail_area(xMailArea * area, word areano)
{
    AREA *thisarea;
    char tempaka[100];
    char *tmp;

    if (area->name[0] == '\0')
        return;

    if (area->base == xPT)
        return;

    /* Passthru? */

    thisarea = init_area();

    if (area->base == xHMB)
        thisarea->base = MSGTYPE_HMB;
    if (area->base == xSQ)
        thisarea->base = MSGTYPE_SQUISH;
    if (area->base == xJAM)
        thisarea->base = MSGTYPE_JAM;
    if (area->base == xSDM)
        thisarea->base = MSGTYPE_SDM;


    switch (area->type)
    {
    case xECHO:
        thisarea->base |= MSGTYPE_ECHO;
        thisarea->type = ECHOMAIL;
        break;
    case xNET:
        if (!(cfg.usr.status & READNET))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_NET;
        thisarea->type = NETMAIL;
        thisarea->stdattr |= aPRIVATE;
        break;
    default:
        if (!(cfg.usr.status & READLOCAL))
        {
            mem_free(thisarea);
            return;
        }
        thisarea->base |= MSGTYPE_LOCAL;
        thisarea->type = LOCAL;
        break;
    }

    thisarea->group = area->group;

    thisarea->tag = xpstrdup(area->name);
    strlwr(thisarea->tag);
    if (area->desc[0] != '\0')
        thisarea->desc = xpstrdup(area->desc);
    else
        thisarea->desc = thisarea->tag;

    if (thisarea->base & MSGTYPE_HMB)
    {
        itoa(areano, tempaka, 10);
        thisarea->dir = mem_strdup(tempaka);
    }
    else
    {
        tmp = xpstrdup(area->dir);
        thisarea->dir = AllocRemapPath(tmp);
        mem_free(tmp);
    }

    sprintf(tempaka, "-P%hu:%hu/%hu.%hu", area->aka.zone,
            area->aka.net, area->aka.node, area->aka.point);

    check_aka(thisarea, tempaka);

    AddLinkedArea(thisarea);
}


// ==============================================================


char *xpstrdup(char *str)
{
    char *out;

    out = mem_calloc(1, str[0] + 1);

    memcpy(out, str + 1, str[0]);

    return out;

}

// ==============================================================


void ReadConfigFile(char *temp, int showname)
{
    XFILE *cfgfile;
    char *line;
    char show[120];
    int counter = 0, number = 0;


    if (showname)
    {
        print(18, 7, 9, "û");
        sprintf(show, "(%s)", temp);
        if (strlen(show) > 36)
        {
            show[36] = '\0';
            show[35] = ')';
            show[34] = '.';
            show[33] = '.';
        }
        sprintf(msg, "%-36.36s", show);
        print(18, 30, 7, msg);
    }

    if ((cfgfile = xopen(temp)) == NULL)
    {
        sprintf(msg, " Darn! I can't open %s! ", temp);
        Message(msg, -1, 254, YES);
    }

    while ((line = xgetline(cfgfile)) != NULL)
    {
        if (showname)
            if (!(counter++ % 5))
                working(18, 7, 7);

        number++;

        if ((line[0] == ';') || (strlen(line) < 5))
            continue;

        analyse(line, number);
    }

    xclose(cfgfile);

    if (showname)
        print(18, 7, 9, "û");


}



/* ---------------------------------------------------- */
/* - Routine to add a message area to the linked list - */
/* - It will also check if it is a dupe               - */
/* ---------------------------------------------------- */

void AddLinkedArea(AREA * thisarea)
{
    AREA *curarea = cfg.first;

    if (thisarea->type == 0)
    {
        Message("No type!!", -1, 0, YES);
    }

    while (curarea)
    {
        if (strcmpi(curarea->tag, thisarea->tag) == 0) /* Dupe! */
        {
            if ((curarea->base & MSGTYPE_SDM)
                && (thisarea->base & MSGTYPE_SQUISH))
            {
                // It is a Squish area, we just didn't know!
                curarea->base &= ~MSGTYPE_SDM;
                curarea->base |= MSGTYPE_SQUISH;
            }

            if (thisarea->desc && (thisarea->desc != thisarea->tag))
                mem_free(thisarea->desc);
            if (thisarea->tag)
                mem_free(thisarea->tag);
            if (thisarea->dir)
                mem_free(thisarea->dir);
            mem_free(thisarea);
            return;
        }
        curarea = curarea->next;
    }

    if ((cfg.first == NULL) || (cfg.usr.alistsort[0] == '\0')) // No
                                                               // sorting..
    {
        if (cfg.first == NULL)  /* first area */
            cfg.first = thisarea;
        else                    /* establish link */
        {
            last->next = thisarea;
            thisarea->prev = last;
        }

        last = thisarea;
        return;
    }

    for (curarea = cfg.first; curarea; curarea = curarea->next)
    {
        if (AreaCompare(thisarea, curarea) < 0) // thisarea < curarea?
            break;
    }

    if (!curarea)               // Append at end..
    {
        last->next = thisarea;
        thisarea->prev = last;
        last = thisarea;
    }
    else if (curarea == cfg.first) // New first area..
    {
        thisarea->next = cfg.first;
        cfg.first->prev = thisarea;
        cfg.first = thisarea;
    }
    else                        // Insert in middle..
    {
        thisarea->prev = curarea->prev;
        thisarea->prev->next = thisarea;
        curarea->prev = thisarea;
        thisarea->next = curarea;
    }

}

// ==============================================================
// Select startup area selection view


void startupmode(char *s)
{
    if (!strcmpi(s, "new"))
        cfg.mode = SHOWNEW;
    else if (!strcmpi(s, "tagged"))
        cfg.mode = SHOWTAGGED;
    else if (!strcmpi(s, "newtagged"))
        cfg.mode = SHOWNEWTAGGED;
    else if (!strcmpi(s, "all"))
        cfg.mode = SHOWALL;
    else
    {
        sprintf(msg, "Unknow startup mode: %s", s);
        Message(msg, -1, 0, YES);
    }
}


// ==============================================================


void freqattribs(char *s)
{

    while (*s != '\0')
    {
        switch (*s)
        {
        case 'c':
        case 'C':
            cfg.usr.frqattr |= aCRASH;
            break;

        case 'k':
        case 'K':
            cfg.usr.frqattr |= aKILL;
            break;

        case 'd':
        case 'D':
            cfg.usr.frqattr |= aDIR;
            break;

        case 'i':
        case 'I':
            cfg.usr.frqattr |= aIMM;
            break;

        case 'h':
        case 'H':
            cfg.usr.frqattr |= aHOLD;
            break;

        case 'p':
        case 'P':
            cfg.usr.frqattr |= aPRIVATE;
            break;
        }

        s++;
    }
}

// ---------------------------------------------

void akaforce(char *s)
{
    AKAFORCE *thisone;
    char *a, *b;
    NETADDR adr = { 0, 0, 0, 0 };
    int n;
    static AKAFORCE *last = NULL;

    if ((a = strtok(s, " \t\r\n")) == NULL ||
        (b = strtok(NULL, " \t\r\n")) == NULL)
    {
        sprintf(msg, "Error in AKAFORCE statement (line %d)!", globalline);
        Message(msg, -1, 0, YES);
        return;
    }

    thisone = mem_calloc(1, sizeof(AKAFORCE));
    if (filladdress(&thisone->mask, a) == -1 || filladdress(&adr, b) == -1)
    {
        mem_free(thisone);
        return;
    }

    for (n = 0; n < 24; n++)
    {
        if (addrcmp(&cfg.usr.address[n], &adr) == 0)
        {
            thisone->aka = n;
            break;
        }
    }

    if (n == 24)
    {
        sprintf(msg, "Unknown AKA in AKAFORCE statement (line %d)!",
                globalline);
        Message(msg, -1, 0, YES);
        mem_free(thisone);
        return;
    }

    if (cfg.akaforce == 0)
        cfg.akaforce = thisone;
    else
        last->next = thisone;

    last = thisone;

}

// ==============================================================


int near filladdress(NETADDR * thisaddress, char *adr)
{
    char *zone, *net, *node, *point;

    if ((strchr(adr, ':') == NULL) || (strchr(adr, '/') == NULL))
    {
        sprintf(msg, "Illegal address format (line %d)!", globalline);
        Message(msg, -1, 0, YES);
        return -1;
    }

    zone = strtok(adr, " :");
    net = strtok(NULL, " /");
    node = strtok(NULL, " .");
    point = strtok(NULL, " \t\r\n");

    if ((zone == NULL) || (net == NULL) || (node == NULL))
    {
        sprintf(msg, "Illegal address format (line %d)!", globalline);
        Message(msg, -1, 0, YES);
        return -1;
    }

    if (*zone == '*')
        thisaddress->zone = (word) - 99;
    else
        thisaddress->zone = (word) atoi(zone);

    if (*net == '*')
        thisaddress->net = (word) - 99;
    else
        thisaddress->net = (word) atoi(net);

    if (*node == '*')
        thisaddress->node = (word) - 99;
    else
        thisaddress->node = (word) atoi(node);

    if (point)
    {
        if (*point == '*')
            thisaddress->point = (word) - 99;
        else
            thisaddress->point = (word) atoi(point);
    }

    return 0;

}

// ==============================================================


 // one < two? ==> < 0
int near AreaCompare(AREA * one, AREA * two)
{
    int i, ret;
    char what;

    for (i = 0; i < 6; i++)
    {
        what = toupper(cfg.usr.alistsort[i]);
        switch (what)
        {
        case '\0':
            return 0;

        case 'G':              // Group
            if ((ret = (one->group - two->group)) != 0)
                return ret;
            break;

        case 'D':              // Description
            if ((ret = strcmpi(one->desc, two->desc)) != 0)
                return ret;
            break;

        case 'N':              // Nametag
            if ((ret = strcmpi(one->tag, two->tag)) != 0)
                return ret;
            break;

        case 'A':              // AKA
            if ((ret =
                 addrcmp(&cfg.usr.address[(int)one->aka],
                         &cfg.usr.address[(int)two->aka])) != 0)
                return ret;
            break;

        case 'T':              // Type (netm, loc, echo)
            if ((ret = (one->type - two->type)) != 0)
                return ret;
            break;
        }
    }

    return 0;
}

// ===============================================

void addwritename(char *filename)
{
    static char first = 1;
    OUTFLIST *thisone, *last;

    if (strlen(filename) > 119)
        filename[119] = '\0';

    if (first)
    {
        strcpy(cfg.usr.writename, filename);
        first = 0;
    }

    thisone = mem_calloc(1, sizeof(OUTFLIST));
    thisone->filename = mem_strdup(filename);
    if (cfg.usr.outfiles == NULL)
        cfg.usr.outfiles = thisone;
    else
    {
        for (last = cfg.usr.outfiles; last->next != NULL;
             last = last->next)
                                /* nothing */ ;
                                // Search last item, and..
        last->next = thisone;   // .. append
    }

}

// ===============================================

void uucpaddress(char *address)
{
    sscanf(address, "%hu:%hu/%hu.%hu", &cfg.usr.uucpaddress.zone,
           &cfg.usr.uucpaddress.net, &cfg.usr.uucpaddress.node,
           &cfg.usr.uucpaddress.point);
}

// ==============================================================
void ReadSoup2SQCFG(void)
{

    XFILE *soupfile;
    char *line, *rest, *tag, *path, *type, temp[80];
    char *keyword;
    unsigned lineno = 0;
    AREA *thisarea;


    if ((soupfile = xopen(Soup2SQcfg)) == NULL)
    {
        sprintf(msg, "Can't open Soup2SQ config! (%s)", Soup2SQcfg);
        Message(msg, -1, 254, YES);
    }

    sprintf(temp, "(%s)", Soup2SQcfg);
    if (strlen(temp) > 33)
    {
        temp[33] = '\0';
        temp[32] = '.';
        temp[31] = '.';
    }
    print(14, 36, 7, temp);


    while ((line = xgetline(soupfile)) != NULL)
    {
        if (!(lineno++ % 5))
            working(14, 7, 7);

        if (line[0] == ';' || strlen(line) < 11) /* Quick check for speed */
            continue;

        if ((keyword = strtok(line, " \t\r")) != NULL)
        {
            if (strcmpi(keyword, "MovePostedArticles") == 0)
            {
                if ((rest = strtok(NULL, " \t\r\n")) != NULL)
                {
                    thisarea = init_area();
                    thisarea->base = MSGTYPE_NEWS | MSGTYPE_SQUISH;
                    thisarea->type = NEWS;
                    thisarea->desc = mem_strdup("Soup2SQ posted articles");
                    thisarea->tag = mem_strdup("Posted_articles");
                    Strip_Trailing(rest, '\\');
                    thisarea->dir = AllocRemapPath(rest);
                    AddLinkedArea(thisarea);
                }
            }
            else if (strcmpi(keyword, "Newsgroup") == 0)
            {
                tag = strtok(NULL, " \t\r\n");
                path = strtok(NULL, " \t\r\n");
                type = strtok(NULL, " \t\r\n");

                if (tag && path && type)
                {
                    thisarea = init_area();
                    if (strchr(type, 'n') == NULL
                        && strchr(type, 'N') == NULL)
                    {
                        thisarea->base = MSGTYPE_MAIL | MSGTYPE_SQUISH;
                        thisarea->type = MAIL;
                    }
                    else
                    {
                        thisarea->base = MSGTYPE_NEWS | MSGTYPE_SQUISH;
                        thisarea->type = NEWS;
                    }

                    thisarea->desc = mem_strdup(tag);
                    thisarea->tag = thisarea->desc;
                    Strip_Trailing(path, '\\');
                    thisarea->dir = AllocRemapPath(path);
                    AddLinkedArea(thisarea);
                }
            }
        }
    }

    xclose(soupfile);

    print(14, 7, 9, "û");

}

// -----------------------------------------------------------------

void MapDrive(char *mapping, int line)
{
    int i;
    char *from, *to;

    if (!mapping)
        return;
    from = strtok(mapping, " :\t\r\n");
    to = strtok(NULL, " :\t\r\n");

    if (!from || !to)
    {
        sprintf(msg, "Incorrect 'DriveReplace' format on line %d!", line);
        Message(msg, -1, 0, YES);
    }


    i = tolower(*from) - 'a';

    if (((strlen(from) + strlen(to)) != 2) ||
        !isalpha(*from) || !isalpha(*to) || i < 0 || i > 25)
    {
        sprintf(msg, "Incorrect 'DriveReplace' format on line %d!", line);
        Message(msg, -1, 0, YES);
    }

    cfg.drivemap[i] = *to;

}


// -----------------------------------------------------------------

char *AllocRemapPath(char *s)
{
    char *colon, drive;
    char *start, *end;
    char temp[120];
    char *envvar;

    if ((colon = strchr(s, ':')) != NULL && colon > s) // > s, we need
                                                       // colon-1!
    {
        drive = tolower(*(colon - 1));
        if (isalpha(drive) && (cfg.drivemap[drive - 'a'] != 0))
            *(colon - 1) = cfg.drivemap[drive - 'a'];
    }

    if (strchr(s, '$') == NULL ||
        (start = strstr(s, "$[")) == NULL ||
        (end = strchr(start, ']')) == NULL)
    {
        // So, there is no $[env] stuff in it. Maybe %env% though?

        if ((start = strchr(s, '%')) != NULL &&
            (end = strchr(start + 1, '%')) != NULL)
        {
            // Yep, there's a %env% variable.

            memset(temp, '\0', sizeof(temp));
            *end = '\0';
            memcpy(temp, s, start - s);

            if ((envvar = getenv(start + 1)) != NULL)
                strcat(temp, envvar);

            strcat(temp, end + 1);
            return mem_strdup(temp);
        }
        else
            return (char *)mem_strdup(s);
    }


    // If we get here, there is a $[<env>] string present.

    memset(temp, '\0', sizeof(temp));
    *end = '\0';
    memcpy(temp, s, start - s);

    if ((envvar = getenv(start + 2)) != NULL)
        strcat(temp, envvar);

    strcat(temp, end + 1);
    return mem_strdup(temp);

}

// ------------------------------------------------------------------

void AddConvert(char *s)
{
    char *charset;
    char *tag;
    CHARSETLIST *current;
    static CHARSETLIST *last;

    if (!s ||
        *s == '\0' ||
        (charset = strtok(s, " \t\r\n")) == NULL ||
        (tag = strtok(NULL, " \t\r\n")) == NULL)
    {
        Message("ConvertOutput statement without charset name!", -1, 0,
                YES);
        return;
    }

    if (strlen(charset) > (MAXCHARSETLEN - 1))
        charset[MAXCHARSETLEN - 1] = '\0';
    strupr(charset);

    do
    {

        current = mem_calloc(1, sizeof(CHARSETLIST));
        strcpy(current->charset, charset);
        current->tag = mem_strdup(tag);
        if (!CharSetList)
            CharSetList = current;
        else
            last->next = current;
        last = current;

    }
    while ((tag = strtok(NULL, " \t\r\n")) != NULL);

}

// ------------------------------------------------------------------

void AddAssume(char *s)
{
    char *charset;
    char *tag;
    CHARSETLIST *current;
    static CHARSETLIST *last;

    if (!s ||
        *s == '\0' ||
        (charset = strtok(s, " \t\r\n")) == NULL ||
        (tag = strtok(NULL, " \t\r\n")) == NULL)
    {
        Message("AssumeCharset statement without charset name!", -1, 0,
                YES);
        return;
    }

    if (strlen(charset) > (MAXCHARSETLEN - 1))
        charset[MAXCHARSETLEN - 1] = '\0';
    strupr(charset);

    do
    {

        current = mem_calloc(1, sizeof(CHARSETLIST));
        strcpy(current->charset, charset);
        current->tag = mem_strdup(tag);
        if (!AssumeList)
            AssumeList = current;
        else
            last->next = current;
        last = current;

    }
    while ((tag = strtok(NULL, " \t\r\n")) != NULL);

}


// -------------------------------------------------------------------

void ProcessCharsets(void)
{
    CHARSETLIST *current, *last;

    if (!CharSetList && !AssumeList)
        return;                 // No conversions to process.

    current = CharSetList;
    while (current)
    {
        DoOneCharset(current, 0);

        last = current;
        current = current->next;
        if (last->tag)
            mem_free(last->tag);
        mem_free(last);
    }

    current = AssumeList;
    while (current)
    {
        DoOneCharset(current, 1);

        last = current;
        current = current->next;
        if (last->tag)
            mem_free(last->tag);
        mem_free(last);
    }


}

// --------------------------------------------------------------------

void DoOneCharset(CHARSETLIST * current, int isAssume)
{
    AREA *area;

    if (strchr(current->tag, '*') == NULL)
    {
        if ((area = FindArea(current->tag)) != NULL)
        {
            if (isAssume)
                strcpy(area->csread, current->charset);
            else
                strcpy(area->cswrite, current->charset);
        }
        return;
    }

    // Getting here means we have a wildcard..

    for (area = cfg.first; area; area = area->next)
    {
        if (inlist(current->tag, area->tag))
        {
            if (isAssume)
                strcpy(area->csread, current->charset);
            else
                strcpy(area->cswrite, current->charset);
        }
    }

}

// --------------------------------------------------------------------

int inlist(char *charptr, char *tag)
{
    char *str1, *str2;


    if (charptr[0] == '*')      /* starts w/ wildchart */
    {
        str1 = strdup(charptr + 1);
        strrev(str1);
        str2 = strdup(tag);
        strrev(str2);
        if (strncmpi(str1, str2, strlen(str1)) == 0)
        {
            mem_free(str1);
            mem_free(str2);
            return 1;
        }
        mem_free(str1);
        mem_free(str2);
    }
    else if (charptr[strlen(charptr) - 1] == '*') // Ends with wildcard.
    {
        str1 = strdup(charptr);
        str1[strlen(str1) - 1] = '\0';
        if (strncmpi(str1, tag, strlen(str1)) == 0)
        {
            mem_free(str1);
            return 1;
        }
        mem_free(str1);
    }

    return 0;

}

// ==============================================================

void V7Addflag(char *data, char type)
{
    static int flagno = 0;
    char *number, *flag;

    if (!data)
    {
        sprintf(msg, "Incorrect V7 flag definition on line %d!",
                globalline);
        Message(msg, -1, 0, YES);
        return;
    }

    number = strtok(data, " \r\n\t");
    flag = strtok(NULL, " \r\n\t");

    if (!flag || !number || !isdigit(*number))
    {
        sprintf(msg, "Incorrect V7 flag definition on line %d!",
                globalline);
        Message(msg, -1, 0, YES);
        return;
    }

    if (flagno >= MAXV7FLAGS)
    {
        Message("Too many V7 flags defined!", -1, 0, YES);
        return;
    }

    cfg.V7flags[flagno].type = (char)type;
    cfg.V7flags[flagno].value = (word) atoi(number);
    strncpy(cfg.V7flags[flagno].flag, flag, 9);

    flagno++;

}

// ==============================================================

int BoardNotUsed(byte boardno)
{
    AREA *curarea;
    char asciinumber[10];

    sprintf(asciinumber, "%d", boardno);

    for (curarea = cfg.first; curarea; curarea = curarea->next)
    {
        if (!(curarea->base & MSGTYPE_HMB))
            continue;           // We need a Hudson area.

        if (strcmpi(asciinumber, curarea->dir) == 0)
            return 0;           // Board _is_ being used.
    }

    // We didn't find it, so it's not used.
    return 1;

}

// ==============================================================

void AddLevelOne(char *s)
{
    char *charptr;

    if (!s || *s == '\0')
        return;

    charptr = strtok(s, " \t\r\n");
    while (charptr)
    {
        strupr(charptr);
        cfg.FirstLevelOne =
            AddToStringList(cfg.FirstLevelOne, charptr, NULL, 0);
        charptr = strtok(NULL, " \t\r\n");
    }

}

// ==============================================================
