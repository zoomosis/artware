#include "includes.h"

#define NORMALCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'. !?,*&^@#$%()-_=+[]{}\\\"|';:,.<>/?`~1234567890"
#define ADDRESSCHARS "0123456789:/.-"
#define VFNWW ""

char * near strip_subject(char *s, int netmail);
int    near MaskMatch(NETADDR *adr, NETADDR *mask);

int    aliaslist(MIS *mis);

// ----------------------------------------------------------------

void PaintHeaderFrame(void);
void PaintNewHeader  (MMSG *curmsg, word type, word special);

#define pUPDATE  0x0001
#define pAKA     0x0002
#define pUNKNOWN 0x0004
#define pATTACH  0x0008

void   ShowAttached    (MIS *mis);
void   ShowRequested   (MIS *mis);
char * CompressPath    (char *s);

void EditRequest  (MMSG *curmsg, word type);
void EditAttach   (MMSG *curmsg, word type);
void RemoveElement(STRINGLIST **start, int elno);
void SetSpecial   (MMSG *curmsg);

int EditOneRequest(char * filename, char * password);


// ----------------------------------------------------------------

// Reply == 2 : reply to original addressee

#ifdef __WATCOMC__
  #define MAXFILE _MAX_FNAME
  #define MAXEXT  _MAX_EXT
#endif

int MakeHeader(MSG *areahandle, MMSG *curmsg, int reply, AREA *area, UMSGID reply_to_id, char *subject, MMSG *newmmsg)
{
   int editret=0;

   memset(&newmmsg->mis, '\0', sizeof(MIS));

   newmmsg->mis.attr1 = area->stdattr;
   if(cfg.usr.status & SPELLCHECKDFLT)
      newmmsg->status |= SPELLCHECK;

   if(reply == 2)            // Shut off Usenet stuff for OrigReply/Followup
     {
     curmsg->status &= ~REPLYTO;
     // If the original is addressed to internet, do it for this as well.
     if(curmsg->mis.destinter[0] != '\0')
        strcpy(newmmsg->mis.destinter, curmsg->mis.destinter);
     }

   strcpy(newmmsg->mis.from, cfg.usr.name[custom.name].name);
   if (reply)
      {
      if(
          (!(curmsg->status & REPLYTO)) ||
          (area->type != NETMAIL && area->type != MAIL)
        )
         strcpy(newmmsg->mis.to, (reply==2) ? curmsg->mis.to : curmsg->mis.from);
      else
        {
        strcpy(newmmsg->mis.to, curmsg->rep_gate);
        strcpy(newmmsg->mis.destinter, curmsg->rep_name);
        }

      strcpy(newmmsg->mis.subj, strip_subject(curmsg->mis.subj, area->type == NETMAIL));
      if(newmmsg->mis.subj[0] == '\0')
         Files2Subject(&curmsg->mis, newmmsg->mis.subj);

      newmmsg->mis.replyto = reply_to_id;

      if(area->type != ECHOMAIL && area->type != NEWS && (curmsg->mis.attr1 & aPRIVATE))
         newmmsg->mis.attr1 |= aPRIVATE;
      }

   newmmsg->mis.origfido = cfg.usr.address[custom.aka];

   if (reply)
      {
      if( !(curmsg->status & REPLYTO))
         {
         newmmsg->mis.destfido = (reply == 2) ? curmsg->mis.destfido : curmsg->mis.origfido;
         }
      else if(area->type == NETMAIL || area->type == MAIL)
         {
         newmmsg->mis.destfido = curmsg->rep_addr;
         }
      }

   newmmsg->mis.msgwritten = JAMsysTime(NULL);

   if(reply == 3)
      {
      if(area->type == NETMAIL)
         matchaka(&newmmsg->mis);
      return 0;    /* Turbo reply, return immed. */
      }

   if(subject != NULL)
      strcpy(newmmsg->mis.subj, subject);

   if (area->type == NETMAIL)
      editret = EditHeader(areahandle, newmmsg, 1, custom.aka, 1, area);
   else
       editret = EditHeader(areahandle, newmmsg, 0, 0, 0, area);

   if (editret == ABORT)
      {
      return -1;
      }
   else
      return 0;
}


/* ----------------------------------------------- */


int EditHeader(MSG *areahandle, MMSG *curmsg, int address, int aka, int domatch, AREA *area)
{
   char  tempaddress[101], addrcopy[101];
   int   what=3, ret=0, alias=0, akamatched=0;
   ADDRLIST *found=NULL;
   NETADDR tempnet;
   word special = pUPDATE;

   if(!domatch) akamatched=1;

   if(area->type == NEWS)  // Skip TO: in news...
      what = 5;

   PaintNewHeader(curmsg, areahandle->type, pATTACH);

   while(what < 7)
      {
      switch(what)
         {
         case 1:
           bottom("Type your name or alias, press ~TAB~ to get selection list..");
           ret = getstring(1,13,curmsg->mis.from,65,99,"",cfg.col[Centry], cfg.col[Cfinddata]);     /* Edit the FROM: field */
           if(ret == TAB)
              {
              if((ret=choose_name()) != -1)
                 {
                 strcpy(curmsg->mis.from, cfg.usr.name[ret].name);
                 ret=0;
                 }
              else ret=ESC;
              }
           else if(ret == RESET)
             EditRequest(curmsg, areahandle->type);
           else if(ret == ATTACH)
             EditAttach(curmsg, areahandle->type);
           else if(ret == SAVE)
             SetSpecial(curmsg);


           break;

         // -------------------------------------------------------------

         case 2:

           bottom("Type your address, press ~TAB~ to get selection list..");
           strcpy(tempaddress, FormAddress(&curmsg->mis.origfido));
           strcpy(addrcopy, tempaddress);
           ret = getstring(2,13,tempaddress,29,79,ADDRESSCHARS,cfg.col[Centry], cfg.col[Cfinddata]);
           if(strcmp(addrcopy, tempaddress) != 0) /* Did address change? */
              akamatched=1;   /* Never match AKA after manual change */

           if(tempaddress[0] != '\0') address_expand(tempaddress, &curmsg->mis.origfido, aka);

           if(ret == TAB)
               {
               if((ret=choose_address()) != -1)
                   {
                   curmsg->mis.origfido = cfg.usr.address[ret];
                   ret=0;
                   akamatched=1; /* Never match on manually changed address */
                   }
               else ret = ESC;
               }
           else if(ret == RESET)
             EditRequest(curmsg, areahandle->type);
           else if(ret == ATTACH)
             EditAttach(curmsg, areahandle->type);
           else if(ret == SAVE)
             SetSpecial(curmsg);

           break;

         // -------------------------------------------------------------

         case 3:
           if(address)
             bottom("Name or address to send to, ~F2~ for a fresh nodelist lookup, ~TAB~ for a macrolist");
           else
             bottom("Type name of the person to send this message to..");

           strcpy(tempaddress, curmsg->mis.to);  /* Save copy, to detect a change */

           if(address && curmsg->mis.destinter[0] != '\0')
             {
             ret = getstring(3,13,curmsg->mis.destinter,65,99,"",cfg.col[Centry], cfg.col[Cfinddata]);       /* Edit the TO: field */
             if(strchr(curmsg->mis.destinter, '@') == NULL &&
                strchr(curmsg->mis.destinter, '!') == NULL )  // I'net address lost..
               {
               strcpy(curmsg->mis.to, curmsg->mis.destinter);
               memset(curmsg->mis.destinter, '\0', sizeof(curmsg->mis.destinter));
               }
             }
           else
             {
             ret = getstring(3,13,curmsg->mis.to,65,99,"",cfg.col[Centry], cfg.col[Cfinddata]);       /* Edit the TO: field */
             }

           if((ret != 0)  &&
              (ret != F2) &&
              (ret != RESET) &&
              (ret != ATTACH) &&
              (ret != SAVE) &&
              (ret != TAB) ) break;   /* <ENTER> not pressed */

           if(ret == TAB)
             {
             if(aliaslist(&curmsg->mis) != 0)
                {
                what--;
                break;
                }
             }

           if(ret == RESET)
             {
             EditRequest(curmsg, areahandle->type);
             break;
             }

           else if(ret == ATTACH)
             {
             EditAttach(curmsg, areahandle->type);
             break;
             }

           else if(ret == SAVE)
             {
             SetSpecial(curmsg);
             break;
             }

           if( (ret != F2) && ((alias=check_alias(&curmsg->mis)) != 0) )    /* Is it an alias? */
               {
               what += alias;   /* skip # of fields defined by alias */
               if(alias == 2)   /* Also filled in subject, do AKA matching now! */
                  {
                  if(matchaka(&curmsg->mis) != 0)
                     {
                     special |= pAKA;
                     }
                  }
               break;           /* stop editting this field          */
               }

           if(address)  /* Netmail, we need to do nodelist lookups.. */
                {
                // Let's first check to see if an internet address is filled
                // in. In that case we need to move the internet address to
                // mis.destinter and address to the gate..

                if( (strchr(curmsg->mis.to, '@') != NULL ||
                     strchr(curmsg->mis.to, '!') != NULL ) &&
                     cfg.usr.uucpname[0] != '\0')
                  {
                  strcpy(curmsg->mis.destinter, curmsg->mis.to);
                  curmsg->mis.destfido = cfg.usr.uucpaddress;
                  strcpy(curmsg->mis.to, cfg.usr.uucpname);
                  what++;
                  break;
                  }

                /* Check if TO: address is filled, and TO: *not* changed */

                if(
                     (curmsg->mis.destfido.zone!=0) &&
                     (strcmp(curmsg->mis.to, tempaddress) == 0) &&
                     (ret != F2)
                  )

                  { /* Check only if node address exists */
                  found=NULL;
                  memset(&tempnet, '\0', sizeof(NETADDR));
                  tempnet.zone = curmsg->mis.destfido.zone;
                  tempnet.net  = curmsg->mis.destfido.net;
                  tempnet.node = curmsg->mis.destfido.node;
                  /* Leave point 0! That's why we do this! */

//                  if(cfg.usr.nodelist[0] != '\0')       /* Version 7 present? */
//                      found = ver7find(&tempnet);

//                  if( (found==NULL) && (cfg.usr.fdnodelist[0] != '\0') )  /* FD nodelist? */
//                      found = getFDnode(&tempnet);

                  found = NodeLookup((char *)&tempnet, 0);

                  if(found == NULL)   /* Address not found in nodelist(s) */
                     {
                     special |= pUNKNOWN;
                     }
                  else
                     {
                     what++;
                     mem_free(found);
                     }
                  }

                else if(check_node(&curmsg->mis, aka, PROMPT) == 1)  /* Found it?            */
                   {
                   what++;                      /* Skip address fill in */
                   }
                else
                   {
                   special |= pUNKNOWN;
                   }
                }

           break;

         case 4:

                 bottom("Type address to send to..");
                 strcpy(tempaddress, FormAddress(&curmsg->mis.destfido));
                 if( (curmsg->mis.destfido.zone==0) && (curmsg->mis.destfido.net==0) && (curmsg->mis.destfido.node==0) && (curmsg->mis.destfido.point==0) )
                     memset(tempaddress, '\0', sizeof(tempaddress));
                 ret = getstring(4,13,tempaddress,29,99,ADDRESSCHARS,cfg.col[Centry], cfg.col[Cfinddata]);
                 if(tempaddress[0] != '\0') address_expand(tempaddress, &curmsg->mis.destfido, aka);

                 /* Check for forced AKA match */
                 if((ret == F2) && (address))
                    {
                    if(matchaka(&curmsg->mis) != 0)
                       {
                       special |= pAKA;
                       }

                    what++;
                    }
                 else if(ret == RESET)
                   EditRequest(curmsg, areahandle->type);
                 else if(ret == ATTACH)
                   EditAttach(curmsg, areahandle->type);
                 else if(ret == SAVE)
                   SetSpecial(curmsg);

                 break;

         case 5:

                 if((!akamatched) && (address))
                    {
                    if(matchaka(&curmsg->mis) != 0)
                       {
                       PaintNewHeader(curmsg, areahandle->type, (pAKA|pUPDATE));
                       }

                    }

                 if( (!(areahandle->type & MSGTYPE_JAM)) &&
                     (curmsg->mis.attached || curmsg->mis.requested) )
                     {
                     ret = 0;   // Skip subject for attach/request in non-JAM
                     break;
                     }

                 bottom("Give the subject of the message..");
                 ret = getstring(5,13,curmsg->mis.subj,65,99,"",cfg.col[Centry], cfg.col[Cfinddata]);     /* Edit the SUBJ: field */
                 if(ret == RESET)
                   EditRequest(curmsg, areahandle->type);
                 else if(ret == ATTACH)
                   EditAttach(curmsg, areahandle->type);
                 else if(ret == SAVE)
                   SetSpecial(curmsg);
                 break;

         case 6:

                 ret = SetAttributes(curmsg, area->base, 1);
                 if(ret == RESET)
                   EditRequest(curmsg, areahandle->type);
                 else if(ret == ATTACH)
                   EditAttach(curmsg, areahandle->type);
                 else if(ret == SAVE)
                   SetSpecial(curmsg);
                 break;
         }

      PaintNewHeader(curmsg, areahandle->type, special);
      special = pUPDATE;

      switch(ret)
         {
         case 0:
         case DOWN:
         case TAB:
            what++;
            if(area->type == NEWS && what == 2)
              what = 5;
            if( ((what==2) || (what == 4)) && !address) what++;
            break;

         case UP:
         case BTAB:
            if(--what < 1) what=1;
            if( (what==4) && (area->type == NEWS) )
               what = 1;
            if( ((what==2) || (what == 4)) && !address ) what--;
            if( (what==5) &&
                (!(areahandle->type & MSGTYPE_JAM)) &&
                (curmsg->mis.attached || curmsg->mis.requested) ) what--;  // Skip subj. - non-JAM attach/request
            break;
         case ESC:
            return ABORT;
         default:
                ;

         }

      /* Check for file attach and verify files.. */

      if (address && (curmsg->mis.attr1 & (aFILE|aFRQ)) && (what > 6))
         {
         if((curmsg->mis.attr1 & aFILE) && curmsg->mis.attached == NULL)
           Message("Warning: attach bit set, but no files attached!", -1, 0, YES);

         if((curmsg->mis.attr1 & aFRQ) && curmsg->mis.requested == NULL)
           Message("Warning: request bit set, but no files requested!", -1, 0, YES);

         writefa(&curmsg->mis, areahandle, area, 1);
         }

      }

   return 0;

}

/* ----------------------------------------------- */

int SetAttributes (MMSG *curmsg, word base, int fullscreen)
{

   BOX *attrbox;
   MIS *mis = &curmsg->mis;

   savescreen();

   if(base & MSGTYPE_NET)
     {
     attrbox = initbox(8,16,22,63,cfg.col[Casframe],cfg.col[Castext],SINGLE,YES,' ');
     drawbox(attrbox);

     biprint(9,17,cfg.col[Castext], cfg.col[Casaccent], " ~P~rivate", '~');
     biprint(10,17,cfg.col[Castext], cfg.col[Casaccent], " ~C~rash", '~');
     biprint(11,17,cfg.col[Castext], cfg.col[Casaccent], " ~A~ttach file", '~');
     biprint(12,17,cfg.col[Castext], cfg.col[Casaccent], " ~R~equest file", '~');
     biprint(13,17,cfg.col[Castext], cfg.col[Casaccent], " ~K~ill/sent", '~');
     biprint(14,17,cfg.col[Castext], cfg.col[Casaccent], " ~L~ocal", '~');
     biprint(15,17,cfg.col[Castext], cfg.col[Casaccent], " ~H~old", '~');
     biprint(16,17,cfg.col[Castext], cfg.col[Casaccent], " sca~N~ned", '~');
     biprint(17,17,cfg.col[Castext], cfg.col[Casaccent], " ~F~orward", '~');
     biprint(18,17,cfg.col[Castext], cfg.col[Casaccent], " ~S~ent", '~');
     biprint(19,17,cfg.col[Castext], cfg.col[Casaccent], " recei~V~ed", '~');
     biprint(20,17,cfg.col[Castext], cfg.col[Casaccent], " ~D~irect", '~');
     biprint(21,17,cfg.col[Castext], cfg.col[Casaccent], " ~U~pdate req.", '~');

     biprint(9, 36,cfg.col[Castext], cfg.col[Casaccent], "~O~rphan", '~');
     biprint(11,36,cfg.col[Castext], cfg.col[Casaccent], "~Y~ return receipt", '~');
     biprint(10, 36,cfg.col[Castext], cfg.col[Casaccent], "~Q~ ret. receipt re[Q]uest", '~');

     print(13,36,cfg.col[Castext], "Extra 'flags':");

     biprint(15,36,cfg.col[Castext], cfg.col[Casaccent], "~E~rase file when sent", '~');
     biprint(16,36,cfg.col[Castext], cfg.col[Casaccent], "~X~ archive file when sent", '~');
     biprint(17,36,cfg.col[Castext], cfg.col[Casaccent], "~I~mmediate", '~');
     biprint(18,36,cfg.col[Castext], cfg.col[Casaccent], "~T~runcate file when sent", '~');
     biprint(19,36,cfg.col[Castext], cfg.col[Casaccent], "~W~ Lock", '~');
     biprint(20,36,cfg.col[Castext], cfg.col[Casaccent], "~M~ confirm receipt request", '~');
     }
   else
     {
     #ifdef __SENTER__
     attrbox = initbox(8,33,15,60,cfg.col[Casframe],cfg.col[Castext],SINGLE,NO,' ');
     #else
     attrbox = initbox(8,33,14,44,cfg.col[Casframe],cfg.col[Castext],SINGLE,NO,' ');
     #endif
     drawbox(attrbox);

     biprint(9,34, cfg.col[Castext], cfg.col[Casaccent], " ~P~rivate", '~');
     biprint(10,34, cfg.col[Castext], cfg.col[Casaccent], " ~L~ocal", '~');
     biprint(11,34, cfg.col[Castext], cfg.col[Casaccent], " sca~N~ned", '~');
     biprint(12,34, cfg.col[Castext], cfg.col[Casaccent], " ~S~ent", '~');
     biprint(13,34, cfg.col[Castext], cfg.col[Casaccent], " recei~V~ed", '~');
     #ifdef __SENTER__
     biprint(14,34, cfg.col[Castext], cfg.col[Casaccent], " Confir~M~ receipt request", '~');
     #endif
     }

   if(fullscreen)
     bottom("Press letter to toggle attributes, ~ENTER~ to continue..");
   else
     statusbar("Press letter to toggle attributes, ~ENTER~ to continue..");

   while(1)
      {
      if(!fullscreen)
        #ifdef __SENTER__
        vprint(1,40,cfg.col[Cmsgattribs], "%40.40s", attr_to_txt(mis->attr1, mis->attr2));
        #else
        vprint(0,40,cfg.col[Cmsgattribs], "%40.40s", attr_to_txt(mis->attr1, mis->attr2));
        #endif
      else
        PaintNewHeader(curmsg, base, pUPDATE);  // local, who cares (just attribs)

      switch(get_idle_key(1, GLOBALSCOPE))
         {
         case 328:   /* up */
             delbox(attrbox);
             putscreen();
             return UP;

         case 271:  /* BTAB */
             delbox(attrbox);
             putscreen();
             return BTAB;

         case 286:  /* ALT-A */
             delbox(attrbox);
             putscreen();
             return ATTACH;

         case 275:  /* ALT-R */
             delbox(attrbox);
             putscreen();
             return RESET;

         case 287:  /* ALT-S */
             delbox(attrbox);
             putscreen();
             return SAVE;

         case 27 :
             delbox(attrbox);
             putscreen();
             return ESC;

         case 'D':
         case 'd':
           mis->attr1 ^= aDIR;
           break;

         case 'W':
         case 'w':
           mis->attr1 ^= aLOK;
           break;

         case 'M':
         case 'm':
           mis->attr1 ^= aCFM;
           break;

         case 'I':
         case 'i':
             mis->attr1 ^= aIMM;
           break;

         case 'T':
         case 't':
             mis->attr1 ^= aTFS;
           break;

         case 'X':
         case 'x':
             mis->attr1 ^= aAS;
           break;

         case 'E':
         case 'e':
             mis->attr1 ^= aKFS;
           break;

		 case 'P' :
       case 'p' :
          mis->attr1 ^= aPRIVATE;
			 break;

		 case 'Q' :
       case 'q' :
          mis->attr1 ^= aRRQ;
			 break;

		 case 'Y' :
       case 'y' :
          mis->attr1 ^= aCPT;
			 break;

       case 'c' :
		 case 'C' :
          mis->attr1 ^= aCRASH;
			 break;

       case 's' :
		 case 'S' :
          mis->attr1 ^= aSENT;
			 break;

       case 'a' :
		 case 'A' :
          mis->attr1 ^= aFILE;
			 break;

       case 'f' :
		 case 'F' :
          mis->attr1 ^= aFWD;
			 break;

       case 'k' :
		 case 'K' :
          mis->attr1 ^= aKILL;
			 break;

       case 'l' :
		 case 'L' :
          mis->attr1 ^= aLOCAL;
			 break;

       case 'h' :
		 case 'H' :
          mis->attr1 ^= aHOLD;
			 break;

       case 'r' :
		 case 'R' :
          mis->attr1 ^= aFRQ;
			 break;

       case 'n' :
		 case 'N' :
          mis->attr1 ^= aSCANNED;
			 break;

       case 'v' :
		 case 'V' :
          mis->attr1 ^= aREAD;
			 break;

       case 'u':
       case 'U':
          mis->attr1 ^= aURQ;
          break;

       case 'o':
       case 'O':
          mis->attr1 ^= aORPHAN;
          break;

		 case 13 :
          delbox(attrbox);
          putscreen();
          return 0;
         }          /* switch */

      } /* while */
}



void statusbar(char *s)
{
/*   char temp[MAX_SCREEN_WIDTH];

   strcpy(temp, s);

   memset(temp+strlen(temp), ' ',maxx-strlen(temp));
   temp[maxx]='\0';

	print(maxy-1,0,cfg.col.msgbar,temp);*/

   biprinteol(maxy-1,0,cfg.col[Cmsgbar],cfg.col[Cmsgbaraccent],s,'~');
}



/* Check TO: address and all AKA's to pick the most appropriate one.. */


int matchaka(MIS *mis)
{
   int hits, i, firsthit = -1, found_zone_net=0;
   static int akano = -1;
   int currentmatcheszone = 0; /* The current address matches */
   AKAFORCE *thisone;

   if( (thisone=cfg.akaforce) != NULL )
     {
     while(thisone)
       {
       if(MaskMatch(&mis->destfido, &thisone->mask) != 0)  // Match?
         {
         if(addrcmp(&cfg.usr.address[thisone->aka], &mis->origfido)==0)
            return 0; // Found match, but no change needed..

         mis->origfido = cfg.usr.address[thisone->aka];
         return 1;   // Change was needed & forced
         }
       thisone = thisone->next;
       }
     }

   if((cfg.usr.status & AKAMATCHING) == 0) return 0; /* AKA matching is off.. */

   if(akano == -1)     /* No of AKA's not counted yet */
      {
      for(akano=0, i=0; i< tMAXAKAS; i++)  /* Count no of AKA's */
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
      if(cfg.usr.address[i].zone == mis->destfido.zone)
         {
         if(addrcmp(&cfg.usr.address[i], &mis->origfido)==0)
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
             (cfg.usr.address[i].zone == mis->destfido.zone) &&
             (cfg.usr.address[i].net  == mis->destfido.net )
           )
             {
             if(addrcmp(&cfg.usr.address[i], &mis->origfido)==0)
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

   if(addrcmp(&cfg.usr.address[firsthit], &mis->origfido)==0)
     return 0;    /* Current address, so no AKA change */

   /* Fill in new adress */

   mis->origfido = cfg.usr.address[firsthit];

   /* Return 1 to signal change */

   return 1;
}


/* ----------------------------------------------------------- */

int addrcmp(NETADDR *one, NETADDR *two)
{
  if(one->zone != two->zone)
    return (one->zone - two->zone);

  if(one->net != two->net)
    return (one->net - two->net);

  if(one->node != two->node)
    return (one->node - two->node);

  if(one->point != two->point)
    return (one->point - two->point);

  return 0;
}


// --------------------------------------------------------------

char * near strip_subject(char *s, int netmail)
{
   static char temp[72];
   char temp2[72];
   char *el;
   char name[MAXFILE], ext[MAXEXT];

   memset(temp, '\0', 72);

   // Path spec?
   if( netmail && (isalpha(*s)) && (s[1] == ':') && (s[2] == '\\' || s[2] == '/') )
     {
     strcpy(temp2, s);
     el = strtok(temp2, " ");
     while(el != NULL)
       {
       fnsplit(el, NULL, NULL, name, ext);
       strcat(temp, name);
       strcat(temp, ext);
       strcat(temp, " ");
       el=strtok(NULL, " ");
       }
     }
   else if(strncmpi(s, "re:", 3) == 0)
     {
     s += 3;
     while(*s == ' ' && *s != '\0')
       s++;
     strcpy(temp, s);
     }
   else strcpy(temp, s);

   return temp;

}

// -- Check if an address matches an address mask (akamatching force)

int near MaskMatch(NETADDR *adr, NETADDR *mask)
{
   if(
       (mask->zone != (word) -99) &&
       (adr->zone != mask->zone)
     )
     return 0;

   if(
       (mask->net != (word) -99) &&
       (adr->net != mask->net)
     )
     return 0;

   if(
       (mask->node != (word) -99) &&
       (adr->node != mask->node)
     )
     return 0;

   if(
       (mask->point != (word) -99) &&
       (adr->point != mask->point)
     )
     return 0;

   return 1;
}

// ========================================================

int aliaslist(MIS *mis)
{
  MACROLIST *thismacro;
  int count = 0, ret, i;
  char **alist;
  char temp[MAX_SCREEN_WIDTH];


  for(thismacro=cfg.firstmacro; thismacro; thismacro=thismacro->next)
    count++;

  if(count == 0) return -1;

  alist = mem_calloc(count+1, sizeof(char *));


  for(thismacro = cfg.firstmacro, i=0; thismacro; thismacro = thismacro->next, i++)
    {
    if(thismacro->usenet[0] != '\0')
        sprintf(temp, " %-8.8s - %-39.39s", thismacro->macro, thismacro->usenet);
    else
        sprintf(temp, " %-8.8s - %-39.39s", thismacro->macro, thismacro->toname);
    alist[i] = mem_strdup(temp);
    }

  ret = pickone(alist, 5, 15, maxy-2, 65);
  free_picklist(alist);

  if(ret != -1)       // find entry and put it in TO:
    {
    for(thismacro = cfg.firstmacro, i=0; i < ret; thismacro = thismacro->next, i++)
       /* nothing */ ;
    strcpy(mis->to, thismacro->macro);
    }

  return (ret == -1) ? -1 : 0;

}

// =============================================================

void PaintNewHeader(MMSG *curmsg, word type, word special)
{
   char temp[MAX_SCREEN_WIDTH], temp2[80];
   MIS *mis = &curmsg->mis;

   if(!(special & pUPDATE))
      PaintHeaderFrame();

   vprint(1, 13, cfg.col[Cfinddata], "%-65.65s", mis->from);

   if(type & (MSGTYPE_NET|MSGTYPE_ECHO))
     {
     if(special & pAKA)
       vprint(2, 13, cfg.col[Cfinddata], "%-51.51s (AKA matched)", FormAddress(&mis->origfido));
     else
       vprint(2, 13, cfg.col[Cfinddata], "%-65.65s", FormAddress(&mis->origfido));
     }

   if(!(type & MSGTYPE_NEWS))
     {
     if(mis->destinter[0] != '\0' && (type & (MSGTYPE_MAIL|MSGTYPE_NET)))
       {
       sprintf(temp2, " (via %s)",  mis->to);
       memset(temp, '\0', sizeof(temp));
       strncpy(temp, mis->destinter, 65 - strlen(temp2));
       strcat(temp, temp2);
       vprint(3, 13, cfg.col[Cfinddata], "%-65.65s", temp);
       }
     else // Not Internet gated message
       {
       vprint(3, 13, cfg.col[Cfinddata], "%-65.65s", mis->to);
       }

     if(type & MSGTYPE_NET)
       {
       if(special & pUNKNOWN)
         vprint(4, 13, cfg.col[Cfinddata], "%-47.47s (Unknown address)", FormAddress(&mis->destfido));
       else
         vprint(4, 13, cfg.col[Cfinddata], "%-65.65s", FormAddress(&mis->destfido));
       }
     }

   vprint(5, 13, cfg.col[Cfinddata], "%-65.65s", mis->subj);

   vprint(6 ,13, cfg.col[Cfinddata], "%-65.65s", attr_to_txt(mis->attr1, mis->attr2));

   if(special & pATTACH)
     {
     ShowAttached(mis);
     ShowRequested(mis);
     }

   if(curmsg->status & SPELLCHECK)
      printc(12, 65, cfg.col[Cfinddata], 'û');

   if(curmsg->status & SIGN)
      printc(11, 65, cfg.col[Cfinddata], 'û');

   if(curmsg->status & ENCRYPT)
      printc(10, 65, cfg.col[Cfinddata], 'û');

}

// ========================================================

void PaintHeaderFrame(void)
{
  BOX *box;

  box = initbox(0, 0, maxy-2, 79, cfg.col[Cfindpassive], cfg.col[Cfindtext], SINGLE, NO, ' ');
  drawbox(box);
  delbox(box);

  box = initbox(7, 38, maxy-4, 62, cfg.col[Cfindpassive], cfg.col[Cfindtext], SINGLE, NO, ' ');
  drawbox(box);
  delbox(box);

  printc(0, 11, cfg.col[Cfindpassive], 'Ñ');
  biprint(1, 1, cfg.col[Cfindtext], cfg.col[Cfindpassive], "Sender    ~³~", '~');
  biprint(2, 1, cfg.col[Cfindtext], cfg.col[Cfindpassive], "ÀÄ Address~³~", '~');
  biprint(3, 1, cfg.col[Cfindtext], cfg.col[Cfindpassive], "Recipient ~³~", '~');
  biprint(4, 1, cfg.col[Cfindtext], cfg.col[Cfindpassive], "ÀÄ Address~³~", '~');
  biprint(5, 1, cfg.col[Cfindtext], cfg.col[Cfindpassive], "Subject   ~³~", '~');
  biprint(6, 1, cfg.col[Cfindtext], cfg.col[Cfindpassive], "Attributes~³~", '~');

  printn(7, 0, cfg.col[Cfindpassive],"ÃÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´", 80);
  printn(8, 2, cfg.col[Cfindtext], "Attached files:", 15);
  printn(8,40, cfg.col[Cfindtext], "Requested files:", 16);
  biprint(9, 0, cfg.col[Cfindpassive], cfg.col[Cfindtext], "ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ ~Name         Password~ ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´", '~');
  printn(8,64, cfg.col[Cfindtext], "Special:", 8);
  printn(10, 38, cfg.col[Cfindpassive], "ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´", 25);

  printn(10, 64, cfg.col[Cfindtext], "[ ] Encrypt", 11);
  printn(11, 64, cfg.col[Cfindtext], "[ ] Sign", 8);
  printn(12, 64, cfg.col[Cfindtext], "[ ] Spellcheck", 14);

  printn(maxy-4, 0, cfg.col[Cfindpassive],"ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´", 80);
  biprint(maxy-3, 2, cfg.col[Cfindtext], cfg.col[Cfindaccent], "~ALT-A~: Attach files                   ~ALT-R~: Request files    ~ALT-S~: Special", '~');

}

// ============================================================

void ShowAttached(MIS *mis)
{
   ATTACHLIST *alist, *current;
   int i;

   if(mis->attached == NULL)
     {
     print(10, 2, cfg.col[Cfindtext], "- none -");
     return;
     }

   alist = MakeAttachList(mis->attached);

   for(current=alist, i=0; current && i<(maxy-5-9); current=current->next, i++)
     {
     vprint(10+i, 2, cfg.col[Cfindtext], "%-27.27s %4lu kB", current->name, current->size);
     }

   FreeAttachList(alist);

}
 
// ============================================================

void ShowRequested(MIS *mis)
{
   STRINGLIST *current;
   int i;

   if(mis->requested == NULL)
     {
     print(11, 40, cfg.col[Cfindtext], "- none -");
     return;
     }

   for(current=mis->requested, i=0; current && i<(maxy-5-10); current=current->next, i++)
     {
     vprint(11+i, 40, cfg.col[Cfindtext], "%-12.12s %-8.8s",
                                 current->s, current->pw ? current->pw : "");
     }


}

// ============================================================

ATTACHLIST * MakeAttachList(STRINGLIST *slist)
{
   ATTACHLIST *first, *current, *last;
   struct stat mystat;


   for(first=NULL, last=NULL; slist; slist=slist->next)
     {
     if(slist->s == NULL)
       continue;

     current = mem_calloc(1, sizeof(FLIST));
     if(!first)
       first = current;
     else
       last->next = current;
     last = current;

     if(strlen(slist->s) < 28)
        strcpy(current->name, slist->s);
     else
        strcpy(current->name, CompressPath(slist->s));

     if(stat(slist->s, &mystat) != -1)
       current->size = (long) (mystat.st_size/1024);
     }

   return first;

}

// ===============================================================

void FreeAttachList(ATTACHLIST *l)
{
   ATTACHLIST *last;

   while(l)
     {
     last = l->next;
     mem_free(l);
     l = last;
     }
}

// ===============================================================

char * CompressPath(char *s)
{
  static char temp[40];
  char temp2[40];
  char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
  size_t len;


  _splitpath(s, &drive, &dir, &fname, &ext);

  strcpy(temp, drive);
  // Adjust long filenames (OS/2 HPFS)
  if(strlen(fname) > 15) fname[15] = '\0';
  if(strlen(ext) > 5) ext[5] = '\0';
  len = strlen(fname) + strlen(ext) + strlen(drive);
  Strip_Trailing(dir, '\\');
  strncpy(temp2, dir, 24-len);
  temp2[24-len] = '\0';
  sprintf(temp, "%s%s..\\%s%s", drive, temp2, fname, ext);

  return temp;

}

// ===============================================================

void EditRequest(MMSG *curmsg, word type)
{
  STRINGLIST *current, *highlighted;
  int i, rows, howmany, colour;
  int curline=0, start=0;
  int retval;
  char filename[40], password[20];
  MIS *mis = &curmsg->mis;

  if(!(type & MSGTYPE_NET))
    {
    Message("Requests can only be entered in netmail areas!", -1, 0, YES);
    return;
    }

  if((mis->attached != NULL) && !(type & MSGTYPE_JAM))
    {
    Message("Simultaneous attach & request only in JAM areas!", -1, 0, YES);
    return;
    }

  start:

  if(mis->requested == NULL)
    {
    print(11, 40, cfg.col[Cfindtext], "- none -");
    stuffkey(338);
    }


  for(current = mis->requested, howmany=0; current; current = current->next)
      howmany++;

  rows = howmany > (maxy-5-10) ? (maxy-5-10) : howmany;

  if(rows < (maxy-5-10))
     ClsRectWith(11+rows,40,maxy-5,61,cfg.col[Cfinddata],' ');

  while(1)
    {
    bottom(" ~INS~: add request   ~DEL~: Delete request   ~ctrl-enter~: Accept");

    if(howmany > 0)
      {
      // find first to display
      for(i=0, current=mis->requested; i<start; i++, current=current->next)
        /* nothing */ ;

      // Adjust for impossible values
      if(curline > (howmany-1)) curline = howmany-1;

      for(i=0; i<rows; i++, current=current->next)
        {
        if(i+start == curline)
          {
          highlighted = current;
          colour = cfg.col[Cfindhigh];
          MoveXY(41, 11+i+1);
          }
        else
          colour = cfg.col[Cfindtext];

        vprint(11+i, 40, colour, "%-12.12s %-8.8s", current->s, current->pw);
        }
      }

    switch(get_idle_key(1, GLOBALSCOPE))
       {
       case 336:      /* down */
           if(howmany == 0) break;
           if(curline < (howmany-1))
              curline++;
           if(curline > (start+rows-1))
              start++;
           break;

         case 328:      /* up */
           if(howmany == 0) break;
           if(curline)
              curline--;
           if(curline<start) start--;
           break;

         case 327:      /* home */
           curline=0; start=0;
           break;

         case 335:      /* end */
           if(howmany == 0) break;
           curline = howmany-1;
           start  = howmany-rows;
           break;

         case 329:        /* Page Up */
           if(howmany == 0) break;
           curline -= rows;
           if(curline < 0)
              curline = 0;
           start -= rows;
           if(start < 0) start=0;
           if(curline < start)
              start = curline;
           break;

         case 337:       /* Page Down */
           if(howmany == 0) break;
           curline += rows;
           if(curline > (howmany-1))
              curline = howmany-1;
           start += rows;
           if(start > howmany-rows)
              start = howmany-rows;
           break;

//          case 315:        /* F1 */
//             show_help(3);
//             break;

         case 338:        /* Insert */

             memset(filename, '\0', 40);
             memset(password, '\0', 20);
             retval = EditOneRequest(filename, password);

             if(filename[0] == '\0' || retval == ESC)
               {
               if(mis->requested == NULL) stuffkey(10);
               break;
               }

             mis->requested = AddToStringList(mis->requested,
                                              filename,
                                              password[0] == '\0' ? NULL : password,
                                              0);
             goto start;

         case 13:

            if(howmany == 0) break;

            current = mis->requested;
            for(i=0; i<curline; i++)
              {
              if(current->next == NULL)
                {
                Message("Attempt to edit non-existant element from stringlist!", -1, 0, YES);
                break;
                }
              current = current->next;
              }

            if(current->s) strcpy(filename, current->s);
            if(current->pw)
               strcpy(password, current->pw);
            else
               memset(password, '\0', 20);

            retval = EditOneRequest(filename, password);
            if(retval == ESC)
              break;

            if(filename[0] == '\0')
              {
              stuffkey(339);
              break;
              }

            if(current->s) mem_free(current->s);
            current->s = mem_strdup(filename);
            if(current->pw) mem_free(current->pw);
            if(password[0] != '\0')
              current->pw = mem_strdup(password);
            else
              current->pw = NULL;

            break;

         case 339:        /* Delete */
            if(howmany == 0) break;
            RemoveElement(&mis->requested, curline);

            if(howmany >= (maxy-5-9))  // If more than one screen full..
              {
              if(start && (start > (howmany-maxy-5-9))) // Last line still filled?
                 start--;
              if(curline >= (start+rows)) curline--;
              }

            goto start;

         case  9:         /* <tab> */
         case 10:         /* ctrl-enter */
         case 27:         /* esc */
           if(mis->requested != NULL)
              mis->attr1 |= aFRQ;
           else
              mis->attr1 &= ~aFRQ;
           PaintNewHeader(curmsg, MSGTYPE_ECHO, (pATTACH|pUPDATE));
           return;

         default:
            break;
         }
    }


}

// ==============================================================

void EditAttach(MMSG *curmsg, word type)
{
  ATTACHLIST *alist, *current, *highlighted;
  int i, rows, howmany, colour;
  int curline=0, start=0;
  BOX *getnamebox;
  int retval;
  char mask[80];
  MIS *mis = &curmsg->mis;
  STRINGLIST *curelement;
  struct stat mystat;


  if(!(type & (MSGTYPE_NET|MSGTYPE_LOCAL|MSGTYPE_MAIL)))
    {
    Message("Attaches can only be entered in netmail areas!", -1, 0, YES);
    return;
    }

  if((mis->requested != NULL) && !(type & MSGTYPE_JAM))
    {
    Message("Simultaneous attach & request only in JAM areas!", -1, 0, YES);
    return;
    }

  start:

  alist = MakeAttachList(mis->attached);

  if(mis->attached == NULL)
    {
    print(10, 2, cfg.col[Cfindtext], "- none -");
    stuffkey(338);
    }


  for(current = alist, howmany=0; current; current = current->next)
      howmany++;

  rows = howmany > (maxy-5-9) ? (maxy-5-9) : howmany;

  if(rows < (maxy-5-9))
     ClsRectWith(10+rows,1,maxy-5,37,cfg.col[Cfinddata],' ');


  while(1)
    {
    bottom(" ~Ins~: Add attach   ~Del~: Delete attach   ~ctrl-enter~: Accept");

    if(howmany > 0)
      {
      // find first to display
      for(i=0, current=alist; i<start; i++, current=current->next)
        /* nothing */ ;

      // Adjust for impossible values
      if(curline > (howmany-1)) curline = howmany-1;

      for(i=0; i<rows; i++, current=current->next)
        {
        if(i+start == curline)
          {
          highlighted = current;
          colour = cfg.col[Cfindhigh];
          MoveXY(3, 10+i+1);
          }
        else
          colour = cfg.col[Cfindtext];

        vprint(10+i, 2, colour, "%-27.27s %4lu kB", current->name, current->size);
        }
      }

    switch(get_idle_key(1, GLOBALSCOPE))
       {
       case 336:      /* down */
           if(howmany == 0) break;
           if(curline < (howmany-1))
              curline++;
           if(curline > (start+rows-1))
              start++;
           break;

         case 328:      /* up */
           if(howmany == 0) break;
           if(curline)
              curline--;
           if(curline<start) start--;
           break;

         case 327:      /* home */
           curline=0; start=0;
           break;

         case 335:      /* end */
           if(howmany == 0) break;
           curline = howmany-1;
           start  = howmany-rows;
           break;

         case 329:        /* Page Up */
           if(howmany == 0) break;
           curline -= rows;
           if(curline < 0)
              curline = 0;
           start -= rows;
           if(start < 0) start=0;
           if(curline < start)
              start = curline;
           break;

         case 337:       /* Page Down */
           if(howmany == 0) break;
           curline += rows;
           if(curline > (howmany-1))
              curline = howmany-1;
           start += rows;
           if(start > howmany-rows)
              start = howmany-rows;
           break;

//          case 315:        /* F1 */
//             show_help(3);
//             break;

         case 338:        /* Insert */
             getnamebox = initbox(9, 2, 14, 77, cfg.col[Cfindactive], cfg.col[Cfindtext], SINGLE, YES, ' ');
             drawbox(getnamebox);
             boxwrite(getnamebox,0,1,"Filename or mask to attach:");
             memset(mask, '\0', sizeof(mask));

             do {
               retval=getstring(12, 4, mask, 72, 71, VFNWW,cfg.col[Centry], cfg.col[Cfindtext]);
             } while(retval != 0 && retval != ESC);

             delbox(getnamebox);

             if(retval == ESC)
               {
               if(mis->attached == NULL) stuffkey(10);
               break;
               }

             check_attach(mis, mask, type & MSGTYPE_LOCAL);

             FreeAttachList(alist);
             goto start;

         case 13:

             if(howmany == 0) break;

             curelement = mis->attached;
             for(i=0; i<curline; i++)
               {
               if(curelement->next == NULL)
                 {
                 Message("Attempt to edit non-existant element from stringlist!", -1, 0, YES);
                 break;
                 }
               curelement = curelement->next;
               }

             getnamebox = initbox(9, 2, 14, 77, cfg.col[Cfindactive], cfg.col[Cfindtext], SINGLE, YES, ' ');
             drawbox(getnamebox);
             boxwrite(getnamebox,0,1,"Filename or mask to attach:");
             strncpy(mask, curelement->s, 71);
             mask[71] = '\0';

             do {
               retval=getstring(12, 4, mask, 72, 71, VFNWW,cfg.col[Centry], cfg.col[Cfindtext]);
             } while(retval != 0 && retval != ESC);

             delbox(getnamebox);

             if(retval != ESC && mask[0] != '\0')
               {
               mem_free(curelement->s);
               curelement->s = mem_strdup(mask);
               strcpy(highlighted->name, strlen(mask) < 28 ? mask : CompressPath(mask));
               if(stat(curelement->s, &mystat) == -1)
                  Message("Warning, file does not exist!", -1, 0, YES);
               else
                  highlighted->size = (long) (mystat.st_size/1024);
               }

             break;


         case 339:        /* Delete */
            if(howmany == 0) break;
            FreeAttachList(alist);
            RemoveElement(&mis->attached, curline);

            if(howmany >= (maxy-5-9))  // If more than one screen full..
              {
              if(start && (start > (howmany-maxy-5-9))) // Last line still filled?
                 start--;
              if(curline >= (start+rows)) curline--;
              }

            goto start;

         case  9:         /* <tab> */
         case 10:         /* ctrl-enter */
         case 27:         /* esc */
           FreeAttachList(alist);
           if(mis->attached != NULL)
              mis->attr1 |= aFILE;
           else
              mis->attr1 &= ~aFILE;
           PaintNewHeader(curmsg, MSGTYPE_ECHO, (pATTACH|pUPDATE));
           return;

         default:
            break;
         }
    }

}

// ==============================================================

void RemoveElement(STRINGLIST **start, int elno)
{
   int i;
   STRINGLIST *current = *start,
              *prev;

   if(*start == NULL) return;

   for(i=0; i<elno; i++)
     {
     if(current->next == NULL)
       {
       Message("Attempt to remove non-existant element from stringlist!", -1, 0, YES);
       return;
       }
     prev = current;
     current = current->next;
     }

   if(current == *start)      // First element?
     *start = (*start)->next;
   else
     prev->next = current->next;

   if(current->pw) mem_free(current->pw);
   if(current->s) mem_free(current->s);
   mem_free(current);

}

// ====================================================================

void SetSpecial(MMSG *curmsg)
{
  int whatnow = 0;
  int key;

  bottom(" Press ~enter~ or ~space~ to toggle status, or ~esc~ to end.");



  do
    {
    printn(10, 68, whatnow == 0 ? cfg.col[Cfindhigh] : cfg.col[Cfindtext], "Encrypt   ", 10);
    printn(11, 68, whatnow == 1 ? cfg.col[Cfindhigh] : cfg.col[Cfindtext], "Sign      ", 10);
    printn(12, 68, whatnow == 2 ? cfg.col[Cfindhigh] : cfg.col[Cfindtext], "Spellcheck", 10);

    if(curmsg->status & SPELLCHECK)
       printc(12, 65, cfg.col[Cfinddata], 'û');
    else
       printc(12, 65, cfg.col[Cfinddata], ' ');

    if(curmsg->status & SIGN)
       printc(11, 65, cfg.col[Cfinddata], 'û');
    else
       printc(11, 65, cfg.col[Cfinddata], ' ');

    if(curmsg->status & ENCRYPT)
       printc(10, 65, cfg.col[Cfinddata], 'û');
    else
       printc(10, 65, cfg.col[Cfinddata], ' ');

    key = get_idle_key(1, GLOBALSCOPE);

    switch(key)
      {
      case 336:      /* down */
        if(++whatnow > 2) whatnow = 2;
        break;

      case 328:      /* up */
        if(--whatnow < 0) whatnow = 0;
        break;

      case 327:      /* home */
        whatnow = 0;
        break;

      case 335:      /* end */
        whatnow = 2;
        break;

      case 32:
      case 13:
        switch(whatnow)
          {
          case 0:
            curmsg->status ^= ENCRYPT;
            break;
          case 1:
            curmsg->status ^= SIGN;
            break;
          case 2:
            curmsg->status ^= SPELLCHECK;
            break;
          }
        break;
      }

    } while(key != 27);

  printn(10, 68, cfg.col[Cfindtext], "Encrypt   ", 10);
  printn(11, 68, cfg.col[Cfindtext], "Sign      ", 10);
  printn(12, 68, cfg.col[Cfindtext], "Spellcheck", 10);


}

// =====================================================================

int EditOneRequest(char * filename, char * password)
{
  BOX *getnamebox;
  int retval;


  getnamebox = initbox(10, 5, 15, 75, cfg.col[Cfindactive], cfg.col[Cfindtext], SINGLE, YES, ' ');
  drawbox(getnamebox);
  boxwrite(getnamebox,1,1,"Filename to request:");
  boxwrite(getnamebox,2,1,"Password to use:");

  if(password[0] != '\0')
     print(13, 28, cfg.col[Cfindtext], password);

  do {
    retval=getstring(12, 28, filename, 39, 39, VFNWW,cfg.col[Centry], cfg.col[Cfindtext]);
  } while(retval != 0 && retval != ESC);

  if(filename[0] == '\0' || retval == ESC)
    {
    delbox(getnamebox);
    return ESC;
    }

  do {
    retval=getstring(13, 28, password, 19, 19, VFNWW,cfg.col[Centry], cfg.col[Cfindtext]);
  } while(retval != ESC && retval != 0);

  delbox(getnamebox);

  if(retval == ESC)
    {
    password[0] = '\0';
    return ESC;
    }

  return 0;

}
