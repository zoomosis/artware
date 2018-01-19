#include "includes.h"


int pickone(char **choices, int y1, int x1, int y2, int x2)
{
   int l, total, maxlen=0, len, cur=0, col, onescreen, start=0;
   BOX *pickbox;

   for(total=0; (choices[total] != NULL) && (choices[total][0] != '\0'); total++)
     {
     if( (len=strlen(choices[total])) > maxlen)
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


   pickbox = initbox(y1, x1, y2, x2, cfg.col[Casframe], cfg.col[Castext], SINGLE, YES, ' ');
   drawbox(pickbox);

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
       col = (l==cur) ? cfg.col[Cashigh] : cfg.col[Castext];
       if(l == cur)
          MoveXY(x1+2, y1+2+l-start);
       print(y1+1+l-start, x1+1, col, choices[l]);
       }

     switch(get_idle_key(1, GLOBALSCOPE))
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
           break;
        }
     }

}

void free_picklist(char **choices)
{
   int i;

   if(choices == NULL) return;

   for(i=0; choices[i] != NULL; i++)
       mem_free(choices[i]);

   mem_free(choices);
}
