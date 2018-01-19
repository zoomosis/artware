#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <ctype.h>
#include <sys\stat.h>

#include <msgapi.h>
#include <progprot.h>
#include "nstruct.h"
#include "txtbuild.h"
#include "netfuncs.h"
#include "xfile.h"
#include "binkpack.h"
#include "command.h"

#ifndef __FLAT__

   #include <exec.h>

#endif


extern CFG         cfg;
extern MESSAGE     msg;
extern COMMANDINFO CommandInfo;
extern XMASKLIST   *firstxmask;


#ifdef __WATCOMC__
  #define MAXPATH _MAX_PATH

#endif

// Some defines for SumAttachRequests

#define SARattach    0x01
#define SARrequest   0x02
#define SARboth      0x04


char *BuildCommandLine(char *charptr, char *curfile, char *newfile, char *repfile, char *curareadir);
int runexternal(char *prog, char *args, char *curareadir);
void SumAttachesRequests(MIS *mis, char *temp, int maxlen, int what);

// ==============================================================

char * BuildCommandLine(char *charptr, char *curfile, char *newfile, char *repfile, char *curareadir)
{
  char *tmpout, *out, *tmpptr;
  int len;
  char temp[120];

  tmpptr = tmpout = mem_calloc(1, 2048);

  while(charptr && *charptr != '\0')
    {
    if(*charptr != '[' || *(charptr+1) == '[')  // No 'variable' or two [[
      *tmpptr++ = *charptr++;
    else
      {
      if(strnicmp(charptr+1, "from", 4) == 0)
        {
        strcpy(temp, msg.mis.from);
        charptr += 5;
        }
      else if (strnicmp(charptr+1, "to", 2) == 0)
        {
        strcpy(temp, msg.mis.to);
        charptr += 3;
        }
      else if (strnicmp(charptr+1, "subject", 7) == 0)
        {
        strcpy(temp, msg.mis.subj);
        charptr += 8;
        }
      else if (strnicmp(charptr+1, "attach", 6) == 0)
        {
        SumAttachesRequests(&msg.mis, temp, 100, SARattach);
        charptr += 7;
        }
       else if (strnicmp(charptr+1, "request", 7) == 0)
        {
        SumAttachesRequests(&msg.mis, temp, 100, SARrequest);
        charptr += 8;
        }
       else if (strnicmp(charptr+1, "orig", 4) == 0)
        {
        strcpy(temp, fancy_address(&msg.mis.origfido));
        charptr += 5;
        }
      else if (strnicmp(charptr+1, "dest", 4) == 0)
        {
        strcpy(temp, fancy_address(&msg.mis.destfido));
        charptr += 5;
        }
      else if (strnicmp(charptr+1, "areadir", 7) == 0)
        {
        strcpy(temp, curareadir);
        charptr += 8;
        }
      else if (strnicmp(charptr+1, "msgno", 5) == 0)
        {
        sprintf(temp, "%lu", MsgUidToMsgn(msg.areahandle, msg.mis.msgno, UID_EXACT));
        charptr += 6;
        }
      else if (strnicmp(charptr+1, "realmsgno", 9) == 0)
        {
        sprintf(temp, "%lu", msg.mis.msgno);
        charptr += 10;
        }
      else if (strnicmp(charptr+1, "file", 4) == 0)
        {
        if(curfile)
           strcpy(temp, curfile);
        else
           strcpy(temp, "unknown");
        charptr += 5;
        }
      else if (strnicmp(charptr+1, "newfile", 7) == 0)
        {
        if(newfile)
           strcpy(temp, newfile);
        else
           strcpy(temp, "unknown");
        charptr += 8;
        }
      else if (strnicmp(charptr+1, "repfile", 7) == 0)
        {
        if(repfile)
           strcpy(temp, repfile);
        else
           strcpy(temp, "unknown");
        charptr += 8;
        }
       else
        {
        temp[0] = *charptr;    // Unknown, just copy it..
        temp[1] = '\0';
        charptr += 1;
        }

      len = strlen(temp);
      memcpy(tmpptr, temp, len);
      tmpptr  += len;
 
      if(*charptr == ']')  charptr++;     // If it ends with ], skip it.
      }
    }

  if(*tmpout != '\0')       // Nothing?
    out = mem_strdup(tmpout);
  else
    out = NULL;

  mem_free(tmpout);
  return out;
}

// ======================================================================

int runaprog(char *prog, char *parms)
{
  char *curdir = NULL;
  int retval=0;
  unsigned drive, total;

  curdir = getcwd(NULL, MAXPATH);
  #ifndef __WATCOMC__
  drive = getdisk();
  #else
  _dos_getdrive(&drive);
  #endif

  print_and_log("Executing: \"%s %s\" \n", prog, parms);
  printf("\n");

  #ifndef __FLAT__
  retval = do_exec(prog, parms, USE_ALL|HIDE_FILE|CHECK_NET, 0xFFFF, environ);
  #else
  {
    char temp[200];
    sprintf(temp, "%s %s", prog, parms);
    retval = system(temp);
  }
  #endif

  #ifndef __WATCOMC__
  setdisk(drive);
  #else
  _dos_setdrive(drive, &total);
  #endif

  chdir(curdir);
  if(curdir) mem_free(curdir);

  return retval;
}

// =============================================================

int runexternal(char *prog, char *args, char *curareadir)
{
  char *parms = NULL;
  char path[_MAX_PATH],
       file[_MAX_PATH],
       newfile[_MAX_PATH],
       repfile[_MAX_PATH];

  if(cfg.cfgdir[0] != '\0')
    strcpy(path, cfg.cfgdir);
  else
    strcpy(path, cfg.homedir);

  sprintf(file,    "%s\\netmsg.msg", path);
  sprintf(newfile, "%s\\netmsg.new", path);
  sprintf(repfile, "%s\\netmsg.rep", path);

  unlink(file); unlink(newfile); unlink(repfile);

  writemsg(MsgUidToMsgn(msg.areahandle, msg.mis.msgno, UID_EXACT), file, WRITE_BODY);

  parms = BuildCommandLine (args, file, newfile, repfile, curareadir);

  runaprog(prog, parms);

  if(parms) mem_free(parms);

  if(FileExists(newfile))
     AddNote(newfile, MsgUidToMsgn(msg.areahandle, msg.mis.msgno, UID_EXACT), 1);

  if(FileExists(repfile))
    {
    bounce(repfile, MsgUidToMsgn(msg.areahandle, msg.mis.msgno, UID_EXACT), NULL, XEMPTYBOUNCE, NULL, 1, NULL);
    }

  unlink(file); unlink(newfile); unlink(repfile);

  return 0;
}

// ==============================================================
//
//  Make a list of attaches/requests, outpout in 'temp', maximum
//  length (not including '\0') is maxlen.
//
// ===============================================================


void SumAttachesRequests(MIS *mis, char *temp, int maxlen, int what)
{
  STRINGLIST *files;
  char temp2[80];

  *temp = '\0';       // Start with a clean slate.

  if(maxlen > 132) maxlen = 132;

  if(what == SARattach)
    files = mis->attached;
  else if(what == SARrequest)
    files = mis->requested;
  else   // Both
    {
    if(mis->attached)
      files = mis->attached;
    else
      files = mis->requested;
    }

  while(files)
    {
    if(files->s != NULL)
      {
      strncpy(temp2, files->s, 70);
      temp2[70] = '\0';
      if(files->pw)
        {
        if((strlen(temp2) + strlen(files->pw) + 2) < 71)
           {
           strcat(temp2, " !");
           strcat(temp2, files->pw);
           }
        }

      if((strlen(temp) + strlen(temp2)) < 71)
        {
        if(strlen(temp)) strcat(temp, " ");
        strcat(temp, temp2);
        }
      }
    files = files->next;
    }

}

// ==============================================================

