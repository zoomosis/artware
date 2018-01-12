/***************************************************************************\
*
*
*   PROCEDURE         - DESQview TC++ ConIO
*
*   PURPOSE           - makes the TC++ version 1.0 conio routines
*                       DESQview screen aware.
*
*   GENERAL ALGORITHM - The author (MAP) has noted that during the TC++
*                       start up code a program identifies the screen type of
*                       screen it is running on, and places the segment address
*                       of the screen display in the word directly before the
*                       directvideo.  Therefore, using this as the input to
*                       dv_get_video_buffer() and placing the return value in
*                       the same location should make all the later conio
*                       calls use the DESQview video buffer.  This function
*                       should be called as close to the begin of the programs
*                       executions as possible.  It will no longer be required
*                       to set directvideo to 0 for the program to be DESQview
*                       screen nice.
*
*   INPUT             - none.
*
*   OUTPUT            - none.
*
*   SYSTEM            - Borland's Turbo C++ v1.0 on an EPSON EQUITY III+
*                       running MS-DOS v4.01 & DESQview v2.26.  Should operate
*                       under any MS/PC-DOS 2.x or greater & DESQview 2.x or
*                       greater.  Unknown if it will work under any version
*                       of TC greater than 1.0.
*
*   HISTORY:
*      90-07-28       - Initiated Mark Potter
*
\***************************************************************************/

#include <conio.h>

#include "dvaware.h"

void dv_conio( void ) {
   *(&directvideo-1) = (unsigned int)dv_get_video_buffer(
                             (char _seg*)(*(&directvideo-1)) );
}



// END DVCONIO.CPP

