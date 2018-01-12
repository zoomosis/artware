#include <stdio.h>
#include <video.h>
#include <scrnutil.h>
#include <alloc.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>

#include <msgapi.h>
#include "wrap.h"
#include "tstruct.h"
#include "idlekey.h"
#include "global.h"
#include "showhdr.h"
#include "showhdr.h"
#include "input.h"

#include "mysqdata.h"

void showinfo(MMSG *curmsg, AREA *area, MSG *areahandle);


void showinfo(MMSG *curmsg, AREA *area, MSG *areahandle)

{
   BOX *infobox, editbox;
   char temp[81];
   struct _mysqdata * areadata = (struct _mysqdata *)areahandle->apidata;
   int key, action, ret=1;
   char max[5], skip[5], keep[5];


   #ifndef __OS2__
   infobox = initbox(0,7,22,73,cfg.col.popframe,cfg.col.poptext,S_VERT,YES,' ');
   #else
   infobox = initbox(0,7,17,73,cfg.col.popframe,cfg.col.poptext,S_VERT,YES,' ');
   #endif
   drawbox(infobox);

   /* Info about the current message */

   boxwrite(infobox, 0, 1, "Message info:");

   sprintf(temp, "Message date : %s", MakeT(&curmsg->hdr.date_written));
   boxwrite(infobox, 2, 1, temp);
   sprintf(temp, "Arrival date : %s", MakeT(&curmsg->hdr.date_arrived));
   boxwrite(infobox, 3, 1, temp);
   sprintf(temp, "Message size : %lu (body: %lu  controlinfo: %lu)",
                               (dword) (curmsg->txtsize + curmsg->ctrlsize),
                               curmsg->txtsize,
                               curmsg->ctrlsize);
   boxwrite(infobox, 4, 1, temp);
   sprintf(temp, "Message UID  : %-5ld", curmsg->uid);
   boxwrite(infobox, 5, 1, temp);

   /* Info about the current area */

   boxwrite(infobox, 7, 1, "Area info:");

   sprintf(temp, "Desc : %-50.50s", area->desc);
   boxwrite(infobox, 9, 1, temp);

   sprintf(temp, "Tag  : %s", area->tag);
   boxwrite(infobox, 10, 1, temp);

   if(area->type == ECHOMAIL)
      strcpy(temp, "Type : Echomail");
   else if(area->type == NETMAIL)
      strcpy(temp, "Type : Netmail");
   else strcpy(temp, "Type : Local");
   boxwrite(infobox, 11, 1, temp);

   sprintf(temp, "Attr : %s", attr_to_txt(area->stdattr));
   boxwrite(infobox,12,1,temp);

   if(!(area->base & MSGTYPE_HMB))
      sprintf(temp, "Path : %-50.50s", area->dir);
   else
      sprintf(temp, "Board: %-50.50s", area->dir);

   boxwrite(infobox,13,1,temp);

   if(area->base & MSGTYPE_SDM)
      sprintf(temp, "Base : *.MSG");
   else if(area->base & MSGTYPE_JAM)
      sprintf(temp, "Base : JAM");
   else if(area->base & MSGTYPE_HMB)
      sprintf(temp, "Base : Hudson");
   else
      sprintf(temp,
        "Base : Squish (Max=%lu Skip=%lu Days=%u) Hit 'C' to change.",
                                areadata->max_msg,
                                areadata->skip_msg,
                                areadata->keep_days);

   boxwrite(infobox,14,1,temp);

   sprintf(temp, "AKA  : %d:%d/%d.%d", cfg.usr.address[area->aka].zone,
                                       cfg.usr.address[area->aka].net,
                                       cfg.usr.address[area->aka].node,
                                       cfg.usr.address[area->aka].point);
   boxwrite(infobox,15,1,temp);

   /* General info about system */

   #ifndef __OS2__
   boxwrite(infobox, 17, 1, "General info:");

     sprintf(temp, "Memory free : %dK", ((unsigned long)coreleft())/1024);
     boxwrite(infobox, 19,1,temp);
     if(_osmajor > 9)
        sprintf(temp, "DOS version : OS/2-DOS");
     else sprintf(temp, "DOS version : %d.%d", _osmajor, _osminor);
     boxwrite(infobox, 20, 1, temp);
   #endif


   while( ((key=get_idle_key(1)) == 99) || (key==67) )
      {
      if(!(area->base & MSGTYPE_SQUISH))
        break; /* We surely don't want to do this for *.MSG! */

      action=0;
      ltoa( (long) areadata->max_msg, max, 10);
      ltoa( (long) areadata->skip_msg, skip, 10);
      itoa( (int)  areadata->keep_days, keep, 10);


      while(action < 3)
      {
      sprintf(temp, "%-4lu Skip=%-4lu Days=%-4u)                 ",
                                areadata->max_msg,
                                areadata->skip_msg,
                                areadata->keep_days);
      boxwrite(infobox,14,20,temp);

      switch(action)
         {
         case 0:

            while( (ret != ESC) && (ret != 0) && (ret != TAB) )
               ret = getstring(15,28,max,4,"0123456789",cfg.col.entry);

            areadata->max_msg   = atol(max );

            action = (ret == ESC) ? 3 : 1;
            ret=1;
            break;

         case 1:

            while( (ret != ESC) && (ret != 0) && (ret!=BTAB) && (ret!=TAB) )
               ret = getstring(15,38,skip,4,"0123456789",cfg.col.entry);

            areadata->skip_msg  = atol(skip);

            if( (ret == 0) || (ret == TAB) )
               action++; /* move one further */

            else if(ret == BTAB)
               action--; /* Move one back */
            else action=3; /* Quit, ESC */

            ret=1;
            break;

         case 2:

            while( (ret != ESC) && (ret != 0) && (ret != BTAB) && (ret!=TAB) )
               ret = getstring(15,48,keep,4,"0123456789",cfg.col.entry);

            areadata->keep_days = atoi(keep);

            if( (ret == 0) || (ret == TAB) )
               action++; /* move one further */
            else if(ret == BTAB)
               action--; /* Move one back */
            else action=3; /* Quit, ESC */

            ret=1;
            break;

         default:
            break;    /* nothing */
         }
      }


      sprintf(temp, "Base : Squish (Max=%lu Skip=%lu Days=%u) Hit 'C' to change.",
                                areadata->max_msg,
                                areadata->skip_msg,
                                areadata->keep_days);
      boxwrite(infobox,14,1,temp);
      }


   delbox(infobox);

   if(key == 126)
      {
      FILE *out;
      struct heapinfo hi;
      int i;


      if( (out=fopen("chain.txt", "w+")) == NULL )
           return;

      hi.ptr = NULL;
      fprintf(out, "Pointer       Size   Status\n" );
      fprintf(out, "-------       ----   ------\n\n" );

      while( heapwalk( &hi ) == _HEAPOK )
         {
         fprintf(out, "%p  %7u    ", hi.ptr, hi.size);
         if(hi.in_use)
            fprintf(out, "Used.\n");
         else
            fprintf(out, "Free. <--\n");
         }

      fclose(out);
      }
}



