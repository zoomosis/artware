#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "video.h"
#include "scrnutil.h"


#define COLFRAME     11
#define COLTITLE     30
#define COLTEXT       7
#define COLCURRENT  113
#define COLCURHL    126
#define COLHL        14
#define COLSTATTEXT   7
#define COLSTATHL    14


int MatchHigh(char **choices, int total, char key);



int picklist(char **choices, char **help, int y1, int x1, int y2, int x2)
{
   int l, total, maxlen=0, len, cur=0, col, colhl, onescreen, start=0;
   BOX *pickbox;
   int key, found;

   for(total=0; (choices[total] != NULL) && (choices[total][0] != '\0'); total++)
     {
     if( (len=HLstrlen(choices[total])) > maxlen)
        {
        if(len < (x2-x1))
           maxlen = len;
        else      /* Chop off */
           {
           maxlen = x2-x1-1;
           choices[total][x2-x1-1] = '\0';
           }
        }
     }

   if(total == 0) return -1;                          /* No choices */

   onescreen = (total > (y2-y1-1)) ? y2-y1-1 : total;

   y2 = y1 + onescreen + 1;

   x2 = (maxlen < (x2-x1-1)) ? x1 + maxlen + 1 : x2;


   pickbox = initbox(y1, x1, y2, x2, COLFRAME, COLTEXT, S_VERT, YES, ' ');
   drawbox(pickbox);
   titlewin(pickbox, TLEFT, "~ test ~", COLTITLE);


   while(1)
     {
     /* check for out of limit values */

     if(cur > total-1)
        cur = total-1;  /* current beyond list */

     if(cur < 0)
        cur = 0;

     if(start < 0)
        start = 0;

     if(start > total - onescreen)
        start = total - onescreen;

     if(cur > start + onescreen - 1)   /* current beyond screen */
        start = cur - onescreen + 1;

     if(cur < start)     /* current 'before' screen */
        start = cur;

     for(l=start; l < (start+onescreen); l++)  /* paint choices + 1 highlighted */
       {
       col   = (l==cur) ? COLCURRENT : COLTEXT;
       colhl = (l==cur) ? COLCURHL : COLHL;
       if(l == cur)
          {
          MoveXY(x1+2, y1+2+l-start);
          if(help != NULL) // print help at bottom..
             biprinteol(maxy-1, 0, COLSTATTEXT, COLSTATHL, help[l], ' ');
          }
       biprint(y1+1+l-start, x1+1, col, colhl, choices[l], ' ');
       }

     switch(key=get_idle_key(1))
        {
        case 336:      /* down */
          cur++;
          break;

        case 328:      /* up */
          cur--;
          break;

        case 327:      /* home */
          cur=0; start=0;
          break;

        case 335:      /* end */
          cur   = total-1;
          start = total-onescreen;
          break;

        case 329:        /* Page Up */
          cur -= onescreen;
          break;

        case 337:       /* Page Down */
          cur += onescreen;
          start += onescreen;
          break;

        case 13:        /* enter */
           delbox(pickbox);
           return cur;

        case 27:         /* esc */
           delbox(pickbox);
           return -1;

        default:
           if( (found=MatchHigh(choices, total, key)) != -1)
             cur = found;
           break;
        }
     }

}



int MatchHigh(char **choices, int total, char key)
{
   int i;
   char match1[4], match2[4];

   sprintf(match1, "~%c~", tolower(key));
   sprintf(match2, "~%c~", toupper(key));

   for(i=0; i<total; i++)
     {
     if(strstr(choices[i], match1) != NULL)
       return i;
     if(strstr(choices[i], match2) != NULL)
       return i;
      }

    return -1;

}


