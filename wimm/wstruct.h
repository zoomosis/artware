
#define PASSTHRU  0x01
#define SCANNED   0x02
#define EXCLUDE   0x04
#define FORCED    0x08

typedef struct arearec

{

        char    tag[60];        /* Official area tag                 */
        char    dir[80];        /* Directory/Base name               */
        int     base;           /* MSGTYPE_SDM or MSGTYPE_SQUISH     */
        char    status;         /* 0 = normal, 1 = passthru          */
        struct arearec  *next;  /* Pointer to next area              */

}	AREA;


typedef struct _namelist

{

        char             name[36];
        long             hash;
        struct _namelist *next;

}       NAMELIST;


typedef struct _arealist

{

        char             tag[60];
        struct _arealist *next;

}       AREALIST;


typedef struct _msglist
  {

  UMSGID             id;
  struct _msglist    *next;

  }      MSGLIST;



typedef struct _persmsglist

{
        char                    tag[60];
        char                    from[36];
        char                    subject[80];
        long                    number;
        struct _persmsglist     *next;

}       PERSMSGLIST;


#define LIST      1
#define MOVE      2
#define COPY      3

#define LASTREAD  1
#define ALL       2

/* Some prototypes for screen output */

void areastatus(char *area, char scroll);
void what_act(char *desc);
void message(char *desc);
void getout(char *error);
