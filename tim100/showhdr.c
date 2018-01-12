#include <string.h>
#include <stdio.h>
#include <alloc.h>

#include <msgapi.h>
#include <video.h>
#include <scrnutil.h>

#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "reply.h"
#include "showmail.h"

static char *mtext[]={	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


void paint_header(XMSG *hdr, int address, int personal);
char *attr_to_txt(dword attr);
char *FormAddress(NETADDR *address);
char *attr_to_txt(dword attr);


void paint_header(XMSG *hdr, int address, int personal)
{
   char	temp[133];

   if(maxx > 80)
      ClsRectWith(0,80,3,maxx-1,cfg.col.msgheader,' ');

   memset(temp, 'Ä', sizeof(temp));
   temp[maxx] = '\0';
	printn(4,0,cfg.col.msgline,temp, maxx);

   printn(0,0,cfg.col.msgdata,"Date :", 6);
	sprintf(temp, " %-32.32s", MakeT(&hdr->date_written));
	printn(0,6,cfg.col.msgdate,temp, 33);

   printn(1,0,cfg.col.msgdata,"From :", 6);
	sprintf(temp, " %-43.43s", hdr->from);
	printn(1,6,cfg.col.msgheader,temp, 44);

	sprintf(temp, "%-30.30s", FormAddress(&hdr->orig));
	printn(1,50,cfg.col.msgheader,temp, 30);

   printn(2,0,cfg.col.msgdata,"To   :", 6);

   sprintf(temp, " %-73.73s", hdr->to);
   printn(2,6,cfg.col.msgheader,temp, 74);

   if(personal)
     {
     sprintf(temp, "%s", hdr->to);
	  print(2,7,cfg.col.msgspecial,temp);
     }

   if (address)
      {
	   sprintf(temp, "%-30.30s", FormAddress(&hdr->dest));
	   printn(2,50,cfg.col.msgheader,temp,30);
      }

   printn(3,0,cfg.col.msgdata,"Subj :",6);
	sprintf(temp, " %-73.73s", hdr->subj);
	printn(3,6,cfg.col.msgheader,temp,74);
	sprintf(temp, "%41.41s", attr_to_txt(hdr->attr));
	printn(0,39,cfg.col.msgattribs,temp,41);


}




char *FormAddress(NETADDR *address)

{
	static char	a_string[80];

	memset(a_string, '\0', sizeof(a_string));
	sprintf(a_string, "%i:%i/%i.%i", address->zone,
                                    address->net,
                                    address->node,
                                    address->point);

	return a_string;
}





char *MakeT(struct _stamp *t)

{
	static char temp[40];

   if( (t->date.mo < 1) || (t->date.mo > 12))
        t->date.mo = 1;

	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%s %2.2i '%2.2i, %2.2i:%2.2i",
     mtext[t->date.mo-1], t->date.da, t->date.yr+80, t->time.hh, t->time.mm);

	return temp;
}



char *attr_to_txt(dword attr)

{
	static char temp[120];  /* Watch maximum length (# of possible attrs)!! */
   char direct=0;

	memset(&temp, '\0', sizeof(temp));

   if (attr & ADD_DIR)
      strcat(temp, "Dir ");

   if (attr & ADD_IMM)
      strcat(temp, "Imm ");

   if (attr & ADD_AS)
      strcat(temp, "A/S ");

   if (attr & ADD_KFS)
      strcat(temp, "KFS ");

   if (attr & ADD_TFS)
      strcat(temp, "TFS ");

   if (attr & ADD_LOK)
      strcat(temp, "LOK ");

   if (attr & ADD_CFM)
      strcat(temp, "CFM ");

    if (attr & MSGPRIVATE)
			strcat(temp, "Pvt ");

	if (attr & MSGREAD)
			strcat(temp, "Rvd ");

	if (attr & MSGCRASH)
			strcat(temp, "Cra ");

	if (attr & MSGSENT)
			strcat(temp, "Snt ");

	if (attr & MSGFILE)
			strcat(temp, "F/a ");

	if (attr & MSGFWD)
			strcat(temp, "Fwrd ");

	if (attr & MSGKILL)
			strcat(temp, "Kill ");

	if (attr & MSGLOCAL)
			strcat(temp, "Loc ");

	if (attr & MSGHOLD)
			strcat(temp, "Hold ");

	if (attr & MSGFRQ)
			strcat(temp, "Frq ");

	if (attr & MSGSCANNED)
			strcat(temp, "Scn ");

	if (attr & MSGURQ)
			strcat(temp, "Urq ");

	if (attr & MSGORPHAN)
			strcat(temp, "Orph ");

	if (attr & MSGCPT)
			strcat(temp, "Cpt ");

	if (attr & MSGRRQ)
			strcat(temp, "Rrq ");

	if (attr & MSGXX2)
			strcat(temp, "XX2 ");

	return temp;
}
