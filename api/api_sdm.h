#ifndef __API_SDM_H_DEFINED
#define __API_SFM_H_DEFINED


#define MAX_SDM_CLEN  2048   /* Maximum number of bytes which can be used    *
                              * for kludge lines at top of *.MSG-type        *
                              * messages.                                    */


struct _msgh
{
  MSG *sq;
  dword id;      /* Must always equal MSGH_ID */

  dword bytes_written;
  dword cur_pos;
  word  mode;

  /* For *.MSG only! */

  sdword clen;
  sdword msg_len;
  sdword msgtxt_start;
  word zplen;
  int fd;

  MIS mis;
  dword msgnum;
  char *kludges;
  dword rawclen;
  dword bodylen;

  char *trail;
  dword trailsize;

};


/*************************************************************************/
/* This junk is unique to *.MSG!       NO APPLICATIONS SHOULD USE THESE! */
/*************************************************************************/

struct _sdmdata
{
  byte base[80];
  
  unsigned *msgnum;   /* has to be of type 'int' for qksort() fn */
  word msgnum_len;
    
  dword hwm;
  word hwm_chgd;
  
  word msgs_open;
};


#endif /* __API_SDM_H_DEFINED */

//int WriteZPInfo(XMSG *msg, void (*wfunc)(byte *str), byte *kludges);

