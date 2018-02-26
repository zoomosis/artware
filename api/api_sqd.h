
struct _sqdata
{

    int sfd;                    /* SquishFile handle */
    int ifd;                    /* SquishIndex handle */

    byte base[80];              /* Base name for SquishFile */

    FOFS begin_frame;           /* Offset of first frame in file */
    FOFS last_frame;            /* Offset to last frame in file */
    FOFS free_frame;            /* Offset of first FREE frame in file */
    FOFS last_free_frame;       /* Offset of LAST free frame in file */
    FOFS end_frame;             /* Pointer to end of file */

    FOFS next_frame;
    FOFS prev_frame;
    FOFS cur_frame;

    dword uid;
    dword max_msg;
    dword skip_msg;
/*dword zero_ofs;*/
    word keep_days;

    byte flag;
    byte rsvd1;

    word sz_sqhdr;
    byte rsvd2;

    word len;                   /* Old length of sqb structure */

    dword huge *uidlist;
    dword maxlen;

    struct _sqbase delta;       /* Copy of last-read sqbase, to determine
                                   changes */
    word msgs_open;

};
