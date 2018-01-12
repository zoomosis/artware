
#ifdef __OS2__
//   #define far
#endif

/* #define cls() bios_scroll_up(0,0,0,maxy-1,maxx-1,0) */

#define SINGLE 0
#define DOUBLE 1
#define S_HOR  2
#define S_VERT 3

#define TLEFT   0
#define TRIGHT  1
#define TCENTER 2

typedef int YesNo;
#define YES 1
#define NO  0


typedef struct
   {
   int   x1, y1, x2, y2;         /* Corners */
   int   ColBorder, ColInside;   /* Color.. */
   int   borderstyle;
   char * blockptr;              /* Pointer to where screen is saved */
   char  fill;                   /* The character to fill the screen */
   YesNo save;
   }  BOX;


BOX *initbox(int x1, int y1, int x2, int y2, int border, int inside, int bstyle, YesNo save, char fill);
void drawbox(BOX *data);
void delbox(BOX *data);
void error(char message[80]);
void boxwrite(BOX *box, int x, int y, char *line);
void savescreen(void);
void putscreen(void);
void bios_scroll_up(int count, int sr, int sc, int er, int ec, int attr);
void restoreblock(int x, int y, int w, int h, char * b);
void saveblock(int x1, int y1, int x2, int y2, char * b);
void titlewin(BOX *win, int where, char *s, int hlcol);

