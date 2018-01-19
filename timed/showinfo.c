#include "includes.h"

char * SumAttaches(MMSG *curmsg);
void ShowFileInfo(MMSG *curmsg);


void showinfo(MMSG *curmsg, AREA *area, MSG *areahandle)

{
   BOX *infobox;
   char temp[81];
   struct _mysqdata * areadata = (struct _mysqdata *)areahandle->apidata;
   int key, action, ret=1;
   char max[5], skip[5], keep[5];
   MMSG fakemsg;

   if(curmsg == NULL)
     {
     memset(&fakemsg, '\0', sizeof(MMSG));
     curmsg = &fakemsg;
     }


   #ifndef __OS2__
   infobox = initbox(0,7,24,73,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   #else
   infobox = initbox(0,7,20,73,cfg.col[Cpopframe],cfg.col[Cpoptext],SINGLE,YES,' ');
   #endif
   drawbox(infobox);

   /* Info about the current message */

   printn(1, 9, cfg.col[Cpoptext], "Message info:", 13);

   vprint(3, 9, cfg.col[Cpoptext], "Date written : %s", MakeT(curmsg->mis.msgwritten, DATE_FULL));
   vprint(4, 9, cfg.col[Cpoptext], "Date arrived : %s", MakeT(curmsg->mis.msgprocessed, DATE_FULL));
   vprint(5, 9, cfg.col[Cpoptext], "Date received: %s", MakeT(curmsg->mis.msgreceived, DATE_FULL));

   vprint(6, 9, cfg.col[Cpoptext], "Message size : %lu (body: %lu  controlinfo: %lu)",
                               (dword) (curmsg->txtsize + curmsg->ctrlsize),
                               curmsg->txtsize,
                               curmsg->ctrlsize);

   if(area->type == NETMAIL && (curmsg->mis.attr1 & (aFILE|aFRQ)))
     {
     vprint(7, 9, cfg.col[Cpoptext], "Files        : %-49.49s", SumAttaches(curmsg));
     }
   else
     print(7, 9, cfg.col[Cpoptext], "Files        : None");

   vprint(8, 9, cfg.col[Cpoptext], "Message UID  : %-5ld", curmsg->uid);

   /* Info about the current area */

   printn(10, 9, cfg.col[Cpoptext], "Area info:", 10);

   vprint(12, 9 , cfg.col[Cpoptext], "Desc : %-50.50s", area->desc);

   vprint(13, 9, cfg.col[Cpoptext], "Tag  : %s", area->tag);

   if(area->type == ECHOMAIL)
      strcpy(temp, "Echomail");
   else if(area->type == NETMAIL)
      strcpy(temp, "Netmail");
   else if(area->type == MAIL)
      strcpy(temp, "Mail");
    else if(area->type == NEWS)
      strcpy(temp, "News");
    else strcpy(temp, "Local");

   vprint(14, 9, cfg.col[Cpoptext], "Type : %s", temp);

   vprint(15, 9, cfg.col[Cpoptext], "Attr : %s", attr_to_txt(area->stdattr, 0L));

   if(!(area->base & MSGTYPE_HMB))
      sprintf(temp, "Path : %-50.50s", area->dir);
   else
      sprintf(temp, "Board: %-50.50s", area->dir);

   print(16,9,cfg.col[Cpoptext],temp);

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

   print(17,9,cfg.col[Cpoptext],temp);

   vprint(18,9,cfg.col[Cpoptext], "AKA  : %hu:%hu/%hu.%hu", cfg.usr.address[(int) area->aka].zone,
                                       cfg.usr.address[(int) area->aka].net,
                                       cfg.usr.address[(int) area->aka].node,
                                       cfg.usr.address[(int) area->aka].point);

   vprint(19, 9, cfg.col[Cpoptext], "Chars: %s (write), %s (read)", area->cswrite, area->csread);

   /* General info about system */

   #if !defined(__OS2__) && !defined(__NT__)
     printn(21, 9, cfg.col[Cpoptext], "General info:", 13);

     vprint(23,9,cfg.col[Cpoptext], "Memory free : %dK", ((unsigned long)coreleft())/1024);
//     if(_osmajor > 9)
//        sprintf(temp, "DOS version : OS/2-DOS");
//     else sprintf(temp, "DOS version : %d.%d", _osmajor, _osminor);
//     print(23, 9, cfg.col[Cpoptext],temp);
   #endif


   while( ((key=get_idle_key(1, GLOBALSCOPE)) == 99) || (key==67) || (key==105))
      {
      if(key == 105)
        {
        if(!(area->base & MSGTYPE_JAM))
          break;

        delbox(infobox);
        ShowFileInfo(curmsg);
        return;
        }

      if(!(area->base & MSGTYPE_SQUISH))
        break; /* We surely don't want to do this for *.MSG! */

      action=0;
      ltoa( (long) areadata->max_msg, max, 10);
      ltoa( (long) areadata->skip_msg, skip, 10);
      itoa( (int)  areadata->keep_days, keep, 10);


      while(action < 3)
      {
      vprint(17,28,cfg.col[Cpoptext], "%-4lu Skip=%-4lu Days=%-4u)                 ",
                                areadata->max_msg,
                                areadata->skip_msg,
                                areadata->keep_days);

      switch(action)
         {
         case 0:

            while( (ret != ESC) && (ret != 0) && (ret != TAB) )
               ret = getstring(17,28,max,4,4,"0123456789",cfg.col[Centry], cfg.col[Cpoptext]);

            areadata->max_msg   = atol(max );

            action = (ret == ESC) ? 3 : 1;
            ret=1;
            break;

         case 1:

            while( (ret != ESC) && (ret != 0) && (ret!=BTAB) && (ret!=TAB) )
               ret = getstring(17,38,skip,4,4,"0123456789",cfg.col[Centry], cfg.col[Cpoptext]);

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
               ret = getstring(17,48,keep,4,4,"0123456789",cfg.col[Centry], cfg.col[Cpoptext]);

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


      vprint(17,9,cfg.col[Cpoptext], "Base : Squish (Max=%lu Skip=%lu Days=%u) Hit 'C' to change.",
                                areadata->max_msg,
                                areadata->skip_msg,
                                areadata->keep_days);
      }

#ifndef __WATCOMC__
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
#endif

   delbox(infobox);

}

char * SumAttaches(MMSG *curmsg)
{
   static char temp[85];
   char tmpsize[15];
   struct stat mystat;
   dword totalsize=0;
   int files=0;
   STRINGLIST *current;

   temp[0]='\0';

   if(curmsg->mis.origbase & MSGTYPE_JAM)
     {
     strcpy(temp, "Press 'I' for more info..");
     return temp;
     }
   else if(curmsg->mis.attr1 & aFRQ)  // Not JAM, reqs always in subject...
     {
     strcpy(temp, "None");
     return temp;
     }

   current = curmsg->mis.attached;
   while(current)
     {
     if(current->s && (stat(current->s, &mystat) != -1))
        {
        files++;
        if(temp[0] != '\0')
           strcat(temp, "+ ");
        sprintf(tmpsize, "%lu ", mystat.st_size/1024);
        totalsize += mystat.st_size/1024;
        strcat(temp, tmpsize);
        }
     if(strlen(temp) > 70)
        break;
     current = current->next;
     }

   if(files == 1) strcat(temp, "KB");
   if(files > 1)
      {
      sprintf(tmpsize, "= %lu KB", totalsize);
      strcat(temp, tmpsize);
      }

   return temp;

}

// ================================================================

void ShowFileInfo(MMSG *curmsg)
{
  ATTACHLIST *alist, *current;
  STRINGLIST *curslist;
  int i, height, top;
  int numattaches=0, numreqs=0;
  BOX *box;

  if(curmsg->mis.attached == NULL && curmsg->mis.requested == NULL)
    return;

  alist = MakeAttachList(curmsg->mis.attached);

  for(curslist=curmsg->mis.attached; curslist; curslist=curslist->next)
     numattaches++;

  for(curslist=curmsg->mis.requested; curslist; curslist=curslist->next)
     numreqs++;

  height = max(numattaches, numreqs);
  if(height > (maxy - 6))
    height = maxy-6;

  top = maxy/2 - 2 - height/2;

  box = initbox(top,
                8,
                top+height+4,
                70,
                cfg.col[Cfindactive],
                cfg.col[Cfindtext],
                SINGLE,
                YES,
                ' ');
  drawbox(box);

  printc(top, 46, cfg.col[Cfindactive], 'Ñ');
  biprint(top+1, 8, cfg.col[Cfindactive], cfg.col[Cfindtext], "³ ~Attached files:~                     ³ ~Requested files:~      ³", '~');
  biprint(top+2, 8, cfg.col[Cfindactive], cfg.col[Cfindtext], "³                                     ³ ~Filename     Password~ ³", '~');
  print(top+3, 8, cfg.col[Cfindactive], "ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´");
  printc(top+height+4, 46, cfg.col[Cfindactive], 'Ï');
  for(i=top+4; i<top+height+4; i++)
    printc(i, 46, cfg.col[Cfindactive], '³');

  for(current=alist, i=0; current && i<height; current=current->next, i++)
    {
    vprint(top+4+i, 10, cfg.col[Cfindtext], "%-27.27s %4lu kB", current->name, current->size);
    }

  FreeAttachList(alist);

  for(curslist=curmsg->mis.requested, i=0; curslist && i<height; curslist=curslist->next, i++)
    {
    vprint(top+4+i, 48, cfg.col[Cfindtext], "%-12.12s %-8.8s",
                                curslist->s, curslist->pw ? curslist->pw : "");
    }

  get_idle_key(1, GLOBALSCOPE);
  delbox(box);

}

