#include "includes.h"

int read_key(void);
void check_registration(void);


int read_key(void)
{
   int regfile;
   char filename[120];
   int got, want;

   if(cfg.homedir[0] != '\0')
      sprintf(filename, "%s\\timed.key", cfg.homedir);
   else
     strcpy(filename, "timed.key");

   if( (regfile=sopen(filename, O_BINARY | O_RDONLY, SH_DENYNO)) == -1)
      return 0;

   want = (int) sizeof(union KEY);
   got = read(regfile, &cfg.key, (unsigned) want);

   if(want != got)
      {
      Message("Error reading keyfile!", -1, 254, YES);
      return 0;
      }

   close(regfile);

   return 1;
}




void check_registration(void)
{
   char name[36];
   char *charptr;
//   dword ascvals=0L, ascvals2=0L;
   word ascvals4=0;
   dword ascvals3=0L;
//   dword total=0L;

   #ifdef __SENTER__

      *(cfg.usr.registered) = 1;

   #else


   strcpy(name, cfg.usr.name[0].name);
   strlwr(name);

   if(TIMREGISTERED)
     {
     *(cfg.usr.registered) = 0;
     return;
     }

//   for(charptr=name; *charptr; charptr++)
//      ascvals += (unsigned char) (*charptr + 27);

//   if(cfg.key.strkey.ascvals != ascvals)
//    {
//    Message("Invalid key!", -1, 0, YES);
//    return;
//    }

   for(charptr=name; *charptr; charptr++)
      ascvals4 += (unsigned char) (255 - *charptr);

   if(cfg.key.strkey.keyval != ( (cfg.usr.name[0].crc/3) +
                                 (cfg.usr.name[0].hash/3) +
                                 (TIMEDCODE/3) ) )
      {
      Message("** Invalid keycode! **", -1, 0, YES);
      return;
      }

   if(cfg.key.strkey.ascvals4 != ascvals4)
     {
     Message("** Invalid key! **", -1, 0, YES);
     return;
     }

//   for(charptr=name; *charptr; charptr++)
//      ascvals2 += ((unsigned char) ((*charptr*2) + 3));

//   if(cfg.key.strkey.ascvals2 != ascvals2)
//     {
//     Message("Invalid key!", -1, 0, YES);
//     return;
//     }

   for(charptr=name; *charptr; charptr++)
      ascvals3 += ((unsigned char) ( ((*charptr/3)*5) + 11) );

//   if(cfg.key.strkey.ascvals3 != ascvals3)
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

   strcat(myname, "+");

   *(cfg.usr.registered) = 1;

   #endif
}

