#include "includes.h"

typedef struct
{
   long bit;
   char txt[5];

} BITTXT;

#define NUMCONVS1 31

BITTXT Attr2Text1[] = {

{ aPRIVATE, "pvt "},
{ aCRASH  , "cra "},
{ aREAD   , "rcv "},
{ aSENT   , "snt "},
{ aFILE   , "f/a "},
{ aFWD    , "fwd "},
{ aORPHAN , "orp "},
{ aKILL   , "kil "},
{ aLOCAL  , "loc "},
{ aHOLD   , "hld "},
{ aXX2    , "xx2 "},
{ aFRQ    , "frq "},
{ aRRQ    , "rrq "},
{ aCPT    , "cpt "},
{ aARQ    , "arq "},
{ aURQ    , "urq "},
{ aSCANNED, "scn "},
{ aDIR    , "dir "},
{ aAS     , "a/s "},
{ aIMM    , "imm "},
{ aKFS    , "kfs "},
{ aTFS    , "tfs "},
{ aCFM    , "cfm "},
{ aLOK    , "lok "},
{ aZGT    , "zgt "},
{ aENC    , "enc "},
{ aCOM    , "com "},
{ aESC    , "esc "},
{ aFPU    , "fpu "},
{ aNODISP , "nod "},
{ aDEL    , "del "}

};


#define NUMCONVS2 7

BITTXT Attr2Text2[] = {

{ aHUB, "hub "},
{ aXMA, "xma "},
{ aHIR, "hir "},
{ aCOV, "cov "},
{ aSIG, "sig "},
{ aLET, "let "},
{ aFAX, "fax "}

};

void MakeSubject(int x, int y, MIS *mis);

// =============================================================

#ifdef __SENTER__

void paint_header(MMSG *curmsg, word type)
{
   char  temp[MAX_SCREEN_WIDTH], temp2[80];
   MIS *mis = &curmsg->mis;

   if(maxx > 80)
      ClsRectWith(0,80,4,maxx-1,cfg.col[Cmsgheader],' ');

   memset(temp, 'Ä', sizeof(temp));
   temp[maxx] = '\0';
   printn(5,0,cfg.col[Cmsgline],temp, maxx);

   biprinteol(0,0,cfg.col[Cmsgbar], cfg.col[Cmsgbaraccent], " ~F1~ Help  ~ins~ New msg  ~del~ Delete msg  ~R~ reply to msg  ~C~ Change msg  ~P~ Print", '~');

   printn(1,0,cfg.col[Cmsgdata],"Date :", 6);
   vprint(1,6,cfg.col[Cmsgdate], " %-32.32s", MakeT(mis->msgwritten, DATE_HDR));

   printn(2,0,cfg.col[Cmsgdata],"From :", 6);
   if(type == NEWS || type == MAIL)
     vprint(2,6,cfg.col[Cmsgheader], " %-73.73s", mis->from);
   else
     {
     vprint(2,6,cfg.col[Cmsgheader], " %-43.43s", mis->from);
     vprint(2,50,cfg.col[Cmsgheader], "%-30.30s", FormAddress(&mis->origfido));
     }

   if(mis->destinter[0] != '\0' && (type == MAIL || type == NETMAIL))
     {
     printn(3,0,cfg.col[Cmsgdata],"To   :", 6);

     sprintf(temp2, " (via %s, %s)",  mis->to,
                                      FormAddress(&mis->destfido));
     memset(temp, '\0', sizeof(temp));
     strncpy(temp, mis->destinter, 73 - strlen(temp2));
     strcat(temp, temp2);
     vprint(3,6,cfg.col[Cmsgheader], " %-73.73s", temp);
     }
   else
     {
     if(type != NEWS)
       {
       printn(3,0,cfg.col[Cmsgdata],"To   :", 6);
       vprint(3,6,cfg.col[Cmsgheader], " %-73.73s", mis->to);
       }
     else
       {
       printn(3,0,cfg.col[Cmsgdata],"Org  :", 6);
       vprint(3,6,cfg.col[Cmsgheader], " %-73.73s", curmsg->org);
       }

     if(curmsg->status & PERSONAL)
       vprint(3,7,cfg.col[Cmsgspecial], "%s", mis->to);

     if(type == NETMAIL)
       vprint(3,50,cfg.col[Cmsgheader], "%-30.30s", FormAddress(&mis->destfido));
     }

   vprint(1,39,cfg.col[Cmsgattribs], "%41.41s", attr_to_txt(mis->attr1, mis->attr2));

   MakeSubject(4, 0, mis);

}

#else

// =============================================================

void paint_header(MMSG *curmsg, word type)
{
   char  temp[MAX_SCREEN_WIDTH], temp2[80];
   MIS *mis = &curmsg->mis;

   if(maxx > 80)
      ClsRectWith(0,80,3,maxx-1,cfg.col[Cmsgheader],' ');

   memset(temp, 'Ä', sizeof(temp));
   temp[maxx] = '\0';
   printn(4,0,cfg.col[Cmsgline],temp, maxx);

   printn(0,0,cfg.col[Cmsgdata],"Date :", 6);
   vprint(0,6,cfg.col[Cmsgdate], " %-32.32s", MakeT(mis->msgwritten, DATE_HDR));

   printn(1,0,cfg.col[Cmsgdata],"From :", 6);
   if(type == NEWS || type == MAIL)
     vprint(1,6,cfg.col[Cmsgheader], " %-73.73s", mis->from);
   else
     {
     vprint(1,6,cfg.col[Cmsgheader], " %-43.43s", mis->from);
     vprint(1,50,cfg.col[Cmsgheader], "%-30.30s", FormAddress(&mis->origfido));
     }

   if(mis->destinter[0] != '\0' && (type == MAIL || type == NETMAIL))
     {
     printn(2,0,cfg.col[Cmsgdata],"To   :", 6);

     sprintf(temp2, " (via %s, %s)",  mis->to,
                                      FormAddress(&mis->destfido));
     memset(temp, '\0', sizeof(temp));
     strncpy(temp, mis->destinter, 73 - strlen(temp2));
     strcat(temp, temp2);
     vprint(2,6,cfg.col[Cmsgheader], " %-73.73s", temp);
     }
   else
     {
     if(type != NEWS)
       {
       printn(2,0,cfg.col[Cmsgdata],"To   :", 6);
       vprint(2,6,cfg.col[Cmsgheader], " %-73.73s", mis->to);
       }
     else
       {
       printn(2,0,cfg.col[Cmsgdata],"Org  :", 6);
       vprint(2,6,cfg.col[Cmsgheader], " %-73.73s", curmsg->org);
       }

     if(curmsg->status & PERSONAL)
       vprint(2,7,cfg.col[Cmsgspecial], "%s", mis->to);

     if(type == NETMAIL)
       vprint(2,50,cfg.col[Cmsgheader], "%-30.30s", FormAddress(&mis->destfido));
     }

   vprint(0,39,cfg.col[Cmsgattribs], "%41.41s", attr_to_txt(mis->attr1, mis->attr2));

   MakeSubject(3, 0, mis);

}

#endif            // __SENTER__

// =============================================================



char *FormAddress(NETADDR *address)

{
   static char a_string[25];

   memset(a_string, '\0', sizeof(a_string));
   if(address->point != 0)
      sprintf(a_string, "%hu:%hu/%hu.%hu", address->zone,
                                           address->net,
                                           address->node,
                                           address->point);
   else
      sprintf(a_string, "%hu:%hu/%hu", address->zone,
                                       address->net,
                                       address->node);

   return a_string;
}


// ==============================================================


char *MakeT(dword t, int type)
{
   static char temp[50];
   JAMTM *tm;

   if(t == 0L)
     {
     strcpy(temp, "-");
     return temp;
     }

   tm = JAMsysLocalTime(&t);

   if( (tm->tm_mon < 0) || (tm->tm_mon > 11))
        tm->tm_mon = 0;

   memset(temp, '\0', sizeof(temp));
   switch(type)
     {
     case DATE_LIST:
       sprintf(temp, "%s %2.2i",
          months_ab[tm->tm_mon], tm->tm_mday);
       break;

     case DATE_FULL:
       sprintf(temp, "%s %s %2.2i '%2.2i, %2.2i:%2.2i:%2.2i",
          weekday_ab[tm->tm_wday], months_ab[tm->tm_mon], tm->tm_mday, tm->tm_year % 100, tm->tm_hour, tm->tm_min, tm->tm_sec);
       break;

     case DATE_HDR:
       sprintf(temp, "%s %s %2.2i, %2.2i:%2.2i",
                   weekday_ab[tm->tm_wday], months_ab[tm->tm_mon], tm->tm_mday,
                   tm->tm_hour, tm->tm_min);
       break;
     }

   return temp;
}


// ===============================================================

char *attr_to_txt(dword attr1, dword attr2)
{
   static char temp[160];  /* Watch maximum length (# of possible attrs)!! */
   int l;

   memset(&temp, '\0', sizeof(temp));

   for(l=0; l < NUMCONVS1; l++)
     {
     if(attr1 & Attr2Text1[l].bit)
        strcat(temp, Attr2Text1[l].txt);
     }

   for(l=0; l < NUMCONVS2; l++)
     {
     if(attr2 & Attr2Text2[l].bit)
        strcat(temp, Attr2Text2[l].txt);
     }

   return temp;
}

// =====================================================================
// "Subj: " st start of line takes 6 chars. This leaves maxx-6...
//    print(3,6,cfg.col[Cmsgheader], MakeSubject(mis));


void MakeSubject(int x, int y, MIS *mis)
{
  char temp[MAX_SCREEN_WIDTH];
  char temp2[50];

  if(mis->attached == NULL && mis->requested == NULL)
    {
    printn(x, y, cfg.col[Cmsgdata], "Subj :", 6);
    vprint(x, y+6, cfg.col[Cmsgheader], " %-73.73s", mis->subj);
    return;
    }

  memset(temp, '\0', sizeof(temp));
  strcpy(temp, mis->subj);

  if( !(mis->origbase & MSGTYPE_JAM) ||
      ((mis->origbase & MSGTYPE_JAM) && temp[0] == '\0' && !(mis->attached && mis->requested)) )
    {
    if(mis->attached)
       printn(x, y, cfg.col[Cmsgdata], "Att  :", 6);
    else
       printn(x, y, cfg.col[Cmsgdata], "Freq :", 6);
    }
  else
    {
    printn(x, y, cfg.col[Cmsgdata], "Subj :", 6);

    if(mis->attached && mis->requested)
       strcpy(temp2, " (Att/Frq)");
    else if(mis->attached)
       strcpy(temp2, " (Att)");
    else
       strcpy(temp2, " (Frq)");
    if(strlen(temp) > (74 - strlen(temp2) - 1))
      {
      temp[74 - strlen(temp2) - 2] = '\0';
      temp[74 - strlen(temp2) - 3] = ' ';
      temp[74 - strlen(temp2) - 4] = '.';
      temp[74 - strlen(temp2) - 5] = '.';
      }
    strcat(temp, temp2);
    vprint(x, y+6, cfg.col[Cmsgheader], " %-73.73s", temp);
    return;
    }

  SumAttachesRequests(mis, temp, 73, SARboth);

  vprint(x, y+6, cfg.col[Cmsgheader], " %-73.73s", temp);

}


// ===============================================================
//
//  Make a list of attaches/requests, outpout in 'temp', maximum
//  length (not including '\0') is maxlen.
//
// ===============================================================


void SumAttachesRequests(MIS *mis, char *temp, int maxlen, int what)
{
  STRINGLIST *files;
  char temp2[80];

  *temp = '\0';       // Start with a clean slate.

  if(maxlen > 132) maxlen = 132;

  if(what == SARattach)
    files = mis->attached;
  else if(what == SARrequest)
    files = mis->requested;
  else   // Both
    {
    if(mis->attached)
      files = mis->attached;
    else
      files = mis->requested;
    }

  while(files)
    {
    if(files->s != NULL)
      {
      strncpy(temp2, files->s, 70);
      temp2[70] = '\0';
      if(files->pw)
        {
        if((strlen(temp2) + strlen(files->pw) + 2) < 71)
           {
           if(cfg.usr.status & NOSPACEPASSWORD)
             strcat(temp2, "!");
           else
             strcat(temp2, " !");
           strcat(temp2, files->pw);
           }
        }

      if((strlen(temp) + strlen(temp2)) < 71)
        {
        if(strlen(temp)) strcat(temp, " ");
        strcat(temp, temp2);
        }
      }
    files = files->next;
    }

}

// ==============================================================

