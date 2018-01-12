#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <sys\stat.h>

#include <msgapi.h>
#include <progprot.h>
#include "nstruct.h"
#include "nodestru.h"
#include "packet.h"
#include "txtbuild.h"
#include "netfuncs.h"
#include "lastread.h"
#include "akamatch.h"
#include "binkpack.h"

// Temp structure to hold file requests. Unfortunately we have to
// do it complicated, because a filename could be followed by a
// password. So we first parse the subject line and then write the
// .REQ file.

typedef struct _reqlist
{

   char name[72];   // Hopefully nobody requests files this long :)
   char password[10];
   long unixtime;

} REQUESTLIST;

// ------------------------------------------------------

extern CFG     cfg;
extern MESSAGE msg;
extern long    MemLastRead;

char dirscreated[1024]="þ";  // This holds all directories that were created.

// ===================== Prototypes =============================

int    MakePacket  (NETADDR *orig, NETADDR *dest, char *password);
int    mymakedir   (char *buf);
void   createdir   (char *name);

int    MakeUniquePacket (NETADDR *orig, NETADDR *dest, char *password, char *dir);

int    CreateMailPacket(char * filename, NETADDR *orig, NETADDR *dest, char *password, int binkley);
int    AddPackedMsg    (NETADDR *orig, int packethandle, char *newsubject);
char * date2ascii      (dword d);
char * ConvertKludges  (void);
int    KillSent        (dword current);
char * PrepareTrailingKludges(MIS * mis, dword * trailsize);
void   AddToSubject    (char *subj, char *s);


// ==============================================================

int PackMail(NETADDR *orig, NETADDR *dest, dword current, char *password)
{
   NETADDR newto;
   int     result;
   char    temp[120], temp2[40];
   MIS     *tempmis;

   if(msg.mis.attr1 & aSENT) return 0;  // Don't pack 'sent' messages.
   if(msg.mis.attr1 & aLOK)  return 0;  // Don't pack 'locked' messages.

   // Copy to newto, we may change this below! Don't mess up the original!
   memcpy(&newto, dest, sizeof(NETADDR));

   if(cfg.outbound[0] == '\0')
     {
     print_and_log("No outbound defined in NetMgr.cfg!\n");
     return -1;
     }

   GetRawMsg(0);

   // dest may be *:*/*.*, so fill these in...

   if(dest->zone  == (word) -99)
      newto.zone  = msg.mis.destfido.zone;
   if(dest->net   == (word) -99)
      newto.net   = msg.mis.destfido.net;
   if(dest->node  == (word) -99)
      newto.node  = msg.mis.destfido.node;
   if(dest->point == (word) -99)
      newto.point = msg.mis.destfido.point;

   // We make a temp MIS struct, so we can do a standard AKAmatch on it
   // to get the right origination address.

   tempmis = mem_calloc(1, sizeof(MIS));
   tempmis->origfido = *orig;
   tempmis->destfido = newto;
   matchaka(tempmis);
   *orig = tempmis->origfido;
   mem_free(tempmis);

   result = MakePacket(orig, &newto, password);

   sprintf(temp2, "%s", formaddress(&newto));
   if(result == -1)
     {
     sprintf(temp, "Error packing msg #%d to %s (via %s).\n", (int) current, formaddress(&msg.mis.destfido), temp2);
     }
   else
     {
     sprintf(temp, "Packed msg #%d to %s (via %s).\n", (int) current, formaddress(&msg.mis.destfido), temp2);
     KillSent(current);
     if(msg.mis.attr1 & aKILL)
       result = 1;   // Give back to NetMgr, stop performing action 'cause msg is killed.
     }

   print_and_log(temp);

   return result;   // -1 = error, 0 = OK, 1 = OK, but message k/s - stop scanning.

}

// ==============================================================

int MoveMail(NETADDR *orig, NETADDR *dest, char *dir, dword current, char *password)
{
   int     result;
   char    temp[120], temp2[40];
   MIS     *tempmis;

   if(msg.mis.attr1 & aSENT) return 0;  // Don't pack 'sent' messages.
   if(msg.mis.attr1 & aLOK)  return 0;  // Don't pack 'locked' messages.

   GetRawMsg(0);

   // We make a temp MIS struct, so we can do a standard AKAmatch on it
   // to get the right origination address.

   tempmis = mem_calloc(1, sizeof(MIS));
   tempmis->origfido = *orig;
   tempmis->destfido = *dest;
   matchaka(tempmis);
   *orig = tempmis->origfido;
   mem_free(tempmis);

   result = MakeUniquePacket(orig, dest, password, dir);

   strcpy(temp2, formaddress(dest));
   if(result == -1)
     {
     sprintf(temp, "Error moving msg #%d to %s (via %s).\n", (int) current, formaddress(&msg.mis.destfido), temp2);
     }
   else
     {
     sprintf(temp, "Moved msg #%d to %s (via %s).\n", (int) current, formaddress(&msg.mis.destfido), temp2);
     KillSent(current);
     if(msg.mis.attr1 & aKILL)
        result = 1;   // Give back to NetMgr, stop performing action 'cause msg is killed.
     }

   print_and_log(temp);

   return result;   // -1 = error, 0 = OK, 1 = OK, but message k/s - stop scanning.

}


// =========================================================
//
//   Make an .FLO file. Return 0 on success, -1 on error.
//
// =========================================================

int MakeFlo(char *filename, int CreateOnly, char *newsubject)
{
   char buf[120];
   int  handle;
   char temp[150];
   char fname[_MAX_FNAME], ext[_MAX_EXT];
   STRINGLIST *current;

   strcpy(buf, filename);

   // Generate correct filename according to flavour.
   if (msg.mis.attr1 & aDIR)
       strcat(buf, ".dlo");
   else if ((msg.mis.attr1 & (aCRASH | aHOLD)) == (aCRASH | aHOLD))
       strcat(buf, ".dlo");
   else if (msg.mis.attr1 & aCRASH)
       strcat(buf, ".clo");
   else if (msg.mis.attr1 & aHOLD)
       strcat(buf, ".hlo");
   else if (msg.mis.attr1 & aIMM)
       strcat(buf, ".ilo");
    else
       strcat(buf, ".flo");

   handle = sopen(buf, O_APPEND | O_CREAT | O_RDWR, SH_DENYRW, S_IREAD | S_IWRITE);
   if (handle < 1)
      {
      print_and_log("Could not create %s\n", buf);
      return -1;
      }

   if(CreateOnly)
     {
     close(handle);
     return 0;
     }

   if(newsubject)
      memset(newsubject, '\0', 72);

   for(current = msg.mis.attached; current; current = current->next)
     {
     if(current->s == NULL || current->s[0] == '\0')
       continue;

     if (msg.mis.attr1 & aKFS)       // Kill the file after send..
       write(handle,"^",1);
     else if (msg.mis.attr1 & aTFS)  // Truncate file after send..
       write(handle,"#",1);

     write(handle, current->s, strlen(current->s));
     write(handle, "\n", 1);

     // Write filename without path to newsubject.
     if(newsubject)
       {
       strcpy(temp, current->s);
       _splitpath(temp, NULL, NULL, fname, ext);
       sprintf(temp, "%s%s", fname, ext);
       AddToSubject(newsubject, temp);
       }
     }

   close(handle);
   return 0;
}

// =========================================================

int MakePacket(NETADDR *orig, NETADDR *dest, char *password)
{
   char filename[120];
   char newsubject[72];  // Used to strip paths from file attaches.
   int  packethandle,
        result;

   strcpy(filename, OutboundName(dest));

   if(MakeBusyFlag(filename) != 0)
     {
     print_and_log("Node %s is busy - packet generation skipped\n", formaddress(dest));
     return -1;
     }

   memset(newsubject, '\0', 72);
   strncpy(newsubject, msg.mis.subj, 71);  // We start with the original subj

   if(msg.mis.attr1 & aFILE)
     {
     if(msg.mis.subj[0] != '\0')    // We have a subject _and_ and attach.
       {
       // We can ignore the attaches (for the subject contents), if it is not
       // a routed message. However, routed messages (including file attaches
       // to points!) _need_ the filenames in the subject, so the recipient
       // knows what files to route through! So check routing here..

       if(matchaddress(&msg.mis.destfido, dest) == 0)  // Oops, routed file.
         MakeFlo(filename, 0, newsubject);             // Fill in filenames in subj
       else
         MakeFlo(filename, 0, NULL);               // preserve subj field.
       }
     else  // No subject, fill in a new one with the files.
        MakeFlo(filename, 0, newsubject);  // 0 == Not only create, but add names to it as well.
     }

   if(msg.mis.attr1 & (aFRQ|aURQ))
     {
     MakeFlo(filename, 1, NULL);  // 1 == Only create it, that's all.
     MakeReq(filename, (msg.mis.attr1 & aFRQ) ? 0 : 1, newsubject[0] == '\0' ? newsubject : NULL);
     }

   packethandle = CreateMailPacket(filename, orig, dest, password, 1);  // 1 == Binkley outbound
   if(packethandle == -1)
     {
     RemoveBusyFlag(filename);
     return -1;
     }

   result = AddPackedMsg(orig, packethandle, newsubject);

   close(packethandle);

   RemoveBusyFlag(filename);

   return result;

}

// =========================================================

int MakeUniquePacket(NETADDR *orig, NETADDR *dest, char *password, char *dir)
{
   int  packethandle,
        result;
   STRINGLIST *current;
   char newsubject[72] = "";
   char fname[_MAX_FNAME], ext[_MAX_EXT];
   char temp[_MAX_PATH];


   if(msg.mis.attr1 & aFILE)
     {
     for(current = msg.mis.attached; current; current=current->next)
       {
       if(msg.mis.attr1 & aKFS)
          result = CopyFile(current->s, dir, 1);  // 1 == Delete after copy
       else
          result = CopyFile(current->s, dir, 0);  // 0 == Just copy, no delete

       if( (result != -1) && (msg.mis.attr1 & aTFS) )
         Truncate(current->s);

       // Make sure this stuff ends up in the subject for further processing.
       _splitpath(current->s, NULL, NULL, fname, ext);
       sprintf(temp, "%s%s", fname, ext);
       AddToSubject(newsubject, temp);
       }
     }

   if(msg.mis.attr1 & aFRQ)
     {
     for(current = msg.mis.requested; current; current=current->next)
       {
       // Make sure this stuff ends up in the subject for further processing.
       if(current->s)
          AddToSubject(newsubject, current->s);
       }
     }

   packethandle = CreateMailPacket(dir, orig, dest, password, 0);  // 0 == Not binkley outbound
   if(packethandle == -1)
     return -1;

   result = AddPackedMsg(orig,
                         packethandle,
                         newsubject[0] == '\0' ? NULL : newsubject);

   close(packethandle);

   return result;

}


// =========================================================
//
//   Make a .req file. Return 0 on success, -1 on error.
//
// =========================================================

int MakeReq(char *filename, int update, char *newsubject)
{
   char buf [120];
   char temp[150];
   int  handle;
   char fname[_MAX_FNAME], ext[_MAX_EXT];
   struct stat mystat;
   STRINGLIST *current;
   time_t unixtime;


   strcpy(buf, filename);
   strcat(buf, ".req");

   if(newsubject)
      memset(newsubject, '\0', 72);

   handle = sopen(buf, O_APPEND | O_CREAT | O_RDWR, SH_DENYRW, S_IREAD | S_IWRITE);
   if (handle < 1)
      {
      print_and_log("Could not create %s!\n", buf);
      return -1;
      }

   for(current=msg.mis.requested; current; current=current->next)
     {
     if(current->s == NULL || current->s[0] == '\0')
       continue;

     memset(temp, '\0', sizeof(temp));
     if(update)          // Update request, we need time/date as well!
       {
       if(stat(current->s, &mystat) == -1)          // Uh oh!
          {
          print_and_log("Can't find update request info for: %s!\n", current->s);
          unixtime = 0L;
          }
       else
         {
         #ifdef __OS2__
            unixtime = mystat.st_mtime;
         #else
            unixtime = mystat.st_atime;
         #endif
         }

       // Strip path from filename.
       _splitpath(current->s, NULL, NULL, fname, ext);
       strcpy(temp, fname);
       strcat(temp, ext);
       }
     else
       {
       strcpy(temp, current->s);
       if(newsubject) AddToSubject(newsubject, current->s);
       }

     if(current->pw != '\0')  // Add password?
        {
        strcat(temp, " !");
        strcat(temp, current->pw);
        }

     if(update)                       // Add unixtime?
        {
        strcat(temp, " +");
        sprintf(buf, "%ld", unixtime);
        strcat(temp, buf);
        }

     strcat(temp, "\n");
     write(handle, temp, strlen(temp));
     }

   close(handle);
   return 0;

}


// =========================================================
//
// Create 'basename' for a mailpacket in Bink's outbound.
//
// =========================================================

char * OutboundName(NETADDR *address)
{
   static char outboundname[120];
   char temp[100];

   if(address->zone != cfg.homeaddress[0].zone)
      sprintf(outboundname, "%s.%03x\\", cfg.outbound, address->zone);
   else
      sprintf(outboundname, "%s\\", cfg.outbound);

   if(address->point != 0)
      sprintf(temp, "%04x%04x.PNT\\0000%04x", address->net, address->node, address->point);
   else
      sprintf(temp, "%04x%04x", address->net, address->node);

   strcat(outboundname, temp);

   createdir(outboundname);

   return outboundname;
}


// ==========================================================
//
//  Check a directory for existence, create if it doesn't.
//  Actual work-function that can create a dir.
//
// =============================================================


int mymakedir(char *buf)
{
 char temp[256];

 strcpy(temp,"þ");                // Store created paths separated with 'þ'..
 strcat(temp,buf);
 strcat(temp,"þ");
 strupr(temp);
 if (strchr(buf,'\\')==NULL) return 0;    // Is this a path, or just root?

 if (strstr(dirscreated,temp)) return 0;  // Did we already make this?
 if (strlen(dirscreated)+strlen(temp)+5 < sizeof(dirscreated))
   strcat(dirscreated,temp);              // Only if there's enough room to add it.

 return mkdir(buf);
}


// =============================================================
//
// Check a directory for existence, create if it doesn't.
// Main loop that checks dir in parts.
//
// =============================================================

void createdir(char *name)
{
    char buf[256];
    int  i,
         len,
         status = 0;

    len = strlen(name);
    for (i = 0; i < len; i++) {
        strcpy(buf, name);
        if (buf[i] == '\\') {
            buf[i] = 0;
            status = mymakedir(buf);
        };
    }

}

// ================================================================
//
// Create a mailpacket, with header, ready for adding messages.
// If 'binkley' is 0, create a new unique name and name it
// ending with .pkt in the directory found in 'filename'.
// Otherwise create extension according to the current message's
// attribute and add that to 'filename'.
//
// ================================================================

int CreateMailPacket(char * filename, NETADDR *orig, NETADDR *dest, char *password, int binkley)
{
   char packetname[120];
   int handle;
   static PKTHEADER pktheader;  // Not on stack..
   long size;
   time_t now;
   struct tm *nowtm;

   if(binkley)
     {
     strcpy(packetname, filename);

     if (msg.mis.attr1 & aDIR)
         strcat(packetname, ".dut");
     else if ((msg.mis.attr1 & (aCRASH | aHOLD)) == (aCRASH | aHOLD))
         strcat(packetname, ".dut");
     else if (msg.mis.attr1 & aCRASH)
         strcat(packetname, ".cut");
     else if (msg.mis.attr1 & aHOLD)
         strcat(packetname, ".hut");
     else if (msg.mis.attr1 & aIMM)
         strcat(packetname, ".iut");
      else
         strcat(packetname, ".out");
     }
   else
     {
     // Generate a unique name..
     now = time(NULL);
     now <<= 4;

     do {
        sprintf(packetname, "%s\\%08lx.pkt", filename, now++);
        handle = sopen(packetname, O_BINARY | O_RDONLY, SH_DENYNO, S_IREAD);
        if (handle > 0)
            close(handle);
        } while(handle != -1);
     }

   handle = sopen(packetname, O_BINARY | O_CREAT | O_RDWR, SH_DENYRW, S_IREAD | S_IWRITE);
   if (handle < 1)
      {
      print_and_log("Could not create %s\n", packetname);
      return -1;
      }

   size = filelength(handle);
   if(size < 60)
      {
      memset(&pktheader, 0, sizeof(pktheader));
      pktheader.pkttype = 2;

      now = time(NULL);
      nowtm = localtime(&now);

      pktheader.year   = nowtm->tm_year + 1900;
      pktheader.month  = nowtm->tm_mon;
      pktheader.day    = nowtm->tm_mday;
      pktheader.hour   = nowtm->tm_hour;
      pktheader.minute = nowtm->tm_min;
      pktheader.second = nowtm->tm_sec;

      pktheader.destnode   = dest->node;
      pktheader.destnet    = dest->net;
      pktheader.destzone   = dest->zone;
      pktheader.destzone2  = dest->zone;
      pktheader.destpoint2 = dest->point;

      pktheader.orignode   = orig->node;
      pktheader.orignet    = orig->net;
      pktheader.origzone   = orig->zone;
      pktheader.origzone2  = orig->zone;
      pktheader.origpoint2 = orig->point;

      strncpy(pktheader.password, password, 8);

      pktheader.capabilword = 0x0001;
      pktheader.CWvalcopy   = 0x0100;

      pktheader.productcode = 0xFE;
      pktheader.ProductCode = 0;
      pktheader.revision = 0;
      pktheader.Revision = 0;

      write(handle, &pktheader, sizeof(pktheader));
      write(handle, "\0\0", 2);                        // end of pkt file ..
      }

   lseek(handle, -2L, SEEK_END);

   return handle;

}

// ================================================================

int AddPackedMsg(NETADDR *orig, int packethandle, char *newsubject)
{
   PACKEDMSG    packedmsg;
   char *       kludges = NULL,
        *       trailing = NULL,
        *       charptr;
   dword        trailsize = 0;
   RAWBLOCK *   last;
   time_t       t;
   struct tm *  gmt;
   char         buf[100], datetime[80];
   int          i;

   tzset();

   t = time(NULL);
   gmt = gmtime(&t);

   // Set values for packed message.

   memset(&packedmsg, 0, sizeof(packedmsg));
   packedmsg.msgtype = 2;

   packedmsg.destnode = msg.mis.destfido.node;
   packedmsg.destnet  = msg.mis.destfido.net;
   packedmsg.orignode = msg.mis.origfido.node;
   packedmsg.orignet  = msg.mis.origfido.net;

   packedmsg.attribute = (word) (msg.mis.attr1 & 0xFFFF);

   // Zero out several bits (follow FTS-0001).
   packedmsg.attribute &= (aPRIVATE|aCRASH|aFILE|aXX2|aRRQ|aCPT|aARQ);

   if(write(packethandle, &packedmsg, (unsigned) sizeof(PACKEDMSG)) != (int) sizeof(PACKEDMSG))
     return -1;

// We *always* generate a correct date, don't assume it's OK already.
   memcpy(msg.mis.ftsc_date, date2ascii(msg.mis.msgwritten), 20);

   write(packethandle, msg.mis.ftsc_date, 20);
   write(packethandle, msg.mis.to, strlen(msg.mis.to) + 1);
   write(packethandle, msg.mis.from, strlen(msg.mis.from) + 1);

   if(newsubject == NULL)
     write(packethandle, msg.mis.subj, strlen(msg.mis.subj) + 1);
   else
     write(packethandle, newsubject, strlen(newsubject) + 1);

   // Now we convert the attributes & kludges.

   kludges = ConvertKludges();
   write(packethandle, kludges, strlen(kludges));

   // Now we are going to mess around :-( We have to, because of the fact
   // that some message are written out with trailing garbage (Maximus' QWK
   // facility for example).
   // We don't want that to end up in the packet, so...

   if(msg.fblk)   // If there *is* a body
     {
     last = JoinLastBlocks(msg.fblk, 2048);  // Let's assume there's never more junk than 2 K..
     if(last)
       {
       // Now check for a '\0' inside the block.
       for(i=0, charptr=last->txt; i<last->curlen; i++, charptr++)
          {
          if(*charptr == '\0')
            {
            last->curlen = strlen(last->txt);         // Reset len
            last->curend = last->txt + last->curlen;
            break;
            }
          }
       // curlen is now length, without trailing '\0', just what we need!
       }
     }

   // Now write out the actual body.
   for(last = msg.fblk; last; last = last->next)
     {
     if(last->curlen > 0)     // Only if we have anything to write..
        if(write(packethandle, last->txt, (unsigned) last->curlen) != (int) last->curlen)
           return -1;
     }

   // Now we need an ending '\r' in the message, 'cause we want to write
   // a ^AVIA line.

   if(msg.fblk)
      {
      if(GetLastChar(msg.fblk) != '\r')
         write(packethandle, "\r", (unsigned) 1);
      }

   // With that done, we need to add all the trailing kludges.

   trailing = PrepareTrailingKludges(&msg.mis, &trailsize);
   if(trailing)
     {
     if(write(packethandle, trailing, (unsigned) trailsize) != (int) trailsize)
        return -1;
     }

   // Generate a ^VIA line..

   strftime(datetime, 80, "%Y%m%d.%H%M%S", gmt);
   sprintf(buf, "Via %s @%s.UTC NetMgr " VERSION "%s\r",
                                       fancy_address(orig),
                                       datetime,
                                       REGISTERED ? "+" : " [unreg]"
                                       );

   if(write(packethandle, buf, (unsigned) strlen(buf)) != (int) strlen(buf))
      return -1;

   // Properly end message with '\0', and packet with '\0\0'.
   if(write(packethandle, "\0\0\0", (unsigned) 3) != (int) 3)
      return -1;

   if(kludges) mem_free(kludges);
   if(trailing) mem_free(trailing);

   return 0;
}

// ====================================================================
//
//       Generate a correct FTSC date (add extra checks later..)
//
// ====================================================================

char * date2ascii(dword d)
{
  struct JAMtm *jtm;
  static char  buf[40];

  jtm = JAMsysLocalTime(&d);

  if( (jtm->tm_mday > 31) ||
      (jtm->tm_mon  > 11) ||
      (jtm->tm_year < 90) ||
      (jtm->tm_hour > 23) ||
      (jtm->tm_min  > 59) ||
      (jtm->tm_sec  > 59) )
     {
     d = JAMsysTime(NULL);
     jtm = JAMsysLocalTime(&d);
     print_and_log("Invalid date detected in msgheader. Set date to today..\n");
     }


  sprintf(buf, "%02u %-3.3s %02u  %02u:%02u:%02u   ",
          jtm->tm_mday, months_ab[jtm->tm_mon], jtm->tm_year,
          jtm->tm_hour, jtm->tm_min, jtm->tm_sec);
  buf[19] = 0;

  return buf;

}

// ==============================================================

char * ConvertKludges(void)
{
   byte  temp[120];
   char * kludges, *tmpkludges;

   if(!msg.kludges)
      return mem_calloc(1, 1);  // Let's return something, but not NULL..!

   kludges = mem_calloc(1, strlen(msg.kludges) + 512);  // For additions, will be enough.
   if( (tmpkludges = CvtCtrlToKludge(msg.kludges)) == NULL ) // Get 'm ready for writing to disk.
     {
     mem_free(kludges);
     return mem_calloc(1,1);
     }

   if(msg.mis.origfido.point && !strstr(tmpkludges, "\x01" "FMPT"))
     {
     sprintf(temp, "\x01" "FMPT %u\r", msg.mis.origfido.point);
     strcat(kludges, temp);
     }

   if(msg.mis.destfido.point && !strstr(tmpkludges, "\x01" "TOPT"))
     {
     sprintf(temp, "\x01" "TOPT %u\r", msg.mis.destfido.point);
     strcat(kludges, temp);
     }

   if(!strstr(tmpkludges, "\x01INTL"))  // Always add INTL if not there already
     {
     sprintf(temp, "\x01INTL %u:%u/%u %u:%u/%u\r",
             msg.mis.destfido.zone, msg.mis.destfido.net, msg.mis.destfido.node,
             msg.mis.origfido.zone, msg.mis.origfido.net, msg.mis.origfido.node);
     strcat(kludges, temp);
     }

   // Maybe we need to add support for other flags as well ?!
   if(msg.mis.attr1 & aCFM)
     {
     strcpy(temp, "\x01" "FLAGS CFM\r");

     // This seems to be the only useful one to transmit?!?

     strcat(kludges, temp);
     }

   strcat(kludges, tmpkludges);      // Add what we already had.
   mem_free(tmpkludges);             // Give up mem allocated by CvtCtrlToKludge().

   return kludges;

}

// ======================================================================

int KillSent(dword current)
{

   MsgCloseMsg(msg.msghandle);
   msg.msghandle = NULL;

   if(msg.mis.attr1 & aKILL)
      {
      // See if we need to adjust lastread pointer..
      if(MemLastRead == current)
         MemLastRead = AdjustLastread(MemLastRead, msg.areahandle);

      if(MsgKillMsg(msg.areahandle, current) == -1)
        {
        print_and_log("Error killing message #%d!\n", (int) current);
        return -1;
        }
      return 0;
      }

   // If we get here, we need to mark 'sent'.

   msg.mis.attr1 |= aSENT;

   msg.msghandle = MsgOpenMsg(msg.areahandle, MOPEN_RW, current);
   if(msg.msghandle == NULL)
     {
     print_and_log("Can't open message #%d for write!\n", current);
     return -1;
     }

   if(MsgWriteMsg(msg.msghandle, 0, &msg.mis, 0L, 0L, 0L, 0L, 0L) == -1)
     {
     print_and_log("Error setting 'sent' bit on msg #%d!\n", (int) current);
     // Don't return here, we need to close etc..
     }

   MsgCloseMsg(msg.msghandle);

   // Re-open in READ mode for future messing around by NetMgr.

   msg.msghandle = MsgOpenMsg(msg.areahandle, MOPEN_READ, current);
   if(msg.msghandle == NULL)
       {
       print_and_log("Can't reopen message #%d!\n", current);
       return -1;
       }

   return 0;
}

// ======================================================================

int MakeBusyFlag(char *location)
{
   char filename[120];
   int  flagfile;

   sprintf(filename, "%s.bsy", location);

   flagfile = sopen(filename, O_CREAT | O_RDWR | O_EXCL, SH_DENYRW, S_IREAD | S_IWRITE);

   if(flagfile != -1)
      close(flagfile);

   return (flagfile == -1) ? 1 : 0;

}

// ======================================================================

void RemoveBusyFlag(char *location)
{
   char filename[120];

   sprintf(filename, "%s.bsy", location);

   unlink(filename);
}

// ======================================================================

int CopyFile(char *filename, char *todir, int delete)
{
   char fname[_MAX_FNAME], ext[_MAX_EXT];
   char dest[120];
   char *buf;
   int  got,
        in,
        out;


   _splitpath(filename, NULL, NULL, fname, ext);
   sprintf(dest, "%s\\%s%s", todir, fname, ext);

   if(delete)     // It's a move, try fast rename()
     {
     if(rename(filename, dest) == 0)      // It works!
       {
       print_and_log("Moved %s to %s.\n", filename, dest);
       return 0;                          // So that's all.
       }
     }

   if( (in=sopen(filename, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
     {
     print_and_log("Can't open %s for read!\n", filename);
     return -1;
     }

   if((out=sopen(dest, O_BINARY | O_CREAT | O_WRONLY | O_EXCL, SH_DENYRW, S_IREAD | S_IWRITE)) == -1)     {
     print_and_log("Can't open %s for writing!\n", dest);
     close(in);
     return -1;
     }

   buf = mem_calloc(1, 16384);

   while( (got=read(in, buf, (unsigned) 16384)) > (int) 0)
     {
     if(write(out, buf, (unsigned) got) != (int) got)
       {
       print_and_log("Error writing to %s!\n", dest);
       got = -1;
       break;
       }
     }

   if(got == -1)
     {
     print_and_log("Error reading from %s!\n", filename);
     }

   close(in);
   close(out);
   mem_free(buf);

   if(delete & (got != -1))
     unlink(filename);

   if(got != -1)
     {
     print_and_log("%s %s to %s.\n", delete ? "Moved" : "Copied", filename, dest);
     }

   return (got == -1) ? -1 : 0;

}

// ======================================================================

int Truncate(char *filename)
{
  int handle;

  handle = sopen(filename, O_RDWR | O_BINARY | O_TRUNC, SH_DENYRW);

  if(handle == -1)
    return -1;

  print_and_log("Truncated %s.\n", filename);

  close(handle);

  return 0;

}

// ======================================================================

char * fancy_address(NETADDR *this)
{
   static char temp[100];

   memset(temp, '\0', sizeof(temp));

   if(this->point != 0)
     sprintf(temp, "%d:%d/%d.%d", (sword) this->zone,
                                  (sword) this->net,
                                  (sword) this->node,
                                  (sword) this->point);
   else
     sprintf(temp, "%d:%d/%d", (sword) this->zone,
                               (sword) this->net,
                               (sword) this->node);

   return temp;
}

// ==============================================================

int SkipPacketHeader(int handle, int SeekToEnd)
{
   PKTHEADER pkthdr;
   char nulls[2];

   lseek(handle, 0L, SEEK_SET);
   if(read(handle, &pkthdr, (unsigned) sizeof(PKTHEADER)) !=
                                                      (int) sizeof(PKTHEADER))
       return -1;

   if(pkthdr.pkttype != 2)
      {
      print_and_log("Weird packet type (type %d?)\n", pkthdr.pkttype);
      return -1;
      }

   // We now seeked past the packetheader and are positioned at the start
   // of the packed messages (if any).

   if(SeekToEnd)
     {
     lseek(handle, -2L, SEEK_END);
     if( (read(handle, nulls, (unsigned) 2) != (int) 2) ||
         (nulls[0] != '\0') ||
         (nulls[1] != '\0') )
         {
         print_and_log("Packet not correctly NULL terminated!\n");
         return -1;
         }
     lseek(handle, -2L, SEEK_END);  // Now we seek back again, so we can add.
     }

   return 0;

}

// ==============================================================

char * PrepareTrailingKludges(MIS *mis, dword * trailsize)
{

  STRINGLIST * current;
  dword        trailmax;
  char       * trailing = NULL;

  if(mis->seenby || mis->path || mis->via)
    {
    trailmax   = 512;
    *trailsize = 0;
    trailing   = mem_calloc(1, 512);

    for(current=mis->seenby; current; current=current->next)
      {
      AddToString(&trailing, &trailmax, trailsize, "SEEN-BY: ");
      AddToString(&trailing, &trailmax, trailsize, current->s);
      AddToString(&trailing, &trailmax, trailsize, "\r");
      }

    for(current=mis->path; current; current=current->next)
      {
      AddToString(&trailing, &trailmax, trailsize, "\01PATH: ");
      AddToString(&trailing, &trailmax, trailsize, current->s);
      AddToString(&trailing, &trailmax, trailsize, "\r");
      }

    for(current=mis->via; current; current=current->next)
      {
      AddToString(&trailing, &trailmax, trailsize, "\01Via ");
      AddToString(&trailing, &trailmax, trailsize, current->s);
      AddToString(&trailing, &trailmax, trailsize, "\r");
      }
    }

  return trailing;

}

// ===========================================================================
//
//   Add the string 's' to the subject 'subj', if it fits in a 72 byte string
//
// ===========================================================================

void AddToSubject(char *subj, char *s)
{
   if(!subj || !s) return;

   if(strlen(subj) + strlen(s) < 71)   // space + trailing '\0'
      {
      if(subj[0] != '\0')        // Not first addition?
         strcat(subj, " ");
      strcat(subj, s);
      }

}

// ==============================================================
