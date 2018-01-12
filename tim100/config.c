#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmalloc.h"
#include <dir.h>
#include <ctype.h>
#include <conio.h>
#include <share.h>
#include <io.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include <progprot.h>
#include <video.h>
#include <scrnutil.h>
#include "message.h"
#include "reply.h"
#include "working.h"
#include "idlekey.h"
#include "xfile.h"

#include "fe_cfg.dat"
#include "gestruct.h"
#include "im_struc.h"
#include "fmstruct.h"
#include "xmail.h"

char *xgstr (char *line);

void readconfig(void);
void analyse (char *line);
void parseaddress(char *address);
void addname(char *name);
void addarea(char *value, word type);
char *getquoted(char *line);

AREA *init_area(void);
void check_aka(AREA *current, char *line);
void addmacro(char *name, int usenet);
void getcolour(char *keyword, char *value);
void startup(char *what);
int  one_of_three(char *what);

void AddEchoArea(char *line);
void AddAreasArea(char *line);
void ReadSquishCfg(void);
void ReadAreasBBS(void);
void ReadFECFG(void);
void ana_FE_area(Area *area, SysAddress *aka_array);
void ReadGEchoCFG(void);
void ana_GECHO_area(AREAFILE_GE *area, ADDRESS *aka_array);
void ReadImailCFG(void);
void ana_Imail_area(areas_record_type *area, naddress *aka_array);
void ReadxMailCFG(void);
void ana_xMail_area(xMailArea *area, word areano);
void ReadFmailCFG(void);
void old_ana_Fmail_area(rawEchoTypeOld *area, nodeFakeType *aka_array);
void ana_Fmail_area(rawEchoType *area, nodeFakeType *aka_array);
void add_to_ptlist (char *tag);
int  checkpt       (char *tag);
void freeptlist    (void);
char *xpstrdup(char *str);
void ReadConfigFile(char *temp, int showname);
void AddLinkedArea(AREA *thisarea);


typedef struct _ptlist
{

   char           *tag;
   struct _ptlist *next;

}  PTLIST;

/* -----------  Some Global Variables :  --------------  */

char           *AreasBBS  = NULL;    /* Path and name of areas.bbs file */
char           *SquishCfg = NULL;
char           *FEcfg     = NULL;
char           *GEchocfg  = NULL;
char           *Imailcfg  = NULL;
char           *xMailcfg  = NULL;
char           *Fmailcfg  = NULL;

int            stuff=0;
PTLIST         *firstpt=NULL;

CFG cfg;
CUSTOM custom;
AREA *last = NULL;


/* Here goes... */

void readconfig(void)
{
   char temp[100];

   strcpy(temp, cfg.homedir);

   strcpy(cfg.homedir, temp);

   /* now set the default colors */

   cfg.col.asbar     = 112;              /* Area selection top bar         */
   cfg.col.asframe   =   7;              /*                frame           */
   cfg.col.astext    =   7;              /*                text            */
   cfg.col.ashigh    = 112;              /*                highlighted bar */
   cfg.col.asspecial =  14;              /*                personal/newmail*/

   cfg.col.msgheader =   7;              /* Message reading header         */
   cfg.col.msgdata   =   7;
   cfg.col.msgdate   =   7;
   cfg.col.msgattribs=   7;
   cfg.col.msglinks  =   7;

   cfg.col.msgline   =   7;              /*                 horiz. line    */
   cfg.col.msgtext   =   7;              /*                 normal text    */
   cfg.col.msgquote  =  14;              /*                 quotes         */
   cfg.col.msgorigin =  14;              /*                 origin         */
   cfg.col.msgkludge =  14;              /*                 kludge         */
   cfg.col.msgbar    = 112;              /*                 bottom bar     */
   cfg.col.msgspecial= 112;              /*                 personal msgs  */

   cfg.col.entry = 112;

   cfg.col.editcurnormal = 14;
   cfg.col.editcurquote  = 7;
   cfg.col.edithcr       = 7;
   cfg.col.editblock     = 112;
   cfg.col.editcurblock  = 112;

   cfg.col.popframe  = 112;              /* Popup boxes frame              */
   cfg.col.poptext   = 112;              /* Popup boxes test               */

   sprintf(cfg.usr.writename, "%s\\timed.prn", cfg.homedir);
   strcpy(cfg.usr.lr, "lastread");
   strcpy(cfg.usr.printer, "PRN");
   cfg.usr.status |= (CONFIRMDELETE | NETTEAR | SHOWNOTES | ENDAREAMENU | SHOWEDITHCR | CONFIRMEDITEXIT);

   strcat(temp, "\\timed.cfg");

   print(11,7,9,"û");
   print(11,35,7,"(timEd.cfg)");

   ReadConfigFile(temp, 0);

   if (SquishCfg != NULL)
      {
      ReadSquishCfg();
      free(SquishCfg);
      }

   if (AreasBBS != NULL)
      {
      ReadAreasBBS();
      free(AreasBBS);
      }

   if (FEcfg != NULL)
      {
      ReadFECFG();
      free(FEcfg);
      }

   if(GEchocfg != NULL)
      {
      ReadGEchoCFG();
      free(GEchocfg);
      }

   if(Imailcfg != NULL)
      {
      ReadImailCFG();
      free(Imailcfg);
      }

   if(xMailcfg != NULL)
      {
      ReadxMailCFG();
      free(xMailcfg);
      }

   if(Fmailcfg != NULL)
      {
      ReadFmailCFG();
      free(Fmailcfg);
      }

   if(cfg.first == NULL)
      Message("No areas defined!", -1, 254, YES);

   freeptlist();

   if(strcmpi(cfg.usr.editor, "INTERNAL") == 0)
      cfg.usr.internal_edit = 1;

   if(stuff != 0) stuffkey(stuff);
}


/* Analyse one line from the config file */


void analyse (char *line)

{
   char	*keyword;


   keyword=strtok(line, " \n\r\t\0");	/* Get keyword */

	if ((keyword == NULL) || (keyword[0]==';'))			/* It's a comment! */
		  return;

	strupr(keyword);

	if(strcmp(keyword, "ECHOAREA")==0)
		  addarea(strtok(NULL, "\n"), ECHOMAIL);

	else if(strcmp(keyword, "ADDRESS")==0)
		  parseaddress(strtok(NULL, " \t\n"));

	else if(strcmp(keyword, "NETAREA")==0)
		  addarea(strtok(NULL, "\n"), NETMAIL);

	else if(strcmp(keyword, "LOCALAREA")==0)
        addarea(strtok(NULL, "\n"), LOCAL);

   else if(strncmp(keyword, "COLOR", 5)==0)
        getcolour(keyword, strtok(NULL, " \t\n"));

	else if(strcmp(keyword, "MACRO")==0)
        addmacro(xgstr(strtok(NULL, "\n")), 0);

	else if(strcmp(keyword, "USENETMACRO")==0)
        addmacro(xgstr(strtok(NULL, "\n")), 1);

	else if(strcmp(keyword, "NAME")==0)
        addname(getquoted(strtok(NULL, "\n")));

	else if(strcmp(keyword, "ALIAS")==0)
        addname(getquoted(strtok(NULL, "\n")));

	else if(strcmp(keyword, "ORIGIN")==0)
        strncpy(cfg.usr.origin, getquoted(strtok(NULL, "\n")), 65);

	else if(strcmp(keyword, "SIGNOFF")==0)
        strncpy(cfg.usr.signoff, getquoted(strtok(NULL, "\n")), 200);

	else if(strcmp(keyword, "NODELIST")==0)
        strcpy(cfg.usr.nodelist, Strip_Trailing(xgstr(strtok(NULL, " \t\n")), '\\') );

	else if(strcmp(keyword, "FDNODELIST")==0)
        strcpy(cfg.usr.fdnodelist, Strip_Trailing(xgstr(strtok(NULL, " \t\n")), '\\') );

	else if(strcmp(keyword, "FIDOUSER")==0)
        strcpy(cfg.usr.fidouser, xgstr(strtok(NULL, " \t\n")));

	else if(strcmp(keyword, "SQUISHCFG")==0)
        SquishCfg = xstrdup(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "AREASBBS")==0)
        AreasBBS = xstrdup(xgstr(strtok(NULL, " \t\n")));

	else if(strcmp(keyword, "FASTECHOCFG")==0)
        FEcfg = xstrdup(xgstr(strtok(NULL, " \t\n")));

	else if(strcmp(keyword, "GECHOCFG")==0)
        GEchocfg = xstrdup(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "IMAILCFG")==0)
        Imailcfg = xstrdup(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "XMAILCFG")==0)
        xMailcfg = xstrdup(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "FMAILCFG")==0)
        Fmailcfg = xstrdup(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "INCLUDE")==0)
        ReadConfigFile(xgstr(strtok(NULL, " \t\n")), 1);

	else if(strcmp(keyword, "ECHOLOG")==0)
        strcpy(cfg.usr.echolog, xgstr(strtok(NULL, " \t\n")));

	else if(strcmp(keyword, "JAMLOG")==0)
        strcpy(cfg.usr.jamlogpath, xgstr(strtok(NULL, " \t\n")));

	else if(strcmp(keyword, "NETMAIL_SEMAPHORE")==0)
        strcpy(cfg.usr.netsema, xgstr(strtok(NULL, " \t\n")));

	else if(strcmp(keyword, "HUDSONPATH")==0)
        strcpy(cfg.usr.hudsonpath, Strip_Trailing(xgstr(strtok(NULL, " \t\n")), '\\'));

	else if(strcmp(keyword, "EDITOR")==0)
        strcpy(cfg.usr.editor, xgstr(strtok(NULL, " \t\n")));

	else if(strcmp(keyword, "HELLO")==0)
        strncpy(cfg.usr.hello, getquoted(strtok(NULL, "\n")), 200);

	else if(strcmp(keyword, "REPHELLO")==0)
        strncpy(cfg.usr.rephello, getquoted(strtok(NULL, "\n")), 200);

	else if(strcmp(keyword, "FOLLOWHELLO")==0)
        strncpy(cfg.usr.followhello, getquoted(strtok(NULL, "\n")), 200);

	else if(strcmp(keyword, "SHOWKLUDGES")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | SHOWKLUDGES : cfg.usr.status & ~SHOWKLUDGES;

	else if(strcmp(keyword, "JUMPY_EDIT")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | JUMPYEDIT : cfg.usr.status & ~JUMPYEDIT;

	else if(strcmp(keyword, "ARCMAILATTACH")==0)
        cfg.usr.arcmail = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? 1 : 0;

	else if(strcmp(keyword, "EMPTY_TEARLINE")==0)
        cfg.usr.emptytear = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? 1 : 0;

	else if(strcmp(keyword, "ASK_CONFIRMATION")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | (CONFIRMDELETE|CONFIRMEXIT) : cfg.usr.status;

	else if(strcmp(keyword, "CONFIRM_DELETE")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | CONFIRMDELETE : cfg.usr.status & ~CONFIRMDELETE;

	else if(strcmp(keyword, "CONFIRM_EXIT")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | CONFIRMEXIT : cfg.usr.status & ~CONFIRMEXIT;

	else if(strcmp(keyword, "CONFIRM_EDIT_EXIT")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | CONFIRMEDITEXIT : cfg.usr.status & ~CONFIRMEDITEXIT;

	else if(strcmp(keyword, "SHOW_EDIT_HCR")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | SHOWEDITHCR : cfg.usr.status & ~SHOWEDITHCR;

	else if(strcmp(keyword, "PERSONAL_SKIP_RECEIVED")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | SKIPRECEIVED : cfg.usr.status & ~SKIPRECEIVED;

	else if(strcmp(keyword, "INTLFORCE")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | INTLFORCE : cfg.usr.status & ~INTLFORCE;

	else if(strcmp(keyword, "LOWLEVELKB")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | LOWLEVELKB : cfg.usr.status & ~LOWLEVELKB;

	else if(strcmp(keyword, "NETMAIL_TEARLINE")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | NETTEAR : cfg.usr.status & ~NETTEAR;

	else if(strcmp(keyword, "MOVE_NOTES")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | SHOWNOTES : cfg.usr.status & ~SHOWNOTES;

	else if(strcmp(keyword, "END_OF_AREA_MENU")==0)
        cfg.usr.status = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? cfg.usr.status | ENDAREAMENU : cfg.usr.status & ~ENDAREAMENU;

	else if(strcmp(keyword, "SWAP_ON_EDIT")==0)
        cfg.usr.swap_edit = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? 1 : 0;

	else if(strcmp(keyword, "SWAP_ON_SHELL")==0)
        cfg.usr.swap_shell = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? 1 : 0;

	else if(strcmp(keyword, "AKAMATCHING")==0)
        cfg.usr.akamatch = (strcmpi(xgstr(strtok(NULL, " \t\n")), "YES") == 0) ? 1 : 0;

   else if(strcmp(keyword, "WRITENAME")==0)
        strncpy(cfg.usr.writename, xgstr(strtok(NULL, " \t\n")), 99);

   else if(strcmp(keyword, "PRINTER")==0)
        strncpy(cfg.usr.printer, xgstr(strtok(NULL, " \t\n")), 19);

   else if(strcmp(keyword, "STARTUP_SCAN")==0)
        startup(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "KILL_ORIGINAL")==0)
        cfg.usr.delreply = one_of_three(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "ZONEGATE")==0)
        cfg.usr.zonegate = one_of_three(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "SQUISH_OFFSET")==0)
        cfg.usr.sqoff = (word) atoi(xgstr(strtok(NULL, " \t\n")));

   else if(strcmp(keyword, "LASTREAD")==0)
        strncpy(cfg.usr.lr, xgstr(strtok(NULL, " \t\n")), 19);

   else
      {
      sprintf(msg, "Unknown keyword: %s!", keyword);
      Message(msg, -1, 0, YES);
      }
}


void parseaddress(char *address)
{
   static n=0;

   if (n>24)
      {
      sprintf(msg, "Too many AKA's defined! Skipping %s", address);
      Message(msg, -1, 0, YES);
      return;
      }

   sscanf(address, "%hd:%hd/%hd.%hd", &cfg.usr.address[n].zone, &cfg.usr.address[n].net, &cfg.usr.address[n].node, &cfg.usr.address[n].point);
   n++;
}


void addname(char *name)
{
   static n=0;

   if(name == NULL) return;

   if (n>9)
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


AREA *init_area()
{
   AREA *thisarea;

   thisarea = xcalloc(1, sizeof(AREA));

   thisarea->highest = thisarea->lr = -1;
   thisarea->stdattr |= MSGLOCAL;

   return thisarea;
}



void check_aka(AREA *current, char *rest)
{
   char *charptr, *aka;
   char temp[80];
   NETADDR tempaddr;
   char n;


   if(rest == NULL) return;

   if ( ((charptr = strstr(rest, "-P")) != NULL) || ((charptr = strstr(rest, "-p")) != NULL) )
      {
      charptr += 2;

      if((aka=strtok(charptr, " \t\n\r")) != NULL)
         strcpy(temp, aka);
      else
         {
         Message("Can't figure out -P switch in config :-( ", -1, 0, YES);
         return;
         }


      memset(&tempaddr, '\0', sizeof(NETADDR));
      sscanf(temp, "%hd:%hd/%hd.%hd", &tempaddr.zone, &tempaddr.net, &tempaddr.node, &tempaddr.point);

      for(n=0; n<24; n++)
         {
         if ( cfg.usr.address[n].zone  == tempaddr.zone &&
              cfg.usr.address[n].net   == tempaddr.net  &&
              cfg.usr.address[n].node  == tempaddr.node &&
              cfg.usr.address[n].point == tempaddr.point )
                {
                current->aka = n;
                break;
                }
         }

      if(n==24)
         {
         sprintf(msg, "Unknown AKA (%d:%d/%d.%d) referenced!", tempaddr.zone, tempaddr.net, tempaddr.node, tempaddr.point);
         Message(msg, -1, 0, YES);
         }

      }


}


/* Add an area found in timEd.cfg */

void addarea(char *value, word type)

{
   AREA *current;

   char temp[100], *tempptr, *tagptr, *pathptr, *rest, *charptr;


   if(value == NULL)
       return;

   current = init_area();

   strcpy(temp, getquoted(value));      /* First get the (quoted) description */

   if (strlen(temp) > 59)   temp[59] = 0;

   current->desc = xstrdup(temp);

   tempptr = value;
   while( (*tempptr != '"') && (*tempptr != '\0') )
          tempptr++ ;

   if(tempptr != '\0')
      tempptr++;       /* skip over first "   */

   while( (*tempptr != '"') && (*tempptr != '\0') )
          tempptr++;


    if(*tempptr == '\0')
        {
        free(current);           /* fail */
        return;
        }

    value = tempptr+1;     /* Skip over second "  */

   tagptr  = strtok(value, " \t\n\r");
   pathptr = strtok(NULL,  " \t\n\r");

   if( (tagptr==NULL) || (pathptr==NULL) )
       {
       free(current);
       return;
       }

   current->tag = xstrdup(tagptr);

   memset(temp, '\0', sizeof(temp));
   strncpy(temp,  pathptr, 79);
   Strip_Trailing(temp, '\\');
   current->dir = xstrdup(temp);

   current->type = type;

   if ( (rest = strtok(NULL, "\n")) != NULL)
      {
      if (strstr(rest, "-$") != NULL)
         current->base |= MSGTYPE_SQUISH;

      else if( (strstr(rest, "-J") != NULL) ||
               (strstr(rest, "-j") != NULL) )
          current->base |= MSGTYPE_JAM;

      else if( (strstr(rest, "-H") != NULL) ||
               (strstr(rest, "-h") != NULL) )
          current->base |= MSGTYPE_HMB;

      else
          current->base |= MSGTYPE_SDM;


      if ( ((charptr = strstr(rest, "-A")) != NULL) || ((charptr = strstr(rest, "-a")) != NULL) )
         {
         charptr += 2;
         while(*charptr != ' ' && *charptr != '-' && *charptr != '\t' && *charptr != '\n' && *charptr != '\0' && *charptr != '\r')
            {
            switch(*charptr)
               {
               case 'p':
               case 'P':
                    current->stdattr |= MSGPRIVATE;
                    break;

               case 'c':
               case 'C':
                    current->stdattr |= MSGCRASH;
                    break;

               case 'k':
               case 'K':
                    current->stdattr |= MSGKILL;
                    break;

               case 'd':
               case 'D':
                    current->stdattr |= ADD_DIR;
                    break;

               case 'i':
               case 'I':
                    current->stdattr |= ADD_IMM;
                    break;

               default:
                    sprintf(msg, "Junk attribute in %s", rest);
                    Message(msg, -1, 0, YES);
                    break;
               }
            charptr++;
            }
         }

      check_aka(current, rest);        /* See if an AKA is used.. */
      }
   else /* rest was NULL, so *.MSG */
     current->base |= MSGTYPE_SDM;

   /* Set local, echomail, netmail bits */

   if(type == ECHOMAIL)
     current->base |= MSGTYPE_ECHO;
   else if(type == LOCAL)
     current->base |= MSGTYPE_LOCAL;
   else
     current->base |= MSGTYPE_NET;


   AddLinkedArea(current);

}


/* ------------------------------- */
/* Extract text from within quotes */
/* ------------------------------- */


char *getquoted(char *line)

{

   static char     temp[301];
   char             *charptr, *tempptr;


   memset(temp, '\0', sizeof(temp));
   tempptr = temp;

   if( (charptr = line) == NULL)
       return temp;

   if( strchr(line, '"') == NULL || strchr(line, '"') == strrchr(line, '"') )
      {
      sprintf(msg, "Illegal line format in %s", line);
      Message(msg, -1, 0, YES);
      return temp;
      }

   while(*charptr++ != '"') /* nothing */ ;     /* skip leading space */

   while(*charptr != '"' && (strlen(temp) < 300))     /* Copy chars */
        *tempptr++ = *charptr++;

   return temp;

}


char *xgstr (char *line)

{
   static char temp[5] = "";


   if(line != NULL)
      return line;
   else return temp;
}


/* ------------------------------------------------------ */
/* --  Add an area from Squish.cfg to the linked list  -- */
/* ------------------------------------------------------ */


void AddEchoArea(char *line)

{
   AREA  *thisarea, *curarea;
   char  *rest_of_line, *tagptr, *pathptr;
   char temp[100];


   if(line == NULL)
      return;

   thisarea = init_area();

   tagptr  = strtok(line, " \t\n\r");
   pathptr = strtok(NULL, " \t\n\r");

   if( (tagptr==NULL) || (pathptr==NULL) )
      {
      free(thisarea);
      return;
      }

   thisarea->tag  = xstrdup(tagptr);
   thisarea->desc = thisarea->tag;

   memset(temp, '\0', sizeof(temp));
   strncpy(temp, pathptr, 79);
   Strip_Trailing(temp, '\\');
   thisarea->dir = xstrdup(temp);

   if( (rest_of_line = strtok(NULL, "\n")) != NULL)
      {
      if (strchr(rest_of_line, '$') != NULL)    /* Squish area */
         thisarea->base = MSGTYPE_SQUISH | MSGTYPE_ECHO;
      else
         thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;

      if (strstr(rest_of_line, "-0") != NULL)            /* passthru */
         {
         /* Add to list of passthru area */
         add_to_ptlist(thisarea->tag);

         free(thisarea->tag);
         free(thisarea->dir);
         free(thisarea);
         return;
         }

      check_aka(thisarea, rest_of_line);
      }
   else   /* SDM area */
      thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;


   AddLinkedArea(thisarea);

}


/* ------------------------------------------------------------- */
/* --  Here we add an area from a areas.bbs line to the list  -- */
/* ------------------------------------------------------------- */


void AddAreasArea(char *line)        /* Here we add an area to the list */

{
   AREA  *thisarea, *curarea;
   char *tagptr, *pathptr, *rest;
   char temp[100];

   thisarea = init_area();

   if(line == NULL) return;


   while(line[0] == '$' || line[0] == '#' || line[0] == 'p' || line[0] == 'P' || line[0] == '!')
      {
      if (line[0] == '$')                /* Squish area */
         {
         thisarea->base = MSGTYPE_SQUISH | MSGTYPE_ECHO;
         line++;                         /* Skip first char ($) */
         }

      else if (line[0] == '!')                /* Squish area */
         {
         thisarea->base = MSGTYPE_JAM | MSGTYPE_ECHO;
         line++;                         /* Skip first char ($) */
         }

      else  // Passthru area
         {
         free(thisarea);
         return;
         }
      }

   if(!(thisarea->base & (MSGTYPE_SQUISH | MSGTYPE_JAM)))
     {
     if(isdigit(*line))
        thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;
     else
        thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;
     }

   pathptr = strtok(line, " \t\n\r");
   tagptr  = strtok(NULL, " \t\n\r");

   if( (pathptr==NULL) || (tagptr==NULL) )
      {
      free(thisarea);
      return;
      }

   if(checkpt(tagptr) != 0)
      {
      free(thisarea);
      return;
      }

   thisarea->tag = xstrdup(tagptr);

   memset(temp, '\0', sizeof(temp));
   strncpy(temp, pathptr, 79);
   Strip_Trailing(temp, '\\');
   thisarea->dir = xstrdup(temp);

   thisarea->desc = thisarea->tag;

   if( (rest=strtok(NULL, "\n\r")) != NULL)
      check_aka(thisarea, rest);

   curarea = cfg.first;
   while(curarea)
      {
      if (strcmpi(curarea->tag, thisarea->tag) == 0)           /* Dupe! */
         {
         if(thisarea->base & MSGTYPE_SQUISH)  /* See if it's finally squish type */
             {
             curarea->base |= MSGTYPE_SQUISH;
             curarea->base &= ~MSGTYPE_SDM; /* Switch off SDM bit */
             }

         free(thisarea->tag);
         free(thisarea->dir);
         free(thisarea);
         return;
         }
      curarea = curarea ->next;
      }

   AddLinkedArea(thisarea);
}



/* ------------------------------------------------------------ */
/* --  Read the squish.cfg file and get Echoareas out of it  -- */
/* ------------------------------------------------------------ */

void ReadSquishCfg(void)
{

   XFILE	*squishfile;
   char  *line, temp[80];
   char *keyword;
   unsigned lineno=0;


   if ( (squishfile=xopen(SquishCfg)) == NULL )
      {
      sprintf(msg, "Can't open squish.cfg! (%s)", SquishCfg);
      Message(msg, -1, 254, YES);
      }


   sprintf(temp, "(%s)", SquishCfg);
   if(strlen(temp) > 33)
      {
      temp[33]='\0';
      temp[32]='.';
      temp[31]='.';
      }
   print(13,36,7,temp);


	while( (line=xgetline(squishfile)) != NULL)
      {
      if(!(lineno++ % 5))
         working(13,7,7);


      if (line[0] == ';' || strlen(line) < 11)    /* Quick check for speed */
         continue;

      if ( (keyword = strtok(line, " \t\r")) != NULL)
         {
         if ( strcmpi(keyword, "EchoArea") == 0 )
            AddEchoArea(strtok(NULL, "\n"));
         else if (strcmpi(keyword, "AreasBBS") == 0 )
            AreasBBS = xstrdup(xgstr(strtok(NULL, " \t\n")));
         else if (strcmpi(keyword, "ArcMailAttach") == 0)
            cfg.usr.arcmail = 1;
         }
      }

   xclose(squishfile);

   print(13,7,9,"û");

}


/* ------------------------------------------------------- */
/* --  Read a Areas.bbs file and get the defined areas  -- */
/* ------------------------------------------------------- */


void ReadAreasBBS()

{

   XFILE    *areasfile;
   char  *line, drive[MAXDRIVE], dir[MAXDIR], temppath[125];
   int skip = 0;    /* To skip first line */
   char *p, temp[80];
   unsigned lineno=0;

   areasfile=xopen(AreasBBS);

   if (areasfile==NULL)
      {
      fnsplit(SquishCfg, drive, dir, NULL, NULL);
      sprintf(temppath, "%s%s%s", drive, dir, AreasBBS);
      if ( (areasfile=xopen(temppath)) == NULL)
         {
         sprintf(msg, "Can't open areas.bbs file! (%s)", AreasBBS);
         Message(msg, -1, 0, YES);
         return;
         }
      }

   sprintf(temp, "(%s)", AreasBBS);
   if(strlen(temp) > 44)
     {
     temp[44]='\0';
     temp[43]='.';
     temp[42]='.';
     }
   print(15,25,7,temp);


	while( (line=xgetline(areasfile)) != NULL)
      {
      if(!(lineno++ % 5))
                    working(15,7,7);
      p = line;
      while( p && isspace(*p) )  p++;
      strcpy(line, p);

      if ((strlen(line) > 4) && (line[0] != ';') && (line[0] != '-') && skip++)
         AddAreasArea(line);
      }

   xclose(areasfile);

   print(15,7,9,"û");

}


void ReadFECFG()
{
   CONFIG *fecfg;
   int l, extraleft, n, left, toread;
   long areaoffset = 0L;
   int cfgfile;
   Area *areas;
   char temp[100];
   ExtensionHeader eh;
   SysAddress *aka_array;


   if( (cfgfile=open(FEcfg, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      {
      sprintf(msg, "Can't open %s!", FEcfg);
      Message(msg, -1, 0, YES);
      return;
      }

   sprintf(temp, "(%s)", FEcfg);
   if(strlen(temp) > 33)
      {
      temp[33]='\0';
      temp[32]='.';
      temp[31]='.';
      }
   print(13,36,7,temp);

   fecfg = xcalloc(1, sizeof(CONFIG));

   if(read(cfgfile, fecfg, sizeof(CONFIG)) != sizeof(CONFIG))
      {
      sprintf(msg, "Error reading %s!", FEcfg);
      Message(msg, -1, 0, YES);
      close(cfgfile);
      free(fecfg);
      return;
      }

   extraleft = fecfg->offset;
   if(fecfg->AkaCnt==0) fecfg->AkaCnt=1;
   aka_array = xcalloc(fecfg->AkaCnt, sizeof(SysAddress));
   if(fecfg->AkaCnt == 1)
      memmove(aka_array, fecfg->oldakas, sizeof(SysAddress));

   for(n=0; extraleft > 0; n++)
     {
     if(read(cfgfile, &eh, sizeof(ExtensionHeader)) != sizeof(ExtensionHeader))
        {
        sprintf(msg, "Error reading %s!", FEcfg);
        Message(msg, -1, 0, YES);
        close(cfgfile);
        free(fecfg);
        free(aka_array);
        return;
        }

     if(eh.type == EH_AKAS)
        {
        read(cfgfile, aka_array, eh.offset);
        break;
        }

     extraleft -= eh.offset;
     lseek(cfgfile, eh.offset, SEEK_CUR);
     }

   areaoffset = (long) sizeof(CONFIG) + (long) fecfg->offset + (long) (fecfg->NodeCnt * sizeof(Node));

   lseek(cfgfile, areaoffset, SEEK_SET);

   areas = xcalloc(25, sizeof(Area));  /* Read buffer to read disk */

   for(l=0, left = fecfg->AreaCnt; left > 0; left -= 25)
     {
     toread = min(left, 25);
     if(toread == 0)
        break;

     if(read(cfgfile, areas, (unsigned) (toread *sizeof(Area))) != (int) (toread * sizeof(Area)) )
        {
        sprintf(msg, "Error reading %s!", FEcfg);
        Message(msg, -1, 0, YES);
        close(cfgfile);
        free(fecfg);
        free(aka_array);
        free(areas);
        return;
        }

     for(l=0; l<toread; l++)
       {
       if(!(l % 5)) working(13,7,7);
       ana_FE_area(&areas[l], aka_array);
       }
     }

   close(cfgfile);
   free(areas);
   free(aka_array);

   print(13,7,9,"û");

}


/* ------------------------------------------ */

void ana_FE_area(Area *area, SysAddress *aka_array)
{
   AREA  *thisarea, *curarea;
   char tempaka[100];


   if( (area->flags.type == PT_BOARD) || (area->type != ECHOMAIL) )
        return;

   thisarea = init_area();

   if (area->flags.type == SQUISH)    /* Squish area */
        thisarea->base = MSGTYPE_SQUISH | MSGTYPE_ECHO;
   else if(area->flags.type == JAM)
        thisarea->base = MSGTYPE_JAM | MSGTYPE_ECHO;
   else if(area->flags.type == QBBS)
        thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;
   else
        thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;

   strlwr(area->name);

   thisarea->tag  = xstrdup(area->name);
   if(area->desc[0] != '\0')
        thisarea->desc = xstrdup(area->desc);
   else
        thisarea->desc = thisarea->tag;

   if(!(thisarea->base & MSGTYPE_HMB))
      {
      Strip_Trailing(area->path, '\\');
      thisarea->dir = xstrdup(area->path);
      }
   else
      {
      sprintf(tempaka, "%d", area->board);
      thisarea->dir = xstrdup(tempaka);
      }

   sprintf(tempaka, "-P%d:%d/%d.%d", aka_array[area->flags.aka].main.zone,
                                     aka_array[area->flags.aka].main.net,
                                     aka_array[area->flags.aka].main.node,
                                     aka_array[area->flags.aka].main.point);

   check_aka(thisarea, tempaka);

   AddLinkedArea(thisarea);

}


/* ------------------------------------------ */


void addmacro(char *name, int usenet)
{

          char     temp[40], *usenetptr, *macroptr, *toptr, *toaddrptr, *subjectptr;
          MACROLIST *thismacro;
   static MACROLIST *lastmacro;


   if(name == NULL) return;

   memset(temp, '\0', sizeof(temp));

   macroptr    = strtok(name, " ,\t");
   if(usenet)
     {
     if( (usenetptr = strtok(NULL, " ,\t")) == NULL)
       {
       sprintf(msg, "Illegal macro format in %s", name);
       Message(msg, -1, 0, YES);
       return;
       }
     }

   toptr       = strtok(NULL, ",\n\t\r");
   toaddrptr   = strtok(NULL, ",\n\t\r");
   subjectptr  = strtok(NULL, "\n\t\r");

   if( (!macroptr) || (!toptr) )
      {
      sprintf(msg, "Illegal macro format in %s", name);
      Message(msg, -1, 0, YES);
      return;
      }

   thismacro = xcalloc(1, sizeof(MACROLIST));

   if(usenet)
      strcpy(thismacro->usenet, usenetptr);

   strcpy(thismacro->macro,  macroptr);
   strcpy(thismacro->toname, toptr);

   if(toaddrptr)
        {
        address_expand(toaddrptr, &thismacro->toaddress, -1);
        if(subjectptr)
          {
          strcpy(thismacro->subject, subjectptr);
          }
        }

   thismacro->next = NULL;

   if (cfg.firstmacro == NULL)           /* first name */
        cfg.firstmacro = thismacro;
   else                             /* establish link */
        lastmacro->next = thismacro;

   lastmacro = thismacro;

}


void getcolour(char *keyword, char *value)
{

   if(value == NULL) return;


   if(strcmp(keyword, "COLOR_ASBAR")==0)
        cfg.col.asbar = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_ASFRAME")==0)
        cfg.col.asframe = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_ASTEXT")==0)
        cfg.col.astext = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_ASHIGH")==0)
        cfg.col.ashigh = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_ASSPECIAL")==0)
        cfg.col.asspecial = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGHEADER")==0)
        cfg.col.msgheader = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGDATA")==0)
        cfg.col.msgdata = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGLINKS")==0)
        cfg.col.msglinks = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGATTRIBS")==0)
        cfg.col.msgattribs = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGDATE")==0)
        cfg.col.msgdate = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGLINE")==0)
        cfg.col.msgline = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGQUOTE")==0)
        cfg.col.msgquote = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGTEXT")==0)
        cfg.col.msgtext = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGKLUDGE")==0)
        cfg.col.msgkludge = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGORIGIN")==0)
        cfg.col.msgorigin = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGBAR")==0)
        cfg.col.msgbar = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_MSGSPECIAL")==0)
        cfg.col.msgspecial = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_ENTRY")==0)
        cfg.col.entry = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_EDITCURNORMAL")==0)
        cfg.col.editcurnormal = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_EDITCURQUOTE")==0)
        cfg.col.editcurquote = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_EDITHCR")==0)
        cfg.col.edithcr = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_EDITBLOCK")==0)
        cfg.col.editblock = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_EDITCURBLOCK")==0)
        cfg.col.editcurblock = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_POPFRAME")==0)
        cfg.col.popframe = (word) atoi(value);

   else if(strcmp(keyword, "COLOR_POPTEXT")==0)
        cfg.col.poptext = (word) atoi(value);

   else
      {
      sprintf(msg, "Unknown colour: %-27.27s", keyword);
      Message(msg, -1, 0, YES);
      }

}


void startup(char *what)
{

   if(what == NULL) return;


   if(strcmpi(what, "yes")==0)
      stuff = 287;
   else if(strcmpi(what, "personal")==0)
      stuff = 281;
/*      stuffkey(281); */
   else if(strcmpi(what, "no")!=0)
      {
      sprintf(msg, "Invalid value (%s) for Startup_Scan keyword!", what);
      Message(msg, -1, 0, YES);
      }

}


/* Choose from Yes, No or Ask */

int one_of_three(char *what)
{

  if(what != NULL)
     {
     if(strcmpi(what, "no" ) == 0) return 0;
     if(strcmpi(what, "yes") == 0) return 1;
     if(strcmpi(what, "ask") == 0) return 2;
     }

  sprintf("Error: %s should be 'Yes', 'No' or 'Ask'!", what);
  Message(msg, -1, 0, YES);

  return 0;
}


/* Add an entry to the list of passthru tags from squish.cfg */

void add_to_ptlist(char *tag)
{
   PTLIST *curtag;
   static PTLIST *last=NULL;

   curtag = xcalloc(1, sizeof(PTLIST));

   curtag->tag = xstrdup(tag);
   if(firstpt == NULL)
      firstpt = curtag;
   else last->next = curtag;

   last = curtag;

}


/* See if a tag is in the Passthru list from Squish.cfg */

int checkpt(char *tag)
{
   PTLIST *curtag=firstpt;

   while(curtag)
     {
     if(strcmpi(curtag->tag, tag)==0)
       return 1;
     curtag = curtag->next;
     }

   return 0;

}

/* Free mem allocated for ptlist */

void freeptlist(void)
{
   PTLIST *current;

   while(firstpt)
     {
     current=firstpt->next;
     free(firstpt->tag);
     free(firstpt);
     firstpt=current;
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

   sys = xcalloc(1, sizeof(SETUP_GE));

   SetupGE = open(GEchocfg, O_RDONLY|O_DENYNONE|O_BINARY);
   if (SetupGE == -1)
   {
      Message("Unable to open SETUP.GE", -1, 0, YES);
      return;
   }
   result = read(SetupGE, sys, sizeof(SETUP_GE));
   close(SetupGE);
/*   if (result < sizeof(SETUP_GE))
   {
      Message("Error reading SETUP.GE", -1, 0, YES);
      return;
   } */

   if (sys->sysrev != GE_THISREV)
   {
      Message("System file revision level mismatch", -1, 0, YES);
      return;
   }

/*
**  Opening AREAFILE.GE and checking the header
*/
   fnsplit(GEchocfg, drive, dir, NULL, NULL);
   free(GEchocfg);
   fnmerge(temp, drive, dir, "areafile", "ge");
   GEchocfg = xstrdup(temp);

   AreaFileGE = open(GEchocfg, O_RDONLY|O_DENYNONE|O_BINARY);
   if (AreaFileGE == -1)
   {
      Message("Unable to open AREAFILE.GE", -1, 0, YES);
      return;
   }
   result = read(AreaFileGE, &AreaHdr, sizeof(AREAFILE_HDR));
   if (result < sizeof(AREAFILE_HDR))
   {
      Message("Error reading AREAFILE header", -1, 0, YES);
      close(AreaFileGE);
      return;
   }

   sprintf(temp, "(%s)", GEchocfg);
   if(strlen(temp) > 33)
      {
      temp[33]='\0';
      temp[32]='.';
      temp[31]='.';
      }
   print(13,36,7,temp);


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

   arearecsize = AreaHdr.systems * sizeof(EXPORTENTRY) + AreaHdr.recsize;
   records = (filelength(AreaFileGE) - AreaHdr.hdrsize) / arearecsize;


   for (counter = 0; counter < records; counter++)
   {
      if(!(counter % 5)) working(13,7,7);
      lseek(AreaFileGE, (long) AreaHdr.hdrsize + (long) arearecsize * counter, SEEK_SET);
      result = read(AreaFileGE, &AreaFile, sizeof(AREAFILE_GE));
      if (result < sizeof(AREAFILE_GE)) break;
      if ((AreaFile.options & REMOVED) == 0)
         ana_GECHO_area(&AreaFile, (ADDRESS *)&sys->aka);
   }

   close(AreaFileGE);

   free(sys);
   print(13,7,9,"û");

}

/* Analyse area found in GEcho areafile */

void ana_GECHO_area(AREAFILE_GE *area, ADDRESS *aka_array)
{
   AREA  *thisarea, *curarea;
   char tempaka[100];

   if( area->areatype != ECHOMAIL )
        return;

   if(area->areaformat == FORMAT_PT)
        return;

   thisarea = init_area();

   strlwr(area->name);

   thisarea->tag  = xstrdup(area->name);
   if(area->comment[0] != '\0')
        thisarea->desc = xstrdup(area->comment);
   else
        thisarea->desc = thisarea->tag;

   if (area->areaformat == FORMAT_SQ)    /* Squish area */
        thisarea->base = MSGTYPE_SQUISH | MSGTYPE_ECHO;
   else if(area->areaformat == FORMAT_JAM)
        thisarea->base = MSGTYPE_JAM | MSGTYPE_ECHO;
   else if(area->areaformat == FORMAT_HMB)
        thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;
   else
        thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;

   if(thisarea->base & MSGTYPE_HMB)
     {
     itoa(area->areanumber, tempaka, 10);
     thisarea->dir = xstrdup(tempaka);
     }
   else
     {
     Strip_Trailing(area->path, '\\');
     thisarea->dir = xstrdup(area->path);
     }

   sprintf(tempaka, "-P%d:%d/%d.%d", aka_array[area->pkt_origin].zone,
                                     aka_array[area->pkt_origin].net,
                                     aka_array[area->pkt_origin].node,
                                     aka_array[area->pkt_origin].point);

   check_aka(thisarea, tempaka);

   AddLinkedArea(thisarea);
}



void ReadImailCFG()
{
   im_config_type *imcfg;
   int cfgfile;
   areas_record_type area;
   char temp[120];
   char drive[MAXDRIVE], dir[MAXDIR];
   int counter=0;


   if( (cfgfile=open(Imailcfg, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      {
      sprintf(msg, "Can't open %s!", Imailcfg);
      Message(msg, -1, 0, YES);
      return;
      }

   sprintf(temp, "(%s)", Imailcfg);
   if(strlen(temp) > 33)
      {
      temp[33]='\0';
      temp[32]='.';
      temp[31]='.';
      }
   print(13,36,7,temp);

   imcfg = xcalloc(1, sizeof(im_config_type));

   if(read(cfgfile, imcfg, sizeof(im_config_type)) != sizeof(im_config_type))
      {
      sprintf(msg, "Error reading %s!", Imailcfg);
      Message(msg, -1, 0, YES);
      close(cfgfile);
      free(imcfg);
      return;
      }

   close(cfgfile);

   fnsplit(Imailcfg, drive, dir, NULL, NULL);
   free(Imailcfg);
   fnmerge(temp, drive, dir, "imail", "ar");
   Imailcfg = xstrdup(temp);

   if( (cfgfile=open(Imailcfg, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      {
      sprintf(msg, "Can't open %s!", Imailcfg);
      Message(msg, -1, 0, YES);
      return;
      }


   while(read(cfgfile, &area, sizeof(area)) == sizeof(area) )
        {
        if(!(counter++ % 5)) working(13,7,7);
        ana_Imail_area(&area, &imcfg->aka);
        }


   close(cfgfile);
   free(imcfg);

   print(13,7,9,"û");

}


/* ------------------------------------------ */

void ana_Imail_area(areas_record_type *area, naddress *aka_array)
{
   AREA  *thisarea, *curarea;
   char tempaka[100];


   if( (area->deleted) || (!area->active) )
        return;

   if( !IsEcho(area->msg_base_type) ) /* Only take echomail */
        return;

   /* Only take supported msgbase formats */

   if( IsPassth(area->msg_base_type) )
       return;

   thisarea = init_area();

   if (IsSquish(area->msg_base_type))    /* Squish area */
        thisarea->base = MSGTYPE_SQUISH | MSGTYPE_ECHO;
   else if(IsJam(area->msg_base_type))
        thisarea->base = MSGTYPE_JAM | MSGTYPE_ECHO;
   else if( (IsQbbs(area->msg_base_type)) ||
            (IsHudson(area->msg_base_type)) )
        thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;
   else
        thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;


   strlwr(area->aname);

   thisarea->tag  = xstrdup(area->aname);
   if(area->comment[0] != '\0')
        thisarea->desc = xstrdup(area->comment);
   else
        thisarea->desc = thisarea->tag;

   if(!(thisarea->base & MSGTYPE_HMB))  // We need a path.
     {
     Strip_Trailing(area->msg_path, '\\');
     thisarea->dir = xstrdup(area->msg_path);
     }
   else   // We need a board.
     {
     itoa(area->brd, tempaka, 10);
     thisarea->dir = xstrdup(tempaka);
     }


   sprintf(tempaka, "-P%d:%d/%d.%d", aka_array[area->o_addr-1].zone,
                                     aka_array[area->o_addr-1].net,
                                     aka_array[area->o_addr-1].node,
                                     aka_array[area->o_addr-1].point);

   check_aka(thisarea, tempaka);

   AddLinkedArea(thisarea);
}


void ReadxMailCFG()
{
   FILE *cfgfile;
   xMailArea area;
   char temp[100];
   word areano=0;
   int counter=0;


   if( (cfgfile=_fsopen(xMailcfg, "rb", SH_DENYNO)) == NULL)
      {
      sprintf(msg, "Can't open %s!", xMailcfg);
      Message(msg, -1, 0, YES);
      return;
      }

   setvbuf(cfgfile, NULL, _IOFBF, 4096);

   sprintf(temp, "(%s)", xMailcfg);
   if(strlen(temp) > 33)
      {
      temp[33]='\0';
      temp[32]='.';
      temp[31]='.';
      }
   print(13,36,7,temp);

   while(fread(&area, sizeof(area), 1, cfgfile) == 1 )
        {
        if(!(counter++ % 5)) working(13,7,7);
        ana_xMail_area(&area, ++areano);
        }

   fclose(cfgfile);

   print(13,7,9,"û");


}



void ReadFmailCFG()
{
   configType *fmcfg;
   int cfgfile;
   rawEchoTypeOld oldarea;
   rawEchoType area;
   char temp[120];
   char drive[MAXDRIVE], dir[MAXDIR];
   int counter=0;
   headerType hdr;

   if( (cfgfile=open(Fmailcfg, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      {
      sprintf(msg, "Can't open %s!", Imailcfg);
      Message(msg, -1, 0, YES);
      return;
      }

   sprintf(temp, "(%s)", Fmailcfg);
   if(strlen(temp) > 33)
      {
      temp[33]='\0';
      temp[32]='.';
      temp[31]='.';
      }
   print(13,36,7,temp);

   fmcfg = xcalloc(1, sizeof(configType));

   if(read(cfgfile, fmcfg, sizeof(configType)) != sizeof(configType))
      {
      sprintf(msg, "Error reading %s!", Fmailcfg);
      Message(msg, -1, 0, YES);
      close(cfgfile);
      free(fmcfg);
      return;
      }

   close(cfgfile);

//   if( (fmcfg->versionMajor == 0) && (fmcfg->versionMinor < 97) )
//       oldtype = 1;   Didin't work on .97 beta, identified as 0.94!

   fnsplit(Fmailcfg, drive, dir, NULL, NULL);
   free(Fmailcfg);
   fnmerge(temp, drive, dir, "fmail", "ar");
   Fmailcfg = xstrdup(temp);

   if( (cfgfile=open(Fmailcfg, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      {
      sprintf(msg, "Can't open %s!", Fmailcfg);
      Message(msg, -1, 0, YES);
      return;
      }

   if(read(cfgfile, &hdr, sizeof(headerType)) != sizeof(headerType))
      {
      sprintf(msg, "Error reading %s!", Fmailcfg);
      Message(msg, -1, 0, YES);
      close(cfgfile);
      return;
      }

   if(strncmpi(hdr.versionString, "fmail", 5) != 0)   // Old type?
      {
      lseek(cfgfile, 0L, SEEK_SET);

      while(read(cfgfile, &oldarea, sizeof(oldarea)) == sizeof(oldarea) )
           {
           if(!(counter++ % 5)) working(13,7,7);
           old_ana_Fmail_area(&oldarea, &fmcfg->akaList);
           }
      }
   else
      {
      while(read(cfgfile, &area, sizeof(area)) == sizeof(area) )
           {
           if(!(counter++ % 5)) working(13,7,7);
           ana_Fmail_area(&area, &fmcfg->akaList);
           }
      }

   close(cfgfile);
   free(fmcfg);

   print(13,7,9,"û");

}


void old_ana_Fmail_area(rawEchoTypeOld *area, nodeFakeType *aka_array)
{
   AREA  *thisarea, *curarea;
   char tempaka[100];


   if( (!area->options.active) || (area->options.local) || (area->options.disconnected) )
        return;

   if(area->board == 0)
         return; /* Passthru */

   /* Passthru? */

   thisarea = init_area();

   strlwr(area->areaName);

   thisarea->tag  = xstrdup(area->areaName);
   if(area->comment[0] != '\0')
        thisarea->desc = xstrdup(area->comment);
   else
        thisarea->desc = thisarea->tag;

   itoa(area->board, tempaka, 10);
   thisarea->dir = xstrdup(tempaka);

   thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;

   sprintf(tempaka, "-P%d:%d/%d.%d", aka_array[area->address].nodeNum.zone,
                                     aka_array[area->address].nodeNum.net,
                                     aka_array[area->address].nodeNum.node,
                                     aka_array[area->address].nodeNum.point);

   check_aka(thisarea, tempaka);

   AddLinkedArea(thisarea);
}

void ana_Fmail_area(rawEchoType *area, nodeFakeType *aka_array)
{
   AREA  *thisarea, *curarea;
   char tempaka[100];


   if( (!area->options.active) || (area->options.local) || (area->options.disconnected) )
        return;

   if( (area->board == 0) && (area->msgBasePath[0] == '\0') )
         return; /* Passthru */

   /* Passthru? */

   thisarea = init_area();

   strlwr(area->areaName);

   thisarea->tag  = xstrdup(area->areaName);
   if(area->comment[0] != '\0')
        thisarea->desc = xstrdup(area->comment);
   else
        thisarea->desc = thisarea->tag;

   if(area->board != 0)
      {
      itoa(area->board, tempaka, 10);
      thisarea->dir = xstrdup(tempaka);
      thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;
      }
   else
      {
      thisarea->dir = xstrdup(area->msgBasePath);
      thisarea->base = MSGTYPE_JAM | MSGTYPE_ECHO;
      }

   sprintf(tempaka, "-P%d:%d/%d.%d", aka_array[area->address].nodeNum.zone,
                                     aka_array[area->address].nodeNum.net,
                                     aka_array[area->address].nodeNum.node,
                                     aka_array[area->address].nodeNum.point);

   check_aka(thisarea, tempaka);

   AddLinkedArea(thisarea);
}


void ana_xMail_area(xMailArea *area, word areano)
{
   AREA  *thisarea, *curarea;
   char tempaka[100];

   if((area->name[0] == '\0') || (area->type != 0))
        return;

   if(area->base == xPT)
      return;

   /* Passthru? */

   thisarea = init_area();

   thisarea->tag  = xpstrdup(area->name);
   strlwr(thisarea->tag);
   if(area->desc[0] != '\0')
        thisarea->desc = xpstrdup(area->desc);
   else
        thisarea->desc = thisarea->tag;

   if(area->base == xHMB)
      thisarea->base = MSGTYPE_HMB | MSGTYPE_ECHO;
   if(area->base == xSQ)
      thisarea->base = MSGTYPE_SQUISH | MSGTYPE_ECHO;
   if(area->base == xJAM)
      thisarea->base = MSGTYPE_JAM | MSGTYPE_ECHO;
   if(area->base == xSDM)
      thisarea->base = MSGTYPE_SDM | MSGTYPE_ECHO;


   if(thisarea->base & MSGTYPE_HMB)
     {
     itoa(areano, tempaka, 10);
     thisarea->dir = xstrdup(tempaka);
     }
   else
      thisarea->dir = xpstrdup(area->dir);


   sprintf(tempaka, "-P%d:%d/%d.%d", area->aka.zone,
                                     area->aka.net,
                                     area->aka.node,
                                     area->aka.point);

   check_aka(thisarea, tempaka);

   AddLinkedArea(thisarea);
}


char *xpstrdup(char *str)
{
   char *out;

   out = xcalloc(1, str[0]+1);

   memmove(out, str + 1, str[0]);

   return out;

}



void ReadConfigFile(char *temp, int showname)
{
   XFILE *cfgfile;
   char *line;
   char show[120];
   int counter=0;


   if(showname)
      {
      print(17,7,9,"û");
      sprintf(show, "(%s)", temp);
      if(strlen(show) > 33)
        {
        show[42]='\0';
        show[41]='.';
        show[40]='.';
        }
      print(17,30,7,show);
      }

   if ((cfgfile = xopen(temp)) == NULL)
      {
      sprintf(msg, " Darn! I can't open %s! ", temp);
      Message(msg, -1, 254, YES);
      }

   while((line = xgetline(cfgfile)) != NULL)
     {
     if(showname)
        if(!(counter++ % 5)) working(17,7,7);

     if ((line[0] == ';') || (strlen(line) < 5))
        continue;

     analyse(line);

     }

   xclose(cfgfile);

   if(showname)
      print(17,7,9,"û");


}



/* ---------------------------------------------------- */
/* - Routine to add a message area to the linked list - */
/* - It will also check if it is a dupe               - */
/* ---------------------------------------------------- */

void AddLinkedArea(AREA *thisarea)
{
   AREA *curarea = cfg.first;

   while(curarea)
      {
      if (strcmpi(curarea->tag, thisarea->tag) == 0)           /* Dupe! */
         {
         if( thisarea->desc && (thisarea->desc != thisarea->tag) )
            free(thisarea->desc);
         if(thisarea->tag) free(thisarea->tag);
         if(thisarea->dir) free(thisarea->dir);
         free(thisarea);
         return;
         }
      curarea = curarea->next;
      }


   if (cfg.first == NULL)           /* first area */
       cfg.first = thisarea;
   else                             /* establish link */
      {
      last->next = thisarea;
      thisarea->prev=last;
      }

   last = thisarea;

}
