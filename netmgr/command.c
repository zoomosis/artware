#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <ctype.h>
#include <sys\stat.h>

#include <msgapi.h>
#include <progprot.h>
#include "nstruct.h"
#include "txtbuild.h"
#include "netfuncs.h"
#include "xfile.h"
#include "binkpack.h"
#include "command.h"

extern CFG         cfg;
extern MESSAGE     msg;
extern COMMANDINFO CommandInfo;
extern XMASKLIST   *firstxmask;

// ===============================================================

RAWBLOCK * ReadBodyFromDisk(char *filename);
void       GetBaseName(NETADDR *addr);
int        GeneratePoll(int removebusy);
void       GetFile(int update);
void       SendFile(void);
int        CheckParms(char *needed);
char     * fancy_status(char status);
void       AppendFlo(char *from, char *to);
void       AppendPacket(char *from, char *to);

char BaseName[120] = "";

// ==============================================================

void ProcessCommand(void)
{

   switch(CommandInfo.CommandMode)
     {
     case POST:
       PostMessage();
       break;

     case POLL:
       GeneratePoll(1);
       break;

     case GET:
       GetFile(0);
       break;

     case UPDATE:
       GetFile(1);
       break;

     case SEND:
       SendFile();
       break;

     case CHANGE:
       ChangeStatus();
       break;

     default:
       ; // nothing
     }

}

// ==============================================================

void GetFile(int update)
{

   if(CheckParms("f#s") == -1)
      return;

   // Also sets Basename!
   // 0 means: don't remove Busy Flag that is generated.

   if(GeneratePoll(0) == -1)
      return;

   msg.mis.requested = AddToStringList(msg.mis.requested, CommandInfo.file,
              CommandInfo.password[0] == '\0' ? NULL : CommandInfo.password, 0);

   MakeReq(BaseName, update, NULL);

   RemoveBusyFlag(BaseName);

   print_and_log("Requested %s from %s (%s).\n", CommandInfo.file, fancy_address(&CommandInfo.address), fancy_status(CommandInfo.flavour));

   FreeMIS(&msg.mis);

}

// ==============================================================

void ChangeStatus(void)
{
   char from[120],
        to[120];
   int  didsome = 0;


   if(CheckParms("#sn") == -1)
      return;

   GetBaseName(&CommandInfo.address);

   if(MakeBusyFlag(BaseName) != 0)
     {
     print_and_log("Node %s is busy. Operation skipped!\n", fancy_address(&CommandInfo.address));
     return;
     }

   // First we tackle the .flo/.clo type files.....

   sprintf(from, "%s.%clo", BaseName, CommandInfo.flavour);
   sprintf(to  , "%s.%clo", BaseName, CommandInfo.newflavour);

   if(FileExists(from))
     {
     didsome = 1;
     if(FileExists(to))
        AppendFlo(from, to);
     else
       {
       if(rename(from, to) != 0)
          print_and_log("Error renaming %s to %s!\n", from, to);
       }
     }

   // With that out of the way, we concentrate on .OUT/.CUT type files.

   sprintf(from, "%s.%cut", BaseName, CommandInfo.flavour);
   sprintf(to  , "%s.%cut", BaseName, CommandInfo.newflavour);

   // It's not .fut, but .out, so correct for 'normal flavour' packets.

   if(from[strlen(from)-3] == 'f') from[strlen(from)-3] = 'o';
   if(to[strlen(to)-3] == 'f') to[strlen(to)-3] = 'o';

   if(FileExists(from))
     {
     didsome = 1;
     if(FileExists(to))
        AppendPacket(from, to);
     else
       {
       if(rename(from, to) != 0)
          print_and_log("Error renaming %s to %s!\n", from, to);
       }
     }

   RemoveBusyFlag(BaseName);

   FreeMIS(&msg.mis);

   if(didsome)
     print_and_log("Changed mail for %s from %s to %s.\n", fancy_address(&CommandInfo.address), fancy_status(CommandInfo.flavour), fancy_status(CommandInfo.newflavour));
   else
     print_and_log("No mail for %s flavoured %s present. No action needed.\n", fancy_address(&CommandInfo.address), fancy_status(CommandInfo.flavour));

}

// ==============================================================

void SendFile(void)
{

   if(CheckParms("f#s") == -1)
      return;

   // Also sets Basename!
   // 0 means: don't remove Busy Flag that is generated.

//   if(GeneratePoll(0) == -1)
//      return;

   GetBaseName(&CommandInfo.address);

   if(MakeBusyFlag(BaseName) != 0)
     {
     print_and_log("Node %s is busy. Operation skipped!\n", fancy_address(&CommandInfo.address));
     return;
     }

   switch(CommandInfo.flavour)
     {
     case 'c':
        msg.mis.attr1 = aCRASH;
        break;
     case 'h':
        msg.mis.attr1 = aHOLD;
        break;
     case 'i':
        msg.mis.attr1 = aIMM;
        break;
     case 'd':
        msg.mis.attr1 = aDIR;
        break;
     default:
        // Normal or unknown, do nothing!
        break;
     }

   msg.mis.attached = AddToStringList(msg.mis.attached, CommandInfo.file,
              CommandInfo.password[0] == '\0' ? NULL : CommandInfo.password, 0);

   MakeFlo(BaseName, 0, NULL);

   RemoveBusyFlag(BaseName);

   FreeMIS(&msg.mis);

   print_and_log("Sent %s to %s (%s).\n", CommandInfo.file, fancy_address(&CommandInfo.address), fancy_status(CommandInfo.flavour));


}


// ==============================================================

int GeneratePoll(int removebusy)
{

   if(CheckParms("#s") == -1)
      return -1;

   GetBaseName(&CommandInfo.address);

   if(MakeBusyFlag(BaseName) != 0)
     {
     print_and_log("Node %s is busy. Operation skipped!\n", fancy_address(&CommandInfo.address));
     return -1;
     }

   switch(CommandInfo.flavour)
     {
     case 'c':
        msg.mis.attr1 = aCRASH;
        break;
     case 'h':
        msg.mis.attr1 = aHOLD;
        break;
     case 'i':
        msg.mis.attr1 = aIMM;
        break;
     case 'd':
        msg.mis.attr1 = aDIR;
        break;
     default:
        // Normal or unknown, do nothing!
        break;
     }

   MakeFlo(BaseName, 1, NULL);

   if(removebusy)
     {
     RemoveBusyFlag(BaseName);
     print_and_log("Created poll for %s (%s).\n", fancy_address(&CommandInfo.address), fancy_status(CommandInfo.flavour));
     }

   return 0;

}

// ==============================================================

void PostMessage(void)
{
   XMASK *xmask = NULL;
   XMASKLIST *curmask;
   MSG *areahandle;
   word type;
   char *dir;
   char *kludges = NULL;
   MIS *mis = NULL;
   char addtomsg[300];
   RAWBLOCK *blk = NULL;


   // First we check to see if all needed info is there.

   if(CommandInfo.area[0] == '\0')
     {
     print_and_log("Command POST without target area to post in!\n");
     return;
     }

   if(CommandInfo.file[0] == '\0')
     {
     print_and_log("Command POST without file to post in message!\n");
     return;
     }

   if(CommandInfo.xmask[0] == '\0')
     {
     print_and_log("Command POST without xmask to use for header!\n");
     return;
     }

   for(curmask=firstxmask; curmask; curmask=curmask->next)
     {
     if(strcmpi(curmask->xmask->xmaskname, CommandInfo.xmask) == 0)
       {
       xmask = curmask->xmask;
       break;
       }
     }

   if(!xmask)
     {
     print_and_log("Can't find the XMASK called %s!\n", CommandInfo.xmask);
     return;
     }

   if(xmask->fromname == NULL || xmask->fromname->name == NULL ||
      xmask->toname   == NULL || xmask->toname->name   == NULL ||
      xmask->subj     == NULL || xmask->subj->name     == NULL ||
      xmask->orig     == NULL  )
      {
      print_and_log("XMASK (%s) doesn't contain all needed info!\n", CommandInfo.xmask);
      return;
      }


   // First we fill the header of the message;

   mis = mem_calloc(1, sizeof(MIS));

   mis->attr1 = aLOCAL;
   mis->msgwritten = JAMsysTime(NULL);

   strcpy(mis->from, xmask->fromname->name);
   strcpy(mis->to, xmask->toname->name);
   strcpy(mis->subj, xmask->subj->name);
   mis->origfido = xmask->orig->addr;

   if(xmask->dest != NULL)
      mis->destfido = xmask->dest->addr;

   if(xmask->attribs && xmask->attribs->yes_attribs1 != 0)
     mis->attr1 = xmask->attribs->yes_attribs1;

   if(xmask->attribs && xmask->attribs->yes_attribs2 != 0)
     mis->attr2 = xmask->attribs->yes_attribs2;

   // Now we get the text from the bodyfile.

   blk = ReadBodyFromDisk(CommandInfo.file);
   if(blk == NULL)
     goto getout;

   if(CommandInfo.isecho)
     {
     #ifndef __OS2__
     sprintf(addtomsg, "\r--- NetMgr " VERSION "%s\r * Origin: %s (%s)\r", REGISTERED ? "+" : " [unreg]", cfg.origin, fancy_address(&mis->origfido));
     #else
     sprintf(addtomsg, "\r--- NetMgr/2 " VERSION "%s\r * Origin: %s (%s)\r", REGISTERED ? "+" : " [unreg]", cfg.origin, fancy_address(&mis->origfido));
     #endif

     AddToBlock(blk, addtomsg, -1);
     }

   kludges = MakeKludge(NULL, &mis->origfido);

   /* Get area type (Squish or *.MSG) */

   dir = CommandInfo.area;

   switch(*dir)
     {
     case '$':
        type = MSGTYPE_SQUISH;
        dir++; /* Skip the $ */
        break;

     case '!':
        type = MSGTYPE_JAM;
        dir++; /* Skip the ! */
        break;

     case '#':
        type = MSGTYPE_HMB;
        dir++; /* Skip the # */
        break;

     default:
        type = MSGTYPE_SDM;
        break;
     }

     if(CommandInfo.isecho == 1)
       type |= MSGTYPE_ECHO;
     else
       type |= MSGTYPE_NET;


   if( (areahandle = MsgOpenArea(dir, MSGAREA_CRIFNEC, type)) == NULL)
     {
     print_and_log("Can't open  area (%s)!\n", dir);
     goto getout;
     }

   SaveMessage(areahandle, mis, blk, kludges, 0, CommandInfo.isecho ? 0 : 1);

   if(MsgCloseArea(areahandle) == -1)
     {
     print_and_log("Can't close area (%s)!\n", dir);
     goto getout;
     }

   print_and_log("Posted %s as a message in %s\n", CommandInfo.file, CommandInfo.area);

   getout:

   if(blk) FreeBlocks(blk);
   if(mis)
     {
     FreeMIS(mis);
     mem_free(mis);
     }

   if(kludges) mem_free(kludges);

}

// ==============================================================

RAWBLOCK *ReadBodyFromDisk(char *filename)
{
   XFILE *in;
   char *line;
   RAWBLOCK *blk;

   in = xopen(filename);
   if(in == NULL)
     {
     print_and_log("Can't open %s!\n", filename);
     return NULL;
     }

   blk = InitRawblock(4096, 1024, 8192);   // Updated for perfbeta

   while( (line=xgetline(in)) != NULL)
     {
     AddToBlock(blk, line, -1);
     AddToBlock(blk, "\r", 1);
     }

   xclose(in);

   if(blk->curlen < 2)
     {
     print_and_log("File (%s) appears to be empty..\n", filename);
     FreeBlocks(blk);
     blk = NULL;
     }

   return blk;

}

// =========================================================================

void GetBaseName(NETADDR *addr)
{
   if(cfg.outbound[0] == '\0')
     {
     print_and_log("Outbound area not defined!\n");
     exit(254);
     }

   strcpy(BaseName, OutboundName(addr));

}

// =========================================================================

int CheckParms(char *needed)
{

   if(needed == NULL || *needed == '\0')
     return -1;

   while(*needed != '\0')
     {
     switch(tolower(*needed))
       {
       case 'f':
         if(CommandInfo.file[0] == '\0')
           {
           print_and_log("Filename parameter (-f) required!\n");
           return -1;
           }
         break;

       case '#':
         if(CommandInfo.address.zone == 0 &&
            CommandInfo.address.net  == 0 &&
            CommandInfo.address.node == 0 )
           {
           print_and_log("Node address paramater (-#) required!\n");
           return -1;
           }
         break;

       case 's':
         if(CommandInfo.flavour == 0)
           {
           print_and_log("Mail status paramater (-s) required!\n");
           return -1;
           }
         break;

       case 'n':
         if(CommandInfo.newflavour == 0)
           {
           print_and_log("New mail status paramater (-n) required!\n");
           return -1;
           }
         break;
       }
     needed++;
     }

   return 0;
}

// =========================================================================

char *fancy_status(char status)
{
   switch(tolower(status))
     {
     case 'c':
       return "crash";
     case 'i':
       return "immediate";
     case 'h':
       return "hold";
     case 'f':
       return "normal";
     case 'd':
       return "direct";
       }

   return "unkown";

}

// ====================================================================

int FileExists(char *s)
{
   struct stat mystat;

   if(stat(s, &mystat) != 0)  // Doesn't exist.
      return 0;

   return 1;            // Exists.

}

// ==============================================================

void AppendFlo(char *from, char *to)
{
   FILE *in, *out;
   char line[120];
   
   if((in = _fsopen(from, "rt", SH_DENYWR)) == NULL)
      {
      print_and_log("Error opening %s!\n", from);
      return;
      }

   if((out = _fsopen(to, "at", SH_DENYRW)) == NULL)
      {
      print_and_log("Error opening %s!\n", to);
      return;
      }


   while(fgets(line, 119, in))
     {
     Strip_Trailing(line, '\n');
     if(strlen(line) > 0)
       {
       if(fprintf(out, "%s\n", line) <= strlen(line))
         {
         print_and_log("Error writing to %s!\n", to);
         fclose(out);
         return;
         }
       }
     }

   fclose(in);

   if(fclose(out) != 0) // We need to close (flush buffers) successfully too!
     print_and_log("Error writing to %s!\n", to);
   else if(unlink(from) != 0) // Only then we delete the original.
     print_and_log("Error deleting %s!\n", from);


}

// ==============================================================

void AppendPacket(char *from, char *to)
{
   int  in, out;
   int  got;
   char *buf;

   if( (in=sopen(from, O_RDONLY | O_BINARY, SH_DENYWR)) == -1)
      {
      print_and_log("Error opening %s!\n", from);
      return;
      }

   if( (out=sopen(to, O_RDWR | O_BINARY, SH_DENYRW)) == -1)
      {
      print_and_log("Error opening %s!\n", to);
      return;
      }

   if(SkipPacketHeader(in, 0) == -1)      // ,0 -> don't seek to end of packet.
     {
     print_and_log("Packet (%s) is invalid!\n", from);
     return;
     }

   if(SkipPacketHeader(out, 1) == -1)     // ,1 -> seek to end of packet
     {
     print_and_log("Packet (%s) is invalid!\n", to);
     return;
     }

   buf = mem_calloc(1, 4096);
   while( (got=read(in, buf, (unsigned) 4096)) > (int) 0)
     {
     if(write(out, buf, (unsigned) got) != (int) got)
       {
       print_and_log("Error writing to %s!\n", out);
       got = -1;
       break;
       }
     }

   mem_free(buf);

   close(in);
   close(out);

   if(got == -1)
     print_and_log("Error reading from %s!\n", from);
   else
     {
     if(unlink(from) != 0)
       print_and_log("Error deleting %s!\n", from);
     }

}

// ==============================================================

int IsCommand(char *s)
{
   if(!s || *s == '\0')
     return 0;

   if(strcmpi(s, "post") == 0)      // Command line posting of msg
      {
      CommandInfo.CommandMode = POST;
      return 1;
      }
   else if(strcmpi(s, "poll") == 0)      // Command line posting of msg
      {
      CommandInfo.CommandMode = POLL;
      return 1;
      }
   else if(strcmpi(s, "get") == 0)      // Command line posting of msg
      {
      CommandInfo.CommandMode = GET;
      return 1;
      }
   else if(strcmpi(s, "send") == 0)      // Command line posting of msg
      {
      CommandInfo.CommandMode = SEND;
      return 1;
      }
   else if(strcmpi(s, "change") == 0)      // Command line posting of msg
      {
      CommandInfo.CommandMode = CHANGE;
      return 1;
      }
   else if(strcmpi(s, "UPDATE") == 0)      // Command line posting of msg
      {
      CommandInfo.CommandMode = UPDATE;
      return 1;
      }

   return 0;

}

// ==============================================================

char GetFlavour(char *s)
{

   if(strcmpi(s, "normal") == 0)
     return 'f';
   if(strcmpi(s, "crash") == 0)
     return 'c';
   if(strcmpi(s, "hold") == 0)
     return 'h';
   if(strnicmp(s, "imm", 3) == 0)
     return 'i';
   if(strnicmp(s, "dir", 3) == 0)
     return 'd';

   print_and_log("Unknown mail flavour: %s\n", s);

   return 'f';

}

