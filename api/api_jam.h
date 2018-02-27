
struct _msgh
{
    MSGA *sq;
    dword id;                   /* Must always equal MSGH_ID */

    dword bytes_written;
    dword cur_pos;

    /* For JAM only! */

    dword clen;
    dword cur_len;
    dword msgnum;
    dword totlen;

    long hdroffset;
    long txtoffset;

    word mode;

    /* Data that is 'buffered' 'cause we need to analyse it to */
    /* find out lengths anyway.  */

    char *kludges;

    MIS mis;

};


#define JamData ((JAMDATA *)sq->apidata)
