#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <video.h>
#include <scrnutil.h>
#include <msgapi.h>

#include "wrap.h"
#include "tstruct.h"
#include "global.h"
#include "input.h"
#include "message.h"

void edit_hello_strings(AREA *area);


#define NORMALCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'. !?,*&^#@$%()-_=+[]{}\\\"|';:,.<>/?`~1234567890"


void edit_hello_strings(AREA *area)
{

   char datafile[100], origfile[100], action=0, temp[80];
   FILE *data, *orig;
   BOX *editbox;
   int ret=1, i, changed=0;


   if(area->base & MSGTYPE_SQUISH)
     {
     sprintf(datafile, "%s.SQT", area->dir);
     sprintf(origfile, "%s.SQO", area->dir);
     }
   else if(area->base & MSGTYPE_SDM)
     {
     sprintf(datafile, "%s\\timed.dat", area->dir);
     sprintf(origfile, "%s\\origin", area->dir);
     }
   else if(area->base & MSGTYPE_JAM) /* JAM */
     {
     sprintf(datafile, "%s.JTI", area->dir);
     sprintf(origfile, "%s.JTO", area->dir);
     }
   else
     {
     sprintf(datafile, "%s\\MSG%d.TIM", cfg.usr.hudsonpath, atoi(area->dir));
     }

   editbox = initbox(3,0,22,79,112,3,DOUBLE,YES,' ');
   drawbox(editbox);
   boxwrite(editbox, 1 , 2, "Edit 'rephello' and 'hello' strings:");
   boxwrite(editbox, 3 , 2, "Start replies with:");
   sprintf(temp, "%-76.76s", custom.rephello);
   print(8, 3, 7, temp);
   boxwrite(editbox, 6 , 2, "Start new messages with:");
   sprintf(temp, "%-76.76s", custom.hello);
   print(11 , 3, 7,temp);
   boxwrite(editbox, 9,  2, "Start followup with:");
   sprintf(temp, "%-76.76s", custom.followhello);
   print(14, 3, 7, temp);
   boxwrite(editbox, 12 , 2, "End messages with:");
   sprintf(temp, "%-76.76s", custom.signoff);
   print(17, 3, 7, temp);
   boxwrite(editbox, 15, 2, "Origin:");
   sprintf(temp, "%-76.76s", custom.origin);
   print(20, 3, 7, temp);

   while(action < 5)
      {
      ret=1;
      switch(action)
         {
         case 0:

            while( (ret != ESC) && (ret != 0) )
               {
               if(strlen(custom.rephello) < 80)
                  strcpy(temp, custom.rephello);
               ret = getstring(8,3,custom.rephello,76,"",cfg.col.entry);
               if( (strlen(custom.rephello) < 80) &&
                   strcmpi(temp, custom.rephello) )
                   changed = 1;
               }
            action = (ret == 0) ? 1 : 5;
            break;

         case 1:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.hello) < 80)
              strcpy(temp, custom.hello);
              ret = getstring(11,3,custom.hello,76,"",cfg.col.entry);
              if( (strlen(custom.hello) < 80) &&
                  strcmpi(temp, custom.hello) )
                  changed = 1;
              }

            if ( (ret == UP) || (ret == BTAB) )
               action = 0;
            else action = (ret == 0) ? 2 : 5;
            break;

         case 2:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.followhello) < 80)
              strcpy(temp, custom.followhello);
              ret = getstring(14,3,custom.followhello,76,"",cfg.col.entry);
              if( (strlen(custom.followhello) < 80) &&
                  strcmpi(temp, custom.followhello) )
                  changed = 1;
              }

            if ( (ret == UP) || (ret == BTAB) )
               action = 1;
            else action = (ret == 0) ? 3 : 5;
            break;

         case 3:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.signoff) < 80)
              strcpy(temp, custom.signoff);
              ret = getstring(17,3,custom.signoff,76,"",cfg.col.entry);
              if( (strlen(custom.signoff) < 80) &&
                  strcmpi(temp, custom.signoff) )
                  changed = 1;
              }

            if ( (ret == UP) || (ret == BTAB) )
               action = 2;
            else action = (ret == 0) ? 4 : 5;
            break;

         case 4:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.origin) < 80)
              strcpy(temp, custom.origin);
              ret = getstring(20,3,custom.origin,76,"",cfg.col.entry);
              if( (strlen(custom.origin) < 80) &&
                  strcmpi(temp, custom.origin) )
                  changed = 1;
              }

            action = ( (ret == UP) || (ret == BTAB) ) ? 3 : 5;

         default:
            break;    /* nothing */
         }
      }

   delbox(editbox);

   if ( (changed == 0) ||
        (confirm("Data changed. Save? [y/N]") == 0) )
    return;

   if(!(REGISTERED))
     {
     Message("Permanently saving is only possible in registered versions.", -1, 0, YES);
     return;
     }

   if( (data=fopen(datafile, "w")) != NULL )
      {
      if(area->base & MSGTYPE_HMB) // For Hudson we add it here..
         fprintf(data, "%s\n", custom.origin);

      fprintf(data, "%s\n", custom.rephello    );
      fprintf(data, "%s\n", custom.hello       );
      fprintf(data, "%s\n", custom.signoff     );
      fprintf(data, "%s\n", custom.followhello );

      if(strlen(custom.sargs[0].string) > 2)
         {
         fprintf(data, "%s\n", custom.allareas ? "all" : "current");
         fprintf(data, "%s\n", custom.allmsgs  ? "all" : "lastread");

         for(i=0; i<10; i++)
            fprintf(data, "%s\n", custom.sargs[i].string);
         }

      fclose(data);
      }
   else
      {
      sprintf(msg, "Can't open %s!", datafile);
      Message(msg, -1, 0, YES);
      }

   if(!(area->base & MSGTYPE_HMB))
     {
     if( (orig=fopen(origfile, "w")) != NULL )
        {
        fprintf(orig, "%s\n", custom.origin);
        fclose(orig);
        }
     else
        {
        sprintf(msg, "Can't open %s!", origfile);
        Message(msg, -1, 0, YES);
        }
     }

}


/* ------------------------------------------------------------ */
/* Read the file xxxx.SQT (or timed.dat for *.MSG) and get data */
/* ------------------------------------------------------------ */


void get_custom_info(AREA *area)
{

   char datafile[100], origfile[100], temp[100];
   FILE *data, *orig;
   int i;


   memset(custom.hello,       '\0', sizeof(custom.hello      ));
   memset(custom.rephello,    '\0', sizeof(custom.rephello   ));
   memset(custom.followhello, '\0', sizeof(custom.followhello));
   memset(custom.signoff,     '\0', sizeof(custom.signoff    ));
   memset(custom.origin,      '\0', sizeof(custom.origin     ));
   memset(&custom.sargs,      '\0', sizeof(custom.sargs      ));

   custom.aka      = area->aka;
   custom.allareas = 0;
   custom.allmsgs  = 0;


   if(area->base & MSGTYPE_SQUISH)
     {
     sprintf(datafile, "%s.SQT", area->dir);
     sprintf(origfile, "%s.SQO", area->dir);
     }
   else if(area->base & MSGTYPE_SDM)
     {
     sprintf(datafile, "%s\\timed.dat", area->dir);
     sprintf(origfile, "%s\\origin", area->dir);
     }
   else if(area->base & MSGTYPE_JAM) /* JAM */
     {
     sprintf(datafile, "%s.JTI", area->dir);
     sprintf(origfile, "%s.JTO", area->dir);
     }
   else  /* HMB */
     {
     sprintf(datafile, "%s\\MSG%d.TIM", cfg.usr.hudsonpath, atoi(area->dir));
     }

   if( (REGISTERED) && ((data=fopen(datafile, "r")) != NULL) )
      {
      if(area->base & MSGTYPE_HMB)  // For Hudson, origin is here too
         {
         fgets(custom.origin, 190, data);
         if(custom.origin[strlen(custom.origin)-1] == '\n')
            custom.origin[strlen(custom.origin)-1] = '\0';
         }

      fgets(custom.rephello, 100, data);
      if(
          (strlen(custom.rephello) < 3) ||
          (strncmpi(custom.rephello, "<default>", 9) == 0)
        )
         strcpy(custom.rephello, cfg.usr.rephello);
      else if (custom.rephello[strlen(custom.rephello)-1] == '\n')
              custom.rephello[strlen(custom.rephello)-1] = '\0';

      if(!feof(data))
         {
         fgets(custom.hello, 100, data);
         if(
             (strlen(custom.hello) < 3) ||
             (strncmpi(custom.hello, "<default>", 9) == 0)
           )
            strcpy(custom.hello, cfg.usr.hello);
         else if (custom.hello[strlen(custom.hello)-1] == '\n')
                 custom.hello[strlen(custom.hello)-1] = '\0';
         }

      if(!feof(data))
         {
          fgets(custom.signoff, 100, data);
          if(
              (strlen(custom.signoff) < 3) ||
              (strncmpi(custom.signoff, "<default>", 9) == 0)
            )
             strcpy(custom.signoff, cfg.usr.signoff);
          else if (custom.signoff[strlen(custom.signoff)-1] == '\n')
                  custom.signoff[strlen(custom.signoff)-1] = '\0';
         }

      if(!feof(data))
         {
         fgets(custom.followhello, 100, data);
         if(
              (strlen(custom.followhello) < 3) ||
              (strncmpi(custom.followhello, "<default>", 9) == 0)
           )
            strcpy(custom.followhello, cfg.usr.followhello);
         else if (custom.followhello[strlen(custom.followhello)-1] == '\n')
                 custom.followhello[strlen(custom.followhello)-1] = '\0';
         }


      if(!feof(data))
        {
         if(fgets(temp, 30, data) != NULL)
            {
            if(strncmpi(temp, "all", 3) == 0)
                custom.allareas = 1;
            }
         if( (!feof(data)) && (fgets(temp, 30, data) != NULL) )
            {
            if(strncmpi(temp, "all", 3) == 0)
                custom.allmsgs = 1;
            }
        }


      for(i=0; i<10; i++)
         {
         if(!feof(data))
            {
            if(fgets(temp, 30, data) != NULL)
               {
               temp[25] = '\0';
               strcpy(custom.sargs[i].string, temp);
               }
            }
         }

      fclose(data);
      }
   else  // Can't open or not registered
     {
     strcpy(custom.hello,       cfg.usr.hello      );
     strcpy(custom.rephello,    cfg.usr.rephello   );
     strcpy(custom.signoff,     cfg.usr.signoff    );
     strcpy(custom.followhello, cfg.usr.followhello);

     if(area->base & MSGTYPE_HMB)
       strcpy(custom.origin, cfg.usr.origin);
     }

   /* Now lets get the origin! */

   if(!(area->base & MSGTYPE_HMB))
     {
     if( (REGISTERED) && ((orig=fopen(origfile, "r")) != NULL) )
        {
        fgets(custom.origin, 190, orig);
        if(custom.origin[strlen(custom.origin)-1] == '\n')
           custom.origin[strlen(custom.origin)-1] = '\0';
        fclose(orig);
        }
     else strcpy(custom.origin, cfg.usr.origin);
     }

}

