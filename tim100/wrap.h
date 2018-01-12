
typedef struct linelist				/* Used to store "wrapped" lines */
	{
   char            *ls;                    /* line start */
   char            status;     /* what kind of line? (see below) */
   char            len;
	struct linelist	*next;		/* next line */
	struct linelist	*prev;
	} LINE;


/* defines for status of a line */

#define    NORMTXT    0x01
#define    QUOTE      0x02
#define    ORIGIN     0x04
#define    KLUDGE     0x08
#define    TEAR       0x10
#define    WITHTAB    0x20
#define    HCR        0x40
#define    HIGHLIGHT  0x80

LINE *FormatText(char *txt, int ll);
LINE *fastFormatText(char *txt, int column);

