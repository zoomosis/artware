#include "includes.h"

// Defines for SCOPE

#define sALLAREAS     0x0001
#define sTAGGEDAREAS  0x0002
#define sCURAREA      0x0004
#define sALLMSGS      0x0008
#define sNEWMSGS      0x0010

// Defines for 'where'

#define wFROM    0x0001
#define wTO      0x0002
#define wSUBJECT 0x0004
#define wBODY    0x0008
#define wKLUDGES 0x0010
#define wORIGIN  0x0020

// Defines for 'options'

#define oWHOLE   0x0001
#define oCASE    0x0002
#define oNOT     0x0004

// Defines for direction

#define dFORWARD  0x0001
#define dBACKWARD 0x0002

// Defines for action

#define saREAD    0x0001
#define saDELETE  0x0002
#define saWRITE   0x0004
#define saCOPY    0x0008
#define saMOVE    0x0010
#define saMARK    0x0020

#define saALL (saREAD|saDELETE|saWRITE|saCOPY|saMOVE|saMARK)

#define LEGAL " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-_=+\\|][{}'\";:/?.>,<"

typedef struct _searcharg
{
   bmgARG              pattern;
   char                string[21];
   word                options;
   word                where;
   struct _searcharg * and;
   struct _searcharg * or;

} SEARCHARG;

typedef struct _diskarg
{

   char string[21];
   word options;
   word where;
   char next;

} DISKARG;

// Defines for 'next' above

#define LAST    0xFF
#define NEXTAND 0x01
#define NEXTOR  0x02


typedef struct
{

   word        scope;
   word        direction;
   word        action;
   char        target[120];
   FILE      * targetFILE;
   AREA      * targetarea;
   MSG       * targethandle;
   SEARCHARG * first;

} SEARCHBLOCK;


typedef struct _diskbase
{

   char sig[4];
   char version;
   char deleted;
   char thisareatag[60];
   word scope;
   word direction;
   word action;
   char target[120];
   char targetareaname[60];
   char hasargs;

} DISKBASE;

#define FDBVERSION 1

typedef struct
{

   dword crc;        // CRC on areatag
   long  offset;     // Offset of 'base record' for this search stuff.

}  FINDIDX;

static SEARCHBLOCK base;
static dword totmsgs, totfound;

static int idx = -1,
           dat = -1;


void   ShowSearchOptions  (int output);
int    ShowSearchStrings  (AREA *curarea);
char * MakeSearchString   (SEARCHARG * thisone);
char * MakeOptions        (SEARCHARG * thisone);
void   EditSearchArgument (SEARCHARG * highlighted);
char * FullOptions        (SEARCHARG * thisone);
void   EditElement        (SEARCHARG * el, int curline, int topline);
void   GetOptions         (SEARCHARG * thisone, int topline);
void   FreeChain          (SEARCHARG * first);
void   GlobalOptions      (void);

void   DoSearch           (MSG *starthandle, AREA *startarea, long startlast);
void   MakeBMGTables      (void);
int    search_area        (MSG *areahandle, AREA *area);
AREA * GetNextArea        (AREA *area);
dword  GetNextMsg         (dword curno, AREA *area, MSG *areahandle);

int    check_msg          (MMSG *thismsg);
int    check_arg          (MMSG *thismsg, SEARCHARG *arg);

int    searchitem         (char *txt, int len, SEARCHARG *arg, char howstrict, char complete);
dword  HandleMessage      (dword curno, MSG *areahandle, AREA *area, MMSG *thismsg, dword thisuid);
void   bottom             (char *s);

void   CopyMoveMsg        (AREA *area, MSG *areahandle, MMSG *curmsg, int kill, dword msgno, dword thisuid);
void   MarkLines          (MMSG *curmsg);
void   check_line         (LINE *line);

void   SaveCurrent        (char *areatag);
void   ReadCurrent        (char *areatag);
long   GetOffset          (dword crc, long *idxoffset, char *areatag);
void   FreeAllMem         (void);
int    OpenFindDatabase   (int write);
void   CloseFindDatabase  (void);

// defines for 'howstrict' above..

#define ALWAYS    0x00
#define NOTFIRST  0x01
#define NOTLAST   0x02

// =======================================================
// Main findmessage function.

void FindMessage(MSG *areahandle, AREA *area, long last)
{
   BOX *baseframe;
   char temp[MAX_SCREEN_WIDTH];
   static char firsttime = 1;

   last = MsgMsgnToUid(areahandle, last);

   // Set default values for search.
   if(firsttime)
      {
      memset(&base, '\0', sizeof(SEARCHBLOCK));
      base.scope     = 0 | (sCURAREA | sNEWMSGS);
      base.direction = dFORWARD;
      base.action    = saREAD;
      firsttime      = 0;
      ReadCurrent("default");    // Get default profile (if it exists).
      }

   baseframe = initbox(0, 0, maxy-3, 79, cfg.col[Cfindactive], cfg.col[Cfindtext], DOUBLE, NO, ' ');
   drawbox(baseframe);
   delbox(baseframe);

   biprint(1, 21, cfg.col[Cfindtext], cfg.col[Cfindaccent], " ~timEd~ \07 Search configuration manager ", '~');

   memset(temp, 'Ä', 80);
   temp[80]='\0';
   printn(maxy-2,0,cfg.col[Cfindpassive],temp,80);


   temp[78] = '\0';
   printn(2,1,cfg.col[Cfindpassive],temp,78);
   memcpy(temp+29, "[                 ]",19);
   printn(maxy-6,1,cfg.col[Cfindpassive],temp,78);
   print(maxy-6,31,cfg.col[Cfindtext]," Search options: ");

   ReadCurrent(area->tag);

   ShowSearchOptions(0);

   if(base.first == NULL)   // No items defined (yet), stuff <INS>.
      stuffkey(338);

   while(ShowSearchStrings(area) != ESC)
     {
     baseframe = initbox(0, 0, maxy-3, 79, cfg.col[Cfindpassive], cfg.col[Cfindtext], DOUBLE, YES, -1);
     drawbox(baseframe);

     DoSearch(areahandle, area, last);

     delbox(baseframe);
     ShowSearchOptions(0);
     }

}


// =======================================================
// Show the global search option at the bottom of the screen

void ShowSearchOptions(int output)
{
   char  areas[10],
         msgs[10],
         direction[12];
   char temp[MAX_SCREEN_WIDTH];

   if(base.scope & sALLAREAS)
     strcpy(areas, "All    ");
   else if(base.scope & sTAGGEDAREAS)
     strcpy(areas, "Tagged ");
   else if(base.scope & sCURAREA)
     strcpy(areas, "Current");

   if(base.scope & sALLMSGS)
     strcpy(msgs, "All");
   else if(base.scope & sNEWMSGS)
     strcpy(msgs, "New");

   if(base.direction & dFORWARD)
     strcpy(direction, "Forward ");
   else
     strcpy(direction, "Backward");

   if(base.action & saREAD)
      sprintf(temp, "%-60.60s", "Read");
   if(base.action & saMARK)
      sprintf(temp, "%-60.60s", "Mark");
   else if(base.action & saDELETE)
      sprintf(temp, "%-60.60s", "Delete");
   else if(base.action & saCOPY)
      sprintf(temp, "Copy to %-52.52s", base.targetarea->tag);
   else if(base.action & saMOVE)
      sprintf(temp, "Move to %-52.52s", base.targetarea->tag);
   else if(base.action & saWRITE)
      sprintf(temp, "Write to %-51.51s", base.target);


   if(output == 0)
     {
     printn(maxy-5, 1, cfg.col[Cfindtext], " Areas :                 Messages:                 Direction:                 ", 78);

     print(maxy-5, 10, cfg.col[Cfinddata], areas);
     print(maxy-5, 36, cfg.col[Cfinddata], msgs);
     print(maxy-5, 63, cfg.col[Cfinddata], direction);

     printn(maxy-4, 1, cfg.col[Cfindtext], " Action:", 8);
     print(maxy-4, 10, cfg.col[Cfinddata], temp);
     }
   else
     {
     print(output+1, 33, cfg.col[Cfinddata], areas);
     print(output+2, 33, cfg.col[Cfinddata], msgs);
     print(output+3, 33, cfg.col[Cfinddata], direction);
     temp[25] = '\0';
     print(output+4, 33, cfg.col[Cfinddata], temp);
     }

}


// =======================================================
// Show a list of already defined search strings.

int ShowSearchStrings(AREA *curarea)
{
   SEARCHARG *thisone, *highlighted=base.first;
   int howmany = 0,
       start   = 0,
       curpos  = 0,
       rows,
       colour;
   char temp[MAX_SCREEN_WIDTH];
   int i;
   BOX *baseframe;

   start:

   for(thisone = base.first, howmany=0; thisone; thisone = thisone->or)
       howmany++;

   rows = howmany > (maxy-9) ? maxy-9 : howmany;

   if(rows<(maxy-9))
     {
     memset(temp, ' ', 78);
     printn(3+rows, 1, cfg.col[Cfindtext], temp, 78);
     }

   while(1)
     {
     bottom(" ~ENTER~ = edit   ~INS~ = add item   ~ALT-O~ = search Options   ~ctrl-ENTER~ = search");

     if(howmany > 0)
       {
       // find first to display
       for(i=0, thisone=base.first; i<start; i++, thisone=thisone->or)
         /* nothing */ ;

       // Adjust for impossible values
       if(curpos > (howmany-1)) curpos = howmany-1;

       for(i=0; i<rows; i++, thisone=thisone->or)
         {
         if(i+start == curpos)
           {
           highlighted = thisone;
           colour = cfg.col[Cfindhigh];
           MoveXY(2, 3+i+1);
           }
         else
           colour = cfg.col[Cfindtext];

         sprintf(temp, "%-78.78s", MakeSearchString(thisone));
         printn(3+i, 1, colour, temp, 78);
         }
       }

     switch(get_idle_key(1, GLOBALSCOPE))
        {
        case 275:       // ALT - R
           FreeAllMem();
           if(confirm("Read default profile? (y/N)") != 0)
             ReadCurrent("default");
           ClsRectWith(3,1,maxy-7,78,cfg.col[Cfindtext],' ');
           ShowSearchOptions(0);
           goto start;

        case 336:      /* down */
            if(howmany == 0) break;
            if(curpos < (howmany-1))
               curpos++;
            if(curpos > (start+rows-1))
               start++;
            break;

          case 328:      /* up */
            if(howmany == 0) break;
            if(curpos)
               curpos--;
            if(curpos<start) start--;
            break;

          case 327:      /* home */
            curpos=0; start=0;
            break;

          case 335:      /* end */
            if(howmany == 0) break;
            curpos = howmany-1;
            start  = howmany-rows;
            break;

          case 329:        /* Page Up */
            if(howmany == 0) break;
            curpos -= rows;
            if(curpos < 0)
               curpos = 0;
            start -= rows;
            if(start < 0) start=0;
            if(curpos < start)
               start = curpos;
            break;

          case 337:       /* Page Down */
            if(howmany == 0) break;
            curpos += rows;
            if(curpos > (howmany-1))
               curpos = howmany-1;
            start += rows;
            if(start > howmany-rows)
               start = howmany-rows;
            break;

          case 287:        /* ALT-S */
           SaveCurrent(curarea->tag);
           break;

          case 288:        /* ALT-D */
           SaveCurrent("default");
           break;

          case 13:        /* enter */

            if(howmany == 0) break;
            baseframe = initbox(0, 0, maxy-3, 79, cfg.col[Cfindpassive], cfg.col[Cfindtext], DOUBLE, YES, -1);
            drawbox(baseframe);
            sprintf(temp, "%-78.78s", MakeSearchString(highlighted));
            printn(3+curpos-start, 1, cfg.col[Cfindtext], temp, 78);
            EditSearchArgument(highlighted);
            delbox(baseframe);
            goto start;

          case 280:        /* ALT - O */
             baseframe = initbox(0, 0, maxy-3, 79, cfg.col[Cfindpassive], cfg.col[Cfindtext], DOUBLE, YES, -1);
             drawbox(baseframe);
             if(howmany > 0)
                {
                sprintf(temp, "%-78.78s", MakeSearchString(highlighted));
                printn(3+curpos-start, 1, cfg.col[Cfindtext], temp, 78);
                }
             GlobalOptions();
             delbox(baseframe);
             ShowSearchOptions(0);
             break;

          case 315:        /* F1 */
             show_help(3);
             break;

          case 338:        /* Insert */

              howmany++;
              for(thisone=base.first; thisone && thisone->or != NULL; thisone=thisone->or)
                /* nothing */ ;
              if(thisone)
                 thisone->or = mem_calloc(1, sizeof(SEARCHARG));
              else
                 base.first = mem_calloc(1, sizeof(SEARCHARG));

              stuffkey(13);            // <enter> - edit immediately
              stuffkey(13);            // <enter> - edit immediately
              stuffkey(335);           //  But goto bottom (new el) first!
              goto start;


          case 339:        /* Delete */
             if(howmany == 0) break;
             if(highlighted == base.first)
               {
               // Shit :-)
               //rows--;
               //howmany--;
               base.first = highlighted->or;
               FreeChain(highlighted);
               }
             else    // highlighted != base.first
               {
               for(thisone=base.first; thisone && thisone->or != highlighted; thisone=thisone->or)
                  /* nothing */ ;

               if(thisone)   // Found
                 {
                 thisone->or = highlighted->or;
                 FreeChain(highlighted);
                 // rows--;
                 howmany--;
                 if(curpos>howmany-1)     // Don't point to unexisting..
                    curpos--;

                 if(howmany >= (maxy-9))  // If more than one screen full..
                   {
                   if(start && (start > (howmany-maxy-9))) // Last line still filled?
                      start--;
                   if(curpos >= (start+rows)) curpos--;
                   }
                 }
               else
                 Message("Thisone not found in delete (main)!", -1, 0, YES);
               }

             // Clean last line, there will be on less now (if screen not full)
             if(howmany < (maxy-9))
                vprint(3+howmany, 1, cfg.col[Cfindtext], "%-78.78s", "");
             goto start;


          case 27:         /* esc */
            return ESC;

          case 10:
            return 0;

          default:
             break;
          }
     }
}


// =======================================================
// Create a string out of the definition of one 'OR' search argument

char * MakeSearchString(SEARCHARG *thisone)
{
   static char temp[MAX_SCREEN_WIDTH];
   int n=0;
   char tmpstring[40];

   memset(temp, '\0', sizeof(temp));
   for(; thisone && (strlen(temp)<78); thisone=thisone->and)
     {
     n++;
     if(n == 1)
       strcat(temp, " ");
     else
       if(n<4) strcat(temp, " & ");

     if(n<4)
       {
       sprintf(tmpstring, "%-11.11s (%s)", thisone->string, FullOptions(thisone));
       strcat(temp, tmpstring);
       }
     }

   if(n>3)
     strcat(temp, " >");

   return temp;
}


// =======================================================
// Create a string out of options & 'where' arguments (like 'FT--B-C').

char * FullOptions(SEARCHARG * thisone)
{
   static char temp[15];

   strcpy(temp, "----- ---");

   if(thisone->where & wBODY)
      temp[0]='B';
   if(thisone->where & wFROM)
      temp[1]='F';
   if(thisone->where & wTO)
      temp[2]='T';
   if(thisone->where & wSUBJECT)
      temp[3]='S';
   if(thisone->where & wKLUDGES)
      temp[4]='K';
//   if(thisone->where & wORIGIN)
//      temp[5]='O';

   if(thisone->options & oWHOLE)
      temp[6]='W';
   if(thisone->options & oCASE)
      temp[7]='C';
   if(thisone->options & oNOT)
      temp[8]='!';

   return temp;

}

// =======================================================
// Edit one certain AND AND argument

void  EditSearchArgument(SEARCHARG * first)
{
   SEARCHARG *thisone, *highlighted=first, *mainline;
   int howmany = 0,
       start   = 0,
       curpos  = 0,
       rows,
       colour;
   char temp[MAX_SCREEN_WIDTH];
   int i;
   BOX *elbox;
   int topline = maxy/2-8;

   elbox = initbox(topline, 21, topline+14, 57, cfg.col[Cfindactive], cfg.col[Cfindtext], SINGLE, YES, ' ');
   drawbox(elbox);

   print(topline+1, 22, cfg.col[Cfindtext], " String:                Options:");
   memset(temp, 'Ä', 35);
   temp[35]='\0';
   print(topline+2, 22, cfg.col[Cfindpassive], temp);

   for(thisone = first; thisone; thisone = thisone->and)
       howmany++;

   rows = howmany;

   while(1)
     {
     bottom(" ~ENTER~: edit item            ~INS~: add item            ~DEL~: delete item");
     if(howmany > 0)
       {
       // find first to display
       thisone=first;

       for(i=0; i<rows; i++, thisone=thisone->and)
         {
         if(i+start == curpos)
           {
           highlighted = thisone;
           colour = cfg.col[Cfindhigh];
           MoveXY(23, topline+3+i+1);
           }
         else
           colour = cfg.col[Cfindtext];

         sprintf(temp, " %-20.20s   %s ", thisone->string, FullOptions(thisone));
         printn(topline+3+i, 22, colour, temp, 35);
         }
       }

     switch(get_idle_key(1, GLOBALSCOPE))
        {
        case 336:      /* down */
            if(howmany == 0) break;
            if(curpos < (howmany-1))
               curpos++;
            if(curpos > (start+rows-1))
               start++;
            break;

          case 328:      /* up */
            if(howmany == 0) break;
            if(curpos)
               curpos--;
            if(curpos<start) start--;
            break;

          case 327:      /* home */
            curpos=0; start=0;
            break;

          case 335:      /* end */
            if(howmany == 0) break;
            curpos = howmany-1;
            start  = howmany-rows;
            break;

          case 329:        /* Page Up */
            if(howmany == 0) break;
            curpos -= rows;
            if(curpos < 0)
               curpos = 0;
            start -= rows;
            if(start < 0) start=0;
            if(curpos < start)
               start = curpos;
            break;

          case 337:       /* Page Down */
            if(howmany == 0) break;
            curpos += rows;
            if(curpos > (howmany-1))
               curpos = howmany-1;
            start += rows;
            if(start > howmany-rows)
               start = howmany-rows;
            break;

          case 13:        /* enter */
            if(howmany == 0) break;
            EditElement(highlighted, topline+3+curpos, topline);
            if(highlighted->string[0] == '\0')
              stuffkey(339);
            break;

          case 338:        /* Insert */
            if(howmany < 10)
              {
              howmany++;
              rows++;
              curpos=howmany-1;
              for(thisone=first; thisone && thisone->and != NULL; thisone=thisone->and)
                /* nothing */ ;
              if(!thisone)
                {
                howmany--;
                rows--;
                Message("Ugh! Thisone == NULL!", -1, 0, YES);
                }
              else
                thisone->and = mem_calloc(1, sizeof(SEARCHARG));
              stuffkey(13);
              }
            break;

          case 339:        /* Delete */
            if(howmany == 0) break;
             if(highlighted == first)
               {
               // Shit :-)
               if(highlighted == base.first)
                 {
                 if(highlighted->and)
                   {
                   base.first = highlighted->and;
                   highlighted->and->or = highlighted->or;
                   rows--;
                   howmany--;
                   first = highlighted->and;
                   mem_free(highlighted);
                   }
                 else
                   {
                   base.first = highlighted->or;
                   mem_free(highlighted);
                   delbox(elbox);
                   return;
                   }
                 }
               else  // Not first in 'main line'.
                 {
                 for(mainline=base.first; mainline && mainline->or != highlighted; mainline=mainline->or)
                    /* nothing */ ;
                 if(mainline)
                   {
                   if(highlighted->and)
                     {
                     mainline->or = highlighted->and;
                     highlighted->and->or = highlighted->or;
                     mem_free(highlighted);
                     if(curpos) curpos--;
                     rows--;
                     howmany--;
                     first = highlighted->and;
                     }
                   else
                     {
                     mainline->or = highlighted->or;
                     mem_free(highlighted);
                     delbox(elbox);
                     return;
                     }
                   }
                 else
                   Message("Mainline == NULL!", -1, 0, YES);
                 }
               }
             else    // highlighted != first
               {
               for(thisone=first; thisone && thisone->and != highlighted; thisone=thisone->and)
                  /* nothing */ ;
               if(thisone)   // Found
                 {
                 thisone->and = highlighted->and;
                 mem_free(highlighted);
                 rows--;
                 howmany--;
                 if(curpos>howmany-1)
                    curpos--;
                 }
               else
                 Message("Thisone not found in delete!", -1, 0, YES);
               }

             // Clean last line, there will be on less now
             vprint(topline+3+howmany, 22, cfg.col[Cfindtext], "%-35.35s", "");
             break;

          case 10:         /* ctrl-enter */
            stuffkey(10);
            stuffkey(27);
            break;

          case 27:         /* esc */
            delbox(elbox);
            return;

          default:
             break;
          }
     }

}

// =======================================================
// Edit one element (<string> <where> <options>).

void EditElement(SEARCHARG *el, int curline, int topline)
{
   int ret;
   BOX *elbox;
   char temp[MAX_SCREEN_WIDTH];

   sprintf(temp, " %-20.20s   %s  ", el->string, FullOptions(el));
   printn(curline, 22, cfg.col[Cfindtext], temp, 35);

   do {
      bottom("Enter string to search for.");
      ret = getstring(curline,23,el->string,20,20,"",cfg.col[Centry], cfg.col[Cfindtext]);
   } while(ret != 0 && ret != ESC);

//   print(curline, 23, cfg.col[Cfindtext], el->string);

   if(ret != ESC)
      {
      elbox = initbox(topline, 21, topline+14, 57, cfg.col[Cfindpassive], cfg.col[Cfindtext], SINGLE, YES, -1);
      drawbox(elbox);
      GetOptions(el, topline);
      delbox(elbox);
      }

}

// =======================================================

void GetOptions(SEARCHARG *thisone, int topline)
{
   BOX *obox;
   int howmany;

   static char *options[] =
     {
     "   Search body           ",
     "   Search from: field    ",
     "   Search to: field      ",
     "   Search subject        ",
     "   Search kludges        ",
//     "   Search Origin line    ",
     "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ",
     "   Whole words only      ",
     "   Case sensitive search ",
     "   Should NOT be present "
     };

   static char *helptext[] =
     {
     " Look in the body of the message",
     " Look in the from: field of the message header",
     " Look in the to: field of the message header.",
     " Look in the subject of the message header.",
     " Look in the control kludges of the message.",
     "",
     " Matched string must be a word (separated by a space or punctuation character).",
     " The case (upper or lower case) of the search string is significant.",
     " The search string must NOT be present in the message."
     };


   int i, colour;
   int curpos=0;


   obox = initbox(topline+2, 27, topline+12, 53, cfg.col[Cfindactive], cfg.col[Cfindtext], SINGLE, YES, -1);
   drawbox(obox);

   while(1)
     {
     for(i=0; i<9; i++)
       {
       if(i != 5)
          options[i][1] = ' ';
       }

     if(thisone->where & wBODY)
        options[0][1]='û';
     if(thisone->where & wFROM)
        options[1][1]='û';
     if(thisone->where & wTO)
        options[2][1]='û';
     if(thisone->where & wSUBJECT)
        options[3][1]='û';
     if(thisone->where & wKLUDGES)
        options[4][1]='û';
//     if(thisone->where & wORIGIN)
//        options[5][1]='û';
     if(thisone->options & oWHOLE)
        options[6][1]='û';
     if(thisone->options & oCASE)
        options[7][1]='û';
     if(thisone->options & oNOT)
        options[8][1]='û';

     for(i=0; i<9; i++)
       {
       if(i == curpos)
         {
         colour = cfg.col[Cfindhigh];
         MoveXY(29, topline+3+i+1);
         }
       else
         colour = cfg.col[Cfindtext];
       print(topline+3+i, 28, colour, options[i]);
       }

     bottom(helptext[curpos]);

     switch(get_idle_key(1, GLOBALSCOPE))
       {
       case 336:      /* down */
           if(curpos < 8)
              curpos++;
           if(curpos == 5) curpos++;
           break;

         case 328:      /* up */
           if(curpos)
              curpos--;
           if(curpos == 5) curpos--;
           break;

         case 327:      /* home */
           curpos=0;
           break;

         case 335:      /* end */
           curpos = 8;
           break;

         case 13:        /* enter */
         case 32:        /* space */
           switch(curpos)
             {
             case 0:
                thisone->where   ^= wBODY;    break;
             case 1:
                thisone->where   ^= wFROM;    break;
             case 2:
                thisone->where   ^= wTO;      break;
             case 3:
                thisone->where   ^= wSUBJECT; break;
             case 4:
                thisone->where   ^= wKLUDGES; break;
//             case 5:
//                thisone->where   ^= wORIGIN;  break;
             case 6:
                thisone->options ^= oWHOLE;   break;
             case 7:
                thisone->options ^= oCASE;    break;
             case 8:
                howmany=0;
                if(thisone->where & wBODY)    howmany++;
                if(thisone->where & wFROM)    howmany++;
                if(thisone->where & wTO  )    howmany++;
                if(thisone->where & wSUBJECT) howmany++;
                if(thisone->where & wKLUDGES) howmany++;
                if((howmany>1) && (!(thisone->options & oNOT)))
                  Message("Can't combine NOT with more than one place to search (From, To etc.)", -1, 0, YES);
                else
                  thisone->options ^= oNOT;
                break;
             }
           break;

         case 27:         /* esc */
           if(thisone->where == 0)
             {
             sprintf(msg, "Please enter where to search for %s!", thisone->string);
             Message(msg, -1, 0, YES);
             }
           else
             {
             delbox(obox);
             return;
             }
           break;

         default:
            break;
       }

     }

}

// =======================================================
// Free a chain of SEARCHARG (and -> and -> and)

void FreeChain(SEARCHARG * first)
{
   SEARCHARG *thisone;

   while(first)
     {
     thisone = first->and;
     mem_free(first);
     first = thisone;
     }

}

// =======================================================

void GlobalOptions(void)
{
   BOX *obox, *passive;
   FILE *out;
   static char *GlobalOptions[] =
     {
     "Areas    ",
     "Messages ",
     "Direction",
     "Action   "
     };

   static char *GlobalOptionsHelp[] =
     {
     "Areas that should be searched.",
     "Messages that should be searched.",
     "Direction of the search.",
     "Action performed on found messages."
     };

   static char *Actions[] =
     {
     " ~R~ead          ",
     " ~M~ark          ",
     " ~D~elete        ",
     " ~W~rite to file ",
     " ~C~opy          ",
     " M~o~ve          ",
     ""
     };

   int i, colour, action;
   int curpos=0;
   int topline = maxy/2 -3;

   obox = initbox(topline, 21, topline+5, 59, cfg.col[Cfindactive], cfg.col[Cfindtext], SINGLE, YES, ' ');
   drawbox(obox);

   while(1)
     {
     for(i=0; i<4; i++)
       {
       if(i == curpos)
         {
         colour = cfg.col[Cfindhigh];
         MoveXY(24, topline+1+i+1);
         }
       else
         colour = cfg.col[Cfindtext];
       print(topline+1+i, 23, colour, GlobalOptions[i]);

       ShowSearchOptions(topline);
       }

     bottom(GlobalOptionsHelp[curpos]);

     switch(get_idle_key(1, GLOBALSCOPE))
       {
       case 336:      /* down */
           if(curpos < 3)
              curpos++;
           break;

         case 328:      /* up */
           if(curpos)
              curpos--;
           break;

         case 327:      /* home */
           curpos=0;
           break;

         case 335:      /* end */
           curpos = 3;
           break;

         case 13:        /* enter */
           switch(curpos)
             {
             case 0:
               if(base.scope & sALLAREAS)
                 {
                 base.scope ^= sALLAREAS;
                 base.scope |= sTAGGEDAREAS;
                 }
               else if(base.scope & sTAGGEDAREAS)
                 {
                 base.scope ^= sTAGGEDAREAS;
                 base.scope |= sCURAREA;
                 }
               else if(base.scope & sCURAREA)
                 {
                 base.scope ^= sCURAREA;
                 base.scope |= sALLAREAS;
                 }
                break;

             case 1:
                base.scope ^= (sALLMSGS | sNEWMSGS);
                break;

             case 2:
                base.direction ^= (dFORWARD | dBACKWARD);
                break;

             case 3:
                passive = initbox(topline, 21, topline+5, 59, cfg.col[Cfindpassive], cfg.col[Cfindtext], SINGLE, YES, -1);
                drawbox(passive);

                action = picklist(Actions, NULL, topline-1, 31, maxy-1, 79);
                if(action != -1)
                  {
                  // Clear action bits
                  base.action &= ~saALL;
                  switch(action)
                    {
                    case 0: base.action |= saREAD  ; break;
                    case 1: base.action |= saMARK  ; break;
                    case 2: base.action |= saDELETE; break;
                    case 3: base.action |= saWRITE ; break;
                    case 4: base.action |= saCOPY  ; break;
                    case 5: base.action |= saMOVE  ; break;
                    }

                  // Do we need an area as target for move/copy?
                  if(base.action & (saCOPY|saMOVE))
                    {
                    savescreen();
                    base.targetarea = SelectArea(cfg.first, 1, cfg.first);
                    putscreen();
                    if(base.targetarea == NULL)
                       {
                       // No area given, back to read. (NULL ptr otherwise!)
                       base.action &= ~(saREAD|saDELETE|saWRITE|saCOPY|saMOVE);
                       base.action |= saREAD;
                       }
                    }
                  // Filename to write to needed?
                  else if(base.action & saWRITE)
                    {
                    if((out=OpenExist(base.target, NULL, NULL, NULL)) != NULL)
                      fclose(out);
                    else
                      {
                      // No (valid) file given, back to read.
                      base.action &= ~(saREAD|saDELETE|saWRITE|saCOPY|saMOVE);
                      base.action |= saREAD;
                      }
                    }
                  }

                delbox(passive);

                break;
             }
           break;

         case 10:          /* ctrl - enter */
           stuffkey(10);
           stuffkey(27);
           break;

         case 27:         /* esc */

           delbox(obox);
           return;

         default:
            break;
       }

     }

}

// =======================================================

void DoSearch(MSG *starthandle, AREA *startarea, long startlast)
{
   MSG  *curhandle;
   AREA *curarea = NULL;
   BOX *statusbox;
   int retval = 0;
   dword totareas = 0L;

   totmsgs = 0;
   totfound = 0;

   MakeBMGTables();

   statusbox = initbox(maxy/2-5, 24, maxy/2+5, 56, cfg.col[Cfindactive], cfg.col[Cfindtext], SINGLE, NO, ' ');
   drawbox(statusbox);
   titlewin(statusbox, TCENTER, " Searching.. ", 0);
   boxwrite(statusbox, 0, 1, "Current area :");
   boxwrite(statusbox, 1, 1, "Current msg  :");
   boxwrite(statusbox, 2, 1, "Highest msg  :");
   boxwrite(statusbox, 3, 1, "Lowest  msg  :");
   boxwrite(statusbox, 4, 1, "Action       :");
   print(maxy/2+1, 25, cfg.col[Cfindpassive], "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");
   boxwrite(statusbox, 6, 1, "Total areas  :");
   boxwrite(statusbox, 7, 1, "Total msgs   :");
   boxwrite(statusbox, 8, 1, "Total found  :");
   delbox(statusbox);

   bottom("Press ~ESC~ to abort the search.");

   //  ===========  Open output file or output area  ==========

   if(base.action & saWRITE)
     {
     if((base.targetFILE=fopen(base.target, "a")) == NULL)
       {
       Message("Error opening output file!", -1, 0, YES);
       return;
       }
     setvbuf(base.targetFILE, NULL, _IOFBF, 4096);
     }
   else if(base.action & (saCOPY|saMOVE))
     {
     base.targethandle = MsgOpenArea(base.targetarea->dir,
                                     MSGAREA_CRIFNEC,
                                     base.targetarea->base);
     if(base.targethandle == NULL)
       {
       Message("Cannot open target area!", -1, 0, YES);
       showerror();
       return;
       }
     }

   // ======== Perform the actual search =========

   if(base.scope & sCURAREA)
     {
     UpdateLastread(startarea, startlast, startlast, starthandle);
     if((base.action & (saCOPY|saMOVE)) && (base.targetarea == startarea))
       {
       Message("Target area of Copy/Move is current area!", -1, 0, YES);
       }
     else
       retval = search_area(starthandle, startarea);
     }
   // Not current area, so it is ALL or TAGGED
   else while( (curarea = GetNextArea(curarea)) != NULL )
     {
     ShowSearchOptions(0);
     if((base.action & (saCOPY|saMOVE)) && (base.targetarea == curarea))
       {
          continue;     // Prevent loops and needless action.
       }
     else if(curarea == startarea)
        {
        curhandle = starthandle;
        }
     else if( (curhandle=MsgOpenArea(curarea->dir, MSGAREA_CRIFNEC, curarea->base)) == NULL)
       {
       sprintf(msg, "Error opening area! (%s)", curarea->tag);
       Message(msg, -1, 0, YES);
       showerror();
       continue;
       }

     retval = search_area(curhandle, curarea);

     vprint(maxy/2+2, 41, cfg.col[Cfinddata], "%-5lu     ", ++totareas);
     vprint(maxy/2+3, 41, cfg.col[Cfinddata], "%-5lu     ", totmsgs);

     if(curarea != startarea)
        MsgCloseArea(curhandle);

     if(retval == ESC)
       break;
     }

   // ================  Close the output file or area again  ============

   if(base.action & saWRITE)
     fclose(base.targetFILE);
   else if(base.action & (saCOPY|saMOVE))
     {
     ScanArea(base.targetarea, base.targethandle, 1);
     MsgCloseArea(base.targethandle);
     }

   if(retval == ESC)
     print(maxy/2, 41, cfg.col[Cfinddata], "Search aborted");
   else
     print(maxy/2, 41, cfg.col[Cfinddata], "Ready         ");

   bottom("Press any key to continue");

   get_idle_key(1, GLOBALSCOPE);

}


// ============================================================
// Compile all BMG tables to be used for Boyer-Moore searches..
// ============================================================

void MakeBMGTables(void)
{
   SEARCHARG *mainline, *andline;

   for(mainline = base.first; mainline; mainline = mainline->or)
     {
     for(andline = mainline; andline; andline = andline->and)
       {
       bmgCompile(andline->string,
                  &andline->pattern,
                  (andline->options & oCASE) ? 0 : 1,
                  (andline->options & oWHOLE) ? 1 : 0);
       }
     }
}

// ============================================================

int search_area(MSG *areahandle, AREA *area)
{
   dword curno = 0L;
   UMSGID thisuid;
   MMSG *thismsg;
   int retval=0, maysee, l;
   int topline = maxy/2-4;
   dword msgs=0;
   dword command;
   int key;

   ScanArea(area, areahandle, 1);

   vprint(topline, 41, cfg.col[Cfinddata], "%-14.14s", area->tag);
   MoveXY(42, topline+1);
   vprint(topline+2, 41, cfg.col[Cfinddata], "%-5lu     ", area->highest);
   vprint(topline+3, 41, cfg.col[Cfinddata], "%-5lu     ", area->lowest);

   while( (curno = GetNextMsg(curno, area, areahandle)) != 0)
    {
    if(xkbhit())
       {
       key=get_idle_key(0, GLOBALSCOPE);
       switch(key)
         {
         case 27:
           retval = ESC;
           break;

         case 43:   // '+'
           retval = NEXTAREA;
           base.direction = dFORWARD;
           break;

         case 45:   // '-'
           retval = NEXTAREA;
           base.direction = dBACKWARD;
           break;

         case 331:  // left
           base.direction = dBACKWARD;
           ShowSearchOptions(0);
           break;

         case 333:  // right
           base.direction = dFORWARD;
           ShowSearchOptions(0);
           break;

         default:
           break;
         }

       if(retval != 0)  // Break out if it was a relevant key.
          break;
       }

    msgs++;

    thisuid = MsgMsgnToUid(areahandle, curno);

    vprint(topline+1, 41, cfg.col[Cfinddata], "%-5lu     ", curno);
    printn(topline+4, 41, cfg.col[Cfinddata], "Scanning", 8);

    if( (thismsg = GetRawMsg(curno, areahandle, READALL, 1)) == NULL)
       {
       curno = thisuid;
       continue;
       }

    maysee = 1;   // Are we allowed to see this msg (private etc)? Start 'yes'.

    if(BePrivate(area) && (thismsg->mis.attr1 & aPRIVATE))
      {
      maysee = 0; // Private area, start with 'on', unless this is from/to us.

      for(l=0; (cfg.usr.name[l].name[0] != '\0') && (l<10); l++)
        {
        if(strcmpi(thismsg->mis.to, cfg.usr.name[l].name)==0)
          maysee = 1;
        else if(strcmpi(thismsg->mis.from, cfg.usr.name[l].name)==0)
          maysee = 1;
        }
      }

    if(maysee && check_msg(thismsg) == 1)
      {
      command = HandleMessage(curno, areahandle, area, thismsg, thisuid);
      ShowSearchOptions(0);  // direction might have changed
      }

    ReleaseMsg(thismsg, 1);

    curno = thisuid;

    switch(command)
      {
      case PREV:
        base.direction = dBACKWARD;
        break;

      case NEXT:
        base.direction = dFORWARD;
        break;

      case ESC:
        return ESC;

      case NEXTAREA:
        base.direction = dFORWARD;
        return NEXTAREA;

      case PREVAREA:
        base.direction = dBACKWARD;
        return NEXTAREA;

      default:
        break;
      }

    }

   totmsgs += msgs;

   ScanArea(area, areahandle, 1);

   return retval;

}


// =======================================================================
// Get next area to scan according to direction and scope (all <-> tagged)
// =======================================================================

AREA * GetNextArea(AREA *area)
{

   if(area == NULL)
     {
     area = cfg.first;                    // Set to first

     if(base.direction & dBACKWARD)
        while(area->next)                 // Find last area
          area = area->next;

     // If we need to scan all areas, this is always OK,
     // else we need a tagged one..

     if( (base.scope & sALLAREAS) || (area->tagged == 1) )
        return area;

     // Else we need tagged area, fall through to find it
     }
   else
     area = (base.direction & dFORWARD) ? area->next : area->prev;

   if(base.scope & sALLAREAS)
     return area;

   while(area && (area->tagged == 0))
     area = (base.direction & dFORWARD) ? area->next : area->prev;

   return area;

}

// ====================================================================
// Find next message to be scanned by search (forward <-> backward etc
// ====================================================================

dword GetNextMsg(dword curno, AREA *area, MSG *areahandle)
{
   word type;
   dword next;

   if(curno == 0L)
     {
     if(area->nomsgs == 0) return 0;

     if(base.direction & dBACKWARD)
        {
        curno = area->highest;
        if(base.scope & sALLMSGS)  // We scan all msgs, so return it.
           return curno;

        // Only new messages, so see if there's any new msgs...
        if(curno <= area->lr)
          return 0;    // Nope, nothing new, sorry..

        return curno;
        }
     else  // Looking forward direction..
        {
        if(base.scope & sALLMSGS)
           return area->lowest;

        // Only new messages. Let's see if there are any..

        if(area->lr < area->highest)
          return (area->lr+1);
        else
          return 0;
        }
     }

   curno = (base.direction & dFORWARD) ? curno + 1 : curno - 1;

   type  = (base.direction & dFORWARD) ? UID_NEXT : UID_PREV;

   next = MsgUidToMsgn(areahandle, curno, type);

   if( (base.scope & sNEWMSGS) && (next < area->lr) )
      return 0;

   return next;
}

// =================================================================
// Check a message, by firing off searches for separate arguments..
// =================================================================


int check_msg(MMSG *thismsg)
{
   SEARCHARG *curarg, *andarg;

   for(curarg = base.first; curarg; curarg = curarg->or)
     {
     for(andarg = curarg; andarg; andarg = andarg->and)
       {
       // This doesn't match? Next 'OR' argument then (outer loop).
       if(check_arg(thismsg, andarg) == 0)
          break;             // break out of inner loop.

       // Last 'and' argument? Then we have a full match!
       if(andarg->and == NULL)
          return 1;
       }
     }

   return 0;
}

// =================================================================
// Check a message, with a certain search argument
// =================================================================

int check_arg(MMSG *thismsg, SEARCHARG *arg)
{
   int retval;
   RAWBLOCK *thisblock;
   char tempbuffer[51];
   int copylen1, copylen2;
   char howstrict;

   // Check the from field.
   if(arg->where & wFROM)
     {
     if( (retval=searchitem(thismsg->mis.from, -1, arg, ALWAYS, 1)) != -1)
       return retval;
     }

   // Check the to field.
   if(arg->where & wTO)
     {
     if( (retval=searchitem(thismsg->mis.to, -1, arg, ALWAYS, 1)) != -1)
       return retval;
     }

   // Check the subject field.
   if(arg->where & wSUBJECT)
     {
     if( (retval=searchitem(thismsg->mis.subj, -1, arg, ALWAYS, 1)) != -1)
       return retval;
     }

   // Check the subject field.
   if(arg->where & wKLUDGES)
     {
     if( (retval=searchitem(thismsg->ctxt, -1, arg, ALWAYS, 1)) != -1)
       return retval;
     }

   if(arg->where & wBODY)
     {
     for(thisblock = thismsg->firstblock; thisblock; thisblock = thisblock->next)
       {
       howstrict = ALWAYS;

       if(arg->options & oWHOLE)
         {
         // If there's a previous block, no match if it is first word
         // (messes up checking for a 'whole word only').
         if(thisblock != thismsg->firstblock)
            howstrict |= NOTFIRST;

         if(thisblock->next)
            howstrict |= NOTLAST;
         }

       if( (retval=searchitem(thisblock->txt, thisblock->curlen, arg, howstrict, 0)) != -1)
         return retval;
       }

     if((thisblock=thismsg->firstblock) != NULL)  // If there are blocks at all
       {
       while(thisblock->next)    // Is there (still) a transition to a new block?
         {
         // Now we fill a buffer with last part or current block, and first
         // part of next block, to see if any matches can be found that are
         // crossing block borders.

         memset(tempbuffer, '\0', sizeof(tempbuffer));

         copylen1 = (thisblock->curlen < 25) ? thisblock->curlen : 25;
         if(copylen1)
            memcpy(tempbuffer, thisblock->txt + (thisblock->curlen-25), copylen1);

         copylen2 = (thisblock->next->curlen < 25) ? thisblock->next->curlen : 25;
         if(copylen2)
            memcpy(tempbuffer+copylen1, thisblock->next->txt, copylen2);

         howstrict = NOTFIRST | NOTLAST;
         if(thisblock->next->next == NULL)    // end of buffer
           {
           // Check if last block is in buffer entirely
           if(copylen2 == thisblock->next->curlen)
             {
             // In that case the match may be the end of buffer!
             howstrict &= ~NOTLAST;
             }
           }

         if( (retval=searchitem(tempbuffer, copylen1+copylen2, arg, howstrict, 0)) != -1)
           return retval;

         thisblock = thisblock->next;
         }
       }

     // If we get here, we did not find the string in any of the sub-strings.
     // If we have a 'NOT' search, that actually means we are successful!

     if(arg->options & oNOT)
        return 1;
     }

   return 0;
}



// =====================================================================
//
// returns:  1  ==  matched this item!
//           0  ==  found, but you wanted it NOT to be present
//          -1  ==  no match but this is not complete string, search on.
//
// =====================================================================


int searchitem(char *txt, int len, SEARCHARG *arg, char howstrict, char complete)
{

   if(txt == NULL)       // No text given.
     {
     if( (complete!=0) && (arg->options & oNOT) )  // Well, it's definitely not here.
        return 1;
     else
        return -1;
     }


   if(len == -1)
      len = strlen(txt);

  if(bmgSearch(txt,
               len,
               &arg->pattern,
               howstrict) != NULL)
    {
    // Found a hit!
    if(arg->options & oNOT)
       return 0;
    else
       return 1;
    }
  else if(complete)   // Not found! If complete this is the string entirely,
    {                 // and we can be sure it's NOT there. Not so if this is only a part..
    if(arg->options & oNOT)
      return 1;
    }

  return -1;
}

// =========================================================

dword HandleMessage(dword curno, MSG *areahandle, AREA *area, MMSG *thismsg, dword thisuid)
{
   int added=0;
   int topline = maxy/2-4;
   dword command;
   dword retval = 0L;


   vprint(maxy/2+4, 41, cfg.col[Cfinddata], "%-5lu", ++totfound);

   if(base.action & saREAD)    // ------------- Read ----------------
     {
     savescreen();
     ReleaseMsg(thismsg, 0);  // Don't remove base!! Second parm = 0!
     _heapmin();
     get_custom_info(area);
     if((thismsg = GetFmtMsg(curno, areahandle, area)) == NULL)
       {
       putscreen();
       return NEXT;
       }
     if(area->mlist == NULL)
        {
        added=1;
        area->mlist = InitMarkList();
        }

     MarkLines(thismsg);
     command = ShowMsg(thismsg, area, areahandle, SCAN_DISPLAY);
     switch(command)
       {
       case ESC     :
       case NEXT    :
       case PREV    :
       case NEXTAREA:
       case PREVAREA:
         retval = command;
         break;
       default:
         /* nothing */
         break;
       }

     ReleaseMsg(thismsg, 1);            // Dump formatted copy...

     if(area->mlist->active == 0 && added)
       {
       DumpMarkList(area->mlist);
       area->mlist = NULL;
       }
     putscreen();
     }

   else if(base.action & saMARK)  // ------------- Mark ----------------
     {
     printn(topline+4, 41, cfg.col[Cfinddata], "Marking ", 8);
     if(area->mlist == NULL)
        area->mlist = InitMarkList();

     AddMarkList(area->mlist, thisuid);
     }

   else if(base.action & saWRITE)  // ------------- Write ----------------
     {
     printn(topline+4, 41, cfg.col[Cfinddata], "Writing ", 8);

     ReleaseMsg(thismsg, 0);  // Don't remove base!! Second parm = 0!
     if((thismsg = GetFmtMsg(curno, areahandle, area)) == NULL)
       return NEXT;

     do_print(base.targetFILE, area, thismsg, PRINTALL);

     ReleaseMsg(thismsg, 1);            // Dump formatted copy...
     }

   else if(base.action & saCOPY)  // ------------- Copy ----------------
     {
     printn(topline+4, 41, cfg.col[Cfinddata], "Copying ", 8);
     CopyMoveMsg(area, areahandle, thismsg, 0, curno, thisuid);
     }

   else if(base.action & saMOVE)  // ------------- Move ----------------
     {
     printn(topline+4, 41, cfg.col[Cfinddata], "Moving  ", 8);
     CopyMoveMsg(area, areahandle, thismsg, 1, curno, thisuid);
     }

   else if(base.action & saDELETE)  // ------------- Del ----------------
     {
     printn(topline+4, 41, cfg.col[Cfinddata], "Deleting", 8);
     if(AttemptLock(areahandle) != -1)
       {
       if(MsgKillMsg(areahandle, curno) == -1)
          {
          Message("Error deleting message!", -1, 0, YES);
          showerror();
          }
       else
          {
          if(area->mlist)
             RemoveMarkList(area->mlist, thisuid);
          }
       ScanArea(area, areahandle, 0);
       MsgUnlock(areahandle);
       }
     }

   return retval;

}

// =========================================

void bottom(char *s)
{
   biprinteol(maxy-1, 0, cfg.col[Cfindtext], cfg.col[Cfindaccent], s, '~');
}

// ================================================

void CopyMoveMsg(AREA *area, MSG *areahandle, MMSG *curmsg, int kill, dword msgno, dword thisuid)
{

   memset(&curmsg->mis.replyto, '\0', sizeof(curmsg->mis.replyto));
   memset(&curmsg->mis.replies, '\0', 9 * sizeof(word));

   SaveMessage(base.targethandle,
               base.targetarea,
               &curmsg->mis,
               curmsg->firstblock,
               curmsg->ctxt,
               0,
               0);

   add_tosslog(base.targetarea, base.targethandle);

   if (kill)
      {
      if(AttemptLock(areahandle) != -1)
        {
        if(MsgKillMsg(areahandle, msgno) == -1)
           {
           Message("Error deleting message!", -1, 0, YES);
           showerror();
           }
        else
           if(area->mlist) RemoveMarkList(area->mlist, thisuid);
        ScanArea(area, areahandle, 1);
        MsgUnlock(areahandle);
        }
      }

}

//

void MarkLines(MMSG *curmsg)
{
   LINE *thisline;

   if(!(TIMREGISTERED))
     return;

   for(thisline=curmsg->txt; thisline; thisline=thisline->next)
      {
      check_line(thisline);
      }

}


// ============== Check a line for a matching search string ===========

void check_line(LINE *line)
{
   SEARCHARG *curarg, *andarg;

   if(line == NULL || line->ls == NULL) return;

   for(curarg = base.first; curarg; curarg = curarg->or)
     {
     for(andarg = curarg; andarg; andarg = andarg->and)
       {
       if(andarg->options & oNOT)
          continue;   // Can't highlight all lines with NOT something..

       if(!(andarg->where & (wBODY|wKLUDGES)))
          continue;   // Must be in body or kludges..

       if(bmgSearch(line->ls,
                    line->len,
                    &andarg->pattern,
                    ALWAYS) == NULL)
           continue;

//       Mustn't search for all args in 1 LINE, fool!
//       // Last 'and' argument? Then we have a full match!
//       if(andarg->and == NULL)
//          {
       line->status |= HIGHLIGHT;
       return;
//          }
       }
     }

}


// ===========================================================

void SaveCurrent(char *areatag)
{
   long dataoffset, idxoffset;
   char temp[120];
   DISKARG diskarg;
   DISKBASE diskbase;
   FINDIDX findidx;
   SEARCHARG *thisone, *lastmain;
   dword crc;

   strncpy(temp, areatag, 119);
   strupr(temp);
   crc = JAMsysCrc32(temp, strlen(temp), -1L);

   if(OpenFindDatabase(1) == -1)
     {
     Message("Error opening datafiles for write!", -1, 0, YES);
     return;
     }

   // Does it already exist?
   dataoffset = GetOffset(crc, &idxoffset, areatag);
   if(dataoffset == -1)  // Not found
     idxoffset  = lseek(idx, 0L, SEEK_END);
   else  // existing record
    {
    if(lseek(dat, dataoffset, SEEK_SET) != dataoffset)
      {
      Message("Error seeking datafile!", -1, 0, YES);
      goto Exit;
      }
    if(read(dat, &diskbase, sizeof(DISKBASE)) != sizeof(DISKBASE))
      {
      Message("Error reading datafile!", -1, 0, YES);
      goto Exit;
      }
    diskbase.deleted = 1;
    if(lseek(dat, dataoffset, SEEK_SET) != dataoffset)
      {
      Message("Error seeking datafile!", -1, 0, YES);
      goto Exit;
      }
    if(write(dat, &diskbase, sizeof(DISKBASE)) != sizeof(DISKBASE))
      {
      Message("Error writing datafile!", -1, 0, YES);
      goto Exit;
      }
    }

   dataoffset = lseek(dat, 0L, SEEK_END);

   findidx.offset = dataoffset;
   findidx.crc    = crc;
   if(lseek(idx, idxoffset, SEEK_SET) != idxoffset)
     {
     Message("Error seeking index!", -1, 0, YES);
     goto Exit;
     }
   if(write(idx, &findidx, sizeof(FINDIDX)) != sizeof(FINDIDX))
     {
     Message("Error writing index!", -1, 0, YES);
     goto Exit;
     }

   memset(&diskbase, '\0', sizeof(DISKBASE));

   strcpy(diskbase.sig, "Art");
   diskbase.version = FDBVERSION;
   strncpy(diskbase.thisareatag, areatag, 59);
   diskbase.scope = base.scope;
   diskbase.direction = base.direction;
   diskbase.action = base.action;
   strcpy(diskbase.target, base.target);
   if(base.targetarea != NULL)
     strncpy(diskbase.targetareaname, base.targetarea->tag, 59);
   diskbase.hasargs = (base.first == NULL) ? 0 : 1;

   if(lseek(dat, dataoffset, SEEK_SET) != dataoffset)
     {
     Message("Error seeking datafile!", -1, 0, YES);
     goto Exit;
     }
   if(write(dat, &diskbase, sizeof(DISKBASE)) != sizeof(DISKBASE))
     {
     Message("Error writing datafile!", -1, 0, YES);
     goto Exit;
     }

   thisone = lastmain = base.first;

   while(thisone)
     {
     memset(&diskarg, '\0', sizeof(DISKARG));
     strcpy(diskarg.string, thisone->string);
     diskarg.options = thisone->options;
     diskarg.where   = thisone->where;

     if(thisone->and)
        {
        diskarg.next = NEXTAND;
        thisone      = thisone->and;
        }
     else if(lastmain->or)
       {
       diskarg.next = NEXTOR;
       thisone      = lastmain->or;
       lastmain     = thisone;
       }
     else
      {
      diskarg.next = LAST;
      thisone      = NULL;
      }

     if(write(dat, &diskarg, sizeof(DISKARG)) != sizeof(DISKARG))
       {
       Message("Error writing find database!", -1, 0, YES);
       break;
       }
     }

   Message("Find configuration saved.", -1, 0, YES);

   Exit:

   CloseFindDatabase();

}

// ===========================================================

void ReadCurrent(char *areatag)
{
   dword crc;
   char temp[120];
   long offset;
   DISKBASE diskbase;
   DISKARG diskarg;
   SEARCHARG *thisone, *lastmain;

   strncpy(temp, areatag, 119);
   strupr(temp);
   crc = JAMsysCrc32(temp, strlen(temp), -1L);

   if(!(TIMREGISTERED) || OpenFindDatabase(0) == -1)
     return;

   if( (offset=GetOffset(crc, NULL, areatag)) == -1L)
     {
     CloseFindDatabase();
     return;                      // not found.
     }

   FreeAllMem();  // Dump any old stuff

   lseek(dat, offset, SEEK_SET);
   if(read(dat, &diskbase, sizeof(DISKBASE)) != sizeof(DISKBASE))
     {
     Message("Error reading find database!", -1, 0, YES);
     CloseFindDatabase();
     return;
     }

   base.scope = diskbase.scope;
   base.direction = diskbase.direction;
   base.action = diskbase.action;
   strcpy(base.target, diskbase.target);
   if(diskbase.targetareaname[0] != '\0')
     {
     base.targetarea = FindArea(diskbase.targetareaname);
     if(base.targetarea == NULL)
        base.action = saREAD;
     }

   if(diskbase.hasargs == 0)
     {
     CloseFindDatabase();
     return;
     }

   base.first = thisone = lastmain = mem_calloc(1, sizeof(SEARCHARG));

   do
     {
     if(read(dat, &diskarg, sizeof(DISKARG)) != sizeof(DISKARG))
       {
       Message("Error reading find database!", -1, 0, YES);
       break;
       }

     strcpy(thisone->string, diskarg.string);
     thisone->options = diskarg.options;
     thisone->where   = diskarg.where;

     switch(diskarg.next)
       {
       case NEXTAND:
         thisone->and = mem_calloc(1, sizeof(SEARCHARG));
         thisone = thisone->and;
         break;
       case NEXTOR:
         lastmain->or = mem_calloc(1, sizeof(SEARCHARG));
         thisone = lastmain->or;
         lastmain = thisone;
         break;
       case LAST:
         break;
       default:
         Message("Unknown 'next' field in find database!", -1, 0, YES);
         diskarg.next = LAST;
         break;
       }

     } while(diskarg.next != LAST);

   CloseFindDatabase();

}

// ============================================================

long GetOffset(dword crc, long *idxoffset, char *areatag)
{
   FINDIDX *index= NULL;
   long size;
   unsigned howmany;
   int i;
   DISKBASE diskbase;
   long found = -1L;

   if( (size = filelength(idx)) == 0L)
     goto ErrorExit;

   if((size % sizeof(FINDIDX)) != 0)
     {
     Message("Find Index corrupt!", -1, 0, YES);
     goto ErrorExit;
     }

   howmany = (unsigned) (size/sizeof(FINDIDX));

   index = mem_malloc((unsigned) size);

   if(read(idx, index, (unsigned) size) != (int) size)
     {
     Message("Error reading find index!", -1, 0, YES);
     goto ErrorExit;
     }

   for(i=0; i<howmany; i++)
     {
     if(index[i].crc == crc)
       {
       lseek(dat, index[i].offset, SEEK_SET);
       if(read(dat, &diskbase, sizeof(DISKBASE)) != sizeof(DISKBASE))
         {
         Message("Error reading find database!", -1, 0, YES);
         goto ErrorExit;
         }

       if(strcmp(diskbase.sig, "Art") != 0)
         {
         Message("Invalid signature in find database!", -1, 0, YES);
         goto ErrorExit;
         }

       if(diskbase.version != FDBVERSION)
         {
         Message("Invalid version number in find database!", -1, 0, YES);
         goto ErrorExit;
         }

       if(diskbase.deleted == 1) continue;

       if(strcmpi(diskbase.thisareatag, areatag) == 0)
         {
         found = index[i].offset;
         if(idxoffset != NULL)
           *idxoffset = (long) ((long) i * (long) sizeof(FINDIDX));
         break;
         }
       }
     }

   ErrorExit:

   if(index != NULL) mem_free(index);

   return found;

}

// ============================================================

void FreeAllMem(void)
{
   SEARCHARG *thisone = base.first, *next = NULL;

   while(thisone)
     {
     next = thisone->or;
     FreeChain(thisone);
     thisone = next;
     }

   memset(&base, '\0', sizeof(SEARCHBLOCK));
   base.scope     = 0 | (sCURAREA | sNEWMSGS);
   base.direction = dFORWARD;
   base.action    = saREAD;

}

// =============================================================

int OpenFindDatabase(int write)
{
   char temp[120];
   int access;
   int shareflags;

   if(idx != -1 || dat != -1) beep();

   if(write)
      {
      access     = O_BINARY | O_CREAT | O_RDWR;
      shareflags = SH_DENYWR;
      }
   else
      {
      access     = O_BINARY | O_RDONLY;
      shareflags = SH_DENYNO;
      }

   sprintf(temp, "%s\\fdb.idx", cfg.homedir);
   if((idx = sopen(temp, access, shareflags, S_IREAD|S_IWRITE)) == -1)
      return -1;

   sprintf(temp, "%s\\fdb.dat", cfg.homedir);
   if((dat = sopen(temp, access, shareflags, S_IREAD|S_IWRITE)) == -1)
     {
     close(idx);
     idx=-1;
     return -1;
     }

   return 0;

}

// ==========================================================

void CloseFindDatabase(void)
{

   if(idx != -1) close(idx); else beep();
   if(dat != -1) close(dat); else beep();
   dat = idx = -1;

}

