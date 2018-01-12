#include "includes.h"

void       get_JAM_thread(MMSG *curmsg, MSG *areahandle);
int        ConfirmReceipt(MSG *areahandle, AREA *area, MIS *mis, dword no);
RAWBLOCK * CreateCFMbody(MIS * mis, MIS * newmis);


/* This routine will first check for msgid, and also get the addres from there */
/* If there is no MSGID, it will try to find the origin and get it from there  */
/* It now also scans for replyto/replyaddr kludges, and Flags DIR stuff */


void checkaddress(MMSG *curmsg, AREA *area)

{
   NETADDR afz;
   LINE *lptr = curmsg->txt,
        *lineptr,
        fakeline;       // Used if a TO: line is deleted..
   char *begin, *end, temp[300], *tmpptr;
   int have_address=0;
   int lines = 0;

   memset(&afz, '\0', sizeof(NETADDR));
   memset(temp, '\0', sizeof(temp));

   if (area->type != NETMAIL && area->type != MAIL)
      memset(&curmsg->mis.origfido, '\0', sizeof(NETADDR));
   else have_address = 1; /* for netmail, the address is OK already */

   for(; lptr; lptr = lptr->next)
      {
      if(lptr->status & KLUDGE)
         {
         if(strncmp(lptr->ls, "\01MSGID:", 7) == 0)
            {
            if(curmsg->id[0] == '\0') /* Only if we don't have one - next might be MSGID in msgtext.. ?! */
               strcpy(curmsg->id, lptr->ls+8);
            if (area->type != NETMAIL && area->type != MAIL)
               {
               tmpptr=curmsg->id;
               while( (!(isdigit(*tmpptr))) && (*tmpptr!=0) ) tmpptr++;
               sscanf(tmpptr, "%hu:%hu/%hu.%hu", &afz.zone, &afz.net, &afz.node, &afz.point);
               if( (afz.zone != 0) && (afz.net != 0) )
                 {
                 curmsg->mis.origfido = afz;
                 have_address = 1; /* We have address, could stop */
                 }
               }
            }
         else if(strnicmp(lptr->ls, "\01ORGANIZATION: ", 15) == 0)
            {
            strncpy(curmsg->org, lptr->ls+15, 80);
            }
          else if(strncmpi(lptr->ls, "\01REPLYTO", 8) == 0)
            {
            strcpy(temp, lptr->ls+9);
            if((tmpptr=strtok(temp, " \t\n\r")) != NULL)
               {
               sscanf(tmpptr, "%hu:%hu/%hu.%hu", &curmsg->rep_addr.zone,
                                          &curmsg->rep_addr.net,
                                          &curmsg->rep_addr.node,
                                          &curmsg->rep_addr.point);

               if((tmpptr=strtok(NULL, "\n\r")) != NULL)
                  {
                  while(*tmpptr==' ') tmpptr++;
                  strcpy(curmsg->rep_gate, tmpptr);
                  }
               }

            } /* ^AREPLYTO */

         else if(strncmpi(lptr->ls, "\01REPLYADDR", 10) == 0)
            {
            strcpy(temp, lptr->ls+11);
            tmpptr=temp;
            while(*tmpptr == ' ') tmpptr++;
            strcpy(curmsg->rep_name, tmpptr);
            }

         if(                 /* FSC35 'replyto' kludges are both there */
              (curmsg->rep_addr.zone != 0)  &&
              (curmsg->rep_gate[0] != '\0') &&
              (curmsg->rep_name[0] != '\0')
           )
           curmsg->status |= REPLYTO; /* So remember this (and use in ALT-N) */
         }

      else if(lptr->status & ORIGIN)
        {
        end   = strrchr(lptr->ls, ')');       /* Closing bracket for address */
        begin = strrchr(lptr->ls, '(');       /* Opening bracket for address */
        if ( begin &&
             end &&
             (end > begin) &&
             (*(end+1) == '\0') &&
             (strchr(begin, '/') != NULL) )
           {
//           while( (!isdigit(*begin)) && (begin<end))
//              begin++;                                /* find first digit */
//           while( (!isdigit(*end)) && (begin<end) )
//              end--;

           tmpptr = strchr(begin, '/');       // Begin slash (like 281/527)
           if(!tmpptr) break;

           while( (isdigit(*tmpptr) || *tmpptr==':' || *tmpptr=='/') &&
                  (tmpptr > begin) ) // Go tho start of address
             tmpptr--;
           if(!isdigit(*tmpptr)) tmpptr++;
           strcpy(temp, tmpptr);
           tmpptr = strtok(temp, " ()@\t\r\n");
           if(tmpptr && strchr(tmpptr, '/') != NULL)
              {
              address_expand(temp, &afz, -1);
              if (area->type != NETMAIL && area->type != MAIL)
                 curmsg->mis.origfido = afz;
              }
           }
        }
      else
        {
        lines++;
        if( (area->type == MAIL || area->type == NETMAIL) &&
            (lines < 5) &&
            (curmsg->mis.destinter[0] == '\0') &&
            (strncmpi(lptr->ls, "to:", 3) == 0) )
           {
           if( (strcmpi(curmsg->mis.to, "uucp") == 0) ||
               (strcmpi(curmsg->mis.to, "postmaster") == 0) ||
               (strcmpi(curmsg->mis.to, cfg.usr.uucpname) == 0) )
               {
               // Addressed to the gate. Get this TO: line from body!
               tmpptr = lptr->ls+3;
               while(*tmpptr==' ') tmpptr++;
               strncpy(curmsg->mis.destinter, tmpptr, 99);

               // If the next line is empty, remove it.
               lineptr = lptr->next;
               if(lineptr && lineptr->ls[0] == '\0')
                 {
                 lptr->next = lineptr->next;
                 if(lineptr->next)
                   lineptr->next->prev = lptr;
                 if(lineptr->ls) mem_free(lineptr->ls);
                 mem_free(lineptr);
                 }

               // Remove this TO: line from the body..
               if(lptr->prev)
                  lptr->prev->next = lptr->next;
               else
                  curmsg->txt = lptr->next;
               if(lptr->next) lptr->next->prev = lptr->prev;
               fakeline.next = lptr->next;
               if(lptr->ls) mem_free(lptr->ls);
               mem_free(lptr);
               lptr = &fakeline;
               }
           }
         else if(strnicmp(lptr->ls, "ORGANIZATION: ", 14) == 0)
            strncpy(curmsg->org, lptr->ls+14, 80);
         }
      }

}


#ifdef __NEVER__
void pascal normalize(char *s)
{
    char   *tmp = s;

    while (*s)
        {
        if ((unsigned) (0xff & *s) == (unsigned) 0x8d)
            s++;
        else if (*s == '\n')
            s++;
        else
            *tmp++ = *s++;
        }
    *tmp = '\0';
}
#endif


void get_JAM_thread(MMSG *curmsg, MSG *areahandle)
{
   long current, nextread;
   int pos = 1;
   MSGH *msghandle;
   MIS mis;

   if(curmsg->mis.replies[0] == 0)  /* No replies */
      return;

   current = MsgGetCurMsg(areahandle);
   nextread  = curmsg->mis.replies[0];

   while(nextread && (pos < 9))
     {
     if( (msghandle=MsgOpenMsg(areahandle, MOPEN_READ, nextread)) == NULL)
        break;

     if(MsgReadMsg(msghandle, &mis, 0L, 0L, NULL, 0L, NULL) == -1)
        {
        Message("Error reading reply-chain message!", -1, 0, YES);
        MsgCloseMsg(msghandle);
        showerror();
        break;
        }

     MsgCloseMsg(msghandle);
    
     if(nextread == mis.nextreply) // Loop?!?!
       {
       FreeMIS(&mis);
       break;
       }

     nextread = curmsg->mis.replies[pos++] = mis.nextreply;
     FreeMIS(&mis);
     }

   if((msghandle=MsgOpenMsg(areahandle, MOPEN_READ, current)) != NULL) /* Reset current MsgApi msg */
      MsgCloseMsg(msghandle);

}



/* ----------------------------------------------- */
/* -  Update a message to the 'received' status  - */
/* ----------------------------------------------- */


int MarkReceived(MSG *areahandle, AREA *area, dword no, int not, int hasCFM)
{
  MIS *tempmis    = NULL;
  MSGH *msghandle = NULL;
  int didlock = 0;
  int retval = -1;
  int DoConfirmReceipt = 0;
 

  if(!areahandle->locked)
    {
    if(AttemptLock(areahandle) == -1)
       return retval;
    didlock=1;
    }

  tempmis = mem_calloc(1, sizeof(MIS));

  if((msghandle = MsgOpenMsg(areahandle, MOPEN_RW, no)) == NULL)
      goto Exit;

  if(MsgReadMsg(msghandle, tempmis, 0L, 0L, NULL, 0L, NULL) == -1)
      goto Exit;

  if(not)
     tempmis->attr1 &= ~aREAD;
  else
     {
     tempmis->attr1 |= aREAD;
     if(tempmis->msgreceived == 0L)
        tempmis->msgreceived = JAMsysTime(NULL);
     }

//  if(area->base & MSGTYPE_SDM)
//     {
//     time(&time_now);
//     tmdate=localtime(&time_now);
//     temphdr->date_arrived=(TmDate_to_DosDate(tmdate,&combo))->msg_st;
//     }

  // Now check the 'CFM' flag. If it is set, and we are not resetting
  // the 'read' flag, *and* we are configured to reply to messages
  // with this flag, then we can toggle the flag off and set remember
  // to write the confirmation!

  if( !not &&
      hasCFM &&
      (cfg.usr.status & REPLYTOCFM) &&
      (area->type == NETMAIL || area->type == LOCAL || area->type == MAIL) )
    {
    tempmis->attr1 &= ~aCFM;     // Toggle the flag off
    DoConfirmReceipt = 1;
    }

  if(MsgWriteMsg(msghandle, 0, tempmis, NULL, 0L, 0L, 0L, NULL) == -1)
      goto Exit;

  MsgCloseMsg(msghandle);
  msghandle = NULL;

  if(DoConfirmReceipt)
     retval = ConfirmReceipt(areahandle, area, tempmis, no);
  else
     retval = 0;

  Exit:

  if(retval == -1) showerror();

  if(msghandle)
     MsgCloseMsg(msghandle);

  if(tempmis)
    {
    FreeMIS(tempmis);
    mem_free(tempmis);
    }

  if(didlock)
     MsgUnlock(areahandle);

  return retval;

}

// ==============================================================
//
//   Write a confirmation to a message that had the CFM flag.
//   This function is called from a situation where the
//   area ('areahandle') is already locked!
//
//   Returns 0 on success, -1 on error.
//
// ==============================================================

int ConfirmReceipt(MSG *areahandle, AREA *area, MIS *mis, dword no)
{
   MIS *newmis;
   RAWBLOCK *blk = NULL;
   char *kludges;
   int retval = 0;
   MIS *tempmis    = NULL;
   MSGH *msghandle = NULL;


   // Read the message (now in RDHDR mode, so we get header fields),
   // including to:/from: fields in JAM..

   if((msghandle = MsgOpenMsg(areahandle, MOPEN_RDHDR, no)) != NULL)
     {
     MsgReadMsg(msghandle, mis, 0L, 0L, NULL, 0L, NULL);
     MsgCloseMsg(msghandle);
     }

   // First we create and fill the header.

   newmis = mem_calloc(1, sizeof(MIS));

   newmis->attr1 = aLOCAL | aPRIVATE;
   strcpy(newmis->from, cfg.usr.name[custom.name].name);
   strcpy(newmis->to, mis->from);
   newmis->origfido = cfg.usr.address[custom.aka];
   newmis->destfido = mis->origfido;
   strcpy(newmis->subj, "Confirmation of receipt.");
   if(area->type == NETMAIL)
         matchaka(newmis);
   newmis->msgwritten = JAMsysTime(NULL);

   // The we create the body of the message.

   blk = CreateCFMbody(mis, newmis);
   if(blk == NULL)
     {
     mem_free(newmis);
     return 0;
     }

   kludges = MakeKludge(NULL, newmis, area->type == NETMAIL ? 1 : 0);

   retval = SaveMessage(areahandle, area, newmis, blk, kludges, 0, 0);

   mem_free(kludges);
   mem_free(newmis);

   if(blk) FreeBlocks(blk);

   return retval;

}

// ==============================================================

RAWBLOCK * CreateCFMbody(MIS * mis, MIS * newmis)
{
   XFILE *infile;
   RAWBLOCK *blk;
   char *line;

   infile = xopen(cfg.usr.cfmfile);
   if(!infile)
     {
     sprintf(msg, "Can't open %0.32s to create receipt confirmation", cfg.usr.cfmfile);
     Message(msg, -1, 0, YES);
     return NULL;
     }

   blk = InitRawblock(2048, 1024, 4096);
   while(line=xgetline(infile))
     {
     AddToBlock(blk, expand(line, mis, newmis), -1);
     AddToBlock(blk, "\r", -1);
     }

   xclose(infile);

   return blk;

}

// ==============================================================

