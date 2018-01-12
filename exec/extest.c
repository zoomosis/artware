/*
   --- Version 3.3 91-11-26 15:54 ---

   EXTEST.C: EXEC function with memory swap - Test program.

   Public domain software by

        Thomas Wagner
        Ferrari electronic GmbH
        Beusselstrasse 27
        D-1000 Berlin 21
        Germany
*/


#include "compat.h"
#include "exec.h"

/*>e*/
#define ENGLISH 1
/*<*/

#ifdef ENGLISH
int help (void)
{
   puts ("Usage: EXTEST [options]");
   puts ("Options:");
   puts ("     -E    do not use EMS  for swapping");
   puts ("     -X    do not use XMS  for swapping");
   puts ("     -F    do not use File for swapping");
   puts ("     -T    Terminate after EXEC (no swap)");
   puts ("     -N    do Not swap");
   puts ("     -C    do not Copy environment");
   puts ("     -H    Hide swapfile");
   puts ("     -Z    Try XMS first, then EMS");

   return 1;
}
#else
int help (void)
{
   puts ("Benutzung: EXTEST [Optionen]");
   puts ("Optionen:");
   puts ("     -E    bei Auslagern kein EMS benutzen");
   puts ("     -X    bei Auslagern kein XMS benutzen");
   puts ("     -F    bei Auslagern keine Datei benutzen");
   puts ("     -T    Terminieren nach EXEC (keine Auslagerung)");
   puts ("     -N    Nicht Auslagern");
   puts ("     -C    Umgebungsvariablen nicht kopieren");
   puts ("     -H    Auslagerungsdatei 'hidden'");
   puts ("     -Z    Zuerst XMS versuchen, dann EMS");

   return 1;
}
#endif

#ifdef ENGLISH
int spcheck (int cmdbat, int swapping, char *execfn, char *progpars)
{
   if (swapping > 0)
      {
      switch (swap_prep.swapmethod)
         {
         case USE_EMS:
               printf ("Swapping to EMS handle %d\n", swap_prep.handle);
               break;

         case USE_XMS:
               printf ("Swapping to XMS handle %d\n", swap_prep.handle);
               break;

         case USE_FILE:
               printf ("Swapping to File <%s>\n", swap_prep.swapfilename);
               break;
         }
      }
   printf ("Executing %s %s\n\n", execfn, progpars);
   if (cmdbat == 3 && strstr (progpars, "/c") == NULL)
      puts ("Enter EXIT to return to EXTEST");

   return 0;
}
#else
int spcheck (int cmdbat, int swapping, char *execfn, char *progpars)
{
   if (swapping > 0)
      {
      switch (swap_prep.swapmethod)
         {
         case USE_EMS:
               printf ("Auslagerung auf EMS Handle %d\n", swap_prep.handle);
               break;

         case USE_XMS:
               printf ("Auslagerung auf XMS Handle %d\n", swap_prep.handle);
               break;

         case USE_FILE:
               printf ("Auslagerung auf Datei <%s>\n", swap_prep.swapfilename);
               break;
         }
      }
   printf ("AusfÅhren %s %s\n\n", execfn, progpars);
   if (cmdbat == 3 && strstr (progpars, "/c") == NULL)
      puts ("Geben Sie EXIT ein um zu EXTEST zurÅckzukehren");

   return 0;
}
#endif


int main (int argc, char **argv)
{
   static char fn [255], par [130];
   char *com;
   int i;
   char far *x;
   char far *y;
   int method;
   unsigned needed;
   char **env;
   union REGS regs;

   method = USE_ALL;
   needed = 0xffff;
   env = environ;
   spawn_check = spcheck;

   for (i = 1; i < argc; i++)
      {
      if (argv [i][0] != '-' && argv [i][0] != '/')
         return help ();
      com = &argv [i][1];
      do
         {
         switch (toupper (*com))
            {
            case 'E':   method &= ~USE_EMS; break;
            case 'X':   method &= ~USE_XMS; break;
            case 'F':   method &= ~USE_FILE; break;
            case 'T':   method = 0; break;
            case 'N':   needed = 0; break;
            case 'C':   env = NULL; break;
            case 'H':   method |= HIDE_FILE; break;
            case 'Z':   method |= XMS_FIRST; break;
            case '-':   break;
            case '/':   break;
            default:    return help ();
            }
         com++;
         } while (*com);
      }

   x = (char far *)farmalloc (64L * 1024L);
   y = (char far *)farmalloc (64L * 1024L);

#ifdef ENGLISH
   putenv ("XYZ=This is an environment string for the spawned process");
#else
   putenv ("XYZ=Dies ist eine Umgebungsvariable fÅr den aufgerufenen Proze·");
#endif

   while (1)
      {
#ifdef ENGLISH
      printf ("\nEXEC filename,params ('.' to exit): ");
#else
      printf ("\nDateiname,Parameter ('.' beendet): ");
#endif
      gets (fn);

      if (fn [0] == '.')
         return 0;

      com = strchr (fn, ',');
      if (com != NULL)
         {
         strcpy (par, &com [1]);
         *com = 0;
         }
      else
         par [0] = 0;

      i = do_exec (fn, par, method, needed, env);

#ifdef ENGLISH
      printf ("\nDO_EXEC returns %04x\n", i);
#else
      printf ("\nDO_EXEC liefert %04x\n", i);
#endif
      }
}

