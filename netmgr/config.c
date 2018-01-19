#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <share.h>

#include <msgapi.h>
#include <progprot.h>

#include "nstruct.h"
#include "akamatch.h"
#include "netfuncs.h"
#include "config.h"

#define MASKLINE    1
#define XMASKLINE   2
#define ACTIONLINE  3


extern CFG cfg;
int    lineno = 0;
static AREALIST  * lastarea   = NULL;
static MASKLIST  * lastmask   = NULL;
static MASKLIST  * lastaction = NULL;
static FILE      * configfile;          // Config file handle.
extern XMASKLIST * firstxmask;          // List of defined XMASKS for later use

void   GetConfig (char *cfgname);
void   Analyse   (char *line);
void   addmask   (int what, char *value);
void   fillXmask (MASKLIST *mask, XMASK *xmask, char *value);
char * strip_around (char *string);
int    filladdress (NETADDR *thisaddress, char *adr, int *not, int notstar, int notaka, int notexit);
void   parseaddress(char *address);
void   getattributes(char *attributes, dword  *y1, dword *y2, dword *n1, dword *n2, int *fromlisted, int *tolisted, int notexit);
void   add_area (char *dir);
char * getstring (char *line);
void   addxmaskdefinition (char *value);
//char * GetQuotedString(int *inputlist, char *s);
void akaforce(char *s);

void AddSearchName(NAMELIST **root, char *s);
void AddSearchAttr(ATTRLIST **root, char *s, XMASK *xmask);
void AddSearchAddress(ADDRLIST **root, char *s);

ADDRLIST * AddToAddressList(ADDRLIST *root, char *address, int and);
NAMELIST * AddToNameList(NAMELIST *root, char *name, int and);
ATTRLIST * AddToAttrList(ATTRLIST *root, dword yes_attribs1, dword yes_attribs2, dword no_attribs1, dword no_attribs2);

void AddFile(NAMELIST **nameroot, ADDRLIST **addrroot, char *s);

// ================================================

void GetConfig(char *cfgname)
{
	char  line[256];

   cfg.useflags = 1;
   firstxmask = NULL;

   if ((configfile=_fsopen(cfgname, "rt", SH_DENYWR)) == NULL)
      {
      printf("\nCan't open configfile! (%s)\n", cfgname);
      exit(254);
      }

   printf("Reading: %s\n", cfgname);

	while(fgets(line, 256, configfile))
      {
      lineno++;
		Analyse(line);
      }

   fclose(configfile);

   lineno = 1;

}


// ================================================


void Analyse(char *line)
{
	char	*keyword, *value;
   char  *temp;

	if((keyword=strtok(line, " \n\r\t\0")) == NULL)			/* Get keyword */
        return;

   strupr(line);

	if ((keyword == NULL) || (keyword[0]==';'))			/* It's a comment! */
							return;

   if(strcmp(keyword, "NOFLAGS") == 0)
      {
      cfg.useflags = 0;
      return;
      }

   if(strcmp(keyword, "FRODOLOG") == 0)
      {
      cfg.frodolog = 1;
      return;
      }

   if(strcmp(keyword, "INTLFORCE") == 0)
      {
      cfg.intlforce = 1;
      return;
      }


   value=strtok(NULL, "\n\0");

   if(strcmp(keyword, "XMASK") == 0)      // XMASK can have a NULL value
      {
      addmask(XMASKLINE, value);
      return;
      }

   if(value == NULL )
      return;		                                    /* Get value */

   if(strcmp(keyword, "MASK") == 0)
        addmask(MASKLINE, value);

   else if(strcmp(keyword, "ACTION") == 0)
        addmask(ACTIONLINE, value);

   else if(strcmp(keyword, "DEFINEXMASK") == 0)
        addxmaskdefinition(value);

   else if(strcmp(keyword, "LOG") == 0)
        strncpy(cfg.logfile, getstring(strtok(value, " \t\r\n")), 99);

   else if(strcmp(keyword, "REGISTERNAME") == 0)
       {
       temp = strtok(value, "\n");
       if( (temp != NULL) && (temp[0] != '\0') )
          {
          while(*temp == ' ') temp++;
          Strip_Trailing(temp, ' ');
          strncpy(cfg.registername, temp, 35);
          }
       }

   else if(strcmp(keyword, "SCANDIR") == 0)
        add_area(strtok(value, "\r\n"));

   else if(strcmp(keyword, "HOME") == 0)
        parseaddress(strtok(value, " \t\r\n"));

   else if(strcmp(keyword, "ADDRESS") == 0)
        parseaddress(strtok(value, " \t\r\n"));

   else if(strcmp(keyword, "AKAFORCE") == 0)
        akaforce(strtok(value, "\r\n"));

   else if(strcmp(keyword, "HUDSONPATH") == 0)
        strcpy(cfg.hudsonpath, Strip_Trailing(getstring(strtok(value, " \t\r\n")), '\\'));

   else if(strcmp(keyword, "JAMLOG") == 0)
        strcpy(cfg.jamlog, Strip_Trailing(getstring(strtok(value, " \t\r\n")), '\\'));

   else if(strcmp(keyword, "NODELIST") == 0)
        strcpy(cfg.nodelist, Strip_Trailing(getstring(strtok(value, " \t\r\n")), '\\'));

   else if(strcmp(keyword, "FDNODELIST") == 0)
        strcpy(cfg.fdnodelist, Strip_Trailing(getstring(strtok(value, " \t\r\n")), '\\'));

   else if(strcmp(keyword, "GIGONODELIST") == 0)
        strcpy(cfg.gigonodelist, Strip_Trailing(getstring(strtok(value, " \t\r\n")), '\\'));

   else if(strcmp(keyword, "OUTBOUND") == 0)
        strcpy(cfg.outbound, Strip_Trailing(getstring(strtok(value, " \t\r\n")), '\\'));

   else if(strcmp(keyword, "NODELISTCACHE") == 0)
        strcpy(cfg.nodebuf, Strip_Trailing(getstring(strtok(value, " \t\r\n")), '\\'));

   else if(strcmp(keyword, "CACHESIZE") == 0)
        cfg.bufentries = atoi(getstring(strtok(value, " \t\r\n")));

   else if(strcmp(keyword, "ORIGIN") == 0)
           {
           temp = strtok(value, "\n");
           if( (temp != NULL) && (temp[0] != '\0') )
              {
              while(*temp == ' ') temp++;
              Strip_Trailing(temp, ' ');
              strncpy(cfg.origin, temp, 70);
              }
           }

   else printf("Unknown keyword: %s on line %d!\n", keyword, lineno);

}


//
//  ===================================================
//

void addmask(int what, char *value)
{
   static MASKLIST *current = NULL;
   static int curno = 1;    // Counter for MASK number..
   char *thisaction;
   char temp[120], temp2[120];
   int fake;

   if(value == NULL && what != XMASKLINE) return;


   if(what == MASKLINE || what == XMASKLINE)
      {
      current = mem_calloc(1, sizeof(MASKLIST));
      current->action = GETNEXT;
      current->no = curno++;

      if(what == MASKLINE)
        fillmask(&current->mask, value, 0);
      else
        {
        current->xmask = mem_calloc(1, sizeof(XMASK));
        fillXmask(current, NULL, value);
        }

      if(lastmask == NULL)            // First mask for this dir (area).
         {
         if(!lastarea)   // What, no area defined yet?!
           {
           printf("\nMask found before any ScanDir statement on line %d!\n", lineno);
           exit(254);
           }
         lastarea->firstmask = current;   // Set up masklist for this area.
         }
      else
         lastmask->next = current;

      lastmask   = current;
      lastaction = NULL;
      }
   else         /* actionline */
      {
      if(lastaction == NULL && lastmask == NULL)
        {
        printf("\nFound action without any preceding mask on line %d!\n", lineno);
        exit(254);
        }

      if(lastaction != NULL)
        {
        current = mem_calloc(1, sizeof(MASKLIST));
        current->action = GETNEXT;
        lastaction->nextaction = current;
        }

      lastaction = current;

      thisaction = strtok(value, " ,\t");
      if(thisaction == NULL)
         {
         printf("ACTION keyword without value on line %d!\n", lineno);
         exit(254);
         }

      if(strcmpi(thisaction, "COPY") == 0)
         {
         current->action = COPY;
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No path specified for COPY action on line %d!\n", lineno);
            exit(254);
            }
         }   /* else if COPY */

      else if(strcmpi(thisaction, "ECHOCOPY") == 0)
         {
         current->action = ECHOCOPY;
         strcpy(temp, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n")));
         current->seenby = (char *) mem_strdup(getstring(strtok(NULL, "\r\n")));

         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("Incorrect format for ECHOCOPY action on line %d!\n Specify both <address> and <path>!\n", lineno);
            exit(254);
            }

         filladdress(&current->origaddr, temp, &fake, 1, 1, 0);

         }   /* else if ECHOCOPY */

      else if(strcmpi(thisaction, "MOVE") == 0)
         {
         current->action = MOVE;

         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No path specified for COPY action on line %d!\n", lineno);
            exit(254);
            }

         }  /* else if MOVE */

      else if(strcmpi(thisaction, "ECHOMOVE") == 0)
         {
         current->action = ECHOMOVE;

         strcpy(temp, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n")));
         current->seenby = (char *) mem_strdup(getstring(strtok(NULL, "\r\n")));

         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("Incorrect format for ECHOMOVE action on line %d!\n Specify both <address> and <path>!\n", lineno);
            exit(254);
            }

         filladdress(&current->origaddr, temp, &fake, 1, 1, 0);
         }   /* else if ECHOMOVE */

      else if(strcmpi(thisaction, "DELETE") == 0)
         {
         current->action = DELETE;
         }

      else if(strcmpi(thisaction, "DELETEATTACH") == 0)
         {
         current->action = DELETEATTACH;
         }

      else if(strcmpi(thisaction, "IGNORE") == 0)
         {
         current->action = IGNORE;
         }

      else if(strcmpi(thisaction, "CHANGEPATH") == 0)
         {
         current->action = CHANGEPATH;
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No path specified for CHANGEPATH action on line %d!\n", lineno);
            exit(254);
            }
         }   /* else if CHANGEPATH */

      else if(strcmpi(thisaction, "CHANGEPATHMOVE") == 0)
         {
         current->action = CHANGEPATHMOVE;
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No path specified for CHANGEPATHMOVE action on line %d!\n", lineno);
            exit(254);
            }
         }   /* else if CHANGEPATH */

      else if(strcmpi(thisaction, "DISPLAY") == 0)
         {
         current->action = DISPLAY;
         memset(temp, '\0', sizeof(temp));
         strncpy(temp, getstring(strtok(NULL, "\r\n")), 117);
         if(temp[0] == '\0')
           {
           printf("No line to display specified for action on line %d!\n", lineno);
           exit(254);
           }
         strcat(temp, "\n");
         current->value = (char *) mem_strdup(temp);
        }   /* else if DISPLAY */

      else if(strcmpi(thisaction, "RUNEXTERNAL") == 0)
         {
         current->action = RUNEXTERNAL;
         memset(temp, '\0', sizeof(temp));
         strncpy(temp, getstring(strtok(NULL, " \r\n")), 117);
         if(temp[0] == '\0')
           {
           printf("No external program to run specified on line %d!\n", lineno);
           exit(254);
           }
         current->bodyfile = mem_strdup(temp);
         strncpy(temp, getstring(strtok(NULL, "\r\n")), 117);
         current->value = (char *) mem_strdup(temp);
         }   /* else if RUNEXTERNAL */

      else if(strcmpi(thisaction, "REWRITE") == 0)
         {
         current->action = REWRITE;
         current->value = (MASK *) mem_calloc(1, sizeof(MASK));
         fillmask((MASK *)current->value, strtok(NULL, "\r\n"), 0);
         }

      else if(strcmpi(thisaction, "FORWARD") == 0 ||
              strcmpi(thisaction, "FORWARDIN") == 0)
         {
         if(strcmpi(thisaction, "FORWARDIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = FORWARD;
         current->value = (MASK *) mem_calloc(1, sizeof(MASK));
         fillmask((MASK *)current->value, strtok(NULL, "\r\n"), 0);
         }

      else if(strcmpi(thisaction, "MAKEMSG") == 0 ||
              strcmpi(thisaction, "MAKEMSGIN") == 0)
         {
         if(strcmpi(thisaction, "MAKEMSGIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = MAKEMSG;
         current->value = (MASK *) mem_calloc(1, sizeof(MASK));
         current->bodyfile = mem_strdup(getstring(strtok(NULL, " ,\t\r\n")));
         fillmask((MASK *)current->value, strtok(NULL, "\r\n"), 0);
         }

      else if(strcmpi(thisaction, "XBOUNCE") == 0 ||
              strcmpi(thisaction, "XBOUNCEIN") == 0)
         {
         if(strcmpi(thisaction, "XBOUNCEIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = XBOUNCE;
         current->value = (MASK *) mem_calloc(1, sizeof(MASK));
         current->bodyfile = mem_strdup(getstring(strtok(NULL, " ,\t\r\n")));
         fillmask((MASK *)current->value, strtok(NULL, "\r\n"), 0);
         }

      else if(strcmpi(thisaction, "XHDRBOUNCE") == 0 ||
              strcmpi(thisaction, "XHDRBOUNCEIN") == 0)
         {
         if(strcmpi(thisaction, "XHDRBOUNCEIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = XHDRBOUNCE;
         current->value = (MASK *) mem_calloc(1, sizeof(MASK));
         current->bodyfile = mem_strdup(getstring(strtok(NULL, " ,\t\r\n")));
         fillmask((MASK *)current->value, strtok(NULL, "\r\n"), 0);
         }

      else if(strcmpi(thisaction, "XEMPTYBOUNCE") == 0 ||
              strcmpi(thisaction, "XEMPTYBOUNCEIN") == 0)
         {
         if(strcmpi(thisaction, "XEMPTYBOUNCEIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = XEMPTYBOUNCE;
         current->value = (MASK *) mem_calloc(1, sizeof(MASK));
         current->bodyfile = mem_strdup(getstring(strtok(NULL, " ,\t\r\n")));
         fillmask((MASK *)current->value, strtok(NULL, "\r\n"), 0);
         }

      else if(strcmpi(thisaction, "UUCPREWRITE") == 0)
         {
         current->action = UUCP;
         current->value = (MASK *) mem_calloc(1, sizeof(MASK));
         fillmask((MASK *)current->value, strtok(NULL, "\r\n"), 0);
         }

      else if(strcmpi(thisaction, "PACKMAIL") == 0)
         {
         current->action = PACKMAIL;
         strcpy(temp, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         strcpy(temp2, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         strncpy(current->password, getstring(strtok(NULL, " ,\t\r\n")), 8);

         if(temp2[0] == '\0')
            {
            printf("Incorrect format for PackMail acttion on line %d!\nSpecify both <origaddress> and <destaddress>!\n", lineno);
            exit(254);
            }

         filladdress(&current->origaddr, temp, &fake, 1, 0, 0);
         filladdress(&current->destaddr, temp2, &fake, 0, 1, 0);
         }  /* else if packmail */

      else if(strcmpi(thisaction, "MOVEMAIL") == 0)
         {
         current->action = MOVEMAIL;
         strcpy(temp, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         strcpy(temp2, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         current->value = (char *) mem_strdup(Strip_Trailing(getstring(strtok(NULL, " ,\t\r\n")), '\\'));
         strncpy(current->password, getstring(strtok(NULL, " ,\t\r\n")), 8);

         if(temp2[0] == '\0')
            {
            printf("Incorrect format for MoveMail action on line %d!\nSpecify <origaddress>, <destaddress> and <dir>!\n", lineno);
            exit(254);
            }

         filladdress(&current->origaddr, temp, &fake, 1, 0, 0);
         filladdress(&current->destaddr, temp2, &fake, 0, 1, 0);
         }  /* else if movemail */

      else if(strcmpi(thisaction, "BOUNCE") == 0 ||
              strcmpi(thisaction, "BOUNCEIN") == 0)
         {
         if(strcmpi(thisaction, "BOUNCEIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = BOUNCE;
         strcpy(temp, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         current->bodyfile = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->bodyfile, '\\');
         if(*(char *)current->bodyfile == '\0')
            {
            printf("Incorrect format for BOUNCE action on line %d!\nSpecify both <address> and <file>!\n", lineno);
            exit(254);
            }

         filladdress(&current->origaddr, temp, &fake, 1, 0, 0);
         }  /* else if bounce */

      else if(strcmpi(thisaction, "EMPTYBOUNCE") == 0 ||
              strcmpi(thisaction, "EMPTYBOUNCEIN") == 0)
         {
         if(strcmpi(thisaction, "EMPTYBOUNCEIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = EMPTYBOUNCE;
         strcpy(temp, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         current->bodyfile = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->bodyfile, '\\');
         if(*(char *)current->bodyfile == '\0')
            {
            printf("Incorrect format for EMPTYBOUNCE action on line %d!\nSpecify both <address> and <file>!\n", lineno);
            exit(254);
            }

         filladdress(&current->origaddr, temp, &fake, 1, 0, 0);
         }  /* else if emptybounce */

      else if(strcmpi(thisaction, "HDRBOUNCE") == 0 ||
              strcmpi(thisaction, "HDRBOUNCEIN") == 0)
         {
         if(strcmpi(thisaction, "HDRBOUNCEIN") == 0)
            current->destarea = mem_strdup(getstring(strtok(NULL, " ,\r\n\t")));
         current->action = HDRBOUNCE;
         strcpy(temp, (char *) mem_strdup(getstring(strtok(NULL, " ,\t\r\n"))));
         current->bodyfile = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->bodyfile, '\\');
         if(*(char *)current->bodyfile == '\0')
            {
            printf("Incorrect format for HDRBOUNCE action on line %d!\nSpecify both <address> and <file>!\n", lineno);
            exit(254);
            }

         filladdress(&current->origaddr, temp, &fake, 1, 0, 0);
         }  /* else if hdrbounce */

      else if(strcmpi(thisaction, "SEMAPHORE") == 0)
         {
         current->action = SEMA;
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No file specified for SEMAPHORE action on line %d!\n", lineno);
            exit(254);
            }

         }  /* else if bounce */

      else if(strcmpi(thisaction, "FILE") == 0)
         {
         current->action = WRITE;
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No file specified for FILE action on line %d!\n", lineno);
            exit(254);
            }

         }  /* else if file */

      else if(strcmpi(thisaction, "HDRFILE") == 0)
         {
         current->action = HDRFILE;
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No file specified for HDRFILE action on line %d!\n", lineno);
            exit(254);
            }

         }  /* else if hdrfile */

      else if(strcmpi(thisaction, "ADDNOTE") == 0)
         {
         current->action = ADDNOTE;
         current->value = (char *) mem_strdup(getstring(strtok(NULL, " \t\r\n")));
         Strip_Trailing((char *)current->value, '\\');
         if(*(char *)current->value == '\0')
            {
            printf("No file specified for ADDNOTE action on line %d!\n", lineno);
            exit(254);
            }
         }  /* else if hdrfile */
      else
         {
         printf("Unknown action specified on line %d (%s)!\n", lineno, thisaction);
         exit(254);
         }

      current = NULL;            /* important for check if both Mask & Action are defined */
      }

}


// ================================================


int fillmask(MASK *maskslot, char *value, int dontexit)
{
   char *fromname,
        *fromaddress,
        *toname,
        *toaddress,
        *subject,
        *attributes;

   fromname    = strtok(value, ","  );
   fromaddress = strtok(NULL,  ","  );
   toname      = strtok(NULL,  ","  );
   toaddress   = strtok(NULL,  ","  );
   subject     = strtok(NULL,  ","  );
   attributes  = strtok(NULL,  " ,\t\n");

   if( (fromname    == NULL) ||
       (fromaddress == NULL) ||
       (toname      == NULL) ||
       (toaddress   == NULL) ||
       (subject     == NULL) ||
       (attributes  == NULL)  )
         {
         print_and_log("Illegal mask format in line %d!\n", lineno);
         if(!dontexit)
           exit(254);
         else
           return -1;
         }


   strncpy(maskslot->fromname, strip_around(fromname), 35);

   maskslot->fromlisted = filladdress(&maskslot->fromaddress, strip_around(fromaddress), &maskslot->fromnot, 0, 0, dontexit);

   strncpy(maskslot->toname, strip_around(toname), 35);

   maskslot->tolisted = filladdress(&maskslot->toaddress, strip_around(toaddress), &maskslot->tonot, 0, 0, dontexit);

   strncpy(maskslot->subject, strip_around(subject), 71);

   getattributes(attributes,
                 &maskslot->yes_attribs1,
                 &maskslot->yes_attribs2,
                 &maskslot->no_attribs1,
                 &maskslot->no_attribs2,
                 &maskslot->fromlisted,
                 &maskslot->tolisted,
                 dontexit);

   return 0;

}

// ================================================
//
//
// ================================================

void fillXmask(MASKLIST *mask, XMASK *xmask, char *value)
{
   char temp[256];
   char *charptr, *valptr;
   char *keyword;
   XMASKLIST *curxmask;

   if(xmask == NULL)
      xmask = mask->xmask;    // Passed MASK or XMASK?

   if(value != NULL) // This is the name of a predefined XMASK
     {
     // copy (or let point to) a predefined xmask
     valptr = strtok(value, " \t\n\r");
     if(valptr)
       {
       for(curxmask = firstxmask; curxmask; curxmask = curxmask->next)
         {
         if(strcmpi(valptr, curxmask->xmask->xmaskname) == 0)
           {
           if(mask->xmask) mem_free(mask->xmask);
           mask->xmask = curxmask->xmask;
           return;
           }
         }
       }
     printf("Unknown XMASK name on line %d!\n", lineno);
     exit(254);
     }

   while(fgets(temp, 255, configfile) != NULL)
     {
     lineno++;
     charptr = temp;
     while(isspace(*charptr))
        charptr++;

     if(strnicmp(charptr, "end", 3) == 0)    // End of mask reached;
       return;

     keyword = strtok(charptr, " \t\r\n");
     if(keyword == NULL || keyword[0] == ';')
       continue;

     // ++++++++++++++++
     if(strcmpi(keyword, "from") == 0)
       {
       AddSearchName(&xmask->fromname, strtok(NULL, "\r\n"));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "to") == 0)
       {
       AddSearchName(&xmask->toname, strtok(NULL, "\r\n"));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "subject") == 0)
       {
       AddSearchName(&xmask->subj, strtok(NULL, "\r\n"));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "kludge") == 0)
       {
       AddSearchName(&xmask->kludge, strtok(NULL, "\r\n"));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "body") == 0)
       {
       AddSearchName(&xmask->body, strtok(NULL, "\r\n"));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "bodybytes") == 0)
       {
       valptr = strtok(NULL, " \t\r\n");
       if(valptr == NULL)
         {
         printf("Keyword without value on line %d!\n", lineno);
         exit(254);
         }

       xmask->bodybytes = atol(valptr);
       if(xmask->bodybytes > 4096)
         {
         printf("Warning! Bytelimit on line %d set to 4096 bytes!\n", lineno);
         xmask->bodybytes = 4096;
         }

       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "bodylines") == 0)
       {
       valptr = strtok(NULL, " \t\r\n");
       if(valptr == NULL)
         {
         printf("Keyword without value on line %d!\n", lineno);
         exit(254);
         }
       xmask->bodylines = atol(valptr);
       if(xmask->bodybytes == 0)
          xmask->bodybytes = 4096;
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "olderwritten") == 0)
       {
       valptr = strtok(NULL, " \t\r\n");
       if(valptr == NULL)
         {
         printf("Keyword without value on line %d!\n", lineno);
         exit(254);
         }
       xmask->olderwritten = JAMsysTime(NULL) - (atol(valptr) * (60L*60L*24L));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "olderprocessed") == 0)
       {
       valptr = strtok(NULL, " \t\r\n");
       if(valptr == NULL)
         {
         printf("Keyword without value on line %d!\n", lineno);
         exit(254);
         }
       xmask->olderprocessed = JAMsysTime(NULL) - (atol(valptr) * (60L*60L*24L));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "olderread") == 0)
       {
       valptr = strtok(NULL, " \t\r\n");
       if(valptr == NULL)
         {
         printf("Keyword without value on line %d!\n", lineno);
         exit(254);
         }
       xmask->olderread = JAMsysTime(NULL) - (atol(valptr) * (60L*60L*24L));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "attr") == 0)
       {
       AddSearchAttr(&xmask->attribs, strtok(NULL, "\r\n"), xmask);
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "orig") == 0)
       {
       AddSearchAddress(&xmask->orig, strtok(NULL, "\r\n"));
       }
     // ++++++++++++++++
     else if(strcmpi(keyword, "dest") == 0)
       {
       AddSearchAddress(&xmask->dest, strtok(NULL, "\r\n"));
       }
     // ++++++++++++++++
     else
       {
       printf("Unknown keyword specified on line %d!\n", lineno);
       exit(254);
       }
     }
}


// ==============================================================
//      Strip leading and trailing spaces from a string
// ==============================================================


char *strip_around(char *string)
{

   if(string == NULL)
      {
      printf("Illegal name format in line %d!\n", lineno);
      exit(254);
      }

   while(*string == ' ')
      string++;

   while( (strlen(string)) && (string[strlen(string)-1] == ' ') )
      string[strlen(string)-1] = '\0';

   if(*string == '\0')
      {
      printf("Illegal name format in line %d!\n", lineno);
      exit(254);
      }

   return string;
}


// ================================================


int filladdress(NETADDR *thisaddress, char *adr, int *not, int notstar, int notaka, int notexit)
{
   char *zone,
        *net,
        *node,
        *point;
   char temp[120];

   if(adr == NULL)
      {
      print_and_log(temp, "Illegal address format in line %d!\n\nYou *must* specify full 4D addresses!", lineno);
      if(!notexit)
         exit(254);
      else
         return 0;
      }

   if( (*adr == '!') && (not != NULL) )
      {
      *not = 1;
      adr++;
      }

   if( (*adr=='*') && (*(adr+1)=='\0') )
     {
     if(notstar == 1)
       {
       printf("Illegal format in line %d!\n\nYou *must* specify an origination address here!\n", lineno);
       if(!notexit)
         exit(254);
       else
         return 0;
       }
     thisaddress->zone = thisaddress->net = thisaddress->node = thisaddress->point = -99;
     return 0;
     }

   if( (*adr=='@') && (strcmpi(adr, "@myaka") == 0))
     {
     if(notaka == 1)
       {
       printf("Illegal format in line %d!\n\nYou can't use @myaka here!\n", lineno);
       if(!notexit)
         exit(254);
       else
         return 0;
        }
     thisaddress->zone = thisaddress->net = thisaddress->node = thisaddress->point = -88;
     return 0;
     }

   if( (strchr(adr, ':')==NULL) || (strchr(adr, '/')==NULL) || (strchr(adr, '.')==NULL) )
      {
      print_and_log("Illegal address format in line %d!\n\nYou *must* specify full 4D addresses!", lineno);
      if(!notexit)
        exit(254);
      else
        return 0;
      }

   zone  = strtok(adr , " :");
   net   = strtok(NULL, " /");
   node  = strtok(NULL, " .");
   point = strtok(NULL, " \t\r\n");

   if( (zone  == NULL) ||
       (net   == NULL) ||
       (node  == NULL) ||
       (point == NULL)  )
         {
         print_and_log("Illegal address format in line %d!\n\nYou *must* specify full 4D addresses!", lineno);
         if(!notexit)
           exit(254);
         else
           return 0;
         }

   if(*zone == '*') thisaddress->zone = (sword) -99;
      else thisaddress->zone = (word) atoi(zone);

   if(*net == '*') thisaddress->net = (sword) -99;
      else thisaddress->net = (word) atoi(net);

   if(*node == '*') thisaddress->node = (sword) -99;
      else thisaddress->node = (word) atoi(node);

   if(*point == '*') thisaddress->point = (sword) -99;
      else thisaddress->point = (word) atoi(point);

   if( notstar &&
       ((thisaddress->zone  == (word) -99)  ||
        (thisaddress->net   == (word) -99)  ||
        (thisaddress->node  == (word) -99)  ||
        (thisaddress->point == (word) -99)) )
        {
        printf("Illegal format in line %d!\n\nYou *must* specify an origination address here!\n", lineno);
        if(!notexit)
           exit(254);
        else
           return 0;
        }

   return 0;

}

// ================================================

void getattributes(char *attributes, dword  *y1, dword *y2, dword *n1, dword *n2, int *fromlisted, int *tolisted, int notexit)
{
   char thischar;
   dword *whatnow1, *whatnow2;
   int mustbepresent;

   if(attributes == NULL) return;

   if(*attributes == '*') return;

   /* p = private           */
   /* c = crash             */
   /* r = received          */
   /* s = sent              */
   /* a = attach            */
   /* i = forward/intransit */
   /* o = orphan            */
   /* k = kill              */
   /* l = local             */
   /* h = hold              */
   /* f = file request      */
   /* n = scaNned           */

   /* d = Direct            */
   /* m = iMmediate         */
   /* t = TFS               */
   /* e = KFS (erase)       */
   /* z = Archive sent      */
   /* u = Update request    */
   /* q = return receipt reQuest */
   /* y = return receipt    */

   /* @ = TO: address listed   */
   /* # = FROM: address listed */


   while(*attributes)
     { 
     if(*attributes == '+')
        {
        whatnow1 = y1;
        whatnow2 = y2;
        mustbepresent=1;
        }
     else if(*attributes == '-')
        {
        whatnow1 = n1;
        whatnow2 = n2;
        mustbepresent=0;
        }
     else
        {
        print_and_log("Wrong attributes format in line #%d!\n\n", lineno);
        if(notexit)
          return;
        else
          exit(254);
        }

     if(*++attributes == '\0') 
        {
        print_and_log("Wrong attributes format in line #%d!\n\n", lineno);
        if(notexit)
          return;
        else
          exit(254);
        }

     thischar = tolower(*attributes);

     if(thischar == 'p') *whatnow1 |= aPRIVATE;
     else if(thischar == 'c') *whatnow1 |= aCRASH;
     else if(thischar == 'r') *whatnow1 |= aREAD;
     else if(thischar == 's') *whatnow1 |= aSENT;
     else if(thischar == 'a') *whatnow1 |= aFILE;
     else if(thischar == 'i') *whatnow1 |= aFWD;
     else if(thischar == 'o') *whatnow1 |= aORPHAN;
     else if(thischar == 'k') *whatnow1 |= aKILL;
     else if(thischar == 'l') *whatnow1 |= aLOCAL;
     else if(thischar == 'h') *whatnow1 |= aHOLD;
     else if(thischar == '2') *whatnow1 |= aXX2;           // new
     else if(thischar == 'f') *whatnow1 |= aFRQ;
     else if(thischar == 'q') *whatnow1 |= aRRQ;
     else if(thischar == 'y') *whatnow1 |= aCPT;
     else if(thischar == 'b') *whatnow1 |= aARQ;           // new
     else if(thischar == 'u') *whatnow1 |= aURQ;
     else if(thischar == 'n') *whatnow1 |= aSCANNED;
     else if(thischar == 'd') *whatnow1 |= aDIR;
     else if(thischar == 'z') *whatnow1 |= aAS;
     else if(thischar == 'm') *whatnow1 |= aIMM;
     else if(thischar == 'e') *whatnow1 |= aKFS;
     else if(thischar == 't') *whatnow1 |= aTFS;
     else if(thischar == 'g') *whatnow1 |= aCFM;           // new
     else if(thischar == '$') *whatnow1 |= aLOK;           // new
     else if(thischar == '%') *whatnow1 |= aZGT;           // new
     else if(thischar == 'x') *whatnow2 |= aFAX;           // new
     else if(thischar == '@') *tolisted   = mustbepresent ? 1 : -1;
     else if(thischar == '#') *fromlisted = mustbepresent ? 1 : -1;
     else
         {
         print_and_log("Unknown attribute (%c) specified in line %d!", *attributes, lineno);
         if(!notexit)
           exit(254);
         }
     attributes++;
     }

}


// ================================================

void add_area(char *dir)
{
   AREALIST *this;
   char *areadir, *extra;

   lastmask   = NULL;
   lastaction = NULL;

   if(!dir)
     printf("\nError in ScanDir statement on line %s!\n", lineno);

   areadir = strtok(dir, " \t\r\n");
   extra   = strtok(NULL, " \t\n\r");

   if(!areadir)
     printf("\nError in ScanDir statement on line %s!\n", lineno);

   Strip_Trailing(areadir, '\\');

   this = mem_calloc(1, sizeof(AREALIST));
   this->dir = mem_strdup(getstring(areadir));

   if(extra && strnicmp(extra, "renum", 5) == 0)
      this->renumber = 1;

   if(cfg.firstarea == NULL)
      cfg.firstarea = this;
   else lastarea->next = this;

   lastarea = this;

}


// ================================================


char * getstring(char *line)

{
   static char temp[] = "";


   if(line != NULL)
      return line;
   else return temp;
}

// =============================================================

void addxmaskdefinition(char *value)
{
   char *name;
   static XMASKLIST *last = NULL;
   XMASKLIST *current;

   name = strtok(value, " \t\n\r");
   if(name == NULL || *name == '\0')
     {
     printf("XMASK definition without a name on line %d!\n", lineno);
     exit(254);
     }

  current = mem_calloc(1, sizeof(XMASKLIST));
  current->xmask = mem_calloc(1, sizeof(XMASK));
  current->xmask->xmaskname = mem_strdup(value);
  if(!last)
     firstxmask = current;
  else
    last->next = current;

  last = current;

  fillXmask(NULL, current->xmask, NULL);

}


// =============================================================

ADDRLIST * AddToAddressList(ADDRLIST *root, char *address, int and)
{
   ADDRLIST *new, *current;

   if(!address)
     return root;

   new = mem_calloc(1, sizeof(ADDRLIST));
   filladdress(&new->addr, address, &new->not, 0, 1, 0);

   // if root was NULL, we set up a new one.

   if(!root)
     root = new;
   else            // Link at end of list
     {
     current = root;
     while(current->and)         // Find end of list.
        current = current->and;

     if(and)
       current->and = new;
     else                  // add an 'or' element
       {
       while(current->or)
          current = current->or;
       current->or = new;
       }
     }

   return root;
}

// =============================================================

ATTRLIST * AddToAttrList(ATTRLIST *root, dword yes_attribs1, dword yes_attribs2, dword no_attribs1, dword no_attribs2)
{
   ATTRLIST *new, *current;

   new = mem_calloc(1, sizeof(ATTRLIST));
   new->yes_attribs1 = yes_attribs1;
   new->yes_attribs2 = yes_attribs2;
   new->no_attribs1  = no_attribs1;
   new->no_attribs2  = no_attribs2;

   // if root was NULL, we set up a new one.

   if(!root)
     root = new;
   else            // Link at end of list
     {
     current = root;
     while(current->or)
        current = current->or;
     current->or = new;
     }

   return root;
}

// =============================================================

NAMELIST * AddToNameList(NAMELIST *root, char *name, int and)
{
   NAMELIST *new, *current;

   if(!name)
     return root;

   new = mem_calloc(1, sizeof(NAMELIST));
   new->name = mem_strdup(name);

   // if root was NULL, we set up a new one.

   if(!root)
     root = new;
   else            // Link at end of list
     {
     current = root;
     while(current->and)         // Find end of list.
        current = current->and;
      if(and)
         current->and = new;
     else   // add an 'or' element.
       {
       while(current->or)
         current = current->or;
       current->or = new;
       }
     }

   return root;
}

// =============================================================

void AddSearchName(NAMELIST **root, char *s)
{
  static char temp[200];
  char quotechar;
  int len;
  int FirstElement = 1;

  if(s == NULL || s[0] == '\0')
    {
    printf("Keyword without value on line %d!\n", lineno);
    exit(254);
    }

  while(isspace(*s)) s++;

  quotechar = *s;

  if(quotechar != '"' && quotechar != '\'') // No quotes, ret full string.
    {
    while(s[strlen(s)-1] == ' ')      // strip trailing space
      s[strlen(s)-1] = '\0';

    if(*s == '<')         // a file as input.
      {
      AddFile(root, NULL, s+1);
      return;
      }
    else  // Just one name;

    *root = AddToNameList(*root, s, 1);     // last el == 1: AND element.
    return;
    }

  while(*s != '\0')     // Add all elements until end.
    {
    if(!FirstElement)
      {
      while(*s && isspace(*s))
        s++;     // skip space;

      if(*s == '\0')
         return;

      if(strnicmp(s, "or", 2) != 0) // not the first so must be separated by 'OR'
        {
        printf("Junk found on line %d!\n", lineno);
        exit(254);
        }

      s += 2;  // skip "or"

      while(*s && isspace(*s))
        s++;     // skip space;

      if(*s == '\0')
         return;

      quotechar = *s;
      if(quotechar != '"' && quotechar != '\'') // No quotes!
        {
        printf("Junk found on line %d!\n", lineno);
        exit(254);
        }
      }

    s++;    // skip first quote token.
    len = 0;

    while(*s && *s != quotechar && len < 199)
      temp[len++] = *s++;

    if(*s != quotechar)
      {
      printf("Quote without ending quote found on line %d!\n", lineno);
      exit(254);
      }

    s++;    // skip trailing quotecharacter

    temp[len] = '\0';

    *root = AddToNameList(*root, temp, FirstElement ? 1 : 0);
    FirstElement = 0;
    }

}

// =============================================================

void AddSearchAddress(ADDRLIST **root, char *s)
{
  static char temp[200];
  int len;
  int FirstElement = 1;

  if(s == NULL || s[0] == '\0')
    {
    printf("Keyword without value on line %d!\n", lineno);
    exit(254);
    }

  while(isspace(*s)) s++;

  if(*s == '<')         // a file as input.
    {
    AddFile(NULL, root, s+1);
    return;
    }

  while(*s != '\0')     // Add all elements until end.
    {
    if(!FirstElement)
      {
      while(*s && isspace(*s))
        s++;     // skip space;

      if(*s == '\0')
         return;

      if(strnicmp(s, "or", 2) != 0) // not the first so must be separated by 'OR'
        {
        printf("Junk found on line %d!\n", lineno);
        exit(254);
        }

      s += 2;  // skip "or"

      while(*s && isspace(*s))
        s++;     // skip space;

      if(*s == '\0')
         return;
      }

    len = 0;

    while(*s && !isspace(*s) && len < 199)
      temp[len++] = *s++;

    temp[len] = '\0';

    *root = AddToAddressList(*root, temp, FirstElement ? 1 : 0);
    FirstElement = 0;
    }

}

// =============================================================

void AddSearchAttr(ATTRLIST **root, char *s, XMASK *xmask)
{
  static char temp[200];
  int len;
  int FirstElement = 1;
  dword yes_attribs1,
        yes_attribs2,
        no_attribs1,
        no_attribs2;


  if(s == NULL || s[0] == '\0')
    {
    printf("Keyword without value on line %d!\n", lineno);
    exit(254);
    }

  while(isspace(*s)) s++;

  while(*s != '\0')     // Add all elements until end.
    {
    if(!FirstElement)
      {
      while(*s && isspace(*s))
        s++;     // skip space;

      if(*s == '\0')
         return;

      if(strnicmp(s, "or", 2) != 0) // not the first so must be separated by 'OR'
        {
        printf("Junk found on line %d!\n", lineno);
        exit(254);
        }

      s += 2;  // skip "or"

      while(*s && isspace(*s))
        s++;     // skip space;

      if(*s == '\0')
         return;
      }

    len = 0;

    while(*s && !isspace(*s) && len < 199)
      temp[len++] = *s++;

    temp[len] = '\0';
    yes_attribs1 = yes_attribs2 = no_attribs1 = no_attribs2 = 0;

    getattributes(temp, &yes_attribs1,
                        &yes_attribs2,
                        &no_attribs1,
                        &no_attribs2,
                        &xmask->fromlisted,
                        &xmask->tolisted,
                        0);

    *root = AddToAttrList(*root,
                          yes_attribs1,
                          yes_attribs2,
                          no_attribs1,
                          no_attribs2);

    FirstElement = 0;
    }

}



// ==============================================================

void AddFile(NAMELIST **nameroot, ADDRLIST **addrroot, char *s)
{
   char *filename;
   FILE *in;
   char *statptr, *tmpptr, *endptr;
   int FirstElement = 1;

   filename = strtok(s, " \t\r\n");
   if(!filename)
     {
     printf("Incomplete input filelist on line %d!\n", lineno);
     exit(254);
     }

   in = _fsopen(filename, "rt", SH_DENYNO);
   if(in == NULL)
     {
     printf("Error opening %s\n!", filename);
     exit(254);
     }

   statptr = tmpptr = mem_calloc(1, 256);

   while(fgets(tmpptr, 255, in) != NULL)
     {
     while(*tmpptr && isspace(*tmpptr))
       tmpptr++;
     endptr = tmpptr + strlen(tmpptr) - 1;
     while(endptr >= tmpptr &&
           (*endptr == '\n' || *endptr == '\r' || isspace(*endptr)))
       {
       *endptr = '\0';
       endptr--;
       }

     if(*tmpptr == '\0' || *tmpptr == ';')
       continue;

     if(nameroot)
       *nameroot = AddToNameList(*nameroot, tmpptr, FirstElement ? 1 : 0);
     else
       *addrroot = AddToAddressList(*addrroot, tmpptr, FirstElement ? 1 : 0);

     FirstElement = 0;
     }

   mem_free(statptr);
   fclose(in);

}

// ==============================================================

void parseaddress(char *address)
{
   static int n=0;

   if(address == NULL) return;

   if (n>(tMAXAKAS-1))
      {
      printf("Too many AKA's defined! Skipping %s\n", address);
      return;
      }

   sscanf(address, "%hu:%hu/%hu.%hu", &cfg.homeaddress[n].zone, &cfg.homeaddress[n].net, &cfg.homeaddress[n].node, &cfg.homeaddress[n].point);
   n++;
}


// ---------------------------------------------

void akaforce(char *s)
{
   AKAFORCE *thisone;
   char *a, *b;
   NETADDR adr = {0,0,0,0};
   int n;
   static AKAFORCE *last = NULL;

   if( (s == NULL) ||
       (a=strtok(s, " \t\r\n")) == NULL ||
       (b=strtok(NULL, " \t\r\n")) == NULL
     )
      {
      printf("Error in AKAFORCE statement (line %d)!\n", lineno);
      return;
      }

   thisone = mem_calloc(1, sizeof(AKAFORCE));
   if( filladdress(&thisone->mask, a, NULL, 0, 1, 0) == -1 ||
       filladdress(&adr, b, NULL, 0, 1, 0) == -1 )
     {
     mem_free(thisone);
     return;
     }

   for(n=0; n<24; n++)
      {
      if(addrcmp(&cfg.homeaddress[n], &adr)==0)
         {
         thisone->aka = n;
         break;
         }
      }

   if(n == 24)
     {
     printf("Unknown AKA in AKAFORCE statement (line %d)!\n", lineno);
     mem_free(thisone);
     return;
     }

   if(cfg.akaforce == NULL)
      cfg.akaforce = thisone;
   else
      last->next = thisone;

   last = thisone;

}

// ==============================================================


