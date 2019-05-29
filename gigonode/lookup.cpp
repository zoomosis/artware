#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <process.h>
#include <stdlib.h>
#include <share.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include "fidoadr.h"
#if !defined(word)
 #ifdef __OS2__
  #define word unsigned short int
 #else
  #define word    unsigned int
  #endif
#endif

#if !defined(byte)
#define byte    unsigned char
#endif

#define eofidx (feof(nodefile))
#define gnextidx fread(&nextidx,sizeof(nextidx),1,nodefile)
FILE *nodefile;

int _validaddress(word czone,word cnet,word cnode)
{word zone,net;register word node;
 zone=net=node=0;
 word nextidx=0;
 fseek(nodefile,0,SEEK_SET);
 while (!eofidx)
  { gnextidx;
    node=nextidx;
    if (node==65535)
      {
newzone:
       gnextidx; zone=net=nextidx;node=0;
       if (zone>czone) return 0; // assume that the idx is in zone order?
      /*
       if (zone<czone)
         {
          while(!eofidx)
           {
            gnextidx;
            if (nextidx==65535) goto newzone;
           }
          }
          */
       }
    if (node==65534)
      {gnextidx;net=nextidx;node=0;}
    if ((cnet==net) && (cnode==node) && (czone==zone)) return 1;
  }
 return 0;
}

int validaddress(word czone,word cnet,word cnode)
{ int i;
 nodefile=fopen("NODELIST.GIG","rb");
 if (!nodefile)
  {
  cprintf("NODELIST.GIG not in current directory or not openable.\r\n");
  exit(1);
  }
 setvbuf(nodefile,NULL, _IOFBF,2048);
 if (!nodefile)
  {
   return 1;
  }
 i=_validaddress(czone,cnet,cnode);
 fclose(nodefile);
 return i;
}

void testaddresses(void)
{
 unsigned zone,net,node;
 for (zone=5;zone<=6;zone++)
  for (net=100;net<300;net++)
    for (node=0;node<1000;node++)
     { putch('.');
     if (validaddress(zone,net,node))
      cprintf("\r%u:%u/%u\r\n",zone,net,node);
      }
}


void main(int argc, char *argv[])
{

 int i;
 FIDOADR fidoadr;
 if (argc == 1)

  {
  cprintf("Usage: %s [fidoaddress [fidoaddress ... ]]\r\n",argv[0]);
  cprintf(" where fidoaddress is in the form zone:net/node\r\n");
  }


 for (i=1;i<argc;i++)
  {
   fidoadr_split(argv[i],&fidoadr);
   if (!fidoadr.zone) fidoadr.zone=1;
   cprintf("Checking: %u:%u/%u.*  ",fidoadr.zone,fidoadr.net,fidoadr.node);
   if (validaddress(fidoadr.zone,fidoadr.net,fidoadr.node))
     cprintf("In index.\r\n");
   else
     cprintf(" does not exist.\r\n");
  }
}


