#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "prog.h"

/* Code to handle chinese characters */

#define ISLEFT(c) ((c) > (byte)0x80 && (c) < (byte)0xff)
#define ISRIGHT(c) (((c) >= (byte)0x40 && (c) <= (byte)0x7e) || \
                    ((c) >= (byte)0xa1 && (c) <= (byte)0xfe))

static word near ischin(byte *buf)
{
  return (ISLEFT(buf[0]) && ISRIGHT(buf[1]));
}

char * _fast stristr(char *string,char *search)
{
  /* "register" keyword used to fix the brain-dead MSC (opti)mizer */

  word last_found=0;
  word strlen_search=strlen(search);
  byte l1, l2;
  word i;

  if (string)
  {
    while (*string)
    {
      /**** start chinese modifications *****/
      l1=(byte)(ischin(string) ? 2 : 1);
      l2=(byte)(ischin(search+last_found) ? 2 : 1);

      if (l1==l2)
        i=(l1==1) ? memicmp(string, search+last_found, l1) : 1;
      else i=1;
      
      if (!i)
        last_found += l1;
      /**** end chinese modifications *****/
      /* old code: if ((tolower(*string))==(tolower(search[last_found])))
                     last_found++;
      */
      else
      {
        if (last_found != 0)
        {
          last_found=0;
          continue;
        }
      }

      string += l1;

      if (last_found==strlen_search) return(string-last_found);
    }
  }

  return(NULL);
}

