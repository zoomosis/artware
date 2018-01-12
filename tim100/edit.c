#include <os2comp.h>

#include <stdlib.h>
#include <conio.h>
#ifndef __OS2__
        #include <bios.h>
#endif
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <dir.h>

#include <msgapi.h>

#include "wrap.h"
#include "tstruct.h"

#include <video.h>
#include <scrnutil.h>
#include "xmalloc.h"
#include <alloc.h>

#include "idlekey.h"
#include "input.h"
#include "xfile.h"
#include "message.h"
#include "readarea.h"
#include "global.h"
#include "attach.h"  /* For FILELIST struct */

#define legalset "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890~`!@#$%^&*()-_=+[]{}\\|'\";:,./?>< "

#define VFN "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:\\._|=+-)(&^%$#@![]{}<>~`0123456789?*"

//#define HILITE      15
//#define NORMTEXT    7
//#define HCRCOLOUR   8
//#define QUOTECOL    14
//#define BLOCKCOL    112
//#define BLOCKCURCOL 113

#define TABSIZE 3

int pascal IsQuote(char far *line);

static int pos=0, row=1; /* pos == position on line (zero based */
                         /* row == row on screen, start with 1 (status bar) */

static LINE  *first   = NULL,   /* first line of the text    */
             *start   = NULL,   /* first line on screen      */
             *current = NULL,   /* current line being edited */
             *bbegin  = NULL,   /* Start of 'block'          */
             *bend    = NULL;   /* End of 'block'            */

static char templine[150];     /* 1 line buffer to manipulate throughout */

int ret, insert, curlen, l, c;
unsigned line;
char *charptr, temp[100];
LINE *lineptr, *lptr;
int changestart;

#define CLIPLEN 30

static char *clip[CLIPLEN];

void  update_screen (int entire         );
LINE  *edittext     (LINE *begin, char *topline);
void  add_n_wrap    (char c, char insert);
LINE  *insert_after (void               );
void  leave_line    (int repaint        );
void  enter_line    (void               );

void  press_enter    (void);
void  erase_line     (void);
void  move_left      (void);
void  move_right     (void);
void  delchar        (void);
void  move_up        (void);
void  move_down      (void);
void  page_up        (void);
void  page_down      (void);
void  word_left      (void);
void  word_right     (void);
void  backspace      (void);

void  import_file     (void);
void  top_of_screen   (void);
void  bottom_of_screen(void);
void  top_of_text     (void);
void  bottom_of_text  (void);
void  del_word_right  (void);
void  del_word_left   (void);
void  unerase         (void);
void  tab             (void);
void  copy_line       (void);

void clear_unerase(void);

void block_border (void);
void unmark       (void);
int  del_block    (void);
int  copy_block   (void);
void move_block   (void);

char *getfile(char *filespec);
char *showget(FILELIST *first);

int get_quote_string(char *line, char *quote);

/* -------------------------------- */


/* void main(void)

{
   LINE *this;

   video_init();

   this = xcalloc(1, sizeof(LINE));
   this->ls = xstrdup("Dit is even een test om te kijken of er nog leven in de");
   this->len = strlen(this->ls);
   edittext(this);
}
*/




/* ------------------------------------------------------- */

LINE *edittext(LINE *begin, char *topline)

{
   LINE *curline, *nextline;

   /* init values.. */

    ret=0;
    insert=1;
    curlen=0;
    c = 0;
    line=1;

    memset(&clip, '\0', sizeof(clip));

    if(begin == NULL) /* No text given, make first line ourselves */
       {
       begin = xcalloc(1, sizeof(LINE));
       lineptr->ls = xstrdup("");
       lineptr->status |= HCR;
       }

    first = current = start = begin;
    bbegin = bend = NULL;
    pos = 0;
    row = 1; /* Allow top line for statusbar */

	 _setcursortype(_NORMALCURSOR);
    clsw(cfg.col.msgtext);
    printeol(0,0,cfg.col.msgbar,topline);
    sprintf(temp, "Pos: %-3d Line: %-4d                                          %s  ", pos, line, insert ? "Insert  " : "TypeOver");
    printeol(maxy-1,0,cfg.col.msgbar,temp);

    update_screen(1);

    enter_line();

    printeol(row, 0, cfg.col.editcurnormal, templine);
    if( (current->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
       {
       if(current->len < maxx)
           printc(row, current->len, cfg.col.edithcr, 20);
       }

    MoveXY(pos+1, row+1);

	 while (1)
		  {
//        if(heapcheck() == -1)
//           {
//           beep(); beep();
//           print(maxy-1, 0, 7, "====>> Heap Corrupt! <<====");
//           get_idle_key(1);
//           get_idle_key(1);
//           }

		  c = get_idle_key(1);

		  switch(c) {
             case 13:              /* Enter */

               press_enter();
               break;


             case 20:              /* CTRL - T */
               del_word_right();
               break;

             case 21:              /* CTRL - U */

               unerase();
               break;

             case 25:              /* CTRL - Y */

               erase_line();
               break;


				 case 27:               /* ESC */
               if(cfg.usr.status & CONFIRMEDITEXIT)
                 {
                 _setcursortype(_NOCURSOR);
                 if(!confirm("Do you really want to dump this message? [y/N]"))
                   {
                   _setcursortype(_NORMALCURSOR);
                   break;
                   }
                 }

               curline = first;          /* Free all lines */
               while(curline)
                  {
                  nextline = curline->next;
                  if(curline->ls)
                     free(curline->ls);
                  free(curline);
                  curline=nextline;
                  }
               clear_unerase();
               _setcursortype(_NOCURSOR);
					return NULL;          /* return nothing */

             case 127:             /* CTRL-Backspace */
                 del_word_left();
                 break;

             case 279:             /* ALT-I */

                 import_file();
                 update_screen(1);
                 _setcursortype(_NORMALCURSOR);
                 break;

             case 26:               /* CTRL-Z  */
             case 287:              /* ALT - S */
                 leave_line(0);
                 _setcursortype(_NOCURSOR);
                 clear_unerase();
                 return first;

             case 291:              /* ALT-H */

                  if(cfg.usr.status & SHOWEDITHCR)
                     cfg.usr.status &= ~SHOWEDITHCR;
                  else
                     cfg.usr.status |= SHOWEDITHCR;
                  update_screen(1);
                  break;

             case 292:              /* ALT-J */
                  shell_to_DOS();
                  _setcursortype(_NORMALCURSOR);
                  break;

             case 315:    /* F1 */

                show_help(5);
                break;

             case 318:    /* F4 */
                 copy_line();
                 break;

             case 294:              /* ALT-L */
                 block_border();
                 break;

             case 278:              /* ALT-U */
                 unmark();
                 break;

             case 302:              /* ALT-C */
                 copy_block();
                 break;

             case 306:              /* ALT-M */
                 move_block();
                 break;

             case 288:              /* ALT-D */
                 del_block();
                 break;

				 case 331:              /* left */

                 move_left();
                 break;

				 case 333:        /* right */
                 move_right();
					  break;

				 case 335:        /* end */
					  pos = strlen(templine);
					  break;

				 case 327:        /* home */
					  pos = 0;
					  break;

             case 338:        /* insert */

                  insert = insert ? 0 : 1;
                  break;

             case 388:        /* ctrl page-up */
                  top_of_screen();
                  break;

             case 374:        /* ctrl page-dn */
                  bottom_of_screen();
                  break;

             case 375:        /* ctrl home */
                  top_of_text();
                  break;

             case 373:        /* ctrl end */
                  bottom_of_text();
                  break;

				 case 339:        /* del */

                 delchar();
					  break;

             case 328:        /* up */
                 move_up();
                 break;

             case 336:        /* down */
                 move_down();
                 break;

             case 329:        /* Page Up */
                 page_up();
                 break;

             case 337:        /* Page Down */
                 page_down();
                 break;

             case 271:        /* TAB left */
                 break;

             case 371:        /* ctrl - left */
                  word_left();
                  break;

             case 372:        /* ctrl - right */
                  word_right();
                  break;

             case 403:       /* CTRL - del */
                  memset(templine+pos, '\0', strlen(templine) - pos);
                  break;

				case 8:                 /* backspace */
                backspace();
                break;

            case 9:                 /* TAB */
                tab();
                break;

            default:
                if (c>31 && c<256)  /* A legal char */
                  {
						if ( pos < (maxx-1) )    /* so a character can be added.. */
                     {
                     if (!insert)
							   templine[pos++] = c;
                     else        /* insert */
                         {
                         if(strlen(templine) < (maxx-1) )
                            {
                            templine[strlen(templine)] = '\0';  /* Just be sure! */
                            memmove(templine+pos+1, templine+pos, strlen(templine) - pos);
                            templine[pos++] = c;
                            }
                         else /* string too long, wrap to next line */
                            {
                            add_n_wrap(c, 1);
                            update_screen(0);
                            }
                         }       /* else (=insert) */
                     }   /* pos < 80 */

                  else   /* no character can be added, goto newline */
                     {
                     if (!insert)
                        {
                        add_n_wrap(c, 0);
                        update_screen(0);
                        }  /* !insert */

                     else  /* insert */
                        {
                        add_n_wrap(c, 1);
                        update_screen(0);
                        } /* else (=insert) */

                     }  /* else, no char can be added */

						}

		  } /* end switch */


        if(current->status & HIGHLIGHT)
           printeol(row, 0, cfg.col.editcurblock, templine);
	     else if(IsQuote(templine))
           printeol(row, 0, cfg.col.editcurquote, templine);
        else
           printeol(row, 0, cfg.col.editcurnormal, templine);

        if( (current->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
           {
           if(strlen(templine) < maxx)
               printc(row, strlen(templine), cfg.col.edithcr, 20);
           }

        sprintf(temp, "Pos: %-3d Line: %-4d                                          %s ", pos, line, insert ? "Insert  " : "TypeOver");
        printeol(maxy-1,0,cfg.col.msgbar,temp);
        MoveXY(pos+1, row+1);

		}   /* end while */

}


/* -------------------------------------------------------- */


void update_screen(int entire)

{
   LINE *thisone;
   char lines = 0;
   char temp[133];
   int  reached_current = 0;

   while (row > (maxy-2) )
         {
         start = start->next;
         row--;
         entire = 1;
         }

   while (row < 1)
         {
         start = start->prev;
         row++;
         entire = 1;
         }

   for(thisone=start; thisone && lines < (maxy-2); thisone = thisone->next, lines++)
      {
      if(thisone == current)
          {
          reached_current = 1;
          if(thisone->status & HIGHLIGHT)
             printeoln(lines+1, 0, cfg.col.editcurblock, thisone->ls, thisone->len);
          else if(thisone->status & QUOTE)
             printeoln(lines+1, 0, cfg.col.editcurquote, thisone->ls, thisone->len);
          else
             printeoln(lines+1, 0, cfg.col.editcurnormal, thisone->ls, thisone->len);
          }
      else if(reached_current || entire || (thisone->next && (thisone->next == current)) )
          {
          if(thisone->status & HIGHLIGHT)
             printeoln(lines+1, 0, cfg.col.editblock, thisone->ls, thisone->len);
          else if(thisone->status & QUOTE)
             printeoln(lines+1, 0, cfg.col.msgquote, thisone->ls, thisone->len);
          else
             printeoln(lines+1, 0, cfg.col.msgtext, thisone->ls, thisone->len);

          if( (thisone->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
             {
             if(thisone->len < maxx)
                 printc(lines+1, thisone->len, cfg.col.edithcr, 20);
             }
          }

      }

   if(lines < (maxy-2))
      ClsRectWith(1+lines,0,maxy-2,maxx-1,cfg.col.msgtext,' ');

}


/* -------------------------------------------------------- */


void add_n_wrap(char c, char insert)

{
   int bufsize, cursize=2000;
   LINE *lptr = NULL, *prev = NULL, *next = NULL;
   int check, len, buffilled;
   char *buffer, *tmpbuf, *curend=NULL;
   char quotestring[20], *quotedline;
   int quote = 0, unmarked=0;

   if(current->status & HIGHLIGHT)
      {
      unmarked=1;
      unmark();  // Shut off block, will be messed up anyway
      }

   if (current == first)   /* this won't last ;-) */
      first = NULL;

   if (current == start)
      start = NULL;

   leave_line(0);

   buffer = xcalloc(1, cursize);

   strcpy(buffer, current->ls);

   /* insert the character */

   if (c)
      {
      if (insert)
         memmove(buffer+pos+1, buffer+pos, strlen(buffer) - pos + 1);
      buffer[pos++] = c;
      }

   if((current->status & QUOTE) && (get_quote_string(current->ls, quotestring)!=0))
      {
      quote = 1;
      strcpy(buffer, buffer + strlen(quotestring));
      }

   bufsize = buffilled = strlen(buffer);

   if (!(current->status & HCR))
      lptr = current->next;

   prev = current->prev;
   next = current->next;

   if(current->ls) free(current->ls);
   free(current);


   while(lptr)
      {
/*    len = strlen(lptr->ls); */
      if(lptr->status & HIGHLIGHT)
         {
         if(unmarked == 0)
            unmark();
         unmarked=1;
         }
      if(quote)
        {
        if(strncmpi(lptr->ls, quotestring, strlen(quotestring)) == 0)
           {
           strcpy(lptr->ls, lptr->ls + strlen(quotestring));
           lptr->len = strlen(lptr->ls);
           }
        }

      len = lptr->len;
      bufsize += len;
      if(bufsize > cursize)
          {
          cursize += 1000;
          if( (buffer = realloc(buffer, cursize)) == NULL)
            {
            cls();
            puts("Outa mem!\n");
            exit(254);
            }
          }

      if( (lptr->ls[0] != ' ') &&
          (buffilled != 0)     &&
          (*(buffer+buffilled-1) != ' ') )  /* Have to add a space? */
        {
        *(buffer+buffilled) = ' ';
        buffilled++;
        }


      memmove(buffer + buffilled, lptr->ls, len+1);
      buffilled += len;

/*    strcat(buffer, lptr->ls); */

      next = lptr->next;
      if (lptr->status & HCR)
         {
         if(lptr->ls) free(lptr->ls);
         free(lptr);
         break;
         }
      current = lptr;
      lptr = lptr->next;
      if(current->ls) free(current->ls);
      free(current);
      }

   if(!quote)
       current = lptr = fastFormatText(buffer, maxx);
   else
       current = lptr = fastFormatText(buffer, 65);

   if(quote)
     {
     while(lptr)
       {
       quotedline = xcalloc(1, lptr->len + strlen(quotestring) + 1);
       sprintf(quotedline, "%s%s", quotestring, lptr->ls);
       if(lptr->ls) free(lptr->ls);
       lptr->ls = quotedline;
       lptr->status |= (QUOTE | HCR);
       lptr->len = strlen(lptr->ls);
       lptr = lptr->next;
       }
     lptr = current;
     }


   if(prev) prev->next = current; /* If this isn't first, make link with past :-) */

   current->prev = prev;
   free(buffer);

   if (!first) first = current;
   if (!start) start = current;

   while(lptr->next)
      lptr = lptr->next;

   lptr->next = next;
   if (next)
      next->prev = lptr;

   if (pos > strlen(current->ls))
      {
      if(quote)
         pos += strlen(quotestring);
      pos -= strlen(current->ls);
      if (!current->next)          /* This is last line, add one */
        {
        lptr = xcalloc(1, sizeof(LINE));
        lptr->prev = current;
        current->next = lptr;
        }
      current = current->next;
      row++; line++;
      }

   enter_line();

}

/* -------------------------------------------------------- */
/* Insert a new line after the current line.. */


LINE *insert_after()
{
   LINE *lineptr;

   lineptr = xcalloc(1, sizeof(LINE));   /* get a new line */
   lineptr->ls = xstrdup("");
   lineptr->status |= HCR;
   if(current->status & HIGHLIGHT)
     {
     if( (current != bend) && ( !((bend==NULL) & (current==bbegin)) ) )
        lineptr->status |= HIGHLIGHT;
     }
   lineptr->next = current->next;   /* forward link */
   if (lineptr->next)               /* link back from next element */
      lineptr->next->prev = lineptr;

   current->next = lineptr;         /* insert after current line */
   lineptr->prev = current;         /* backward link */

   return lineptr;
}


/* ------------------------------------------------------------------ */
/* Leave line: copy actual line contents to LINE struct, update attrs */
/* ------------------------------------------------------------------ */


void leave_line(int repaint)
{
   int len = strlen(templine);
   char *charptr;

//   if(len)
//     {
//     charptr = templine + len - 1;
//     while( (*charptr == ' ') && (charptr >= templine) )
//       {
//       *charptr-- = '\0';
//       len--;
//       }
//     }

   if(current->ls) free(current->ls);

   current->ls  = xmalloc(len+1);
   memmove(current->ls, templine, len+1);
   current->len = len;
   current->status &= ~QUOTE;
   if(IsQuote(current->ls))
       current->status |= QUOTE;

   if(repaint)
     {
     if(current->status & HIGHLIGHT)
         printeol(row, 0, cfg.col.editblock, current->ls);
     else if(current->status & QUOTE)
         printeol(row, 0, cfg.col.msgquote, current->ls);
     else
         printeol(row, 0, cfg.col.msgtext, current->ls);

     if( (current->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
         {
         if(current->len < maxx)
             printc(row, current->len, cfg.col.edithcr, 20);
         }
     }

}

/* -------------------------------------------------------- */
/* Make current line the actual line in edit buffer */

void enter_line(void)
{
   memset(templine, '\0', sizeof(templine));
   if(current->ls)
       strcpy(templine, current->ls);
}

/* -------------------------------------------------------- */
/* Import a file from disk.. */

void import_file(void)
{
   BOX *getnamebox, *status;
   static char filename[60] = "";
   LINE *firstnewline = NULL, *lastnewline=NULL, *curline=NULL;
   XFILE *infile;
   char *temp, templine[10];
   unsigned long line=0;


   getnamebox = initbox(10, 5, 14, 75, cfg.col.popframe, cfg.col.poptext, SINGLE, YES, ' ');
   drawbox(getnamebox);
   boxwrite(getnamebox,1,1,"Give file to import: ");
   if(strlen(filename) > 40) filename[40] = '\0';
   if(getstring(12, 28, filename, 43, VFN,cfg.col.entry) != 0)
      memset(filename, '\0', sizeof(filename));

   delbox(getnamebox);

   if( (strchr(filename, '*') != NULL) ||
       (strchr(filename, '?') != NULL) )     /* Wildcard */
       {
       strcpy(filename, getfile(filename));
       }

   if(filename[0] == '\0')
      return;

   if(current->status & HIGHLIGHT) // Shut off block.
     unmark();

   if( (infile = xopen(filename)) == NULL)
      {
      Message("Unable to open inputfile!", -1, 0, YES);
      return;
      }

   status = initbox(10, 25, 14, 55, cfg.col.popframe, cfg.col.poptext, SINGLE, YES, ' ');
   drawbox(status);
   print(12,28,cfg.col.poptext,"Working, line #");

   while( (temp=xgetline(infile)) != NULL)
      {
      if(!(line++ % 10))
         {
         sprintf(templine, "%lu", line++);
         print(12,43,cfg.col.poptext,templine);
         }

      curline = fastFormatText(temp, maxx);

      if(firstnewline == NULL)
         firstnewline = curline;
      else
         {
         lastnewline->next = curline;
         curline->prev = lastnewline;
         }

      while(curline->next)
            curline = curline->next;
      lastnewline = curline;
      }

   delbox(status);

   xclose(infile);

   if(lastnewline)      // Only if any text added!
     {
     if(current->next)
        {
        lastnewline->next = current->next;
        current->next->prev = lastnewline;
        }

     firstnewline->prev = current;
     current->next = firstnewline;
     }

}



/* -------------------------------------------------------- */


void press_enter(void)
{
   char quotestring[20];
   char *tempstring;
   unsigned char oldpos;


  if ( (pos == strlen(templine)) && insert) /* on last pos on line */
       {
       leave_line(0);
       insert_after();
       current->status |= HCR;
       current = current->next;
       enter_line();
       row++; pos= 0; line++;
       }
  else if (insert)
       {
       if(pos == 0) /* We add a blank line, make sure prev line is HCR ended */
          {
          if(current->prev)
             current->prev->status |= HCR;
          }

       lineptr = insert_after();
       if(!(current->status & HCR))  /* Current line not HCR, so toggle off */
           lineptr->status &= ~HCR;

       lineptr->ls = xstrdup(templine + pos);
       templine[pos] = '\0';
       current->status |= HCR;
       oldpos = pos;
       pos = 0;

       if(current->status & QUOTE)
         {
         if(get_quote_string(current->ls, quotestring) != 0)
           {
           if(strncmpi(lineptr->ls, quotestring, strlen(quotestring)) != 0)
              {
              tempstring = xcalloc(1, strlen(lineptr->ls) + strlen(quotestring) + 1);
              if(oldpos < strlen(quotestring))  // Are we actually inside quotestring?
                 {
                 memset(quotestring, '\0', sizeof(quotestring)); // Don't quote quotes!
                 }

              sprintf(tempstring, "%s%s", quotestring, lineptr->ls);
              if(lineptr->ls) free(lineptr->ls);
              lineptr->ls = tempstring;
              pos = strlen(quotestring);
              }
           lineptr->len = strlen(lineptr->ls);
           lineptr->status |= QUOTE;
           }
         /* We moved the entire quote down, leaving only the
            quotestring. Who needs it? Zap out! (Jelle..)    */
         if(strcmp(templine, quotestring) == 0)
            memset(templine, '\0', sizeof(templine));
         }
       leave_line(0);
       current = current->next;
       enter_line();
       row++; line++;
       add_n_wrap(0, 0);
       }
   else
       {
       leave_line(0);
       pos=0;
       row++; line++;
       if (!current->next)
          insert_after();
       current = current->next;
       enter_line();
       }

    update_screen(0);

}

/* -------------------------------------------------------- */

void erase_line(void)
{
   /* If it is the last line, *and* empty, we don't do anything,
      otherwise it might be slowly zapping out the unerase buffer! */

   if( (templine[0] == '\0') && (current->next == NULL) )
      return;

   /* First check the unerase buffer */

   if(clip[CLIPLEN-1] != NULL) /* Remove last element */
      free(clip[CLIPLEN-1]);

   /* Shift all entries one 'up' */

   memmove(clip + 1, clip, (CLIPLEN-1) * (sizeof(char *)));

   clip[0] = xstrdup(templine);

   memset(templine, '\0', sizeof(templine));

   if(current->prev && current->next)  /* Easy, take it out */
      {
      if(bbegin)  // Is there a block?
        {
        if(bbegin == current)
          bbegin = current->next;
        if(bend == current)
          bend = current->next;
        }

      current->prev->next = current->next;
      current->next->prev = current->prev;
      if(current->ls) free(current->ls);
      lptr = current->next;
      if(start == current)
         start = current->next;
      free(current);
      current = lptr;
      }
   else if(!current->prev && !current->next)  /* Only line */
      {
      leave_line(0);
      /* Nothing! */
      }
   else if(!current->prev) /* First line */
      {
      if(bbegin)  // Is there a block?
        {
        if(bbegin == current)
          bbegin = current->next;
        if(bend == current)
          bend = current->next;
        }

      start = first = current->next;
      if(current->ls) free(current->ls);
      free(current);
      current = start;
      current->prev = NULL;
      }
   else                    /* Last line */
      {
/*      lptr = current->prev;
      if(start == current)
         start = current->prev;
      if(row > 1)
         row--;
      if(current->ls) free(current->ls);
      free(current);
      current = lptr;
      current->next = NULL; */
      /* Do nothing! Line was already erased by memset! */
      leave_line(0);
      }

   pos=0;
   enter_line();
   update_screen(0);

}

/* -------------------------------------------------------- */

void move_left(void)
{
	if (pos)
			pos--;
   else if (current->prev)
        {
        leave_line(1);
        current = current->prev;
        enter_line();
        pos = strlen(templine);
        row--; line--;
        }
   if (pos<0) update_screen(1);

}

/* -------------------------------------------------------- */

void move_right(void)
{
	 if (pos < strlen(templine))
			 pos++;
    else if (current->next)
         {
         leave_line(1);
         current = current->next;
         enter_line();
         pos = 0;
         row++; line++;
         }
    if (row>maxy-2) update_screen(1);
}

/* -------------------------------------------------------- */

void delchar(void)
{
   int span;

    curlen = strlen(templine);
	 if( (pos < curlen) && (current->status & HCR))
		 {
		 memmove(templine+pos, templine+pos+1, curlen-pos-1);
		 templine[strlen(templine)-1] = '\0';
		 }
    else
       {
       if (pos < curlen)
          {
			 memmove(templine+pos, templine+pos+1, curlen-pos-1);
			 templine[strlen(templine)-1] = '\0';
          }
       else if (pos == curlen)
          {
          current->status &= ~HCR;
          /* Append space if lines are to be joined */

          if(
              strlen(templine) &&
              current->next &&
              (templine[strlen(templine)-1] != ' ') &&
              strlen(current->next->ls)
            )
             {
        //     templine[strlen(templine)] = '\0';
             strcat(templine, " ");
             }
          }
       if (current->next)
          {
          if((span=strcspn(current->next->ls, " .!?")) < (maxx - strlen(templine)-1) )
                 {
                 add_n_wrap(0, 0);
                 update_screen(0);
                 }
          }
       }
}

/* -------------------------------------------------------- */

void move_up(void)
{
    if (current->prev)
       {
       leave_line(1);
       current = current->prev;
       row--; line--;
       enter_line();
       if (pos > strlen(templine))
          pos = strlen(templine);
       }
    if (row < 1) update_screen(1);

}

/* -------------------------------------------------------- */

void move_down(void)
{
    if (current->next)
       {
       leave_line(1);
       current = current->next;
       row++; line++;
       enter_line();
       if (pos > strlen(templine))
          pos = strlen(templine);
       }
    if (row > maxy-2) update_screen(1);

}


/* -------------------------------------------------------- */

void page_up(void)
{
    leave_line(1);

    if (start->prev)
       {
       for(l=1; l<maxy-2; l++)
          {
          if(start->prev)
             {
             start = start->prev;
             line--;
             }
          else break;
          }
       }
    else
        {
        row=1; /* Can't go one screen up, goto first line */
        line=1;
        }

    current = start;

    for(l=1; l<row; l++)
       {
       if(current->next)
          current = current->next;
       else
          {
          row = l;
          break;
          }
       }

    enter_line();

    if (pos > strlen(templine))
          pos = strlen(templine);

    update_screen(1);


}

/* -------------------------------------------------------- */

void page_down(void)
{
   LINE *walkline;

    leave_line(1);

    /* Find last line on screen */

    current = start;
    changestart = 1;

    for(l=1; l<maxy-2; l++)
       {
       if(current->next)
          current = current->next;
       else
          {
          changestart = 0;
          row = l;
          break;
          }
       }


    if(changestart)
       {
       start = current;

       for(l=1; l<row; l++)
          {
          if(current->next)
             current = current->next;
          else
            {
            row = l;
            break;
            }
          }
       }

    enter_line();

    if (pos > strlen(templine))
          pos = strlen(templine);

    /* Let's recalc the line number, I've lost track of it :-) */
   for(line=1, walkline=first; walkline; walkline=walkline->next, line++)
       {
       if(walkline == current)
          break;
       }

    update_screen(1);

}

/* -------------------------------------------------------- */

void word_left(void)
{

    charptr = templine + pos - 1;

    /* if we are on a space, jump over it.. */

    while( (charptr >= templine) && (*charptr == ' ') )
        {
        pos--;
        charptr--;
        }

    if(pos == 0)  /* Beginning of line, so jump one line up? */
       {
       if(current->prev)
          {
          leave_line(1);
          current = current->prev;
          row--; line--;
          enter_line();
          if(row<1)
            {
            if(start->prev)
              start = start->prev;
            update_screen(1);
            row++;
            }
          pos = current->len; /* -1; */
/*          if(pos<0) pos=0; */
          }
       return;
       }

    charptr = templine + pos - 1;

    /* if we are on a space, jump over it.. */

    while( (charptr >= templine) && (*charptr == ' ') )
        charptr--;

    while(charptr >= templine)
       {
       if (*charptr == ' ')
          {
          pos = (int) (charptr - templine + 1);
          break;
          }
       else if (charptr == templine)
          {
          pos = 0;
          break;
          }
       charptr--;
       }

    if(row<1)
       update_screen(1);

}


/* -------------------------------------------------------- */


void word_right(void)
{
   int foundspace = 0;

     /* Are we at end of line? */

     if(pos == strlen(templine))
        {  /* Yes, so goto next */
        if(current->next)
            {
            leave_line(1);
            current = current->next;
            row++; line++;
            enter_line();
            pos=0;
            }
        /* else */ return; /* There is no next line.. */
        }

     charptr = templine + pos;

     while(*charptr == ' ')
        {
        foundspace = 1;
        charptr++;
        pos++;
        }

     if(foundspace)
        pos = (int) (charptr - templine);
     else
        {

        while(*charptr)
           {
           if(*charptr == ' ')
              {
              pos = (int) (charptr - templine + 1);
              break;
              }
           charptr++;
           }

        if((*charptr == '\0') || (*(charptr+1) == '\0')) /* Reached end of line, start next line */
           {
/*           if(current->next)
               {
               leave_line(1);
               current = current->next;
               row++;
               enter_line();
               pos=0;
               while(templine[pos] == ' ')
                  pos++;
               } */
             pos = strlen(templine);
           }
        }

     if (row > maxy-2) update_screen(1);

}

/* -------------------------------------------------------- */

void backspace(void)
{
   int doredraw = 0;

	 if (pos)
		 {
		 memmove(templine+pos-1, templine+pos, strlen(templine)-pos);
		 templine[strlen(templine)-1] = '\0';
		 pos--;

       if(current->next && !(current->status & HCR))
         {
         if(strcspn(current->next->ls, " .!?") < (maxx - strlen(templine) - 1) )
            {
            add_n_wrap(0, 0);
            update_screen(0);
            }
         }

		 }
    else if (current->prev)
       {
       leave_line(0);
       current = current->prev;
       enter_line();
/*       if (strlen(templine))   /* Add space at end to append */
          {
          if(templine[strlen(templine)-1] != ' ')
             {
             templine[strlen(templine)] = '\0';
             strcat(templine, " ");
             }
          }*/
       pos = strlen(templine);
       current->status &= ~HCR;   /* Dump HCR if it was there */

       line--;
       if(--row < 1) /* Scrolling off top of screen? */
         {
         start = current;
         row = 1;
         doredraw = 1;
         }

       if(strcspn(current->next->ls, " .!?") < (maxx - pos - 1) )
             {
             add_n_wrap(0, 0);
             doredraw = 1;
             }

       if(doredraw)
           update_screen(0);
       }

}

/* Move to the top of the screen (home position) */

void top_of_screen(void)
{

   leave_line(1);
   current = start;
   line -= (row-1);
   pos = 0;
   row = 1;
   enter_line();
/*   update_screen(1); */

}

/* Move to the bottom of the screen */

void bottom_of_screen(void)
{
   leave_line(1);

   while( (row < (maxy-2)) && (current->next) )
     {
     current = current->next;
     row++; line++;
     }
   pos = 0;

   enter_line();
}

/* Top of text */

void top_of_text(void)
{

   leave_line(0);
   current = start = first;
   row = 1; line=1;
   pos = 0;
   enter_line();
   update_screen(1);

}

/* Bottom of text */

void bottom_of_text(void)
{
   int l;
   LINE *end;

   leave_line(0);

   /* First we find the last line */

   while(current->next)
     {
     current = current->next;
     line++;
     }

   /* Find new line at top op page */

   start = current;

   for(l=0; (l< (maxy-2)) && (start->prev); l++)
     start = start->prev;

   /* We are at the end op page */

   row = l + 1;
   pos = 0;

   enter_line();
   update_screen(1);

}

/* Delete the word to the left of the cursor */

void del_word_left(void)
{
  int foundspace = 0;
  char *charptr, *charptr2;

     /* Are we at begin of line? */

     if(pos == 0)
        {  /* Yes, so goto prev */
        if(current->prev)
            {
            leave_line(1);
            current = current->prev;
            enter_line();
            row--; line--;
            pos = strlen(templine);
            if(current->status & HCR)
               current->status &= ~HCR;
            add_n_wrap(0,0);
            update_screen(0);
            }
        return;
        }

     charptr = charptr2 = templine + pos;

     charptr--; pos--;

     /* Are we on a space? */

     if(*charptr == ' ')
       {
       while( (*charptr == ' ') && (pos>0) )
          {
          charptr--;
          pos--;
          }
       if(*charptr != ' ')
         {
         charptr++;
         pos++;
         }
       }
     else   /* So, we are _not_ on a space */
       {
       while( (pos>0) && (*charptr != ' ') )
         {
         charptr--;
         pos--;
         }

       if(*charptr == ' ')
          {
          charptr++;
          pos++;
          }

       }

     memmove(charptr, charptr2, strlen(charptr2)+1);

     add_n_wrap(0,0);
     update_screen(0);

}


void del_word_right(void)
{
  int foundspace = 0;
  char *charptr, *charptr2;

     /* Are we at end of line? */

     if(pos == strlen(templine))
        {  /* Yes, so goto next */
        if(current->next)
            {
            leave_line(1);
            if(current->status & HCR)
               current->status &= ~HCR;
            add_n_wrap(0,0);
            enter_line();
            pos = current->len;
            update_screen(0);
            }
        return;
        }

     charptr = charptr2 = templine + pos;

     /* Are we on a space? */

     if(*charptr == ' ')
       {
       charptr2 = charptr;
       while(*charptr == ' ')
          charptr++;
       }
     else   /* So, we are _not_ on a space */
       {
       while( (*charptr != '\0') && (*charptr != ' ') )
         charptr++;

       if(*charptr == ' ')
          charptr++;
       }

     strcpy(charptr2, charptr);

     add_n_wrap(0,0);
     update_screen(0);

}



/* Unerase a line that was deleted with CTRL-Y */

void unerase(void)
{
   LINE *new;

   if(clip[0] == NULL)
     return;

   if(current->prev)
      {
      leave_line(0);
      current = current->prev;
      enter_line();
      new = insert_after(); /* Get a new line */
      if(new->ls)
          free(new->ls);
      leave_line(0);
      current=current->next;
      }
   else  /* First line! */
      {
      leave_line(0);
      new = first = start = xcalloc(1, sizeof(LINE));
      first->status |= HCR;
      first->next = current;
      current->prev = first;
      current = first;
      }


   new->ls = clip[0]; /* New line is first from unerase buff */
   new->len = strlen(new->ls);

   /* Shift whole buf one 'down' */
   memmove(clip, clip + 1, (CLIPLEN-1) * (sizeof(char *)));
   clip[CLIPLEN-1] = NULL;

   enter_line();

   update_screen(0);

}


/* Do a tab: stuff keys in the 'idlekey' buffer */

void tab(void)
{
   int howmuch=1;
   int key = insert ? 32 : 333; /* Either stuff spaces or <cursor right> */

   while((pos+howmuch) % TABSIZE) /* How far to next TAB position? */
     howmuch++;

   while(howmuch--)
      stuffkey(key);
}


/* --------------------------------------------------------------  */


void clear_unerase(void)
{
   int i;


   for(i=0; i<CLIPLEN; i++)
      if(clip[i])
          {
          free(clip[i]);
          clip[i] = NULL;
          }

}


/* ---------------------------------------------- */

void block_border(void)
{
   LINE *walkline = first;

   /* First border of the block? */

   if( (bbegin == NULL) && (bend == NULL) )
     {
     bbegin = current;
     current->status |= HIGHLIGHT;
     return;
     }

   if(current == bbegin) /* Nothing to do in this case */
      return;

   while( walkline &&
           (walkline != current) &&
           (walkline != bbegin ) )
       walkline = walkline->next;

   if(!walkline)  /* something strange! */
      return;

   /* Is this a new 'end' of the block? If we found 'bbegin' now    */
   /* before we reached 'current', we are defining a new 'end' with */
   /* the current line..                                            */

   if(walkline == bbegin)
     {
     /* Mark all lines from where we are now, till 'current' */
     while(walkline && (walkline != current) )
         {
         walkline->status |= HIGHLIGHT;
         walkline = walkline->next;
         }

     if(!walkline) /* Very strange! */
       return;

     /* We are now at 'current', mark 'current' as well */

     walkline->status |= HIGHLIGHT;
     bend = walkline;         /* This is now new end of block */

     walkline = walkline->next;

     /* Now we 'unmark' everything until the end of the text, in case */
     /* there are old block-highlights around (if we are making the   */
     /* current block smaller                                         */

     while(walkline)
        {
        walkline->status &= ~HIGHLIGHT;
        walkline = walkline->next;
        }
     }
   else  /* We are at current, BEFORE bbegin */
     {
     /* Mark evrything from here, until we reach bbegin */

     while(walkline && (walkline != bbegin) )
         {
         walkline->status |= HIGHLIGHT;
         walkline = walkline->next;
         }

     if(!walkline)   /* Strange! */
        return;

     /* If there is no 'bend', we are defining the start of the block  */
     /* In that case, the value of 'bbegin' turns out the the 'bend'.. */

     if(!bend)
        bend = bbegin;

     bbegin = current;
     }

   update_screen(1);

}


/* --------------------------------------------------- */

void unmark(void)
{
   LINE *walkline = first;

   bbegin = bend = NULL;

   while(walkline)
     {
     walkline->status &= ~HIGHLIGHT;
     walkline = walkline->next;
     }

   update_screen(1);

}

/* ---------------------------------------------------------- */

int del_block(void)
{
   LINE *walkline,
        *bleft,      /* Begin of what is Left */
        *eleft,      /* End of what is Left   */
        *next;
   int i;


   /* We need a block! */

   if(!bbegin)
      return -1;

   bleft = bbegin->prev;
   if(bend)
      eleft = bend->next;
   else eleft = bbegin->next;

   if(bbegin == first)    /* Delete first line? New begin = end of block */
     {
     if(bend)
        first = bend->next; /* Might be NULL now! */
     else
        first = first->next;
     }

   if(bbegin == start)
     start = start->prev;

   if(start == NULL)
       {
       if(bend != NULL)
          start = bend->next; /* Might be NULL now! */
       else
          start = bbegin->next;
       }


   /* Walk through block */

   walkline = bbegin;

   while(
           walkline &&
          (walkline != bend)
        )
          {
          if(walkline == current) /* Current line is deleted! */
             current = NULL;

          if(walkline == start)
             start = NULL;

          if(walkline->ls)
             free(walkline->ls);
          next = walkline->next;

          free(walkline);
          walkline = next;
          if(!bend)
             break;
          }

  /* if(!walkline)   /* Very strange */
     return; */

   /* Walkline == bend now.. */

   if((bend == current) || ((!bend) &&(bbegin==current))) /* Current line is deleted! */
     current = NULL;

   if((bend == start) || ((!bend) &&(bbegin==start)))
     start = NULL;

   if(bend && bend->ls)
       free(bend->ls);

   if(bend)
      free(bend);

   bend   = NULL;
   bbegin = NULL;

   if(bleft) bleft->next = eleft;
   if(eleft) eleft->prev = bleft;

   if(current == NULL)
     {
     if(bleft)
       current = bleft;
     else current = eleft; /* Might be NULL.. */
     if(current)
       enter_line();
     }

   if(current)
      {
      i = row;
      walkline = current;
      while(walkline && (i>1))
        {
        i--;
        walkline = walkline->prev;
        if(walkline)
           start = walkline;
        }
      if(!walkline)
        row -= i;

      }

    if(start == NULL)   // Do YOU feel desoriented?
       start = current;

   if(first == NULL)     /* All text was deleted! */
      {
      first = start = current = xcalloc(1, sizeof(LINE));
      first->ls = xstrdup("");
      row=1;
      pos=0;
      enter_line();
      }


    /* Let's recalc the line number, I've lost track of it :-) */
   for(line=1, walkline=first; walkline; walkline=walkline->next, line++)
       {
       if(walkline == current)
          break;
       }

   update_screen(1);

   return 0;
}


/* Copy a marked block to another destination */


int copy_block(void)
{
   LINE *walkline,
        *bleft,      /* Begin of what is Left */
        *eleft,      /* End of what is Left   */
        *next;
   int i;


   /* We need a block! */

   if(!bbegin)
      return -1;

   if(!bend)
      bend = bbegin;

   if(current->next && (current->next->status & HIGHLIGHT))
      return -1;

   for(walkline=bend; walkline != NULL; walkline = walkline->prev)
      {
      next = insert_after();
      if(next->ls) free(next->ls);
      next->ls = xstrdup(walkline->ls);
      next->status = (walkline->status & ~HIGHLIGHT);
      next->len = walkline->len;
      if(walkline == bbegin) /* Found begin of block, stop */
         break;
      }

   update_screen(1);

   return 0;

}


/* Move an entire block to another destination */


void move_block(void)
{
   if(copy_block() != -1)
      del_block();
}

/* ------------------------------------------------- */


char *getfile(char *filespec)

{
   struct ffblk ffblk;
   FILELIST *ff=NULL, *cf;
   int done;
   char temp[60], *location, drive[MAXDRIVE], dir[MAXDIR], *lastpath=NULL;
   static char selected[120];

   memset(selected, '\0', sizeof(selected));

   fnsplit(filespec, drive, dir, NULL, NULL);
   location = xmalloc(MAXPATH);
   sprintf(location, "%s%s", drive, dir);

   done = findfirst(filespec, &ffblk, 0);
   while (!done)
     {
     if(!ff)
        cf = ff = xmalloc(sizeof(FILELIST));
     else
        {
        cf->next = xmalloc(sizeof(FILELIST));
        cf = cf->next;
        }

     strcpy(cf->name, ffblk.ff_name);
     cf->size = ffblk.ff_fsize;
     cf->tagged = 0;
     cf->path = location;
     cf->next = NULL;

     done = findnext(&ffblk);
     }

   strcpy(selected, showget(ff));

   cf = ff;
   while(cf)
     {
     ff = cf->next;
     if (lastpath != cf->path)
        {
        lastpath = cf->path;
        free(cf->path);
        }
     free(cf);
     cf = ff;
     }

   return selected;

}


/* ------------------------------------------------------ */


char *showget(FILELIST *first)

{

   BOX *filebox;

   FILELIST    *curptr, *highlighted;
   int   maxlines=25, numlines=0, rows=0, l=0;
   int curline=0, start=0;
   char  temp[81];
   static char selected[120];

   memset(selected, '\0', sizeof(selected));

   if(!first)
      {
      Message("No files found!", -1, 0, YES);
      return selected;
      }

   if(first->next == NULL)
      {
      sprintf(selected, "%s%s", first->path, first->name);
      return selected;
      }

   for(curptr=first; curptr->next != NULL; curptr=curptr->next)
         numlines++;

   rows = numlines>maxlines-4 ? maxlines-4 : numlines+1;

   filebox = initbox(1,0,rows+2,24,cfg.col.asframe,cfg.col.astext,SINGLE,YES,' ');
   drawbox(filebox);

   while(1)    /* Exit is w/ return statement that gives command back */
      {
      curptr=first;

      /* search first line to display */

      for(l=0; l<start; l++)
         curptr=curptr->next;

      /* display them.. */

      for(l=start; l<start+rows; l++, curptr=curptr->next)
         {
         sprintf(temp, " %-12.12s   %4d K ", curptr->name, curptr->size/1024);

         if (l==curline)
            {
            print(l-start+2, 1, cfg.col.ashigh, temp);
            highlighted = curptr;
            MoveXY(2,l-start+3);
            }
         else
            print(l-start+2, 1, cfg.col.astext, temp);
         }

      /* Now check for commands... */

      switch (get_idle_key(1))
         {
         case 328:      /* up */
            if (curline)
               {
               curline--;
               if (curline<start)
                  start=curline;
               }
            else
               {
               curline=numlines;
               start=numlines-rows+1;
               }
            break;

         case 336:      /* down */
            if (curline<numlines)
               {
               curline++;
               if (curline>=start+rows)
                     start++;
               }
            else
               start=curline=0;
            break;

         case 327:      /* home */
            curline=start=0;
            break;

         case 335:      /* end */
            curline=numlines;
            start=numlines-rows+1;
            break;

         case 329:      /* page up */

            if (start == 0)
               {
               curline = 0;
               break;
               }

            start = curline-rows;

            if (start < 0)
               start = 0;

            if (curline > start + rows - 1)
               curline = start;

            break;

         case 337:      /* page down */

            if (start == numlines-rows+1)
               {
               curline = numlines;
               break;
               }

            start = curline+rows;

            if (start > numlines-rows+1)
               start = numlines-rows+1;

            if (curline < start)
               curline = start;

            break;

         case 27:
            delbox(filebox);
            return selected;

         case 13:    /* <CR> */
         case 32:    /* Space */

            sprintf(selected, "%s%s", highlighted->path, highlighted->name);
            return selected;

         }           /* switch */

   }        /* while(1) */

}


/* ---------------------------------------------------- */


int get_quote_string(char *line, char *quote)
{
   char *p = line;
   char *last = NULL;
   int teller=0;

   quote[0] = '\0';

   if(!line || !quote)
     return 0;

   while( (*p != '<') && (teller++ < 11) && (*p != '\0'))
      {
      if(*p == '>')
         last = p;
      p++;
      }

   if(!last)
      return 0;

   if(*(last+1) == ' ')
      last++;

   memmove(quote, line, last-line+1);
   *(quote + (last - line) + 1) = '\0';

   return 1;
}


void copy_line (void)
{
   LINE *copy;

   copy = insert_after();
   copy->ls = xstrdup(templine);
   copy->len = strlen(templine);
   copy->status = current->status;
   if(bend == current)
      bend = copy;
   if(!bend && (bbegin==current))
      bend = copy;

   leave_line(1);
   current = current->next;
   row++; line++;
   enter_line();
   if (pos > strlen(templine))
          pos = strlen(templine);

    if (row > maxy-2)
       update_screen(1);
    else
       update_screen(0);

}
