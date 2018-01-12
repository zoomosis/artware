#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <time.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>

#include <msgapi.h>
#include <progprot.h>

#include "nstruct.h"

extern CFG cfg;

int read_key(void);
void check_registration(void);



void check_registration(void)
{
   int i;
   char name[36];
   char *charptr;
   dword ascvals=0L, ascvals2=0L, ascvals3=0L;
   word ascvals4=0;
   dword total=0L;
   dword code;
   dword crc, hash, ncode;

   strcpy(name, cfg.registername);
   strlwr(name);

   if(REGISTERED)
     {
     cfg.registered = 0;
     return;
     }

   crc  = JAMsysCrc32(name, strlen(name),-1L) /3;
   hash = SquishHash(name)/3;
   ncode = NETMGRCODE/3;

   code =   (JAMsysCrc32(name, strlen(name),-1L) /3 ) +
                                (SquishHash(name)/3) +
                                (NETMGRCODE/3);

   if(cfg.key.strkey.keyval != code)
      {
      printf("Invalid keycode!\n");
      return;
      }


   for(charptr=name; *charptr; charptr++)
      ascvals += (unsigned char) (*charptr + 27);

   if(cfg.key.strkey.ascvals != ascvals)
    {
    printf("Invalid key!\n");
    return;
    }

   for(charptr=name; *charptr; charptr++)
      ascvals2 += ((unsigned char) ((*charptr*2) + 3));

   if(cfg.key.strkey.ascvals2 != ascvals2)
     {
     printf("Invalid key!\n");
     return;
     }

//   for(charptr=name; *charptr; charptr++)
//      ascvals3 += ((unsigned char) ( ((*charptr/3)*5) + 11) );

//   if(cfg.key.strkey.ascvals3 != ascvals3)
//     {
//     Message("Invalid key!", -1, 0, YES);
//     return;
//     }

//   for(charptr=name; *charptr; charptr++)
//      ascvals4 += (unsigned char) (255 - *charptr);

//   if(cfg.key.strkey.ascvals4 != ascvals4)
//     {
//     Message("Invalid key!", -1, 0, YES);
//     return;
//     }

//   for(i=0; i<100; i++)
//      total += cfg.key.arrkey.bytes[i];

//   total -= cfg.key.arrkey.bytes[91];
//   total -= cfg.key.arrkey.bytes[92];
//   total -= cfg.key.arrkey.bytes[93];
//   total -= cfg.key.arrkey.bytes[94];

//   if(total != cfg.key.strkey.total)
//     {
//     Message("Invalid key!", -1, 0, YES);
//     return;
//     }

   cfg.registered = 1;

}


int read_key(void)
{
   int regfile;
   char filename[120];

   strcpy(filename, "netmgr.key");

   if( (regfile=sopen(filename, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
      {
      if(cfg.homedir[0] != '\0')
         sprintf(filename, "%s\\netmgr.key", cfg.homedir);

      if( (regfile=sopen(filename, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
          {
          if(cfg.cfgdir[0] != '\0')
             sprintf(filename, "%s\\netmgr.key", cfg.cfgdir);

          if( (regfile=sopen(filename, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
               return 0;
          }
      }

   if(read(regfile, &cfg.key, sizeof(union KEY)) != sizeof(union KEY))
      {
      printf("Error reading keyfile!\n");
      return 0;
      }

   close(regfile);

   return 1;
}


