#include "includes.h"

#ifdef __WATCOMC__
#ifdef __OS2__

#define INCL_DOSDEVICES         /* Device values */
#include <os2.h>
#include <stdio.h>
#include <fcntl.h>
#else
#include <dos.h>
#include <string.h>
#include <i86.h>
#endif
#endif

int GetPrinterStatus(unsigned int printer_no);


int PrinterReady(char *printer)
{
    int status;
    int printerno;

    if (printer == NULL)        // Default printer
        printer = cfg.usr.printer;

    if (strncmpi(printer, "lpt", 3) == 0)
    {
        strupr(printer);
        sscanf(printer, "LPT%d", &printerno);
        status = GetPrinterStatus(printerno - 1);
    }
    else
        status = GetPrinterStatus(0);

    if (status & (0x01 & 0x08 & 0x20)) // timeout, IO error or out of
                                       // paper
        return 0;

    if ((status & 0x80) && (status & 0x10))
        return 1;
    else
        return 0;
}

#if !defined(__OS2__) && !defined(__NT__) && !defined(__UNIX__)

int GetPrinterStatus(unsigned int printer_no)
{
    union REGS regs;

    regs.h.ah = 2;
#ifdef __386__
    regs.x.edx = printer_no;
    int386(0x17, &regs, &regs);
#else
    regs.x.dx = printer_no;
    int86(0x17, &regs, &regs);
#endif
    return regs.h.ah;

}

#elif defined(__NT__) || defined(__UNIX__)

int GetPrinterStatus(unsigned int printer_no)
{
    return (0x80 | 0x10);
}

#else

/* OS/2 */

int GetPrinterStatus(unsigned int printer_no)
{
    HFILE DevHandle;            /* Device handle specifies the device */
    ULONG Category;             /* Device category */
    ULONG Function;             /* Device function */
    UCHAR ParmArea;             /* Command-specific argument list */
    ULONG ParmLengthMax;        /* Command arguments list max length */
    ULONG ParmLengthInOut;      /* Command arguments length (returned) */
    UCHAR DataArea;             /* Data area */
    ULONG DataLengthMax;        /* Data area maximum length */
    ULONG DataLengthInOut;      /* Data area length (returned) */
    APIRET rc;                  /* Return code */
    char name[6] = "";

    sprintf(name, "LPT%d", printer_no + 1);
    DevHandle = open(name, O_BINARY | O_WRONLY);

    Category = 0x05;            /* Specify device driver category hex 05 */
    Function = 0x66;            /* Specify device driver function hex 66 */

    ParmLengthInOut = 1;        /* are being passed to the device, */
    ParmLengthMax = 1;          /* and that there is no buffer to */
    /* receive parameters back from */
    /* the device */

    DataLengthInOut = 1;        /* Indicate that no input data is */
    /* being passed to the device */

    DataLengthMax = 1;          /* Indicate the maximum amount of data */
    /* (in bytes) that can be returned */
    /* to the caller by the device */

    ParmArea = 0;               // clear these, otherwise junk can remain! 
                                // (In practise!)
    DataArea = 0;

    rc = DosDevIOCtl(DevHandle, Category, Function, &ParmArea,
                     ParmLengthMax, &ParmLengthInOut, &DataArea,
                     DataLengthMax, &DataLengthInOut);

    close(DevHandle);

    return rc ? 0x08 : DataArea; // 0x08 == IOerror

}

#endif
