#include "includes.h"

void  DoWrite(MIS *mis, MSG *areahandle, AREA *area);
int   CopyFile(char *filename, char *tofilename, int delete);

int ok = 1;

/* -------------------------------------------------- */


#ifdef __WATCOMC__
#define MAXPATH PATH_MAX
#endif

int check_attach(MIS *mis, char *list, int copyfiles)
{
   char  *file, filename[MAXPATH], temp[80];
   char **master = NULL;
   char **thisone;
   struct stat mystat;
   int top = 10, now = 0, i;
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   ok = 1;

   while(*list == ' ') list++;   // Skip leading space.

   if(list[0] == '\0')
      strcpy(list, "*.*");

   file = strtok(list, " ");

   while(file && ok)         /* Do all listed filespecs on subj line */
     {
     thisone = NULL;
     strcpy(filename, file);

     if(file[0] == '\0')
        strcpy(filename, "*.*");

     if( (strchr(filename, '*') != NULL) ||
         (strchr(filename, '?') != NULL) )     /* Wildcard */
         {
         if( (thisone = dirlist(filename, 1)) == NULL)
            {
            if(master)
               free_filelist(master);
            return 0; // Not OK!
            }
         }
     else     // No explicit wildcard, but may be directory name..
         {
         Strip_Trailing(filename, '\\');
         if(stat(filename, &mystat) == -1)
            {
            Message("Cannot find file!", -1, 0, YES);
            return 0; // Not OK!
            }

         if(mystat.st_mode & S_IFDIR)          // It's a directory!
            {
            strcat(filename, "\\*.*");
            if( (thisone = dirlist(filename, 1)) == NULL)
              {
              if(master)
                free_filelist(master);
              return 0;  // Not OK!
              }
            }
         else              // Expand full filename.
           {
           strcpy(temp, filename);
           if(strncmpi(temp, "\\\\", 2) != 0)
              _fullpath(filename, temp, MAXPATH-1);
           }
         }


     if(!master)
       {
       if(!thisone)     // No master, no list now. Make one...
          {
          master = mem_calloc(top, sizeof(char *));
          master[now++] = mem_strdup(filename);
          }
       else
          {
          master = thisone;   // Make thisone master
          thisone = NULL;     // Make NULL, don't mem_free() just yet!

          while(master[now] != NULL)  // Set current pointers right.
             now++;
          top = now;
          }
       }
     else   // We have a master list, we must add...
       {
       if(!thisone)        // We only add one file..
         {
         if(now > (top-2))
            {
            top += 10;
            master = mem_realloc(master, top * sizeof(char *));
            if(master == NULL)
               return 0;
            }
         master[now++] = mem_strdup(filename);
         master[now]   = NULL;
         }
       else // Add multiple files
         {
         for(i=0; thisone[i] != NULL; i++)
            {
            if(now > (top-2))
               {
               top += 10;
               master = mem_realloc(master, top * sizeof(char *));
               if(master == NULL)
                  return 0;
               }
            master[now++] = mem_strdup(thisone[i]);
            master[now] = NULL;  // Zap out next el to indicate end of list. Realloc messed it up!
            }
         free_filelist(thisone);
         }
       }

     file = strtok(NULL, " ");
     }

     if(ok)
       {
       for(i=0; master[i] != NULL; i++)
          {
          if(copyfiles && cfg.usr.localfiles[0] != '\0')
            {
            fnsplit(master[i], NULL, NULL, fname, ext);
            sprintf(filename, "%s\\%s%s", cfg.usr.localfiles, fname, ext);

            sprintf(temp, " þ Copying %s to %s..", master[i], filename);
            printeol(maxy-1,0,cfg.col[Cmsgbar],temp);

            if(CopyFile(master[i], filename, 0) != 0)
              {
              sprintf(msg, "Error copying %s to %s!", master[i], filename);
              Message(msg, -1, 0, YES);
              }
            else
             mis->attached = AddToStringList(mis->attached, filename, NULL, 1);
            }
          else
             mis->attached = AddToStringList(mis->attached, master[i], NULL, 1);
          }
       }

     free_filelist(master);

     return ok;
}


// =======================================================================

void writefa(MIS *mis, MSG *areahandle, AREA *area, int exceptfirst)
{
   STRINGLIST **files, *tmplist=NULL, *temp;

   // JAM can handle as many files as you want, so this is the always first..
   if(area->base & MSGTYPE_JAM)
     {
     if(exceptfirst)
       return;
     else
       {
       DoWrite(mis, areahandle, area);
       return;
       }
     }

   // Otherwise we are not handling JAM, so we need to split if the subject
   // gets too long..

   if(mis->attached)
     files = &mis->attached;
   else
     files = &mis->requested;

   while( ((StrListLen(*files) > 71) && exceptfirst) ||
           (*files && !exceptfirst) )
     {
     while( *files &&
            ((StrListLen(tmplist) + StrListElLen(*files)) < 71)  // -1 for space!
          )
       {
       tmplist = AddToStringList(tmplist, (*files)->s, (*files)->pw, 0);
       temp = *files;
       *files = (*files)->next;
       temp->next = NULL;
       FreeStringList(temp);
       }

     temp    = *files;
     *files  = tmplist;
     tmplist = NULL;
     DoWrite(mis, areahandle, area);
     FreeStringList(*files);
     *files  = temp;
     }

}

// ==============================================================

void DoWrite(MIS *mis, MSG *areahandle, AREA *area)
{
   char *kludges;

   kludges = MakeKludge(NULL, mis, 1);       /* Last==1, always netmail */

   SaveMessage(areahandle, area, mis, NULL, kludges, 0L, 0);

   mem_free(kludges);

   add_tosslog(area, areahandle);

}

// ==============================================================

int CopyFile(char *filename, char *tofilename, int delete)
{
   char *buf;
   int  got,
        in,
        out;

   if(fexist(tofilename))
     {
     sprintf(msg, "%s exists already. Overwrite? (y/n)", tofilename);
     if(confirm(msg) == 0)
       return -1;

     if(unlink(tofilename) != 0)
        {
        Message("Error overwriting file!", -1, 0, YES);
        return -1;
        }
     }

   if(delete)     // It's a move, try fast rename()
     {
     if(rename(filename, tofilename) == 0)      // It works!
       return 0;                          // So that's all.
     }

   if( (in=sopen(filename, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
     {
     sprintf(msg, "Can't open %s for reading. (Errno %d)", filename, errno);
     Message(msg, -1, 0, YES);
     return -1;
     }

   if((out=sopen(tofilename, O_BINARY | O_CREAT | O_WRONLY, SH_DENYRW, S_IREAD | S_IWRITE)) == -1)
     {
     sprintf(msg, "Can't open %s for writing. (Errno %d)", tofilename, errno);
     Message(msg, -1, 0, YES);
     return -1;
     }

   buf = mem_calloc(1, 16384);

   while( (got=read(in, buf, (unsigned) 16384)) > (int) 0)
     {
     if(write(out, buf, (unsigned) got) != (int) got)
       {
       sprintf(msg, "Error writing to %s.", tofilename);
       Message(msg, -1, 0, YES);
       got = -1;
       break;
       }
     }

   if(close(in) != 0 || close(out) != 0)
     {
     Message("Error closing file after copy.", -1, 0, YES);
     got = -1;
     }

   free(buf);

   if(delete & (got != -1))
     unlink(filename);

   return (got == -1) ? -1 : 0;

}

// ======================================================================


