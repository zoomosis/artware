#include <msgapi.h>
#include <string.h>
#include <alloc.h>
#include <stdio.h>
#include <video.h>
#include <scrnutil.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <time.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>

#include "xmalloc.h"
#include "wrap.h"
#include "tstruct.h"
#include "select.h"
#include "readarea.h"
#include "config.h"
#include "global.h"
#include "message.h"
#include "idlekey.h"

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

   strcpy(name, cfg.usr.name[0].name);
   strlwr(name);

   if(REGISTERED)
     {
     *(cfg.usr.registered) = 0;
     return;
     }

   if(cfg.key.strkey.keyval != ( (cfg.usr.name[0].crc/3) +
                                 (cfg.usr.name[0].hash/3) +
                                 (TIMEDCODE/3) ) )
      {
      Message("Invalid keycode!", -1, 0, YES);
      return;
      }


   for(charptr=name; *charptr; charptr++)
      ascvals += (unsigned char) (*charptr + 27);

   if(cfg.key.strkey.ascvals != ascvals)
    {
    Message("Invalid key!", -1, 0, YES);
    return;
    }

   for(charptr=name; *charptr; charptr++)
      ascvals2 += ((unsigned char) ((*charptr*2) + 3));

   if(cfg.key.strkey.ascvals2 != ascvals2)
     {
     Message("Invalid key!", -1, 0, YES);
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

   *(cfg.usr.registered) = 1;

}


int read_key(void)
{
   int regfile;
   char filename[120];

   if(cfg.homedir[0] != '\0')
      sprintf(filename, "%s\\timed.key", cfg.homedir);
   else
     strcpy(filename, "timed.key");

   if( (regfile=open(filename, O_BINARY | O_RDONLY | SH_DENYNO)) == -1)
      return 0;

   if(read(regfile, &cfg.key, sizeof(union KEY)) != sizeof(union KEY))
      {
      Message("Error reading keyfile!", -1, 254, YES);
      return 0;
      }

   close(regfile);

   return 1;
}





