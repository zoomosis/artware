/*
   --- Version 3.4 93-06-22 13:40 ---

   EXEC.C: EXEC function with memory swap - Prepare parameters.

   Public domain software by

        Thomas Wagner
        Ferrari electronic GmbH
        Beusselstrasse 27
        D-1000 Berlin 21
        Germany
*/

#include "compat.h"
#include "exec.h"
#include "checkpat.h"
#include <bios.h>

/*>e
   Set REDIRECT to 1 to support redirection, else to 0.
   CAUTION: The definition in 'spawn.asm' must match this definition!!
<*/
/*>d
   Setzen Sie REDIRECT auf 1 um Dateiumleitung zu untert�tzen, sonst auf 0.
   ACHTUNG: Die Definition in 'spawn.asm' mu� mit dieser Definition 
   �bereinstimmen!!
<*/


#define REDIRECT  0

#define SWAP_FILENAME "$$AAAAAA.AAA" 

/*e internal flags for prep_swap */
/*d interne Flags f�r prep_swap */

#define CREAT_TEMP      0x0080
#define DONT_SWAP_ENV   0x4000

#define ERR_COMSPEC     -7
#define ERR_NOMEM       -8


spawn_check_proc *spawn_check = NULL;

/*e local variables */
/*d Lokale Variablen */

static char drive [MAXDRIVE], dir [MAXDIR];
static char name [MAXFILE], ext [MAXEXT];
static char cmdpath [MAXPATH] = "";
static char cmdpars [80] = "";


#ifdef __cplusplus
extern "C" int
#else
extern int _cdecl
#endif
do_spawn (int swapping,     /*e swap if non-0 */
                            /*d Auslagern wenn nicht 0 */
          char *xeqfn,      /*e file to execute */
                            /*d auszuf�hrende Datei */
          char *cmdtail,    /*e command tail string */
                            /*d Kommandozeile */
          unsigned envlen,  /*e environment length */
                            /*d L�nge Umgebungsvariablen */
          char *envp        /*e environment pointer */
                            /*d Zeiger auf Umgebungsvariablen */
#if (REDIRECT)
          ,char *rstdin,    /*e redirection file names */
                            /*d Umleitungs-Dateinamen */
          char *rstdout,
          char *rstderr
#endif
          );

#ifdef __cplusplus
extern "C" int
#else
extern int _cdecl
#endif
prep_swap (int method,      /*e swap method */
                            /*d Auslagerungsmethode */
           char *swapfn);   /*e swap file name and/or path */
                            /*d Auslagerungsdateiname und/oder Pfad */

#ifdef __cplusplus
extern "C" int
#else
extern int _cdecl
#endif
exists (char *fname);

#define isslash(ch)  (ch == '\\' || ch == '/')

/* --------------------------------------------------------------------- */

/*>e Try '.COM', '.EXE', and '.BAT' on current filename, modify 
   filename if found. <*/
/*>d '.COM', '.EXE' und '.BAT' mit dem aktuellen Dateinamen versuchen,
   Dateinamen modifizieren wenn gefunden. <*/

static int tryext (char *fn)
{
   char *ext;

   ext = strchr (fn, '\0');
   strcpy (ext, ".COM");
   if (exists (fn))
      return 1;
   strcpy (ext, ".EXE");
   if (exists (fn))
      return 1;
   strcpy (ext, ".BAT");
   if (exists (fn))
      return 2;
   strcpy (ext, ".BTM");
   if (exists (fn))
      return 2;
    *ext = 0;
   return 0;
}

/*>e Try to find the file 'fn' in the current path. Modifies the filename
   accordingly. <*/
/*>d Versuchen die Datei 'fn' im aktuellen Pfad zu finden. Der Dateiname
   wird entsprechend modifiziert. <*/

static int findfile (char *fn)
{
   char *path, *penv;
   char *prfx;
   int found, check, hasext;

   if (!*fn)
      return (cmdpath [0]) ? 3 : ERR_COMSPEC;

   if (isslash (fn [0]) && isslash (fn [1]))
      {
      }
   check = checkpath (fn, INF_NODIR, drive, dir, name, ext, fn);
   if (check < 0)
      return check;

   if ((check & HAS_WILD) || !(check & HAS_FNAME))
      return ERR_FNAME;

   hasext = (check & HAS_EXT) ? ((!stricmp (ext, ".bat") || !stricmp (ext, ".btm")) ? 2 : 1) : 0;

   if (hasext)
      {
      if ( (check & FILE_EXISTS) &&
           ( !stricmp(ext, ".bat") ||
             !stricmp(ext, ".btm") ||
             !stricmp(ext, ".com") ||
             !stricmp(ext, ".exe") ) )
         found = hasext;
      else if ( (check & FILE_EXISTS) &&
                ( stricmp(ext, ".bat") &&
                  stricmp(ext, ".btm") &&
                  stricmp(ext, ".com") &&
                  stricmp(ext, ".exe") ) )
          return ERR_FNAME;
      else
         found = 0;
      }
   else
      found = tryext (fn);

   if (found || (check & (HAS_PATH | HAS_DRIVE)))
      return found;

   penv = getenv ("PATH");
   if (!penv)
      return 0;
   path = (char *)malloc (strlen (penv) + 1);
   if (path == NULL)
      return ERR_NOMEM;

   strcpy (path, penv);
   prfx = strtok (path, ";");

   while (!found && prfx != NULL)
      {
      while (isspace (*prfx))
         prfx++;
      if (*prfx)
         {
         strcpy (fn, prfx);
         prfx = strchr (fn, '\0');
         prfx--;
         if (*prfx != '\\' && *prfx != '/' && *prfx != ':')
            {
            *++prfx = '\\';
            }
         prfx++;
         strcpy (prfx, name);
         strcat (prfx, ext);
         check = checkpath (fn, INF_NODIR, drive, dir, name, ext, fn);
         if (check > 0 && (check & HAS_FNAME))
            {
            if (hasext)
               {
               if (check & FILE_EXISTS)
                  found = hasext;
               }
            else
               found = tryext (fn);
            }
         }
      prfx = strtok (NULL, ";");
      }
   free (path);
   return found;
}


/*>e 
   Get name and path of the command processor via the COMSPEC 
   environmnt variable. Any parameters after the program name
   are copied and inserted into the command line.
<*/
/*>d
   Namen und Pfad des Kommandoprozessors �ber die COMSPEC-Umgebungs-
   Variable bestimmen. Parameter nach dem Programmnamen werden kopiert
   und in die Kommandozeile eingef�gt.
<*/

static void getcmdpath (void)
{
   char *pcmd;
   int found = 0;

   if (cmdpath [0])
      return;
   pcmd = getenv ("COMSPEC");
   if (pcmd)
      {
      strcpy (cmdpath, pcmd);
      pcmd = cmdpath;
      while (isspace (*pcmd))
         pcmd++;
      if (NULL != (pcmd = strpbrk (pcmd, ";,=+/\"[]|<> \t")))
         {
         while (isspace (*pcmd))
            *pcmd++ = 0;
         if (strlen (pcmd) >= 79)
            pcmd [79] = 0;
         strcpy (cmdpars, pcmd);
         strcat (cmdpars, " ");
         }
      found = findfile (cmdpath);
      }
   if (!found)
      {
      cmdpars [0] = 0;
      strcpy (cmdpath, "COMMAND.COM");
      found = findfile (cmdpath);
      if (!found)
         cmdpath [0] = 0;
      }
}


/*>e
   tempdir: Set temporary file path.
            Read "TMP/TEMP" environment. If empty or invalid, clear path.
            If TEMP is drive or drive+backslash only, return TEMP.
            Otherwise check if given path is a valid directory.
            If so, add a backslash, else clear path.
<*/
/*>d
   tempdir: Pfad f�r tempor�re Datei setzen.
            Die Umgebungsvariable "TMP" oder "TEMP" wird gelesen. Ist
            keine der beiden vorhanden, oder sind sie ung�ltig, wird
            der Pfad gel�scht.
            Besteht TMP/TEMP nur aus Laufwerksbuchstaben, oder aus
            Laufwerk und Backslash, liefern TEMP.
            Sonst pr�fen ob der Pfad g�ltig ist, und einen Backslash
            anf�gen.
<*/

static int tempdir (char *outfn)
{
   int i, res;
   char *stmp [4];

   stmp [0] = getenv ("TMP");
   stmp [1] = getenv ("TEMP");
   stmp [2] = ".\\";
   stmp [3] = "\\";

   for (i = 0; i < 4; i++)
      if (stmp [i])
         {
         strcpy (outfn, stmp [i]);
         res = checkpath (outfn, 0, drive, dir, name, ext, outfn);
         if (res > 0 && (res & IS_DIR) && !(res & IS_READ_ONLY))
            return 1;
         }
   return 0;
}


#if (REDIRECT)

static int redirect (char *par, char **rstdin, char **rstdout, char **rstderr)
{
   char ch, sav;
   char *fn, *fnp, *beg;
   int app;

   do
      {
      app = 0;
      beg = par;
      ch = *par++;
      if (ch != '<')
         {
         if (*par == '&')
            {
            ch = '&';
            par++;
            }
         if (*par == '>')
            {
            app = 1;
            par++;
            }
         }

      while (isspace (*par))
         par++;
      fn = par;
      if ((fnp = strpbrk (par, ";,=+/\"[]|<> \t")) != NULL)
         par = fnp;
      else
         par = strchr (par, '\0');
      sav = *par;
      *par = 0;

      if (!strlen (fn))
         return 0;
      fnp = (char *)malloc (strlen (fn) + app + 1);
      if (fnp == NULL)
         return 0;
      if (app)
         {
         strcpy (fnp, ">");
         strcat (fnp, fn);
         }
      else
         strcpy (fnp, fn);

      switch (ch)
         {
         case '<':   if (*rstdin != NULL)
                        return 0;
                     *rstdin = fnp;
                     break;
         case '>':   if (*rstdout != NULL)
                        return 0;
                     *rstdout = fnp;
                     break;
         case '&':   if (*rstderr != NULL)
                        return 0;
                     *rstderr = fnp;
                     break;
         }

      *par = sav;
      strcpy (beg, par);
      par = strpbrk (beg, "<>");
      }
   while (par);

   return 1;
}

#endif


int do_exec (char *exfn, char *epars, int spwn, unsigned needed, char **envp)
{
   static char swapfn [MAXPATH];
   static char execfn [MAXPATH];
   unsigned avail;
   union REGS regs;
   unsigned envlen;
   int rc, ffrc;
   int idx;
   char **env;
   char *ep, *envptr, *envbuf;
   char *progpars;
   int swapping;
#if (REDIRECT)
   char *rstdin = NULL, *rstdout = NULL, *rstderr = NULL;
#endif

   envlen = 0;
   envptr = NULL;
   envbuf = NULL;

   if (epars == NULL)
      epars = "";
   if (exfn == NULL)
      execfn [0] = 0;
   else
      strcpy (execfn, exfn);

   getcmdpath ();

   /*e First, check if the file to execute exists. */
   /*d Zun�chst pr�fen ob die auszuf�hrende Datei existiert. */

   if ((ffrc = findfile (execfn)) <= 0)
      return RC_NOFILE | -ffrc;

   if (ffrc > 1)   /* COMMAND.COM or Batch file */
      {
      if (!cmdpath [0])
         return RC_NOFILE | -ERR_COMSPEC;

      idx = (ffrc == 2) ? strlen (execfn) + 5 : 1;
      progpars = (char *)malloc (strlen (epars) + strlen (cmdpars) + idx);
      if (progpars == NULL)
         return RC_NOFILE | -ERR_NOMEM;
      strcpy (progpars, cmdpars);
      if (ffrc == 2)
         {
         strcat (progpars, "/c ");
         strcat (progpars, execfn);
         strcat (progpars, " ");
         }
      strcat (progpars, epars);
      strcpy (execfn, cmdpath);
      }
   else
      {
      progpars = (char *)malloc (strlen (epars) + 1);
      if (progpars == NULL)
         return RC_NOFILE | -ERR_NOMEM;
      strcpy (progpars, epars);
      }

#if (REDIRECT)
   if ((ep = strpbrk (progpars, "<>")) != NULL)
      if (!redirect (ep, &rstdin, &rstdout, &rstderr))
         {
         rc = RC_REDIRERR;
         goto exit;
         }
#endif

   /*e Now create a copy of the environment if the user wants it. */
   /*d Nun eine Kopie der Umgebungsvariablen anlegen wenn angefordert. */

   if (envp != NULL)
      for (env = envp; *env != NULL; env++)
         envlen += strlen (*env) + 1;

   if (envlen)
      {
      /*e round up to paragraph, and alloc another paragraph leeway */
      /*d Auf Paragraphengrenze runden, plus einen Paragraphen zur Sicherheit */
      envlen = (envlen + 32) & 0xfff0;
      envbuf = (char *)malloc (envlen);
      if (envbuf == NULL)
         {
         rc = RC_ENVERR;
         goto exit;
         }

      /*e align to paragraph */
      /*d Auf Paragraphengrenze adjustieren */
      envptr = envbuf;
      if (FP_OFF (envptr) & 0x0f)
         envptr += 16 - (FP_OFF (envptr) & 0x0f);
      ep = envptr;

      for (env = envp; *env != NULL; env++)
         {
         ep = stpcpy (ep, *env) + 1;
         }
      *ep = 0;
      }

   if (!spwn)
      swapping = -1;
   else
      {
      /*e Determine amount of free memory */
      /*d Freien Speicherbereich feststellen */

      regs.x.ax = 0x4800;
      regs.x.bx = 0xffff;
      intdos (&regs, &regs);
      avail = regs.x.bx;

      /*e No swapping if available memory > needed */
      /*d Keine Auslagerung wenn freier Speicher > ben�tigter */

      if (needed < avail)
         swapping = 0;
      else
         {
         /*>e Swapping necessary, use 'TMP' or 'TEMP' environment variable
           to determine swap file path if defined. <*/
         /*>d Auslagerung notwendig, 'TMP' oder 'TEMP' Umgebungsvariable
            verwenden um Auslagerungsdateipfad festzulegen. <*/

         swapping = spwn;
         if (spwn & USE_FILE)
            {
            if (!tempdir (swapfn))
               {
               spwn &= ~USE_FILE;
               swapping = spwn;
               }
            else if (OS_MAJOR >= 3)
               swapping |= CREAT_TEMP;
            else
               {
               strcat (swapfn, SWAP_FILENAME);
               idx = strlen (swapfn) - 1;
               while (exists (swapfn))
                  {
                  if (swapfn [idx] == 'Z')
                     idx--;
                  if (swapfn [idx] == '.')
                     idx--;
                  swapfn [idx]++;
                  }
               }
            }
         }
      }

   /*e All set up, ready to go. */
   /*d Alles vorbereitet, jetzt kann's losgehen. */

   if (swapping > 0)
      {
      if (!envlen)
         swapping |= DONT_SWAP_ENV;

      rc = prep_swap (swapping, swapfn);
      if (rc < 0)
         rc = RC_PREPERR | -rc;
      else
         rc = 0;
      }
   else
      rc = 0;

   if (!rc)
      {
      if (spawn_check != NULL)
         rc = spawn_check (ffrc, swapping, execfn, progpars);
      if (!rc)
#if (REDIRECT)
         rc = do_spawn (swapping, execfn, progpars, envlen, envptr, rstdin, rstdout, rstderr);
#else
         rc = do_spawn (swapping, execfn, progpars, envlen, envptr);
#endif
      }

   /*e Free the environment buffer if it was allocated. */
   /*d Den Umgebungsvariablenblock freigeben falls er alloziert wurde. */

exit:
   free (progpars);
#if (REDIRECT)
   if (rstdin)
      free (rstdin);
   if (rstdout)
      free (rstdout);
   if (rstderr)
      free (rstderr);
#endif
   if (envlen)
      free (envbuf);

   return rc;
}

