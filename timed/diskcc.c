
#include "includes.h"

/* prototypes */

typedef struct _hdrlist
{

   MIS               mis;
   int               hidden;
   int               attaches;
   int               requests;

}  HDRLIST;

typedef struct
{

   char filename[30];
   char password[30];

} REQUESTEDFILE;


// static HDRLIST *firsthdr, *last;
int doCCnow = 0;
FILE * HdrFile = NULL;


typedef struct _arealist
{

   AREA             * areaptr;
   struct _arealist * next;

}  AREALIST;

static AREALIST *firstarea, *lastarea;

char nowcc, nowxc, nowhc;

void      check_element(char *charptr, MIS *header, AREA *area);
void      write_one(MSG *areahandle, AREA *area, MIS *curheader, char *msgbuf);
void      get_bomb_list(char *charptr, MIS *header, AREA *area);
HDRLIST * get_copy(MIS *header);
void      WriteCCs(MSG *areahandle, AREA *area, RAWBLOCK **first, MIS *header);
void      WriteXCs(AREA *area, RAWBLOCK **first, MIS *header);
void      AddToArealist(char *charptr);
void      FixEnd(RAWBLOCK **first, AREA *area);
int       which_aka(NETADDR *addr);
int       MoreToCome(HDRLIST *thisone);


// Header list on file functions..

void ClearHeaderList(void);
int  OpenHeaderList(void);
int  ReadFirstHeader(HDRLIST * hdr);
int  ReadNextHeader(HDRLIST *hdr);
int  WriteHeader(HDRLIST *hdr);

// ----

void check_cc(AREA *area, MSG *areahandle, MIS *mis, RAWBLOCK **first)
{

   char *start, *charptr, *nextnl, *nextel;
   char temp[256];
   char *end;
   size_t howmuch;


   // !!!!!!!!!!!!!!!!!!!!!
   // Always shut off CCVERBOZE for this disk based header stuff!


   cfg.usr.status &= ~CCVERBOSE;


   if(*first == NULL)
      return;

   firstarea = lastarea = NULL;
   ClearHeaderList();

   start = (*first)->txt;

   if(start == NULL) return;

   while( (nextnl = strchr(start, '\r')) != NULL)  /* analyse line by line.. */
      {
      memset(temp, '\0', sizeof(temp));
      howmuch = min(255, (size_t)(nextnl - start));
      strncpy(temp, start, howmuch);    /* copy line (strtok messes it up) */
      end = temp + sizeof(temp);
      charptr = temp;
      start = nextnl+1;

      while(*charptr == ' ' || *charptr ==  '\r')  /* skip leading space */
         charptr++;

      nowcc = nowxc = nowhc = 0;

      if(strncmpi(charptr, "CC:", 3) == 0)   /* CC: ? */
        {
        if(area->type == ECHOMAIL)
           break;
        nowcc = 1;
        }
      else if(strncmpi(charptr, "HC:", 3) == 0)   /* HC: ? */
        {
        if(area->type == ECHOMAIL)
           break;
        nowcc = 1;
        nowhc = 1;
        }
      else
        {
        if(strncmpi(charptr, "XC:", 3) != 0)   /* No CC:, return */
          break;
        nowxc = 1;
        }

      strcpy((*first)->txt, start);
      start = (*first)->txt;

      charptr = charptr + 3;  /* Skip CC: chars */

      while(*charptr == ' ')  /* skip space again */
         charptr++;

      charptr = strtok(charptr, ",");     /* get elements, separated by comma's */
      if(charptr != NULL)
         nextel  = charptr + strlen(charptr) + 1;

      while(charptr)
        {
        while(*charptr == ' ') charptr++;
        if(nowcc)
          {
          if(*charptr == '<')
             get_bomb_list(charptr+1, mis, area);
          else
             check_element(charptr, mis, area);
          }
        else
          AddToArealist(charptr);

        if(nextel <= end)
           {
           charptr = strtok(nextel, ",");
           if(charptr)
              nextel  = charptr + strlen(charptr) + 1;
           }
        else charptr=NULL;
        }

      }

   if(doCCnow)          // Only if we actually have any CC:'s to write!
      WriteCCs(areahandle, area, first, mis);

   if(firstarea)
      WriteXCs(area, first, mis);

   // mem_free memory taken by list of headers...

//   while(firsthdr)
//     {
//     last = firsthdr->next;
//     if(firsthdr->mis)
//        {
//        FreeMIS(firsthdr->mis);
//        mem_free(firsthdr->mis);
//        }

//     mem_free(firsthdr);
//     firsthdr = last;
//     }

   while(firstarea)
     {
     lastarea = firstarea->next;
     mem_free(firstarea);
     firstarea = lastarea;
     }

   ClearHeaderList();

}


/* -------------------------------------------------------------- */
/* Here we look at the element, try to expand it to a full header */
/* modified from the (passed) original header                     */
/* If we can't make it, we return NULL, else the created header   */
/* -------------------------------------------------------------- */


void check_element(char *el, MIS *mis, AREA *area)
{
   HDRLIST *thisheader;
   char *sep;
   char elcopy[80], *element;

   strcpy(elcopy, el);
   element = elcopy;


   thisheader = get_copy(mis);
   memset(thisheader->mis.to, '\0', sizeof(thisheader->mis.to));

   while(*element == ' ')
      element++;

   while(element[strlen(element)-1] == ' ')
      element[strlen(element)-1] = '\0';

   // Is this a 'hidden CC:? */

   if(nowhc == 1)
     thisheader->hidden = 1;

   if(*element == '#')
     {
     element++;
     thisheader->hidden = 1;
     }

   /* we check if there is an explicit address, using a '#' */

   if((sep = strchr(element, '#')) != NULL)
      {
      strncpy(thisheader->mis.to, element, (size_t) (sep - element));
      address_expand(sep+1, &thisheader->mis.destfido, which_aka(&mis->origfido));
      WriteHeader(thisheader);
      FreeMIS(&thisheader->mis);
      mem_free(thisheader);
      return;
      }

   strcpy(thisheader->mis.to, element);

   /* we start to check if the element is a macro.. */

   if(check_alias(&thisheader->mis))   /* return 0 if no luck, # of elements else */
     {
     WriteHeader(thisheader);
     FreeMIS(&thisheader->mis);
     mem_free(thisheader);
     return;
     }

   if( (strchr(element, '@') != NULL || strchr(element, '!') != NULL) &&
        cfg.usr.uucpname[0] != '\0')
     {
     strcpy(thisheader->mis.destinter, element);
     thisheader->mis.destfido = cfg.usr.uucpaddress;
     strcpy(thisheader->mis.to, cfg.usr.uucpname);
     WriteHeader(thisheader);
     FreeMIS(&thisheader->mis);
     mem_free(thisheader);
     return;
     }

   if(area->type == NETMAIL)
     {
     sprintf(msg, "Checking nodelist for %s..", thisheader->mis.to);
     statusbar(msg);
     if(check_node(&thisheader->mis, which_aka(&mis->origfido), NOPROMPT) != 1)  /* 0 == not found, -1 = abort */
        {
        sprintf(msg, "%s not found in nodelist!", thisheader->mis.to);
        Message(msg, -1, 0, YES);
        FreeMIS(&thisheader->mis);
        mem_free(thisheader);

        return;
        }
     else
       {
       WriteHeader(thisheader);
       FreeMIS(&thisheader->mis);
       mem_free(thisheader);
       }
     }

}


// ===================================================================

void WriteCCs(MSG *areahandle, AREA *area, RAWBLOCK **first, MIS *mis)
{
   RAWBLOCK *ccinfo = *first, *thismsgstart;
   HDRLIST *thisone;
   char temp[132];
   char info[] = "* Carbon copies sent to: ";
   char *kludges;
   int status;


   // Reset info for first block (CC: lines removed!)
   (*first)->curlen = strlen((*first)->txt);
   (*first)->curend = (*first)->txt + (*first)->curlen;

   // Expand CC: names and build a textblock out of it

   ccinfo = InitRawblock(1024, 1024, 64000u);

   sprintf(temp, "* Original message addressed to: %s.", mis->destinter[0] == '\0' ? mis->to : mis->destinter);

   if(area->type == NETMAIL && mis->destinter[0] == '\0')
     {
     strcat(temp, " (");
     strcat(temp, FormAddress(&mis->destfido));
     strcat(temp, ").");
     }
   strcat(temp, "\r");

   AddToBlock(ccinfo, temp, -1);
   AddToBlock(ccinfo, info, -1);

   sprintf(temp, "%d other recipient%s.\r", doCCnow, doCCnow > 1 ? "s" : "");
   AddToBlock(ccinfo, temp, -1);

   // Prepend expanded name block to text
   ccinfo->next = *first;
   *first = ccinfo;

   // ============ First lines fiddling finished.. pff.. ===========

   // Write all messages

   thisone = mem_calloc(1, sizeof(HDRLIST));
   for(status = ReadFirstHeader(thisone); status == 0; status = ReadNextHeader(thisone))
     {
     sprintf(temp, "Generating CC: message for %s", thisone->mis.to);
     statusbar(temp);

     thismsgstart = ccinfo;

     /* First we try to match AKA's... */

     matchaka(&thisone->mis);

     kludges = MakeKludge(NULL, &thisone->mis, 1);  /* Last parm == 1, always netmail */

     zonegate(&thisone->mis, kludges, 1, area->base);

     // Add kill attribute for cc:'s
     thisone->mis.attr1 |= aKILL;

     if(SaveMessage(areahandle,
                    area,
                    &thisone->mis,
                    thismsgstart,
                    kludges,
                    0L,
                    1) == 0)
                      add_tosslog(area, areahandle);

     if (kludges) mem_free(kludges);
     if(thismsgstart->next == ccinfo)     // Free usenet address mem
        {
        thismsgstart->next = NULL;
        FreeBlocks(thismsgstart);
        }

     FreeMIS(&thisone->mis);
     }

  mem_free(thisone);

}


/* get a list from disk, with names to CC: the message to.. */

void get_bomb_list(char *charptr, MIS *mis, AREA *area)
{
   FILE *ccfile;
   char temp[256];


   while(*charptr == ' ')
      charptr++;

   while(charptr[strlen(charptr)-1] == ' ')
      charptr[strlen(charptr)-1] = '\0';

   if( (ccfile = fopen(charptr, "rt")) == NULL)
      {
      sprintf(msg, "Can't open %s!", charptr);
      Message(msg, -1, 0, YES);
      return;
      }

   while(fgets(temp, 256, ccfile))
     {
     if(temp[0] == '\0' || temp[0] == '\n' || temp[0] == ';')
        continue;

     charptr = temp;

     while(*charptr == ' ') charptr++;

     if(charptr[strlen(charptr)-1] == '\n')
        charptr[strlen(charptr)-1] = '\0';

     while(charptr[strlen(charptr)-1] == ' ')
      charptr[strlen(charptr)-1] = '\0';

     if(nowcc)
        check_element(charptr, mis, area);
     else
        AddToArealist(charptr);
     }

   fclose(ccfile);

}


HDRLIST * get_copy(MIS *mis)
{
   MIS *newmis;
   HDRLIST *thisone;

   thisone = mem_calloc(1, sizeof(HDRLIST));
   newmis = &thisone->mis;

   if(!doCCnow)
      {
      doCCnow = 1;
      if(HdrFile == NULL)
        {
        if(OpenHeaderList() != 0)
           Message("Fatal error opening CC file!", -1, 254, YES);
        }
      }
   else doCCnow++;      // We count the CC:'s!

   strcpy(newmis->from, mis->from);
   strcpy(newmis->to, mis->to);

   newmis->origfido = mis->origfido;

   strcpy(newmis->subj, mis->subj);

   newmis->attr1 = mis->attr1;
   newmis->attr2 = mis->attr2;

   newmis->msgwritten = JAMsysTime(NULL);

   CopyStringList(mis->attached, &newmis->attached);
   CopyStringList(mis->requested, &newmis->requested);

   return thisone;
}

// ============================================

void AddToArealist(char *charptr)
{
   AREALIST *thisarea;

   thisarea = mem_calloc(1, sizeof(AREALIST));

   if((thisarea->areaptr = FindArea(charptr)) == NULL)
     {
     sprintf(msg, "Can't find area: %s!", charptr);
     Message(msg, -1, 0, YES);
     mem_free(thisarea);
     return;
     }

   if(!firstarea)
     firstarea = thisarea;
   else
     lastarea->next = thisarea;

   lastarea = thisarea;

}


// -----------------------------------------------------

void WriteXCs(AREA *area, RAWBLOCK **first, MIS *mis)
{
   RAWBLOCK *ccinfo = NULL;
   AREALIST *thisone;
   char temp[132];
   char info[] = "* Crossposted in: ";
   char *kludges;
   MSG *thisareahandle;
   HDRLIST *ourhdr = get_copy(mis);

   // Expand XC: names and build a textblock out of it

   ccinfo = InitRawblock(1024, 1024, 32000u);
   sprintf(temp, "* Original message posted in: %s.\r", area->tag);

   AddToBlock(ccinfo, temp, -1);
   AddToBlock(ccinfo, info, -1);

   for(thisone=firstarea; thisone; thisone=thisone->next)
     {
     sprintf(temp, "%s", thisone->areaptr->tag);

     AddToBlock(ccinfo, temp, -1);
     if(thisone->next) AddToBlock(ccinfo, ", ", -1);
     }

   AddToBlock(ccinfo, ".\r", -1);

   // Reset info for first block (CC: lines removed!)
   (*first)->curlen = strlen((*first)->txt);
   (*first)->curend = (*first)->txt + (*first)->curlen;

   // Prepend expanded name block to text
   ccinfo->next = *first;
   *first = ccinfo;

   // Write all messages
   for(thisone=firstarea; thisone; thisone=thisone->next)
     {
     if(thisone->areaptr == area)   // Crossposting in the same area?
        continue;

     sprintf(temp, "Generating crosspost message in %s", thisone->areaptr->tag);
     statusbar(temp);

     // !!!!!!

     if(thisone->areaptr->type == NETMAIL)
       {
       sprintf(msg, "Checking nodelist for %s..", ourhdr->mis.to);
       statusbar(msg);
       if(check_node(&ourhdr->mis, which_aka(&ourhdr->mis.origfido), NOPROMPT) != 1)  /* 0 == not found, -1 = abort */
          {
          sprintf(msg, "%s not found in nodelist!", ourhdr->mis.to);
          Message(msg, -1, 0, YES);
          }
       }


//     thismsgstart = ccinfo;

     if( (thisareahandle=MsgOpenArea(thisone->areaptr->dir, MSGAREA_CRIFNEC, thisone->areaptr->base))==NULL)
        {
        sprintf(msg, "Error opening area: %s!", thisone->areaptr->tag);
        Message(msg, -1, 0, YES);
        showerror();
        continue;
		  }

     get_custom_info(thisone->areaptr);

     ourhdr->mis.origfido = cfg.usr.address[custom.aka];

     kludges = MakeKludge(NULL, &ourhdr->mis, 0);  /* Last parm == 1, always netmail */

     FixEnd(first, thisone->areaptr);

     if(SaveMessage(thisareahandle,
                    thisone->areaptr,
                    &ourhdr->mis,
                    *first,
                    kludges,
                    0L,
                    1)
                    == 0)
                      add_tosslog(thisone->areaptr, thisareahandle);

     ScanArea(thisone->areaptr, thisareahandle, 1);

     MsgCloseArea(thisareahandle);

     if (kludges) mem_free(kludges);
     }

   get_custom_info(area);

   FixEnd(first, area);

}

// --------------------------------------------------

AREA * FindArea(char *tag)
{
   AREA *thisone = cfg.first;

   while(thisone)
     {
     if(!strcmpi(thisone->tag, tag))
       return thisone;
     thisone = thisone->next;
     }

   return NULL;
}


// Strip off Origin, add a new one if necessary, or a tearline in netmail (if nec..)

void FixEnd(RAWBLOCK **first, AREA *area)
{
   RAWBLOCK *lastblock;
   char *startorig;
   char temp[150];

   lastblock = JoinLastBlocks(*first, 1024);

   if(!lastblock) return;

   lastblock->maxlen += 100;  // please keep all this in one block!

   if(clean_origin(lastblock->txt, &startorig) == 0)
     {
     if(startorig != NULL)
        *startorig = '\0';
     }

   lastblock->curlen = strlen(lastblock->txt);
   lastblock->curend = lastblock->txt + lastblock->curlen;

   if(area->type == ECHOMAIL)
      AddToBlock(lastblock, make_origin(custom.aka), -1);
   else if(area->type == NETMAIL)
      {
      if(cfg.usr.status & NETTEAR)
         {
         sprintf(temp, "\r--- %s", (cfg.usr.status&EMPTYTEAR) ? "" : myname);
         AddToBlock(lastblock, temp, -1);
         }
      }

}

// =====================================================
// Get index number of a certain AKA

int which_aka(NETADDR *addr)
{
   int i;

   for(i=0; i<tMAXAKAS; i++)
     {
     if(addrcmp(addr, &cfg.usr.address[i]) == 0)
         return i;
     }

   return 0;
}

// ==============================================================

void ClearHeaderList(void)
{
   char temp[120];

   if(HdrFile != NULL)
     fclose(HdrFile);

   sprintf(temp, "%s\\hdr_tmp.###", cfg.homedir);
   unlink(temp);

   HdrFile = NULL;
   doCCnow = 0;

}


// ==============================================================

int OpenHeaderList(void)
{
   char temp[120];

   sprintf(temp, "%s\\hdr_tmp.###", cfg.homedir);
   HdrFile = _fsopen(temp, "w+b", SH_DENYRW);

   if(HdrFile == NULL)
     return -1;

   return 0;

}

// ==============================================================
// Returns:  0 == OK
//          -1 == ERROR
//           1 == No more
// ==============================================================

int ReadFirstHeader(HDRLIST * hdr)
{

   if(HdrFile == NULL) return 0;

   if(fseek(HdrFile, 0L, SEEK_SET) != 0)
     {
     Message("Error seeking CC header file!", -1, 0, YES);
     return -1;
     }

   return ReadNextHeader(hdr);
}

// ==============================================================

int ReadNextHeader(HDRLIST *hdr)
{
   int i;
   char attfile[120];
   REQUESTEDFILE reqfile;


   if(HdrFile == NULL) return -1;

   memset(hdr, '\0', sizeof(HDRLIST));

   if(fread(hdr, sizeof(HDRLIST), 1, HdrFile) != 1)
     {
     if(feof(HdrFile))
       return 1;
     else
       {
       Message("Error reading CC header file!", -1, 0, YES);
       return -1;
       }
     }

   // Clear old pointers that were written to disk

   hdr->mis.attached    = NULL;
   hdr->mis.requested   = NULL;
   hdr->mis.seenby      = NULL;
   hdr->mis.path        = NULL;
   hdr->mis.via         = NULL;
   hdr->mis.extrasub    = NULL;
   hdr->mis.extrasublen = 0L;

   for(i = 0; i < hdr->attaches; i++)
     {
     if(fread(&attfile, 120, 1, HdrFile) != 1)
       {
       Message("Error reading CC header file!", -1, 0, YES);
       return -1;
       }
     hdr->mis.attached = AddToStringList(hdr->mis.attached, &attfile, NULL, 0);
     }

   for(i = 0; i < hdr->requests; i++)
     {
     if(fread(&reqfile, sizeof(REQUESTEDFILE), 1, HdrFile) != 1)
       {
       Message("Error reading CC header file!", -1, 0, YES);
       return -1;
       }
     if(reqfile.password[0] == '\0')         // !!!!!!!!
        hdr->mis.requested = AddToStringList(hdr->mis.requested, reqfile.filename, NULL, 0);
     else
        hdr->mis.requested = AddToStringList(hdr->mis.requested, reqfile.filename, reqfile.password, 0);
     }

    return 0;
}

// ==============================================================

int WriteHeader(HDRLIST *hdr)
{
  int attaches = 0,
      requests = 0;

  STRINGLIST *thisone;
  char attfile[120], temp[120];
  REQUESTEDFILE reqfile;


  if(HdrFile == NULL) return -1;


  for(thisone = hdr->mis.attached; thisone; thisone=thisone->next)
     attaches++;

  for(thisone = hdr->mis.requested; thisone; thisone=thisone->next)
     requests++;

  hdr->attaches = attaches;
  hdr->requests = requests;

  sprintf(temp, "Spooling CC: info to disk for %s", hdr->mis.to);
  statusbar(temp);

  if(fwrite(hdr, sizeof(HDRLIST), 1, HdrFile) != 1)
    {
    Message("Error writing CC header file!", -1, 0, YES);
    return -1;
    }

  for(thisone = hdr->mis.attached; thisone; thisone=thisone->next)
    {
    memset(&attfile, '\0', 120);
    if(thisone->s) strcpy(attfile, thisone->s);
    // !!!!!!!!!!!!!!
    if(fwrite(attfile, 120, 1, HdrFile) != 1)
      {
      Message("Error writing CC header file!", -1, 0, YES);
      return -1;
      }
    }

  for(thisone = hdr->mis.requested; thisone; thisone=thisone->next)
    {
    memset(&attfile, '\0', sizeof(REQUESTEDFILE));
    if(thisone->s) strncpy(reqfile.filename, thisone->s, 29);
    if(thisone->pw) strncpy(reqfile.password, thisone->pw, 29);
    if(fwrite(&reqfile, sizeof(REQUESTEDFILE), 1, HdrFile) != 1)
      {
      Message("Error writing CC header file!", -1, 0, YES);
      return -1;
      }
    }

  return 0;

}

// ==============================================================
