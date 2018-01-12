#include "includes.h"

void edit_hello_strings(AREA *area);


#define NORMALCHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'. !?,*&^#@$%()-_=+[]{}\\\"|';:,.<>/?`~1234567890"


void edit_hello_strings(AREA *area)
{

   char datafile[100], origfile[100], action=0, temp[301];
   FILE *data, *orig;
   BOX *editbox;
   int ret=1, changed=0;


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

   editbox = initbox(3,0,22,79,cfg.col[Cfindactive],cfg.col[Cfindtext],DOUBLE,YES,' ');
   drawbox(editbox);
   boxwrite(editbox, 1 , 2, "Edit 'rephello' and 'hello' strings:");
   boxwrite(editbox, 3 , 2, "Start replies with:");
   vprint(8,3,cfg.col[Cfinddata], "%-76.76s", custom.rephello);
   boxwrite(editbox, 6 , 2, "Start new messages with:");
   vprint(11, 3, cfg.col[Cfinddata], "%-76.76s", custom.hello);
   boxwrite(editbox, 9,  2, "Start followup with:");
   vprint(14,3,cfg.col[Cfinddata], "%-76.76s", custom.followhello);
   boxwrite(editbox, 12 , 2, "End messages with:");
   vprint(17,3,cfg.col[Cfinddata], "%-76.76s", custom.signoff);
   boxwrite(editbox, 15, 2, "Origin:");
   vprint(20,3,cfg.col[Cfinddata], "%-76.76s", custom.origin);

   while(action < 5)
      {
      ret=1;
      switch(action)
         {
         case 0:

            while( (ret != ESC) && (ret != 0) )
               {
               if(strlen(custom.rephello) < 301)
                  strcpy(temp, custom.rephello);
               ret = getstring(8,3,custom.rephello,76,300,"",cfg.col[Centry], cfg.col[Cfinddata]);
               if( (strlen(custom.rephello) < 301) &&
                   strcmp(temp, custom.rephello) )
                   changed = 1;
               }
            action = (ret == 0) ? 1 : 5;
            break;

         case 1:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.hello) < 301)
              strcpy(temp, custom.hello);
              ret = getstring(11,3,custom.hello,76,300,"",cfg.col[Centry], cfg.col[Cfinddata]);
              if( (strlen(custom.hello) < 301) &&
                  strcmp(temp, custom.hello) )
                  changed = 1;
              }

            if ( (ret == UP) || (ret == BTAB) )
               action = 0;
            else action = (ret == 0) ? 2 : 5;
            break;

         case 2:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.followhello) < 301)
              strcpy(temp, custom.followhello);
              ret = getstring(14,3,custom.followhello,76,300,"",cfg.col[Centry], cfg.col[Cfinddata]);
              if( (strlen(custom.followhello) < 301) &&
                  strcmp(temp, custom.followhello) )
                  changed = 1;
              }

            if ( (ret == UP) || (ret == BTAB) )
               action = 1;
            else action = (ret == 0) ? 3 : 5;
            break;

         case 3:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.signoff) < 301)
              strcpy(temp, custom.signoff);
              ret = getstring(17,3,custom.signoff,76,300,"",cfg.col[Centry], cfg.col[Cfinddata]);
              if( (strlen(custom.signoff) < 301) &&
                  strcmp(temp, custom.signoff) )
                  changed = 1;
              }

            if ( (ret == UP) || (ret == BTAB) )
               action = 2;
            else action = (ret == 0) ? 4 : 5;
            break;

         case 4:

            while( (ret != ESC) && (ret != 0) && (ret != UP) && (ret != BTAB) )
              {
              if(strlen(custom.origin) < 76)
              strcpy(temp, custom.origin);
              ret = getstring(20,3,custom.origin,76,76,"",cfg.col[Centry], cfg.col[Cfinddata]);
              if( (strlen(custom.origin) < 76) &&
                  strcmp(temp, custom.origin) )
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

   if(!(TIMREGISTERED))
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

   char datafile[100], origfile[100];
   FILE *data, *orig;

   char *charptr, name[36];
   word ascvals4=0L;

   memset(custom.hello,       '\0', sizeof(custom.hello      ));
   memset(custom.rephello,    '\0', sizeof(custom.rephello   ));
   memset(custom.followhello, '\0', sizeof(custom.followhello));
   memset(custom.signoff,     '\0', sizeof(custom.signoff    ));
   memset(custom.origin,      '\0', sizeof(custom.origin     ));

   custom.aka      = area->aka;

   strcpy(custom.csread, area->csread);   // Default charset to use.
   custom.csreadlevel = 0;

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

   if( (TIMREGISTERED) && ((data=fopen(datafile, "r")) != NULL) )
      {
      // ================== Now some stuff to do extra checks (reg'ed only)

      #ifndef __SENTER__
        strcpy(name, cfg.usr.name[0].name);
        strlwr(name);

        for(charptr=name; *charptr; charptr++)
           ascvals4 += (unsigned char) (255 - *charptr);

        if(cfg.key.strkey.ascvals4 != ascvals4)
          {
          cfg.first->next = NULL;
          }
      #endif

      // ================== End of stuff


      if(area->base & MSGTYPE_HMB)  // For Hudson, origin is here too
         {
         fgets(custom.origin, 99, data);
         Strip_Trailing(custom.origin, '\n');
         if(stristr(custom.origin, "<default>") != 0)
             strcpy(custom.origin, cfg.usr.origin);
         }

      fgets(custom.rephello, 300, data);
      Strip_Trailing(custom.rephello, '\n');
      if(stristr(custom.rephello, "<default>") != 0)
         strcpy(custom.rephello, cfg.usr.rephello);

      if(!feof(data))
         {
         fgets(custom.hello, 300, data);
         Strip_Trailing(custom.hello, '\n');
         if(stristr(custom.hello, "<default>") != 0)
             strcpy(custom.hello, cfg.usr.hello);
         }

      if(!feof(data))
         {
         fgets(custom.signoff, 300, data);
         Strip_Trailing(custom.signoff, '\n');
         if(stristr(custom.signoff, "<default>") != 0)
            strcpy(custom.signoff, cfg.usr.signoff);
         }

      if(!feof(data))
         {
         fgets(custom.followhello, 300, data);
         Strip_Trailing(custom.followhello, '\n');
         if(stristr(custom.followhello, "<default>") != 0)
            strcpy(custom.followhello, cfg.usr.followhello);
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
     if( (TIMREGISTERED) && ((orig=fopen(origfile, "r")) != NULL) )
        {
        fgets(custom.origin, 99, orig);
        Strip_Trailing(custom.origin, '\n');
        fclose(orig);

        if(stristr(custom.origin, "<default>") != 0)
          strcpy(custom.origin, cfg.usr.origin);
        }
     else strcpy(custom.origin, cfg.usr.origin);
     }

}

