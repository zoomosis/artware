#include <conio.h>
#include <ctype.h>
#include <direct.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include <ctype.h>

#define addbackslash(x)  if (x[0]) if (x[strlen(x)-1]!='\\') strcat(x,"\\")


#ifdef __OS2__
 #define DW unsigned short
#else
 #define DW unsigned int
#endif

  #define INBUFSIZE 32000
  #define OUTBUFSIZE 8192

  #define bumpby 35

// #define messy    This define shows us what it's compiling

char defaultdir[256] = ".\\";
char defaultnodelist[256] = "NODELIST";
char workbuf[256];
int append=0;

int inhandle=0;
char *inbuf=NULL;
int insize=0,inptr=0,inread=0;

FILE *outfile=NULL;
   char namebuf[256];

  static char *wb1=&workbuf[1];
  static char *wb4=&workbuf[4];
  static char *wb5=&workbuf[5];
  static char *wb7=&workbuf[7];
  static char *wb8=&workbuf[8];

  static DW uZONE=65535u;
  static DW uHOST=65534u;



void myatoi(char *p)
{
 DW u=0;
 while ((*p)!=',')
  u=(DW) (u*10-'0'+(*p++));
 if ((u&uHOST)==uHOST) // also covers uZONE
  {
  cprintf("Skipping a node using 65534 or 65535 as an address.\r\n");
  u=0;
  }
 fwrite(&u,2,1,outfile);
}

/*
char nextchar(void)
{
 if (inptr>=inread)
   {
    inptr=0;
    inbuf[0]=0;
    inread=read(inhandle,inbuf,insize);
    if (!inread) return 0;
   }
 return(inbuf[inptr++]);
}
*/
#define prepnextchar(); if (inptr>=inread) {inptr=0;inbuf[0]=0;inread=read(inhandle,inbuf,insize);}
#define nextchar() inbuf[inptr++]

void nextline(char *workbuf)
{
 register char ch;
runagain:
 *workbuf=0;
 prepnextchar();
 *workbuf=nextchar();
 if (!*workbuf) return;
 if (*workbuf==13) return;
 if (*workbuf==27) return;
 if (*workbuf==10) return;
 if (*workbuf==';')
   {
    do
     {prepnextchar();ch=nextchar();}
    while ((ch!=0) && (ch!='\n'));
    goto runagain;
   }
 if (*workbuf==26) {*workbuf=0;return;}
 do  //first string + ,
   {workbuf++;
    prepnextchar();
    *workbuf=ch=nextchar();}
 while (*workbuf!=',');

 do  // second char + ,
   { workbuf++;
     prepnextchar();
     *workbuf=nextchar();}
 while (*workbuf!=',');

 inptr+=bumpby;
 if (inptr>=inread) inptr=inread;

 do
   {prepnextchar();
    ch=nextchar();}
    while ((ch!=0) && (ch!='\n'));
 return;
}
int why=0;
void parselines(void)
{
//  register DW value;

/*
    Hub,        4
    Pvt,        4
    Zone,       5
    Host,       5
    Region,     7
    */

newline:
  nextline(workbuf);
  switch (*workbuf)
   {
    case 0   : return;

    case ',' : myatoi(wb1);
               #ifdef messy
                 gotoxy(12,why);cprintf("%4u",atoi(wb1));
               #endif
               goto newline;

    case 'H' : if (workbuf[1]=='u') goto Hub; // we only want HOST here
               if (workbuf[2]=='l') goto priv; // DOWN? make it private
               #ifdef messy
                 gotoxy(7,why);cprintf("%4u",atoi(wb5));
               #endif
               fwrite(&uHOST,2,1,outfile);
               myatoi(wb5);goto newline;

    Hub      : myatoi(wb4);goto newline;

//    case 'p' :
    priv     :
    case 'D' :  // down
    case 'P' : myatoi(wb4); goto newline;

    case 'R' : fwrite(&uHOST,2,1,outfile);
               #ifdef messy
                 gotoxy(7,why);cprintf("%4u",atoi(wb7));
               #endif
               myatoi(wb7);goto newline;

    case 'Z' : fwrite(&uZONE,2,1,outfile);
               #ifdef messy
                 gotoxy(1,why);cprintf("%4u",atoi(wb5));
               #else
     	       cprintf("\r\nZone %u",atoi(wb5));
               #endif
               myatoi(wb5);goto newline;

   }
  goto newline;
}
void parsenodelist(void)
{  int ibsize;
  inhandle=sopen(defaultnodelist, O_BINARY|O_RDONLY, SH_DENYNO, S_IREAD);
  if (inhandle<1)
   {
    cprintf("Could not open nodelist file %s.\r\n",defaultnodelist);
    exit(1);
   }


  if (append)
    outfile= fopen("NODELIST.GIG", "ab");
  else
    outfile= fopen("NODELIST.GIG", "wb");
  if (!outfile)
   {
    cprintf("Could not open output index file %s.\r\n",workbuf);
    exit(1);
   }

  if (setvbuf(outfile, NULL, _IOFBF, OUTBUFSIZE) != 0)
     cprintf("Failed to set up output buffer\r\n");

  ibsize=INBUFSIZE;
 loop:
  if (ibsize<256)
   {
    cprintf("Failed to set up input buffer.\r\n");
    fclose(outfile);
    exit(1);
   }
  inbuf=(char *) calloc(1,ibsize);
  if (!inbuf) {ibsize-=128;goto loop;}
  insize=ibsize;inptr=0,inread=0;
#ifdef messy
  why = wherey();
#endif
  parselines();

  close(inhandle);
  fclose(outfile);
}

char *getnodelist(char *dir, char *name)
{ 
char *p; 

DIR *dirp;
char buf[255];
unsigned long d_date,d_time;
struct dirent *direntp;

   int valuemode=0;
   strcpy(namebuf,dir);
   addbackslash(namebuf);
   strcat(namebuf,name);
   if (!strchr(name,'.')) {valuemode=1;strcat(namebuf,".*");}
   buf[0]=0; d_time=d_date=0;

dirp = opendir(namebuf); 
if (dirp == NULL) {
   fprintf(stderr,"Could not opendir: %s\r\n",namebuf);
} else {
   for (;;) {
      direntp = readdir(dirp);
      if (direntp==NULL) break;
      p=strchr(direntp->d_name,'.');
      if (p==NULL) continue;
      p++;
      if (!isdigit(*p)) continue;
      if ((direntp->d_date > d_date) && (direntp->d_time > d_time))
           {
           strcpy(buf,direntp->d_name);
           d_date = direntp->d_date;
           d_time = direntp->d_time;
            }
    }
   closedir(dirp);
} /* endif */

   if (!buf[0])
     {
      cprintf("No nodelist files found.\r\n");
      exit(1);
     }
   strcpy(namebuf,dir);
   addbackslash(namebuf);
   strcat(namebuf,buf);
   return namebuf;
}

void main(int argc, char *argv[])
{
//  clrscr();
  if (argc > 1)
   {
    if (strcmp(argv[1],"/?")==NULL)
      {
  cputs("\r\n\r\n");
  cprintf("Nodelist Index Builder for GIGO vsn "__DATE__"\r\n");
  cprintf("Usage:  COMPILE [path] [nodelistname] [/Append]\r\n"
                 " Note that PATH and NODELISTNAME _must_ be specified\r\n"
                 " seperately for this program to work.  It's quick, dirty,\r\n"
                 " and way fast!  /APPEND may be specified if you need to\r\n"
		 " run this for more than one nodelist file.  Sample:\r\n"
                 "   C> COMPILE .\ NODELIST\r\n"
		 "   C> COMPILE .\ EGGNET /A\r\n");
     return;
      }
    strcpy(defaultdir,argv[1]);
     }
  addbackslash(defaultdir);
  if (argc > 2)
    strcpy(defaultnodelist,argv[2]);
  if (argc > 3)
    for (int index=3; index < argc; index++)
     {
       if (strnicmp(argv[index],"/A",2)==NULL)
         append=1;
     }
  strcpy(defaultnodelist,getnodelist(defaultdir,defaultnodelist));
  cprintf("Nodelist Index Builder for GIGO vsn "__DATE__"\r\n");
  cprintf("Type %s /? for help info.\r\n",argv[0]);
  cprintf(       "\r\nOptions selected:\r\n"
                 "  Directory:  %s\r\n"
                 "  Nodelist:   %s\r\n"
                 "  Append:     %s\r\n\n",defaultdir,defaultnodelist,append?"Yes":"No (Overwriting)");

 if (!append)
  unlink("NODELIST.GIG");
 parsenodelist();
 cprintf("\r\n");
}
