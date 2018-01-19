/*
**  SHEL2DOS.C - Shell to DOS from a running program
**
**  Original Copyright 1989-1991 by Bob Stout as part of
**  the MicroFirm Function Library (MFL)
**
**  This subset version is hereby donated to the public domain.
*/

#include "includes.h"

#ifdef __WATCOMC__
  #define MAXPATH _MAX_PATH

#endif


int    GetCommandLine(char **progptr, char **parmsptr, AREA *area, MSG *areahandle, MMSG *curmsg);


int shell_to_DOS(void)
{
      static char *comspec, prompt[256], oldprompt[256];
      char *ptr, *curdir;
      int retval=0;
      unsigned drive, total;
      static char didswap=0;
      char temp[100];
      int clockstate, oldlines = maxy;

      curdir = getcwd(NULL, MAXPATH);
      #ifndef __WATCOMC__
      drive = getdisk();
      #else
      _dos_getdrive(&drive);
      #endif

      savescreen();

      clockstate=showclock;
      clockoff();

      cls();

      #ifdef __WATCOMC__
      _settextcursor(0x0607);
      #else
      _setcursortype(_NORMALCURSOR);
      #endif

      clrscr();

      sprintf(temp, "%s  (c) 1992-'96  Gerard van Essen (2:281/527)", myname);
      print(1,0,7,temp);

      print(3,0,7,"OS shell - type 'exit' to return to timEd..");

      MoveXY(1,5);

      comspec = getenv("COMSPEC");

      if(comspec == NULL)
      #ifndef __OS2__
           comspec = "COMMAND.COM";     /* Better than nothing... */
      #else
           comspec = "CMD.EXE";
      #endif

      #ifdef __WATCOMC__
      _heapmin();
      #endif


      if(!didswap)
         {
         memset(oldprompt, '\0', sizeof(oldprompt));
         memset(prompt, '\0', sizeof(prompt));

         if((ptr=getenv("PROMPT")) != NULL)
            {
            sprintf(oldprompt, "%0.200s", ptr);
            }

         sprintf(prompt, "PROMPT=[Type EXIT to return to timEd]\r\n%s",
                                                                   oldprompt);
         putenv(prompt);
         didswap = 1;
         }

      kbflush();


      #ifndef __FLAT__
      undo09();
      retval = do_exec("", "", USE_ALL|HIDE_FILE|CHECK_NET, (cfg.usr.status&SWAPSHELL) ? 0xFFFF : 0x0000, environ);
      ins09();
      #else
      retval = spawnlpe(P_WAIT, comspec, comspec, NULL, environ);
      #endif

     video_init();

     if(oldlines != maxy)
       {
       setlines(oldlines);
       video_init();
       }

      #ifdef __WATCOMC__
      _settextcursor(0x2000);
      #else
      _setcursortype(_NOCURSOR);
      #endif

     putscreen();

     if(clockstate == 1)
        clockon();

     #ifndef __WATCOMC__
     setdisk(drive);
     #else
     _dos_setdrive(drive, &total);
     #endif
     chdir(curdir);
//     if(curdir) direct_free(curdir);  //  !!!!!!!!!!!!!!!!!
     if(curdir) mem_free(curdir);

     return retval;
}


// =============================================================

int runexternal(AREA *area, MSG **areahandle, MMSG *curmsg, long lastorhigh, dword highest)
{
  char *curdir;
  int retval=0;
  unsigned drive, total;
  char * prog  = NULL,
       * parms = NULL;
  int clockstate, oldlines = maxy;
  char temp[200];

  curdir = getcwd(NULL, MAXPATH);
  #ifndef __WATCOMC__
  drive = getdisk();
  #else
  _dos_getdrive(&drive);
  #endif


  if(GetCommandLine(&prog, &parms, area, *areahandle, curmsg) == -1)
    return 0;

  UpdateLastread(area, lastorhigh, highest, *areahandle);

  if(MsgCloseArea(*areahandle))
     {
     Message("Error closing area!", 2, 0, NO);
     return 0;
     }

  write_echolog();

  savescreen();

  clockstate=showclock;
  clockoff();

  cls();

  vprint(0,0,7, "Executing: \"%s %s\"", prog, parms);
  MoveXY(1,3);

  #ifdef __WATCOMC__
  _settextcursor(0x0607);
  #else
  _setcursortype(_NORMALCURSOR);
  #endif

  #ifndef __FLAT__
  undo09();
  retval = do_exec(prog, parms, USE_ALL|HIDE_FILE|CHECK_NET, (cfg.usr.status&SWAPSHELL) ? 0xFFFF : 0x0000, environ);
  ins09();
  #else
  sprintf(temp, "%s %s", prog, parms);
  system(temp);
  #endif

  if(prog) mem_free(prog);
  if(parms) mem_free(parms);

  video_init();

  if(oldlines != maxy)
    {
    setlines(oldlines);
    video_init();
    }

  #ifdef __WATCOMC__
  _settextcursor(0x2000);
  #else
  _setcursortype(_NOCURSOR);
  #endif

  statusbar("Press any key to return to timEd");
  get_idle_key(1, GLOBALSCOPE);

  putscreen();

  if(clockstate == 1)
    clockon();

  #ifndef __WATCOMC__
  setdisk(drive);
  #else
  _dos_setdrive(drive, &total);
  #endif

  chdir(curdir);
//  if(curdir) direct_free(curdir);  // !!!!!!!!!!
  if(curdir) mem_free(curdir);

  if(!(*areahandle=MsgOpenArea(area->dir, MSGAREA_CRIFNEC, area->base)))
     {
     Message("Error re-opening area!", -1, 0, YES);
     showerror();
     return -1;
     }

  /* Read area statistics */

  ScanArea(area, *areahandle, 0);     /* Do a 'real' MSGAPI scan */

  return retval;
}

// =============================================================

int GetCommandLine(char **progptr, char **parmsptr, AREA *area, MSG *areahandle, MMSG *curmsg)
{
   BOX *inputbox;
   int ret;
   char prog[70] = "", parms[70] = "";   // Used for input strings.


   inputbox = initbox(10,0,15,79,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   drawbox(inputbox);
   boxwrite(inputbox,0,1,"Give the name of program & the parameters to use:");
   boxwrite(inputbox,2,1,"Program: ");
   boxwrite(inputbox,3,1,"Parms  : ");

   do {
     ret = getstring(13, 11, prog, 67, 69, "",cfg.col[Centry], cfg.col[Cpoptext]);
   } while(ret != ESC && ret != 0);

   if(ret == ESC)
     {
     delbox(inputbox);
     return -1;
     }

   do {
     ret = getstring(14, 11, parms, 67, 69, "",cfg.col[Centry], cfg.col[Cpoptext]);
   } while(ret != ESC && ret != 0);

   delbox(inputbox);

   if(ret == ESC) return -1;
   if(prog[0] == '\0') return -1;

   *progptr = mem_strdup(prog);

   if(parms[0] == '\0')
     {
     *parms = '\0';
     return 0;
     }

   *parmsptr = BuildCommandLine(parms, area, areahandle, curmsg, NULL, NULL);

   return 0;
}

// ==============================================================

char *BuildCommandLine(char *charptr, AREA *area, MSG *areahandle, MMSG *curmsg, char *curfile, char *newfile)
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
      if(strncmpi(charptr+1, "from", 4) == 0)
        {
        strcpy(temp, curmsg->mis.from);
        charptr += 5;
        }
      else if (strncmpi(charptr+1, "to", 2) == 0)
        {
        strcpy(temp, curmsg->mis.to);
        charptr += 3;
        }
      else if (strncmpi(charptr+1, "subject", 7) == 0)
        {
        strcpy(temp, curmsg->mis.subj);
        charptr += 8;
        }
      else if (strncmpi(charptr+1, "attach", 6) == 0)
        {
        SumAttachesRequests(&curmsg->mis, temp, 100, SARattach);
        charptr += 7;
        }
       else if (strncmpi(charptr+1, "request", 7) == 0)
        {
        SumAttachesRequests(&curmsg->mis, temp, 100, SARrequest);
        charptr += 8;
        }
       else if (strncmpi(charptr+1, "orig", 4) == 0)
        {
        strcpy(temp, FormAddress(&curmsg->mis.origfido));
        charptr += 5;
        }
      else if (strncmpi(charptr+1, "dest", 4) == 0)
        {
        strcpy(temp, FormAddress(&curmsg->mis.destfido));
        charptr += 5;
        }
      else if (strncmpi(charptr+1, "areatype", 8) == 0)
        {
        if(area->type == NETMAIL) strcpy(temp, "netmail");
        else if(area->type == MAIL) strcpy(temp, "mail");
        else if(area->type == ECHOMAIL) strcpy(temp, "echomail");
        else if(area->type == LOCAL) strcpy(temp, "local");
        else if(area->type == NEWS) strcpy(temp, "news");
        charptr += 9;
        }
      else if (strncmpi(charptr+1, "areatag", 7) == 0)
        {
        strcpy(temp, area->tag);
        charptr += 8;
        }
      else if (strncmpi(charptr+1, "format", 6) == 0)
        {
        if(area->base & MSGTYPE_JAM) strcpy(temp, "jam");
        else if(area->base & MSGTYPE_SDM) strcpy(temp, "sdm");
        else if(area->base & MSGTYPE_HMB) strcpy(temp, "hmb");
        else if(area->base & MSGTYPE_SQUISH) strcpy(temp, "squish");
        charptr += 7;
        }
      else if (strncmpi(charptr+1, "areadir", 7) == 0)
        {
        strcpy(temp, area->dir);
        charptr += 8;
        }
      else if (strncmpi(charptr+1, "msgno", 5) == 0)
        {
        if(areahandle)
          sprintf(temp, "%lu", MsgGetCurMsg(areahandle));
        else
          strcpy(temp, "0");
        charptr += 6;
        }
      else if (strncmpi(charptr+1, "realmsgno", 9) == 0)
        {
        sprintf(temp, "%lu", curmsg->mis.msgno);
        charptr += 10;
        }
      else if (strncmpi(charptr+1, "file", 4) == 0)
        {
        if(curfile)
           strcpy(temp, curfile);
        else
           strcpy(temp, "unknown");
        charptr += 5;
        }
      else if (strncmpi(charptr+1, "newfile", 7) == 0)
        {
        if(newfile)
           strcpy(temp, newfile);
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

int runaprog(char *prog, char *parms, int waitforkey)
{
  char *curdir;
  int retval=0;
  unsigned drive, total;
  int clockstate, oldlines = maxy;
  char temp[200];

  curdir = getcwd(NULL, MAXPATH);
  #ifndef __WATCOMC__
  drive = getdisk();
  #else
  _dos_getdrive(&drive);
  #endif

  savescreen();

  clockstate=showclock;
  clockoff();

  cls();

  vprint(0,0,7, "Executing: \"%s %s\"", prog, parms);
  MoveXY(1,3);

  #ifdef __WATCOMC__
  _settextcursor(0x0607);
  #else
  _setcursortype(_NORMALCURSOR);
  #endif

  #ifndef __FLAT__
  undo09();
  retval = do_exec(prog, parms, USE_ALL|HIDE_FILE|CHECK_NET, (cfg.usr.status&SWAPSHELL) ? 0xFFFF : 0x0000, environ);
  ins09();
  #else
  sprintf(temp, "%s %s", prog, parms);
  retval = system(temp);
  #endif

  video_init();

  if(oldlines != maxy)
    {
    setlines(oldlines);
    video_init();
    }

  #ifdef __WATCOMC__
  _settextcursor(0x2000);
  #else
  _setcursortype(_NOCURSOR);
  #endif

  if(waitforkey)
    {
    statusbar("Press any key to return to timEd");
    get_idle_key(1, GLOBALSCOPE);
    }

  putscreen();

  if(clockstate == 1)
    clockon();

  #ifndef __WATCOMC__
  setdisk(drive);
  #else
  _dos_setdrive(drive, &total);
  #endif

  chdir(curdir);
//  if(curdir) direct_free(curdir);  // !!!!!!!!!!!!
  if(curdir) mem_free(curdir);

  return retval;
}

// =============================================================

int editrunexternal(AREA *area, MSG *areahandle, MMSG *curmsg)
{
  char * prog  = NULL,
       * parms = NULL;

  if(GetCommandLine(&prog, &parms, area, areahandle, curmsg) == -1)
    return 0;

  runaprog(prog, parms, 1);

  if(prog) mem_free(prog);
  if(parms) mem_free(parms);

  return 0;
}

// ==============================================================

int FileDelete(void)
{
  BOX  *inputbox;
  int ret;
  char filename[80] = "";

  inputbox = initbox(10,2,15,77,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
  drawbox(inputbox);
  boxwrite(inputbox,0,1,"Give the name of the file to delete:");
  ret = getstring(13, 4, filename, 72, 79, "",cfg.col[Centry], cfg.col[Cpoptext]);
  delbox(inputbox);

  if (ret == ESC) return 0;

  unlink(filename);

  return 0;
}

// ==============================================================
