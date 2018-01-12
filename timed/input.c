#include "includes.h"

/* #define LEGAL "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-'./ !?" */


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


int getstring(int row, int col, char *answer, int len, int maxlen, char *legalset, word colour, word exitcol)

{
    int pos=0, ret=0, insert=1;
    char temp[301], show[301];     /* Max input supported = 256 long.. */
	 char *charptr;
    int c=0;
    int keyspressed=0;
    int start = 0;

    if(maxlen > 300)
      return 0;

    if(strlen(answer) > maxlen)
       {
       Message("String too long to edit!", -1, 0, YES);
       return 0;
       }

    #ifdef __WATCOMC__
    _settextcursor(0x0407);
    #else
    _setcursortype(_SOLIDCURSOR);
    #endif
	 memset(temp, '\0', sizeof(temp));
	 strcpy(temp, answer);

    pos=strlen(temp);

    if(pos >= len && (pos-len+1) > start)
      start = pos-len+1;

    memset(show, '°', sizeof(show));
    strcpy(show, temp+start);
    show[strlen(show)]='°';
    show[len]='\0';

    MoveXY(col+1+pos-start,row+1);
    print(row, col, colour, show);

	 while (c != 13)
		  {
        c = get_idle_key(1, GLOBALSCOPE);

        keyspressed++;

		  switch(c) {
             case 25:              /* CTRL - Y */
               memset(temp, '\0', sizeof(temp));
               pos=0, start=0;
               break;

				 case 27:
					ret = ESC;          /* ESC */
               goto getout;

             case 316:        /* F2 */
               ret = F2;
               goto getout;

             case 292:              /* ALT-J */
               shell_to_DOS();
               #ifdef __WATCOMC__
               _settextcursor(0x0607);
               #else
               _setcursortype(_NORMALCURSOR);
               #endif
               break;

				 case 331:        /* left */
                 if(pos)
                    pos--;
                 if(pos<start)
                    start=pos;
                 break;

				 case 333:        /* right */
                 if(pos<strlen(temp))
                    pos++;
                 break;

				 case 335:        /* end */

                 pos = strlen(temp);
                 break;

				 case 327:        /* home */
                 pos = 0, start=0;
					  break;

             case 338:        /* insert */
                  if (!insert)
                     {
                     insert = 1;
                     #ifdef __WATCOMC__
                     _settextcursor(0x0407);
                     #else
                     _setcursortype(_SOLIDCURSOR);
                     #endif
                     }
                  else
                     {
                     insert = 0;
                     #ifdef __WATCOMC__
                     _settextcursor(0x0607);
                     #else
                     _setcursortype(_NORMALCURSOR);
                     #endif
                     }
                  break;
				 case 339:        /* del */

					  if(pos < strlen(temp))
						  {
						  memmove(temp+pos, temp+pos+1, strlen(temp)-pos-1);
						  temp[strlen(temp)-1] = '\0';
						  }
					  break;

             case 328:        /* up */
                 ret = UP;
                 goto getout;

             case 336:        /* down */
                  ret = DOWN;
                  goto getout;

             case 271:        /* TAB left */
                  ret = BTAB;
                  goto getout;

             case 286:        /* ALT - A */
                  ret = ATTACH;
                  strcpy(answer, temp);
                  goto getout;

//             case 275:        /* ALT - R */
//                  ret = REQUEST;
//                  goto getout;

              case 287:        /* ALT - S */
                  ret = SAVE;
                  strcpy(answer, temp);
                  goto getout;

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

             case 275:
                  ret = RESET;
                  strcpy(answer, temp);
                  goto getout;

				case 8:                 /* backspace */
               if(pos)
                 {
                 memmove(temp+pos-1, temp+pos, strlen(temp)-pos);
                 temp[strlen(temp)-1] = '\0';
                 pos--;
                 }
               break;

            case 9:
                 ret = TAB;
                 goto getout;

            case 10:
                 ret = ACCEPT;
                 goto getout;

            case 315:
                 ret = HELP;
                 goto getout;

            default:
                if(
                    (strchr(legalset, c)!=NULL) || /* A legal char */
                    ( (legalset[0] == '\0') && ((c>30) && (c<256)) )
                  )
                  {
                  if((keyspressed == 1) && (cfg.usr.status & JUMPYEDIT))
                     {     /* This is the first key */
                     memset(temp, '\0', sizeof(temp));
                     pos=0;
                     }

                  if (pos<maxlen)
                     {
                     if (!insert)
							   temp[pos++] = c;
                     else
                         {
                         if(strlen(temp) < maxlen)
                            {
                            temp[strlen(temp)] = '\0';  /* Just be sure! */
                            memmove(temp+pos+1, temp+pos, strlen(temp) - pos);
                            temp[pos++] = c;
                            }
                         }
                     }
						}

		  } /* end switch */

        if(pos<start)
           start=pos;

        if(pos >= len && (pos-len+1) > start)
          start = pos-len+1;

//        vprint(20,0,7,"Pos: %d  Len: %d  Start: %d  Strlen: %d", pos, len, start, strlen(temp));

		  memset(show, '°', sizeof(show));
        strcpy(show, temp+start);
		  show[strlen(show)]='°';
		  show[len]='\0';

        MoveXY(col+1+pos-start,row+1);
		  print(row, col, colour, show);

		}   /* end while */

	strcpy(answer, temp);

   getout:

	memset(show, ' ', sizeof(show));
	strcpy(show, answer);
	show[strlen(show)]=' ';
	show[len]='\0';

   MoveXY(col+1+pos,row+1);
   print(row, col, exitcol, show);

   #ifdef __WATCOMC__
     _settextcursor(0x2000);
   #else
     _setcursortype(_NOCURSOR);
   #endif

	return ret;

}
