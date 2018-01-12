#include <stdio.h>
#include <io.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <dir.h>
#include <conio.h>
#include <sys\stat.h>

#include <typedefs.h>
#include <video.h>
#include <scrnutil.h>

#include "input.h"
#include "idlekey.h"
#include "message.h"

#define NETMGRCODE 0xAEAE5656L
#define REGSITE   4           // 1 == me, 2 == evin, 3 = Steve, 4 = Sven


typedef struct
{
   char   rand1[12];
   dword  generated;
   char   rand2[10];
   dword  keyval;
   char   rand3[7];
   dword  ascvals;
   dword  crc2;
   char   regsite;
   word   serial;
   dword  ascvals2;
   dword  ascvals3;
   char   rand4[17];
   word   ascvals4;
   word   zero;
   char   rand5[14];
   dword  total;
   char   rand6[33];

} STRKEY;


typedef struct
{

   char   bytes[128];

} ARRKEY;


union KEY
{
   STRKEY strkey;
   ARRKEY arrkey;
} key;


word serial;   // Serial number to use now

//union KEY key;

dword SquishHash(byte *f);
dword JAMsysCrc32(void *pBuf, unsigned int len, dword crc);
void GenerateKey(char *name);


void main(void)
{
   int ok;
   char name[36],
        lowername[36],
        number[10],
        keyfilename[100],
        descfilename[100],
        filename[100];

   FILE *sfile, *descfile, *logfile;
   int keyfile;
   BOX *hdr;
   char temp[120], keyname[20];
   time_t curtime;

   video_init();
   check_enhanced();
   cls();

   hdr = initbox(0,0,2,79,113,113,S_VERT,NO, ' ');
   drawbox(hdr);
   delbox(hdr);

   print(1,2,113," NetMgr key generator ù Exclusively for Sven Mueller ù DO NOT DISTRIBUTE!");


   if( (sfile = fopen("serial.txt", "r")) == NULL )
      {
      print(5,0,7,"Can't open serial.txt!");
      MoveXY(1,7);
      exit(254);
      }

   if(fgets(number, 9, sfile) == NULL)
      {
      print(5,0,7,"Can't read serial.txt!");
      MoveXY(1,7);
      exit(254);
      }

   fclose(sfile);

   serial = atoi(number);  // This is the serial number we will now use.

   print(4,0,7," þ Reading serial.txt...");
   sprintf(temp, " þ Next serial # to be used: %d", serial);
   print(5,0,7,temp);
   print(6,0,7," þ Registername:");

   memset(name, '\0', sizeof(name));
   if(getstring(6, 17, name, 35, "", 30) == ESC)
     goto Exit;

   strcpy(lowername, name);
   strlwr(lowername);

   print(7,0,7," þ Filename:");

   ok=0;
   do
     {
     memset(filename, '\0', sizeof(filename));
     if(getstring(7, 13, filename, 65, "", 30) == ESC)
        goto Exit;

     sprintf(keyfilename,  "%s.KEY", filename);
     sprintf(descfilename, "%s.ASC", filename);

     if( (keyfile=open(keyfilename, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE)) == -1)
        Message("Error opening file!", -1, 0, YES);
     else if( (descfile=fopen(descfilename, "w+")) == NULL)
        Message("Error opening file!", -1, 0, YES);
     else
        ok = 1;
     }
   while(!ok);

   GenerateKey(lowername);

   sprintf(temp, " þ Writing keyfile (%s).", keyfilename);
   print(8,0,7,temp);

   if(write(keyfile, &key, sizeof(key)) != sizeof(key))
     {
     print(8,0,7, "Error writing keyfile!");
     serial--;
     close(keyfile);
     fclose(descfile);
     goto Exit;
     }

   close(keyfile);

   sprintf(temp, " þ Writing msgfile (%s).", descfilename);
   print(9,0,7,temp);

   fnsplit(keyfilename, NULL, NULL, keyname, NULL);
   fprintf(descfile, "\nRegistername: \"%s\"\n\n", name);
   fprintf(descfile, "The registration key was calculated using the 'registername'\n");
   fprintf(descfile, "listed above. You should put the statement:\n");
   fprintf(descfile, "RegisterName %s\nin NetMgr.cfg\n", name);
   fprintf(descfile, "The file %s.KEY holds your registration key, and should\n", strupr(keyname));
   fprintf(descfile, "be renamed to NETMGR.KEY in the NetMgr directory.\n");

   fclose(descfile);

   if( (logfile = fopen("nkey.log", "a")) == NULL )
     Message("Error opening logile!", -1, 0, YES);
   else
     {
     curtime = time(NULL);
     fprintf(logfile, "\n ==> %s (NetMgr)\n", ctime(&curtime));
     fprintf(logfile, " þ Generated key for : \"%s\"\n", name);
     fprintf(logfile, " þ Filename          : %s\n", keyfilename);
     fprintf(logfile, " þ Serial number     : %d\n\n", key.strkey.serial);
     fclose(logfile);
     }


   Exit:

   if( (sfile=fopen("serial.txt", "w+")) == NULL )
      {
      Message("Can't open serial.txt for write!", -1, 0, YES);
      exit(254);
      }

   fprintf(sfile, "%d\n", serial);
   fclose(sfile);

   print(10,0,7," þ Writing serial.txt...");
   print(11,0,7," þ Clean up successful, press any key to exit.");

   get_idle_key();

   MoveXY(1,20);
   _setcursortype(_NORMALCURSOR);

}


// =======================================
// Generate our registration key in here..
// =======================================

void GenerateKey(char *name)
{
   dword crc;
   dword hash;
   dword total = 0L;
   char *charptr;
   int i;
   dword ncode = NETMGRCODE/3;

   memset(&key, '\0', sizeof(union KEY));

   crc              = JAMsysCrc32(name, strlen(name), -1L);
   key.strkey.crc2  = JAMsysCrc32(name, strlen(name), 0xEE55DD12L);
   hash             = SquishHash(name);

   for(charptr=name; *charptr; charptr++)
      key.strkey.ascvals += ((unsigned char) (*charptr + 27));

   for(charptr=name; *charptr; charptr++)
      key.strkey.ascvals2 += ((unsigned char) ((*charptr*2) + 3));

   for(charptr=name; *charptr; charptr++)
      key.strkey.ascvals3 += ((unsigned char) ( ((*charptr/3)*5) + 11) );

   for(charptr=name; *charptr; charptr++)
      key.strkey.ascvals4 += ((unsigned char) (255 - *charptr));

   key.strkey.keyval = ( (crc/3) + (hash/3) + (NETMGRCODE/3) );

   key.strkey.generated = time(NULL);

   randomize();

   for(i=0; i<12; i++)
      key.strkey.rand1[i] = (unsigned char) random(255);

   key.strkey.rand1[3] = 0;

   for(i=0; i<10; i++)
      key.strkey.rand2[i] = (unsigned char) random(255);

   key.strkey.rand2[7] = 0;

   for(i=0; i<7; i++)
      key.strkey.rand3[i] = (unsigned char) random(255);

   for(i=0; i<17; i++)
      key.strkey.rand4[i] = (unsigned char) random(255);

   key.strkey.rand4[3] = 0;

   for(i=0; i<14; i++)
      key.strkey.rand5[i] = (unsigned char) random(255);

   key.strkey.rand5[6] = 0;

   for(i=0; i<33; i++)
      key.strkey.rand6[i] = (unsigned char) random(255);

   key.strkey.serial = serial++;

   key.strkey.regsite = REGSITE;

   for(i=0; i<100; i++)
      total += key.arrkey.bytes[i];

   key.strkey.total = total;

}


// =============================================
// Calculate the hash value for a certain string
// =============================================


dword SquishHash(byte *f)
{
  dword hash=0, g;
  char *p;

  for (p=f; *p; p++)
  {
    hash=(hash << 4) + tolower(*p);

    if ((g=(hash & 0xf0000000L)) != 0L)
    {
      hash |= g >> 24;
      hash |= g;
    }
  }

  /* Strip off high bit */

  return (hash & 0x7fffffffLu);
}
