#include "includes.h"


int ChooseTagset(void);
char * GetDesc(int no);


// Save areatags to a diskfile

void SaveTags(void)
{
   BOX *savebox;
   int choice, ret;
   FILE *out;
   char name[120];
   char desc[60];
   AREA *curarea;
   char *tilde;

   choice = ChooseTagset();

   if(choice >= 0 && choice <= 9)
     {
     strcpy(desc, GetDesc(choice));

     savebox=initbox(maxy/2-2, maxx/2-30, maxy/2+2, maxx/2+30,
                          cfg.col[Cpopframe], cfg.col[Cpoptext], SINGLE, YES, ' ');

     drawbox(savebox);
     titlewin(savebox, TLEFT, " Description", 0);
     ret = getstring(maxy/2, maxx/2-28, desc, 55, 55, "" , cfg.col[Centry], cfg.col[Cpoptext]);
     delbox(savebox);

     if(ret != 0)
        return;

     // Replace tildes, will mess up 'picklist()' function..
     while( (tilde=strchr(desc, '~')) != NULL )
        *tilde = '^';

     sprintf(name, "%s\\timed%d.tag", cfg.homedir, choice);
     if( (out=fopen(name, "w")) == NULL )
       Message("Error opening file!", -1, 0, YES);
     else
       {
       fprintf(out, ";%s\n", desc);   // First description..
       for(curarea = cfg.first; curarea; curarea=curarea->next)
           if(curarea->tagged) fprintf(out, "%s\n", curarea->tag);
       fclose(out);
       }
     }

}


// Load areatags from a diskfile


void LoadTags(void)
{
   int choice;

   choice = ChooseTagset();

   if(choice != -1)
     {
     if(ReadTagFile(choice) == -1)
        Message("Error reading tagfile!", -1, 0, YES);
     }

}

// ======================================================

int ChooseTagset(void)
{
   int choice, i;
   char **setlist;
   char temp[80];

   setlist = mem_calloc(11, sizeof(char *));

   for(i=0; i<10; i++)
     {
     sprintf(temp, " ~%d~ - %-55s ", (char) i, GetDesc(i));
     setlist[i] = mem_strdup(temp);
     }

   choice = picklist(setlist, NULL, 5, 10, 20, 79);

   free_picklist(setlist);

   if(choice >= 0 && choice <= 9)
      return choice;
   else
      return -1;

}


// Read a certain tagfile in memory.

int ReadTagFile(int choice)
{
   XFILE * in;
   char    name[120];
   AREA *  curarea;
   char *  line;


   sprintf(name, "%s\\timed%d.tag", cfg.homedir, (char) choice);

   if( (in=xopen(name)) == NULL )
     return -1;

   // clear all tags
   for(curarea=cfg.first; curarea; curarea=curarea->next)
       curarea->tagged = 0;

   while( (line=xgetline(in)) != NULL)
     {
     if(strlen(line) == 0) continue;

     for(curarea = cfg.first; curarea; curarea=curarea->next)
         if(!strcmpi(curarea->tag, line)) curarea->tagged = 1;
     }

   xclose(in);

   return 0;

}

// =========================================================

char * GetDesc(int no)
{
   XFILE * in;
   char    name[120];
   char *  line;
   static char desc[80];

   strcpy(desc, "Not found.");

   sprintf(name, "%s\\timed%d.tag", cfg.homedir, no);

   if( (in=xopen(name)) == NULL )
     return desc;

   if( (line=xgetline(in)) != NULL)
     {
     if(line[0] == ';')             // description?
        strncpy(desc, line+1, 78);  // Copy it!
     else
        strcpy(desc, "No description present.");  // Better than nothing!
     }

   xclose(in);

   return desc;
}

