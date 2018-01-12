#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#pragma aux __PRO "__PRO";
#pragma aux __EPI "__EPI";

// lengte byte staat op ESP-4
// <lengte> bytes daarvoor staat de naam van de functie..

static char * ptr;
char format[60];

void AddFunction(char *f);
extern char * getptr(void);

#pragma aux getptr = \
   "mov eax, [esp+36]"    \
   value  [eax]      \
   modify [eax]   ;


typedef struct _ftrack
{

   char *  s;
   int     returned;
   clock_t begin;
   clock_t end;

} FTRACK;


#define MAXFUNCS 100

static FTRACK FunctionList[MAXFUNCS];
static int curfunc = 0;


// =====================================================

void __PRO(void)
{
   size_t len=0;
   char *name;

   ptr = getptr();
   len = *(ptr-9);
   name = ptr-9-len;

//   sprintf(format, "Called \"%%-%d.%ds\"\n", len, len);
//   printf(format, name);
   strncpy(format, name, len);
   format[len] = '\0';
   AddFunction(format);

}

// =====================================================

void __EPI(void)
{

   FunctionList[curfunc-1].returned = 1;
   FunctionList[curfunc-1].end      = clock();

}

// =====================================================

void AddFunction(char *f)
{

   if(curfunc == MAXFUNCS)
     {
     if(FunctionList[0].s)
       free(FunctionList[0].s);
     memmove(&FunctionList[0], &FunctionList[1], sizeof(FTRACK) * (MAXFUNCS-1));
     curfunc--;
     }

   FunctionList[curfunc].s        = strdup(f);
   FunctionList[curfunc].begin    = clock();
   FunctionList[curfunc].returned = 0;
   curfunc++;

}

// =================================================================

void PrintFuncList(void)
{
   int i;

   printf("\nFunction tracking - CLOCKS_PER_SEC: %lu\n\n", CLOCKS_PER_SEC);

   for(i=0; i<curfunc; i++)
     {
     printf("Name: %-20.20s Ret: %s  Time: %lu\n",
                  FunctionList[i].s,
                  FunctionList[i].returned ? "Yes" : "No ",
                  (unsigned long) FunctionList[i].end - FunctionList[i].begin);
     }
}

// =================================================================

