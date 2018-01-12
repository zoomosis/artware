#include <stdlib.h>
#include <conio.h>
#ifndef __OS2__
#include <bios.h>
#endif
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <video.h>
#include <scrnutil.h>
#include "idlekey.h"

#include <typedefs.h>

#define ESC 27

/* #define LEGAL "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'./ !?" */

int getstring(int row, int col, char *answer, int len, char *legalset, word colour);


/* main()

{
    char    answer[10],
				name[61];

	 video_init();

	 strcpy(name, "Default SysOp");

	 textattr(3);
	 clrscr();
	 getstring(1, 1, name, 60, LEGAL);
	 clrscr();
	 printf("En je antwoord was: %s\n", name);
    getch();

}  */


int getstring(int row, int col, char *answer, int len, char *legalset, word colour)

{
    int pos=0, ret=0, insert=1;
	 char temp[256], show[256];     /* Max input supported = 256 long.. */
	 char *charptr;
    int c=0;
    int keyspressed=0;


    window(1,1,80,25);
	 _setcursortype(_SOLIDCURSOR);
	 memset(temp, '\0', sizeof(temp));
	 strcpy(temp, answer);

	 memset(show, '°', sizeof(show));
	 strcpy(show, temp);
	 show[strlen(temp)]='°';
	 show[len]='\0';
    pos=strlen(temp);

     MoveXY(col+1+pos, row+1);
	 print(row, col, colour, show);

	 while (c != 13)
		  {
		  c = get_idle_key();

        keyspressed++;

		  switch(c) {
             case 25:              /* CTRL - Y */
               memset(temp, '\0', sizeof(temp));
               pos=0;
               break;

				 case 27:
					ret = ESC;          /* ESC */
               goto getout;

				 case 331:        /* left */
					  if (pos)
							  pos--;
					  break;
				 case 333:        /* right */
					  if (pos<strlen(temp))
							  pos++;
					  break;
				 case 335:        /* end */
				  pos = strlen(temp);
					  break;
				 case 327:        /* home */
					  pos = 0;
					  break;
             case 338:        /* insert */
                  if (!insert)
                     {
                     insert = 1;
                     _setcursortype(_SOLIDCURSOR);
                     }
                  else
                     {
                     insert = 0;
                     _setcursortype(_NORMALCURSOR);
                     }
                  break;
				 case 339:        /* del */
					  if(pos < strlen(temp))
						  {
						  memmove(temp+pos, temp+pos+1, strlen(temp)-pos-1);
						  temp[strlen(temp)-1] = '\0';
						  }
					  break;

             case 371:        /* ctrl - left */
                  charptr = temp + pos - 2;
                  while(charptr >= temp)
                     {
                     if (*charptr == ' ')
                        {
                        pos = (int) (charptr - temp + 1);
                        break;
                        }
                     else if (charptr == temp)
                        {
                        pos = 0;
                        break;
                        }
                     charptr--;
                     }
                  break;

             case 372:        /* ctrl - right */
                  charptr = temp + pos;
                  while(*charptr)
                     {
                     if (*charptr == ' ')
                        {
                        pos = (int) (charptr - temp + 1);
                        break;
                        }
                     charptr++;
                     }
                  break;

             case 373:       /* CTRL - END */
                  memset(temp+pos, '\0', strlen(temp) - pos);
                  break;


				case 8:                 /* backspace */
					if (pos)
						{
						memmove(temp+pos-1, temp+pos, strlen(temp)-pos);
						temp[strlen(temp)-1] = '\0';
						pos--;
						}
                break;

            default:
                if(
                    (strchr(legalset, c)!=NULL) || /* A legal char */
                    ( (legalset[0] == '\0') && ((c>30) && (c<256)) )
                  )
                  {
                  if(keyspressed == 1)
                     {     /* This is the first key */
                     memset(temp, '\0', sizeof(temp));
                     pos=0;
                     }

						if (pos<len)
                     {
                     if (!insert)
							   temp[pos++] = c;
                     else
                         {
                         if(strlen(temp) < len)
                            {
                            temp[strlen(temp)] = '\0';  /* Just be sure! */
                            memmove(temp+pos+1, temp+pos, strlen(temp) - pos);
                            temp[pos++] = c;
                            }
                         }
                     }
						}

		  } /* end switch */

		  memset(show, '°', sizeof(show));
		  strcpy(show, temp);
		  show[strlen(show)]='°';
		  show[len]='\0';

          MoveXY(col+1+pos,row+1);
		  print(row, col, colour, show);

		}   /* end while */

	strcpy(answer, temp);

   getout:

	memset(show, ' ', sizeof(show));
	strcpy(show, answer);
	show[strlen(show)]=' ';
	show[len]='\0';

   MoveXY(col+1+pos,row+1);
   print(row, col, 7, show);

	_setcursortype(_NOCURSOR);
	return ret;

}
