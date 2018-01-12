
 SQReadIndex(char *path)
{
  dword ofslen;

  /* Seek to end, and find length of file */

  lseek(Sqd->ifd, 0L, SEEK_END);
  ofslen=tell(Sqd->ifd);

  Sqd->idxbuf_used=ofslen;
  Sqd->idxbuf_write=ofslen;
  Sqd->idxbuf_delta=ofslen;
  Sqd->idxbuf_size=ofslen+(EXTRA_BUF*(dword)sizeof(SQIDX));
  
  if (
#ifdef __MSDOS__
    Sqd->idxbuf_size >= 65300L ||
#endif
      (Sqd->idxbuf=farpalloc((size_t)Sqd->idxbuf_size))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return -1;
  }

  /* Seek to beginning of file, and try to read everything in */
  
  if (ofslen &&
      (lseek(Sqd->ifd, 0L, SEEK_SET)==-1 ||
       farread(Sqd->ifd, (char far *)Sqd->idxbuf, (unsigned)ofslen) !=
          (int)ofslen))
  {
    farpfree(Sqd->idxbuf);
    Sqd->idxbuf=NULL;
    msgapierr=MERR_BADF;
    return -1;
  }
  
  return 0;
}

