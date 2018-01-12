#include <process.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <alloc.h>
#include <msgapi.h>
#include <conio.h>
#include <errno.h>
#include <sys\stat.h>
#include <video.h>
#include <scrnutil.h>
#include <stdlib.h>

#include "input.h"
#include "xmalloc.h"
#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "showmail.h"
#include "getmsg.h"
#include "select.h"
#include "readarea.h"
#include <progprot.h>
#include "attach.h"
#include "message.h"
#include "showhdr.h"
#include "idlekey.h"
#include "header.h"
#include "reply.h"
#include "choose.h"
#include "nodeglue.h"
#include "v7.h"
#include "fdnode.h"

#define NORMALCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'. !?,*&^@#$%()-_=+[]{}\\\"|';:,.<>/?`~1234567890"
#define ADDRESSCHARS "0123456789:/.-"

static char *mtext[]={	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


char * mtime(void);

XMSG *MakeHeader(MSG *areahandle, MMSG *curmsg, int reply, AREA *area, UMSGID reply_to_id, char *subject, char *usenet_address)
{

   XMSG *header;
   union stamp_combo combo;
   struct tm *tmdate;
   time_t time_now;
   int editret=0;

   header = (XMSG *)xcalloc(1, sizeof(XMSG));      /* Message header */

   header->attr= area->stdattr;

   strcpy(header->from, cfg.usr.name[custom.name].name);
   if (reply)
      {
      if( (!(curmsg->status & REPLYTO)) || (area->type != NETMAIL) )
         strcpy(header->to, (reply==2) ? curmsg->hdr.to : curmsg->hdr.from);
      else
         strcpy(header->to, curmsg->rep_gate);

      strcpy(header->subj, curmsg->hdr.subj);
      header->replyto = reply_to_id;
      if(curmsg->hdr.attr & MSGPRIVATE)
         header->attr |= MSGPRIVATE;
      }

   header->orig.zone  = cfg.usr.address[custom.aka].zone;          /* Fill this all */
   header->orig.net   = cfg.usr.address[custom.aka].net;
   header->orig.node  = cfg.usr.address[custom.aka].node;
   header->orig.point = cfg.usr.address[custom.aka].point;

   if (reply)
      {
      if(!(curmsg->status & REPLYTO))
         {
         header->dest.zone  = (reply == 2) ? curmsg->hdr.dest.zone  : curmsg->hdr.orig.zone;
         header->dest.net   = (reply == 2) ? curmsg->hdr.dest.net   : curmsg->hdr.orig.net;
         header->dest.node  = (reply == 2) ? curmsg->hdr.dest.node  : curmsg->hdr.orig.node;
         header->dest.point = (reply == 2) ? curmsg->hdr.dest.point : curmsg->hdr.orig.point;
         }
      else if(area->type == NETMAIL)
         {
         header->dest.zone  = curmsg->rep_addr.zone;
         header->dest.net   = curmsg->rep_addr.net;
         header->dest.node  = curmsg->rep_addr.node;
         header->dest.point = curmsg->rep_addr.point;
         }
      }

   time(&time_now);
   tmdate=localtime(&time_now);
   header->date_written=header->date_arrived=(TmDate_to_DosDate(tmdate,&combo))->msg_st;

   strcpy(header->ftsc_date, mtime());

   if(reply == 3)
      {
      if(area->type == NETMAIL)
         matchaka(header);
      return header;    /* Turbo reply, return immed. */
      }

   if(subject != NULL)
      strcpy(header->subj, subject);

   if (area->type == NETMAIL)
	   editret = EditHeader(areahandle, header, 1, custom.aka, 1, usenet_address, area);
   else
       editret = EditHeader(areahandle, header, 0, 0, 0, usenet_address, area);

   if (editret == ABORT)
      {
      free(header);
      return NULL;
      }
   else
	   return (header);
}


/* ----------------------------------------------- */


int EditHeader(MSG *areahandle, XMSG *hdr, int address, int aka, int domatch, char *usenet_address, AREA *area)

{
   char  tempaddress[80], addrcopy[80];
   int   what=3, ret=0, filecheck=0, alias=0, akamatched=0;
   ADDRLIST *found=NULL;
   NETADDR tempnet;

   if(!domatch) akamatched=1;

   paint_header(hdr, address, 0);

   while(what < 7)
      {
      switch(what)
         {
         case 1:
           statusbar("Type your name or alias, press <TAB> to get selection list..");
           ret = getstring(1,7,hdr->from,35,"",cfg.col.entry);     /* Edit the FROM: field */
           if(ret == TAB)
              {
              if((ret=choose_name()) != -1)
                 {
                 strcpy(hdr->from, cfg.usr.name[ret].name);
                 ret=0;
                 }
              else ret=ESC;
              }
           break;

         case 2:


           statusbar("Type your address, press <TAB> to get selection list..");
           strcpy(tempaddress, FormAddress(&hdr->orig));
           strcpy(addrcopy, tempaddress);
           ret = getstring(1,50,tempaddress,29,ADDRESSCHARS,cfg.col.entry);
           if(strcmp(addrcopy, tempaddress) != 0) /* Did address change? */
              akamatched=1;   /* Never match AKA after manual change */

           if(tempaddress[0] != '\0') address_expand(tempaddress, &hdr->orig, aka);

           if(ret == TAB)
               {
               if((ret=choose_address()) != -1)
                   {
                   hdr->orig.zone  = cfg.usr.address[ret].zone;
                   hdr->orig.net   = cfg.usr.address[ret].net;
                   hdr->orig.node  = cfg.usr.address[ret].node;
                   hdr->orig.point = cfg.usr.address[ret].point;
                   ret=0;
                   akamatched=1; /* Never match on manually changed address */
                   }
               else ret = ESC;
               }
           break;

         case 3:
           if(address)
             statusbar("Type name or address to send to, F2 to force nodelist lookup");
           else
             statusbar("Type name of the person to send this message to..");

           strcpy(tempaddress, hdr->to);  /* Save copy, to detect a change */
           ret = getstring(2,7,hdr->to,35,"",cfg.col.entry);       /* Edit the TO: field */

           if((ret != 0) && (ret != F2)) break;   /* <ENTER> not pressed */

           if( (ret != F2) && ((alias=check_alias(hdr, usenet_address)) != 0) )    /* Is it an alias? */
               {
               what += alias;   /* skip # of fields defined by alias */
               if(alias == 2)   /* Also filled in subject, do AKA matching now! */
                  {
                  if(matchaka(hdr) != 0)
                     print(5, 50, cfg.col.msgtext, "AKA matched");
                  }
               break;           /* stop editting this field          */
               }

           if(address)  /* Netmail, we need to do nodelist lookups.. */
                {
                /* Check if TO: address is filled, and TO: *not* changed */

                if(
                     (hdr->dest.zone!=0) &&
                     (strcmp(hdr->to, tempaddress) == 0) &&
                     (ret != F2)
                  )

                  { /* Check only if node address exists */
                  found=NULL;
                  memset(&tempnet, '\0', sizeof(NETADDR));
                  tempnet.zone = hdr->dest.zone;
                  tempnet.net  = hdr->dest.net;
                  tempnet.node = hdr->dest.node;
                  /* Leave point 0! That's why we do this! */

                  if(cfg.usr.nodelist[0] != '\0')       /* Version 7 present? */
                      found = ver7find(&tempnet);

                  if( (found==NULL) && (cfg.usr.fdnodelist[0] != '\0') )  /* FD nodelist? */
                      found = getFDnode(&tempnet);

                  if(found == NULL)   /* Address not found in nodelist(s) */
                     print(5, 0, cfg.col.msgtext, "Unknown address");
                  else
                     {
                     what++;
                     free(found);
                     }
                  }

                else if(check_node(hdr, aka, PROMPT) != 0)  /* Found it?            */
                   {
                   what++;                      /* Skip address fill in */
                   print(5, 0, cfg.col.msgtext, "               ");
                   }
                else print(5, 0, cfg.col.msgtext, "Unknown address");
                }

           break;

         case 4:

                 print(5,50,cfg.col.msgtext, "           "); /* Clear possible 'AKA macthed' */
                 statusbar("Type address to send to..");
                 strcpy(tempaddress, FormAddress(&hdr->dest));
                 if( (hdr->dest.zone==0) && (hdr->dest.net==0) && (hdr->dest.node==0) && (hdr->dest.point==0) )
                     memset(tempaddress, '\0', sizeof(tempaddress));
                 ret = getstring(2,50,tempaddress,29,ADDRESSCHARS,cfg.col.entry);
                 if(tempaddress[0] != '\0') address_expand(tempaddress, &hdr->dest, aka);

                 /* Check for forced AKA match */
                 if((ret == F2) && (address))
                    {
                    if(matchaka(hdr) != 0)
                       {
                       paint_header(hdr, address, 0);
                       print(5, 50, cfg.col.msgtext, "AKA matched");
                       }
                    what++;
                    }

                 break;

         case 5:

                 if((!akamatched) && (address))
                    {
                    if(matchaka(hdr) != 0)
                       {
                       paint_header(hdr, address, 0);
                       print(5, 50, cfg.col.msgtext, "AKA matched");
                       }
                    }
                 statusbar("Give the subject of the message..");
                 print(5, 0, cfg.col.msgtext, "               "); /* Clear possible 'unknown address' string */
	              ret = getstring(3,7,hdr->subj,71,"",cfg.col.entry);		/* Edit the SUBJ: field */
                 break;

         case 6:

                 print(5,50,cfg.col.msgtext, "           ");
                 if( (isalpha(hdr->subj[0])) && (hdr->subj[1] == ':') && address)
                     hdr->attr |= MSGFILE;

                 ret = SetAttributes(hdr);
                 break;
         }

      paint_header(hdr, address, 0);

      switch(ret)
         {
         case 0:
         case DOWN:
         case TAB:
            what++;
            if ( ((what==2) || (what == 4)) && !address) what++;
               break;

         case UP:
         case BTAB:
            if(--what < 1) what=1;
            if ( ((what==2) || (what == 4)) && !address ) what--;
            break;
         case ESC:
            return ABORT;
         default:
                ;

         }

      /* Check for file attach and verify files.. */

      if (address && (hdr->attr & MSGFILE) && (what > 6))
         {
         filecheck = check_attach(hdr, areahandle, area);
         if (!filecheck)                   /* Not a good checkout ;-) */
            what = 5;
         }

      }

   return 0;

}

/* ----------------------------------------------- */


int SetAttributes (XMSG *hdr)

{

   char temp[80];
   char col = cfg.col.msgtext;
   BOX *attrbox;

   attrbox = initbox(7,0,21,47,cfg.col.msgline,cfg.col.msgtext,SINGLE,NO,' ');
   drawbox(attrbox);
   delbox(attrbox);

   print(6,1,112," Attributes:");
   print(8,1,col," [P]rivate");
   print(9,1,col," [C]rash");
   print(10,1,col," [A]ttach file");
   print(11,1,col," [R]equest file");
   print(12,1,col," [K]ill/sent");
   print(13,1,col," [L]ocal");
   print(14,1,col," [H]old");
   print(15,1,col," sca[N]ned");
   print(16,1,col," [F]orward");
   print(17,1,col," [S]ent");
   print(18,1,col," recei[V]ed");
   print(19,1,col," [D]irect");
   print(20,1,col," [U]pdate req.");

   print(8, 20,col,"[O]rphan");
   print(10,20,col,"[Y] return receipt");
   print(9, 20,col,"[Q] ret. receipt re[Q]uest");

   print(12,20,col,"Extra 'flags':");

   print(14,20,col,"[E]rase file when sent");
   print(15,20,col,"[X] archive file when sent");
   print(16,20,col,"[I]mmediate");
   print(17,20,col,"[T]runcate file when sent");
   print(18,20,col,"[W] Lock");
   print(19,20,col,"[M] confirm receipt request");


   statusbar("Press letter to toggle attributes, <ENTER> to continue..");

   while(1)
      {
      sprintf(temp, "%40.40s", attr_to_txt(hdr->attr));
      print(0,40,cfg.col.msgattribs,temp);

      switch(get_idle_key(1))
         {
         case 328:   /* up */
             print(6,1,col," Attributes:");
             return UP;

         case 271:  /* BTAB */
             print(6,1,col," Attributes:");
             return BTAB;

         case 27 :
             return ESC;

         case 'D':
         case 'd':
           hdr->attr ^= ADD_DIR;
           break;

         case 'W':
         case 'w':
           hdr->attr ^= ADD_LOK;
           break;

         case 'M':
         case 'm':
           hdr->attr ^= ADD_CFM;
           break;

         case 'I':
         case 'i':
           if(cfg.usr.arcmail)
             hdr->attr ^= ADD_IMM;
           break;

         case 'T':
         case 't':
           if(cfg.usr.arcmail)
             hdr->attr ^= ADD_TFS;
           break;

         case 'X':
         case 'x':
           if(cfg.usr.arcmail)
             hdr->attr ^= ADD_AS;
           break;

         case 'E':
         case 'e':
           if(cfg.usr.arcmail)
             hdr->attr ^= ADD_KFS;
           break;

		 case 'P' :
       case 'p' :
			 hdr->attr ^= MSGPRIVATE;
			 break;

		 case 'Q' :
       case 'q' :
			 hdr->attr ^= MSGRRQ;
			 break;

		 case 'Y' :
       case 'y' :
			 hdr->attr ^= MSGCPT;
			 break;

       case 'c' :
		 case 'C' :
			 hdr->attr ^= MSGCRASH;
			 break;

       case 's' :
		 case 'S' :
			 hdr->attr ^= MSGSENT;
			 break;

       case 'a' :
		 case 'A' :
			 hdr->attr ^= MSGFILE;
			 break;

       case 'f' :
		 case 'F' :
			 hdr->attr ^= MSGFWD;
			 break;

       case 'k' :
		 case 'K' :
			 hdr->attr ^= MSGKILL;
			 break;

       case 'l' :
		 case 'L' :
			 hdr->attr ^= MSGLOCAL;
			 break;

       case 'h' :
		 case 'H' :
			 hdr->attr ^= MSGHOLD;
			 break;

       case 'r' :
		 case 'R' :
			 hdr->attr ^= MSGFRQ;
			 break;

       case 'n' :
		 case 'N' :
			 hdr->attr ^= MSGSCANNED;
			 break;

       case 'v' :
		 case 'V' :
			 hdr->attr ^= MSGREAD;
			 break;

       case 'u':
       case 'U':
          hdr->attr ^= MSGURQ;
          break;

       case 'o':
       case 'O':
          hdr->attr ^= MSGORPHAN;
          break;

		 case 13 :
             print(6,1,col," Attributes:");
			 return 0;
         }          /* switch */

      } /* while */
}



void statusbar(char *s)
{
/*   char temp[133];

   strcpy(temp, s);

   memset(temp+strlen(temp), ' ',maxx-strlen(temp));
   temp[maxx]='\0';

	print(maxy-1,0,cfg.col.msgbar,temp);*/

   printeol(maxy-1,0,cfg.col.msgbar,s);
}



/* Check TO: address and all AKA's to pick the most appropriate one.. */


int matchaka(XMSG *hdr)
{
   int hits, i, firsthit = -1, found_zone_net=0;
   static int akano = -1;
   int currentmatcheszone = 0; /* The current address matches */

   if(cfg.usr.akamatch == 0) return 0; /* AKA matching is off.. */

   if(akano == -1)     /* No of AKA's not counted yet */
      {
      for(akano=0, i=0; i<24; i++)  /* Count no of AKA's */
         {
         if(cfg.usr.address[i].zone != 0)
            akano++;
         else
            break;
         }
      }


   if(akano == 1) return 0;   /* Only 1 AKA, go home! */


   for(hits=0, i=0; i<akano; i++)  /* Check for zone-match for all AKA's */
      {
      if(cfg.usr.address[i].zone == hdr->dest.zone)
         {
         if(
            (cfg.usr.address[i].zone  == hdr->orig.zone ) &&
            (cfg.usr.address[i].net   == hdr->orig.net  ) &&
            (cfg.usr.address[i].node  == hdr->orig.node ) &&
            (cfg.usr.address[i].point == hdr->orig.point)
           )
           currentmatcheszone = 1;

         if(firsthit == -1)
             firsthit = i;  /* Remember first zone hit for later */
         hits++;
         }
      }

   if(hits == 0)
      return 0;      /* No zone hits found, use current address */

   /* If more than one 'zone match' was found, check nets too.. */

   if(hits > 1)
       {
       for(hits=0, i=0; i<akano; i++)  /* Check for zone AND net */
         {
         if(
             (cfg.usr.address[i].zone == hdr->dest.zone) &&
             (cfg.usr.address[i].net  == hdr->dest.net )
           )
             {
             if(
                (cfg.usr.address[i].zone  == hdr->orig.zone ) &&
                (cfg.usr.address[i].net   == hdr->orig.net  ) &&
                (cfg.usr.address[i].node  == hdr->orig.node ) &&
                (cfg.usr.address[i].point == hdr->orig.point)
               )
               return 0;

             if(found_zone_net == 0)
                {
                firsthit=i; /* Found a match, use it, even if there would be more */
                found_zone_net = 1;
                }
             }  /* if */
         } /* for */

       }

   if(currentmatcheszone && !found_zone_net) /* The current address is one of the matches, no change */
     return 0;

   if(
       (cfg.usr.address[firsthit].zone  == hdr->orig.zone ) &&
       (cfg.usr.address[firsthit].net   == hdr->orig.net  ) &&
       (cfg.usr.address[firsthit].node  == hdr->orig.node ) &&
       (cfg.usr.address[firsthit].point == hdr->orig.point)
     )
     return 0;    /* Current address, so no AKA change */

   /* Fill in new adress */

   hdr->orig.zone  = cfg.usr.address[firsthit].zone;
   hdr->orig.net   = cfg.usr.address[firsthit].net;
   hdr->orig.node  = cfg.usr.address[firsthit].node;
   hdr->orig.point = cfg.usr.address[firsthit].point;

   /* Return 1 to signal change */

   return 1;
}



/* ------------------------------------------------------- */



char * mtime(void)

{
    static char mtime_buffer[21];
    struct tm *timestamp;
    time_t now;

    memset(mtime_buffer, '\0', sizeof(mtime_buffer));

    now = time(NULL);

    timestamp = localtime(&now);

    sprintf(mtime_buffer, "%02d %s %02d  %02d:%02d:%02d",
        timestamp->tm_mday, mtext[timestamp->tm_mon],
        timestamp->tm_year, timestamp->tm_hour,
        timestamp->tm_min, timestamp->tm_sec);

    return(mtime_buffer);
}


/* ----------------------------------------------------------- */


