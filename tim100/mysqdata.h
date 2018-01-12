
struct _mysqdata
{

  int sfd;                /* SquishFile handle */
  int ifd;                /* SquishIndex handle */

  char base[80];          /* Base name for SquishFile */

  long begin_frame;       /* Offset of first frame in file */
  long last_frame;        /* Offset to last frame in file */
  long free_frame;        /* Offset of first FREE frame in file */
  long last_free_frame;   /* Offset of LAST free frame in file */
  long end_frame;         /* Pointer to end of file */

  long next_frame;
  long prev_frame;
  long cur_frame;

  dword uid;
  dword max_msg;
  dword skip_msg;
/*dword zero_ofs;*/
  word keep_days;
  
  byte flag;
  byte rsvd1;
  
  word sz_sqhdr;
  byte rsvd2;

  word len;              /* Old length of sqb structure                     */

  dword idxbuf_size;     /* Size of the allocated buffer                    */
  dword idxbuf_used;     /* # of bytes being used to hold messages          */
  dword idxbuf_write;    /* # of bytes we should write to index file        */
  dword idxbuf_delta;    /* Starting position from which the index has chhg */
  
  byte  delta[256];      /* Copy of last-read sqbase, to determine changes   */
  word msgs_open;
  
};


