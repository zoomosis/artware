#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <sys\stat.h>
#include <process.h>
#include <errno.h>
#include <alloc.h>
#include <stdlib.h>

#include <video.h>
#include <scrnutil.h>

#include <msgapi.h>
#ifndef __OS2__
#include <exec.h>
#endif

#include "message.h"
#include "wrap.h"
#include "tstruct.h"
#include "xmalloc.h"
#include "global.h"
#include "working.h"
#include "reply.h"
#include "copymsg.h"
#include "edit.h"

char *GetBody(int need_origin, char *origline, LINE *firstline, char *topline);

/* ----------------------------------------------- */

char *GetBody(int need_origin, char *origline, LINE *firstline, char *topline)

{
	char *msgbuf, *temp, *lastchar;
   static char commandline[120], msgfile[120];
	int lasthard=1, forcehard=0, editret, i, hadhcr=0;
	FILE *infile;
   struct stat before, after;
   unsigned cursize = 5000, new;
   char *fbuf=NULL, tmpmsg[120];
   unsigned len, totlen=0, no_of_lines=0;
   LINE *thisline, *oldline;
   int last_char_inited;

   if(strcmpi(cfg.usr.editor, "INTERNAL") != 0)
     {
     memset(commandline, '\0', sizeof(commandline));
     memset(msgfile, '\0', sizeof(msgfile));

     sprintf(msgfile, "%s\\timed.msg", cfg.homedir);
     stat(msgfile, &before);

  /*    __spawn_resident = (cfg.usr.swap_edit == 0) ? 0xFFFF : 0; */

     _setcursortype(_NORMALCURSOR);
     textattr(7);
     clrscr();
     MoveXY(1,3);

     #ifdef __OS2__
     if( (strcmpi(cfg.usr.editor + strlen(cfg.usr.editor) - 3, "cmd")== 0) ||
         (strcmpi(cfg.usr.editor + strlen(cfg.usr.editor) - 3, "btm")== 0) )
         {
         sprintf(commandline, "%s %s", cfg.usr.editor, msgfile);
         sprintf(tmpmsg, "þ Executing (batch) %s..", commandline);
         print(0,0,7,tmpmsg);
         savescreen();
         editret = system(commandline);
         }
     else
         {
         sprintf(tmpmsg, "þ Executing (direct) %s %s", cfg.usr.editor, msgfile);
         print(0,0,7,tmpmsg);
         savescreen();
         editret = spawnlp(P_WAIT, cfg.usr.editor, cfg.usr.editor, msgfile, NULL);
         }
     #else
     sprintf(tmpmsg, "þ Executing %s %s", cfg.usr.editor, msgfile);
     print(0,0,7,tmpmsg);
     savescreen();

     editret = do_exec(cfg.usr.editor, msgfile, USE_ALL|HIDE_FILE|CHECK_NET, cfg.usr.swap_edit ? 0xFFFF : 0x0000, environ);
     #endif

     _setcursortype(_NOCURSOR);
     putscreen();
     print(2,0,7,"þ Back in timEd..");

     #ifndef __OS2__
     if(editret & 0xFF00)
        {
        if( (editret & 0xFF00) == 0x0300 )
           {
           if (errno == ENOMEM)
              sprintf(tmpmsg, "Not enough memory");
           else sprintf(tmpmsg, "Error! Error code: %d ", editret);
           }
        else
           {
           if( (editret & 0xFF00) == 0x0100)
               sprintf(tmpmsg, "Error preparing for swap! Code: %d", editret);
           else if( (editret & 0xFF00) == 0x0200)
               sprintf(tmpmsg, "Invalid program name. Code: %d", editret);
           else if( (editret & 0xFF00) == 0x0400)
               sprintf(tmpmsg, "Error allocing env. buffer! Code: %d", editret);
           else if( (editret & 0xFF00) == 0x0500)
               sprintf(tmpmsg, "Error while swapping! Code: %d", editret);
           }



        sprintf(msg, "Error spawning the editor (%s)", tmpmsg);
        Message(msg, -1, 0, YES);
        return NULL;
        }

     #else
     if(editret == -1)
        {
        sprintf(tmpmsg, "Error spawning editor (%s)!", cfg.usr.editor);
        Message(tmpmsg, -1, 0, YES);
        return NULL;
        }
     #endif

     stat(msgfile, &after);

     #ifndef __OS2__
     if(before.st_atime == after.st_atime)
                        return NULL;
     #else
     if(before.st_mtime == after.st_mtime)
                        return NULL;
     #endif

	  if ( (infile = fopen(msgfile, "rt")) == NULL)
			  {
			  Message("Can't open input file!", -1, 0, YES);
           return NULL;
			  }

     fbuf = xmalloc(4096);
     msgbuf = xcalloc(1, cursize);
     temp = xcalloc(1, 1024);

     setvbuf(infile, fbuf, _IOFBF, 4096);

     print(4,0,7,"þ Reading message..");

	  while(fgets(temp, 1024, infile))
		  {
        if(!(no_of_lines++ % 25))
           {
           if(no_of_lines > 100)
              working(maxy-1,79,7);
           }

        len=strlen(temp);
  		  if ( (temp[len-1] == 0x0A) || (temp[len-1] == 0x0D) || (temp[len-1] == 0x8D) )
           {
           while(
                   (temp[len-1] == 0x0A) ||
                   (temp[len-1] == 0x0D) ||
                   (temp[len-1] == 0x8D)
                )
                 temp[--len]='\0';

           hadhcr=1;
           }
        else hadhcr=0;

        new = totlen + len + 10;
        if (new > cursize)
           {
           if ((msgbuf = realloc(msgbuf, (new + 2000))) == NULL)
                 Message("Out of memory!", -1, 254, NO);

           cursize = new + 2000;
           }

        if(!lasthard && (strchr(" -*.,", temp[0])) != NULL) /* Never 'pull' such a line at the end of prev. line */
           {
           if( (*(msgbuf+totlen-1) == ' ') && (totlen!=0) )   /* Strip useless space at end */
              *(msgbuf+totlen-1) = '\r';
           else
             {
             *(msgbuf+totlen)='\r';
             totlen++;
             }
           lasthard=1;
           }

        if( (temp[0] == '~') && (temp[1] == '~') )    /* Force hard returns for next section... */
          forcehard = forcehard ? 0 : 1;

		  else if (
                  IsQuote(temp) ||
                  (temp[len-1] == '~') ||
                  forcehard ||
                  (
                    ( (temp[0] == 'C') || (temp[1] == 'c') )
                    && (strncmpi(temp, "CC:",3)==0)
                  )
                )                       /* ~ forces HCR */
			  {
  		     if (temp[len-1] == '~')    /* Strip 'control' char that forces HCR */
              temp[--len]='\0';

           memcpy(msgbuf+totlen, temp, len);
           *(msgbuf+totlen+len) = '\r';

           totlen = totlen + len + 1;
			  lasthard = 1;
			  }

		  else if ( len < (maxx-20) )        /* short line, end w/ HCR */
			  {
			  if (len == 0 && !lasthard)			/* White line was intended.. */
              {
              if( (*(msgbuf+totlen-1) == ' ') && (totlen!=0) )   /* Strip useless space at end */
                 *(msgbuf+totlen-1) = '\r';
              else
                {
                *(msgbuf+totlen)='\r';
                totlen++;
                }
              }

           memcpy(msgbuf+totlen, temp, len);

           /* Strip useless ending space if added before.. */

           if (*(msgbuf+totlen+len-1) == ' ')
              {
              *(msgbuf+totlen+len-1) = '\r ';
              len--;
              }
           else *(msgbuf+totlen+len) = '\r';

           totlen = totlen + len + 1;
			  lasthard = 1;
			  }
		  else              /* add to paragraph, no HCR */
			  {
           memcpy(msgbuf+totlen, temp, len);

           /* Don't add space if it's a split long line, some editors */
           /* don't end all lines with cr/lf                          */

           if (hadhcr && (temp[len-1] != ' '))
              {
              *(msgbuf+totlen+len) = ' ';
              totlen++;
              }

           totlen += len;
			  lasthard = 0;
			  }
		  }

	  fclose(infile);
     free(fbuf);
     free(temp);

	  unlink(msgfile);

     if (totlen < 2)
        {
        if (msgbuf) free(msgbuf);
        return NULL;
        }

     *(msgbuf+totlen) = '\0';
   }

   else  /* ================= Internal Editor ================= */

   {
   if( (firstline = edittext(firstline, topline)) == NULL)
      return NULL;

   _setcursortype(_NOCURSOR);

   last_char_inited = 0;

   msgbuf = lastchar = xcalloc(1, cursize);
   while(firstline)
     {
     if( (totlen + firstline->len + 10) > cursize) /* Block big enough? */
        {
        if ((msgbuf = realloc(msgbuf, (cursize + 2000))) == NULL)
              Message("Out of memory!", -1, 254, NO);

        cursize += 2000;
        lastchar = msgbuf + totlen - 1;
        }

     /* Do we have to add a space (if we're appending two lines) */
     if( (!lasthard) &&
         (*lastchar != ' ') &&
         (firstline->ls) &&
         (*lastchar != '\0') &&
         (firstline->ls[0] != '\0') &&
         (firstline->ls[0] != ' ') )
        {
        *(++lastchar) = ' ';
        totlen++;
        }

     if(firstline->len)
        {
        memmove(lastchar+1, firstline->ls, firstline->len);
        totlen += firstline->len;
        }

     if(last_char_inited)
        lastchar += firstline->len;
     else
       {
       last_char_inited = 1;
       memmove(msgbuf, msgbuf+1, firstline->len); /* Set buff back 1 char */
       lastchar = msgbuf+firstline->len-1;
       if(lastchar < msgbuf)
          {
          lastchar = msgbuf;
          if(*lastchar == '\0')
             {
             *lastchar = '\r';
             totlen++;
             }
          }
       }

     if(firstline->status & HCR)
       {
       if(totlen)
          *(++lastchar) = '\r';
       else
          *lastchar='\r';
       totlen++;
       lasthard = 1;
       }
     else lasthard = 0;

     oldline = firstline;
     firstline = firstline->next;
     if(oldline->ls) free(oldline->ls);
     free(oldline);
     }

   *(++lastchar) = '\0';

   }

   clean_end_message(msgbuf); /* Strips CR's, PATH, whatever from end of text */
   if( need_origin && clean_origin(msgbuf) )
      {
      Message("Warning! No (valid) Origin line detected, adding one..", -1, 0, YES);
      if( (strlen(msgbuf) + strlen(origline) + 3) > sizeof(msgbuf) )
        {
        if( (msgbuf = realloc(msgbuf, (strlen(msgbuf)+strlen(origline)+1))) == NULL)
           Message("Out of memory!", -1, 254, NO);
        }

      for(i=0; i < strlen(origline); i++) /* Replace \n with \r.. */
         {
         if(origline[i] == '\n')
            origline[i] = '\r';
         }
      strcat(msgbuf, origline);
      }

	return msgbuf;
}
