/***************************************************************************\
*
*
*   PROCEDURE         - Get DESQview Video Buffer
*
*   PURPOSE           - From DESQview API Manual:
*                       Return the address of the Logic Window buffer for the
*                       current task and begins "shadowing" changes to the
*                       buffer.  Programs that have been written to place
*                       text directly into the display memory can be easily
*                       converted to run in a small window by making this
*                       call and writing into the Logical Window Buffer
*                       instead.  DESQview checks periodically to see if the
*                       contents of the buffer have changed and, if so,
*                       updates the display accordingly.  The assumed value
*                       on entry should contain the address of display memory
*                       that the program would write into if running outside
*                       DESQview.  If this routine is called outside DESQview
*                       it does nothing but return with the assumed value.
*                       You can, therefore, make this call and trust the
*                       results either inside of outside DESQview.
*
*                       There is no need to synchronize with video retrace
*                       when writing into the Logical Window Buffer.
*
*   GENERAL ALGORITHM - Call the DESQview (TOPview standard) video function
*                       to return the video buffer address.
*
*   INPUT             - The assumed segment pointer of screen memory.
*
*   OUTPUT            - The assumed segment pointer given if not in DESQview,
*                       or the DESQview video buffer segment if in DESQview.
*
*   SYSTEM            - Borland's Turbo C++ v1.0 on an EPSON EQUITY III+
*                       running MS-DOS v4.01 & DESQview v2.26.  Should operate
*                       under any MS/PC-DOS 2.x or greater & DESQview 2.x or
*                       greater.
*
*   HISTORY:
*      90-07-28       - Initiated Mark Potter
*
\***************************************************************************/

#include <dos.h>
#include "dvaware.h"

//
// Machine code to push and pop various registers.
// implemented using #define instead of inline to avoid possible problems
// if Options/Compiler/C++ options.../Out-of-line inline functions turned on
// (or -vi isn't included in the command line compiler.
//
#define pushdi() __emit__( 0x57 )
#define pushes() __emit__( 0x06 )
#define popdi()  __emit__( 0x5F )
#define popes()  __emit__( 0x07 )


char _seg* dv_get_video_buffer(         // returns video buffer segment
   char _seg* assumed                   // assumed video buffer segment
) {
   pushdi();                            // save registers
   pushes();                            // save registers
   _ES = (unsigned int)assumed;         // get assumed segment into es
   _AH = 0xFE;                          // call get buffer
   geninterrupt( 0x10 );                // video BIOS call
   _AX = _ES;                           // put segment into accumulator
   popes();                             // restore registers
   popdi();                             // restore registers
   return (char _seg*)_AX;              // return segment
}

// END DVGETVID.CPP

