#include <string.h>
#include "prog.h"

char * _fast Strip_Trailing(char *str,char strip)
{
  int x;

  if (str && *str && str[x=strlen(str)-1]==strip)
    str[x]='\0';

  return str;
}

char * _fast Add_Trailing(char *str,char add)
{
  int x;

  if (str && *str && str[x=strlen(str)-1] != add)
  {
    str[x+1]=add;
    str[x+2]='\0';
  }

  return str;
}

