#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>
#include <dos.h>
#include <ctype.h>
#include <conio.h>
#include <malloc.h>

#include <msgapi.h>
#include <progprot.h>
#include "wstruct.h"
#include "xmalloc.h"
#include "xfile.h"

/* --------  Prototypes  --------- */

void GetConfig(char *cfgname);
void readconfig(char *cfgname);
void Analyse(char *line, int readwimm);
void AddName(char *name);
void AddEchoArea(char *line);
void AddAreasArea(char *line);
void SetType(char *value);
void ReadSquishCfg(void);
void ReadAreasBBS(void);
void GetMode(char *value);
void DoMark(char *value);
void getattributes (char *attribs);
void addexcluded(char *areaname);
void addforced(char *areaname);
void check_status(void);
int  inlist(AREALIST *first, char *tag);
void make_empty(AREALIST *first);


/* -----------  Some Global Variables :  --------------  */

char           SquishCfg[100];   /* Where is your Squish configfile? */
char           LocalArea[100];   /* The name of the area to put output in */
char           LogFile[100];
int            LocalType;        /* Type of local area (Squish or *.MSG) */
NAMELIST       *firstname;       /* First name in list to search for */
AREALIST       *exclude_first, *force_first;
AREA           *firstarea;       /* First area in list of areas to search */
char           AreasBBS[100];    /* Path and name of areas.bbs file */
AREA           *lastarea = NULL; /* Used in building list of areas */
int            mode = COPY;      /* Mode of operation (list, move, copy) */
int            markreceived = 1; /* Should personal msgs found be marked received */
int            scanfrom = LASTREAD; /* Scan all msgs or from lastread? */
dword          attr = 0L | MSGSCANNED | MSGPRIVATE | MSGSENT;
int            nonotes;
int            addareakludge = 0;

// =======================================


#ifdef __WATCOMC__

   #define MAXDRIVE _MAX_DRIVE
   #define MAXDIR   _MAX_DIR
   #define MAXFILE  _MAX_FNAME
   #define MAXEXT   _MAX_EXT
   #define MAXPATH  _MAX_PATH

   #define strncmpi   strnicmp
   #define fnsplit    _splitpath
   #define fnmerge    _makepath
   #define heapcheck  _heapchk
   #define clrscr()   cls()
   #define textattr
#endif




/* ------------------------------------- */
/* --  First we read the config file  -- */
/* ------------------------------------- */


void GetConfig(char *cfgname)

{

   /* Initialize & cleanup some vars, to be safe */

   memset(SquishCfg, '\0', sizeof(SquishCfg));
   memset(LocalArea, '\0', sizeof(LocalArea));
   memset(AreasBBS,  '\0', sizeof(AreasBBS ));
   memset(LogFile,   '\0', sizeof(LogFile  ));

   LocalType     = MSGTYPE_SDM;
   firstname     = NULL;
   firstarea     = NULL;
   exclude_first = NULL;
   force_first   = NULL;
   nonotes = 0;

	readconfig(cfgname);

   if (exclude_first || force_first)
      {
      check_status();
      if (exclude_first)
         make_empty(exclude_first);
      if (force_first)
         make_empty(force_first);
      }

}


/* --------------------------------------------------------------- */
/* --  Open the configfile and read & analyse it, line by line  -- */
/* --------------------------------------------------------------- */


void readconfig(char *cfgname)

{
	XFILE	*configfile;
	char  *line, msg[80];


   if ((configfile=xopen(cfgname)) == NULL) {

      sprintf(msg, "Can't open configfile! (%s)", cfgname);
      getout(msg);
   }

   sprintf(msg, "Reading: %s", cfgname);
   what_act(msg);

	while( (line = xgetline(configfile)) != NULL)
      {
      Analyse(line, 1);
      }

   xclose(configfile);

   if (SquishCfg[0] == '\0') {

      sprintf(msg, "Warning: no Squish.cfg specified!");
      what_act(msg);
   }
   else ReadSquishCfg();

   if (AreasBBS[0] != '\0') {

      sprintf(msg, "Reading Areas.bbs file: %s", AreasBBS);
      what_act(msg);
      ReadAreasBBS();
   }

}


/* ------------------------------------------- */
/* --  Analyse every line in de configfile  -- */
/* ------------------------------------------- */


void Analyse(char *line, int readwimm)

{
	char	*keyword, *value, msg[80];
   AREA  *prev;

	keyword=strtok(line, " \n\r\t\0");			/* Get keyword */

	if ((keyword == NULL) || (keyword[0]==';'))			/* It's a comment! */
							return;

   strupr(keyword);

	if ( (value=strtok(NULL, "\r\n")) == NULL )
      return;		                                    /* Get value */


   if( (strcmp(keyword, "ECHOAREA")==0) || (strcmp(keyword, "BADAREA")==0) )
        {
        AddEchoArea(value);
        return;
        }

   if(strcmp(keyword, "DUPEAREA")==0)
        {
        prev = lastarea;       /* what's last now */
        AddEchoArea(value);
        if ( (prev != lastarea) && (lastarea != NULL) )  /* Was one added? - Are there any areas at all? */
            lastarea->status |= EXCLUDE;
        return;
        }

  	if(strcmp(keyword, "NAME")==0)
        {
        AddName(value);
        return;
        }

   /* End of keywords that may contain spaces.... */

   if( (value = strtok(value, " \t")) == NULL)
       return;

   else if(strcmp(keyword, "FORCE") == 0)
        addforced(strupr(value));

   else if(strcmp(keyword, "EXCLUDE") == 0)
        addexcluded(strupr(value));

	else if(strcmp(keyword, "SQUISHCFG")==0)
						strcpy(SquishCfg, value);

	else if(strcmp(keyword, "WIMMAREA")==0)
						strcpy(LocalArea, Strip_Trailing(value, '\\'));

	else if(strcmp(keyword, "WIMMTYPE")==0)
						SetType(value);

	else if(strcmp(keyword, "AREASBBS")==0)
                  strcpy(AreasBBS, value);

	else if(strcmp(keyword, "LOG")==0)
                  strcpy(LogFile, value);

   else if(strcmp(keyword, "MODE") == 0)
                  GetMode(value);

   else if(strcmp(keyword, "MARKRECEIVED") == 0)
                  DoMark(value);

   else if(strcmp(keyword, "SCANFROM") == 0) {

        if (strcmpi(value, "ALL") == 0)
           scanfrom = ALL;
        }

   else if(strcmp(keyword, "ATTRIBUTES") == 0)
        getattributes( strupr(value) );

   else if(strcmp(keyword, "NOTES") == 0)
        {
        if(strcmpi(value, "NO") == 0)
           nonotes = 1;
        }

   else if(strcmp(keyword, "ADDAREAKLUDGE") == 0)
        {
        if(strcmpi(value, "YES") == 0)
           addareakludge = 1;
        }

   else if (readwimm == 1)
        {
        sprintf(msg, "Unknown keyword: %s!", keyword);
        what_act(msg);
        }

}


/* ---------------------------------------------- */
/* --  Add a name to the linked list of names  -- */
/* ---------------------------------------------- */


void AddName(char *name)

{

          char     temp[40], *charptr, *tempptr, msg[80];
          NAMELIST *thisname;
   static NAMELIST *lastname;


   if(name == NULL) return;

   memset(temp, '\0', sizeof(temp));
   tempptr = temp;
   charptr = name;

   if( strchr(name, '"') == NULL || strchr(name, '"') == strrchr(name, '"') ) {

       sprintf(msg, "Illegal name format in %s", name);
       what_act(msg);
       return;
   }

   while(*charptr++ != '"') /* nothing */ ;     /* skip leading space */

   while(*charptr != '"')                       /* Copy chars */
        *tempptr++ = *charptr++;

   thisname = xmalloc(sizeof(NAMELIST));
   strcpy(thisname->name, temp);
   thisname->hash = SquishHash(temp);
   thisname->next = NULL;

   if (firstname == NULL)           /* first name */
      firstname = thisname;
   else                             /* establish link */
      lastname->next = thisname;

   lastname = thisname;

}


/* ------------------------------------------------------ */
/* --  Add an area from Squish.cfg to the linked list  -- */
/* ------------------------------------------------------ */


void AddEchoArea(char *line)

{
   AREA  *thisarea;
   char  *rest_of_line, *dirptr, *tagptr;


   if(line == NULL) return;

   thisarea = xcalloc(1, sizeof(AREA));

   if( (tagptr = strtok(line, " \t\n")) == NULL)
       {
       free(thisarea);
       return;
       }

   if( (dirptr = strtok(NULL, " \t\n")) == NULL)
        {
        free(thisarea);
        return;
        }

   rest_of_line = strtok(NULL, "\n");

   strncpy(thisarea->dir, dirptr, 79);
   Strip_Trailing(thisarea->dir, '\\');
   strncpy(thisarea->tag, strupr(tagptr), 59);

   if (strcmpi(thisarea->dir, LocalArea) == 0)
      {
      free(thisarea);
      return;
      }


   if (rest_of_line == NULL)
      thisarea->base = MSGTYPE_SDM;
   else if ((rest_of_line != NULL) && (strchr(rest_of_line, '$') != NULL))    /* Squish area */
      thisarea->base = MSGTYPE_SQUISH;
   else
       thisarea->base = MSGTYPE_SDM;

   if ( (rest_of_line != NULL) && (strstr(rest_of_line, "-0") != NULL) )
      thisarea->status |= PASSTHRU;

   thisarea->next = NULL;

   if (firstarea == NULL)           /* first name */
      firstarea = thisarea;
   else                             /* establish link */
      lastarea->next = thisarea;

   lastarea = thisarea;

}


/* ------------------------------------------------------------- */
/* --  Here we add an area from a areas.bbs line to the list  -- */
/* ------------------------------------------------------------- */


void AddAreasArea(char *line)        /* Here we add an area to the list */

{
   AREA  *thisarea, *curarea;
   char *dirptr, *tagptr;


   if(line == NULL) return;

   thisarea = xcalloc(1, sizeof(AREA));

   thisarea->base = MSGTYPE_SDM;

   while(line[0] == '$' || line[0] == '#') {

      if (line[0] == '$') {               /* Squish area */
         thisarea->base = MSGTYPE_SQUISH;
         line++;                         /* Skip first char ($) */
      }

      if (line[0] == '#') {
         thisarea->status |= PASSTHRU;
         line++;
      }

   }


   if( (dirptr = strtok(line, " \t\n")) == NULL)
      {
      free(thisarea);
      return;
      }

   strcpy(thisarea->dir, dirptr);
   Strip_Trailing(thisarea->dir, '\\');

   if( (tagptr = strtok(NULL, " \t\n")) == NULL)
      {
      free(thisarea);
      return;
      }

   strncpy(thisarea->tag, strupr(tagptr), 59);

   thisarea->next = NULL;

   if (strcmpi(thisarea->dir, LocalArea) == 0)
      {
      free(thisarea);
      return;
      }

   curarea = firstarea;
   while(curarea) {

      if (strcmpi(curarea->tag, thisarea->tag) == 0) {          /* Dupe! */

         /* Check to see if area is finally marked as being Squish... */

         if ( (curarea->base != MSGTYPE_SQUISH) && (thisarea->base == MSGTYPE_SQUISH))
            curarea->base = MSGTYPE_SQUISH;

         /* .... or as passthru.. */

         if ( (!(curarea->status & PASSTHRU)) && (thisarea->status & PASSTHRU))
            curarea->status |= PASSTHRU;

         free(thisarea);
         return;
      }
      curarea = curarea ->next;
   }


   if (firstarea == NULL)           /* first name */
      firstarea = thisarea;
   else                             /* establish link */
      lastarea->next = thisarea;

   lastarea = thisarea;

}


/* ---------------------------------------------- */
/* --  Set Squish or *.MSG as type of an area  -- */
/* ---------------------------------------------- */

void SetType(char *value)

{

   if(value == NULL) return;

   if (stricmp(value, "SQUISH") == 0)
      LocalType = MSGTYPE_SQUISH;
   else
      LocalType = MSGTYPE_SDM;

}


/* ------------------------------------------------------------ */
/* --  Read the squish.cfg file and get Echoareas out of it  -- */
/* ------------------------------------------------------------ */

void ReadSquishCfg(void)

{

   XFILE	*squishfile;
	char  *line, msg[80];

   squishfile=xopen(SquishCfg);

	if (squishfile==NULL) {

      sprintf(msg, "Can't open squish.cfg! (%s)", SquishCfg);
      getout(msg);
   }

   sprintf(msg, "Reading: %s", SquishCfg);
   what_act(msg);

	while( (line = xgetline(squishfile)) != NULL)
     {
     Analyse(line, 0);
     }

   xclose(squishfile);

}


/* ------------------------------------------------------- */
/* --  Read a Areas.bbs file and get the defined areas  -- */
/* ------------------------------------------------------- */

void ReadAreasBBS()

{

	XFILE	*areasfile;
	char  *line, drive[MAXDRIVE], dir[MAXDIR], temppath[256];
   int skip = 0;    /* To skip first line */
   char *p, msg[80];

   areasfile=xopen(AreasBBS);

	if (areasfile==NULL) {

      fnsplit(SquishCfg, drive, dir, NULL, NULL);
      sprintf(temppath, "%s%s%s", drive, dir, AreasBBS);
      areasfile=xopen(temppath);
      if (areasfile == NULL) {

         sprintf(msg, "Can't open areas.bbs file! (%s)", AreasBBS);
         what_act(msg);
         return;
      }
   }



	while( (line=xgetline(areasfile)) != NULL)
      {
      p = line;
      while( p && isspace(*p) )  p++;
      strcpy(line, p);

      if ((strlen(line) > 4) && (line[0] != ';') && (line[0] != '-') && skip++)
         AddAreasArea(line);
      }

   xclose(areasfile);

}

/* ----------------------------------- */
/* --  Get operating mode for WIMM  -- */
/* ----------------------------------- */

void GetMode(char *value)

{

   if(value == NULL) return;

   if (strcmpi(value, "LIST") == 0)
      mode = LIST;
   else if (strcmpi(value, "MOVE") == 0)
      mode = MOVE;
   else if (strcmpi(value, "COPY") == 0)
      mode = COPY;
}


/* ------------------------------------------- */
/* --  Should messages be marked received?  -- */
/* ------------------------------------------- */

void DoMark(char *value)

{
   if(value == NULL) return;

   if (stricmp(value, "YES") == 0)
      markreceived = 1;
   else markreceived = 0;

}

void getattributes (char *attribs)

{
   attr = 0L;     /* We're specifying attribs, so zap all out first */

   while(*attribs)
      {
      switch(*attribs++)
         {
         case 'P' : attr |= MSGPRIVATE; break;
         case 'S' : attr |= MSGSENT;    break;
         case 'C' : attr |= MSGSCANNED; break;
         case 'L' : attr |= MSGLOCAL;   break;
         }
      }

}

void addforced(char *areaname)
{

   static AREALIST *lastarea;
          AREALIST *thisarea;


   if(areaname == NULL) return;

   if (strlen(areaname) < 2)            /* Too short to my liking */
      return;

   thisarea = xcalloc(1, sizeof(AREALIST));
   strncpy(thisarea->tag, areaname, 59);
   thisarea->next = NULL;

   if (force_first == NULL)           /* first name */
      force_first = thisarea;
   else                             /* establish link */
      lastarea->next = thisarea;

   lastarea = thisarea;

}

void addexcluded(char *areaname)

{

   static AREALIST *lastarea;
          AREALIST *thisarea;


   if(areaname == NULL) return;

   if (strlen(areaname) < 2)            /* Too short to my liking */
      return;

   thisarea = xmalloc(sizeof(AREALIST));
   strncpy(thisarea->tag, areaname, 59);
   thisarea->next = NULL;

   if (exclude_first == NULL)           /* first name */
      exclude_first = thisarea;
   else                             /* establish link */
      lastarea->next = thisarea;

   lastarea = thisarea;

}

void check_status(void)
{
   AREA     *atbc;

   atbc = firstarea;

   while(atbc)
      {
      if (exclude_first && inlist(exclude_first, atbc->tag))
         atbc->status |= EXCLUDE;

      if (force_first && inlist(force_first, atbc->tag))
         atbc->status |= FORCED;

      atbc = atbc->next;
      }

}


int inlist(AREALIST *first, char *tag)
{
   char *charptr, *str1, *str2;
   AREALIST *areaptr = first;


   while(areaptr)
      {
      charptr = areaptr->tag;

      if (charptr[0] == '*')      /* starts w/ wildchart */
         {
         str1 = strdup(charptr + 1);
         strrev(str1);
         str2 = strdup(tag);
         strrev(str2);
         if (strncmpi(str1, str2, strlen(str1)) == 0)
            {
            free(str1);
            free(str2);
            return 1;
            }
         free(str1);
         free(str2);
         }


      else if (charptr[strlen(charptr)-1] == '*')
           {
           str1 = strdup(charptr);
           str1[strlen(str1)-1] = '\0';
           if (strncmpi(str1, tag, strlen(str1)) == 0)
              {
              free(str1);
              return 1;
              }
           free(str1);
           }

      else if (strcmpi(charptr, tag) == 0)
           return 1;

      areaptr = areaptr->next;
      }

   return 0;

}

void make_empty(AREALIST *first)
{
   AREALIST *ptr1, *ptr2;

   ptr1 = first;
   while(ptr1)
      {
      ptr2 = ptr1->next;
      free(ptr1);
      ptr1 = ptr2;
      }
}


