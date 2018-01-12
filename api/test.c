#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

#include "msgapi.h"

typedef struct
{
   long bit;
   char txt[5];

} BITTXT;


#define NUMCONVS1 31

BITTXT Attr2Text1[] = {

{ aPRIVATE, "PVT "},
{ aCRASH  , "CRA "},
{ aREAD   , "RCV "},
{ aSENT   , "SNT "},
{ aFILE   , "F/A "},
{ aFWD    , "FWD "},
{ aORPHAN , "ORP "},
{ aKILL   , "KIL "},
{ aLOCAL  , "LOC "},
{ aHOLD   , "HLD "},
{ aXX2    , "XX2 "},
{ aFRQ    , "FRQ "},
{ aRRQ    , "RRQ "},
{ aCPT    , "CPT "},
{ aARQ    , "ARQ "},
{ aURQ    , "URQ "},
{ aSCANNED, "SCN "},
{ aDIR    , "DIR "},
{ aAS     , "A/S "},
{ aIMM    , "IMM "},
{ aKFS    , "KFS "},
{ aTFS    , "TFS "},
{ aCFM    , "CFM "},
{ aLOK    , "LOK "},
{ aZGT    , "ZGT "},
{ aENC    , "ENC "},
{ aCOM    , "COM "},
{ aESC    , "ESC "},
{ aFPU    , "FPU "},
{ aNODISP , "NOD "},
{ aDEL    , "DEL "}

};


#define NUMCONVS2 7

BITTXT Attr2Text2[] = {

{ aHUB, "HUB "},
{ aXMA, "XMA "},
{ aHIR, "HIR "},
{ aCOV, "COV "},
{ aSIG, "SIG "},
{ aLET, "LET "},
{ aFAX, "FAX "}

};


void   PrintStringList(STRINGLIST *l);
char * StringTime(dword t);
char * AscAddress(NETADDR *addr);
void   rton(char *s);
char * attr_to_txt(dword attr1, dword attr2);
void kton(char *s);


void main(void)
{
   MSG *ahandle;
   MSGH *msghandle;
   MIS mis;
   struct _minf   minf;
   dword current, high;
   dword bodylen, ctrllen;
   char *body, *ctrl;

   minf.def_zone    = 0;
   minf.req_version = 0;

   MsgOpenApi(&minf, "", 1, "/tmp/");

   if( (ahandle=MsgOpenArea("/tmp",
                            MSGAREA_CRIFNEC,
                            MSGTYPE_SDM | MSGTYPE_NET)) == NULL )
      {
      printf("\nError opening area\n");
      exit(254);
      }

   high = MsgGetHighMsg(ahandle);

   for(current = 1; current <= high; current++)
     {
     if( (msghandle=MsgOpenMsg(ahandle, MOPEN_READ, current)) == NULL)
        continue;   // Open failed, go on.

     memset(&mis, '\0', sizeof(mis));

     bodylen = MsgGetTextLen(msghandle);
     ctrllen = MsgGetCtrlLen(msghandle);

     body = ctrl = NULL;
     if(bodylen)
        body = calloc(1, bodylen+1);
     if(ctrllen)
        ctrl = calloc(1, ctrllen+1);

     if(MsgReadMsg(msghandle, &mis, 0L, bodylen, body, ctrllen, ctrl) == -1)
       {
       printf("\nError reading message!\n");
       exit(254);
       }

     rton(body);
     kton(ctrl);

     printf("\n*** Successfully read message #%d - Message Info Structure:\n", MsgGetCurMsg(ahandle));
     printf("Attr         : %s\n", attr_to_txt(mis.attr1, mis.attr2));
     printf("from         : %s\n",  mis.from);
     printf("to           : %s\n",  mis.to);
     printf("subj         : %s\n", mis.subj);
     printf("origfido     : %s\n", AscAddress(&mis.origfido));
     printf("destfido     : %s\n", AscAddress(&mis.destfido));
     printf("origdomain   : %s\n", mis.origdomain);
     printf("destdomain   : %s\n", mis.destdomain);
//     printf("originter    : %s\n", mis.originter);
//     printf("destinter    : %s\n", mis.destinter);
     printf("msgwritten   : %s", StringTime(mis.msgwritten));
     printf("msgprocessed : %s", StringTime(mis.msgprocessed));
     printf("msgreceived  : %s", StringTime(mis.msgreceived));
     printf("ftsc_date    : %s\n", mis.ftsc_date);
     printf("timesread    : %d\n", mis.timesread);
//     printf("utc_ofs      : %d\n", mis.utc_ofs");
     printf("replyto      : %d\n", mis.replyto);
     printf("replies      : %d %d %d %d %d %d %d %d %d\n", mis.replies[0], mis.replies[1], mis.replies[2], mis.replies[3], mis.replies[4], mis.replies[5], mis.replies[6], mis.replies[7], mis.replies[8]);
     printf("nextreply    : %d\n", mis.nextreply);
     printf("origbase     : %d\n", mis.origbase);
     printf("msgno        : %d\n", mis.msgno);
     getchar();
     printf("requested    : "); PrintStringList(mis.requested);
     printf("attached     : "); PrintStringList(mis.attached);
     printf("seenby       : "); PrintStringList(mis.seenby);
     printf("path         : "); PrintStringList(mis.path);
     printf("via          : "); PrintStringList(mis.via);
     getchar();
     printf("extrasub     : %s\n", mis.extrasub == NULL ? "none" : "yes!");
//     getchar();
     printf("%s\n\n", ctrl);
     getchar();
     printf("%sþ\n\n", body);
     getchar();

     if(MsgCloseMsg(msghandle) == -1)
       {
       printf("\nError closing message!\n");
       exit(254);
       }

     if(body) free(body);
     if(ctrl) free(ctrl);
     }

   MsgCloseArea(ahandle);

   MsgCloseApi();

}


void PrintStringList(STRINGLIST *l)
{
   if(l == NULL)
     {
     printf("none\n");
     return;
     }

   printf("\n");
   while(l)
     {
     if(l->pw != NULL)
        printf("%s pw: %s\n", l->s, l->pw);
     else if(l->s != NULL)
        printf("%s\n", l->s);
     else
        printf("NULL pointer l->s!!\n");

     l = l->next;
     }
}


char * StringTime(dword t)
{
   struct tm *tm;
   static char mytime[50];

   memset(mytime, '\0', sizeof(mytime));

   if(t == 0L)
     {
     strcpy(mytime, "<null>\n");
     return mytime;
     }

   tm = localtime((time_t *)&t);
   strcpy(mytime, asctime(tm));
   return mytime;
}


char * AscAddress(NETADDR *addr)
{
   static char address[30];

   sprintf(address, "%u:%u/%u.%u", addr->zone, addr->net, addr->node, addr->point);
   return address;
}


void rton(char *s)
{
   while(s && *s)
     {
     if(*s == '\r')
        *s = '\n';
     s++;
     }
}


void kton(char *s)
{
   while(s && *s)
     {
     if(*s == '\01')
        *s = '\n';
     s++;
     }
}


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

