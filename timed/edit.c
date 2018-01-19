#include "includes.h"

//#define HILITE      15
//#define NORMTEXT    7
//#define HCRCOLOUR   8
//#define QUOTECOL    14
//#define BLOCKCOL    112
//#define BLOCKCURCOL 113

#define TABSIZE 3


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
char *charptr;
LINE *lineptr, *lptr;
int changestart;
int noscreen;

#define CLIPLEN 50


static char *clip[CLIPLEN];

void  update_screen (int entire         );
//LINE  *edittext     (LINE *begin, char *topline);
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
void unmark       (LINE *first, int screenupdate);
int  del_block    (void);
int  copy_block   (void);
void move_block   (void);

char *getfile(char *filespec);
//char *showget(FILELIST *first);

int  get_quote_string(char *line, char *quote);
void zapquotes(void);
void near statusline(void);
void MakeTopLine(AREA *area, MMSG *curmsg);
void SetEditCursor(void);


/* -------------------------------- */


/* void main(void)

{
   LINE *this;

   video_init();

   this = mem_calloc(1, sizeof(LINE));
   this->ls = mem_strdup("Dit is even een test om te kijken of er nog leven in de");
   this->len = strlen(this->ls);
   edittext(this);
}
*/




/* ------------------------------------------------------- */

LINE *edittext(LINE *begin, AREA *area, MSG *areahandle, MMSG *curmsg, int curtxtline)
{
   LINE *curline, *nextline;
   int origclockstate = showclock;
   int i;

   /* init values.. */

    ret=0;
    insert=1;
    curlen=0;
    c = 0;
    line=1;
    noscreen = 0;

    memset(&clip, '\0', sizeof(clip));

    if(begin == NULL) /* No text given, make first line ourselves */
       {
       begin = mem_calloc(1, sizeof(LINE));
       begin->ls = mem_strdup("");
       begin->status |= HCR;
       }

    first = current = start = begin;
    bbegin = bend = NULL;
    pos = 0;
    row = 1; /* Allow top line for statusbar */

    // Position on correct line to start editing..
    for(i=0; i<curtxtline; i++)
        stuffkey(cEDITdown);

#ifndef __WATCOMC__
	 _setcursortype(_NORMALCURSOR);
#else
    _settextcursor(0x0407);
#endif
    clsw(cfg.col[Cmsgtext]);

    MakeTopLine(area, curmsg);

    statusline();
    clockon();

    update_screen(1);

    enter_line();

    printeol(row, 0, cfg.col[Ceditcurnormal], templine);
    if( (current->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
       {
       if(current->len < maxx)
           printc(row, current->len, cfg.col[Cedithcr], 20);
       }

    MoveXY(pos+1, row+1);

	 while (1)
		  {
//        if(heapcheck() != _HEAPOK)
//           {
//           beep(); beep();
//           print(maxy-1, 0, 7, "====>> Heap Corrupt! <<====");
//           kbflush();
//           }

        c = get_idle_key(1, EDITORSCOPE);

		  switch(c) {
             case cEDITeditheader:               /* ALT-E */
               savescreen();
               clockoff();
               clsw(cfg.col[Cmsgtext]);
               EditHeader(areahandle,
                          curmsg,
                          area->type == NETMAIL,
                          custom.aka,
                          area->type == NETMAIL,
                          area);
               SetEditCursor();
               putscreen();
               clockon();
               MakeTopLine(area, curmsg);

               break;

             case cEDITenter:              /* Enter */

               press_enter();
               break;

             case cEDITfiledelete:

               FileDelete();
               SetEditCursor();
               break;

             case cEDITdelwordright:              /* CTRL - T */

               del_word_right();
               break;

             case cEDITunerase:              /* CTRL - U */

               unerase();
               break;

             case cEDITdelline:              /* CTRL - Y */

               erase_line();
               break;


             case cEDITabort:               /* ESC */
               if(cfg.usr.status & CONFIRMEDITEXIT)
                 {
                 #ifdef __WATCOMC__
                 _settextcursor(0x2000);
                 #else
                 _setcursortype(_NOCURSOR);
                 #endif
                 if(!confirm("Do you really want to dump this message? [y/N]"))
                   {
                   SetEditCursor();
                   break;
                   }
                 }

               curline = first;          /* mem_free all lines */
               while(curline)
                  {
                  nextline = curline->next;
                  if(curline->ls)
                     mem_free(curline->ls);
                  mem_free(curline);
                  curline=nextline;
                  }
               clear_unerase();
               #ifdef __WATCOMC__
               _settextcursor(0x2000);
               #else
               _setcursortype(_NOCURSOR);
               #endif
               if(origclockstate == 0)
                  clockoff();
					return NULL;          /* return nothing */

             case cEDITdelwordleft:             /* CTRL-Backspace */
                 del_word_left();
                 break;

             case cEDITimportfile:             /* ALT-I */

                 import_file();
                 update_screen(1);
                 SetEditCursor();
                 break;

             case cEDITrunexternal:
                 editrunexternal(area, areahandle, curmsg);
                 SetEditCursor();
                 break;

             case cEDITwriteraw:
                 WriteToFile(1, 0, area, areahandle, curmsg);
                 SetEditCursor();
                 break;

             case cEDITwritefmt:
                 WriteToFile(0, 0, area, areahandle, curmsg);
                 SetEditCursor();
                 break;

             case cEDITwriterawblock:
                 WriteToFile(1, 1, area, areahandle, curmsg);
                 SetEditCursor();
                 break;

             case cEDITwritefmtblock:
                 WriteToFile(0, 1, area, areahandle, curmsg);
                 SetEditCursor();
                 break;

             case cEDITsave:              /* ALT - S */
                 leave_line(0);
                 #ifdef __WATCOMC__
                 _settextcursor(0x2000);
                 #else
                 _setcursortype(_NOCURSOR);
                 #endif
                 // Now we get to the editmenu
                 if((cfg.usr.status & EDITSAVEMENU) && (ShowEditMenu(curmsg, 1) == -1))
                   {
                   SetEditCursor();
                   break;
                   }
                 clear_unerase();
                 if(origclockstate == 0)
                    clockoff();
                 return first;

             case cEDITsavemenu:
                 leave_line(0);
                 #ifdef __WATCOMC__
                 _settextcursor(0x2000);
                 #else
                 _setcursortype(_NOCURSOR);
                 #endif
                 clear_unerase();
                 if(origclockstate == 0)
                    clockoff();
                 if(ShowEditMenu(curmsg, 1) == -1)
                   {
                   SetEditCursor();
                   break;
                   }
                 return first;

             case cEDITtogglehcr:              /* ALT-H */

                  if(cfg.usr.status & SHOWEDITHCR)
                     cfg.usr.status &= ~SHOWEDITHCR;
                  else
                     cfg.usr.status |= SHOWEDITHCR;
                  leave_line(0);  // Update line in mem for update_screen();
                  enter_line();
                  update_screen(1);
                  break;

             case cEDITshell:              /* ALT-J */

                  shell_to_DOS();
                  SetEditCursor();
                  break;

             case cEDITzapquotes:     // ALT-Z

                zapquotes();
                break;

             case cEDIThelp:    /* F1 */

                #ifdef __WATCOMC__
                _settextcursor(0x2000);
                #else
                _setcursortype(_NOCURSOR);
                #endif
                show_help(5);
                SetEditCursor();
                break;

             case cEDITdupline:    /* F4 */
                 copy_line();
                 break;

             case cEDITmarkblock:              /* ALT-L */
                 block_border();
                 break;

             case cEDITunmarkblock:              /* ALT-U */
                 unmark(NULL, 1);
                 break;

             case cEDITcopyblock:              /* ALT-C */
                 copy_block();
                 break;

             case cEDITmoveblock:              /* ALT-M */
                 move_block();
                 break;

             case cEDITdelblock:              /* ALT-D */
                 del_block();
                 break;

             case cEDITleft:              /* left */

                 move_left();
                 break;

             case cEDITright:        /* right */
                 move_right();
					  break;

             case cEDITendline:        /* end */
					  pos = strlen(templine);
					  break;

             case cEDITbegline:        /* home */
					  pos = 0;
					  break;

             case cEDITtoggleinsert:        /* insert */

                  insert = insert ? 0 : 1;
                  SetEditCursor();
                  break;

             case cEDITbegpage:        /* ctrl page-up */
                  top_of_screen();
                  break;

             case cEDITendpage:        /* ctrl page-dn */
                  bottom_of_screen();
                  break;

             case cEDITbegtext:        /* ctrl home */
                  top_of_text();
                  break;

             case cEDITendtext:        /* ctrl end */
                  bottom_of_text();
                  break;

             case cEDITdel:        /* del */

                 delchar();
					  break;

             case cEDITup:        /* up */
                 move_up();
                 break;

             case cEDITdown:        /* down */
                 move_down();
                 break;

             case cEDITpageup:        /* Page Up */
                 page_up();
                 break;

             case cEDITpagedown:        /* Page Down */
                 page_down();
                 break;

             case cEDITbacktab:        /* TAB left */
                 break;

             case cEDITjumpwordleft:        /* ctrl - left */
                  word_left();
                  break;

             case cEDITjumpwordright:        /* ctrl - right */
                  word_right();
                  break;

             case cEDITdeltoeol:       /* CTRL - del */
                  memset(templine+pos, '\0', strlen(templine) - pos);
                  break;

            case cEDITback:                 /* backspace */
                backspace();
                break;

            case cEDITtab:                 /* TAB */
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
           printeol(row, 0, cfg.col[Ceditcurblock], templine);
	     else if(IsQuote(templine))
           printeol(row, 0, cfg.col[Ceditcurquote], templine);
        else
           printeol(row, 0, cfg.col[Ceditcurnormal], templine);

        if( (current->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
           {
           if(strlen(templine) < maxx)
               printc(row, strlen(templine), cfg.col[Cedithcr], 20);
           }

        statusline();
        MoveXY(pos+1, row+1);

		}   /* end while */

}


/* -------------------------------------------------------- */


void update_screen(int entire)

{
   LINE *thisone;
   char lines = 0;
   int  reached_current = 0;
   int colour;

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

   if(noscreen)
      return;

   if(current == NULL)
      Message("Current == NULL!", -1, 0, YES);

   for(thisone=start; thisone && lines < (maxy-2); thisone = thisone->next, lines++)
      {
      if(thisone == current)
         {
         reached_current = 1;
         if(thisone->status & HIGHLIGHT)
            colour = cfg.col[Ceditcurblock];
         else if(thisone->status & QUOTE)
            colour = cfg.col[Ceditcurquote];
         else
            colour = cfg.col[Ceditcurnormal];

         printeoln(lines+1, 0, colour, thisone->ls, thisone->len);
         }
      else if(reached_current || entire || (thisone->next && (thisone->next == current)) )
          {
          if(thisone->status & HIGHLIGHT)
             colour = cfg.col[Ceditblock];
          else if(thisone->status & QUOTE)
             colour = cfg.col[Cmsgquote];
          else
             colour = cfg.col[Cmsgtext];

          printeoln(lines+1, 0, colour, thisone->ls, thisone->len);

          if( (thisone->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
             {
             if(thisone->len < maxx)
                 printc(lines+1, thisone->len, cfg.col[Cedithcr], 20);
             }
          }
      }

   if(lines < (maxy-2))
      ClsRectWith(1+lines,0,maxy-2,maxx-1,cfg.col[Cmsgtext],' ');

}

/* -------------------------------------------------------- */

void add_n_wrap(char c, char insert)
{
   int bufsize, cursize=200;
   LINE *lptr = NULL, *prev = NULL, *next = NULL;
   int len, buffilled;
   char *buffer;
   char quotestring[20], *quotedline;
   int quote = 0, unmarked=0;


   if(current->status & HIGHLIGHT)
      {
      unmarked=1;
      unmark(NULL, 1);  // Shut off block, will be messed up anyway
      }

   if (current == first)   /* this won't last ;-) */
      first = NULL;

   if (current == start)
      start = NULL;

   leave_line(0);

   buffer = mem_calloc(1, cursize);

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

   if(current->ls) mem_free(current->ls);
   mem_free(current);
   current = NULL;

   while(lptr)
      {
      if(lptr->status & HIGHLIGHT)
         {
         if(unmarked == 0)
            unmark(lptr, 0);
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

      if((buffilled+len+10) > cursize)
          {
          cursize += 200;
          if( (buffer = realloc(buffer, cursize)) == NULL)
            {
            cls();
            puts("Outa mem!\n");
            exit(254);
            }
          }

      if( (lptr->ls[0] != ' ') &&
          (buffilled != 0)     &&
          (*(buffer+buffilled-1) != ' ') &&
          (!(lptr->status & NOSPACE)) )  /* Have to add a space? */
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
         if(lptr->ls) mem_free(lptr->ls);
         mem_free(lptr);
         break;
         }
      current = lptr;
      lptr = lptr->next;
      if(current->ls) mem_free(current->ls);
      mem_free(current);
      }

   if(!quote)
       current = lptr = fastFormatText(buffer, maxx);
   else
// !!!!!!!!!       current = lptr = fastFormatText(buffer, 65);
       current = lptr = fastFormatText(buffer, 73 - strlen(quotestring));

   if(quote)
     {
     while(lptr)
       {
       quotedline = mem_calloc(1, lptr->len + strlen(quotestring) + 1);
       sprintf(quotedline, "%s%s", quotestring, lptr->ls);
       if(lptr->ls) mem_free(lptr->ls);
       lptr->ls = quotedline;
       lptr->status |= (QUOTE | HCR);
       lptr->len = strlen(lptr->ls);
       lptr = lptr->next;
       }
     lptr = current;
     }


   if(prev) prev->next = current; /* If this isn't first, make link with past :-) */

   current->prev = prev;
   mem_free(buffer);

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
        lptr = mem_calloc(1, sizeof(LINE));
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

   lineptr = mem_calloc(1, sizeof(LINE));   /* get a new line */
   lineptr->ls = mem_strdup("");
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
   int colour;

   if(current->ls) mem_free(current->ls);

   current->ls  = mem_malloc(len+1);
   memcpy(current->ls, templine, len+1);
   current->len = len;
   current->status &= ~QUOTE;
   if(IsQuote(current->ls))
       current->status |= QUOTE;

   if(repaint & !noscreen)
     {
     if(current->status & HIGHLIGHT)
         colour = cfg.col[Ceditblock];
     else if(current->status & QUOTE)
         colour = cfg.col[Cmsgquote];
     else
         colour = cfg.col[Cmsgtext];

     printeol(row, 0, colour, current->ls);

     if( (current->status & HCR) && (cfg.usr.status & SHOWEDITHCR) )
         {
         if(current->len < maxx)
             printc(row, current->len, cfg.col[Cedithcr], 20);
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
   static char filename[80] = "";
   LINE *firstnewline = NULL, *lastnewline=NULL, *curline=NULL;
   XFILE *infile;
   char *temp, templine[10];
   unsigned long line=0;
   struct stat mystat;
   char **result = NULL;
   int retval;


   getnamebox = initbox(9, 2, 14, 76, cfg.col[Cpopframe], cfg.col[Cpoptext], SINGLE, YES, ' ');
   drawbox(getnamebox);
   boxwrite(getnamebox,0,1,"Give file to import: ");
   if(strlen(filename) > 69) filename[69] = '\0';
   if((retval=getstring(12, 4, filename, 71, 71, VFNWW,cfg.col[Centry], cfg.col[Cpoptext])) != 0)
      memset(filename, '\0', sizeof(filename));

   delbox(getnamebox);

   if(retval == ESC)
     return;

   if(filename[0] == '\0')
      strcpy(filename, "*.*");

   if( (strchr(filename, '*') != NULL) ||
       (strchr(filename, '?') != NULL) )     /* Wildcard */
       {
       if( (result = dirlist(filename, 0)) != NULL )
          {
          strcpy(filename, result[0]);
          free_filelist(result);
          }
       else
          {
          memset(filename, '\0', 60);
          return;
          }
       }
   else     // No explicit wildcard, but may be directory name..
       {
       if(stat(filename, &mystat) == -1)
          {
          Message("Cannot access file!", -1, 0, YES);
          return;
          }

       if(mystat.st_mode & S_IFDIR)          // It's a directory!
          {
          Strip_Trailing(filename, '\\');
          strcat(filename, "\\*.*");
          if( (result = dirlist(filename, 0)) != NULL )
             {
             strcpy(filename, result[0]);
             free_filelist(result);
             }
          else
             {
             memset(filename, '\0', 80);
             return;
             }
          }
       }


   if(current->status & HIGHLIGHT) // Shut off block.
     unmark(NULL, 1);

   if( (infile = xopen(filename)) == NULL)
      {
      Message("Unable to open inputfile!", -1, 0, YES);
      return;
      }

   status = initbox(10, 25, 14, 55, cfg.col[Cpopframe], cfg.col[Cpoptext], SINGLE, YES, ' ');
   drawbox(status);
   print(12,28,cfg.col[Cpoptext],"Working, line #");

   while( (temp=xgetline(infile)) != NULL)
      {
      if(!(line++ % 10))
         {
         sprintf(templine, "%lu", line++);
         print(12,43,cfg.col[Cpoptext],templine);
         #ifndef __FLAT__
         if(coreleft() < 10000L)
           {
           Message("Not enough memory to load entire file!", -1, 0, YES);
           break;
           }
         #endif
         }

//      curline = fastFormatText(temp, maxx);
      curline = wraptext(temp, maxx, 1, 1);
      checklines(curline);

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
      curline->status |= HCR;
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

       if(lineptr->ls) mem_free(lineptr->ls);
       lineptr->ls = mem_strdup(templine + pos);
       templine[pos] = '\0';
       current->status |= HCR;
       oldpos = pos;
       pos = 0;

       if(current->status & QUOTE)
         {
//         if(get_quote_string(current->ls, quotestring) != 0)
         if(get_quote_string(templine, quotestring) != 0)
           {
           if(strncmpi(lineptr->ls, quotestring, strlen(quotestring)) != 0)
              {
              tempstring = mem_calloc(1, strlen(lineptr->ls) + strlen(quotestring) + 1);
              if(oldpos < strlen(quotestring))  // Are we actually inside quotestring?
                 {
                 memset(quotestring, '\0', sizeof(quotestring)); // Don't quote quotes!
                 }

              sprintf(tempstring, "%s%s", quotestring, lineptr->ls);
              if(lineptr->ls) mem_free(lineptr->ls);
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
      mem_free(clip[CLIPLEN-1]);

   /* Shift all entries one 'up' */

   memmove(clip + 1, clip, (CLIPLEN-1) * (sizeof(char *)));

   clip[0] = mem_strdup(templine);

   memset(templine, '\0', sizeof(templine));

   if(bbegin && (bbegin == bend))
     unmark(NULL, 1);

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
      if(current->ls) mem_free(current->ls);
      lptr = current->next;
      if(start == current)
         start = current->next;
      mem_free(current);
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
      if(current->ls) mem_free(current->ls);
      mem_free(current);
      current = start;
      current->prev = NULL;
      }
   else                    /* Last line */
      {
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
//        leave_line(1);
//        current = current->prev;
//        enter_line();
//        pos = strlen(templine);
//        row--; line--;
// is now:
        move_up();
        pos = strlen(templine);
        }

//   if (pos<0) update_screen(1);

}

/* -------------------------------------------------------- */

void move_right(void)
{
	 if (pos < strlen(templine))
			 pos++;
    else if (current->next)
         {
//         leave_line(1);
//         current = current->next;
//         enter_line();
         move_down();
         pos = 0;
//         row++; line++;
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
            if (row > maxy-2) update_screen(1);
            }
        return;
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
             pos = strlen(templine);
           }
        }

     if (row > maxy-2) update_screen(1);

}

/* -------------------------------------------------------- */

void backspace(void)
{
   if(current->prev || pos!=0)
     {
     move_left();
     delchar();
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
//   int l;
//   LINE *end;

//   leave_line(0);

   /* First we find the last line */

//   while(current->next)
//     {
//     current = current->next;
//     line++;
//     }

   /* Find new line at top op page */

//   start = current;

//   for(l=0; (l< (maxy-2)) && (start->prev); l++)
//     start = start->prev;

   /* We are at the end op page */

//   row = l + 1;
//   pos = 0;

//   enter_line();

   noscreen=1;
   while(current->next)
     move_down();
   noscreen=0;

   update_screen(1);

}

/* Delete the word to the left of the cursor */

void del_word_left(void)
{
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
  char *charptr, *charptr2;

     /* Are we at end of line? */

     if(pos == strlen(templine))
        {  /* Yes, so goto beginning of next line */
        if(current->next == NULL)
          return;       // No more lines, so stop.

        stuffkey(cEDITdel);

//        if(current->next)
//            {
//            leave_line(1);
//            if(current->status & HCR)
//               current->status &= ~HCR;
//            add_n_wrap(0,0);
//            enter_line();
//            pos = current->len;
//            update_screen(0);
//            }
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

   if(current == start)   // First line of screen?
      start = NULL;

   if(current->prev)
      {
      leave_line(0);
      current = current->prev;
      enter_line();
      new = insert_after(); /* Get a new line */
      if(new->ls)
          mem_free(new->ls);
      leave_line(0);
      current=current->next;
      }
   else  /* First line! */
      {
      leave_line(0);
      new = first = start = mem_calloc(1, sizeof(LINE));
      first->status |= HCR;
      first->next = current;
      current->prev = first;
      current = first;
      }

   if(start == NULL)       // Were we at first line of screen?
      start = current;

   new->ls = clip[0]; /* New line is first from unerase buff */
   new->len = strlen(new->ls);
   if(IsQuote(new->ls)) new->status |= QUOTE;

   /* Shift whole buf one 'down' */
   memmove(clip, clip + 1, (CLIPLEN-1) * (sizeof(char *)));
   clip[CLIPLEN-1] = NULL;

   enter_line();
   if(pos > strlen(templine)) pos = strlen(templine);  // Adjust pos on line.

   update_screen(0);

}


/* Do a tab: stuff keys in the 'idlekey' buffer */

void tab(void)
{
   int howmuch=1;

   if(insert)
     {
     stuffkey(32);
     while((pos+howmuch) % TABSIZE) /* How far to next TAB position? */
        {
        howmuch++;
        stuffkey(32);
        }
     return;
     }

   // ! insert

   if(pos == strlen(templine))
     {
     if((current->status & HCR) && (pos < (maxx-4)))
        stuffkey(32);
     else
       {
       stuffkey(cEDITright);
       return;
       }
     }
   else
     stuffkey(cEDITright);

   while((pos+howmuch) % TABSIZE) /* How far to next TAB position? */
     {
     if( (pos+howmuch) < strlen(templine))
        stuffkey(cEDITright);
     else
       {
       if( (current->status & HCR) && ((pos+howmuch) < maxx-4) )
          return;
       }
     howmuch++;
     }

}


/* --------------------------------------------------------------  */


void clear_unerase(void)
{
   int i;


   for(i=0; i<CLIPLEN; i++)
      if(clip[i])
          {
          mem_free(clip[i]);
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

void unmark(LINE *start, int screenupdate)
{
   LINE *walkline;


   walkline = (start != NULL) ? start : first;

   bbegin = bend = NULL;

   while(walkline)
     {
     walkline->status &= ~HIGHLIGHT;
     walkline = walkline->next;
     }

   if(screenupdate) update_screen(1);

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
             mem_free(walkline->ls);
          next = walkline->next;

          mem_free(walkline);
          walkline = next;
          if(!bend)
             break;
          }

   /* Walkline == bend now.. */

   if((bend == current) || ((!bend) &&(bbegin==current))) /* Current line is deleted! */
     current = NULL;

   if((bend == start) || ((!bend) &&(bbegin==start)))
     start = NULL;

   if(bend && bend->ls)
       mem_free(bend->ls);

   if(bend)
      mem_free(bend);

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
      first = start = current = mem_calloc(1, sizeof(LINE));
      first->ls = mem_strdup("");
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
        *next;


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
      if(next->ls) mem_free(next->ls);
      next->ls = mem_strdup(walkline->ls);
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
   if(copy->ls) mem_free(copy->ls);
   copy->ls = mem_strdup(templine);
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

// Zap all quotes lines below cursor until non-quoted, non empty line found

void zapquotes(void)
{
   LINE *oldcur;

   unmark(NULL, 1);

   oldcur = current;
   if(!current->next) return;

   noscreen=1;
   leave_line(0);

   if((current->len == 0) || !(current->status & QUOTE) )
      current = current->next;

   while(current)
     {
     if(!(current->status & QUOTE) && current->len > 0)
       break;  // normal line, break.

     block_border();
     current = current->next;
     }

   current = oldcur;
   enter_line();
   noscreen=0;
   del_block();

}


void near statusline(void)
{
   char temp[140];
   size_t addedlen;

   memset(temp, ' ', sizeof(temp));
   temp[139]='\0';

#if !defined(__OS2__) && !defined(__NT__) && !defined(__GCC__)
        #ifndef __SENTER__
        sprintf(temp, " F1 = Help ³ Pos: %-3d Line: %-4d             %3luK           %s ", pos, line, ((unsigned long)coreleft())/1024, ((insert==1) ? "Insert  " : "TypeOver"));
        #else
        sprintf(temp, " %-3d,%-4d ³ ~F1~ Help   ~ALT-S~ Save   ~esc~ Abort   ~CTRL-Y~ Del line  %s ", pos, line, ((insert==1) ? "Insert  " : "TypeOver"));
        #endif
#else
        sprintf(temp, " F1 = Help ³ Pos: %-3d Line: %-4d                             %s ", pos, line, insert ? "Insert  " : "TypeOver");
#endif

   addedlen = strlen(temp) - HLstrlen(temp);
   temp[strlen(temp)] = ' ';
   temp[maxx-8+addedlen]='\0';
   if(cfg.usr.status & CLOCK)
      biprint(maxy-1,0,cfg.col[Cmsgbar],cfg.col[Cmsgbaraccent], temp, '~');
   else
      biprinteol(maxy-1,0,cfg.col[Cmsgbar],cfg.col[Cmsgbaraccent], temp, '~');
}

// ==============================================================

void MakeTopLine(AREA *area, MMSG *curmsg)
{
   char temp[140];
   char subject[80];
   char to[36];

   strcpy(subject, curmsg->mis.subj);
   if(subject[0] == '\0')
     SumAttachesRequests(&curmsg->mis, subject, 70, SARboth);

   if(curmsg->mis.destinter[0] != '\0')
     strncpy(to, curmsg->mis.destinter, 35);
   else
     strncpy(to, curmsg->mis.to, 35);
   to[35] = '\0';

   sprintf(temp, "Area: %-15.15s To: %-0.36s (%-0.70s)", area->tag, to, subject);
   temp[maxx-1]='\0';
   printeol(0,0,cfg.col[Cmsgbar],temp);

}

// ==============================================================

void WriteToFile(int raw, int block, AREA *area, MSG *areahandle, MMSG *curmsg)
{
  FILE *out;
  BOX *getnamebox, *inputbox;
  char filename[80];
  struct stat mystat;
  char openmode[10];
  LINE *currentline;
  int retval;
  char *WhatToOpen;

  if(block)
    {
    if(bbegin == NULL)
       return;
    if(!bend) bend = bbegin;
    }

  leave_line(0);
  enter_line();

  getnamebox = initbox(9, 2, 14, 77, cfg.col[Cpopframe], cfg.col[Cpoptext], SINGLE, YES, ' ');
  drawbox(getnamebox);
  boxwrite(getnamebox,0,1,"Give file to write to: ");

  memset(filename, '\0', sizeof(filename));
  retval=getstring(12, 4, filename, 72, 72, "",cfg.col[Centry], cfg.col[Cpoptext]);
  delbox(getnamebox);

  if(retval == ESC)
    return;

  WhatToOpen = BuildCommandLine(filename, area, areahandle, curmsg, NULL, NULL);

  if(WhatToOpen == NULL)
     return;

  if(raw)
     strcpy(openmode, "wb");
  else
     strcpy(openmode, "wt");

  if(stat(WhatToOpen, &mystat) != -1)
    {
    inputbox = initbox(10,15,18,65,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
    drawbox(inputbox);
    boxwrite(inputbox,1,4,"File already exists! Append?");
    boxwrite(inputbox,3,4,"[Y]es : Append to file (or press <ENTER>)");
    boxwrite(inputbox,4,4,"[N]o  : Overwrite file.");
    boxwrite(inputbox,5,4,"<ESC> : Argh! Dunno, quit!");

    do
       {
       c = get_idle_key(1, GLOBALSCOPE);
       }
    while (c != 13 && c !='y' && c != 'Y' && c != 'n' && c != 'N' && c != 27);

    delbox(inputbox);

    if (c == 27) return;

    if (c == 'y' || c == 'Y')
      {
      if(raw)
        strcpy(openmode, "ab");
      else
        strcpy(openmode, "at");
      }
    }

  out = fopen(WhatToOpen, openmode);
  if(out == NULL)
    {
    sprintf(msg, "Error opening %s!", WhatToOpen);
    Message(msg, -1, 0, YES);
    return;
    }
  setvbuf(out, NULL, _IOFBF, 2048);

  if(block)
    currentline = bbegin;
  else
    currentline = first;

  while(currentline)
    {
    if(currentline->ls)
      fprintf(out, "%s", currentline->ls);

    if(!raw)
      fprintf(out, "\n");
    else
      {
      if(currentline->status & HCR)
        fprintf(out, "\r");
      }

    if(block && currentline == bend)
      break;
    currentline = currentline->next;
    }


  fclose(out);
  mem_free(WhatToOpen);

}

// ==============================================================

void SetEditCursor(void)
{

 if(insert)
     _settextcursor(0x0407);
 else
     _settextcursor(0x0607);

}
