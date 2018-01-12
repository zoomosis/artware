#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "prog.h"

#include "msgapi.h"

// ==========================================================

void Extract_Attaches(MIS *mis)
{
  char temp[120];
  char *p;

  strcpy(temp, mis->subj);
  p=strtok(temp, " \x09");
  while(p)
    {
    mis->attached = AddToStringList(mis->attached, p, NULL, 1);
    p=strtok(NULL, " \x09");
    }
  memset(mis->subj, '\0', sizeof(mis->subj));
}


// ==========================================================

void Extract_Requests(MIS *mis)
{
  char temp[120];
  char *f, *p, *pw;

  strcpy(temp, mis->subj);
  f=strtok(temp, " \x09");
  while(f)
    {
    pw = NULL;
    p=strtok(NULL, " \x09");
    if(mi.nospace == 0)  // Space between file and password.
      {
      if(p && *p == '!')          // Password!
        {
        pw = p+1;
        p = strtok(NULL, " \x09");
        }
      }
    else // No space between filename & password
      {
      if( (pw = strrchr(f, '!')) != NULL )   // password found!
        {
        *pw = '\0';
        pw++;
        }
      }

    mis->requested = AddToStringList(mis->requested, f, pw, 1);
    f = p;
    }

  memset(mis->subj, '\0', sizeof(mis->subj));
}

// ==========================================================


void Files2Subject(MIS *mis, char *outsubj)
{
  STRINGLIST *current;
  char temp[72];
  int notfirst = 0;

  if(mis->attached)
    {
    memset(outsubj, '\0', 72);
    for(current=mis->attached; current; current=current->next)
      {
      if(strlen(outsubj))
         notfirst = 1;

      if((strlen(outsubj) + strlen(current->s) + notfirst) < 72)
         {
         if(notfirst)    // First addition?
            strcat(outsubj, " ");
         strcat(outsubj, current->s);
         }
      }
    }
  else if(mis->requested)
    {
    memset(outsubj, '\0', 72);
    for(current=mis->requested; current; current=current->next)
      {
      temp[0]='\0';
      if(current->pw)
        {
        if(mi.nospace)
          sprintf(temp, "!%-0.69s", current->pw);  // Password
        else
          sprintf(temp, " !%-0.69s", current->pw);  // Passwora
        }

      if(strlen(outsubj))
         notfirst = 1;

      if((strlen(outsubj) + strlen(current->s) + strlen(temp) + notfirst) < 72)
         {
         if(notfirst)    // First addition?
            strcat(outsubj, " ");
         strcat(outsubj, current->s);
         strcat(outsubj, temp);          // Password (can be "")
         } // if

      } // for

    } //else

}

// ==========================================================

// ------------------------------------------------------------------
// Returns length of trailing stuff..

int AnalyseTrail(char *s, unsigned len, MIS *mis)
{
   char *end, *charptr, *origptr, *nowlast;
   int stophere = 0, foundany = 0;

   // First check to see if there's a NULL in the text. This can happen
   // and indicates the message doesn't occupy the entire msglength
   // (Maximus' internal QWK for example).
   // If there's no NULL in the txt, we end at the trailing '\0' after
   // the buffer..

   end = strchr(s, '\0');  // Remember end.

   if(end == s) return 0;    // Not much, really :-)

   charptr = end-1;

   // Now search back to find beginning of lines. And then check for SB etc.

   origptr = NULL;

   while(!stophere)
     {
     charptr = strrchr(s, '\r');
     if(charptr == NULL)
        {
        // It could be there is no '\r' anymore. Then this could be the
        // first (or first and only) line in the message. And that _can_ be
        // a VIA or SEENBY etc. if there is no 'normal' text at all in the
        // message! So don't just stop here! Check the first line then!

        if(origptr && origptr == s)    // We already were here.. Stop now.
          break;
        else
          charptr = s;
        }

     origptr = charptr;

     // Strip \r, \n and LF's before this.
     while(
             (charptr >= s) &&
             (*charptr=='\r' || *charptr=='\n' || *charptr == 0x8D)
          )

        {
        *charptr = '\0';
        if(charptr>s) charptr--;
        }

     // Position 'nowlast' at the trailing '\0', just after the text

     if(*charptr == '\0' || *charptr == '\x01') // Probably only if we hit the start of buffer (charptr == s)
       nowlast = charptr;
     else
       nowlast = charptr+1;

     charptr = origptr;
     *charptr++ = '\0';   // NULL out original '\r'.
 
     // Skip \r, \n, 's and LF's after..
     while(*charptr=='\r' || *charptr=='\n' || *charptr == 0x8D || *charptr == 0x01)
        charptr++;

     // We are now at the beginning of a line, after skipping junk

     if(strlen(charptr) == 0)   // Empty line, move on..
        {
        end = nowlast;
        continue;
        }

     switch(*charptr)
       {
       case 'S':

         if(strnicmp(charptr, "SEEN-BY: ", 9) == 0)
           {
           mis->seenby = AddToStringList(mis->seenby, charptr+9, NULL, 1);
           foundany = 1;
           end = nowlast;
           }
         else  // No SEEN-BY? Stop!
           stophere = 1;
         break;

       case 'P':
         if(strnicmp(charptr, "PATH: ", 6) == 0)
           {
           mis->path = AddToStringList(mis->path, charptr+6, NULL, 1);
           foundany = 1;
           end = nowlast;
           }
         else
           stophere = 1;
         break;

       case 'V':
         if(strnicmp(charptr, "VIA ", 4) == 0)
           {
           mis->via = AddToStringList(mis->via, charptr+4, NULL, 1);
           foundany = 1;
           end = nowlast;
           }
         else
           stophere = 1;
         break;

       default:       // No VIA, SEEN-BY, PATH. Let's stop now..
         stophere = 1;
         break;
       }  // switch
     }  // while

   // Now, if we found something, we also need to calc the size of the
   // trailing stuff, so we can ignore it when reading the message

   if(foundany)
     return (len - (end - s));
   else
     return 0;

}

// ------------------------------------------------------------------

int AddToString(char **start, dword *curmax, dword *cursize, char *string)
{
   dword len;
   char *new;

   len = strlen(string);

   if( (*cursize + len + 1) > *curmax)
     {
     *curmax += (len+512);
     new = realloc(*start, *curmax);
     if(!new)
       {
       *curmax -= (len+512);
       return -1;
       }
     *start = new;
     }

   memcpy(*start+*cursize, string, len+1);
   *cursize += len;

   return 0;
}

