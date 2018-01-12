#include "includes.h"

#ifdef __OS2__
 #define INCL_DOS
 #define INCL_DOSERRORS
 #include <os2.h>
 #include <process.h>

int clockthreadid = -1;
static HMTX clocksem  = (HMTX) NULL;
static HMTX activesem = (HMTX) NULL;

void clockthread(void *);
#endif


void clockon(void)
{
#ifdef __OS2__
  #define STACKSIZE 32767
#endif

#ifndef __OS2__
  if(!(cfg.usr.status & CLOCK))
    return;
#endif

   if(showclock == 1) return;

   showclock = 1;

   #ifdef __OS2__
   if(clockthreadid == -1)          // not started yet?
     {
     if(DosCreateMutexSem(NULL, &clocksem, 0L, FALSE) != 0)
        Message("Error creating clock semaphore (clocksem)!", -1, 0, YES);

     if(DosCreateMutexSem(NULL, &activesem, 0L, FALSE) != 0)
        Message("Error creating clock semaphore (activesem)!", -1, 0, YES);

     if(DosRequestMutexSem(activesem, 5) != 0)
        Message("Error Requesting active semaphore!", -1, 0, YES);
 
     if((clockthreadid = _beginthread(clockthread, NULL, STACKSIZE, NULL)) == -1)
        Message("Error starting clock thread!", -1, 0, YES);
     }
   #endif

   update_clock(1);
}


void clockoff(void)
{
   #ifdef __OS2__
   APIRET rc;
   #endif

   if(showclock == 0) return;

   #ifdef __OS2__
   if((rc=DosRequestMutexSem(clocksem, 5000)) != 0)
     {
     if(rc == ERROR_TIMEOUT)
        Message("DosRequestMutexSem timed out!", -1, 0, YES);
     else
       Message("Unknown error DosRequestMutexSem!", -1, 0, YES);
     }
   #endif

   showclock = 0;

   #ifdef __OS2__
   if(DosReleaseMutexSem(clocksem) != 0)
     Message("Error releasing Clock semaphore!", -1, 0, YES);
   #endif

}

void update_clock(int forced)
{
   time_t now;
   struct tm *mytime;
   char temp[20];
   static time_t lasttime=0;
   #ifdef __OS2__
   APIRET rc;
   #endif

   if(!showclock) return;


   time(&now);
#ifndef __OS2__
   if(!forced && (now < (lasttime+10)))
     return;
   lasttime=now;
#endif

   mytime = localtime(&now);
   sprintf(temp, "³ %02d:%02d ", mytime->tm_hour, mytime->tm_min);

   #ifdef __OS2__
//   if(!forced) DosEnterCritSec();  --> This causes lockups!! Crit <-> Video output?
//   if(!forced) DosSleep(1);
   if((rc=DosRequestMutexSem(clocksem, 5000)) != 0)
     {
     if(rc == ERROR_TIMEOUT)
        Message("DosRequestMutexSem timed out!", -1, 0, YES);
     else
       Message("Unknown error DosRequestMutexSem!", -1, 0, YES);
     }

   if(showclock)
      VioWrtCharStrAtt(temp, (int) strlen(temp), maxy-1, maxx-8,(PBYTE) &cfg.col[Cmsgbar], 0);

   #else
      print(maxy-1, maxx-8, cfg.col[Cmsgbar], temp);
   #endif
   #ifdef __OS2__
   if(DosReleaseMutexSem(clocksem) != 0)
     Message("Error releasing Clock semaphore!", -1, 0, YES);

   #endif
}


#ifdef __OS2__
void clockthread(void *fake)
{
 while(1)
   {
   if(showclock) update_clock(0);

//   DosSleep(10000);

//   if(DosRequestMutexSem(activesem, 5000) == 0)
   if(DosRequestMutexSem(activesem, 500) == 0)
      {
//      DosBeep(50,2000);
      clockthreadid = -1;
      _endthread();  // != 0 --> timeout, so go on!
      }
//   beep(); beep();
   }
}


void killclock(void)
{
   int rc;
   TID threadid = clockthreadid;

   if(clockthreadid != -1)
      {
      if((rc=DosReleaseMutexSem(activesem)) != 0)
         {
         sprintf(msg, "Error releasing Clock semaphore (active, rc= %d)!", rc);
         Message(msg, -1, 0, YES);
         }

      if(DosCloseMutexSem(clocksem) != 0)
        Message("Error closing semaphore (clock)!", -1, 0, YES);

      // Sem is released, clockthread will catch that and end itself!
      // To be sure the thread is ended, we will now wait for the clock-
      // thread to end its cleanup actions..

      rc = DosWaitThread(&threadid, (ULONG) 0);

      if(rc != 0 && rc != 309)
        {
          DosBeep(50,2000);
          printf("\n\n\nError in 'DosWaitThread': %d, TID: %d\n", rc, threadid);
          getch();
        }

      }
}
#endif
