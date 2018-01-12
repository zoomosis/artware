#include "includes.h"
/*
**  Tasker.C
**
**  public domain by David Gibbs
*/

#ifdef __WATCOMC__
extern void os2slice(void);

#pragma aux os2slice = \
   " mov ax, 1680h "   \
   " int 2fh       "   \
   modify [ax]         ;

#endif

#define TOT_OS  5

#define DOS     0
#define OS2     1
#define DV      2
#define WINS    3
#define WIN3    4
                        /*   76543210  */
#define is_DOS  0x01    /* b'00000001' */
#define is_OS2  0x02    /* b'00000010' */
#define is_DV   0x04    /* b'00000100' */
#define is_WINS 0x08    /* b'00001000' */
#define is_WIN3 0x10    /* b'00010000' */

int t_os_type;
int t_os;


void check_mtask(void)
{
      union REGS t_regs;

      t_os_type = 0;
      t_os = 0;

      /* test for DOS or OS/2 */

      if (_osmajor < 10)
      {
            t_os_type = t_os_type | is_DOS;
      }
      else
      {
            t_os_type = t_os_type | is_OS2;
      }

      /* test for Windows */

      t_regs.w.ax = 0x4680;
      int386(0x2F, &t_regs, &t_regs);

      if (t_regs.w.ax == 0x0000)
      {
            t_os_type = t_os_type | is_WINS;
      }
      else
      {
            t_regs.w.ax = 0x1600 ;
            int386(0x2F, &t_regs, &t_regs);

            switch (t_regs.h.al)
            {
            case 0x00 :
            case 0x80 :
            case 0x01 :
            case 0xFF :
                  break;

            default   :
                  t_os_type = t_os_type | is_WIN3;
                  break ;
            }  /* endswitch  */
      } /* endif */

      /* Test for DESQview */

      t_regs.w.cx = 0x4445;     /* load incorrect date */
      t_regs.w.dx = 0x5351;
      t_regs.w.ax = 0x2B01;     /*  DV set up call     */

      intdos(&t_regs, &t_regs);
      if (t_regs.h.al != 0xFF)
      {
            t_os_type = t_os_type | is_DV;
      }

      if(t_os_type & is_DOS)
            t_os = DOS;

      if(t_os_type & is_WINS)
            t_os = WINS;

      if(t_os_type & is_WIN3)
            t_os = WIN3;

      if(t_os_type & is_DV)
            t_os = DV;

      if(t_os_type & is_OS2)
            t_os = OS2;

}

void give_slice(void)
{
      union REGS t_regs;
    
      switch (t_os)
      {
      case DOS  :
//            asm int 28
            int386(0x28,&t_regs,&t_regs);
            break;

      case OS2  :
      case WIN3 :
      case WINS :
            os2slice();
            break;

      case DV   :
            t_regs.w.ax = 0x1000;
            int386(0x15,&t_regs,&t_regs);
            break;
      } /* switch(t_os) */
}
